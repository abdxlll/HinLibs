#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "mainwindow.h"
#include "database.h"
#include "session.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

//     Open SQLite database file
    QSqlDatabase sqlDb = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QCoreApplication::applicationDirPath() + "/hinlibs.sqlite3";
    sqlDb.setDatabaseName(dbPath);

    if (!sqlDb.open()) {
        qDebug() << "Failed to open SQLite database:";
    } else {
        qDebug() << "Database opened successfully";
        qDebug() << "Database path:" << sqlDb.databaseName();
    }

    // Wrap in your Database and Session classes
    auto database = std::make_shared<hinlibs::Database>(sqlDb);
    auto session  = std::make_shared<hinlibs::Session>(database);

    MainWindow w(session);
    w.show();

    return app.exec();
}
