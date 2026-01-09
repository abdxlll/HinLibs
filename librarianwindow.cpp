#include "librarianwindow.h"
#include "ui_librarianwindow.h"
#include <QDateTime>
#include <QTableWidgetItem>
#include <chrono>


#include "database.h"
#include "types.h"
#include "item.h"

#include <QMessageBox>
#include <QString>

using namespace hinlibs;

librarianWindow::librarianWindow(const std::string &username_display,
                                 QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::librarianWindow)
{
    ui->setupUi(this);

    // ---- Header: welcome text ----
    ui->welcomeUserLabelLibrarian->setText(
        tr("Welcome, %1").arg(QString::fromStdString(username_display)));

    // ---- Wire up log out button ----
    connect(ui->logOutButtonLibrarian, &QToolButton::clicked,
            this, &librarianWindow::logOutHandler);

    // ---- Navigation buttons on the dashboard ----
    connect(ui->addItemButton, &QPushButton::clicked,
            this, &librarianWindow::on_addItemButton_clicked);

    connect(ui->removeItemButton, &QPushButton::clicked,
            this, &librarianWindow::on_removeItemButton_clicked);

    connect(ui->returnItemButton, &QPushButton::clicked,
            this, &librarianWindow::on_returnItemButton_clicked);

    // Start on main profile page
    ui->stackedWidget->setCurrentWidget(ui->profile_librarian);
}

// ----------------- Slots -----------------

void librarianWindow::logOutHandler()
{
    emit logoutRequest();
}

// "Add Item to Catalogue" button
void librarianWindow::on_addItemButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageAddItem);
}

// "Remove Item from Catalogue" button
void librarianWindow::on_removeItemButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageRemoveItem);
}

// "Return Item for Patron" button
void librarianWindow::on_returnItemButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageReturnOnBehalf);
}

// Back arrow on Add Item page
void librarianWindow::on_BackFromAddItem_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->profile_librarian);
}

// Confirm Add Item button
void librarianWindow::on_addItemConfirmButton_clicked()
{
    QString title   = ui->titleEdit->text().trimmed();
    QString author  = ui->authorEdit->text().trimmed();
    QString formatS = ui->formatCombo->currentText();
    int     year    = ui->yearSpin->value();
    QString genre   = ui->genreEdit->text().trimmed();
    QString dewey   = ui->deweyEdit->text().trimmed();
    QString rating  = ui->ratingEdit->text().trimmed();
    QString issue   = ui->issueEdit->text().trimmed();
    // QDate pubDate   = ui->pubDateEdit->date();   // optional, not used yet

    if (title.isEmpty()) {
        QMessageBox::warning(this, tr("Missing data"),
                             tr("Title cannot be empty."));
        return;
    }

    ItemDetails d{};
    // d.id left empty; Database::AddItem will generate it

    d.title           = title.toStdString();
    d.authorOrCreator = author.toStdString();

    if (formatS == "Book") {
        d.format = ItemFormat::Book;
    } else if (formatS == "Magazine") {
        d.format = ItemFormat::Magazine;
    } else if (formatS == "Movie") {
        d.format = ItemFormat::Movie;
    } else {
        d.format = ItemFormat::VideoGame;
    }

    d.status = ItemStatus::Available;

    if (year > 0) {
        d.publicationYear = year;
    }
    if (!genre.isEmpty()) {
        d.genre = genre.toStdString();
    }
    if (!dewey.isEmpty()) {
        d.deweyDecimal = dewey.toStdString();
    }
    if (!rating.isEmpty()) {
        d.rating = rating.toStdString();
    }
    if (!issue.isEmpty()) {
        d.issueNumber = issue.toStdString();
    }

    // Make sure we actually have a DB
    if (!db_) {
        QMessageBox::warning(this, tr("Internal error"),
                             tr("No database connection."));
        return;
    }

    auto result = db_->AddItem(d);

    if (!result.ok) {
        QMessageBox::warning(this, tr("Add Item Failed"),
                             QString::fromStdString(result.message));
        return;
    }

    QString newId;
    if (result.value.has_value()) {
        newId = QString::fromStdString(*result.value);
    } else {
        newId = tr("(no id)");
    }

    QMessageBox::information(this, tr("Item Added"),
                             tr("Item added with ID %1").arg(newId));

    // Clear form for next entry
    ui->titleEdit->clear();
    ui->authorEdit->clear();
    ui->genreEdit->clear();
    ui->deweyEdit->clear();
    ui->ratingEdit->clear();
    ui->issueEdit->clear();
    ui->yearSpin->setValue(0);
}

void librarianWindow::on_BackFromRemoveItem_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->profile_librarian);
}

void librarianWindow::on_BackFromReturnItem_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->profile_librarian);
}

void librarianWindow::on_removeItemConfirmButton_clicked()
{
    QString idText = ui->removeItemIdEdit->text().trimmed();
    std::string itemId = idText.toStdString();

    if (idText.isEmpty()) {
        QMessageBox::warning(this, tr("Missing data"),
                             tr("Please enter an Item ID."));
        return;
    }

    // 🔹 1) Lookup the item details BEFORE removing it
    QString title = "(unknown title)";
    if (db_) {
        auto details = db_->GetItemDetails(itemId);
        if (details) {
            title = QString::fromStdString(details->title);
        }
    }

    // 🔹 2) Call remove
    auto result = db_->RemoveItem(itemId);

    if (!result.ok) {
        QMessageBox::warning(this, tr("Remove Failed"),
                             QString::fromStdString(result.message));
        return;
    }

    // 🔹 3) Success message WITH TITLE
    QMessageBox::information(
        this,
        tr("Item Removed"),
        tr("'%1' (ID: %2) was removed successfully.")
            .arg(title)
            .arg(idText)
    );

    ui->removeItemIdEdit->clear();
}

