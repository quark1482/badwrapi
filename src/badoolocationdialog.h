#ifndef BADOOLOCATIONDIALOG_H
#define BADOOLOCATIONDIALOG_H

#include <QDialog>
#include <QtWidgets>
#include <QMessageBox>
#include "badoowrapper.h"

class BadooWrapper;

class BadooLocationDialog:public QDialog {
    Q_OBJECT
public:
    BadooLocationDialog(BadooWrapper *,QWidget * =nullptr);
    bool show();
public slots:
    void locationItemSelected(int);
    void locationTextEdited(const QString &);
    void locationTextEditTimeout();
    void okButtonClicked(bool);
private:
    QTimer       tmrLocation;
    QVBoxLayout  vblMain,vblLocation;
    QHBoxLayout  hblButtons;
    QLabel       lblLocation;
    QComboBox    cboLocation;
    QPushButton  btnOK,btnCancel;
    QString      sCurrentLocationCode;
    BadooWrapper *bwWrapper;
};

#endif // BADOOLOCATIONDIALOG_H
