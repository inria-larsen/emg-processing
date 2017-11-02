/*
 * EMG human2robot 
 * 
 * Author: Serena Ivaldi, Waldez A. Gomes
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
#include <iostream>
#include <emgutils.h>
#include "robot_interfaces.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace EmgUtils;

#define POLICY_DIRECT               0
#define POLICY_DIRECT_3_STATES      1
#define POLICY_INVERSE              2
#define POLICY_INVERSE_3_STATES     3
#define POLICY_MANUAL               4


#define STATUS_STOPPED 10
#define STATUS_RUNNING 20


//=====================================
//        EMGhuman2robot control thread
//=====================================

class CtrlThread: public RateThread
{
protected:
    
    Port emgHumanPort_;
    std::vector<double> stiffness_arm, stiffness_torso, damping_arm, damping_torso;
    string robot_name_;
    robot_interfaces *robotInt_;
    bool useRightArm_;
    bool useTorso_;
    bool useLeftArm_;
    int policy_;
    int status_;

    double iccForearm_;
    double iccLowZero_,iccLowMax_,iccMedMax_,iccHighMax_;
    
public:
    CtrlThread(const double period, string robot_name, const string policy,
               double iccLowZero,double iccLowMax,double iccMedMax,double iccHighMax,
               bool useLeft, bool useRight, bool useTorso) :
        RateThread(int(period*1000.0)),
        robot_name_(robot_name),
        iccLowZero_(iccLowZero),
        iccLowMax_(iccLowMax),
        iccMedMax_(iccMedMax),
        iccHighMax_(iccHighMax),
        useLeftArm_(useLeft),
        useRightArm_(useRight),
        useTorso_(useTorso)

    {
        if(policy.compare("direct")){
            policy_ = POLICY_DIRECT;
        }
        else if(policy.compare("direct_three_states")){
            policy_ = POLICY_DIRECT_3_STATES;
        }
        else if(policy.compare("direct_inverse")){
            policy_ = POLICY_INVERSE;
        }
        else if(policy.compare("inverse_three_states")){
            policy_ = POLICY_INVERSE_3_STATES;
        }
        else if(policy.compare("manual")){
            policy_ = POLICY_MANUAL;
        }

        status_=STATUS_STOPPED;
    }
    
    ~CtrlThread()
    {
        yInfo("Disconnecting the robot");
        delete robotInt_;
        
    }

    virtual bool threadInit()
    {
        // we start as stopped without controlling the robot
        status_=STATUS_STOPPED;

        //open yarp input ports (connection to EMGhuman)
        
        yInfo("Connecting to the robot");
        //connect to robot
        robotInt_ = new robot_interfaces();
        if (robotInt_->init(robot_name_) == false)
        {
             yError("Failed to connect to the robot");
             delete robotInt_;
             return false;
        }
        

        return true;

    }

    bool readFromEmg()
    {
        
        return true;
    }

    virtual void run()
    {

        //read icc values from EMGhuman module
        // read from port
        // note: we read even if we don't control the robot
        readFromEmg();

        // we only control the robot if we decided that explicitly
        if(status_==STATUS_RUNNING)
        {
            //only reset impedance in the control loop if the policy is adaptive
            if(policy_ != POLICY_MANUAL)
            {
                //Compute impedance depending on policy
                if( !setAdaptiveImpedance() ){
                    yError("Could not set adaptive impedance");
                }
            }
        }

        



    }
    
    virtual void threadRelease()
    {
        status_=STATUS_STOPPED;

        //close all yarp ports
    }

    /**
     * @brief printStatus logs on the screen the current status of the robot/system
     */
    void printStatus()
    {
        yInfo("EMG_human2robot: status");
        switch(policy)
        {
            case POLICY_DIRECT: yInfo(" ** Policy: direct"); break;
            case POLICY_DIRECT_3_STATES: yInfo(" ** Policy: direct 3 states (low/medium/high stiffness"); break;
            case POLICY_INVERSE: yInfo(" ** Policy: inverse"); break;
            case POLICY_INVERSE_3_STATES: yInfo(" ** Policy: inverse 3 states (high/medium/low stiffness"); break;
            case POLICY_MANUAL: yInfo(" ** Policy: manual"); break;
            default:
                yInfo(" ** Policy: NOT SET and we don't know why"); break;
        }



    }
    
    // Follower is zero torque control
    bool setFollower()
    {
        if(useTorso_)
        {
            robotInt_->iimp[TORSO]->setImpedance(0, 0.0, 0.0);
            robotInt_->iimp[TORSO]->setImpedance(1, 0.0, 0.0);
            robotInt_->iimp[TORSO]->setImpedance(2, 0.1, 0.0);
            robotInt_->icmd[TORSO]->setTorqueMode(0);
            robotInt_->icmd[TORSO]->setTorqueMode(1);
            //3rd joint of torso is never in torque control (see demoForceControl)
            robotInt_->icmd[TORSO]->setPositionMode(2);
            robotInt_->iint[TORSO]->setInteractionMode(2, VOCAB_IM_COMPLIANT);
        }
        
        if(useLeftArm_)
        {
            robotInt_->iimp[LEFT_ARM]->setImpedance(0, 0.0, 0.0);
            robotInt_->iimp[LEFT_ARM]->setImpedance(1, 0.0, 0.0);
            robotInt_->iimp[LEFT_ARM]->setImpedance(2, 0.0, 0.0);
            robotInt_->iimp[LEFT_ARM]->setImpedance(3, 0.0, 0.0);
            robotInt_->iimp[LEFT_ARM]->setImpedance(4, 0.0, 0.0);
            robotInt_->icmd[LEFT_ARM]->setTorqueMode(0);
            robotInt_->icmd[LEFT_ARM]->setTorqueMode(1);
            robotInt_->icmd[LEFT_ARM]->setTorqueMode(2);
            robotInt_->icmd[LEFT_ARM]->setTorqueMode(3);
            robotInt_->icmd[LEFT_ARM]->setTorqueMode(4);
        }
        
        if(useRightArm_)
        {
            robotInt_->iimp[RIGHT_ARM]->setImpedance(0, 0.0, 0.0);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(1, 0.0, 0.0);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(2, 0.0, 0.0);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(3, 0.0, 0.0);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(4, 0.0, 0.0);
            robotInt_->icmd[RIGHT_ARM]->setTorqueMode(0);
            robotInt_->icmd[RIGHT_ARM]->setTorqueMode(1);
            robotInt_->icmd[RIGHT_ARM]->setTorqueMode(2);
            robotInt_->icmd[RIGHT_ARM]->setTorqueMode(3);
            robotInt_->icmd[RIGHT_ARM]->setTorqueMode(4);
        
        }
        return true;
    }
    
    // Leader is high stiffness
    bool setLeader()
    {
        return setHighStiffness();
    }
    
    // Hard-coded values come from demoForceControl -> soft Impedance
    bool setLowStiffness()
    {
        if(useRightArm_)
        {
            robotInt_->iimp[RIGHT_ARM]->setImpedance(0,0.2,0.0);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(1,0.2,0.0);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(2,0.2,0.0);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(3,0.2,0.0);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(4,0.1,0.0);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(0);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(1);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(2);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(3);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(4);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(0, VOCAB_IM_COMPLIANT);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(1, VOCAB_IM_COMPLIANT);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(2, VOCAB_IM_COMPLIANT);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(3, VOCAB_IM_COMPLIANT);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(4, VOCAB_IM_COMPLIANT);
        }
        
        if(useLeftArm_)
        {
            robotInt_->iimp[LEFT_ARM]->setImpedance(0,0.2,0.0);
            robotInt_->iimp[LEFT_ARM]->setImpedance(1,0.2,0.0);
            robotInt_->iimp[LEFT_ARM]->setImpedance(2,0.2,0.0);
            robotInt_->iimp[LEFT_ARM]->setImpedance(3,0.2,0.0);
            robotInt_->iimp[LEFT_ARM]->setImpedance(4,0.1,0.0);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(0);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(1);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(2);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(3);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(4);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(0, VOCAB_IM_COMPLIANT);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(1, VOCAB_IM_COMPLIANT);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(2, VOCAB_IM_COMPLIANT);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(3, VOCAB_IM_COMPLIANT);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(4, VOCAB_IM_COMPLIANT);
        }
        
        if(useTorso_)
        {
            robotInt_->iimp[TORSO]->setImpedance(0,0.1,0.0);
            robotInt_->iimp[TORSO]->setImpedance(1,0.1,0.0);
            robotInt_->iimp[TORSO]->setImpedance(2,0.1,0.0);
            robotInt_->icmd[TORSO]->setPositionMode(0);
            robotInt_->icmd[TORSO]->setPositionMode(1);
            robotInt_->icmd[TORSO]->setPositionMode(2);
            robotInt_->iint[TORSO]->setInteractionMode(0, VOCAB_IM_COMPLIANT);
            robotInt_->iint[TORSO]->setInteractionMode(1, VOCAB_IM_COMPLIANT);
            robotInt_->iint[TORSO]->setInteractionMode(2, VOCAB_IM_COMPLIANT);
        }
        
        return true;
    }
    
    // Hard-coded values come from demoForceControl -> medium Impedance
    bool setMediumStiffness()
    {
        if(useRightArm_)
        {
            robotInt_->iimp[RIGHT_ARM]->setImpedance(0,0.4,0.03);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(1,0.4,0.03);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(2,0.4,0.03);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(3,0.2,0.01);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(4,0.2,0.0);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(0);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(1);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(2);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(3);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(4);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(0, VOCAB_IM_COMPLIANT);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(1, VOCAB_IM_COMPLIANT);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(2, VOCAB_IM_COMPLIANT);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(3, VOCAB_IM_COMPLIANT);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(4, VOCAB_IM_COMPLIANT);
        }
        
        if(useLeftArm_)
        {
            robotInt_->iimp[LEFT_ARM]->setImpedance(0,0.4,0.03);
            robotInt_->iimp[LEFT_ARM]->setImpedance(1,0.4,0.03);
            robotInt_->iimp[LEFT_ARM]->setImpedance(2,0.4,0.03);
            robotInt_->iimp[LEFT_ARM]->setImpedance(3,0.2,0.01);
            robotInt_->iimp[LEFT_ARM]->setImpedance(4,0.2,0.0);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(0);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(1);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(2);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(3);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(4);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(0, VOCAB_IM_COMPLIANT);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(1, VOCAB_IM_COMPLIANT);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(2, VOCAB_IM_COMPLIANT);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(3, VOCAB_IM_COMPLIANT);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(4, VOCAB_IM_COMPLIANT);
        }
        
        if(useTorso_)
        {
            robotInt_->iimp[TORSO]->setImpedance(0,0.3,0.0);
            robotInt_->iimp[TORSO]->setImpedance(1,0.3,0.0);
            robotInt_->iimp[TORSO]->setImpedance(2,0.3,0.0);
            robotInt_->icmd[TORSO]->setPositionMode(0);
            robotInt_->icmd[TORSO]->setPositionMode(1);
            robotInt_->icmd[TORSO]->setPositionMode(2);
            robotInt_->iint[TORSO]->setInteractionMode(0, VOCAB_IM_COMPLIANT);
            robotInt_->iint[TORSO]->setInteractionMode(1, VOCAB_IM_COMPLIANT);
            robotInt_->iint[TORSO]->setInteractionMode(2, VOCAB_IM_COMPLIANT);
        }
        
        return true;
    }
    
    // Hard-coded values come from demoForceControl -> hard Impedance
    bool setHighStiffness()
    {
        if(useRightArm_)
        {
            robotInt_->iimp[RIGHT_ARM]->setImpedance(0,0.6,0.06);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(1,0.6,0.06);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(2,0.6,0.06);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(3,0.3,0.02);
            robotInt_->iimp[RIGHT_ARM]->setImpedance(4,0.3,0.0);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(0);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(1);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(2);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(3);
            robotInt_->icmd[RIGHT_ARM]->setPositionMode(4);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(0, VOCAB_IM_COMPLIANT);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(1, VOCAB_IM_COMPLIANT);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(2, VOCAB_IM_COMPLIANT);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(3, VOCAB_IM_COMPLIANT);
            robotInt_->iint[RIGHT_ARM]->setInteractionMode(4, VOCAB_IM_COMPLIANT);
        }
        
        if(useLeftArm_)
        {
            robotInt_->iimp[LEFT_ARM]->setImpedance(0,0.6,0.06);
            robotInt_->iimp[LEFT_ARM]->setImpedance(1,0.6,0.06);
            robotInt_->iimp[LEFT_ARM]->setImpedance(2,0.6,0.06);
            robotInt_->iimp[LEFT_ARM]->setImpedance(3,0.3,0.02);
            robotInt_->iimp[LEFT_ARM]->setImpedance(4,0.2,0.0);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(0);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(1);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(2);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(3);
            robotInt_->icmd[LEFT_ARM]->setPositionMode(4);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(0, VOCAB_IM_COMPLIANT);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(1, VOCAB_IM_COMPLIANT);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(2, VOCAB_IM_COMPLIANT);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(3, VOCAB_IM_COMPLIANT);
            robotInt_->iint[LEFT_ARM]->setInteractionMode(4, VOCAB_IM_COMPLIANT);
        }
        
        if(useTorso_)
        {
            robotInt_->iimp[TORSO]->setImpedance(0,0.7,0.015);
            robotInt_->iimp[TORSO]->setImpedance(1,0.7,0.015);
            robotInt_->iimp[TORSO]->setImpedance(2,0.7,0.015);
            robotInt_->icmd[TORSO]->setPositionMode(0);
            robotInt_->icmd[TORSO]->setPositionMode(1);
            robotInt_->icmd[TORSO]->setPositionMode(2);
            robotInt_->iint[TORSO]->setInteractionMode(0, VOCAB_IM_COMPLIANT);
            robotInt_->iint[TORSO]->setInteractionMode(1, VOCAB_IM_COMPLIANT);
            robotInt_->iint[TORSO]->setInteractionMode(2, VOCAB_IM_COMPLIANT);
        }
        
        return true;
    }
    
    // Policies for adaptive impedance
    bool setAdaptiveImpedance()
    {
        switch (policy_) {
        case POLICY_DIRECT:
            yWarning("linear DIRECT policy not implemented yet");
            break;
        case POLICY_DIRECT_3_STATES:

            yInfo("Setting DIRECT 3 STATES policy");
            if(iccForearm_ <= iccLowMax_){
                return setLowStiffness();
            }
            else if(iccForearm_ <= iccMedMax_ && iccForearm_ > iccLowMax_){
                return setMediumStiffness();
            }
            else if(iccForearm_ <= iccHighMax_ && iccForearm_ > iccMedMax_){
                return setHighStiffness();
            }

            break;
        case POLICY_INVERSE:
            yWarning("linear INVERSE policy not implemented yet");
            break;
        case POLICY_INVERSE_3_STATES:

            yInfo("Setting INVERSE 3 STATES policy");
            if(iccForearm_ <= iccLowMax_){
                return setHighStiffness();
            }
            else if(iccForearm_ <= iccMedMax_ && iccForearm_ > iccLowMax_){
                return setMediumStiffness();
            }
            else if(iccForearm_ <= iccHighMax_ && iccForearm_ > iccMedMax_){
                return setLowStiffness();
            }

            break;
        default:
            yError("Policy was not set properly.");
            return false;
            break;
        }
        return true;
    }
    
    bool setRobotImpedance(std::vector<double> arm_stiff, std::vector<double> arm_damp,
                          std::vector<double> torso_stiff, std::vector<double> torso_damp)
    {

        // TODO

        return true;
    }
    
    bool setPolicy(const int policy)
    {
        policy_ = policy;
        return true;
    }

    int getPolicy() const
    {
        return policy_;
    }

    int getStatus() const
    {
        return status_;
    }

    bool startControllingRobot()
    {
        if(status_==STATUS_RUNNING)
            yWarning("Hey I am already controlling the robot ...");
        else
            yInfo("Starting to control the robot...");
        status_=STATUS_RUNNING;
        return true;
    }

    bool stopControllingRobot()
    {
        if(status_==STATUS_RUNNING)
            yInfo("Stopping controlling the robot...");
        else
            yWarning("Hey I am not controlling the robot ...");
        status_=STATUS_STOPPED;
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
    
    std::vector<double> stiffness_high_arm, stiffness_high_torso, damping_high_arm, damping_high_torso;
    std::vector<double> stiffness_medium_arm, stiffness_medium_torso, damping_medium_arm, damping_medium_torso;
    std::vector<double> stiffness_low_arm, stiffness_low_torso, damping_low_arm, damping_low_torso;
    std::vector<double> stiffness_follower_arm, stiffness_follower_torso, damping_follower_arm, damping_follower_torso;

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

        string name;
        string robotName;
        std::vector<std::pair<int,int>> iccPairs;
        double rate;
        std::string policy;
        double iccLowZero,iccLowMax,iccMedMax,iccHighMax;
        bool useLeft, useRight, useTorso;

        if(rf.check("name"))
            name    = rf.find("name").asString();
        else
            name    = "EMGhuman2robot";
        //....................................................
        
        readValue(rf,"rate",rate,0.01); //10 ms is the default rate for the thread
        readParams(rf,"iccPairs",iccPairs);
        readValue(rf, "robot",robotName,"icubSim");
        readValue(rf, "policy",policy,"direct");
        readValue(rf,"iccLowZero",iccLowZero,0.01);
        readValue(rf,"iccLowMax",iccLowMax,0.1);
        readValue(rf,"iccMediumMax",iccMedMax,0.4);
        readValue(rf,"iccHighMax",iccHighMax,1.0);
        readValue(rf,"useLeftArm",useLeft,false);
        readValue(rf,"useRightArm",useRight,true);
        readValue(rf,"useTorso",useTorso,true);

        cout<<"Parameters from init file: "<<endl;
        DSCPA(name);
        DSCPA(rate);
        DSCPA(robotName)
        DSCPAstdvecpair(iccPairs);
       
        controlTh_ = new CtrlThread(rate, robotName,policy,iccLowZero,iccLowMax,iccMedMax,iccHighMax,useLeft,useRight,useTorso);
        if(!controlTh_->start())
        {
            yError("EMGhuman2robot: cannot start the control thread. Aborting.");
            delete controlTh_;
            return false;
        }
    
        //attach a port to the module, so we can send messages
        //and choose the type of grasp to execute
        //messages received from the port are redirected to the respond method
        rpc_.open(string("/"+name+"/rpc:i").c_str());
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
