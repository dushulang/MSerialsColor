#include "dialogsetting.h"
#include "ui_dialogsetting.h"
#include "mediator.h"
#include "HalconCpp.h"
#include "opencv.hpp"
#include "global.h"
#include "MSerialsCV.h"
#include "excv.h"
#include "mcamera.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>


void __stdcall DispMatImage_Ex(cv::Mat InputArray, void * pWnd, bool Inv){
    try
    {
        HalconCpp::HObject hobj;
        Excv::MatToHObj(InputArray,hobj);
        Excv::h_disp_obj(hobj,Mediator::GetIns()->DialogSetDispHd);
    }catch(cv::Exception ex)
    {

    }
}

unsigned int __stdcall CabliCamera(void * pThis)
{
    static bool isCabling = false;
    if(isCabling) return 1;
    isCabling = true;
    Control_Var Ctrl_Var;
    Ctrl_Var.DispMatImage = DispMatImage_Ex;
    try {
        std::string error_s;
        printf("平均格子长度 %f\n", CvCabli(334, Ctrl_Var, error_s, 60));
        if (strlen(error_s.c_str()) > 0)
        {
            QMessageBox::warning(NULL,QString("Error"),QString(error_s.c_str()).toLocal8Bit());
        }
        else
        {
            HalconCpp::SetTposition(Mediator::GetIns()->DialogSetDispHd,10,0);
            HalconCpp::SetColor(Mediator::GetIns()->DialogSetDispHd,"green");
            HalconCpp::WriteString(Mediator::GetIns()->DialogSetDispHd,"标定完毕,软件重启后载入标定数据");
        }
    }catch(cv::Exception ex)
    {
        HalconCpp::SetTposition(Mediator::GetIns()->DialogSetDispHd,10,0);
        HalconCpp::SetColor(Mediator::GetIns()->DialogSetDispHd,"red");
        HalconCpp::WriteString(Mediator::GetIns()->DialogSetDispHd,ex.what());
    }
    isCabling = false;
    return 0;
}



DialogSetting::DialogSetting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetting)
{
    ui->setupUi(this);

    showFullScreen();

    int nWndWidth = ui->widget->width();
    int nWndHeight= ui->widget->height();
    HalconCpp::SetCheck("~father");
    HalconCpp::OpenWindow(0, 0, nWndWidth, nWndHeight, (Hlong)ui->widget->winId(), "visible", "", &    Mediator::GetIns()->DialogSetDispHd);
    HalconCpp::SetCheck("father");


    ui->lineEdit_CabliBoardSize->setText(QString::number(239.83,'g',5));
    ui->lineEdit_XmmPerpix->setText(QString::number(RATIOX,'g',5));
    ui->lineEdit_YmmPerpix->setText(QString::number(RATIOY,'g',5));

}

DialogSetting::~DialogSetting()
{
    delete ui;
}

void DialogSetting::on_pushButton_Cabli_clicked()
{
    (HANDLE)_beginthreadex(NULL, 0, CabliCamera, this, 0, NULL);
}

void DialogSetting::on_pushButton_SaveCabli_clicked()
{ 
    using namespace HalconCpp;
     HObject Image,Hobj;
     cv::Mat Snap_Image,ColorView;
     static bool isSaving = false;
    try{
        MCamera::GetIns()->GrabImageA(Image);
        Excv::HobjToMat(Image,Snap_Image);
        isSaving = true;
        if(Excv::CheckCabliImage(Snap_Image,ColorView))
        {
            Excv::cv_write_image(Snap_Image,CABLI,CABLI,true);
        }
        Excv::MatToHObj(ColorView,Hobj);
        Excv::h_disp_obj(Hobj,Mediator::GetIns()->DialogSetDispHd);

     }catch(HalconCpp::HException except)
     {
            HTuple ExceptionMessage;
            except.ToHTuple(&ExceptionMessage);
            SetTposition(Mediator::GetIns()->MainWindowDispHd, 0, 1);
            WriteString(Mediator::GetIns()->MainWindowDispHd, ExceptionMessage);
     }
    catch(cv::Exception ex)
        {
            HalconCpp::SetTposition(Mediator::GetIns()->DialogSetDispHd,10,0);
            HalconCpp::SetColor(Mediator::GetIns()->DialogSetDispHd,"red");
            HalconCpp::WriteString(Mediator::GetIns()->DialogSetDispHd,ex.what());
        }
    isSaving = false;
}

void DialogSetting::on_pushButton_Cabli_Dir_clicked()
{
    QDesktopServices::openUrl(QUrl("Cabli", QUrl::TolerantMode));
}

void DialogSetting::on_pushButton_Cabli_Size_clicked()
{
    using namespace HalconCpp;
     HObject Image,Hobj;
     cv::Mat Snap_Image,ColorView;
     static bool isSaving = false;

     double BoardSize = ui->lineEdit_CabliBoardSize->text().toDouble();
     double x_ = ui->lineEdit_XmmPerpix->text().toDouble();
     double y_ = ui->lineEdit_YmmPerpix->text().toDouble();

    try{
        MCamera::GetIns()->GrabImageA(Image);
        Excv::HobjToMat(Image,Snap_Image);
        isSaving = true;


        double square_size = -1;
        Control_Var Ctrl_Var;
        if(Snap_Image.empty())  return;
        Ctrl_Var.DispMatImage = DispMatImage_Ex;
        square_size = CvSquareSize(Snap_Image, Ctrl_Var);
        if(square_size < 0)
        {
            HalconCpp::SetTposition(Mediator::GetIns()->DialogSetDispHd,10,0);
            HalconCpp::SetColor(Mediator::GetIns()->DialogSetDispHd,"red");
            HalconCpp::WriteString(Mediator::GetIns()->DialogSetDispHd,"没有找到标定板");
            return;
        }
        double x = -1;
        double y = -1;
        square_size = BoardSize/square_size;
        if(x_ > 0) x  = (x_+square_size)/2.0; else x = square_size;
        if(y_>0) y = (y_+square_size)/2.0; else y=square_size;
        RATIOX = x;
        RATIOY = y;
        static double MaxV = -1;
        static double MinV = 999999999999;
        if(x>MaxV) MaxV = x;
        if(x<MinV) MinV = x;
        std::cout << "整个冲床可能产生的误差为 "<<1024*(MaxV-MinV)<<std::endl;
        Preference::GetIns()->prj->WriteSettings();
        ui->lineEdit_XmmPerpix->setText(QString::number(x,'g'));
        ui->lineEdit_YmmPerpix->setText(QString::number(y,'g'));



     }catch(HalconCpp::HException except)
     {
            HTuple ExceptionMessage;
            except.ToHTuple(&ExceptionMessage);
            SetTposition(Mediator::GetIns()->DialogSetDispHd, 0, 1);
            WriteString(Mediator::GetIns()->DialogSetDispHd, ExceptionMessage);
     }
    catch(cv::Exception ex)
        {
            HalconCpp::SetTposition(Mediator::GetIns()->DialogSetDispHd,10,0);
            HalconCpp::SetColor(Mediator::GetIns()->DialogSetDispHd,"red");
            HalconCpp::WriteString(Mediator::GetIns()->DialogSetDispHd,ex.what());
        }
    isSaving = false;
}

void DialogSetting::on_pushButton_SavePara_clicked()
{

}

void DialogSetting::on_pushButton_Close1_clicked()
{
    close();
}
