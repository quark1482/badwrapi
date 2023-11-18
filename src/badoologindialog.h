#ifndef BADOOLOGINDIALOG_H
#define BADOOLOGINDIALOG_H

#include <QDialog>
#include <QtWidgets>
#include <QMessageBox>
#include "badoowrapper.h"

class BadooWrapper;

class BadooLoginDialog:public QDialog {
public:
    BadooLoginDialog(BadooWrapper *,QWidget * =nullptr);
    bool show();
public slots:
    void okButtonClicked(bool);
private:
    QVBoxLayout  vblMain;
    QHBoxLayout  hblUser,hblPass,hblButtons;
    QLabel       lblUser,lblPass;
    QLineEdit    ledUser,ledPass;
    QPushButton  btnOK,btnCancel;
    BadooWrapper *bwWrapper;
};

#endif // BADOOLOGINDIALOG_H
