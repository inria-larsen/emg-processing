/*
 * EMG server
 *
 * Author: Serena Ivaldi, Waldez Gomes
 * email:  serena.ivaldi@inria.fr; waldezjr14@gmail.com
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
#include <iostream>
#include "EmgTcp.h"
#include "EmgSignal.h"
#include "emgutils.h"
#include "stdlib.h"
#include <string.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace EmgUtils;


//Used to test the modules when the EMG Delsys Sensors are not immediately available
#define FAKE_EMG_DATA   0

#define STATUS_STOPPED              0x0
#define STATUS_STREAMING_RAW        0x1
#define STATUS_STREAMING_FILTERED   0x2
#define STATUS_STREAMING            0x4

#define CLR_BIT(p,n) ((p) &= ~((1) << (n)))
#define SET_BIT(p,n) ((p) |= (1 << (n)))
#define CHECK_BIT(p,n) ((p) & (1<<(n)))

//==================================================
//        GLOBAL VARIABLES (synched between threads)
//==================================================
//Mutex m;


/**
 * @brief The DelsysThread class gets data as soon as it is available from Delsys TCP server (no trigger input)
 */
class DelsysThread : public Thread {
protected:


    EmgTcp *emgConPtr_; /*!<Pointer to EmgTcp instance, instanced by EMGserverThread */
    EmgSignal emgSig;  /*!<Filters the raw signal from the sensors, RMS + Butterworth*/

    std::vector<float> rawData;/**<Stores the raw data for each one of the 16 sensors*/
    std::vector<double> filteredData; /**<Stores the filtered data for each one of the 16 sensors*/

//    BufferedPort<Bottle> pingTest; /**<Test port used to verify the speed of this DelsysThread*/

public:
    DelsysThread( EmgTcp *emgCon){
        emgConPtr_ = emgCon;
//        pingTest.open("/pingTeste");
    }

    virtual bool threadInit()
    {
        Time::turboBoost();
        yInfo("Starting SensorThread");


        if(!FAKE_EMG_DATA){

            emgConPtr_->configServer();

        }

        return true;
    }

    std::vector<float> getRawData(){
        return rawData;
    }

    std::vector<double> getFilteredData(){
        return filteredData;
    }


    virtual void run() {

        int count = 0;

        long double start = (long double)Time::now();
        while (!isStopping()) {
            //printf("\n Hello, from thread1 ");

            if(FAKE_EMG_DATA){
                std::vector<float> fakeRawData(16,0.0005);
                std::vector<double> fakeFilteredData(16,0.0010);

                fakeFilteredData[0] = fakeRawData[0] = 0.0005;
                fakeFilteredData[1] = fakeRawData[1] = 0.0010;
                fakeFilteredData[2] = fakeRawData[2] = 0.0015;
                fakeFilteredData[3] = fakeRawData[3] = 0.0020;
                fakeFilteredData[4] = fakeRawData[4] = 0.0025;
                fakeFilteredData[5] = fakeRawData[5] = 0.0030;

                rawData = fakeRawData;
                filteredData = fakeFilteredData;
                Time::delay(1/1111); // same as the delay from the sensors
            }
            else if(emgConPtr_->isStreaming() && emgConPtr_->isImEmgConnected() && emgConPtr_->isCmdConnected()){ //check if tcp is connected and streaming

                count++;

                EmgData sample = emgConPtr_->getData();
                emgSig.setSample(sample, count);
                std::vector<double> rmsValues = emgSig.rms();
                std::vector<double> postFiltered = emgSig.butterworth(rmsValues);


                long double curTime = (long double)Time::now() - start;

                rawData = sample.data;
                filteredData = postFiltered;

                /*m.lock();
                            //std::cout<< endl<<(long double)Time::now() - start;
                            std::cout << endl<<"[FAST THREAD]"<<curTime<< " filtered data is: " << filteredData[0];
                        m.unlock();
                        */

//                Bottle& testBot = pingTest.prepare();
//                testBot.clear();
//                testBot.addDouble(1);
//                pingTest.write();

            }


        }
    }
    virtual void threadRelease()
    {
        printf("Goodbye from SensorThread\n");
        if(!FAKE_EMG_DATA){
            emgConPtr_->stopDataStream();
        }
//        pingTest.interrupt();
//        pingTest.close();
    }
};


