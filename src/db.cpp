#include "db.h"

DB::DB() {
    sDBFile.clear();
    sLastError.clear();
    dbDatabase=QSqlDatabase::addDatabase(
        QStringLiteral("QSQLITE"),
        QStringLiteral("SQLite")
    );
}

DB::~DB() {
    if(dbDatabase.isOpen())
        dbDatabase.close();
}

void DB::close() {
    dbDatabase.close();
}

bool DB::create() {
    bool bResult=false;
    sLastError.clear();
    if(dbDatabase.isOpen())
        sLastError=QStringLiteral("The DB is already open");
    else if(sDBFile.isEmpty())
        sLastError=QStringLiteral("The DB filename has not been set");
    else if(this->exists())
        sLastError=QStringLiteral("The DB file already exists");
    else {
        dbDatabase.setDatabaseName(sDBFile);
        if(!dbDatabase.open())
            sLastError=QStringLiteral("Unable to open the DB file: %1").
                       arg(dbDatabase.lastError().text());
        else if(!dbDatabase.transaction())
            sLastError=QStringLiteral("The DB transaction(create) did not start: %1").
                       arg(dbDatabase.lastError().text());
        else {
            bool        bExec=true;
            QString     sSQL;
            QStringList slFields,
                        slScripts;
            QSqlQuery   qryQuery(dbDatabase);
            for(int iG=stgFIRST;iG<stgTOTAL;iG++) {
                SettingsGroup stgGroup=static_cast<SettingsGroup>(iG);
                slFields.append(QStringLiteral("%1X").arg(getSettingsGroupName(stgGroup)));
                slFields.append(QStringLiteral("%1Y").arg(getSettingsGroupName(stgGroup)));
                slFields.append(QStringLiteral("%1Width").arg(getSettingsGroupName(stgGroup)));
                slFields.append(QStringLiteral("%1Height").arg(getSettingsGroupName(stgGroup)));
                slFields.append(QStringLiteral("%1Status").arg(getSettingsGroupName(stgGroup)));
            }
            sSQL.clear();
            for(const auto &s:slFields) {
                if(!sSQL.isEmpty())
                    sSQL.append(QStringLiteral(","));
                sSQL.append(QStringLiteral("%1 INTEGER").arg(s));
            }
            sSQL=QStringLiteral("CREATE TABLE Settings (%1);").arg(sSQL);
            slScripts.append(sSQL);
            sSQL.clear();
            for(const auto &s:slFields) {
                if(!sSQL.isEmpty())
                    sSQL.append(QStringLiteral(","));
                sSQL.append(QStringLiteral("NULL"));
            }
            sSQL=QStringLiteral("INSERT INTO Settings VALUES(%1);").arg(sSQL);
            slScripts.append(sSQL);
            slScripts.append(CREATE_SESSION);
            for(const auto &s:slScripts)
                if(!qryQuery.exec(s)) {
                    bExec=false;
                    break;
                }
            if(!bExec)
                sLastError=QStringLiteral("The script create() failed: %1").
                           arg(qryQuery.lastError().text());
            else if(!dbDatabase.commit())
                sLastError=QStringLiteral("The DB transaction(create) did not commit: %1").
                           arg(dbDatabase.lastError().text());
            else
                bResult=true;
            if(!bResult)
                (void)dbDatabase.rollback();
        }
        if(dbDatabase.isOpen())
            dbDatabase.close();
    }
    return bResult;
}

bool DB::exists() {
    return QDir().exists(sDBFile);
}

QString DB::getLastError() {
    return sLastError;
}

bool DB::loadSession(QString &sSessionId,
                     QString &sDeviceId,
                     QString &sUserId,
                     QString &sAccountId,
                     QString &sResponseToken) {
    bool bResult=false;
    sLastError.clear();
    sSessionId.clear();
    sDeviceId.clear();
    sUserId.clear();
    sAccountId.clear();
    sResponseToken.clear();
    if(!dbDatabase.isOpen())
        sLastError=QStringLiteral("The DB is not open");
    else {
        QString   sSQL;
        QSqlQuery qryQuery(dbDatabase);
        sSQL=QStringLiteral(
            "Select SessionId,"
                   "DeviceId,"
                   "UserId,"
                   "AccountId,"
                   "ResponseToken "
            "From Session"
        );
        if(!qryQuery.exec(sSQL))
            sLastError=QStringLiteral("The script loadSession(select) failed: %1").
                       arg(qryQuery.lastError().text());
        else {
            if(qryQuery.first()) {
                sSessionId=qryQuery.value(0).toString();
                sDeviceId=qryQuery.value(1).toString();
                sUserId=qryQuery.value(2).toString();
                sAccountId=qryQuery.value(3).toString();
                sResponseToken=qryQuery.value(4).toString();
            }
            bResult=true;
        }
    }
    return bResult;
}

