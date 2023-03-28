#ifndef BADOOWRAPPER_H
#define BADOOWRAPPER_H

#include "badooapi.h"
#include "badoologindialog.h"
#include "badoosearchsettingsdialog.h"

typedef QHash<QString,QByteArrayList> MediaContentsHash;

typedef struct {
    QString sSessionId;
    QString sDeviceId;
    QString sUserId;
    QString sAccountId;
    QString sResponseToken;
} SessionDetails;

typedef struct {
    BadooSexTypeList bstlGenders;
    BadooIntRange    birAgeRange;
    BadooIntRange    birAge;
    BadooIntRange    birDistanceAwayRange;
    int              iDistanceAway;
} EncountersSettings;

typedef struct {
    BadooSexTypeList        bstlGenders;
    BadooIntRange           birAgeRange;
    BadooIntRange           birAge;
    int                     iLocationId;
    QString                 sLocationName;
    BadooStrKeyStrValueHash bsksvhDistanceHash;
    QString                 sDistanceCode;
    BadooIntKeyStrValueHash biksvhOwnIntentHash;
    int                     iOwnIntentId;
} PeopleNearbySettings;

class BadooWrapper:public QObject {
    Q_OBJECT
public:
    BadooWrapper();
    template<typename T>
    bool    downloadMultiMediaResources(QStringList,QList<T> &,int=3);
    bool    getEncounters(BadooUserProfileList &,bool=false);
    void    getEncountersSettings(EncountersSettings &);
    bool    getLoggedInProfile(BadooUserProfile &);
    void    getPeopleNearbySettings(PeopleNearbySettings &);
    QString getHTMLFromProfile(BadooUserProfile,bool=false,QString=QString(),QByteArrayList={},QByteArrayList={});
    QString getLastError();
    void    getSessionDetails(SessionDetails &);
    bool    isLoggedIn();
    bool    loadSearchSettings();
    bool    login(QString,QString);
    bool    logout();
    bool    saveSearchSettings();
    void    setEncountersSettings(EncountersSettings);
    void    setPeopleNearbySettings(PeopleNearbySettings);
    bool    showLogin();
    bool    showSearchSettings(BadooSettingsContextType);
    bool    vote(QString,bool,bool &);
private:
    QString              sLastError,
                         sLastEncountersId;
    BadooUserProfile     bupSelf;
    SessionDetails       sdSession;
    EncountersSettings   esEncounters;
    PeopleNearbySettings pnsPeopleNearby;
    void    clearEncountersSettings();
    void    clearPeopleNearbySettings();
    void    clearSessionDetails();
    QString getTextFromBoolean(bool);
    QString getTextFromSexType(BadooSexType);
    QString getTextFromVote(BadooVote);
    void    setEncountersSettings(BadooSexTypeList,BadooIntRange,BadooIntRange,BadooIntRange,int);
    void    setPeopleNearbySettings(BadooSexTypeList,BadooIntRange,BadooIntRange,int,QString,BadooStrKeyStrValueHash,QString,BadooIntKeyStrValueHash,int);
    void    setSessionDetails(QString,QString,QString,QString,QString);
signals:
    void statusChanged(QString);
};

#endif // BADOOWRAPPER_H
