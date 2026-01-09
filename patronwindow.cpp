#include "patronwindow.h"
#include "ui_patronwindow.h"
#include <QDebug>
#include <QString>
#include <string>
#include "types.h"


std::string statusToString(hinlibs::ItemStatus status) {
    switch (status) {
        case hinlibs::ItemStatus::Available: return "Available";
        case hinlibs::ItemStatus::CheckedOut: return "Checked Out";
        default: return "Unknown";
    }
}

PatronWindow::PatronWindow(std::shared_ptr<hinlibs::Patron> patron, QWidget *parent)  : QWidget(parent), ui(new Ui::PatronWindow), patron_(std::move(patron)) {
    ui->setupUi(this);
}

PatronWindow::~PatronWindow() {
    delete ui;
}

void PatronWindow::on_browseButton_clicked() {
    ui->outputArea->clear();
    auto items = patron_->browseCatalogue();
    for (const auto& item : items) {
        ui->outputArea->append(QString::fromStdString(item.title + " by " + item.authorOrCreator +
                                                      " [" + std::to_string(static_cast<int>(item.format)) + "]" + " " + item.id + " | " + statusToString(item.status)
        ));
    }
}

void PatronWindow::on_borrowButton_clicked() {
    const std::string itemId = ui->inputField->text().toStdString();
    auto result = patron_->borrowItem(itemId);
    ui->outputArea->clear();
    if (result.ok) {
        ui->outputArea->append("Item borrowed successfully. Due in 14 days.");
    } else {
        ui->outputArea->append(QString::fromStdString("Borrow failed: " + result.message));
    }
}

void PatronWindow::on_returnButton_clicked() {
    const std::string itemId = ui->inputField->text().toStdString();
    auto result = patron_->returnItem(itemId);
    ui->outputArea->clear();
    if (result.ok) {
        ui->outputArea->append("Item returned successfully.");
    } else {
        ui->outputArea->append(QString::fromStdString("Return failed: " + result.message));
    }
}

void PatronWindow::on_placeHoldButton_clicked() {
    const std::string itemId = ui->inputField->text().toStdString();
    auto result = patron_->placeHold(itemId);
    ui->outputArea->clear();
    if (result.ok) {
        ui->outputArea->append(QString("Hold placed. Queue position: %1").arg(result.value.value()));
    } else {
        ui->outputArea->append(QString::fromStdString("Hold failed: " + result.message));
    }
}

void PatronWindow::on_cancelHoldButton_clicked() {
    const std::string itemId = ui->inputField->text().toStdString();
    auto result = patron_->cancelHold(itemId);
    ui->outputArea->clear();
    if (result.ok) {
        ui->outputArea->append("Hold cancelled successfully.");
    } else {
        ui->outputArea->append(QString::fromStdString("Cancel failed: " + result.message));
    }
}

void PatronWindow::on_accountStatusButton_clicked() {
    ui->outputArea->clear();
    auto status = patron_->viewAccountStatus();

    ui->outputArea->append("Active Loans:");
    for (const auto& loan : status.loans) {
        ui->outputArea->append(QString::fromStdString(
            loan.itemTitle + " — Due in " + std::to_string(loan.daysRemaining) + " days"
        ));
    }

    ui->outputArea->append("\nActive Holds:");
    for (const auto& hold : status.holds) {
        ui->outputArea->append(QString::fromStdString(
            hold.itemTitle + " — Queue position: " + std::to_string(hold.queuePosition)
        ));
    }
}

void PatronWindow::on_logoutButton_clicked() {
    parentWidget()->show();
    this->close();
}