bool DB::loadSetting(SettingsGroup  stgGroup,
                     WidgetGeometry &wggGeometry) {
    bool bResult=false;
    sLastError.clear();
    wggGeometry.recRect=QRect();
    wggGeometry.wgsStatus=static_cast<WidgetStatus>(0);
    if(!dbDatabase.isOpen())
        sLastError=QStringLiteral("The DB is not open");
    else {
        QString   sSQL;
        QSqlQuery qryQuery(dbDatabase);
        sSQL=QStringLiteral(
            "Select %1X,"
                   "%1Y,"
                   "%1Width,"
                   "%1Height,"
                   "%1Status "
            "From Settings"
        ).arg(getSettingsGroupName(stgGroup));
        if(!qryQuery.exec(sSQL))
            sLastError=QStringLiteral("The script loadSetting(select) failed: %1").
                       arg(qryQuery.lastError().text());
        else {
            if(qryQuery.first()) {
                wggGeometry.recRect.setRect(
                    qryQuery.value(0).toInt(),
                    qryQuery.value(1).toInt(),
                    qryQuery.value(2).toInt(),
                    qryQuery.value(3).toInt()
                );
                wggGeometry.wgsStatus=static_cast<WidgetStatus>(qryQuery.value(4).toInt());
            }
            bResult=true;
        }
    }
    return bResult;
}

bool DB::loadSettings(WidgetGeometry &wggMain,
                      WidgetGeometry &wggEncounters,
                      WidgetGeometry &wggFolders,
                      WidgetGeometry &wggCustom,
                      WidgetGeometry &wggProfile) {
    bool bResult=false;
    sLastError.clear();
    wggMain.recRect=QRect();
    wggMain.wgsStatus=static_cast<WidgetStatus>(0);
    wggEncounters.recRect=QRect();
    wggEncounters.wgsStatus=static_cast<WidgetStatus>(0);
    wggFolders.recRect=QRect();
    wggFolders.wgsStatus=static_cast<WidgetStatus>(0);
    wggCustom.recRect=QRect();
    wggCustom.wgsStatus=static_cast<WidgetStatus>(0);
    if(!dbDatabase.isOpen())
        sLastError=QStringLiteral("The DB is not open");
    else {
        QString   sSQL,sFields;
        QSqlQuery qryQuery(dbDatabase);
        for(int iG=stgFIRST;iG<stgTOTAL;iG++) {
            SettingsGroup stgGroup=static_cast<SettingsGroup>(iG);
            if(stgFIRST<stgGroup)
                sFields.append(QStringLiteral(","));
            sFields.append(QStringLiteral("%1X").arg(getSettingsGroupName(stgGroup)));
            sFields.append(QStringLiteral(",%1Y").arg(getSettingsGroupName(stgGroup)));
            sFields.append(QStringLiteral(",%1Width").arg(getSettingsGroupName(stgGroup)));
            sFields.append(QStringLiteral(",%1Height").arg(getSettingsGroupName(stgGroup)));
            sFields.append(QStringLiteral(",%1Status").arg(getSettingsGroupName(stgGroup)));
        }
        sSQL=QStringLiteral(
            "Select %1 "
            "From Settings"
        ).arg(sFields);
        if(!qryQuery.exec(sSQL))
            sLastError=QStringLiteral("The script loadSettings(select) failed: %1").
                       arg(qryQuery.lastError().text());
        else {
            if(qryQuery.first()) {
                wggMain.recRect.setRect(
                    qryQuery.value(0).toInt(),
                    qryQuery.value(1).toInt(),
                    qryQuery.value(2).toInt(),
                    qryQuery.value(3).toInt()
                );
                wggMain.wgsStatus=static_cast<WidgetStatus>(qryQuery.value(4).toInt());
                wggEncounters.recRect.setRect(
                    qryQuery.value(5).toInt(),
                    qryQuery.value(6).toInt(),
                    qryQuery.value(7).toInt(),
                    qryQuery.value(8).toInt()
                );
                wggEncounters.wgsStatus=static_cast<WidgetStatus>(qryQuery.value(9).toInt());
                wggFolders.recRect.setRect(
                    qryQuery.value(10).toInt(),
                    qryQuery.value(11).toInt(),
                    qryQuery.value(12).toInt(),
                    qryQuery.value(13).toInt()
                );
                wggFolders.wgsStatus=static_cast<WidgetStatus>(qryQuery.value(14).toInt());
                wggCustom.recRect.setRect(
                    qryQuery.value(15).toInt(),
                    qryQuery.value(16).toInt(),
                    qryQuery.value(17).toInt(),
                    qryQuery.value(18).toInt()
                );
                wggCustom.wgsStatus=static_cast<WidgetStatus>(qryQuery.value(19).toInt());
                wggProfile.recRect.setRect(
                    qryQuery.value(20).toInt(),
                    qryQuery.value(21).toInt(),
                    qryQuery.value(22).toInt(),
                    qryQuery.value(23).toInt()
                );
                wggProfile.wgsStatus=static_cast<WidgetStatus>(qryQuery.value(24).toInt());
            }
            bResult=true;
        }
    }
    return bResult;
}

