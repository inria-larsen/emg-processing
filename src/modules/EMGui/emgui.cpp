#include "emgui.h"
#include <QVector>

EmGui::EmGui(QObject *parent)
{

    //configure attributes from EMGhuman config files
    loadConfigFiles();

    //open yarp port
    inPortEmg_.open(std::string("/emGui/input:i").c_str());

    opRpcClientPort_.open(std::string("/emGui/op/rpc").c_str());
    colRpcClientPort_.open(std::string("/emGui/col/rpc").c_str());

    op2RobotRpcClientPort_.open(std::string("/emGui/op2robot/rpc").c_str());
    col2RobotRpcClientPort_.open(std::string("/emGui/col2robot/rpc").c_str());

}

void EmGui::close()
{
    //closing all the ports

    inPortEmg_.interrupt();
    inPortEmg_.close();

    opRpcClientPort_.interrupt();
    opRpcClientPort_.close();

    colRpcClientPort_.interrupt();
    colRpcClientPort_.close();

    op2RobotRpcClientPort_.interrupt();
    op2RobotRpcClientPort_.close();

    col2RobotRpcClientPort_.interrupt();
    col2RobotRpcClientPort_.close();

    yInfo("EmGui closing");

}

double EmGui::rate() const
{
    return rate_;
}

int EmGui::opSelectedSensor() const
{
    return opSelectedSensor_;
}

void EmGui::setOpSelectedSensor(int opSelectedSensor)
{
    opSelectedSensor_ = opSelectedSensor;
    emit opSelectedSensorChanged();
}

QVariantList EmGui::opSensorIds() const
{
    return opSensorIds_;
}

void EmGui::setOpSensorIds(const QVariantList &opSensorIds)
{
    opSensorIds_ = opSensorIds;
    emit opSensorIdsChanged();
}

double EmGui::calibDur() const
{
    return calibDur_;
}

void EmGui::setCalibDur(double calibDur)
{
    calibDur_ = calibDur;
    emit calibDurChanged();
}

void EmGui::setRate(double rate)
{
    rate_ = rate;
    emit rateChanged();
}

double EmGui::opBarLevel() const
{
    return opBarLevel_;
}

void EmGui::setOpBarLevel(double opBarLevel)
{
    //Due to a bug in the qml component we're only assigning values to the bar level
    //that are lower than the max value
    if(opBarLevel >= EMG_MAX_VALUE) opBarLevel = EMG_MAX_VALUE;
    else if(opBarLevel<=0) opBarLevel = 0.000001;
    opBarLevel_ = opBarLevel;
    emit opBarLevelChanged();
}

void EmGui::loadConfigFiles()
{
    std::vector<int> sensorIds;
    char* argv2[1];

    ResourceFinder opRf, colRf;

    //getting operator data from config file
    opRf.setDefaultContext("emg-processing");
    opRf.setDefaultConfigFile("human_operator.ini");
    opRf.configure(1,argv2);

    readValue(opRf, "rate",rate_,0.01);
    readValue(opRf,"calibration_duration",calibDur_,5);
    readParams(opRf,"sensorIds",sensorIds);


    DSCPA(rate_);
    DSCPA(calibDur_);
    DSCPA(sensorIds);
    QVariant sensorIdsQVar = QVariant::fromValue ( QVector<int>::fromStdVector(sensorIds) );
    setOpSensorIds( sensorIdsQVar.value<QVariantList>() );

    //now getting collaborator data from config file
    colRf.setDefaultContext("emg-processing");
    colRf.setDefaultConfigFile("human_collaborator.ini");
    colRf.configure(1,argv2);

    sensorIds.clear();
    sensorIdsQVar.clear();

    readParams(colRf,"sensorIds",sensorIds);
    DSCPA(sensorIds);

    sensorIdsQVar = QVariant::fromValue ( QVector<int>::fromStdVector(sensorIds) );
    setColSensorIds( sensorIdsQVar.value<QVariantList>() );

}

QVariantList EmGui::colSensorIds() const
{
    return colSensorIds_;
}

