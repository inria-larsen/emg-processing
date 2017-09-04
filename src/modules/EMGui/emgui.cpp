#include "emgui.h"
#include <QVector>

EmGui::EmGui(QObject *parent)
{

    //configure attributes from EMGhuman config files
    loadConfigFiles();

    //open yarp port
    inPortEmg_.open(std::string("/emGui/input:i").c_str());

    opRpcClientPort_.open(std::string("/emGui/op/rpc").c_str());

}

void EmGui::close()
{
    //closing all the ports

    inPortEmg_.interrupt();
    inPortEmg_.close();

    opRpcClientPort_.interrupt();
    opRpcClientPort_.close();

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
    opBarLevel_ = opBarLevel;
    emit opBarLevelChanged();
}

void EmGui::loadConfigFiles()
{
    std::vector<int> sensorIds;
    char* argv2[1];

    //getting operator
    rf_.setDefaultContext("emg-processing");
    rf_.setDefaultConfigFile("human_operator.ini");
    rf_.configure(1,argv2);

    readValue(rf_, "rate",rate_,0.01);
    readValue(rf_,"calibration_duration",calibDur_,5);
    readParams(rf_,"sensorIds",sensorIds);


    DSCPA(rate_);
    DSCPA(calibDur_);
    DSCPA(sensorIds);
    QVariant sensorIdsQVar = QVariant::fromValue ( QVector<int>::fromStdVector(sensorIds) );
    setOpSensorIds( sensorIdsQVar.value<QVariantList>() );

}

QVariantList EmGui::colSensorIds() const
{
    return colSensorIds_;
}

void EmGui::setColSensorIds(const QVariantList &colSensorIds)
{
    colSensorIds_ = colSensorIds;
}

int EmGui::colSelectedSensor() const
{
    return colSelectedSensor_;
}

void EmGui::setColSelectedSensor(int colSelectedSensor)
{
    colSelectedSensor_ = colSelectedSensor;
}

double EmGui::colBarLevel() const
{
    return colBarLevel_;
}

void EmGui::setColBarLevel(double colBarLevel)
{
    colBarLevel_ = colBarLevel;
}



void EmGui::readEmg(void)
{
    if(inPortEmg_.getInputCount() > 0){

        inEmg_ = inPortEmg_.read();

        if(inEmg_ != NULL){
            for (int i=0; i<inEmg_->size(); i=i+2) {
                int currentSenId = inEmg_->get(i).asInt();
                emgMap_[currentSenId] = inEmg_->get(i+1).asDouble();

    //            DSCPA(opSelectedSensor_);

                if(opSelectedSensor() == currentSenId){
                    setOpBarLevel(emgMap_[currentSenId]);
                }
            }
    //        DSCPAstdMap(emgMap_);
        }

    }
}

void EmGui::opCalibrateMax()
{
    if(opRpcClientPort_.getOutputCount() > 0){
        Bottle cmd;
        cmd.addString("calibrate_max");
        cmd.addInt(opSelectedSensor());

        std::cout<<"[INFO] Sending cmd: " <<cmd.toString().c_str() << "."<<endl;

        Bottle response;
        opRpcClientPort_.write(cmd,response);

        std::cout<<"[INFO] Got response: "<<response.toString().c_str() <<"."<<endl;

        //TODO: send response to QML and act accordingly
    }
}

void EmGui::opSaveCalibration()
{
    if(opRpcClientPort_.getOutputCount() > 0){
        Bottle cmd;
        cmd.addString("save_calibration");

        std::cout<<"[INFO] Sending cmd: " <<cmd.toString().c_str() << "."<<endl;

        Bottle response;
        opRpcClientPort_.write(cmd,response);

        std::cout<<"[INFO] Got response: "<<response.toString().c_str() <<"."<<endl;

        //TODO: send response to QML and act accordingly
    }
}


