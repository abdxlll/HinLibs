#pragma once

#include <QWidget>
#include <memory>
#include "patron.h"

namespace Ui { class PatronWindow; }


class PatronWindow : public QWidget {
    Q_OBJECT

public:
    explicit PatronWindow(std::shared_ptr<hinlibs::Patron> patron, QWidget *parent = nullptr);
    ~PatronWindow();

private slots:
    void on_browseButton_clicked();
    void on_borrowButton_clicked();
    void on_returnButton_clicked();
    void on_placeHoldButton_clicked();
    void on_cancelHoldButton_clicked();
    void on_accountStatusButton_clicked();
    void on_logoutButton_clicked();

private:
    Ui::PatronWindow *ui;
    std::shared_ptr<hinlibs::Patron> patron_;
};
