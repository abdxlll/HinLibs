#include "homewindow.h"
#include "ui_homewindow.h"
#include <QString>
#include <QPushButton>
#include <QGridLayout>
#include <string>
#include "types.h"
#include <QRandomGenerator>
#include <QStringList>
#include <QDateTime>
#include <chrono>
using days = std::chrono::duration<int, std::ratio<60 * 60 * 24>>;
#include <QMainWindow>

HomeWindow::HomeWindow(std::shared_ptr<hinlibs::Patron> patron, std::shared_ptr<hinlibs::Session> session, QWidget *parent)  : QWidget(parent), ui(new Ui::HomeWindow), patron_(std::move(patron)), session_(std::move(session)) {
    ui->setupUi(this);

    // Go to home, since we start from home page
    goToHome();

    // Set the welcome us in the profile
    ui->welcomeUserLabel->setText(tr("Welcome, %1").arg(QString::fromStdString(patron_->getUsername())));
    // Connect to profile buttons.
    // They all do exactly the same job, since they lead you to the profile page
    connect(ui->ProfileButtonInHome, &QToolButton::clicked, this, &HomeWindow::goToProfile);
    connect(ui->ProfileButtonInBorrowItem, &QToolButton::clicked, this, &HomeWindow::goToProfile);
    connect(ui->ProfileButtonInHoldItem, &QToolButton::clicked, this, &HomeWindow::goToProfile);
    connect(ui->ProfileButtonInManageMyHold, &QToolButton::clicked, this, &HomeWindow::goToProfile);
    connect(ui->ProfileButtonInManageMyItem, &QToolButton::clicked, this, &HomeWindow::goToProfile);
    connect(ui->ProfileButtonInProfile, &QToolButton::clicked, this, &HomeWindow::goToProfile);


    // Now connect the back buttons
    // The lead to either profile page or home page

    connect(ui->BackFromProfile, &QToolButton::clicked, this, &HomeWindow::goToHome);
    connect(ui->BackFromBorrowItem, &QToolButton::clicked, this, &HomeWindow::goToHome);
    connect(ui->BackFromHoldItem, &QToolButton::clicked, this, &HomeWindow::goToHome);

    connect(ui->BackFromManageMyHold, &QToolButton::clicked, this, &HomeWindow::goToProfile);
    connect(ui->BackFromManageMyItem, &QToolButton::clicked, this, &HomeWindow::goToProfile);

    connect(ui->ReloadHomeButton, &QToolButton::clicked, this, &HomeWindow::onReloadClickedHome);

    // Add a handler for fltering
    connect(ui->showCheckedOutItems, &QCheckBox::toggled, this, [=](bool checked){
        renderItems(checked);
    });

    connect(ui->borrowItemButton, &QPushButton::clicked, this, &HomeWindow::borrowItemHandler);
    connect(ui->borrowItemPlaceHoldButton, &QPushButton::clicked, this, &HomeWindow::placeHoldHandler);

    connect(ui->logOutButton, &QPushButton::clicked, this, &HomeWindow::logOutHandler);

    connect(ui->returnItemButton, &QPushButton::clicked, this, &HomeWindow::returnItemHandler);

    connect(ui->borrowItemButtonHold, &QPushButton::clicked, this, &HomeWindow::borrowItemFromHoldHandler);
    connect(ui->cancelHoldButton, &QPushButton::clicked, this, &HomeWindow::cancelHoldHandler);

}

void HomeWindow::goToProfile()
{
    int index = ui->NavigationWidget->indexOf(ui->profile);
    ui->NavigationWidget->setCurrentIndex(index);

    renderProfile();
}

void HomeWindow::renderProfile()
{
    QWidget *content = ui->itemsGridContainerProfile;  // the widget INSIDE the scroll area

    QGridLayout *layout = qobject_cast<QGridLayout *>(content->layout());
    if (!layout) {
        // First time: create layout and set it on the container.
        layout = new QGridLayout;        // no parent here
        content->setLayout(layout);      // this gives Qt ownership
    } else {
        // Reuse existing layout: clear child widgets only.
        clearLayout(layout);
    }

    ui->scrollAreaProfile->setWidgetResizable(true);

    populateProfileGrid();
}

