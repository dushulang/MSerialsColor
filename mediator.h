#ifndef MEDIATOR_H
#define MEDIATOR_H

#include <QObject>
#include <QString>
#include <iostream>
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>
#include <unordered_map>
#include "vardefine.h"
#include "serials.h"
#include <mutex>
#include <chrono>
#include <process.h>
#include "HalconCpp.h"
#include "global.h"
#ifndef __APPLE__
#  include "HalconCpp.h"
#  include "HDevThread.h"
#  if defined(__linux__) && !defined(__arm__) && !defined(NO_EXPORT_APP_MAIN)
#    include <X11/Xlib.h>
#  endif
#else
#  ifndef HC_LARGE_IMAGES
#    include <HALCONCpp/HalconCpp.h>
#    include <HALCONCpp/HDevThread.h>
#  else
#    include <HALCONCppxl/HalconCpp.h>
#    include <HALCONCppxl/HDevThread.h>
#  endif
#  include <stdio.h>
#  include <HALCON/HpThread.h>
#  include <CoreFoundation/CFRunLoop.h>
#endif
using namespace HalconCpp;


#define QUERY "QUERY"

class Mediator;

bool InitCamera();

void GrabImageA(HalconCpp::HObject &ho_Image);



class Mediator:public QObject

{
    char StrNak[3] = {NAK,0x0,0x0};
    char StrAck[3] = {ACK,0x0,0x0};
    char StrStx[3] = {STX,0x0,0x0};
    char StrEtx[3] = {ETX,0x0,0x0};
    char StrEot[3] = {EOT,0x0,0x0};
    char StrEnq[3] = {ENQ,0x0,0x0};

    Q_OBJECT
    Serials *serials = nullptr;
    clock_t tick_get_data = clock();
    Mediator();
    bool isAnyPortOpened = false;
    std::unordered_map<std::string,int> CUniMap,BUniMap,PUniMap,DUniMap,SUniMap;
    HTuple  hv_AcqHandle;

public slots:
    void Read_Data()
    {
        serials->readAll();
    }
public:

    typedef void (__stdcall *MsgBox)(std::string);
    MsgBox UpdateHistory = nullptr;
    void UpdateMessage(std::string msg){if(nullptr != UpdateHistory) UpdateHistory(msg);}   ///消息通知主界面

    Serials *GetSerials(){
        return serials;
    }
    cv::Mat m_cameraMatrix;
    cv::Mat m_distCoeffs;

    HalconCpp::HTuple MainWindowDispHd = NULL, DialogSetDispHd = NULL;

    QByteArray ByArr;
    QByteArray ba,recv_data,send_data;
    MEvent evt_PLCMessage,evt_Write, evt_Read;
    QString objectName() const{ return "Mediator";}
    static Mediator *GetIns(){static Mediator m; return &m;}
    bool isOpened() { isAnyPortOpened = serials->isOpen();  return isAnyPortOpened;}
    bool openPort(QString com, QString baund, QString parity, QString data, QString stop){
       serials->setPortName(com.toLocal8Bit().data());

       int baunde = BUniMap[baund.toLocal8Bit().data()];
       QSerialPort::Parity Parity = static_cast<QSerialPort::Parity>(PUniMap[parity.toLocal8Bit().data()]);
               static_cast<QSerialPort::DataBits>(DUniMap[data.toLocal8Bit().data()]);
       static_cast<QSerialPort::StopBits>(SUniMap[stop.toLocal8Bit().data()]);

        std::cout << "com is " << com.toLocal8Bit().data()<<" "<<baunde <<" p " << Parity<<std::endl;
       serials->setBaudRate(BUniMap[baund.toLocal8Bit().data()]);
       serials->setParity(static_cast<QSerialPort::Parity>(PUniMap[parity.toLocal8Bit().data()]));
       serials->setDataBits(static_cast<QSerialPort::DataBits>(DUniMap[data.toLocal8Bit().data()]));
       serials->setStopBits(static_cast<QSerialPort::StopBits>(SUniMap[stop.toLocal8Bit().data()]));
       serials->setFlowControl(QSerialPort::NoFlowControl);
       isAnyPortOpened = serials->open(QIODevice::ReadWrite);
       printf("is Opened %d\n",isAnyPortOpened);
       return (isAnyPortOpened);
    }

    bool closePort(){
        serials->close();
        isAnyPortOpened = false;
        return true;
    }


    QByteArray Sender(QByteArray Barr, clock_t WaitTime = 1){
        if(!isAnyPortOpened)
        {
            return QByteArray();
        }
        static std::mutex mtx;
        std::lock_guard<std::mutex> lck(mtx);
        recv_data.clear();
        serials->write(Barr);
        return recv_data;
    }

    const char* CheckSum(const char *data ,int len)
    {
        unsigned char ret = 0;
        for(int i = 0;i<len;i++){
            ret += (unsigned char)(*(data+i));
        }
        static char ch[2] = {0,0};
        ch[0] = ~ret;
        return ch;
    }

    void WriteInput(){

    }

    bool CanWriteRead()
    {
        return (!evt_PLCMessage.State() && !evt_Read.State());
    }

