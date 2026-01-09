#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QWidget>
#include "patron.h"
#include "session.h"
#include <QMainWindow>

namespace Ui {
class HomeWindow;
}

class HomeWindow : public QWidget
{
    Q_OBJECT

public:
    explicit HomeWindow(std::shared_ptr<hinlibs::Patron> patron, std::shared_ptr<hinlibs::Session> session, QWidget *parent = nullptr);
//    ~HomeWindow();

private:
    Ui::HomeWindow *ui;
    std::shared_ptr<hinlibs::Patron> patron_;
    std::shared_ptr<hinlibs::Session> session_;

    QString formatToString(hinlibs::ItemFormat fmt) const;
    QString statusToString(hinlibs::ItemStatus st) const;
    void renderItems(bool all);
    void renderProfile();

    void populateHomeGrid(bool all);
    void populateBorrowedItem(hinlibs::LoanSnapshot loan);
    void populateHoldItem(hinlibs::HoldSnapshot hold);
    void clearLayout(QLayout* layout);
    void populateBorrowItem(hinlibs::ItemDetails itemDetails);
    void populateProfileGrid();

    QString formatExtraDetails(const hinlibs::ItemDetails& details) const;

    hinlibs::ItemId itemOnFocus;

private slots:
    void goToProfile();
    void goToHome();
    void onItemClickedHome(hinlibs::ItemDetails itemDetails);
    void onReloadClickedHome();

    void onLoanClickedProfile(hinlibs::LoanSnapshot loanDetails);
    void onHoldClickedProfile(hinlibs::HoldSnapshot holdDetails);

    void borrowItemHandler();
    void placeHoldHandler();
    void logOutHandler();
    void returnItemHandler();

    void borrowItemFromHoldHandler();
    void cancelHoldHandler();

signals:
    void logoutRequest();

};

#endif // HOMEWINDOW_H