void HomeWindow::populateProfileGrid()
{
    if (!patron_) return;

    QWidget *content = ui->itemsGridContainerProfile;
    QGridLayout *layout = qobject_cast<QGridLayout *>(content->layout());
    if (!layout) return;

    layout->setSpacing(0);

    auto holds = patron_->activeHolds();
    auto loans = patron_->activeLoans();
    auto loan_number = patron_->activeLoanCount();

    const int columns = 2;
    int row = 0;
    int col = 1;

    QString text = tr("You have %1 active loans:").arg(loan_number);

    QLabel *lbl = new QLabel(content);
    lbl->setText(text);
    lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    lbl->setMinimumWidth(280);
    lbl->setMaximumWidth(280);
    lbl->setMinimumHeight(30);
    lbl->setMaximumHeight(30);

    lbl->setStyleSheet("QLabel { text-align: left; white-space: normal;}");
    layout->addWidget(lbl, 0, 0);


    while (col < columns) {
        QLabel *lbl2 = new QLabel(content);
        lbl2->setText("");
        lbl2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        lbl2->setMinimumWidth(280);
        lbl2->setMaximumWidth(280);
        lbl2->setMinimumHeight(30);
        lbl2->setMaximumHeight(30);

        layout->addWidget(lbl2, row, col);

        col++;
    }

    row++;
    col = 0;
    // Set tthe text size to a bigger one and make it bold

    // Force wrapping



    for (const auto &loan : loans) {
        auto itemDetails = patron_->getItemDetails(loan.itemId);
        using namespace std::chrono;
        auto tp = loan.dueDate;
        auto secs = time_point_cast<seconds>(tp).time_since_epoch().count();
        QDateTime dt = QDateTime::fromSecsSinceEpoch(secs, Qt::LocalTime);
        QString dueStr = dt.toString("yyyy-MM-dd");
        auto now = system_clock::now();
        auto diff = duration_cast<days>(floor<days>(loan.dueDate - now));
        auto days = (int)diff.count();
        QString text =
            QString::fromStdString(itemDetails->title + "\n") +tr("Due date: %1 \n").arg(dueStr) + tr("Days left: %1").arg(QString::number(days));

        QToolButton *btn = new QToolButton(content);
        btn->setText(text);
        btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        btn->setMinimumWidth(280);
        btn->setMaximumWidth(280);
        btn->setMinimumHeight(80);
        btn->setMaximumHeight(80);
        btn->setCursor(Qt::PointingHandCursor);

        // Force wrapping
        btn->setStyleSheet("QToolButton { text-align: left; white-space: normal;}");

        connect(btn, &QToolButton::clicked, this, [this, loan]() {
            onLoanClickedProfile(loan);
        });

        layout->addWidget(btn, row, col);

        if (++col == columns) {
            col = 0;
            ++row;
        }
    }

    if (col != 0) {
        row++;
        col = 0;
    }

    QString text_holds = tr("You have %1 active holds").arg(holds.size());
    QLabel *lbl_hold = new QLabel(content);
    lbl_hold->setText(text_holds);
    lbl_hold->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    lbl_hold->setMinimumWidth(280);
    lbl_hold->setMaximumWidth(280);
    lbl_hold->setMinimumHeight(30);
    lbl_hold->setMaximumHeight(30);
    // Set tthe text size to a bigger one and make it bold

    // Force wrapping
    lbl->setStyleSheet("QLabel { text-align: left; white-space: normal;}");

    layout->addWidget(lbl_hold, row, col);
    row++;


    for (const auto &hold : holds) {
        auto itemDetails = patron_->getItemDetails(hold.itemId);
        using namespace std::chrono;
        auto place_in_line = hold.queuePosition;
        QString text =
            QString::fromStdString(itemDetails->title + "\n") +tr("Your position in line: %1 \n").arg(place_in_line);

        QToolButton *btn = new QToolButton(content);
        btn->setText(text);
        btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        btn->setMinimumWidth(280);
        btn->setMaximumWidth(280);
        btn->setMinimumHeight(80);
        btn->setMaximumHeight(80);
        btn->setCursor(Qt::PointingHandCursor);

        // Force wrapping
        btn->setStyleSheet("QToolButton { text-align: left; white-space: normal;}");

        connect(btn, &QToolButton::clicked, this, [this, hold]() {
            onHoldClickedProfile(hold);
        });

        layout->addWidget(btn, row, col);

        if (++col == columns) {
            col = 0;
            ++row;
        }
    }
}

