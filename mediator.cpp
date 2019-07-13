#include "mediator.h"
#include "mcamera.h"
#include "imagedeal.h"
#include "global.h"


unsigned int PLCMessageThread(void *p){
    std::cout <<"start thread" <<std::endl;
    PositionInfo Pos;
    for(;;)
    {
        ::WaitForSingleObject(Mediator::GetIns()->evt_PLCMessage.get(),INFINITE);
        using namespace  HalconCpp;
        try{

           MCamera::GetIns()->GrabImageA(global::GetIns()->Image);
           Pos = MSerials::GetResult(global::GetIns()->Image,Mediator::GetIns()->MainWindowDispHd,ROW,COLUMN,ANGLE,LEN1,LEN2,SAMPLENUM,-1,
                                                  LINEOFFSET[0],LINEOFFSET[1],LINEOFFSET[2],LINEOFFSET[3]);
           SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
           WriteString(Mediator::GetIns()->MainWindowDispHd, Pos.SerialsInfo.c_str());
           Mediator::GetIns()->Sender(Pos.SerialsInfo.c_str());
        }catch(HalconCpp::HException except)
        {
               HTuple ExceptionMessage;
               except.ToHTuple(&ExceptionMessage);
               SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
               WriteString(Mediator::GetIns()->MainWindowDispHd, ExceptionMessage);
        }

       // Mediator::GetIns()->Sender(Mediator::GetIns()->ByArr);
        //同步接收
        //if(Mediator::GetIns()->p) Mediator::GetIns()->p(QString(Mediator::GetIns()->recv_data));

        Mediator::GetIns()->evt_PLCMessage.ResetEvent();
    }
}

Mediator::Mediator()
{


        BUniMap[Baund1200] = QSerialPort::Baud1200;
        BUniMap[Baund2400] = QSerialPort::Baud2400;
        BUniMap[Baund4800] = QSerialPort::Baud4800;
        BUniMap[Baund9600] = QSerialPort::Baud9600;
        BUniMap[Baund19200] = QSerialPort::Baud19200;
        BUniMap[Baund38400] = QSerialPort::Baud38400;
        BUniMap[Baund57600] = QSerialPort::Baud57600;
        BUniMap[Baund115200] = QSerialPort::Baud115200;

        PUniMap[PARNONE] = QSerialPort::NoParity;
        PUniMap[PARODD] = QSerialPort::OddParity;
        PUniMap[PAREVEN] = QSerialPort::EvenParity;

        DUniMap[DATA7] = QSerialPort::Data7;
        DUniMap[DATA8] = QSerialPort::Data8;

        SUniMap[STOP1] = QSerialPort::OneStop;
        SUniMap[STOP1D5] = QSerialPort::OneAndHalfStop;
        SUniMap[STOP2] = QSerialPort::TwoStop;

        serials = new Serials();
        (HANDLE)_beginthreadex(NULL, 0,PLCMessageThread, this, 0, NULL);
        connect(serials,&QSerialPort::readyRead,[=](){recvMsg();});
    }
