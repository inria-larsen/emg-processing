#ifndef EMGUI_H
#define EMGUI_H

#include <QObject>
#include <QVariant>
#include "emgutils.h"

#define EMG_MAX_VALUE 0.0025

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace EmgUtils;


/**
 * @brief The EmGui class
 * Interfaces the GUI with the other yarp modules
 */
class EmGui : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double rate READ rate NOTIFY rateChanged)
    Q_PROPERTY(double calibDur READ calibDur NOTIFY calibDurChanged)

    Q_PROPERTY(QVariantList opSensorIds READ opSensorIds WRITE setOpSensorIds NOTIFY opSensorIdsChanged)
    Q_PROPERTY(int opSelectedSensor READ opSelectedSensor WRITE setOpSelectedSensor NOTIFY opSelectedSensorChanged)
    Q_PROPERTY(double opBarLevel READ opBarLevel WRITE setOpBarLevel NOTIFY opBarLevelChanged)

    Q_PROPERTY(QVariantList colSensorIds READ colSensorIds WRITE setColSensorIds NOTIFY colSensorIdsChanged)
    Q_PROPERTY(int colSelectedSensor READ colSelectedSensor WRITE setColSelectedSensor NOTIFY colSelectedSensorChanged)
    Q_PROPERTY(double colBarLevel READ colBarLevel WRITE setColBarLevel NOTIFY colBarLevelChanged)


public:
    explicit EmGui(QObject *parent = nullptr);

public slots:

    /**
     * @brief close the Yarp connections before exiting
     */
    void close();

    /**
     * @brief reads from the EMG levels from the input port,
     * and change the bar levels of the selected sensor at the GUI
     */
    void readEmg(void);

    /**
     * @brief outputs a sound to indicate relevant events at the GUI.
     * Currently works ONLY on UBUNTU.
     */
    void beep();

    //Getters
    double rate() const;
    double calibDur() const;

    QVariantList opSensorIds() const;
    int opSelectedSensor() const;
    double opBarLevel() const;

    int colSelectedSensor() const;
    double colBarLevel() const;
    QVariantList colSensorIds() const;

    //Setters
    void setRate(double rate);
    void setCalibDur(double calibDur);

    void setOpSensorIds(const QVariantList &opSensorIds);
    void setOpSelectedSensor(int opSelectedSensor);
    void setOpBarLevel(double opBarLevel);

    void setColBarLevel(double colBarLevel);
    void setColSelectedSensor(int colSelectedSensor);
    void setColSensorIds(const QVariantList &colSensorIds);

    void opCalibrateMax();
    void opSaveCalibration(void);

    void colCalibrateMax();
    void colSaveCalibration(void);


signals:
    void rateChanged(void);
    void calibDurChanged(void);

    void opSensorIdsChanged(void);
    void opSelectedSensorChanged(void);
    void opBarLevelChanged(void);

    void colBarLevelChanged(void);
    void colSelectedSensorChanged(void);
    void colSensorIdsChanged(void);


private:
    //private methods

    /**
     * @brief loadConfigFiles loads the .INI configuration files:
     * ** human_operator.ini
     * ** human_collaborator.ini
     *
     * And use their data to construct THIS object
     */
    void loadConfigFiles(void);


    //yarp related attributes


    Bottle *inEmg_; // input EMG bottle
    BufferedPort<Bottle> inPortEmg_; //input port
    RpcClient opRpcClientPort_; // RPC port for the human operator
    RpcClient colRpcClientPort_; // RPC port for the human collaborator



    //operator attributes

    std::string opName_; // human operator name;
    QVariantList opSensorIds_; //sensor ids for the human operator
    QVariantList opSensorIdMuscleName_; // muscle names for the human operator
    int opSelectedSensor_; // currently selected sensor for the human operator
    double opBarLevel_; // indication of EMG level for visual feedback on gui for human operator


    //collaborator attributes

    std::string colName_; // human collaborator name;
    QVariantList colSensorIdMuscleName_; // muscle names for the human collaborator
    QVariantList colSensorIds_; //sensor ids for the human collaborator
    int colSelectedSensor_; // currently selected sensor for the human collaborator
    double colBarLevel_; // indication of EMG level for visual feedback on gui for human collaborator


    //common attributes

    double calibDur_ ;// calibration duration;
    double rate_; // rate of operation, same rate as for the human operator;

    std::map<int,double> emgMap_; //EMG values for each sensor (id)
};

#endif // EMGUI_H
