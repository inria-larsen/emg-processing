#include "emgui.h"
#include <QVector>

EmGui::EmGui(QObject *parent)
{

    //configure attributes from EMGhuman config files
    loadConfigFiles();

    //open yarp port
    inPortEmg_.open(std::string("/emGui/input:i").c_str());

}

void EmGui::close()
{
    //closing all the ports

    inPortEmg_.interrupt();
    inPortEmg_.close();


    yInfo("EmGui closing");

}

double EmGui::rate() const
{
    return rate_;
}

double EmGui::calibDur() const
{
    return calibDur_;
}

void EmGui::setCalibDur(double calibDur)
{
    calibDur_ = calibDur;
}

void EmGui::setRate(double rate)
{
    rate_ = rate;
    emit rateChanged();
}

void EmGui::loadConfigFiles()
{
    std::vector<int> sensorIds;
    char* argv2[1];
//    argv2[0] = "";

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

QVariantList EmGui::opSensorIds() const
{
    return opSensorIds_;
}

void EmGui::setOpSensorIds(const QVariantList &opSensorIds)
{
    opSensorIds_ = opSensorIds;
    emit opSensorIdsChanged();
}

