#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"
#include "mediator.h"
#include "QSerialPortInfo"
#include <QtSerialPort/qserialport>
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include "HalconCpp.h"
#include "QDir"
#include "dialogsetting.h"
#include <thread>
#include <QMessageBox>
#include "process.h"
#include "mcamera.h"
#include "QTImer"
#include <QFileDialog>
#include "imagedeal.h"
#include "MSerialsCV.h"
#include "../PLC_DLL/PLC_DLL/plc_dll.h"
#include <exception>





#define PLCRESULT   7032
#define PLCTRI      0x7030
#define PLCTRI1     7030

using namespace  HalconCpp;

MainWindow * pMainWin = nullptr;

bool isReadPos = false;
QString RecvStr;
char StrEtx[3] = {ETX,0x0,0x0};

std::string ToHex(long Data)
{
    char iHex[256] = {0};
    char Hex[9] = {0};
    sprintf(iHex,"%08X",Data);
    for(int i = 0;i < 4;i++)
    {
        Hex[4+i] = iHex[i];
        Hex[i] = iHex[4+i];
    }
    printf("%s \n",Hex);
    return std::string(Hex);
}


unsigned int __stdcall PLC_Connect(void * pThis)
{
 //   MainWindow *pDlg = static_cast<MainWindow*>(pThis);
    bool isOK = false;
    sleep(1000);
    while (true) {
        sleep(5);
        if(!isOK)
        {
            sleep(2000);
            try{
            isOK = SocketInit("192.168.0.1",4999);
            }catch(std::exception ex)
            {
                printf_s(ex.what());
                isOK = false;
            }
            if(!isOK){
                 printf_s("连接失败，重连中...\n");
                 global::GetIns()->MachieState |= SOCKETFAIL;
                 SocketClear();
            }
            else
            {
                global::GetIns()->MachieState &= ~SOCKETFAIL;
                printf_s("连接成功！\n");
            }
        }
        else
        {
          try{
              clock_t clk = clock();

              std::string ToReadPos = PLCReadStrEX(GetHostByIndex(0),"D",PLCTRI,1);
              static std::string ToReadPosOld = ToReadPos;
              std::string ReadRes = PLCReadStrEX(GetHostByIndex(0),"D",0x401,1);
              int Np = std::atoi(ToReadPos.c_str());
              int Op = std::atoi(ToReadPosOld.c_str());
              if(Np && Np!=Op){
                  printf_s("进行触发\n");
                  if(!global::GetIns()->evt_Trigger.State()) global::GetIns()->evt_Trigger.SetEvent();
              }

              ToReadPosOld = ToReadPos;


              clock_t endClk = clock();
                //printf_s("超时时间位%zd 返回值为%s\n",endClk-clk,ReadRes.c_str());
                if(ReadRes == "-1"||(endClk-clk) > 500){
                    SocketClear();
                    isOK = false;

                }


            }catch(std::exception ex)
            {
                 printf_s("没有能够连接到PLC\n");
                 isOK = false;
            }
        }

    }
    return 0;

}

unsigned int __stdcall AnsycToAband(void *pThis){
    HObject AbandImage;
    for(;;){
        try{
        MCamera::GetIns()->GrabImageA(AbandImage);
        sleep(5);
        }catch(HalconCpp::HException ex){

        }
    }
}


