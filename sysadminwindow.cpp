#include "sysadminwindow.h"
#include "ui_sysadminwindow.h"

SysadminWindow::SysadminWindow(std::string username_display, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SysadminWindow)
{
    ui->setupUi(this);
    ui->welcomeUserLabelSysAdmin->setText(tr("Welcome, %1").arg(QString::fromStdString(username_display)));

    connect(ui->logOutButtonSysAdmin, &QToolButton::clicked, this, &SysadminWindow::logOutHandler);
}

void SysadminWindow::logOutHandler()
{
    emit logoutRequest();
}