void HomeWindow::populateBorrowedItem(hinlibs::LoanSnapshot loan) {
    auto itemDetails = patron_->getItemDetails(loan.itemId);
    ui->returnItemAuthorLabel->setText(QString::fromStdString(itemDetails->authorOrCreator));
    ui->returnItemFormatLabel->setText(formatToString(itemDetails->format));
    ui->returnItemTitleLabel->setText(QString::fromStdString(itemDetails->title));

    using namespace std::chrono;
    auto tp = loan.dueDate;
    auto secs = time_point_cast<seconds>(tp).time_since_epoch().count();
    QDateTime dt = QDateTime::fromSecsSinceEpoch(secs, Qt::LocalTime);
    QString dueStr = dt.toString("yyyy-MM-dd");

    ui->returnItemWhenBorrowed->setText(tr("Item is due on %1").arg(dueStr));

    auto now = system_clock::now();
    auto diff = duration_cast<days>(floor<days>(loan.dueDate - now));
    auto days = (int)diff.count();
    ui->returnItemDaysLeft->setText(tr("You have %1 days to return this item").arg(QString::number(days)));

    ui->returnItemResultMessage->setText("");
}

void HomeWindow::populateHoldItem(hinlibs::HoldSnapshot hold) {
    auto itemDetails = patron_->getItemDetails(hold.itemId);
    ui->holdItemAuthorLabel->setText(QString::fromStdString(itemDetails->authorOrCreator));
    ui->holdItemFormatLabel->setText(formatToString(itemDetails->format));
    ui->holdItemTitleLabel->setText(QString::fromStdString(itemDetails->title));

    ui->holdItemResultMessage->setText("");

    ui->holdItemPositionInLine->setText(tr("Your position in line: %1").arg(hold.queuePosition));
    bool canBorrow = itemDetails->status == hinlibs::ItemStatus::Available && hold.queuePosition == 1;

    if (canBorrow) {
        ui->holdItemNoteLabel->setText("Great news! You can borrow this item now!");
        ui->borrowItemButtonHold->setEnabled(true);
    }
    else {
        if (itemDetails->status != hinlibs::ItemStatus::Available) {
            ui->holdItemNoteLabel->setText("Item can't be borrowed at the moment, someone else has it.");
        }
        else {
            ui->holdItemNoteLabel->setText(tr("Item can't be borrowed at the moment, you need to be first in line (Your position: %1)").arg(hold.queuePosition));
        }
        ui->borrowItemButtonHold->setDisabled(true);
    }
}

void HomeWindow::onHoldClickedProfile(hinlibs::HoldSnapshot hold) {
    int index = ui->NavigationWidget->indexOf(ui->manageMyHold);
    ui->NavigationWidget->setCurrentIndex(index);

    auto itemDetails = patron_->getItemDetails(hold.itemId);
    itemOnFocus = itemDetails->id;

    populateHoldItem(hold);
}

void HomeWindow::onLoanClickedProfile(hinlibs::LoanSnapshot loan) {
    int index = ui->NavigationWidget->indexOf(ui->manageMyItem);
    ui->NavigationWidget->setCurrentIndex(index);

    auto itemDetails = patron_->getItemDetails(loan.itemId);
    itemOnFocus = itemDetails->id;

    populateBorrowedItem(loan);
}

void HomeWindow::borrowItemFromHoldHandler()
{
    auto result = patron_->borrowItem(itemOnFocus);
    if (result.ok) {
        ui->holdItemResultMessage->setStyleSheet("QLabel { color: #83DC5C;}");
        ui->holdItemResultMessage->setText("Item borrowed successfully.");
    } else {
        ui->holdItemResultMessage->setStyleSheet("QLabel { color: #FF89A2; }");
        ui->holdItemResultMessage->setText(tr("Borrow failed: %1").arg(QString::fromStdString(result.message)));
    }
}
void HomeWindow::cancelHoldHandler(){
    auto result = patron_->cancelHold(itemOnFocus);
    if (result.ok) {
        ui->holdItemResultMessage->setStyleSheet("QLabel { color: #83DC5C;}");
        ui->holdItemResultMessage->setText("Hold cancelled successfully.");
    } else {
        ui->holdItemResultMessage->setStyleSheet("QLabel { color: #FF89A2; }");
        ui->holdItemResultMessage->setText(tr("Couldn't cancel the hold: %1").arg(QString::fromStdString(result.message)));
    }
}

