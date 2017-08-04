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
#include <deque>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

// utils for printing parameters
#define DSCPA(V) cout<<"  "<< #V <<" : "<<V<<endl;
#define DSCPAv(V) cout<<"  "<< #V <<" : "<<V.toString()<<endl;
#define DSCPAs(S,V) cout<<"  "<< S <<" : "<<V.toString()<<endl;
#define DSCPAd(S,V) cout<<"  "<< S <<" : "<<V<<endl;

#define STATUS_STOPPED      0
#define STATUS_STREAMING    1
#define STATUS_CALIBRATION  2


//===============================
//        EMGhuman THREAD
//===============================

class EMGhumanThread: public RateThread
{
    protected:
        // status information
        int status, prevStatus;
        // name used for the ports
        string name;
        // current time
        double curTime;
        // stiffness
        Vector stiffness;
        // icc
        Vector icc;
        // effort
        Vector effort;
        // max EMG value
        Vector emg_max, calibration_emg_max;
        // min EMG value
        Vector emg_min, calibration_emg_min;
        // mean EMG value
        Vector emg_mean, calibration_emg_mean;
        // flag is Calibrated 0=no 1=with default values 2=after calibration procedure
        int isCalibrated;

        //input ports
        Bottle inEmg;
        BufferedPort<Bottle> inPortEmg;
        //output ports
        Bottle outEmg, outIcc, outStiffness, outEffort;
        BufferedPort<Bottle> outPortEmg, outPortIcc, outPortStiffness, outPortEffort;

    public: 

    EMGhumanThread(const double _period, string _name): RateThread(int(_period*1000.0))
    {
        name = _name;
        status = STATUS_STOPPED;
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

        if(status==STATUS_STOPPED)
        {
            // probably do nothing

        }
        else
        {
            
            // read EMG values from EMG server

            // compute stiffness
        
            // compute ICC

            // compute effort 

            // send output to ports


            if(status==STATUS_CALIBRATION)
            {
            // do the necessary things for the calibration

            }



        }

    }
    //end run

    void startStreaming()
    {
        if(isCalibrated<1) yWarning("EMGhuman: streaming but not calibrated yet!");
        status=STATUS_STREAMING;
    }

    void stopStreaming()
    {
        status=STATUS_STOPPED;
    }

    void startCalibration()
    {
        // save current state as we want to go back to this state after the calibration
        prevStatus=status;
        status=STATUS_CALIBRATION;
    }

    void stopCalibration()
    {
        // go back to the status that was before the calibration (stopped or streaming)
        status=prevStatus;
    }

    int getStatus()
    {
        return status;
    }

    bool setCalibrationValues(Vector _emg_calib_max, Vector _emg_calib_min)
    {
        if((calibration_emg_max.size()!=_emg_calib_max.size())||(calibration_emg_min.size()!=_emg_calib_min.size()))
        {
            yWarning("EMGhuman: different size for the calibration parameters that are manually provided. Ignoring calibration.");
            return false;
        }
        calibration_emg_max = _emg_calib_max;
        calibration_emg_min = _emg_calib_min;

        return true;
    }

    bool computeCalibration()
    {
        //TODO
        //procedure for calibrating the EMG with the MAX V C

        return true;
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
    // calibration 
    bool calibration;
    // calibration duration
    double calibration_duration;
    // type of calibration 2=arm2 / 4=arm4 / 8=arm8
    int calibration_type;
    // calibration default values
    Vector calibration_emg_max, calibration_emg_min;
    // use filtered data 0=no, 1=rmse, 2=rmse+filtered
    int use_filtered_data;
    // auto-connect to the ports (VERY RISKY)
    bool autoconnect;
    // numbers of sensors that we will use here
    int numberOfSensors;
    // sensors ids
    Vector sensorIds;
    // sensor names
    deque<string> sensorNames;

    // human thread
    EMGhumanThread *humanThread;

public:

    //---------------------------------------------------------
    EMGhuman()
    {
        count=0;
        rate=0.01;
        name="EMGhuman";
        calibration_emg_max.resize(8,0.0); 
        calibration_emg_min.resize(8,0.0);
        sensorIds.resize(8);
        calibration=false;
        calibration_duration=10.0;
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
            reply.addString("Here is the list of available commands:");
            reply.addString("stop");
            reply.addString("start");
            reply.addString("status");
            reply.addString("calibration_from_file");
            reply.addString("calibration_online");
            return true;
        }  
        else if(cmd=="stop")
        {
            humanThread->stopStreaming();
            reply.clear(); reply.addString("OK");
            return true;
        }
        else if(cmd=="start")
        {
            humanThread->startStreaming();
            reply.clear(); reply.addString("OK");
            return true;
        } 
        else if(cmd=="status")
        {
            int curStatus=humanThread->getStatus();

            reply.clear();
            if(curStatus==STATUS_STOPPED) reply.addString(" Status = STOPPED");
            else if(curStatus==STATUS_CALIBRATION) reply.addString(" Status = CALIBRATION");
            else if(curStatus==STATUS_STREAMING) reply.addString(" Status = STREAMING");
            else reply.addString(" Status = IMPOSSIBLE");

            return true;

        }
        else if(cmd=="calibration_from_file")
        {
            bool ret=humanThread->setCalibrationValues(calibration_emg_max,calibration_emg_min);
            if(ret==false)
            {
                reply.clear();
                reply.addString("Calibration not done - mismatch in size of max/min values");
            }
            else
            {
                humanThread->computeCalibration();  
                reply.clear();
                reply.addString("Calibration done");
            }
            return true;
            
        }
        else if(cmd=="calibration_online")
        {
            humanThread->startCalibration();
            reply.clear();
            reply.addString("OK");
            return true;
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
       
    
        //attach a rpc port to the module
        //messages received from the port are redirected to the respond method
        // to connect to the module do:
        // yarp rpc /EMGhuman/rpc 
        // yarp rpc /human_operator/rpc
        rpc.open(string("/"+name+"/rpc").c_str());
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
