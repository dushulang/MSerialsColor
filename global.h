#ifndef GLOBAL_H
#define GLOBAL_H

#include "preference.h"
#include "excv.h"
#include <functional>
#include <QString>
#include <map>
#include <iostream>
#include "vardefine.h"
#define INITING     (1)
#define NOCAMERA    (2)
#define NORS232     (4)
#define SOCKETFAIL  (8)

#define sleep(ms)	std::this_thread::sleep_for(std::chrono::milliseconds(ms))

//typedef void ( * Display)(QString);




namespace MSerials {

    //功能，高低字节交换，如果是奇数，则填充最高位0 位
    static void SwapLowAndHigh(std::string &Input,std::string &Output, bool FillZero = true)
    {
        using namespace std;
        std::string High,Low;
        int Len = static_cast<int>(Input.length());
        if(0 == Len) {Output.clear(); return;}
        if(1 == (Len%2)){
            if(FillZero){
                Output = "0" + Input;
            }else {
                Output = "1" + Input;
            }
            int HalfLen = (1+Len)/2;


            High = Output.substr(0,size_t(HalfLen));
            Low = Output.substr(size_t(HalfLen));
            Output = Low + High;
        }
        else {
            Output = Input;
            int HalfLen = Len/2;
            High = Output.substr(0,size_t(HalfLen));
            Low = Output.substr(size_t(HalfLen));
            Output = Low + High;
        }
    }

};


struct PositionInfo{
    PositionInfo(){
        Points = std::vector<cv::Point2d>(MAX_VOL);
        Angles = std::vector<double>(MAX_VOL);
        SerialsInfo = "Error\r\n";
    }
    std::vector<double> Angles;
    std::vector<cv::Point2d> Points;
    std::string SerialsInfo;

    double H = 0.0;
    double S = 0.0;
    double V = 0.0;
    //方差
    double gama = 0.0;





};


/*
class MEvent
{
private:
    std::mutex mtx;
    HANDLE evt;
    int state;
public:
    MEvent():
        state(0)
    {
        evt = CreateEventW(FALSE,TRUE,FALSE,NULL);
        ::ResetEvent(evt);
    }
    const HANDLE &get(){return evt;}
    int SetEvent() {std::lock_guard<std::mutex> lck(mtx);::SetEvent(evt);return (state = 1);}
    int ResetEvent(){std::lock_guard<std::mutex> lck(mtx);::ResetEvent(evt);return (state = 0);}
    int &State() { return state; }
};
*/


class global{
    global(){MachieState = INITING;}
public:
    int MachieState = INITING;
    std::function<void(std::string)> UpdateHistoryll;
    static global *GetIns(){static global g; return &g;}

    MEvent evt_Trigger;




    typedef void (__stdcall *MsgBox)(std::string);
    typedef void (__stdcall *PushButton_)(void);
    MsgBox UpdateHistory = nullptr;
    void UpdateMessage(std::string msg){if(nullptr != UpdateHistory) UpdateHistory(msg);}   ///消息通知主界面
    HalconCpp::HObject Image;

   // Display p = nullptr;
};

#endif // GLOBAL_H