void HomeWindow::clearLayout(QLayout* layout)
{
    if (!layout) return;

    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *w = item->widget()) {
            delete w;     // delete the widget immediately
        }
        if (QLayout *childLayout = item->layout()) {
            clearLayout(childLayout);  // delete widgets inside nested layouts
            // DO NOT delete childLayout here; just clearing its items is enough
        }
        delete item;
    }
}

void HomeWindow::goToHome()
{
    int index = ui->NavigationWidget->indexOf(ui->home);
    ui->NavigationWidget->setCurrentIndex(index);

    renderItems(ui->showCheckedOutItems->isChecked());
}

void HomeWindow::renderItems(bool all)
{
    QWidget *content = ui->itemsGridContainer;  // the widget INSIDE the scroll area

    QGridLayout *layout = qobject_cast<QGridLayout *>(content->layout());
    if (!layout) {
        // First time: create layout and set it on the container.
        layout = new QGridLayout;        // no parent here
        content->setLayout(layout);      // this gives Qt ownership
    } else {
        // Reuse existing layout: clear child widgets only.
        clearLayout(layout);
    }

    ui->scrollAreaHome->setWidgetResizable(true);

    populateHomeGrid(all);
}
QString HomeWindow::formatToString(hinlibs::ItemFormat fmt) const
{
    switch (fmt) {
    case hinlibs::ItemFormat::Book:      return "Book";
    case hinlibs::ItemFormat::Magazine:  return "Magazine";
    case hinlibs::ItemFormat::Movie:     return "Movie";
    case hinlibs::ItemFormat::VideoGame: return "Video game";
    }
    return "Unknown";
}

QString HomeWindow::statusToString(hinlibs::ItemStatus st) const
{
    switch (st) {
    case hinlibs::ItemStatus::Available:  return "Available";
    case hinlibs::ItemStatus::CheckedOut: return "Checked out";
    }
    return "Unknown";
}

void HomeWindow::populateHomeGrid(bool all)
{
    if (!patron_) return;

    QWidget *content = ui->itemsGridContainer;
    QGridLayout *layout = qobject_cast<QGridLayout *>(content->layout());
    if (!layout) return;

    auto items = all ? patron_->browseCatalogueAll() : patron_->browseCatalogue();

    const int columns = 2;
    int row = 0;
    int col = 0;

    for (const auto &item : items) {
        QString text =
            QString::fromStdString(item.title + "\nBy " + item.authorOrCreator)
            + "\nFormat: " + formatToString(item.format)
            + "\nAvailability: " + statusToString(item.status)
            // 👇 NEW LINE: show Item ID for librarian to use
            + "\nID: " + QString::fromStdString(item.id);

        QToolButton *btn = new QToolButton(content);
        btn->setText(text);
        btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        btn->setMinimumWidth(280);
        btn->setMaximumWidth(280);
        btn->setMinimumHeight(80);
        btn->setMaximumHeight(80);
        btn->setCursor(Qt::PointingHandCursor);

        btn->setStyleSheet("QToolButton { text-align: left; white-space: normal;}");

        connect(btn, &QToolButton::clicked, this, [this, item]() {
            onItemClickedHome(item);
        });

        layout->addWidget(btn, row, col);

        if (++col == columns) {
            col = 0;
            ++row;
        }
    }
}


void HomeWindow::onItemClickedHome(hinlibs::ItemDetails itemDetails)
{
    int index = ui->NavigationWidget->indexOf(ui->borrowItem);
    ui->NavigationWidget->setCurrentIndex(index);
    itemOnFocus = itemDetails.id;

    populateBorrowItem(itemDetails);
}