void EmGui::setColSensorIds(const QVariantList &colSensorIds)
{
    colSensorIds_ = colSensorIds;
    emit colSensorIdsChanged();
}

int EmGui::colSelectedSensor() const
{
    return colSelectedSensor_;
}

void EmGui::setColSelectedSensor(int colSelectedSensor)
{
    colSelectedSensor_ = colSelectedSensor;
    emit colSelectedSensorChanged();
}

double EmGui::colBarLevel() const
{
    return colBarLevel_;
}

void EmGui::setColBarLevel(double colBarLevel)
{
    //Due to a bug in the qml component we're only assigning values to the bar level
    //that are lower than the max value
    if(colBarLevel >= EMG_MAX_VALUE) colBarLevel = EMG_MAX_VALUE;
    else if(colBarLevel <= 0) colBarLevel = 0.000001;
    colBarLevel_ = colBarLevel;
    emit colBarLevelChanged();
}

void EmGui::readEmg(void)
{
    if(inPortEmg_.getInputCount() > 0){

        inEmg_ = inPortEmg_.read();

        if(inEmg_ != NULL){
            for (int i=0; i<inEmg_->size(); i=i+2) {
                int currentSenId = inEmg_->get(i).asInt();
                emgMap_[currentSenId] = inEmg_->get(i+1).asDouble();

                //change both bar levels
                if(opSelectedSensor() == currentSenId){
                    setOpBarLevel(emgMap_[currentSenId]);
//                    std::cout<<"[INFO] bar level " <<emgMap_[currentSenId]<<endl;
                }

                if(colSelectedSensor() == currentSenId){
                    setColBarLevel(emgMap_[currentSenId]);
                }
            }
//            DSCPAstdMap(emgMap_);
        }

    }
}

void EmGui::opCalibrateMax()
{
    if(opRpcClientPort_.getOutputCount() > 0){
        Bottle cmd;
        cmd.addString("calibrate_max");
        cmd.addInt(opSelectedSensor());

        std::cout<<"[INFO] Sending cmd to operator: " <<cmd.toString().c_str() << "."<<endl;

        Bottle response;
        opRpcClientPort_.write(cmd,response);

        std::cout<<"[INFO] Got response from operator: "<<response.toString().c_str() <<"."<<endl;

        //TODO: send response to QML and act accordingly
    }
}

void EmGui::opSaveCalibration()
{
    if(opRpcClientPort_.getOutputCount() > 0){
        Bottle cmd;
        cmd.addString("save_calibration");

        std::cout<<"[INFO] Sending cmd to operator: " <<cmd.toString().c_str() << "."<<endl;

        Bottle response;
        opRpcClientPort_.write(cmd,response);

        std::cout<<"[INFO] Got response from operator: "<<response.toString().c_str() <<"."<<endl;

        //TODO: send response to QML and act accordingly
    }
}

void EmGui::opStartStreamingIcc()
{
    if(opRpcClientPort_.getOutputCount() > 0){
        Bottle cmd;
        cmd.addString("start");

        std::cout<<"[INFO] Sending cmd to operator: " <<cmd.toString().c_str() << "."<<endl;

        Bottle response;
        opRpcClientPort_.write(cmd,response);

        std::cout<<"[INFO] Got response from operator: "<<response.toString().c_str() <<"."<<endl;

        //TODO: send response to QML and act accordingly
    }
}

void EmGui::opStopStreamingIcc()
{
    if(opRpcClientPort_.getOutputCount() > 0){
        Bottle cmd;
        cmd.addString("stop");

        std::cout<<"[INFO] Sending cmd to operator: " <<cmd.toString().c_str() << "."<<endl;

        Bottle response;
        opRpcClientPort_.write(cmd,response);

        std::cout<<"[INFO] Got response from operator: "<<response.toString().c_str() <<"."<<endl;

        //TODO: send response to QML and act accordingly
    }
}