unsigned int __stdcall ImageDeal(void * pThis)
{
    std::vector<PositionInfo> InfoMations;
    std::vector<double> ColorValue;
    int Idx = 0;
    for(;;){
        ::WaitForSingleObject(global::GetIns()->evt_Trigger.get(),INFINITE);
        using namespace  HalconCpp;
        try{
            ColorValue.clear();
            InfoMations.clear();
            HObject ROI;
            bool isOK = true;
            SetColor(Mediator::GetIns()->MainWindowDispHd,"cyan");
            SetDraw(Mediator::GetIns()->MainWindowDispHd,"margin");
            GenRectangle2(&ROI,ROW[Idx],COLUMN[Idx],ANGLE[Idx],LEN1[Idx],LEN2[Idx]);
            DispObj(ROI,Mediator::GetIns()->MainWindowDispHd);

            clock_t clk = clock();
            int i = 0;
            for(;;){
                if(i >= SAMPLENUM)
                    break;
                if((clock()-clk) > i*SNAP_INTERVAL)
                {
                    MCamera::GetIns()->GrabImageA(global::GetIns()->Image);
                    Excv::h_disp_obj(global::GetIns()->Image,Mediator::GetIns()->MainWindowDispHd);
                    PositionInfo Pos
                        = MSerials::GetResult(global::GetIns()->Image,Mediator::GetIns()->MainWindowDispHd,ROW,COLUMN,ANGLE,LEN1,LEN2,SAMPLENUM,-1,
                                                       LINEOFFSET[0],LINEOFFSET[1],LINEOFFSET[2],LINEOFFSET[3]);
                    InfoMations.push_back(Pos);
                    ColorValue.push_back(Pos.V);
                    i++;
                }else
                {
                sleep(1);
                }
            }

            DispObj(ROI,Mediator::GetIns()->MainWindowDispHd);

            double Sigma = 0.0;
            int Gap = 22;
            if(!InfoMations.empty())
            {
            double MaxV = *std::max_element(ColorValue.begin(),ColorValue.end());
            double MinV = *std::min_element(ColorValue.begin(),ColorValue.end());
            Sigma = MaxV-MinV;
            }
            int Lines = 0;
            double Average = 0.0;
            for(Lines =0;Lines < InfoMations.size();Lines++){
                SetTposition(Mediator::GetIns()->MainWindowDispHd, (1+Lines)*(Gap), 1);
                int ColorClass = MSerials::FindWhichClass(InfoMations.at(Lines).V);
                int isColorOK = ColorClass;
                if(ColorClass >= USER_MAX_SIZE_OF_ID){
                    isColorOK = 0;
                }

                Average += InfoMations.at(Lines).V;
                char Msg[256] = {0};
                 sprintf_s(Msg,"颜色值%5.2f",InfoMations.at(Lines).V);

                if(isColorOK != 0){
                    SetColor(Mediator::GetIns()->MainWindowDispHd,"green");
                }
                else{
                    isOK = false;
                    SetColor(Mediator::GetIns()->MainWindowDispHd,"red");
                }

                WriteString(Mediator::GetIns()->MainWindowDispHd, Msg);

            }

            Lines++;
            SetTposition(Mediator::GetIns()->MainWindowDispHd, Lines*Gap, 1);

            int Size = InfoMations.size();
            if(Size >0){
                Average = Average/(1.0*Size);
            }

            std::string Reason = "";
            int ColorClass_ = MSerials::FindWhichClass(Average);
            int isColorOK_ = ColorClass_;
            if(0 == ColorClass_){
                Reason += "颜色过深 ";
            }
            if(ColorClass_ >= USER_MAX_SIZE_OF_ID){
                isColorOK_ = 0;
                Reason += "颜色过浅 ";
            }
            if(Sigma > COLOR_SIGMA)
            {
                Reason += "同一板子色差过大 ";
                isColorOK_ = 0;
            }


            char Msg[256] = {0};
            sprintf_s(Msg,"颜色平均值%5.2f，分色档次为%d, 色差最大最小值为%5.2f",Average,isColorOK_,Sigma);


            if(isColorOK_ != 0){
                SetColor(Mediator::GetIns()->MainWindowDispHd,"green");
            }
            else{

                SetColor(Mediator::GetIns()->MainWindowDispHd,"red");
            }

            WriteString(Mediator::GetIns()->MainWindowDispHd, Msg);


            Lines++;
            SetTposition(Mediator::GetIns()->MainWindowDispHd, (1+Lines)*(Gap), 1);
            if(Reason.length() > 0)
            WriteString(Mediator::GetIns()->MainWindowDispHd, Reason.c_str());

            std::cout <<"检测结果为" << isColorOK_<<std::endl;



            int Value = static_cast<short>(isColorOK_);
            char Tmp[256] = {0};
            sprintf_s(Tmp,"%04X",Value);
            std::string WillWrite,TmpStr = std::string(Tmp);
            MSerials::SwapLowAndHigh(TmpStr,WillWrite);
            PLCWriteBitEX(GetHostByIndex(0),"D",PLCRESULT,0x1,std::string(Tmp).c_str());
            PLCWriteBitEX(GetHostByIndex(0),"D",PLCTRI1,0x1,"0000");











            /*

           MCamera::GetIns()->GrabImageA(global::GetIns()->Image);
           Excv::h_disp_obj(global::GetIns()->Image,Mediator::GetIns()->MainWindowDispHd);
           PositionInfo Pos = MSerials::GetResult(global::GetIns()->Image,Mediator::GetIns()->MainWindowDispHd,ROW,COLUMN,ANGLE,LEN1,LEN2,SAMPLENUM,-1,
                                                  LINEOFFSET[0],LINEOFFSET[1],LINEOFFSET[2],LINEOFFSET[3]);
           SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
           WriteString(Mediator::GetIns()->MainWindowDispHd, Pos.SerialsInfo.c_str());

           std::string WillWrite;
           char Tmp[256] = {0};
           std::string TmpStr;
           float Value = static_cast<float>(Pos.Points.at(0).x);
           int32_t Var = *(int32_t*)&Value;
           sprintf_s(Tmp,"%08X",Var);
           TmpStr = std::string(Tmp);
           MSerials::SwapLowAndHigh(TmpStr,WillWrite);
           PLCWriteBitEX(GetHostByIndex(0),"D",300,0x2,WillWrite.c_str());

           Value = static_cast<float>(Pos.Points.at(0).y);
           Var = *(int32_t*)&Value;
           sprintf_s(Tmp,"%08X",Var);
           TmpStr = std::string(Tmp);
           MSerials::SwapLowAndHigh(TmpStr,WillWrite);
           PLCWriteBitEX(GetHostByIndex(0),"D",302,0x2,WillWrite.c_str());


           Value = static_cast<float>(Pos.Points.at(1).x);
           Var = *(int32_t*)&Value;
           sprintf_s(Tmp,"%08X",Var);
           TmpStr = std::string(Tmp);
           MSerials::SwapLowAndHigh(TmpStr,WillWrite);
           PLCWriteBitEX(GetHostByIndex(0),"D",304,0x2,WillWrite.c_str());


           Value = static_cast<float>(Pos.Points.at(1).y);
           Var = *(int32_t*)&Value;
           sprintf_s(Tmp,"%08X",Var);
           TmpStr = std::string(Tmp);
           MSerials::SwapLowAndHigh(TmpStr,WillWrite);
           PLCWriteBitEX(GetHostByIndex(0),"D",306,0x2,WillWrite.c_str());


           Value = static_cast<float>(Pos.Points.at(2).x);
           Var = *(int32_t*)&Value;
           sprintf_s(Tmp,"%08X",Var);
           TmpStr = std::string(Tmp);
           MSerials::SwapLowAndHigh(TmpStr,WillWrite);
           PLCWriteBitEX(GetHostByIndex(0),"D",308,0x2,WillWrite.c_str());


           Value = static_cast<float>(Pos.Points.at(2).y);
           Var = *(int32_t*)&Value;
           sprintf_s(Tmp,"%08X",Var);
           TmpStr = std::string(Tmp);
           MSerials::SwapLowAndHigh(TmpStr,WillWrite);
           PLCWriteBitEX(GetHostByIndex(0),"D",310,0x2,WillWrite.c_str());


           Value = static_cast<float>(Pos.Points.at(3).x);
           Var = *(int32_t*)&Value;
           sprintf_s(Tmp,"%08X",Var);
           TmpStr = std::string(Tmp);
           MSerials::SwapLowAndHigh(TmpStr,WillWrite);
           PLCWriteBitEX(GetHostByIndex(0),"D",312,0x2,WillWrite.c_str());


           Value = static_cast<float>(Pos.Points.at(3).y);
           Var = *(int32_t*)&Value;
           sprintf_s(Tmp,"%08X",Var);
           TmpStr = std::string(Tmp);
           MSerials::SwapLowAndHigh(TmpStr,WillWrite);
           PLCWriteBitEX(GetHostByIndex(0),"D",314,0x2,WillWrite.c_str());


           Value = static_cast<float>(Pos.Angles.at(0));
           Var = *(int32_t*)&Value;
           sprintf_s(Tmp,"%08X",Var);
           TmpStr = std::string(Tmp);
           MSerials::SwapLowAndHigh(TmpStr,WillWrite);
           PLCWriteBitEX(GetHostByIndex(0),"D",316,0x2,WillWrite.c_str());

           */

        }catch(HalconCpp::HException except)
        {
               HTuple ExceptionMessage;
               except.ToHTuple(&ExceptionMessage);
               SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
               WriteString(Mediator::GetIns()->MainWindowDispHd, ExceptionMessage);
        }catch(std::out_of_range except)
        {
            SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
            WriteString(Mediator::GetIns()->MainWindowDispHd, except.what());
        }catch(cv::Exception except){
            SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
            WriteString(Mediator::GetIns()->MainWindowDispHd, except.what());
        }
        catch(std::exception except){
                    SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
                    WriteString(Mediator::GetIns()->MainWindowDispHd, except.what());
       }



        global::GetIns()->evt_Trigger.ResetEvent();
    }
}

