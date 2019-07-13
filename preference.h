#ifndef PREFERENCE_H
#define PREFERENCE_H



#include <QSettings>
#include <iostream>
#include <string>
#include <QString>


#define MAX_SIZE_OF_DI  64
#define USER_MAX_SIZE_OF_ID Preference::GetIns()->prj->UserDefineSize
#define SNAP_INTERVAL       Preference::GetIns()->prj->Interval
#define COLOR_SIGMA         Preference::GetIns()->prj->Sigma

#define MAX_VOL         8
#define Baund1200       "1200"
#define Baund2400       "2400"
#define Baund4800       "4800"
#define Baund9600       "9600"
#define Baund19200      "19200"
#define Baund38400      "38400"
#define Baund57600      "57600"
#define Baund115200     "115200"
#define DATA7           "7"
#define DATA8           "8"
#define PARNONE         "None"
#define PARODD          "Odd"
#define PAREVEN         "Even"
#define STOP1           "1"
#define STOP1D5         "1.5"
#define STOP2           "2"


#define SIZEPARA        Preference::GetIns()->prj->SizePara
#define SIZEPARAQ       Preference::GetIns()->prj->SizeParaQuantity
#define ISDRAW          Preference::GetIns()->prj->isDrawing
#define SAMPLENUM       Preference::GetIns()->prj->SampleNum
#define ROW             Preference::GetIns()->prj->Row
#define COLUMN          Preference::GetIns()->prj->Column
#define ANGLE           Preference::GetIns()->prj->Angle
#define LEN1            Preference::GetIns()->prj->Len1
#define LEN2            Preference::GetIns()->prj->Len2
#define LINEOFFSET      Preference::GetIns()->prj->Offset
#define SAVEPRJPARA     Preference::GetIns()->prj->WriteSettings()
#define RATIOX          Preference::GetIns()->prj->RatioX
#define RATIOY          Preference::GetIns()->prj->RatioY
#define CABLI           "Cabli"


#define PARA_SET	(1)
#define PARA_PRJ	(1<<1)
#define PARA_IMAGE	(1<<2)
#define PARA_IO		(1<<3)

#define PARA_ALL	(PARA_SET | PARA_PRJ | PARA_IMAGE |PARA_IO)   //ALL -1

#define STR_SET     "PARA_SET"
#define STR_PRJ     "PARA_PRJ"
#define STR_IMAGE   "IMAGE_SETTING"
#define STR_IO      "PARA_IO"

class Ini
{
private:
    void Init() {
        InitData();
        int SEL = PARA_ALL;
        if (0 == strcmp("sys", m_class_name.c_str()))
        {
            SEL = PARA_PRJ | PARA_IO | PARA_SET;
        }
        WriteSettings(SEL);
    }

    std::string m_class_name;
    QSettings *settings;
    QString str_set,str_prj,str_img,str_io;

public:
    Ini(std::string class_name = "Preference")
        : settings(nullptr)
        ,m_class_name (class_name)
    {
        str_set = STR_SET; str_set += "/";
        str_prj = STR_PRJ; str_prj += "/";
        str_img = STR_IMAGE; str_img += "/";
        str_io = STR_IO; str_io += "/";
        InitData();
    }

    ~Ini() { delete settings; }

    const char* Class_Name() { return m_class_name.c_str(); }

    void SetFilePos(QString str) {
        if (nullptr != settings) delete settings;
        settings = new QSettings(str, QSettings::IniFormat);
        ReadSettings(PARA_ALL);
    }


public:
    void WriteSettings(int SEL = PARA_ALL)
    {

        if (SEL & PARA_SET)
        {

            //表示文件存在
            settings->setValue(str_set + "sign", 1);
        }

        if (SEL & PARA_PRJ)
        {
            settings->setValue(str_prj + "COM", COM);
            settings->setValue(str_prj + "BAUND", BAUND);
            settings->setValue(str_prj + "PARITY", PARITY);
            settings->setValue(str_prj + "DATA", DATA);
            settings->setValue(str_prj + "STOP", STOP);

            settings->setValue(str_prj + "ACC", Acc);
            settings->setValue(str_prj + "SPD", Spd);
            settings->setValue(str_prj + "POS", Pos);
            settings->setValue(str_prj + "SAMNUM", SampleNum);
            for(int i = 0;i<MAX_VOL ;i++){
                settings->setValue(str_prj + "ROW" + QString::number(i), Row[i]);
                settings->setValue(str_prj + "COL" + QString::number(i), Column[i]);
                settings->setValue(str_prj + "ANGLE" + QString::number(i), Angle[i]);
                settings->setValue(str_prj + "LEN1" + QString::number(i), Len1[i]);
                settings->setValue(str_prj + "LEN2" + QString::number(i), Len2[i]);
                settings->setValue(str_prj + "OFS" + QString::number(i), Offset[i]);
            }
            settings->setValue(str_prj + "RATIOX", RatioX);
            settings->setValue(str_prj + "RATIOY", RatioY);

            for(int i = 0;i<MAX_SIZE_OF_DI;i++){
                settings->setValue(str_prj + "SP" + QString::number(i), SizePara[i] );
                settings->setValue(str_prj + "SPQ" + QString::number(i), SizeParaQuantity[i] );
            }

            settings->setValue(str_prj + "USER_DEF", UserDefineSize);

            settings->setValue(str_prj + "ITRVAL", Interval);
            settings->setValue(str_prj + "SIGMA", Sigma);
        }

        if (SEL & PARA_IMAGE)
        {

        }


        if (SEL & PARA_IO)
        {

        }
    }