void librarianWindow::on_findPatronButton_clicked()
{
    // Clear previous messages
    ui->returnPatronStatusLabel->setText("");
    ui->returnItemResultLabel->setText("");
    ui->returnItemResultLabel->setStyleSheet("");

    if (!db_) {
        ui->returnPatronStatusLabel->setText("No database connection.");
        clearReturnTable();
        return;
    }

    QString username = ui->returnPatronUsernameEdit->text().trimmed();
    if (username.isEmpty()) {
        ui->returnPatronStatusLabel->setText("Please enter a patron username.");
        clearReturnTable();
        return;
    }

    // Look up the user in the database
    auto recOpt = db_->FindUserByName(username.toStdString());
    if (!recOpt || recOpt->role != hinlibs::Role::Patron) {
        ui->returnPatronStatusLabel->setText("Patron not found.");
        clearReturnTable();
        currentReturnPatronId.clear();
        return;
    }

    const auto &rec = *recOpt;
    currentReturnPatronId = rec.id;   // remember which patron we’re working with

    // Get their active loans
    auto loans = db_->GetPatronActiveLoans(rec.id);

    if (loans.empty()) {
        ui->returnPatronStatusLabel->setText("This patron has no active loans.");
        clearReturnTable();
        return;
    }

    ui->returnPatronStatusLabel->setText("Active loans loaded.");
    fillReturnTable(loans);
}

void librarianWindow::on_returnSelectedItemButton_clicked()
{
    // Clear previous result style
    ui->returnItemResultLabel->setText("");
    ui->returnItemResultLabel->setStyleSheet("");

    if (!db_) {
        ui->returnItemResultLabel->setText("No database connection.");
        return;
    }

    if (currentReturnPatronId.empty()) {
        ui->returnItemResultLabel->setText("Please find a patron first.");
        return;
    }

    QTableWidget *table = ui->returnLoansTable;
    int row = table->currentRow();
    if (row < 0) {
        ui->returnItemResultLabel->setText("Please select an item to return.");
        return;
    }

    // Column 0 holds the Item ID
    QTableWidgetItem *idItem = table->item(row, 0);
    if (!idItem) {
        ui->returnItemResultLabel->setText("Internal error: no item id in row.");
        return;
    }

    QString itemIdStr = idItem->text();
    auto result = db_->ReturnItem(currentReturnPatronId,
                                  itemIdStr.toStdString());

    if (result.ok) {
        ui->returnItemResultLabel->setStyleSheet("QLabel { color: #2e7d32; }");
        ui->returnItemResultLabel->setText("Item returned successfully.");

        // Refresh the patron's loans in the table
        auto loans = db_->GetPatronActiveLoans(currentReturnPatronId);
        if (loans.empty()) {
            clearReturnTable();
            ui->returnPatronStatusLabel->setText("All items returned. No active loans.");
        } else {
            fillReturnTable(loans);
        }
    } else {
        ui->returnItemResultLabel->setStyleSheet("QLabel { color: #c62828; }");
        ui->returnItemResultLabel->setText(
            QString("Return failed: %1").arg(QString::fromStdString(result.message))
        );
    }
}

void librarianWindow::clearReturnTable()
{
    QTableWidget *table = ui->returnLoansTable;
    table->clearContents();
    table->setRowCount(0);
}

void librarianWindow::fillReturnTable(const std::vector<hinlibs::LoanSnapshot> &loans)
{
    QTableWidget *table = ui->returnLoansTable;

    table->setColumnCount(4);
    table->setHorizontalHeaderLabels(
        QStringList() << "Item ID" << "Title" << "Format" << "Due date"
    );

    table->setRowCount(static_cast<int>(loans.size()));

    int row = 0;
    for (const auto &loan : loans) {
        auto detailsOpt = db_->GetItemDetails(loan.itemId);
        if (!detailsOpt) continue;
        const auto &details = *detailsOpt;

        // Convert due date to string
        using namespace std::chrono;
        auto tp       = loan.dueDate;
        auto secs     = time_point_cast<seconds>(tp).time_since_epoch().count();
        QDateTime dt  = QDateTime::fromSecsSinceEpoch(secs, Qt::LocalTime);
        QString dueStr = dt.toString("yyyy-MM-dd");

        // Item ID
        table->setItem(row, 0, new QTableWidgetItem(
            QString::fromStdString(details.id)
        ));

        // Title
        table->setItem(row, 1, new QTableWidgetItem(
            QString::fromStdString(details.title)
        ));

        // Format (rough string; you can swap in a helper if you have one)
        QString fmtStr;
        switch (details.format) {
        case hinlibs::ItemFormat::Book:      fmtStr = "Book"; break;
        case hinlibs::ItemFormat::Magazine:  fmtStr = "Magazine"; break;
        case hinlibs::ItemFormat::Movie:     fmtStr = "Movie"; break;
        case hinlibs::ItemFormat::VideoGame: fmtStr = "Video game"; break;
        default:                             fmtStr = "Unknown"; break;
        }
        table->setItem(row, 2, new QTableWidgetItem(fmtStr));

        // Due date
        table->setItem(row, 3, new QTableWidgetItem(dueStr));

        ++row;
    }

    table->resizeColumnsToContents();
}