void MainWindow::UpdateHistoryInfo(std::string Info){
    if(pMainWin)
        pMainWin->ShowMsg(Info);
}

void MainWindow::ShowMsg(std::string str)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lck(mtx);
    static std::list<std::string> Content;
    Content.push_back(str);
    std::string str_ex;
    for(auto &s:Content)
    {
        str_ex += s + "\r\n";
    }
    ui->textEdit->setText(QString::fromLocal8Bit(str_ex.c_str()));
}

void MainWindow::Init(const char *program_name){
    MCamera::GetIns()->Init();
}




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pMainWin = this;
    //防止程序多开
    HANDLE m_hMutex  =  ::CreateMutexW(NULL, FALSE,  L"PUNCH_____2019.6.8" );
    //  检查错误代码
    if  (GetLastError()  ==  ERROR_ALREADY_EXISTS)  {
     QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("请勿多开程序!如果频繁出现，请重启电脑"));
     CloseHandle(m_hMutex);
     m_hMutex  =  NULL;
     exit(0);
    }



    QDir dir;
    if(!dir.exists(CABLI)) dir.mkdir(CABLI);

    setWindowTitle(QString::fromLocal8Bit("FuryAI出品"));


    //首先读取配置文件
    Preference::GetIns()->prj->SetFilePos(QString("Prj.ini"));
    int nWndWidth = ui->widget->width();
    int nWndHeight= ui->widget->height();
    HalconCpp::SetCheck("~father");
    HalconCpp::OpenWindow(0, 0, nWndWidth, nWndHeight, (Hlong)ui->widget->winId(), "visible", "", &    Mediator::GetIns()->MainWindowDispHd);
    HalconCpp::SetCheck("father");
    Mediator::GetIns()->UpdateHistory = UpdateHistoryInfo;
  //  showFullScreen();

    QStringList List;
    QList<QSerialPortInfo> Infos = QSerialPortInfo::availablePorts();
    QString SelPORT;
    bool isIn = false;
    for(auto PortInfo : Infos)
    {
        List << PortInfo.portName();

        if(STR_COM==PortInfo.portName()){
            isIn = true;
        }
        if(SelPORT.length() < 2)
        {
            SelPORT = PortInfo.portName();
        }
    }
    if(!isIn && SelPORT.length()>1)
    {
        STR_COM = SelPORT;
    }

    ui->comboBox_COM->addItems(List);
    List.clear();

    List << Baund1200 << Baund2400 << Baund4800 <<Baund9600  << Baund19200 << Baund38400  << Baund57600  << Baund115200 ;
    ui->comboBox_BAUND->addItems(List);
    QComboBox *pCombox = ui->comboBox_BAUND;
    for(int i = 0;i < pCombox->count();i++)
    {
        if(pCombox->itemText(i) == STR_BAUND){
            pCombox->setCurrentIndex(i);
            break;
        }
    }

    List.clear();
    List <<DATA7 << DATA8;
    ui->comboBox_DATA->addItems(List);
    pCombox = ui->comboBox_DATA;
    for(int i = 0;i < pCombox->count();i++)
    {
        if(pCombox->itemText(i) == STR_DATA){
            pCombox->setCurrentIndex(i);
            break;
        }
    }

    List.clear();
    List <<PARNONE <<PARODD <<PAREVEN;
    ui->comboBox_PARITY->addItems(List);
    pCombox = ui->comboBox_PARITY;
    for(int i = 0;i < pCombox->count();i++)
    {
        if(pCombox->itemText(i) == STR_PARITY){
            pCombox->setCurrentIndex(i);
            break;
        }
    }

    List.clear();
    List <<STOP1  << STOP1D5  <<STOP2;
    ui->comboBox_STOP->addItems(List);
    pCombox = ui->comboBox_STOP;
    for(int i = 0;i < pCombox->count();i++)
    {
        if(pCombox->itemText(i) == STR_STOP){
            pCombox->setCurrentIndex(i);
            break;
        }
    }


    connect(ui->comboBox_COM,&QComboBox::currentTextChanged,[=](QString Str){
        std::cout << Str.toLocal8Bit().data()<<std::endl;
        STR_COM = Str;
    });
    connect(ui->comboBox_BAUND,&QComboBox::currentTextChanged,[=](QString Str){
        std::cout << Str.toLocal8Bit().data()<<std::endl;
        STR_BAUND = Str;
    });
    connect(ui->comboBox_DATA,&QComboBox::currentTextChanged,[=](QString Str){
        std::cout << Str.toLocal8Bit().data()<<std::endl;
        STR_DATA = Str;
    });
    connect(ui->comboBox_PARITY,&QComboBox::currentTextChanged,[=](QString Str){
        std::cout << Str.toLocal8Bit().data()<<std::endl;
        STR_PARITY = Str;
    });
    connect(ui->comboBox_STOP,&QComboBox::currentTextChanged,[=](QString Str){
        std::cout << Str.toLocal8Bit().data()<<std::endl;
        STR_STOP = Str;
    });

    if(Mediator::GetIns()->openPort(STR_COM,STR_BAUND,STR_PARITY,STR_DATA,STR_STOP))
    {
        Preference::GetIns()->prj->WriteSettings(-1);
        ui->label_isOpen->setText(QString::fromLocal8Bit("已打开"));
        ui->pushButton_StartSerials->setText(QString::fromLocal8Bit("关闭串口"));
    }
    else
    {
        ui->label_isOpen->setText(QString::fromLocal8Bit("未打开"));
        ui->pushButton_StartSerials->setText(QString::fromLocal8Bit("打开串口"));
    }

    connect(ui->pushButton_StartSerials,&QPushButton::clicked,[=](){
        if(!Mediator::GetIns()->isOpened())
        {
            if(Mediator::GetIns()->openPort(STR_COM,STR_BAUND,STR_PARITY,STR_DATA,STR_STOP))
            {
                Preference::GetIns()->prj->WriteSettings(-1);
                ui->label_isOpen->setText(QString::fromLocal8Bit("已打开"));
                ui->pushButton_StartSerials->setText(QString::fromLocal8Bit("关闭串口"));
            }
            else
            {
                ui->label_isOpen->setText(QString::fromLocal8Bit("未打开"));
                ui->pushButton_StartSerials->setText(QString::fromLocal8Bit("打开串口"));
            }
        }
        else
        {
            Mediator::GetIns()->closePort();
            ui->label_isOpen->setText(QString::fromLocal8Bit("未打开"));
            ui->pushButton_StartSerials->setText(QString::fromLocal8Bit("打开串口"));
        }


    });

   if(SAMPLENUM < 2) SAMPLENUM = 2;
   ui->lineEdit_SampleNum->setText(QString::number(SAMPLENUM));
   ui->lineEdit_Offset1->setText(QString::number(LINEOFFSET[0],'g',5));
   ui->lineEdit_Offset2->setText(QString::number(LINEOFFSET[1],'g',5));
   ui->lineEdit_Offset3->setText(QString::number(LINEOFFSET[2],'g',5));
   ui->lineEdit_Offset4->setText(QString::number(LINEOFFSET[3],'g',5));

   /*
   connect(ui->lineEdit_SampleNum,&QLineEdit::textChanged,[=](){
       SAMPLENUM = ui->lineEdit_SampleNum->text().toInt();
       if(SAMPLENUM < 2)
       {SAMPLENUM = 2;ui->lineEdit_SampleNum->setText(QString::number(SAMPLENUM));
       SAVEPRJPARA;
       }
       });
*/

   global::GetIns()->UpdateHistory = UpdateHistoryInfo;
   global::GetIns()->UpdateMessage("初始化...");


   using namespace cv;
   FileStorage fs2("Cabli.yml", FileStorage::READ);
   fs2["cameraMatrix"] >> Mediator::GetIns()->m_cameraMatrix;
   fs2["distCoeffs"] >> Mediator::GetIns()->m_distCoeffs;
   std::cout<<"旋转" << Mediator::GetIns()->m_cameraMatrix<<std::endl;
   std::cout<<"畸变" << Mediator::GetIns()->m_distCoeffs<<std::endl;


   //线程进行相机初始化
   QTimer *timer_io = new QTimer();
   //设置定时器每个多少毫秒发送一个timeout()信号
   timer_io->setInterval(1000);
   connect(timer_io, &QTimer::timeout, [&]() {
       static bool isShow = true;
       if(isShow && (INITING !=  (INITING&global::GetIns()->MachieState))){

           isShow = false;
           if(NOCAMERA == (NOCAMERA&global::GetIns()->MachieState)){
                global::GetIns()->UpdateMessage("相机初始化失败");
           }
           else
           {
               global::GetIns()->UpdateMessage("相机初始化成功");
           }
       }

       if(SOCKETFAIL == (SOCKETFAIL&global::GetIns()->MachieState))
       {
           global::GetIns()->UpdateMessage("PLC连接失败");
       }

   });
   timer_io->start();

