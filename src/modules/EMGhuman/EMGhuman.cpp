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
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <emgutils.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace EmgUtils;

#define STATUS_STOPPED          0
#define STATUS_STREAMING        1
#define STATUS_CALIBRATION_MAX  2

#define CALIB_STATUS_NOT_CALIBRATED   0
#define CALIB_STATUS_CALIBRATED_ALL   1


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
        // utilizes sensor ids
        std::vector<int> sensorIds_;
        // icc pairs list, based on sensor ids
        std::vector<std::pair<int,int>> iccPairs_;
        //  id of sensor currently being calibrated for its max value
        int curCalibId_;
        //calibration time
        double calibDur_;
        // start time for calibration only
        double startTime_ = 0;
        //Global start time (for every operation)
        double startTimeGlobal_ = 0;
        // mean value counter
        int meanValCounter_ = 1;
        // stiffness
        std::vector<double> stiffness;
        // icc
        std::vector<double> icc;
        // effort
        std::vector<double> effort;


        // EMG value
        std::map<int,double> emgMap;
        // max EMG value (mapped). This is part of the calibration data.
        std::map<int,double> emgMapMax;
        // mean EMG value (mapped) for NO MOVEMENT!This is part of the calibration data.
        std::map<int,double> emgMapMean;
        std::map<int,double> emgMapMeanSum;

        //Normalized EMG values
        std::map<int,double> emgNorm;
        //mapped icc values in pairs
        std::map<std::pair<int,int>,double> iccMap;

        //log and store output flag
        bool logStoreData = true;
        std::ofstream iccLogFile;
        std::ofstream normEmgLogFile;

        // Calibration Flag
        int calibrationStatus_ = CALIB_STATUS_NOT_CALIBRATED;

        //input ports
        Bottle *inEmg;
        BufferedPort<Bottle> inPortEmg;
        //output ports
        Bottle *outNorm, *outIcc, *outStiffness, *outEffort;
        BufferedPort<Bottle> outPortNorm, outPortIcc, outPortStiffness, outPortEffort;

    public: 

    EMGhumanThread(const double _period, string _name, double calibDur,
                   std::vector<int> senId,std::vector<std::pair<int,int>> iccPairs)
        : RateThread(int(_period*1000.0)),
          calibDur_(calibDur),
          name(_name),
          sensorIds_(senId),
          iccPairs_(iccPairs)
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
        outPortNorm.open(string("/"+name+"/norm:o").c_str());

        //open log files
        if(logStoreData){
            iccLogFile.open("/home/waldez/iccLog.csv");
            normEmgLogFile.open("/home/waldez/normEmgLog.csv");
        }

        startTimeGlobal_ = Time::now();


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

        outPortNorm.interrupt();
        outPortNorm.close();

        //close log files
        if(logStoreData){
            iccLogFile.close();
            normEmgLogFile.close();
        }

        yInfo("EMGhuman: thread closing");

    }

    //------ RUN -------
    virtual void run()
    {
        // cyclic operations should be put here!
        //        curTime = Time::now();

        double timeDiffGlobal = Time::now() - startTimeGlobal_;

        if(status==STATUS_STOPPED)
        {
            // Do nothing

        }
        else {

            // read EMG values from EMG server //only the ones configured in rf
            inEmg = inPortEmg.read();

            if (inEmg!=NULL) {
                //cout << "[INFO] [FILTERED EMG] " << inEmg->toString().c_str() << endl;

                for (int i=0; i<inEmg->size(); i=i+2) {

                    int currentSenId = inEmg->get(i).asInt();

                    if(std::find(sensorIds_.begin(), sensorIds_.end(), currentSenId) != sensorIds_.end()) {
                        /* sensorIds_ contains the id we're reading from EMGServer, so we're keeping the value pointed by id */
                        emgMap[currentSenId] = inEmg->get(i+1).asDouble();
                        //cout<<"[DEBUG] : [FROM SERVER, STORED] EMG map id: "<< currentSenId<< " and value is "<< emgMap[currentSenId]<<endl;
                    } else {
                        /* sensorIds_ does not contain currentSenId */
                        //cout<<"[DEBUG] : [FROM SERVER, NOT STORED] EMG map id: "<< currentSenId<<endl;
                    }


                }
//                DSCPAstdMap(emgMap);

            } else{
                yWarning("Cant read filtered EMG data");
                return;
            }

            if(status==STATUS_STREAMING){

                //normalize EMG data
                for(const auto& emgIte:emgMap){
                    int id = emgIte.first;
                    double val = emgIte.second;

                    emgNorm[id] = (val)/(emgMapMax[id]);


                }
//                DSCPAstdMap(emgNorm);
                DSCPAstdMap(emgMap);


                // compute stiffness

                // compute ICC

                for(const auto& iccP: iccPairs_){
                    int id1 = iccP.first;
                    int id2 = iccP.second;

                    if(emgNorm[id1] <= emgNorm[id2]){
                        //add the minimum emg normalized value of the pair to the iccMap
                        iccMap[iccP] = emgNorm[id1];
                    }
                    else{
                        iccMap[iccP] = emgNorm[id2];
                    }
                }

            // send output to ports

            //send icc
                Bottle& b = outPortIcc.prepare();

                b.clear();

                iccLogFile << timeDiffGlobal << ", ";
                for(const auto& iccPair:iccMap){

                    //send icc pairs to yarp ports
                    //(SENSOR_INDEX_1, SENSOR_INDEX_2, ICC)
                    b.addInt(iccPair.first.first);
                    b.addInt(iccPair.first.second);
                    b.addDouble(iccPair.second);

                    if(logStoreData){

                        iccLogFile <<iccPair.first.first<<", "
                                   << iccPair.first.second<<", "
                                   << iccPair.second<<", ";

                    }
                }
                iccLogFile << std::endl;

                outPortIcc.write();

            //send normalized values
                Bottle& b2 = outPortNorm.prepare();

                b2.clear();

                normEmgLogFile << timeDiffGlobal<<", ";
                for(const auto& normIte: emgNorm){
                    //(SENSOR_INDEX_1, NORM_EMG)
                    b.addInt(normIte.first);
                    b.addDouble(normIte.second);

                    if(logStoreData){

                        normEmgLogFile << normIte.first <<", "<< normIte.second<<", ";

                    }
                }
                normEmgLogFile << std::endl;

                outPortNorm.write();


            } else if(status==STATUS_CALIBRATION_MAX){
                // do the necessary things for the calibration

                if(startTime_ == 0) startTime_ = Time::now();

                double timeDiff =(Time::now() - startTime_);
                //                    std::cout << "[INFO] : " << timeDiff << std::endl;

                if(timeDiff <= calibDur_){

                    //do something for the calibration


                    //verify if the sensor intended for calibration is available
                    if(!emgMap.count(curCalibId_)){
                        //if not, abort the calibration
                        cout << "[WARNING] Could not find sensor "<<curCalibId_<<" for calibration. Aborting calibration "
                             << emgMap.count(curCalibId_) <<endl;
                        stopCalibrationMax();
                        startTime_ = 0;
                        return;

                    }

                        //Selects Max Value

                        //tries to find a max value for a certain id
                        if(!emgMapMax.count(curCalibId_)){
                            //if no max value was found, add it to the map
                            emgMapMax[curCalibId_] = emgMap[curCalibId_];

                        }
                        //in case the value found is smaller than the current...
                        else if(emgMap[curCalibId_] >= emgMapMax[curCalibId_]){

                            //we add the current as the max!
                            emgMapMax[curCalibId_] = emgMap[curCalibId_];

                        }

                } else{

                    bool ok = true;
                    for(const auto& senIte:sensorIds_){
                        ok &= emgMapMax.count(senIte);
                        cout <<ok<<" "<< senIte <<endl;

                    }

                    //CHANGE CALIBRATION STATUS only if we have calibrated all of the sensors
                    if(ok)  calibrationStatus_ = CALIB_STATUS_CALIBRATED_ALL;

                    startTime_ = 0;
                    stopCalibrationMax();
                    cout<<"[INFO] Max calibration "<<std::endl;
                    DSCPAstdMap(emgMapMax);
                }


            }
        }

    }
    //end run

    void startStreaming()
    {
        if(calibrationStatus_ != CALIB_STATUS_CALIBRATED_ALL) {
            yError("EMGhuman: Can't stream. Data is not calibrated yet");
            return;
        }
        status=STATUS_STREAMING;
        return;
    }

    void stopStreaming()
    {
        status=STATUS_STOPPED;
    }

    void startCalibrationMax()
    {
        // save current state as we want to go back to this state after the calibration
        prevStatus=status;
        status=STATUS_CALIBRATION_MAX;
    }

    void stopCalibrationMax()
    {
        // go back to the status that was before the calibration (stopped or streaming)
        status=prevStatus;
    }

    void setCalibSenId(int calibSenId){
        curCalibId_ = calibSenId;
    }

    int getStatus()
    {
        return status;
    }

    int getCalibrationStatus(){
        return calibrationStatus_;
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
    //bool calibration;
    // calibration duration
    double calibration_duration;
    // type of calibration 2=arm2 / 4=arm4 / 8=arm8
    //int calibration_type;
    // calibration default values
    //std::vector<double> calibration_emg_max, calibration_emg_min;
    // use filtered data 0=no, 1=rmse, 2=rmse+filtered
    //int use_filtered_data;
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
        calibration_duration=5.0;
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
            reply.addString("calibration_status");
            reply.addString("calibrate_max #SENSOR_ID");
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
            else if(curStatus==STATUS_CALIBRATION_MAX) reply.addString(" Status = CALIBRATION OF MAX VALUE ONLY");
            else if(curStatus==STATUS_STREAMING) reply.addString(" Status = STREAMING");
            else reply.addString(" Status = IMPOSSIBLE");
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;

        }
        else if(cmd=="calibration_status")
        {
            int curCalibStatus=humanThread->getCalibrationStatus();

            reply.clear();
            if(curCalibStatus==CALIB_STATUS_NOT_CALIBRATED) reply.addString(" Calibration Status = NOT CALIBRATED");
            else reply.addString(" Calibration Status = CALIBRATED");
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;

        }

        else if(cmd=="calibrate_max")
        {
            if(command.size() <= 1){
                reply.clear();
                reply.addString("No sensor id input. Please use \"calibrate_max SENSOR_ID_VALUE \" ");
            } else if(humanThread->getStatus() == STATUS_CALIBRATION_MAX){

                reply.clear();
                reply.addString("There is a sensor already being calibrated, please wait.");

            } else{
                int senId = command.get(1).asInt();
                    cout<<"[INFO] second command: " << senId<<endl;
                humanThread->setCalibSenId(senId);
                humanThread->startCalibrationMax();
                reply.clear();
                reply.addString("OK");
            }
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
    virtual bool configure(ResourceFinder &rf)
    {
        Time::turboBoost();

        if(rf.check("name"))
            name    = rf.find("name").asString();
        else
            name    = "EMGhuman";
        //....................................................

        std::vector<std::pair<int,int>> iccPairs;

        readValue(rf,"rate",rate,0.01); //10 ms is the default rate for the thread
        readValue(rf,"calibration_duration",calibration_duration,0.01);
        readParams(rf,"sensorIds",sensorIds);
        readParams(rf,"iccPairs",iccPairs);
        

        cout<<"Parameters from init file: "<<endl;
        DSCPA(name);
        DSCPA(rate);
        DSCPAstdvec(sensorIds);
        DSCPAstdvecpair(iccPairs);

        //creating the thread for the server
        humanThread = new EMGhumanThread(rate,name,calibration_duration, sensorIds, iccPairs);
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

    cout << endl << "back to main";

    return 0;
}
