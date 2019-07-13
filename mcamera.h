#ifndef MCAMERA_H
#define MCAMERA_H

#include "HalconCpp.h"
#include "process.h"
#include "global.h"
using namespace HalconCpp;



class MCamera
{
    MCamera(){
       // Init();
    }
    ~MCamera(){

    }
public:
    static MCamera *GetIns(){static MCamera g; return &g;}

    void Init()
    {
        (HANDLE)_beginthreadex(NULL, 0,InitCamera_, this, 0, NULL);

    }

    static unsigned int __stdcall InitCamera_(void *pLvoid)
    {
        Sleep(100);
        printf_s("init value is %d\n",global::GetIns()->MachieState);
        try{
            bool isOK = MCamera::GetIns()->InitCamera();
            if(isOK)
            {

                printf_s("成功");
                global::GetIns()->MachieState &= ~(NOCAMERA|INITING);
            }
            else
            {
                global::GetIns()->UpdateMessage("相机初始化失败");
                global::GetIns()->MachieState |= (NOCAMERA);
                global::GetIns()->MachieState &= ~(INITING);
                printf_s("失败");
            }

            }catch(HalconCpp::HException ex)
        {
           global::GetIns()->MachieState |= (NOCAMERA);
           global::GetIns()->MachieState &= ~(INITING);
            printf_s("失败2");
        }

        printf_s("value is %d\n",global::GetIns()->MachieState);
        return 0;

    }

    HTuple hv_AcqHandle;
    bool InitCamera(){
        using namespace HalconCpp;
        try{


        printf_s("init start\n");
        OpenFramegrabber("GigEVision2", 0, 0, 0, 0, 0, 0, "progressive", -1, "default",
            -1, "false", "default", "default", 0, -1, &hv_AcqHandle);
        printf_s("init camera success\n");

        SetFramegrabberParam(hv_AcqHandle, "AcquisitionMode", "Continuous");
        SetFramegrabberParam(hv_AcqHandle, "TriggerMode", "Off");
        SetFramegrabberParam(hv_AcqHandle, "TriggerSource", "Software");

        GrabImageStart(hv_AcqHandle, -1);


        printf_s("init over\n");
        return true;
        }catch(HalconCpp::HException ex)
        {
            throw ex;
        }
    }

    void GrabImageA(HalconCpp::HObject &ho_Image, HTuple MaxDelay = 2000){
        static std::mutex mtx;
        std::lock_guard<std::mutex> lck(mtx);
        try{
            using namespace HalconCpp;
            //GrabImageStart(hv_AcqHandle, 20000);
            //GrabImage(&ho_Image,hv_AcqHandle);
            GrabImageAsync(&ho_Image,hv_AcqHandle,MaxDelay);

        }catch(HalconCpp::HException except)
        {
            throw except;
        }
    }
};

#endif // MCAMERA_H
