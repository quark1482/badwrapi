#ifndef BADOOSEARCHSETTINGSDIALOG_H
#define BADOOSEARCHSETTINGSDIALOG_H

#include <QDialog>
#include <QtWidgets>
#include <QtQuickWidgets>
#include <QMessageBox>
#include "badoowrapper.h"

class BadooWrapper;

class BadooSearchSettingsDialog:public QDialog {
    Q_OBJECT
public:
    BadooSearchSettingsDialog(BadooSettingsContextType,BadooWrapper *,QWidget * =nullptr);
    bool show();
public slots:
    void ageRangeSliderChanged();
    void distanceSliderChanged(int);
    void locationItemSelected(int);
    void locationTextEdited(const QString &);
    void locationTextEditTimeout();
    void okButtonClicked(bool);
private:
    QTimer                   tmrLocation;
    QVBoxLayout              vblMain,vblIntent,vblShow,vblAge,vblLocation,vblDistance;
    QHBoxLayout              hblShow,hblAge,hblDistance,hblButtons;
    QLabel                   lblIntent,lblShow,lblAge,lblAgeRange,lblLocation,lblDistance,lblDistanceAway;
    QComboBox                cboIntent,cboLocation,cboDistance;
    QPushButton              btnGuys,btnGirls,btnBoth,btnOK,btnCancel;
    QSlider                  sldDistance;
    QQuickWidget             qkwAge;
    QString                  sCurrentLocationCode;
    QObject                  *objFirstHandle,*objSecondHandle;
    BadooSettingsContextType bsctContext;
    BadooWrapper             *bwWrapper;
};

#endif // BADOOSEARCHSETTINGSDIALOG_H
