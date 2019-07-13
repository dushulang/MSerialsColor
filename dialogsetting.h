#ifndef DIALOGSETTING_H
#define DIALOGSETTING_H

#include <QDialog>

namespace Ui {
class DialogSetting;
}

class DialogSetting : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSetting(QWidget *parent = 0);
    ~DialogSetting();

private slots:
    void on_pushButton_Cabli_clicked();

    void on_pushButton_SaveCabli_clicked();

    void on_pushButton_Cabli_Dir_clicked();

    void on_pushButton_Cabli_Size_clicked();

    void on_pushButton_SavePara_clicked();

    void on_pushButton_Close1_clicked();

private:
    Ui::DialogSetting *ui;
};

#endif // DIALOGSETTING_H
