#pragma once

#include <QMainWindow>
#include "patronwindow.h"
#include "homewindow.h"
#include <memory>
#include "session.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow  {
    Q_OBJECT

public:
    explicit MainWindow(std::shared_ptr<hinlibs::Session> session, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_loginButton_clicked();

private:
    Ui::MainWindow *ui;
    std::shared_ptr<hinlibs::Session> session;
    HomeWindow* patronWindow = nullptr;
};