void HomeWindow::populateBorrowItem(hinlibs::ItemDetails itemDetails)
{

    ui->borrowItemTitleLabel->setText(QString::fromStdString(itemDetails.title));
    ui->borrowItemAuthorLabel->setText(QString::fromStdString(itemDetails.authorOrCreator));
    ui->borrowItemStatusLabel->setText(statusToString(itemDetails.status));
    ui->borrowItemFormatLabel->setText(formatToString(itemDetails.format));
    ui->borrowItemDescriptionLabel->setText(formatExtraDetails(itemDetails));
    ui->borrowItemResultMessage->setText("");
}
//HomeWindow::~HomeWindow()
//{
//    delete ui;
//}

QString HomeWindow::formatExtraDetails(const hinlibs::ItemDetails& details) const
{
    QStringList lines;

    if (details.publicationYear) {
        lines << tr("Publication year: %1").arg(*details.publicationYear);
    }

    if (details.isbn) {
        lines << tr("ISBN: %1").arg(QString::fromStdString(*details.isbn));
    }

    if (details.deweyDecimal) {
        lines << tr("Dewey Decimal: %1").arg(QString::fromStdString(*details.deweyDecimal));
    }

    if (details.genre) {
        lines << tr("Genre: %1").arg(QString::fromStdString(*details.genre));
    }

    if (details.rating) {
        lines << tr("Rating: %1").arg(QString::fromStdString(*details.rating));
    }

    if (details.issueNumber) {
        lines << tr("Issue: %1").arg(QString::fromStdString(*details.issueNumber));
    }

    if (details.publicationDate) {
        // Convert std::chrono::system_clock::time_point -> QDateTime
        using namespace std::chrono;
        auto tp = *details.publicationDate;
        auto tt = system_clock::to_time_t(tp);

        // Use local time; you can switch to Qt::UTC if you prefer
        QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(tt), Qt::LocalTime);
        QString dateStr = dt.toString("yyyy-MM-dd"); // or whatever format you like

        lines << tr("Publication date: %1").arg(dateStr);
    }

    // If nothing is set, you can return an empty string or something like "No extra details".
    if (lines.isEmpty()) {
        return QString(); // or: return tr("No additional details");
    }

    return lines.join("\n");
}

void  HomeWindow::borrowItemHandler()
{
    auto result = patron_->borrowItem(itemOnFocus);
    if (result.ok) {
        ui->borrowItemResultMessage->setStyleSheet("QLabel { color: #83DC5C;}");
        ui->borrowItemResultMessage->setText("Item borrowed successfully.");
    } else {
        ui->borrowItemResultMessage->setStyleSheet("QLabel { color: #FF89A2; }");
        ui->borrowItemResultMessage->setText(tr("Borrow failed: %1").arg(QString::fromStdString(result.message)));
    }
}

void  HomeWindow::placeHoldHandler()
{
    auto result = patron_->placeHold(itemOnFocus);
    if (result.ok) {
        ui->borrowItemResultMessage->setStyleSheet("QLabel { color: #83DC5C;}");
        ui->borrowItemResultMessage->setText(QString("Hold placed. Queue position: %1").arg(result.value.value()));
    } else {
        ui->borrowItemResultMessage->setStyleSheet("QLabel { color: #FF89A2; }");
        ui->borrowItemResultMessage->setText(tr("Placing hold failed: %1").arg(QString::fromStdString(result.message)));
    }
}

void  HomeWindow::returnItemHandler()
{
    auto result = patron_->returnItem(itemOnFocus);
    if (result.ok) {
        ui->returnItemResultMessage->setStyleSheet("QLabel { color: #83DC5C;}");
        ui->returnItemResultMessage->setText("Item has been returned, thank you!");
    } else {
        ui->returnItemResultMessage->setStyleSheet("QLabel { color: #FF89A2; }");
        ui->returnItemResultMessage->setText(tr("Returning this item failed: %1").arg(QString::fromStdString(result.message)));
    }
}

void HomeWindow::onReloadClickedHome()
{
    renderItems(ui->showCheckedOutItems->isChecked());
}

void HomeWindow::logOutHandler()
{
    session_->signOut();
    patron_ = nullptr;
    emit logoutRequest();
}