//===============================================
//        EMGserver Main THREAD (runs every 10ms)
//===============================================

class EMGserverThread: public RateThread
{
protected:

    string name; /**<Name used for the ports*/

    EmgTcp emgCon; /**<Connection to the Delsys Tcp Server*/
    DelsysThread *delTh = NULL; /**<Fast thread to receive and process the data from Delsys Server*/
    BufferedPort<Bottle> raw; /**<Port that outputs the raw data at a slower rate*/
    BufferedPort<Bottle> filtered;/**<Port that outputs the raw data at a slower rate*/

    //bool streamingRaw_ = false;
    //bool streamingFiltered_ = false;
    unsigned int status_ = STATUS_STOPPED;

    std::vector<int> senIds_;/**<Vector that identifies the sensors that are actually being used*/

    int nSensors_ = 0;/**<Number of sensors being used*/

    int count = 0;

public:

    EMGserverThread(const double _period, string _name, string ipadd, int status, int nsens, std::vector<int> senIds)
        :
          RateThread(int(_period*1000.0)),
          name(_name),
          status_(status),
          nSensors_(nsens),
          senIds_(senIds)

    {
        yInfo("EMGserver: thread created");
        emgCon.setIpAdd(ipadd);

    }

    void startCaptureData(int status){
        if (!FAKE_EMG_DATA) emgCon.startDataStream();
        status_ = status;

    }
    void stopCaptureData(int status){
        if (!FAKE_EMG_DATA) emgCon.stopDataStream();
        status_ = status;
    }

    virtual bool threadInit()
    {
        //opening ports
        raw.open(string("/"+name+"/raw:o").c_str());
        filtered.open(string("/"+name+"/filtered:o").c_str());

        bool isConnected = false;

        if(!FAKE_EMG_DATA){

            isConnected = emgCon.connect2Server();

            if(isConnected==false)
            {
                yError("EMGserver: cannot connect to TCP server of Delsys. Aborting module - please check Delsys before restarting.");
                return false;
            }

        }
        else std::cout << "[WARNING] Sending Fake EMG Data (for debugging/developing)"<<endl;

        //creating the thread for the sensors
        delTh = new DelsysThread(&emgCon);
        if(!delTh->start())
        {
            yError("EMGserver: cannot start the sensor thread. Aborting.");
            delete delTh;
            return false;
        }

        if(CHECK_BIT(status_,2) && !FAKE_EMG_DATA ){
            yInfo("Trying to stream real emg data");
            emgCon.startDataStream();
        }


        return true;

    }

