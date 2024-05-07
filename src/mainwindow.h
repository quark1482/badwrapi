#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "badoowrapper.h"
#include "browsefolderdialog.h"
#include "db.h"
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
    bool eventFilter(QObject *,QEvent *) override;
private slots:
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
                         *dlgBrowseVisitors,
                         *dlgBrowseBlocked;
    PlayEncountersDialog *dlgEncounters;
    QMdiArea             mdiArea;
    BadooWrapper         bwMain;
    DB                   *dbMain;
    bool anyChildrenActive();
    bool getCustomFolderParameters(BadooFolderType &,BadooListSectionType &,BadooListFilterList &,int &);
    bool postInit();
    bool setupDB(QString &);
    void showChildDialog(SettingsGroup,QDialog *);
    void showCustomProfile(QString);
    void showSettings(BadooSettingsContextType);
};
#endif // MAINWINDOW_H
