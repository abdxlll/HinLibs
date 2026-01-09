#pragma once

#include <QMainWindow>
#include <string>
#include "types.h"

namespace hinlibs {
    class Database;
}

namespace Ui {
class librarianWindow;
}

class librarianWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit librarianWindow(const std::string &username_display,
                             QWidget *parent = nullptr);

    void setDatabase(hinlibs::Database* db) { db_ = db; }

signals:
    void logoutRequest();

private slots:
    // Dashboard buttons
    void logOutHandler();
    void on_addItemButton_clicked();
    void on_removeItemButton_clicked();
    void on_returnItemButton_clicked();

    // Add Item page
    void on_BackFromAddItem_clicked();
    void on_addItemConfirmButton_clicked();

    // Remove Item page
    void on_BackFromRemoveItem_clicked();
    void on_removeItemConfirmButton_clicked();

    // Return Item page
    void on_BackFromReturnItem_clicked();
    void on_findPatronButton_clicked();         // <--- NEW
    void on_returnSelectedItemButton_clicked(); // <--- NEW

private:
    Ui::librarianWindow *ui;

    hinlibs::Database* db_ { nullptr };

    // Stores the selected patron ID while returning items
    std::string currentReturnPatronId;

    // Helpers for Return Item page
    void clearReturnTable();   // <--- NEW
    void fillReturnTable(const std::vector<hinlibs::LoanSnapshot>& loans); // <--- NEW
};
