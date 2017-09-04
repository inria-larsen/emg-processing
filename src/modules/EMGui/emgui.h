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
    Q_PROPERTY(double calibDur READ calibDur)
    Q_PROPERTY(QVariantList opSensorIds READ opSensorIds WRITE setOpSensorIds NOTIFY opSensorIdsChanged)
public:
    explicit EmGui(QObject *parent = nullptr);

public slots:
    void close();

    double rate() const;
    double calibDur() const;
    QVariantList opSensorIds() const;

    void setRate(double rate);
    void setCalibDur(double calibDur);
    void setOpSensorIds(const QVariantList &opSensorIds);

signals:
    void rateChanged(void);
    void opSensorIdsChanged(void);


private:
    //private methods

    void loadConfigFiles(void);


    //yarp related attributes

    ResourceFinder rf_;
    Bottle *inEmg_;
    BufferedPort<Bottle> inPortEmg_;

    //operator attributes

    std::string opName_;

    QVariantList opSensorIds_;// sensor ids

    QVariantList opSensorIdMuscleName_;


    //collaborator attributes

    std::string colName_;

    QVariantList colSensorIds_;// sensor ids

    QVariantList colSensorIdMuscleName_;


    //common attributes

    double calibDur_;
    double rate_;
};

#endif // EMGUI_H