#if 1
try{
   HalconCpp::HObject Hobj;
   HalconCpp::ReadImage(&Hobj,"1.bmp");
   Excv::h_disp_obj(Hobj,Mediator::GetIns()->MainWindowDispHd);
   }catch(std::exception ex)
   {
       SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
       WriteString(Mediator::GetIns()->MainWindowDispHd, ex.what());
   }catch(HalconCpp::HException ex)
   {
       HTuple ExceptionMessage;
       ex.ToHTuple(&ExceptionMessage);
       SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
       WriteString(Mediator::GetIns()->MainWindowDispHd, ExceptionMessage);
   }
#endif
   ui->widget->setStyleSheet("background-image:url(1.bmp)");



   LineEdits.push_back(ui->lineEdit_1);
   LineEdits.push_back(ui->lineEdit_2);
   LineEdits.push_back(ui->lineEdit_3);
   LineEdits.push_back(ui->lineEdit_4);
   LineEdits.push_back(ui->lineEdit_5);
   LineEdits.push_back(ui->lineEdit_6);
   LineEdits.push_back(ui->lineEdit_7);
   LineEdits.push_back(ui->lineEdit_8);
   LineEdits.push_back(ui->lineEdit_9);
   LineEdits .push_back(ui->lineEdit_10);

   Labels.push_back(ui->label_12);
   Labels.push_back(ui->label_13);
   Labels.push_back(ui->label_14);
   Labels.push_back(ui->label_15);
   Labels.push_back(ui->label_16);
   Labels.push_back(ui->label_17);
   Labels.push_back(ui->label_18);
   Labels.push_back(ui->label_19);
   Labels.push_back(ui->label_20);
   Labels.push_back(ui->label_21);








   connect(ui->lineEdit_1,&QLineEdit::textChanged,[=](){
       SIZEPARA[0] = ui->lineEdit_1->text().toDouble();
       if(SIZEPARA[0]>0){
       UpdateData();
       SAVEPRJPARA;
       }
   });
   connect(ui->lineEdit_2,&QLineEdit::textChanged,[=](){
       SIZEPARA[1] = ui->lineEdit_2->text().toDouble();
       if(SIZEPARA[1]>0){
       UpdateData();
       SAVEPRJPARA;
       }
   });
   connect(ui->lineEdit_3,&QLineEdit::textChanged,[=](){
       SIZEPARA[2] = ui->lineEdit_3->text().toDouble();
       if(SIZEPARA[2]>0){
       UpdateData();
       SAVEPRJPARA;
       }
   });
   connect(ui->lineEdit_4,&QLineEdit::textChanged,[=](){
       SIZEPARA[3] = ui->lineEdit_4->text().toDouble();
       if(SIZEPARA[3]>0){
       UpdateData();
       SAVEPRJPARA;
       }
   });
   connect(ui->lineEdit_5,&QLineEdit::textChanged,[=](){
       SIZEPARA[4] = ui->lineEdit_5->text().toDouble();
       if(SIZEPARA[4]>0){
       UpdateData();
       SAVEPRJPARA;
       }
   });
   connect(ui->lineEdit_6,&QLineEdit::textChanged,[=](){
       SIZEPARA[5] = ui->lineEdit_6->text().toDouble();
       if(SIZEPARA[5]>0){
       UpdateData();
       SAVEPRJPARA;
       }
   });
   connect(ui->lineEdit_7,&QLineEdit::textChanged,[=](){
       SIZEPARA[6] = ui->lineEdit_7->text().toDouble();
       if(SIZEPARA[6]>0){
       UpdateData();
       SAVEPRJPARA;
       }
   });
   connect(ui->lineEdit_8,&QLineEdit::textChanged,[=](){
       SIZEPARA[7] = ui->lineEdit_8->text().toDouble();
       if(SIZEPARA[7]>0){
       UpdateData();
       SAVEPRJPARA;
       }
   });
   connect(ui->lineEdit_9,&QLineEdit::textChanged,[=](){
       SIZEPARA[8] = ui->lineEdit_9->text().toDouble();
       if(SIZEPARA[8]>0){
       UpdateData();
       SAVEPRJPARA;
       }
   });
   connect(ui->lineEdit_10,&QLineEdit::textChanged,[=](){
       SIZEPARA[9] = ui->lineEdit_10->text().toDouble();
       if(SIZEPARA[9]>0){
       UpdateData();
       SAVEPRJPARA;
       }
   });

   ui->lineEdit_Interval->setText(QString::number(SNAP_INTERVAL));
   ui->lineEdit_Sigma->setText(QString::number(COLOR_SIGMA));
   ui->lineEdit_Divides->setText(QString::number(USER_MAX_SIZE_OF_ID));
   on_pushButton_Divides_clicked();
   try{
   for(int i = 0;i < USER_MAX_SIZE_OF_ID;i++){
        LineEdits.at(i)->setText(QString::number(SIZEPARA[i],'g',6));
   }
   }catch(std::out_of_range ex){

   }

   //不必要的控件进行隐藏
    ui->lineEdit_Offset1->setVisible(false);
    ui->lineEdit_Offset2->setVisible(false);
    ui->lineEdit_Offset3->setVisible(false);
    ui->lineEdit_Offset4->setVisible(false);

    ui->label_6->setVisible(false);
    ui->label_9->setVisible(false);
    ui->label_10->setVisible(false);
    ui->label_11->setVisible(false);

    ui->groupBox_3->setVisible(false);
    ui->pushButton_DrawROI2->setVisible(false);
    ui->pushButton_DrawROI3->setVisible(false);
    ui->pushButton_DrawROI4->setVisible(false);

    ui->pushButton_Close->setVisible(false);
    ui->pushButton_CabliSet->setVisible(false);
}

