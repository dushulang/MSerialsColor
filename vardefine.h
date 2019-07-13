#ifndef VARDEFINE_H
#define VARDEFINE_H

#include <Windows.h>
#include <mutex>
#include <QString>
#include "preference.h"
#include "DXGI.h"

#define STR_COM         Preference::GetIns()->prj->COM
#define STR_BAUND       Preference::GetIns()->prj->BAUND
#define STR_PARITY      Preference::GetIns()->prj->PARITY
#define STR_DATA        Preference::GetIns()->prj->DATA
#define STR_STOP        Preference::GetIns()->prj->STOP

//文本开始
#define STX             0x2
//文本结束
#define ETX             0x3
//发送结束
#define EOT             0x4
//查询
#define ENQ             0x5
//肯定响应
#define ACK             0x6
//换行
#define LF              0xA
//清除后，PLC不会发任何数据
#define CL              0xC
//回车
#define CR              0xD
//否定相应
#define NAK             0x15


//默认站号
#define ADD_PLC         "00"
//加速地址软件元
#define ADD_ACC         "X0001"
//速度软件元
#define ADD_DOTSPD      "D0200"
#define ADD_SPD         "D0202"
#define ADD_RELATIVE    "D0220"

#define ADD_CURRENT_POS "D0000"
//移动距离
#define ADD_MOVE_PULS   "X0006"
#define ADD_MOVING_DES  "X0007"
#define ADD_MOVING_L    "M0000"
#define ADD_MOVING_R    "M0001"




class MEvent
{
private:
    std::mutex mtx;
    HANDLE evt;
    int _state;
public:
    MEvent():
        _state(0)
    {
        evt = CreateEventW(FALSE,TRUE,FALSE,NULL);
        ::ResetEvent(evt);
    }
    const HANDLE &get(){return evt;}
    int SetEvent() {std::lock_guard<std::mutex> lck(mtx);::SetEvent(evt);return (_state = 1);}
    int ResetEvent(){std::lock_guard<std::mutex> lck(mtx);::ResetEvent(evt);return (_state = 0);}
    int &State() { return _state; }
};



#endif // VARDEFINE_H