bool DB::open() {
    bool bResult=false;
    sLastError.clear();
    if(dbDatabase.isOpen())
        sLastError=QStringLiteral("The DB is already open");
    else if(sDBFile.isEmpty())
        sLastError=QStringLiteral("The DB filename has not been set");
    else if(!this->exists())
        sLastError=QStringLiteral("The DB file does not exist");
    else {
        dbDatabase.setDatabaseName(sDBFile);
        if(!dbDatabase.open())
            sLastError=QStringLiteral("Unable to open the DB file: %1").
                       arg(dbDatabase.lastError().text());
        bResult=dbDatabase.isOpen();
    }
    return bResult;
}

bool DB::saveSession(QString sSessionId,
                     QString sDeviceId,
                     QString sUserId,
                     QString sAccountId,
                     QString sResponseToken) {
    bool bResult=false,
         bClear=sSessionId.isEmpty()&&
                sDeviceId.isEmpty()&&
                sUserId.isEmpty()&&
                sAccountId.isEmpty()&&
                sResponseToken.isEmpty();
    sLastError.clear();
    if(!dbDatabase.isOpen())
        sLastError=QStringLiteral("The DB is not open");
    else if(!dbDatabase.transaction())
        sLastError=QStringLiteral("The DB transaction(saveSession) did not start: %1").
                   arg(dbDatabase.lastError().text());
    else {
        QString   sSQL;
        QSqlQuery qryQuery(dbDatabase);
        sSQL=QStringLiteral(
            "Delete "
            "From Session"
        );
        if(!qryQuery.exec(sSQL))
            sLastError=QStringLiteral("The script saveSession(delete) failed: %1").
                       arg(qryQuery.lastError().text());
        else if(!bClear) {
            sSQL=QStringLiteral(
                "Insert Into Session ("
                    "SessionId,"
                    "DeviceId,"
                    "UserId,"
                    "AccountId,"
                    "ResponseToken"
                ") "
                "Values (?,?,?,?,?)"
            );
            qryQuery.prepare(sSQL);
            qryQuery.addBindValue(sSessionId);
            qryQuery.addBindValue(sDeviceId);
            qryQuery.addBindValue(sUserId);
            qryQuery.addBindValue(sAccountId);
            qryQuery.addBindValue(sResponseToken);
            if(!qryQuery.exec())
                sLastError=QStringLiteral("The script saveSession(insert) failed: %1").
                           arg(qryQuery.lastError().text());
            else
                bResult=true;
        }
        else
            bResult=true;
        if(bResult)
            if(!dbDatabase.commit()) {
                sLastError=QStringLiteral("The DB transaction(saveSession) did not commit: %1").
                           arg(dbDatabase.lastError().text());
                (void)dbDatabase.rollback();
                bResult=false;
            }
    }
    return bResult;
}

bool DB::saveSetting(SettingsGroup  stgGroup,
                     WidgetGeometry wggGeometry) {
    bool bResult=false;
    sLastError.clear();
    if(!dbDatabase.isOpen())
        sLastError=QStringLiteral("The DB is not open");
    else {
        QString   sSQL;
        QSqlQuery qryQuery(dbDatabase);
        sSQL=QStringLiteral(
           "Update Settings "
           "Set %1X=?,"
               "%1Y=?,"
               "%1Width=?,"
               "%1Height=?,"
               "%1Status=?"
        ).arg(getSettingsGroupName(stgGroup));
        qryQuery.prepare(sSQL);
        qryQuery.addBindValue(wggGeometry.recRect.x());
        qryQuery.addBindValue(wggGeometry.recRect.y());
        qryQuery.addBindValue(wggGeometry.recRect.width());
        qryQuery.addBindValue(wggGeometry.recRect.height());
        qryQuery.addBindValue((int)wggGeometry.wgsStatus);
        if(!qryQuery.exec())
            sLastError=QStringLiteral("The script saveSetting(update) failed: %1").
                       arg(qryQuery.lastError().text());
        else
            bResult=true;
    }
    return bResult;
}

