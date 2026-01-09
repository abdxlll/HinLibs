#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "homewindow.h"
#include "librarianwindow.h"
#include "librarian.h"
#include "sysadminwindow.h"
#include "sysadmin.h"

#include <QString>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

MainWindow::MainWindow(std::shared_ptr<hinlibs::Session> session,
                       QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      session(std::move(session))
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_loginButton_clicked() {
    const auto username = ui->usernameInput->text().toStdString();

    if (!session || !session->signIn(username)) {
        ui->statusLabel->setText("Login failed: user not found.");
        return;
    }

    auto user = session->currentUser();
    if (!user) {
        ui->statusLabel->setText("Login failed: no active user.");
        return;
    }

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->availableGeometry();

    switch (user->role()) {
        case hinlibs::Role::Patron: {
            auto patron = std::dynamic_pointer_cast<hinlibs::Patron>(user);
            auto home = new HomeWindow(patron, session, this);

            connect(home, &HomeWindow::logoutRequest, this, [this, home]() {
                home->close();
                home->deleteLater();
                this->show();
                ui->usernameInput->setText("");
                ui->statusLabel->setText("");
            });

            home->setAttribute(Qt::WA_DeleteOnClose, false);
            home->setWindowModality(Qt::NonModal);
            home->setWindowFlags(Qt::Window);
            home->show();
            home->move(screenRect.center() - home->rect().center());
            this->hide();

            break;
        }

        case hinlibs::Role::Librarian: {
            auto librarian = std::dynamic_pointer_cast<hinlibs::Librarian>(user);
            auto home = new librarianWindow(librarian->getUsername(), this);

            // 🔹 Inject the shared Database into the librarian window
            // Adjust "database()" to "db()" if your Session uses a different name.
            home->setDatabase(session->db().get());


            connect(home, &librarianWindow::logoutRequest, this, [this, home]() {
                home->close();
                home->deleteLater();
                this->show();
                ui->usernameInput->setText("");
                ui->statusLabel->setText("");
            });

            home->setAttribute(Qt::WA_DeleteOnClose, false);
            home->setWindowModality(Qt::NonModal);
            home->setWindowFlags(Qt::Window);
            home->show();
            home->move(screenRect.center() - home->rect().center());
            this->hide();

            break;
        }

        case hinlibs::Role::SysAdmin: {
            auto sysadmin = std::dynamic_pointer_cast<hinlibs::SysAdmin>(user);
            auto home = new SysadminWindow(sysadmin->getUsername(), this);

            connect(home, &SysadminWindow::logoutRequest, this, [this, home]() {
                home->close();
                home->deleteLater();
                this->show();
                ui->usernameInput->setText("");
                ui->statusLabel->setText("");
            });

            home->setAttribute(Qt::WA_DeleteOnClose, false);
            home->setWindowModality(Qt::NonModal);
            home->setWindowFlags(Qt::Window);
            home->show();
            home->move(screenRect.center() - home->rect().center());
            this->hide();

            break;
        }

        default:
            ui->statusLabel->setText("Unknown role.");
            break;
    }
}