void MainWindow::OnInitDialog(){


  (HANDLE)_beginthreadex(NULL, 0, PLC_Connect, this, 0, NULL);
  (HANDLE)_beginthreadex(NULL, 0, ImageDeal, NULL, 0, NULL);
  //异步抓取图片，使得内存废弃以前图片
  (HANDLE)_beginthreadex(NULL, 0, AnsycToAband, NULL, 0, NULL);
}

void MainWindow::UpdateData()
{
    QString Text;
    for(int i=0;i<USER_MAX_SIZE_OF_ID;i++){

    }
//ui->label_DivideInfo

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_CabliSet_clicked()
{
    static DialogSetting *ModelSetDlg  = nullptr;
    if(nullptr != ModelSetDlg)
    {
        delete ModelSetDlg;
        ModelSetDlg = nullptr;
    }

    ModelSetDlg = new DialogSetting ();
    ModelSetDlg->show();
    ModelSetDlg->exec();
}


void MainWindow::on_pushButton_GrabAsync_clicked()
{
    using namespace  HalconCpp;
    try{
       MCamera::GetIns()->GrabImageA(global::GetIns()->Image);
       Excv::h_disp_obj(global::GetIns()->Image,Mediator::GetIns()->MainWindowDispHd);
    }catch(HalconCpp::HException except)
    {
           HTuple ExceptionMessage;
           except.ToHTuple(&ExceptionMessage);
           SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
           WriteString(Mediator::GetIns()->MainWindowDispHd, ExceptionMessage);
    }

}

void MainWindow::on_pushButton_ReadImage_clicked()
{
    using namespace  HalconCpp;
    QString fileName = QFileDialog::getOpenFileName(NULL,
        tr("Open Image"), "",
        tr("Image File (*.jpg)||Image File (*.bmp)||All Files (*)"));
    try{
       ReadImage(&global::GetIns()->Image,fileName.toLocal8Bit().data());
       Excv::h_disp_obj(global::GetIns()->Image,Mediator::GetIns()->MainWindowDispHd);

    }catch(HalconCpp::HException except)
    {
           HTuple ExceptionMessage;
           except.ToHTuple(&ExceptionMessage);
           SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
           WriteString(Mediator::GetIns()->MainWindowDispHd, ExceptionMessage);
    }


}


void DrawRectangle2Ex(int Idx = 0)
{
    try{
        HTuple hv_RowIn = 0,hv_ColumnIn = 0, hv_AngleIn = 0, hv_Len1In = 0,  hv_Len2In = 0,Row= 0,Col=0,Len2=0;
        HObject ROI;
        ISDRAW = 1;
        if (HDevWindowStack::IsOpen())
          SetDraw(HDevWindowStack::GetActive(),"margin");
        if (HDevWindowStack::IsOpen())
          SetColor(HDevWindowStack::GetActive(),"cyan");
        SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);

        SetColor(Mediator::GetIns()->MainWindowDispHd,"cyan");
        SetDraw(Mediator::GetIns()->MainWindowDispHd,"margin");

        WriteString(Mediator::GetIns()->MainWindowDispHd, "点击右键或者长按触摸屏幕等待十字消失确认");
        DrawRectangle2(Mediator::GetIns()->MainWindowDispHd, &hv_RowIn, &hv_ColumnIn, &hv_AngleIn, &hv_Len1In,
        &hv_Len2In);
        ROW[Idx] = hv_RowIn[0].D();
        COLUMN[Idx] = hv_ColumnIn[0].D();
        ANGLE[Idx] = hv_AngleIn[0].D();
        LEN1[Idx] = hv_Len1In[0].D();
        LEN2[Idx] = hv_Len2In[0].D();
        GenRectangle2(&ROI,ROW[Idx],COLUMN[Idx],ANGLE[Idx],LEN1[Idx],LEN2[Idx]);
        DispObj(ROI,Mediator::GetIns()->MainWindowDispHd);

        SAVEPRJPARA;
    }catch(HalconCpp::HException except)
    {
           HTuple ExceptionMessage;
           except.ToHTuple(&ExceptionMessage);
           SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
           WriteString(Mediator::GetIns()->MainWindowDispHd, ExceptionMessage);
    }
    ISDRAW = 0;
}

