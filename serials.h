#ifndef SERIALS_H
#define SERIALS_H

#include <QObject>
#include <QSharedDataPointer>
#include <QtSerialPort/qserialport>
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>

class SerialsData;

class Serials:public QSerialPort
{
    Q_OBJECT
public:
    Serials();
    Serials(const Serials &);
    Serials &operator=(const Serials &);
    void setOpenMode(OpenMode opm){QSerialPort::setOpenMode(opm);}
    ~Serials();
private:
    QSharedDataPointer<SerialsData> data;
};

#endif // SERIALS_H
