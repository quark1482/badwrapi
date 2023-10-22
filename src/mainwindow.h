#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "badoowrapper.h"
#include "browsefolderdialog.h"
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
    void dialogBrowseFolderDestroyed();
    void dialogEncountersDestroyed();
    void menuBrowseFolderTriggered(bool);
    void menuBrowseProfileTriggered(bool);
    void menuEncountersTriggered(bool);
    void menuLoginTriggered(bool);
    void menuLogoutTriggered(bool);
    void menuEncountersSettingsTriggered(bool);
    void menuPeopleNearbySettingsTriggered(bool);
    void wrapperStatusChanged(QString);
private:
    Ui::MainWindow       *ui;
    BrowseFolderDialog   *dlgBrowseCustom,
                         *dlgBrowseFavorites,
                         *dlgBrowseLikes,
                         *dlgBrowseMatches,
                         *dlgBrowsePeopleNearby,
                         *dlgBrowseVisitors;
    PlayEncountersDialog *dlgEncounters;
    QMdiArea             mdiArea;
    BadooWrapper         bwMain;
    bool anyChildrenActive();
    bool getCustomFolderParameters(BadooFolderType &,BadooListSectionType &,BadooListFilterList &,int &);
    void showCustomProfile(QString);
    void showSettings(BadooSettingsContextType);
};
#endif // MAINWINDOW_H
