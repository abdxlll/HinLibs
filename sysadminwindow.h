#ifndef SYSADMINWINDOW_H
#define SYSADMINWINDOW_H

#include <QWidget>

namespace Ui {
class SysadminWindow;
}

class SysadminWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SysadminWindow(std::string username_display, QWidget *parent = nullptr);
//    ~SysadminWindow();

private:
    Ui::SysadminWindow *ui;

    void logOutHandler();

signals:
    void logoutRequest();
};

#endif // SYSADMINWINDOW_H
