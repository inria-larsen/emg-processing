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
//#include <cmath>
#include <map>
//#include "mainwindow.h"
//#include <QApplication>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

// utils for printing parameters
#define DSCPA(V) cout<<"  "<< #V <<" : "<<V<<endl;
#define DSCPAv(V) cout<<"  "<< #V <<" : "<<V.toString()<<endl;
#define DSCPAs(S,V) cout<<"  "<< S <<" : "<<V.toString()<<endl;
#define DSCPAd(S,V) cout<<"  "<< S <<" : "<<V<<endl;
#define DSCPAstdvec(V)  std::cout << "  " << #V << " :"; for(auto vi:V) {std::cout << " " << vi; } std::cout << std::endl;

#define STATUS_STOPPED      0
#define STATUS_STREAMING    1
#define STATUS_CALIBRATION  2

//typedef std::pair<int,double> EmgId;
//typedef  std::map<int,double> EmgMap;


//===============================
//        EMGhuman THREAD
//===============================

class EMGhumanThread: public RateThread
{
    protected:
        // status information
        int status, prevStatus;
        // name used for the ports
        std::string name;
        std::vector<int> sensorIds_;
        //calibration time
        double calibDur_;
        // start time
        double startTime_ = 0;
        // stiffness
        std::vector<double> stiffness;
        // icc
        std::vector<double> icc;
        // effort
        std::vector<double> effort;
        // EMG value
        std::map<int,double> emgMap;
        // max EMG value (mapped)
        std::map<int,double> emgMapMax;
        // max EMG value
        std::vector<double> emg_max, calibration_emg_max;
        // min EMG value
        std::vector<double> emg_min, calibration_emg_min;
        // mean EMG value
        std::vector<double> emg_mean, calibration_emg_mean;
        // flag is Calibrated 0=no 1=with default values 2=after calibration procedure
        int isCalibrated;

        //input ports
        Bottle *inEmg;
        BufferedPort<Bottle> inPortEmg;
        //output ports
        Bottle *outEmg, *outIcc, *outStiffness, *outEffort;
        BufferedPort<Bottle> outPortEmg, outPortIcc, outPortStiffness, outPortEffort;

    public: 

    EMGhumanThread(const double _period, string _name, double calibDur, std::vector<int> senId)
        : RateThread(int(_period*1000.0)),
          calibDur_(calibDur),
          name(_name),
          sensorIds_(senId)
    {
        status = STATUS_STOPPED;
//        status = STATUS_STREAMING;
        yInfo("EMGhuman: thread created");

    }

    virtual bool threadInit()
    {
       
        // opening ports
        inPortEmg.open(string("/"+name+"/emg:i").c_str());
        outPortStiffness.open(string("/"+name+"/stiffness_arm:o").c_str());
        outPortIcc.open(string("/"+name+"/icc:o").c_str());

        return true;

    }

    virtual void threadRelease()
    {
        //closing all the ports

        inPortEmg.interrupt();
        inPortEmg.close();

        outPortStiffness.interrupt();
        outPortStiffness.close();

        outPortIcc.interrupt();
        outPortIcc.close();
    

        yInfo("EMGhuman: thread closing");

    }