bool DB::saveSettings(WidgetGeometry wggMain,
                      WidgetGeometry wggEncounters,
                      WidgetGeometry wggFolders,
                      WidgetGeometry wggCustom,
                      WidgetGeometry wggProfile) {
    bool bResult=false;
    sLastError.clear();
    if(!dbDatabase.isOpen())
        sLastError=QStringLiteral("The DB is not open");
    else if(!dbDatabase.transaction())
        sLastError=QStringLiteral("The DB transaction(saveSettings) did not start: %1").
                   arg(dbDatabase.lastError().text());
    else {
        int          iK;
        QString      sSQL;
        QStringList  slFields;
        QVariantList vlValues={
            wggMain.recRect.x(),
            wggMain.recRect.y(),
            wggMain.recRect.width(),
            wggMain.recRect.height(),
            (int)wggMain.wgsStatus,
            wggEncounters.recRect.x(),
            wggEncounters.recRect.y(),
            wggEncounters.recRect.width(),
            wggEncounters.recRect.height(),
            (int)wggEncounters.wgsStatus,
            wggFolders.recRect.x(),
            wggFolders.recRect.y(),
            wggFolders.recRect.width(),
            wggFolders.recRect.height(),
            (int)wggFolders.wgsStatus,
            wggCustom.recRect.x(),
            wggCustom.recRect.y(),
            wggCustom.recRect.width(),
            wggCustom.recRect.height(),
            (int)wggCustom.wgsStatus,
            wggProfile.recRect.x(),
            wggProfile.recRect.y(),
            wggProfile.recRect.width(),
            wggProfile.recRect.height(),
            (int)wggProfile.wgsStatus
        };
        QList<bool>  blIsNull={
            wggMain.recRect.isNull(),
            wggMain.recRect.isNull(),
            wggMain.recRect.isNull(),
            wggMain.recRect.isNull(),
            wggMain.recRect.isNull(),
            wggEncounters.recRect.isNull(),
            wggEncounters.recRect.isNull(),
            wggEncounters.recRect.isNull(),
            wggEncounters.recRect.isNull(),
            wggEncounters.recRect.isNull(),
            wggFolders.recRect.isNull(),
            wggFolders.recRect.isNull(),
            wggFolders.recRect.isNull(),
            wggFolders.recRect.isNull(),
            wggFolders.recRect.isNull(),
            wggCustom.recRect.isNull(),
            wggCustom.recRect.isNull(),
            wggCustom.recRect.isNull(),
            wggCustom.recRect.isNull(),
            wggCustom.recRect.isNull(),
            wggProfile.recRect.isNull(),
            wggProfile.recRect.isNull(),
            wggProfile.recRect.isNull(),
            wggProfile.recRect.isNull(),
            wggProfile.recRect.isNull()
        };
        QSqlQuery    qryQuery(dbDatabase);
        for(int iG=stgFIRST;iG<stgTOTAL;iG++) {
            SettingsGroup stgGroup=static_cast<SettingsGroup>(iG);
            slFields.append(QStringLiteral("%1X").arg(getSettingsGroupName(stgGroup)));
            slFields.append(QStringLiteral("%1Y").arg(getSettingsGroupName(stgGroup)));
            slFields.append(QStringLiteral("%1Width").arg(getSettingsGroupName(stgGroup)));
            slFields.append(QStringLiteral("%1Height").arg(getSettingsGroupName(stgGroup)));
            slFields.append(QStringLiteral("%1Status").arg(getSettingsGroupName(stgGroup)));
        }
        for(iK=0;iK<slFields.count();iK++)
            if(!blIsNull.at(iK)) {
                sSQL=QStringLiteral(
                    "Update Settings "
                    "Set %1=?"
                ).arg(slFields.at(iK));
                qryQuery.prepare(sSQL);
                qryQuery.addBindValue(vlValues.at(iK));
                if(!qryQuery.exec())
                    break;
            }
        if(iK<slFields.count())
            sLastError=QStringLiteral("The script saveSettings(update) failed: %1").
                       arg(qryQuery.lastError().text());
        else if(!dbDatabase.commit())
            sLastError=QStringLiteral("The DB transaction(saveSettings) did not commit: %1").
                       arg(dbDatabase.lastError().text());
        else
            bResult=true;
        if(!bResult)
            (void)dbDatabase.rollback();
    }
    return bResult;
}

void DB::setFilename(QString sFilename) {
    sDBFile=sFilename;
}

QString DB::getSettingsGroupName(SettingsGroup stgGroup) {
    QString sResult;
    switch(stgGroup) {
        case stgMAIN:
            sResult=QStringLiteral("Main");
            break;
        case stgENCOUNTERS:
            sResult=QStringLiteral("Encounters");
            break;
        case stgFOLDERS:
            sResult=QStringLiteral("Folders");
            break;
        case stgCUSTOM:
            sResult=QStringLiteral("Custom");
            break;
        case stgPROFILE:
            sResult=QStringLiteral("Profile");
            break;
    }
    return sResult;
}
