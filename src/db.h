#ifndef DB_H
#define DB_H

#include <QtCore>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#define CREATE_SESSION R"(
    CREATE TABLE Session (
        SessionId     TEXT NOT NULL,
        DeviceId      TEXT NOT NULL,
        UserId        TEXT NOT NULL,
        AccountId     TEXT NOT NULL
    );
)"

typedef enum {
    stgFIRST=0,
    stgMAIN=stgFIRST,
    stgENCOUNTERS,
    stgFOLDERS,
    stgCUSTOM,
    stgPROFILE,
    stgTOTAL
} SettingsGroup;

typedef enum {
    wgsNORMAL,
    wgsMINIMIZED,
    wgsMAXIMIZED
} WidgetStatus;

typedef struct {
    QRect        recRect;
    WidgetStatus wgsStatus;
} WidgetGeometry;

class DB {
public:
    DB();
    ~DB();
    void    close();
    bool    create();
    bool    exists();
    QString getLastError();
    bool    loadSession(QString &,QString &,QString &,QString &);
    bool    loadSetting(SettingsGroup,WidgetGeometry &);
    bool    loadSettings(WidgetGeometry &,WidgetGeometry &,WidgetGeometry &,WidgetGeometry &,WidgetGeometry &);
    bool    open();
    bool    saveSession(QString,QString,QString,QString);
    bool    saveSetting(SettingsGroup,WidgetGeometry);
    bool    saveSettings(WidgetGeometry,WidgetGeometry,WidgetGeometry,WidgetGeometry,WidgetGeometry);
    void    setFilename(QString);
private:
    QString      sDBFile,
                 sLastError;
    QSqlDatabase dbDatabase;
    QString getSettingsGroupName(SettingsGroup);
};

#endif // DB_H
