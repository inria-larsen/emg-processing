/*
 * EMG human 
 * 
 * Author: Serena Ivaldi
 * email:  serena.ivaldi@inria.fr
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/



#include <yarp/sig/Vector.h>
#include <yarp/os/all.h>
#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

// utils for printing parameters
#define DSCPA(V) cout<<"  "<< #V <<" : "<<V<<endl;
#define DSCPAv(V) cout<<"  "<< #V <<" : "<<V.toString()<<endl;
#define DSCPAs(S,V) cout<<"  "<< S <<" : "<<V.toString()<<endl;
#define DSCPAd(S,V) cout<<"  "<< S <<" : "<<V<<endl;


//===============================
//        EMGhuman THREAD
//===============================

class EMGhumanThread: public RateThread
{
    protected:

        // name used for the ports
        string name;
        // current time
        double curTime;

    public: 

    EMGhumanThread(const double _period, string _name): RateThread(int(_period*1000.0))
    {
        name = _name;
        yInfo("EMGhuman: thread created");

    }

    virtual bool threadInit()
    {
       
        // opening ports

        return true;

    }

    virtual void threadRelease()
    {
        //closing all the ports

    

        yInfo("EMGhuman: thread closing");

    }

    //------ RUN -------
    virtual void run()
    {
        // cyclic operations should be put here!
        curTime = Time::now();

        // read EMG values from EMG server

        // compute stiffness
        
        // compute ICC

        // compute effort 

        // send output to ports



    }




};

//===============================
//        EMGhuman MODULE
//===============================
class EMGhuman: public RFModule
{
private:

     // counter for the number of minutes of execution
    int count;
    // the port to handle messages
    Port rpc; 
    // name of the module, used for creating ports
    string name;
    // rate of the human thread, expressed in seconds: e.g, 20 ms => 0.02
    double rate;

    // human thread
    EMGhumanThread *humanThread;

public:

    //---------------------------------------------------------
    EMGhuman()
    {
        count=0;
        rate=0.01;
        name="EMGhuman";
    }

    //---------------------------------------------------------
    virtual double getPeriod() { return 1.0; }

    //---------------------------------------------------------
    virtual bool updateModule()
    {
        if(count%60==0)
            cout<<" EMGhuman alive since "<<(count/60)<<" mins ... "<<endl;
        count++;
        return true;
    }

    //---------------------------------------------------------
    /* Message handler. Just echo all received messages. */
    bool respond(const Bottle& command, Bottle& reply)
    {
        ConstString cmd = command.get(0).asString();
        cout<<"first command = "<<cmd<<endl;

        if (cmd=="quit")
            return false;

        if (cmd=="list" || cmd=="help")
        {
            reply.clear();
            reply.addString("Here is the list of available commands: ");
            return true;
        }  
        else if(cmd=="stop")
        {
            
        }
        else if(cmd=="start")
        {

        } 
        else if(cmd=="status")
        {

        }
        else if(cmd=="filtering")
        {
            ConstString config = command.get(1).asString();
            cout<<"second command = "<<config<<endl;
            if(config=="off")
            {

            }
            else if(config=="on")
            {


            }
            else
            {

            }

        }
        else
        {
            reply.clear();
            reply.addString("ERROR");
            reply.addString("The first item is left/right, to choose the hand.");
            return true;
        }

 
        reply.clear();
        reply.addString("UNSURE");
        reply.addString(command.get(0).asString());
        reply.addString(command.get(1).asString());
        // DEBUG: echoes the received messages
        //reply = command;
        return true;
    }




    //---------------------------------------------------------
    void readValue(ResourceFinder &rf, string s, double &v, double vdefault)
    {
        if(rf.check(s.c_str()))
        {
            v = rf.find(s.c_str()).asDouble();
        }
        else
        {
            v = vdefault;
            cout<<"Could not find parameters for "<<s<<endl
                <<"Setting default "<<vdefault<<endl;
        }
    }

    //---------------------------------------------------------
    void readParams(ResourceFinder &rf, string s, Vector &v, int len)
    {
        v.resize(len,0.0);
        cout << "s: " << s << endl;
        if(rf.check(s.c_str()))
        {
            Bottle &grp = rf.findGroup(s.c_str());
            for (int i=0; i<len; i++)
                v[i]=grp.get(1+i).asDouble();
            DSCPAs(s,v);
        }
        else
        {
            cout<<"Could not find parameters for "<<s<<endl
                <<"Setting everything to zero by default"<<endl;
        }
    }


    //---------------------------------------------------------
    virtual bool configure(ResourceFinder &rf)
    {
        Time::turboBoost();

        if(rf.check("name"))
            name    = rf.find("name").asString();
        else
            name    = "EMGhuman";
        //....................................................
        readValue(rf,"rate",rate,0.01); //10 ms is the default rate for the thread
        

        cout<<"Parameters from init file: "<<endl;
        DSCPA(name);
        DSCPA(rate);
       

        //creating the thread for the server
        humanThread = new EMGhumanThread(rate,name);
        if(!humanThread->start())
        {
            yError("EMGhuman: cannot start the server thread. Aborting.");
            delete humanThread;
            return false;
        }
       
    
        //attach a port to the module, so we can send messages
        //and choose the type of grasp to execute
        //messages received from the port are redirected to the respond method
        rpc.open(string("/"+name+"/rpc:i").c_str());
        attach(rpc);
        yInfo("EMGhuman: RPC port attached");

        return true;
    }

   

    //---------------------------------------------------------
    virtual bool close()
    {
       
       //closing thread
       humanThread->stop();
       delete humanThread;

        cout<<"Close rpc port"<<endl;
        rpc.interrupt();
        rpc.close();
        Time::delay(0.2);

        
        return true;
    }
};



//---------------------------------------------------------
//                  MAIN
//---------------------------------------------------------
int main(int argc, char * argv[])
{
   
    ResourceFinder rf;
    rf.setDefaultContext("emg-processing");
    rf.setDefaultConfigFile("human_operator.ini");
    rf.configure(argc,argv);
  
    if (rf.check("help"))
    {
		printf("\n");
		yInfo("[EMGhuman] Options:");
        yInfo("  --context           path:   where to find the called resource (default emg-processing).");
        yInfo("  --from              from:   the name of the .ini file (default human_operator.ini).");
        yInfo("  --name              name:   the name of the module (default EMGhuman).");
        printf("\n");

        return 0;
    }
    
    Network yarp;
    if (!yarp.checkNetwork())
    {
        yError("YARP server not available!");
        return -1;
    }

    EMGhuman module;
    module.runModule(rf);

    return 0;
}
