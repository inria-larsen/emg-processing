/*
 * EMG server 
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
#include <EmgTcp/EmgTcp.h>
#include <EmgSignal/EmgSignal.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

// utils for printing parameters
#define DSCPA(V) cout<<"  "<< #V <<" : "<<V<<endl;
#define DSCPAv(V) cout<<"  "<< #V <<" : "<<V.toString()<<endl;
#define DSCPAs(S,V) cout<<"  "<< S <<" : "<<V.toString()<<endl;
#define DSCPAd(S,V) cout<<"  "<< S <<" : "<<V<<endl;

//==================================================
//        GLOBAL VARIABLES (synched between threads)
//==================================================
std::vector<float> rawData(16,0.0);
std::vector<float> filteredData(16,0.0);
Mutex m;

//======================================================================
//        EMGserver Sensor THREAD (gets data as soon as it is available)
//======================================================================
class SensorThread : public Thread {
    protected:
        EmgTcp *emgConPtr_;
        EmgSignal emgSig; 
    public:
        SensorThread( EmgTcp *emgCon){
            emgConPtr_ = emgCon;
        }
        virtual bool threadInit()
        {
            printf("Starting SensorThread\n");

/*                   bool isConnected = emgConPtr_->connect2Server();

                    if(isConnected==false)
                    {
                        yError("EMGserver: cannot connect to TCP server of Delsys. Aborting module - please check Delsys before restarting.");
                        return false;
                    }*/
            
            emgConPtr_->configServer();
            std::cout<<std::endl<<"about to start data stream"<<std::endl;

    //======COMMENT THE START DATA STREAM
            emgConPtr_->startDataStream(); 


            return true;
        }
        virtual void run() {
            while (!isStopping()) {
                // printf("Hello, from thread1\n");
                // Time::delay(1);
                EmgData sample = emgConPtr_->getData();
                //lock global variables
                m.lock();
                    //set global vars
                    rawData = sample.data;
                m.unlock();
                
            }
        }
        virtual void threadRelease()
        {
            printf("Goodbye from SensorThread\n");
            emgConPtr_->stopDataStream(); 
        }
};


//===============================================
//        EMGserver Main THREAD (runs every 10ms)
//===============================================

class EMGserverThread: public RateThread
{
    protected:

        // name used for the ports
        string name;
        // current time
        double curTime;

        EmgTcp emgCon;
        SensorThread *sensorTh;

    public: 

    EMGserverThread(const double _period, string _name, string ipadd): RateThread(int(_period*1000.0))
    {
        name = _name;
        yInfo("EMGserver: thread created");
        emgCon.setIpAdd(ipadd);

    }

    virtual bool threadInit()
    {
        bool isConnected = false;

        // establishing connection with the TCP server
        
        // EmgTcp emgCon("169.254.1.165");
        isConnected = emgCon.connect2Server();



        if(isConnected==false)
        {
            yError("EMGserver: cannot connect to TCP server of Delsys. Aborting module - please check Delsys before restarting.");
            return false;
        }


        //creating the thread for the sensors
        sensorTh = new SensorThread(&emgCon);
        if(!sensorTh->start())
        {
            yError("EMGserver: cannot start the sensor thread. Aborting.");
            delete sensorTh;
            return false;
        }

        // opening ports

        return true;

    }

    virtual void threadRelease()
    {
        //close sensor thread
        sensorTh->stop();
        delete sensorTh;
        //closing all the ports

        // closing connection with TCP server of Delsys if needed

        yInfo("EMGserver: thread closing");

    }

    //------ RUN -------
    virtual void run()
    {
        // cyclic operations should be put here!
        curTime = Time::now();


        // read raw sensors from EMG
            //lock mutex
            m.lock();
            std::cout <<std::endl<<"printing every 10 ms..." << rawData[0];
            m.unlock(); 

        // compute filtered signal

        // send output to raw port

        // send output to filtered port

    }


};

//===============================
//        EMGserver MODULE
//===============================
class EMGserver: public RFModule
{
private:
    // counter for the number of minutes of execution
    int count;
    // the port to handle messages
    Port rpc; 
    // name of the module, used for creating ports
    string name;
    // rate of the server  hread, expressed in seconds: e.g, 20 ms => 0.02
    double rate;

    string ipAdd_;

    // server thread
    EMGserverThread *serverThread;

public:

    //---------------------------------------------------------
    EMGserver()
    {
        count=0;
        rate = 0.01;
    }

    //---------------------------------------------------------
    double getPeriod() { return 1.0; }

    //---------------------------------------------------------
    bool updateModule()
    {
        if(count%60==0)
            cout<<" EMGserver alive since "<<(count/60)<<" mins ... "<<endl;
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

    void readValue(ResourceFinder &rf, string s, string &v, string vdefault)
    {
        if(rf.check(s.c_str()))
        {
            v = rf.find(s.c_str()).asString();
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
    bool configure(ResourceFinder &rf)
    {
        Time::turboBoost();

        if(rf.check("name"))
            name    = rf.find("name").asString();
        else
            name    = "EMGserver";
        //....................................................
        readValue(rf,"rate",rate,0.01); //10 ms is the default rate for the thread
                                        // ipAdd_
                                                
        readValue(rf,"ip_add",ipAdd_,"169.254.1.165");

        cout<<"Parameters from init file: "<<endl;
        DSCPA(name);
        DSCPA(rate);
        DSCPA(ipAdd_);
       

        //creating the thread for the server
        serverThread = new EMGserverThread(rate,name,ipAdd_);
        if(!serverThread->start())
        {
            yError("EMGserver: cannot start the server thread. Aborting.");
            delete serverThread;
            return false;
        }
       
    
        //attach a port to the module, so we can send messages
        //and choose the type of grasp to execute
        //messages received from the port are redirected to the respond method
        rpc.open(string("/"+name+"/rpc:i").c_str());
        attach(rpc);
        yInfo("EMGserver: RPC port attached");

        return true;
    }

    //---------------------------------------------------------
    bool close()
    {
        yInfo("EMGserver: closing module");
        serverThread->stop();
        delete serverThread;

        yInfo("EMGserver: closing RPC port");
        rpc.interrupt();
        rpc.close();
                
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
    rf.setDefaultConfigFile("emg_delsys.ini");
    rf.configure(argc,argv);
  
    if (rf.check("help"))
    {
		printf("\n");
		yInfo("[EMGserver] Options:");
        yInfo("  --context           path:   where to find the called resource (default emg-processing).");
        yInfo("  --from              from:   the name of the .ini file (default emg_delsys.ini).");
        yInfo("  --name              name:   the name of the module (default EMGserver).");
        printf("\n");

        return 0;
    }
    
    Network yarp;
    if (!yarp.checkNetwork())
    {
        yError("YARP server not available!");
        return -1;
    }

    EMGserver module;
    module.runModule(rf);

    return 0;
}
