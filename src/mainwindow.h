#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "badoowrapper.h"
#include "playencountersdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow:public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget * =nullptr);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *) override;
private slots:
    void dialogEncountersDestroyed();
    void menuPlayEncountersTriggered(bool);
    void menuLoginTriggered(bool);
    void menuLogoutTriggered(bool);
    void menuEncountersSettingsTriggered(bool);
    void menuPeopleNearbySettingsTriggered(bool);
    void wrapperStatusChanged(QString);
private:
    Ui::MainWindow       *ui;
    PlayEncountersDialog *dlgEncounters;
    QMdiArea             mdiArea;
    BadooWrapper         bwMain;
    bool anyChildrenActive();
    void showSettings(BadooSettingsContextType);
};
#endif // MAINWINDOW_H
