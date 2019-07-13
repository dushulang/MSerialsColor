#include "mainwindow.h"
#include <QApplication>
#include "mcamera.h"

#pragma comment(lib,"d:/Users/Lux/Desktop/ProjectFile/third_party\\halcon17\\halconcpp.lib")
#pragma comment(lib,"d:\\Users\\Lux\\Desktop\\ProjectFile\\ChongChuang12\\x64\\Release\\MSerialsCV.lib")
#pragma comment(lib,"d:/Users/Lux/Desktop/ProjectFile/third_party/opencv4.0.1/x64/vc15/lib/opencv_world401.lib")
#pragma comment(lib,"../PLC_DLL/x64/Release/PLC_DLL.lib")

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.Init(std::string(argv[0]).c_str());

    w.show();
    w.OnInitDialog();
   // MCamera::GetIns()->Init();
    return a.exec();
}