    void ReadSettings(int SEL = PARA_ALL)
    {
        if (SEL & PARA_SET)
        {
            //表示文件存在
            int Sign = settings->value(str_set+"sign", 0).toInt();
            if (1 != Sign) {
                Init();
                return;
            }
        }


        if (SEL & PARA_PRJ)
        {
            COM = settings->value(str_prj + "COM","COM1").toString();
            BAUND = settings->value(str_prj + "BAUND",Baund9600).toString();
            PARITY = settings->value(str_prj + "PARITY",PAREVEN).toString();
            DATA = settings->value(str_prj + "DATA",DATA7).toString();
            STOP = settings->value(str_prj + "STOP",STOP1).toString();

            Acc = settings->value(str_prj + "ACC","2000").toString();
            Spd = settings->value(str_prj + "SPD","2000").toString();
            Pos = settings->value(str_prj + "POS","2000").toString();
            SampleNum = settings->value(str_prj + "SAMNUM",5).toInt();

            for(int i = 0;i<MAX_VOL ;i++){
                Row[i] = settings->value(str_prj + "ROW" + QString::number(i),0.0).toDouble();
                Column[i] = settings->value(str_prj + "COL" + QString::number(i),0.0).toDouble();
                Angle[i] = settings->value(str_prj + "ANGLE" + QString::number(i),0.0).toDouble();
                Len1[i] = settings->value(str_prj + "LEN1" + QString::number(i),2.0).toDouble();
                Len2[i] = settings->value(str_prj + "LEN2" + QString::number(i),2.0).toDouble();
                Offset[i] = settings->value(str_prj + "OFS" + QString::number(i),2.0).toDouble();
            }

            for(int i = 0;i<MAX_SIZE_OF_DI;i++){
                SizePara[i] = settings->value(str_prj + "SP" + QString::number(i),0.1).toDouble();
                SizeParaQuantity[i] = settings->value(str_prj + "SPQ" + QString::number(i),0.0).toDouble();
            }

            UserDefineSize = settings->value(str_prj + "USER_DEF",6).toInt();
            RatioX = settings->value(str_prj + "RATIOX",0.3).toDouble();
            RatioY = settings->value(str_prj + "RATIOY",0.3).toDouble();

            Interval = settings->value(str_prj + "ITRVAL",100).toInt();
            Sigma = settings->value(str_prj+"SIGMA",12.0).toDouble();
        }


        if (SEL & PARA_IMAGE)
        {

        }


        if (SEL & PARA_IO)
        {

        }
    }


    void InitData(){
        COM = QString("COM1");
        BAUND = QString(Baund9600);
        PARITY = QString(PAREVEN);
        DATA =QString(DATA7);
        STOP = QString(STOP1);
    }
    QString COM = QString("COM1");
    QString BAUND = QString(Baund9600);
    QString PARITY = QString(PAREVEN);
    QString DATA = QString(DATA7);
    QString STOP = QString(STOP1);
    QString Acc = "2000";
    QString Spd = "2000";
    QString Pos = "0";
    int SampleNum = 5;
    double Row[MAX_VOL] = {0,0,0,0,0,0,0,0};
    double Column[MAX_VOL] = {0,0,0,0,0,0,0,0};
    double Angle[MAX_VOL] = {0,0,0,0,0,0,0,0};
    double Len1[MAX_VOL] = {0,0,0,0,0,0,0,0};
    double Len2[MAX_VOL] = {0,0,0,0,0,0,0,0};
    double Offset[MAX_VOL]= {0,0,0,0,0,0,0,0};
    double RatioX = 1.0;
    double RatioY = 1.0;
    int isDrawing = 0;


    //拍照时间间隔
    int Interval = 100;
    //颜色波动最大值
    double Sigma = 12.0;
    int UserDefineSize = 6;
    double SizePara[MAX_SIZE_OF_DI];//= {0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1};
    double SizeParaQuantity[MAX_SIZE_OF_DI];// = {0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1};
};

/*
请配合 Ini类文件使用
使用方法，在初始化地方如下例子即可
//系统配置文件
Preference::GetIns()->sys->SetFilePos(QString("sys.ini"));
//项目配置文件
QString Path = Preference::GetIns()->sys->Project_Name + "/prj.ini";
Preference::GetIns()->prj->SetFilePos(Path);
*/

class Preference
{
public:
    Preference() {
        sys = new Ini("sys");
        prj = new Ini("prj");
    }
    ~Preference() { delete sys; delete prj; }
    static Preference *GetIns() { static Preference ini; return &ini; }
    Ini *sys;
    Ini *prj;
};




#endif // PREFERENCE_H