    virtual void threadRelease()
    {
        //close sensor thread
        if(delTh != NULL){
            if(delTh->isRunning()){
                delTh->stop();
            }
            delete delTh;

        }
        //closing all the ports
        
        raw.interrupt();
        raw.close();
        filtered.interrupt();
        filtered.close();

        // closing connection with TCP server of Delsys if needed
        // //------> closing connection with delsys is done by the other thread;

        yInfo("EMGserver: thread closing");

    }
    //------ RUN -------
    virtual void run()
    {
        count++;
        // cyclic operations should be put here!
        
        std::vector<float> rawData(16,0.0);
        std::vector<double> filteredData(16,0.0);


        // read raw sensors from EMG
        //m.lock();
        rawData = delTh->getRawData();
        filteredData = delTh->getFilteredData();
        //m.unlock();

        if(rawData.size() > 0){

            //                cout << " [INFO] status: "<< status_;

            if(CHECK_BIT(status_,2) && CHECK_BIT(status_,0)){ //if streaming raw data
                //cout << endl<<"[SLOW THREAD] "<<filteredData[0];

                // send output to raw port
                //
                Bottle& outputRaw = raw.prepare();
                outputRaw.clear();

                //send only the configured sensors for this application
                for(auto id:senIds_){
                    outputRaw.addInt(id);
                    outputRaw.addDouble(rawData[id-1]);
                }

                raw.write();
                                //    cout << "[DEBUG] [RAW DATA] "<<outputRaw.toString()<<endl;
            }


            if(CHECK_BIT(status_,2) && CHECK_BIT(status_,1)){ //if streaming filtered

                // send output to filtered port
                //
                Bottle& outputFil = filtered.prepare();
                outputFil.clear();

                //send only the configured sensors for this application
                for(auto id:senIds_){
                    outputFil.addInt(id);
                    outputFil.addDouble(filteredData[id-1]);
                }

                filtered.write();
                                //    cout << "[DEBUG] [FIL DATA] "<<outputFil.toString()<<endl;

            }

        }
        
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


    unsigned int status_ = STATUS_STOPPED;

    int nSensors_ = 0;

    // server thread
    EMGserverThread *serverThread = NULL ;

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

        if (cmd=="quit"){
            yInfo("\nQuit requested\n Closing now.\n");
            this->close(); //stops module, closes ports...
            return false;
        }


        if (cmd=="list" || cmd=="help")
        {
            reply.clear();
            reply.addString("Here is the list of available commands: ");
            reply.addString("start");
            reply.addString("stop");
            reply.addString("status");
            reply.addString("quit");

            cout<<"[INFO] " << reply.toString()<<endl;

            return true;
        }
        else if(cmd=="stop")
        {
            yInfo("\n stopping data stream");
            status_ &= ~(STATUS_STREAMING);
            serverThread->stopCaptureData(status_);

        }
        else if(cmd=="start")
        {
            yInfo("\n starting data stream");

            status_ |= STATUS_STREAMING;
            serverThread->startCaptureData(status_);


        }
        else if(cmd=="status")
        {
            reply.clear();
            reply.addString("Status is: ");
            reply.addInt(status_);
            cout<<"[INFO] " << reply.toString()<<endl;

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
            cout<<"[INFO] " << reply.toString()<<endl;
            return true;
        }


        reply.clear();
        reply.addString("UNSURE");
        reply.addString(command.get(0).asString());
        reply.addString(command.get(1).asString());
        return true;
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

        bool streamingRaw = false;
        bool streamingFiltered = false;
        std::vector<int> senIds;

        readValue(rf,"ip_add",ipAdd_,"169.254.1.165");
        readValue(rf,"raw",streamingRaw,true);
        readValue(rf,"filter",streamingFiltered,true);
        readValue(rf,"numberOfSensors",nSensors_,1);
        readParams(rf,"sensorIds", senIds);

        if(streamingRaw) SET_BIT(status_,0);
        if(streamingFiltered) SET_BIT(status_,1);
        SET_BIT(status_,2);

        cout<<"Parameters from init file: "<<endl;
        DSCPA(name);
        DSCPA(rate);
        DSCPA(ipAdd_);

        DSCPAstdvec(senIds);


        // //creating the thread for the server
        serverThread = new EMGserverThread(rate,name,ipAdd_,status_,nSensors_, senIds);
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
        if(serverThread != NULL ){

            if(serverThread->isRunning()){
                serverThread->stop();

            }
            serverThread->threadRelease();
            delete serverThread;
        }

        yInfo("EMGserver: closing RPC port");
        if(rpc.isOpen()){
            rpc.interrupt();
            rpc.close();

        }

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
        int ret;
        string s("beep");
        ret = system(s.c_str());

        yError("YARP server not available!");
        return -1;
    }

    EMGserver module;
    module.runModule(rf);

    return 0;
}