void MainWindow::on_pushButton_DrawROI1_clicked()
{
    if(ISDRAW) {
        return;
    }
    DrawRectangle2Ex(0);
}

void MainWindow::on_pushButton_DrawROI2_clicked()
{
    if(ISDRAW) {
        return;
    }
    DrawRectangle2Ex(1);
}

void MainWindow::on_pushButton_DrawROI3_clicked()
{
    if(ISDRAW) {
        return;
    }
    DrawRectangle2Ex(2);
}

void MainWindow::on_pushButton_DrawROI4_clicked()
{
    if(ISDRAW) {
        return;
    }
    DrawRectangle2Ex(3);
}

void MainWindow::on_pushButton_CabliTest_clicked()
{
    try{
        
        HObject ROI;

        DispObj(global::GetIns()->Image,Mediator::GetIns()->MainWindowDispHd);
        int Idx = 0;
        SetColor(Mediator::GetIns()->MainWindowDispHd,"cyan");
        SetDraw(Mediator::GetIns()->MainWindowDispHd,"margin");
        GenRectangle2(&ROI,ROW[Idx],COLUMN[Idx],ANGLE[Idx],LEN1[Idx],LEN2[Idx]);
        DispObj(ROI,Mediator::GetIns()->MainWindowDispHd);



        PositionInfo Pos
                = MSerials::GetResult(global::GetIns()->Image,Mediator::GetIns()->MainWindowDispHd,ROW,COLUMN,ANGLE,LEN1,LEN2,SAMPLENUM,-1,
                                               LINEOFFSET[0],LINEOFFSET[1],LINEOFFSET[2],LINEOFFSET[3]);
        SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
        int ColorClass = MSerials::FindWhichClass(Pos.V);
        int isColorOK = ColorClass;
        bool isOK = false;
        double  Sigma = Pos.gama;
        char Msg[256] = {0};
        if(ColorClass >= USER_MAX_SIZE_OF_ID){
            isColorOK = 0;
        }

        if(COLOR_SIGMA >= Sigma  ){
            isOK = false;
        }

        if(isOK){
            SetColor(Mediator::GetIns()->MainWindowDispHd,"green");
        }
        else{
           SetColor(Mediator::GetIns()->MainWindowDispHd,"red");
        }
        sprintf_s(Msg,"颜色值%5.2f,色差分类为%d,色差值%5.2f",Pos.V,ColorClass,Sigma);
        printf_s(Msg);
        WriteString(Mediator::GetIns()->MainWindowDispHd, Msg);


    }catch(HalconCpp::HException except)
    {
           HTuple ExceptionMessage;
           except.ToHTuple(&ExceptionMessage);
           SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
           WriteString(Mediator::GetIns()->MainWindowDispHd, ExceptionMessage);
    }catch(std::out_of_range except)
    {
        SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
        WriteString(Mediator::GetIns()->MainWindowDispHd, except.what());
    }catch(cv::Exception except){
        SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
        WriteString(Mediator::GetIns()->MainWindowDispHd, except.what());
    }
    catch(std::exception ex){
        SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
        WriteString(Mediator::GetIns()->MainWindowDispHd, ex.what());
    }
    printf_s("over \n");
}

