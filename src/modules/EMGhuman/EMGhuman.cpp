/*
 * EMG human 
 * 
 * Author: Serena Ivaldi, Waldez Gomes
 * email:  serena.ivaldi@inria.fr, waldezjr14@gmail.com
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
#include <dirent.h>
// #include <yarp/rosmsg/std_msgs/String.h>
// #include <yarp/rosmsg/std_msgs/Float64MultiArray.h>

// tcp includes to comm with ros router script
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace EmgUtils;

// using yarp::os::Node;
// using yarp::os::Publisher;

#define STATUS_STOPPED          0
#define STATUS_STREAMING        1
#define STATUS_STREAMING_ROS    2
#define STATUS_CALIBRATION_MAX  3

#define CALIB_STATUS_NOT_CALIBRATED   0
#define CALIB_STATUS_CALIBRATED_ALL   1

namespace {
YARP_LOG_COMPONENT(TALKER, "yarp.example.ros.talker")
constexpr double loop_delay = 0.1;
}


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
        // subject's id
        int subjectId_;
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
        std::ofstream calibEmgLogFile;

        // Calibration Flag
        int calibrationStatus_ = CALIB_STATUS_NOT_CALIBRATED;

        //input ports
        Bottle *inEmg;
        BufferedPort<Bottle> inPortEmg;
        //output ports
        Bottle *outNorm, *outIcc, *outStiffness, *outEffort;
        BufferedPort<Bottle> outPortNorm, outPortIcc, outPortStiffness, outPortEffort;

        // ROS
        // std::shared_ptr<Node> node_;
        // yarp::os::Publisher<yarp::rosmsg::std_msgs::Float64MultiArray> rosPub_;
        // yarp::rosmsg::std_msgs::Float64MultiArray iccRosMsg_;

        // tcp connection to ROS router script ---> yarp/ros interface is not working for remote computers--> the topic is sent but data is not received
        struct sockaddr_in addr_;
        std::string rosAdd_ = "jarvis";
        int rosPort_ = 10501; //high number not to conflict with yarp local ports
        int rosSock_;
        int rosStreaming_ = 0;      


    public: 

    EMGhumanThread(const double _period, string _name, double calibDur,
                   std::vector<int> senId,std::vector<std::pair<int,int>> iccPairs, int subId, int rosStreaming)
        : RateThread(int(_period*1000.0)),
          calibDur_(calibDur),
          name(_name),
          sensorIds_(senId),
          iccPairs_(iccPairs),
          subjectId_(subId),
          rosStreaming_(rosStreaming)
    {
        status = STATUS_STOPPED;
//        status = STATUS_STREAMING;

        if(rosStreaming_){
            // connect to ROS router script
                //Initialize addr_ struct
                const char* cServIpAdd = const_cast<char*>(rosAdd_.c_str());
                memset (&addr_, 0, sizeof(addr_));

                addr_.sin_family = AF_INET;

                // if (resolveHostName(cServIpAdd, &(addr_.sin_addr)) != 0 ) {
                    inet_pton(PF_INET, cServIpAdd, &(addr_.sin_addr));        
                // }
                // else{
                //     yFatal("Could not find host address");
                // }

                // connect to port
                //assign correct port
                addr_.sin_port = htons(rosPort_);

                //create a socket
                rosSock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (rosSock_ < 0) {
                    yError("Problem connecting to server [socket]");
                    //throw std::runtime_error("Problem connecting to server [socket]");
                    yFatal("Could not connect to ros router script server socket");
                }

                //try to connect
                if (connect(rosSock_, (struct sockaddr*)&addr_, sizeof(addr_)) != 0)
                {
                    
                    if(close(rosSock_) != 0){
                        yFatal("Failed to close ros router script socket");
                    }
                    // throw std::runtime_error("Problem connecting to server");
                    yFatal("Problem connecting to ros router script port");
                }
        }
        yInfo("EMGhuman: thread created");

    }

    ~EMGhumanThread(){

        if(close(rosSock_) != 0){
			yFatal("Failed to close ros router script socket");
		}
    }

    int resolveHostName(const char* hostname, struct in_addr* addr) 
	{
	    struct addrinfo *res;
	  
	    int result = getaddrinfo (hostname, NULL, NULL, &res);
	    if (result == 0)
	    {
	        memcpy(addr, &((struct sockaddr_in *) res->ai_addr)->sin_addr, sizeof(struct in_addr));
	        freeaddrinfo(res);
    	}

    	return result;
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
            iccLogFile.open(string("EMGLog/DyadExperiment/"+to_string(subjectId_)+"_"+to_string((int)Time::now())+"_"+name+"_IccLog.csv").c_str());
            normEmgLogFile.open(string("EMGLog/DyadExperiment/"+to_string(subjectId_)+"_"+to_string((int)Time::now())+"_"+name+"_NormEmgLog.csv").c_str());
        }

        startTimeGlobal_ = Time::now();

        // node_ = std::make_shared<Node>("/yarp/");

        /* subscribe to topic chatter */
        //TODO: WHAT HAPPENS IF ROS_MASTER IS NOT AVAILABLE?
        // if (!rosPub_.topic("/index_co_contraction")) {
        //     yCError(TALKER) << "Failed to create publisher to /index_co_contraction";
        //     // return -1;
        // }


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

    void saveCalibration(){
        if(calibrationStatus_ == CALIB_STATUS_CALIBRATED_ALL){
            DIR* dir = opendir("EMGLog/Calibration/");
            if (dir)
            {
                /* Directory exists. */
                closedir(dir);
            }
            else if (ENOENT == errno)
            {
                /* Directory does not exist. */
                //Create Directory
                int ret;
                string s("mkdir -p EMGLog/Calibration/");
                ret = system(s.c_str());
                std::cout<<"[INFO] Created 'EMGLog/Calibration' directory"<<std::endl;
            }
            else
            {
                /* opendir() failed for some other reason. */
                std::cout<<"[ERROR] Could not open directory to save log file"<<std::endl;
                return;
            }

            calibEmgLogFile.open(string("EMGLog/Calibration/"+std::to_string(subjectId_)+"_"+to_string((int)Time::now())+"_"+name+"_CalibEmgLog.csv").c_str());

            //save calibration
            for(const auto& vi:emgMapMax) {
                calibEmgLogFile << vi.first << ", "<<vi.second<< ", ";
            }
            calibEmgLogFile << std::endl;

            calibEmgLogFile.close();
            std::cout<<"[INFO] calibration saved"<<std::endl;
        }
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
            // yInfo("stopped");
            // //THIS IS JUST TO DEBUG !!! REMOVE RIGHT AFTER!!!1
            // if(rosStreaming_){
            //     std::string dataStr("");
            //     dataStr.append(std::string("1.0, 2.3, "));
            //     dataStr.append("\r");
            //     ssize_t n = send(rosSock_, dataStr.c_str(), strlen(dataStr.c_str()), 0); 
            //     if (n < 0)	{
            //         //throw std::runtime_error("Problem sending command to server");
            //         yError("Problem sending data to ros router script!");
            //     }

            // }

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

            if(status==STATUS_STREAMING || status == STATUS_STREAMING_ROS){

                //normalize EMG data
                for(const auto& emgIte:emgMap){
                    int id = emgIte.first;
                    double val = emgIte.second;

                    emgNorm[id] = (val)/(emgMapMax[id]);

                }
//                DSCPAstdMap(emgNorm);
//                DSCPAstdMap(emgMap);

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
                if(status == STATUS_STREAMING){
                    Bottle& b = outPortIcc.prepare();

                    b.clear();

                    iccLogFile << timeDiffGlobal << ", ";
                    iccLogFile << Time::now() << ", ";
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
                        b2.addInt(normIte.first);
                        b2.addDouble(normIte.second);

                        if(logStoreData){

                            normEmgLogFile << normIte.first <<", "<< normIte.second<<", ";

                        }
                    }
                    normEmgLogFile << std::endl;

                    outPortNorm.write();

                }
                else if(status == STATUS_STREAMING_ROS){
                    yInfo("Trying to send something");
                    // send ICC to ROS topics from here
                    
                    // iccRosMsg_.data.resize(iccMap.size());
                    // int i=0;
                    // for(const auto& iccPair: iccMap){
                    //     iccRosMsg_.data[i] = iccPair.second;
                    //     i++;
                    // }
                    
                    // iccRosMsg_.data.resize(1);
                    // iccRosMsg_.data[0] = 1.0;
                    // rosPub_.write(iccRosMsg_);
                    std::string dataStr("");
                    for(const auto& iccPair: iccMap){
                        dataStr.append(std::to_string(iccPair.second)+std::string(", "));
                    }
                    // dataStr.append(std::string("1.0, 2.3, "));
                    dataStr.append("\r");
                    ssize_t n = send(rosSock_, dataStr.c_str(), strlen(dataStr.c_str()), 0); 
                    if (n < 0)	{
                        //throw std::runtime_error("Problem sending command to server");
                        yError("Problem sending data to ros router script!");
                    }
                }


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
                    if(ok) {

                        calibrationStatus_ = CALIB_STATUS_CALIBRATED_ALL;

                    }

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

    void startStreamingRos(){
        
        // if(calibrationStatus_ != CALIB_STATUS_CALIBRATED_ALL) {
        //     yError("EMGhuman: Can't stream. Data is not calibrated yet");
        //     return;
        // }
        // else if(status == STATUS_STREAMING){
        //     yError("EMGhuman: Stop Yarp streaming before starting ROS streaming.");
        //     return;
        // }
        // else if(status == STATUS_CALIBRATION_MAX){
        //     yError("EMGhuman: Calibration is in execution, module can not stream ICC.");
        // }
        cout << "change status to streaming ros";
        status = STATUS_STREAMING_ROS;
        return;
    }
    void stopStreamingRos(){
        status = STATUS_STOPPED;
        return;
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
    int count_;
    // the port to handle messages
    Port rpc; 
    // name of the module, used for creating ports
    std::string name_;
    //subject's ID
    int subjectId_;
    // rate of the human thread, expressed in seconds: e.g, 20 ms => 0.02
    double rate_;
    // calibration 
    //bool calibration;
    // calibration duration
    double calibration_duration_;
    // type of calibration 2=arm2 / 4=arm4 / 8=arm8
    //int calibration_type;
    // calibration default values
    //std::vector<double> calibration_emg_max, calibration_emg_min;
    // use filtered data 0=no, 1=rmse, 2=rmse+filtered
    //int use_filtered_data;
    // auto-connect to the ports (VERY RISKY)
    bool autoconnect_;
    // sensors ids
    std::vector<int> sensorIds_;
    // sensor names
    deque<std::string> sensorNames_;

    // human thread
    EMGhumanThread *humanThread_;

    int rosStreaming_ = 0;

public:

    //---------------------------------------------------------
    EMGhuman()
    {
        count_=0;
        rate_=0.01;
        name_="EMGhuman";
        calibration_duration_=5.0;
    }

    //---------------------------------------------------------
    virtual double getPeriod() { return 1.0; }

    //---------------------------------------------------------
    virtual bool updateModule()
    {
        if(count_%60==0)
            cout<<" EMGhuman alive since "<<(count_/60)<<" mins ... "<<endl;
        count_++;
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
            reply.addString("stop_ros");
            reply.addString("start");
            reply.addString("start_ros");
            reply.addString("status");
            reply.addString("calibration_status");
            reply.addString("calibrate_max #SENSOR_ID");
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;
        }  
        else if(cmd=="stop")
        {
            //TODO: the reply should be based on the return from the streaming fun.
            humanThread_->stopStreaming();
            reply.clear(); reply.addString("OK");
            //cout<<" test";
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;
        }
        else if(cmd=="start")
        {
            humanThread_->startStreaming();
            reply.clear(); reply.addString("OK");
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;
        } 
        else if(cmd=="start_ros"){
            humanThread_->startStreamingRos();
            reply.clear(); reply.addString("OK");
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;
        }
        else if(cmd=="stop_ros"){
            humanThread_->stopStreamingRos();
            reply.clear(); reply.addString("OK");
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;
        }
        else if(cmd=="status")
        {
            int curStatus=humanThread_->getStatus();

            reply.clear();
            if(curStatus==STATUS_STOPPED) reply.addString(" Status = STOPPED");
            else if(curStatus==STATUS_CALIBRATION_MAX) reply.addString(" Status = CALIBRATION OF MAX VALUE ONLY");
            else if(curStatus==STATUS_STREAMING) reply.addString(" Status = STREAMING");
            else if(curStatus==STATUS_STREAMING_ROS) reply.addString(" Status = STREAMING TO ROS TOPIC");
            else reply.addString(" Status = IMPOSSIBLE");
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;

        }
        else if(cmd=="calibration_status")
        {
            int curCalibStatus=humanThread_->getCalibrationStatus();

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
            } else if(humanThread_->getStatus() == STATUS_CALIBRATION_MAX){

                reply.clear();
                reply.addString("There is a sensor already being calibrated, please wait.");

            } else{
                int senId = command.get(1).asInt();
                    cout<<"[INFO] second command: " << senId<<endl;
                humanThread_->setCalibSenId(senId);
                humanThread_->startCalibrationMax();
                reply.clear();
                reply.addString("OK");
            }
                        cout<<"[INFO] " << reply.toString()<<endl;
            return true;
        }
        else if(cmd=="save_calibration"){
            humanThread_->saveCalibration();
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
    virtual bool configure(ResourceFinder &rf)
    {
        Time::turboBoost();

        if(rf.check("name"))
            name_    = rf.find("name").asString();
        else
            name_    = "EMGhuman";
        //....................................................

        std::vector<std::pair<int,int>> iccPairs;

        readValue(rf,"rate",rate_,0.01); //10 ms is the default rate for the thread
        readValue(rf,"calibration_duration",calibration_duration_,5);
        readParams(rf,"sensorIds",sensorIds_);
        readParams(rf,"iccPairs",iccPairs);
        readValue(rf,"subject_id",subjectId_,0);
        // readValue(rf,"rosSt",rosStreaming_,0);
        rosStreaming_ = 1;
        

        cout<<"Parameters from init file: "<<endl;
        DSCPA(name_);
        DSCPA(rate_);
        DSCPAstdvec(sensorIds_);
        DSCPAstdvecpair(iccPairs);
        DSCPA(subjectId_);
        std::string foo((rosStreaming_)?"ON":"OFF"); 
        cout<<"ROS streaming is "<<foo<<endl;

        //creating the thread for the server
        humanThread_ = new EMGhumanThread(rate_,name_,calibration_duration_, sensorIds_, iccPairs, subjectId_, rosStreaming_);
        if(!humanThread_->start())
        {
            yError("EMGhuman: cannot start the server thread. Aborting.");
            delete humanThread_;
            return false;
        }
       
    
        //attach a rpc port to the module
        //messages received from the port are redirected to the respond method
        // to connect to the module do:
        // yarp rpc /EMGhuman/rpc 
        // yarp rpc /human_operator/rpc
        rpc.open(string("/"+name_+"/rpc").c_str());
        attach(rpc);
        yInfo("EMGhuman: RPC port attached");

        return true;
    }

   

    //---------------------------------------------------------
    virtual bool close()
    {
       
       //closing thread
       humanThread_->stop();
       delete humanThread_;

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

    /* creates a node called /yarp/talker */
    // Node node("/yarp/talker");
 
    // /* subscribe to topic chatter */
    // yarp::os::Publisher<yarp::rosmsg::std_msgs::String> publisher;
    // if (!publisher.topic("/chatter")) {
    //     yCError(TALKER) << "Failed to create publisher to /chatter";
    //     return -1;
    // }

    // int foo=0;
    // while (foo<10) {
    //     /* prepare some data */
    //     yarp::rosmsg::std_msgs::String data;
    //     data.data = "Hello from YARP";
 
    //     /* publish it to the topic */
    //     publisher.write(data);
 
    //     /* wait some time to avoid flooding with messages */
    //     yarp::os::Time::delay(loop_delay);
    //     foo++;
    // }

    EMGhuman module;
    module.runModule(rf);

    cout << endl << "back to main";

    return 0;
}
