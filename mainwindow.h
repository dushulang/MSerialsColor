#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void Init(const char *program_name = "");
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void OnInitDialog();
    static unsigned int __stdcall InitCamera_(void *pLvoid);
    static void UpdateHistoryInfo(std::string Info);
    void ShowMsg(std::string str);
    std::vector<QLineEdit*> LineEdits;
    std::vector<QLabel*> Labels;

    void UpdateData();

private slots:

    void on_pushButton_CabliSet_clicked();
    //static unsigned int __cdecl PLC_Connect(void * pThis);
    void on_pushButton_GrabAsync_clicked();

    void on_pushButton_ReadImage_clicked();

    void on_pushButton_DrawROI1_clicked();

    void on_pushButton_DrawROI2_clicked();

    void on_pushButton_DrawROI3_clicked();

    void on_pushButton_DrawROI4_clicked();

    void on_pushButton_CabliTest_clicked();

    void on_pushButton_SaveParameter_clicked();

    void on_pushButton_Close_clicked();

    void on_pushButton_Divides_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
