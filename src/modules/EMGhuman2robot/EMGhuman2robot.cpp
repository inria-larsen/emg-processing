/*
 * EMG human2robot 
 * 
 * Author: Serena Ivaldi, Waldez A. Gomes
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
#include <iostream>
#include <emgutils.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace EmgUtils;


//=====================================
//        EMGhuman2robot control thread
//=====================================

class CtrlThread: public RateThread
{
protected:
    
    Port emgHuman;
    Vector stiffness_arm, stiffness_torso, damping_arm, damping_torso;
    
public:
    CtrlThread(const double period) : RateThread(int(period*1000.0))
    {
//        // we wanna raise an event each time the arm is at 20%
//        // of the trajectory (or 80% far from the target)
//        cartesianEventParameters.type="motion-ongoing";
//        cartesianEventParameters.motionOngoingCheckPoint=0.2;
    }

    virtual bool threadInit(){

        //open yarp input ports

        return true;

    }

    virtual void afterStart(bool s){

    }
    
    bool readFromEmg()
    {
        
        return true;
    }

    virtual void run(){

        //read icc values from EMGhuman module
        // read from port
        readFromEmg();

        //get trajectory point from proMP or file

        //control strategy

        //print status on screen
        printStatus();

    }
    virtual void threadRelease(){
        //close all yarp ports
    }

    /**
     * @brief printStatus logs on the screen the current status of the robot/system
     */
    void printStatus(){

    }
    
    bool beFollower()
    {
        
        return true;
    }
    
    bool beLeader()
    {
        
        return true;
    }
    
    bool beAdaptive()
    {
        
        return true;
    }
    
    bool changeAdaptivePolicy()
    {
        
        return true;
    }
    
    

};

//===============================
//        EMGhuman RFMODULE
//===============================

class EMGhuman2robot: public RFModule
{
private:

    Port rpc_; // the port to handle messages
    int count_;
    double rate_;
    
    Vector stiffness_high_arm, stiffness_high_torso, damping_high_arm, damping_high_torso;
    Vector stiffness_medium_arm, stiffness_medium_torso, damping_medium_arm, damping_medium_torso;
    Vector stiffness_low_arm, stiffness_low_torso, damping_low_arm, damping_low_torso;
    Vector stiffness_follower_arm, stiffness_follower_torso, damping_follower_arm, damping_follower_torso;

    //
    string name_;
    std::vector<std::pair<int,int>> iccPairs_;

    CtrlThread *controlTh_;
    //


public:

    //---------------------------------------------------------
    EMGhuman2robot()
    {
        count_=0;
    }

    //---------------------------------------------------------
    double getPeriod() { return 1.0; }

    //---------------------------------------------------------
    bool updateModule()
    {
        if(count_%60==0)
            cout<<" EMGhuman2robot alive since "<<(count_/60)<<" mins ... "<<endl;
        count_++;
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
        else if(cmd=="leader_behavior")
        {
            
        }
        else if(cmd=="follower_behavior")
        {
            
        }
        else if(cmd=="mixed_behavior")
        {
            ConstString config = command.get(1).asString();
            cout<<"second command = "<<config<<endl;
            if(config=="direct_linear")
            {
                
            }
            if(config=="inverse_linear")
            {
                
            }
            if(config=="direct_3classes")
            {
                
            }
            if(config=="inverse_3classes")
            {
                
            }
            else
            {
                
            }
            
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
    bool configure(ResourceFinder &rf)
    {
        Time::turboBoost();

        if(rf.check("name"))
            name_    = rf.find("name").asString();
        else
            name_    = "EMGhuman2robot";
        //....................................................
        
        readValue(rf,"rate",rate_,0.01); //10 ms is the default rate for the thread
        readParams(rf,"iccPairs",iccPairs_);

        cout<<"Parameters from init file: "<<endl;
        DSCPA(name_);
        DSCPA(rate_);
        DSCPAstdvecpair(iccPairs_);
       
        controlTh_ = new CtrlThread(rate_);
        if(!controlTh_->start())
        {
            yError("EMGhuman2robot: cannot start the control thread. Aborting.");
            delete controlTh_;
            return false;
        }
    
        //attach a port to the module, so we can send messages
        //and choose the type of grasp to execute
        //messages received from the port are redirected to the respond method
        rpc_.open(string("/"+name_+"/rpc:i").c_str());
        attach(rpc_);

        return true;
    }

   

    //---------------------------------------------------------
    bool interruptModule()
    {
        cout<<"Interrupting your module, for port cleanup"<<endl;
        return true;
    }

    //---------------------------------------------------------
    bool close()
    {
        controlTh_->stop();
        delete controlTh_;

        cout<<"Close rpc port"<<endl;
        rpc_.interrupt();
        rpc_.close();
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
    rf.setDefaultConfigFile("emg_operator2robot.ini");
    rf.configure(argc,argv);
  
    if (rf.check("help"))
    {
		printf("\n");
		yInfo("[EMGhuman2robot] Options:");
        yInfo("  --context           path:   where to find the called resource (default emg-processing).");
        yInfo("  --from              from:   the name of the .ini file (default emg_operator2robot.ini).");
        yInfo("  --name              name:   the name of the module (default EMGhuman2robot).");
        printf("\n");

        return 0;
    }
    
    Network yarp;
    if (!yarp.checkNetwork())
    {
        yError("YARP server not available!");
        return -1;
    }

    EMGhuman2robot module;
    module.runModule(rf);

    return 0;
}
