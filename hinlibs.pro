QT += core gui widgets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    database.cpp \
    hold.cpp \
    homewindow.cpp \
    item.cpp \
    librarian.cpp \
    librarianwindow.cpp \
    loan.cpp \
    main.cpp \
    mainwindow.cpp \
    patron.cpp \
    patronwindow.cpp \
    session.cpp \
    sysadmin.cpp \
    sysadminwindow.cpp \
    user.cpp

HEADERS += \
    database.h \
    hold.h \
    homewindow.h \
    item.h \
    librarian.h \
    librarianwindow.h \
    loan.h \
    mainwindow.h \
    patron.h \
    patronwindow.h \
    session.h \
    sysadmin.h \
    sysadminwindow.h \
    types.h \
    user.h

FORMS += \
    homewindow.ui \
    librarianwindow.ui \
    mainwindow.ui \
    patronwindow.ui \
    sysadminwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

unix {
    QMAKE_POST_LINK += cp $$shell_path($$PWD/hinlibs.sqlite3) $$shell_path($$OUT_PWD)
}
