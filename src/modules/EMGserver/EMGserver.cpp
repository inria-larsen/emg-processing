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
Mutex m;

//===================================================================================
//        Delsys THREAD (gets data as soon as it is available from Delsys TCP server)
//===================================================================================
class DelsysThread : public Thread {
    protected:
        EmgTcp *emgConPtr_;
        EmgSignal emgSig; 
        
        std::vector<float> rawData;
        std::vector<double> filteredData;

        BufferedPort<Bottle> pingTest;

    public:
        DelsysThread( EmgTcp *emgCon){
            emgConPtr_ = emgCon;
            pingTest.open("/pingTeste");
        }
        virtual bool threadInit()
        {
            Time::turboBoost();
            printf("Starting SensorThread\n");

            emgConPtr_->configServer();
            std::cout<<std::endl<<"about to start data stream"<<std::endl;

    //======COMMENT THE START DATA STREAM for now...
            emgConPtr_->startDataStream(); 


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

                
                if(emgConPtr_->isStreaming() && emgConPtr_->isImEmgConnected() && emgConPtr_->isCmdConnected()){ //check if tcp is connected and streaming
                    
                    count++;

                    EmgData sample = emgConPtr_->getData();
                    emgSig.setSample(sample, count);
                        std::vector<double> rmsValues = emgSig.rms();
                        std::vector<double> postFiltered = emgSig.butterworth(rmsValues); 


                        long double curTime = (long double)Time::now() - start;
                        
                        rawData = sample.data;
                        filteredData = postFiltered;

                        m.lock();
                            //std::cout<< endl<<(long double)Time::now() - start;
                            std::cout << endl<<"[FAST THREAD]"<<curTime<< " filtered data is: " << filteredData[0];
                        m.unlock();

                    Bottle& testBot = pingTest.prepare();
                    testBot.clear();
                    testBot.addDouble(1);
                    pingTest.write();

                }
                
                
            }
        }
        virtual void threadRelease()
        {
            printf("Goodbye from SensorThread\n");
            emgConPtr_->stopDataStream(); 
            pingTest.interrupt();
            pingTest.close();
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

        EmgTcp emgCon;
        DelsysThread *delTh;
        BufferedPort<Bottle> raw;
        BufferedPort<Bottle> filtered;

        bool streamingRaw_ = false;
        bool streamingFiltered_ = false;

        int nSensors_ = 0;

        int count = 0;

    public: 

    EMGserverThread(const double _period, string _name, string ipadd): RateThread(int(_period*1000.0))
    {
        name = _name;
        yInfo("EMGserver: thread created");
        emgCon.setIpAdd(ipadd);

    }

    void startCaptureData(void){
        emgCon.startDataStream();
    }
    void stopCaptureData(void){
        emgCon.stopDataStream();
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


        // //creating the thread for the sensors
        delTh = new DelsysThread(&emgCon);
        if(!delTh->start())
        {
            yError("EMGserver: cannot start the sensor thread. Aborting.");
            delete delTh;
            return false;
        }


        //opening ports
        raw.open(string("/"+name+"/raw:o").c_str());
        filtered.open(string("/"+name+"/filtered:o").c_str());
        

        return true;

    }

    virtual void threadRelease()
    {
        //close sensor thread
        delTh->stop();
        delete delTh;
        //closing all the ports
        
        raw.interrupt();
        raw.close();
        filtered.interrupt();
        filtered.close();        

        // closing connection with TCP server of Delsys if needed
        // //------> closing connection with delsys is done by the other thread;

        yInfo("EMGserver: thread closing");

    }
    void setStreaming(bool rawBool, bool filBool, int nsen){

        streamingRaw_ = rawBool;
        streamingFiltered_ = filBool;
        nSensors_ = nsen;
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

                if(streamingRaw_){
                    cout << endl<<"[SLOW THREAD] "<<filteredData[0];

                    // send output to raw port
                    // 
                    Bottle& outputRaw = raw.prepare();
                    outputRaw.clear();

                    for(int ite = 0; ite < nSensors_ ; ite ++ ){
                        outputRaw.addDouble(rawData[ite]);
                    }
                    
                    raw.write();
                }


                if(streamingFiltered_){

                    // send output to filtered port
                    // 
                    Bottle& outputFil = filtered.prepare();
                    outputFil.clear();

                    for(int ite = 0; ite < nSensors_ ; ite ++ ){
                        outputFil.addDouble(filteredData[ite]);
                    }

                    //cout << "writing " << output.toString().c_str() << endl;
                    filtered.write();
                    
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

    bool streamingRaw_ = false;
    bool streamingFiltered_ = false;

    int nSensors_ = 0;

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

        if (cmd=="quit"){
            yInfo("\nQuit requested\n Closing now.\n");
            this->close(); //stops module, closes ports...
            return false;
        }


        if (cmd=="list" || cmd=="help")
        {
            reply.clear();
            reply.addString("Here is the list of available commands: ");

            return true;
        }  
        else if(cmd=="stop")
        {
            yInfo("\n stopping data stream");
            serverThread->stopCaptureData(); 
        }
        else if(cmd=="start")
        {
            yInfo("\n starting data stream");
            serverThread->startCaptureData();

        } 
        else if(cmd=="status")
        {
            reply.clear();
            //reply.addString()

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

    void readValue(ResourceFinder &rf, string s, int &v, int vdefault)
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

    void readValue(ResourceFinder &rf, string s, bool &v, bool vdefault)
    {
        if(rf.check(s.c_str()))
        {
            v = rf.find(s.c_str()).asBool();
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
        readValue(rf,"raw",streamingRaw_,true);
        readValue(rf,"filter",streamingFiltered_,true);
        readValue(rf,"numberOfSensors",nSensors_,1);

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

        serverThread->setStreaming(streamingRaw_,streamingFiltered_,nSensors_);
       
    
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