void MainWindow::on_pushButton_SaveParameter_clicked()
{
    SAMPLENUM = ui->lineEdit_SampleNum->text().toInt();
    if(SAMPLENUM < 1)
    {
        SAMPLENUM = 1;ui->lineEdit_SampleNum->setText(QString::number(SAMPLENUM));
    }

    SNAP_INTERVAL = ui->lineEdit_Interval->text().toInt();

    COLOR_SIGMA = ui->lineEdit_Sigma->text().toDouble();

    SAVEPRJPARA;
}

void MainWindow::on_pushButton_Close_clicked()
{
    close();
}

void MainWindow::on_pushButton_Divides_clicked()
{
    int Div = ui->lineEdit_Divides->text().toInt();
    if(Div > 2 && Div < 11){
        USER_MAX_SIZE_OF_ID = Div;
        UpdateData();
        int i = 0;
        for(auto &pLineEdit:LineEdits){

            if(i<Div){
                pLineEdit->setVisible(true);
            }else{
                pLineEdit->setVisible(false);
            }
            i++;
        }
        i = 0;
        for(auto &pLineEdit:Labels){

            if(i<Div){
                pLineEdit->setVisible(true);
            }else{
                pLineEdit->setVisible(false);
            }
            i++;
        }
        SAVEPRJPARA;
    }
    else {
        global::GetIns()->evt_Trigger.SetEvent();
    }
}