    //宽字符
    QByteArray ReadOutputW(std::string WhichPLC,std::string Addr, std::string HowManyAddr=std::string("01"),  const char* WaitData = "1"){
        //半双工的缺点，用延时判断是否接收完毕
        //含义 见说明书D50 查询
        //报文等待时间
        if(evt_PLCMessage.State()) return ByArr;
        std::string Msg = StrEnq + WhichPLC + "FF"+"WR"+ WaitData + Addr + HowManyAddr + "\n";// + StrEot;
        std::string willSend = Msg;
        ByArr = QByteArray(willSend.c_str());
        Sender(ByArr);
        return ByArr;

        //return Sender(ByArr, it);
    }

    QByteArray ReadOutputB(std::string WhichPLC,std::string Addr, std::string HowManyAddr=std::string("01"), const char* WaitData = "1"){
        //半双工的缺点，用延时判断是否接收完毕
        //含义 见说明书D50 查询
        if(evt_PLCMessage.State()) return ByArr;
        std::string Msg = StrEnq + WhichPLC + "FF"+"BR"+WaitData + Addr + HowManyAddr + "\n";// + StrEot;
        int Len = strlen(Msg.c_str());
        std::cout<<Msg << std::endl;
        ByArr = QByteArray(Msg.c_str());
        Sender(ByArr);
        return ByArr;
    }


    bool WriteOutputW(std::string WhichPLC,std::string Addr, std::string HowManyAddr=std::string("01"), std::string 数据 = "0", const char* WaitData = "3"){
        //半双工的缺点，用延时判断是否接收完毕
        //含义 见说明书D50 查询
        if(evt_PLCMessage.State()) return false;
        std::string Msg = StrEnq + WhichPLC + "FF"+"WW"+ WaitData + Addr + HowManyAddr + 数据 +"\n";// + StrEot;
        //int Len = strlen(Msg.c_str());
       // std::cout<<Msg << std::endl;
        ByArr = QByteArray(Msg.c_str());
        Sender(ByArr);
        return true;
    }

    bool WriteOutputB(std::string WhichPLC,std::string Addr, std::string HowManyAddr=std::string("01"),std::string 数据 = "0", const char* WaitData = "3"){
        //半双工的缺点，用延时判断是否接收完毕
        //含义 见说明书D50 查询
        if(evt_PLCMessage.State()) return false;
        std::string Msg = StrEnq + WhichPLC + "FF"+"BW"+WaitData + Addr + HowManyAddr+ 数据 + "\n";// + StrEot;
        int Len = strlen(Msg.c_str());
        std::cout<<Msg << std::endl;
        ByArr = QByteArray(Msg.c_str());
        Sender(ByArr);
        return true;
    }

    bool RunPLC(std::string WhichPLC = "00",std::string WaitTime = "1"){
        if(evt_PLCMessage.State()) return false;
        std::string Msg = StrEnq + WhichPLC + "FF" + "RR" + WaitTime + "\n";
        std::cout<<Msg <<std::endl;
        for(auto ch :Msg)
        {
            printf("%x ",ch);
        }
        ByArr = QByteArray(Msg.c_str());
        Sender(ByArr);
        //if(Mediator::GetIns()->p) Mediator::GetIns()->p(QString(Mediator::GetIns()->recv_data));
        return true;
    }

    bool StopPLC(std::string WhichPLC = "00",std::string WaitTime = "1"){
        if(evt_PLCMessage.State()) return false;
        std::string Msg = StrEnq + WhichPLC + "FF" + "RS" +WaitTime + "\n";
        std::cout<<Msg <<std::endl;
        ByArr = QByteArray(Msg.c_str());
        Sender(ByArr);
        //if(Mediator::GetIns()->p) Mediator::GetIns()->p(QString(Mediator::GetIns()->recv_data));
        return true;
    }

    bool GetPLCInfo(std::string WhichPLC = "00", std::string WaitTime = "1"){
        if(evt_PLCMessage.State()) return false;
        std::string Msg = StrEnq + WhichPLC + "FF" + "PC" + WaitTime + "\n";
        std::cout<<Msg <<std::endl;
        for(auto ch :Msg)
        {
           printf("%02x ",ch);
        }
        ByArr = QByteArray(Msg.c_str());
        Sender(ByArr);
        //if(Mediator::GetIns()->p) Mediator::GetIns()->p(QString(Mediator::GetIns()->recv_data));
        return true;
    }

    void sendMsg(QString Str){
        return;
        if(!isAnyPortOpened) return;

        serials->write(Str.toLocal8Bit());
        clock_t tick = clock();
    }


    void PhaseMsg(QString str)
    {
        //响应数据并进行处理
        static QString RecvStr;
        RecvStr+=str;

        bool isToPhase = false;
        for(auto c:str)
        {
            if (c == '\r' || c == '\n'){
                isToPhase = true;
                break;
            }
        }

        if(isToPhase)
        {
            std::string strs(RecvStr.toLocal8Bit().data());
            RecvStr.clear();
            int len = strs.length();
            char Query[] = QUERY;
            int len2 = strlen(QUERY);
            if(len >= len2)
            {
                bool isEqual = true;
                for(int i=0;i<len2;i++)
                {
                    if(strs[i]!=Query[i])
                    {
                        isEqual = false;
                        break;
                    }
                }
                if(isEqual)
                {
                    evt_PLCMessage.SetEvent();
                }
            }
            //QUERY\r\n

        }

    }

    void recvMsg(){
       ba = serials->readAll();
       PhaseMsg(QString(ba));
       //recv_data += ba;
       //tick_get_data = clock();

       //if(p) p(QString(ba));
    }



};

#endif // MEDIATOR_H