void EmGui::colCalibrateMax()
{
    if(colRpcClientPort_.getOutputCount() > 0){
        Bottle cmd;
        cmd.addString("calibrate_max");
        cmd.addInt(colSelectedSensor());

        std::cout<<"[INFO] Sending cmd to collaborator: " <<cmd.toString().c_str() << "."<<endl;

        Bottle response;
        colRpcClientPort_.write(cmd,response);

        std::cout<<"[INFO] Got response from collaborator: "<<response.toString().c_str() <<"."<<endl;

        //TODO: send response to QML and act accordingly
    }
}

void EmGui::colSaveCalibration()
{
    if(colRpcClientPort_.getOutputCount() > 0){
        Bottle cmd;
        cmd.addString("save_calibration");

        std::cout<<"[INFO] Sending cmd to collaborator: " <<cmd.toString().c_str() << "."<<endl;

        Bottle response;
        colRpcClientPort_.write(cmd,response);

        std::cout<<"[INFO] Got response from collaborator: "<<response.toString().c_str() <<"."<<endl;

        //TODO: send response to QML and act accordingly
    }
}

void EmGui::colStartStreamingIcc()
{
    if(colRpcClientPort_.getOutputCount() > 0){
        Bottle cmd;
        cmd.addString("start");

        std::cout<<"[INFO] Sending cmd to collaborator: " <<cmd.toString().c_str() << "."<<endl;

        Bottle response;
        colRpcClientPort_.write(cmd,response);

        std::cout<<"[INFO] Got response from collaborator: "<<response.toString().c_str() <<"."<<endl;

        //TODO: send response to QML and act accordingly
    }
}

void EmGui::colStopStreamingIcc()
{
    if(colRpcClientPort_.getOutputCount() > 0){
        Bottle cmd;
        cmd.addString("stop");

        std::cout<<"[INFO] Sending cmd to collaborator: " <<cmd.toString().c_str() << "."<<endl;

        Bottle response;
        colRpcClientPort_.write(cmd,response);

        std::cout<<"[INFO] Got response from collaborator: "<<response.toString().c_str() <<"."<<endl;

        //TODO: send response to QML and act accordingly
    }
}



QString EmGui::sendOp2RobotRPC(QString cmdStr, QString cmdStr2)
{
    if(op2RobotRpcClientPort_.getOutputCount() > 0){
        Bottle cmd;
        cmd.addString(cmdStr.toStdString().c_str());

        if(cmdStr2.size()>0){
            cmd.addString(cmdStr2.toStdString().c_str());
        }

        std::cout<<"[INFO] Sending cmd to operator2robot module: " <<cmd.toString().c_str() << "."<<endl;

        Bottle response;
        op2RobotRpcClientPort_.write(cmd,response);

        std::cout<<"[INFO] Got response from operator2robot module: "<<response.toString().c_str() <<"."<<endl;

        return QString(response.toString().c_str());

    }

    return QString("NOT CONNECTED");
}

QString EmGui::sendCol2RobotRPC(QString cmdStr, QString cmdStr2)
{
    if(col2RobotRpcClientPort_.getOutputCount() > 0){
        Bottle cmd;
        cmd.addString(cmdStr.toStdString().c_str());

        if(cmdStr2.size()>0){
            cmd.addString(cmdStr2.toStdString().c_str());
        }

        std::cout<<"[INFO] Sending cmd to collaborator2robot module: " <<cmd.toString().c_str() << "."<<endl;

        Bottle response;
        col2RobotRpcClientPort_.write(cmd,response);

        std::cout<<"[INFO] Got response from collaborator2robot module: "<<response.toString().c_str() <<"."<<endl;

        return QString(response.toString().c_str());

    }

    return QString("NOT CONNECTED");
}

void EmGui::beep()
{
    //ONLY WORKS WITH UBUNTU!
    int ret;
    string s("paplay /usr/share/sounds/speech-dispatcher/test.wav");
    ret = system(s.c_str());
}

void EmGui::beep2()
{
    //ONLY WORKS WITH UBUNTU!
    int ret;
    string s("paplay /usr/share/sounds/freedesktop/stereo/screen-capture.oga");
    ret = system(s.c_str());
}


