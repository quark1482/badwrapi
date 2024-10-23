#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "badoowrapper.h"
#include "browsefolderdialog.h"
#include "db.h"
#include "playencountersdialog.h"

// Comment out this line to disable the custom HTTP user-agent:
#define HTTP_USER_AGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Safari/537.36"

// Comment out this line to disable the custom HTTP proxy support:
// #define HTTP_PROXY_HOST "127.0.0.1"
// #define HTTP_PROXY_PORT 8080

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
    void menuEditSessionTriggered(bool);
    void menuEncountersTriggered(bool);
    void menuLocationTriggered(bool);
    void menuLoginTriggered(bool);
    void menuLogoutTriggered(bool);
    void menuEncountersSettingsTriggered(bool);
    void menuPeopleNearbySettingsTriggered(bool);
    void menuVisibilityTriggered(bool);
    void wrapperStatusChanged(QString);
private:
    Ui::MainWindow       *ui;
    BrowseFolderDialog   *dlgBrowseCustom,
                         *dlgBrowseChats,
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
    QNetworkProxy        *pxyMain;
    bool anyChildrenActive();
    bool editSessionDetails(QString &,QString &,QString &,QString &);
    bool getCustomFolderParameters(BadooFolderType &,BadooListSectionType &,BadooListFilterList &,int &);
    bool postInit();
    bool setupDB(QString &);
    void showChildDialog(SettingsGroup,QDialog *);
    void showCustomProfile(QString);
    void showSettings(BadooSettingsContextType);
    void testAndSetLocation(bool=false);
    void testAndSetVisibility(bool=false);
};
#endif // MAINWINDOW_H