    //------ RUN -------
    virtual void run()
    {
        // cyclic operations should be put here!
//        curTime = Time::now();

        if(status==STATUS_STOPPED)
        {
            // probably do nothing

        }
        else {

                // read EMG values from EMG server
                inEmg = inPortEmg.read();

                if (inEmg!=NULL) {
                    cout << "[INFO] [FILTERED EMG] " << inEmg->toString().c_str() << endl;

                    for (int i=0; i<inEmg->size(); i=i+2) {

                        emgMap[inEmg->get(i).asInt()] = inEmg->get(i+1).asDouble();
//                        cout<<"[DEBUG] : EMG map id: "<< inEmg->get(i).asInt()<< " and value is "<< emgMap[inEmg->get(i).asInt()];

                    }


                }

                if(status==STATUS_STREAMING){


                    // compute stiffness

                    // compute ICC

                    // compute effort

                    // send output to ports
                }

                else if(status==STATUS_CALIBRATION){
                // do the necessary things for the calibration

                    if(startTime_ == 0) startTime_ = Time::now();

                    double timeDiff =(Time::now() - startTime_);
//                    std::cout << "[INFO] : " << timeDiff << std::endl;

                    if(timeDiff <= calibDur_){

                        //do something for the calibration
                        //
                        //
                        //...

//                        for(auto& emgIte : emgMap){
//                           std::cout<<"[INFO] : read the sensor "<<emgIte.first<<" with the value: "<<emgIte.second<<std::endl;
                            //check if max value for the id exists
    //                            if not, add value to emgMapMax with the id id
//                        }
                    }
                    else{
                        startTime_ = 0;
                        stopCalibration();
                    }


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

    bool setCalibrationValues(std::vector<double> _emg_calib_max, std::vector<double> _emg_calib_min)
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
    std::string name;
    // rate of the human thread, expressed in seconds: e.g, 20 ms => 0.02
    double rate;
    // calibration 
    bool calibration;
    // calibration duration
    double calibration_duration;
    // type of calibration 2=arm2 / 4=arm4 / 8=arm8
    int calibration_type;
    // calibration default values
    std::vector<double> calibration_emg_max, calibration_emg_min;
    // use filtered data 0=no, 1=rmse, 2=rmse+filtered
    int use_filtered_data;
    // auto-connect to the ports (VERY RISKY)
    bool autoconnect;
    // numbers of sensors that we will use here
    int numberOfSensors;
    // sensors ids
    std::vector<int> sensorIds;
    // sensor names
    deque<std::string> sensorNames;

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

        if (cmd=="quit"){
            return false;
                        cout<<"[INFO] " << reply.toString()<<endl;
        }

        if (cmd=="list" || cmd=="help")
        {
            reply.clear();
            reply.addString("Here is the list of available commands:");
            reply.addString("stop");
            reply.addString("start");
            reply.addString("status");
            reply.addString("calibration_from_file");
            reply.addString("calibration_online");
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;
        }  
        else if(cmd=="stop")
        {
            humanThread->stopStreaming();
            reply.clear(); reply.addString("OK");
            //cout<<" test";
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;
        }
        else if(cmd=="start")
        {
            humanThread->startStreaming();
            reply.clear(); reply.addString("OK");
                        cout<<"[INFO] " << reply.toString()<<endl;
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
                        cout<<"[INFO] " << reply.toString()<<endl;
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
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;
            
        }
        else if(cmd=="calibration_online")
        {
            humanThread->startCalibration();
            reply.clear();
            reply.addString("OK");
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;
        }
        else
        {
            reply.clear();
            reply.addString("ERROR");
            reply.addString("The first item is left/right, to choose the hand.");
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;
        }

 
        reply.clear();
        reply.addString("UNSURE");
        reply.addString(command.get(0).asString());
        reply.addString(command.get(1).asString());
                    cout<<"[INFO] " << reply.toString()<<endl;

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
    void readParams(ResourceFinder &rf, string s, std::vector<int> &v)
    {
        if(rf.check(s.c_str()))
        {
            Bottle &grp = rf.findGroup(s.c_str());
            for (int i=0; !grp.get(1+i).isNull(); i++)
                v.push_back( grp.get(1+i).asInt() );
        }
        else
        {
            cout<<"Could not find parameters for "<<s<<endl
                <<"Setting everything to zero by default"<<endl;
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
        readValue(rf,"calibration_duration",calibration_duration,0.01);
        readParams(rf,"sensorIds",sensorIds);
        

        cout<<"Parameters from init file: "<<endl;
        DSCPA(name);
        DSCPA(rate);
        DSCPAstdvec(sensorIds);

        //creating the thread for the server
        humanThread = new EMGhumanThread(rate,name,calibration_duration, sensorIds);
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
//    QApplication a(argc,argv);
//    MainWindow w;
//    w.show();
//    a.exec();

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

    cout << endl << "back to main";



    return 0;
}
