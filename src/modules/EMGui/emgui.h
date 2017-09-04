#ifndef EMGUI_H
#define EMGUI_H

#include <QObject>
#include <QVariant>
#include "emgutils.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace EmgUtils;



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
    void close();

    double rate() const;
    double calibDur() const;

    QVariantList opSensorIds() const;
    int opSelectedSensor() const;
    double opBarLevel() const;

    int colSelectedSensor() const;
    double colBarLevel() const;
    QVariantList colSensorIds() const;

    void setRate(double rate);
    void setCalibDur(double calibDur);

    void setOpSensorIds(const QVariantList &opSensorIds);
    void setOpSelectedSensor(int opSelectedSensor);
    void setOpBarLevel(double opBarLevel);

    void setColBarLevel(double colBarLevel);
    void setColSelectedSensor(int colSelectedSensor);
    void setColSensorIds(const QVariantList &colSensorIds);

    void readEmg(void);

    void opCalibrateMax();
    void opSaveCalibration(void);

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

    void loadConfigFiles(void);


    //yarp related attributes

    ResourceFinder rf_;
    Bottle *inEmg_;
    BufferedPort<Bottle> inPortEmg_;
    RpcClient opRpcClientPort_;



    //operator attributes

    std::string opName_;
    QVariantList opSensorIds_; //sensor ids
    QVariantList opSensorIdMuscleName_;
    int opSelectedSensor_;
    double opBarLevel_; // indication of EMG level for visual feedback on gui


    //collaborator attributes

    std::string colName_;
    QVariantList colSensorIdMuscleName_;    
    QVariantList colSensorIds_; //sensor ids
    int colSelectedSensor_;
    double colBarLevel_;


    //common attributes

    double calibDur_;
    double rate_;

    std::map<int,double> emgMap_; //EMG values for each sensor (id)
};

#endif // EMGUI_H
