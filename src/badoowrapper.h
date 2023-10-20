#ifndef BADOOWRAPPER_H
#define BADOOWRAPPER_H

#include "badooapi.h"
#include "badoologindialog.h"
#include "badoosearchsettingsdialog.h"

#define MAX_DOWNLOAD_TRIES 3

#define MAX_PROFILE_IDLE_TIME 60

#define PREFIX_NORMAL_USER_ID    "zAhMAC"
#define PREFIX_ENCRYPTED_USER_ID "zAgEAC"

typedef enum {
    FOLDER_TYPE_UNKNOWN,
    FOLDER_TYPE_FAVORITES,
    FOLDER_TYPE_LIKES,
    FOLDER_TYPE_MATCHES,
    FOLDER_TYPE_PEOPLE_NEARBY,
    FOLDER_TYPE_VISITORS
} FolderType;

typedef enum {
    FOLDER_FILTER_ALL,
    FOLDER_FILTER_ONLINE,
    FOLDER_FILTER_NEW,
    FOLDER_FILTER_NEARBY,
    FOLDER_FILTER_MATCHED
} FolderFilter;

typedef QList<FolderFilter> FolderFilterList;

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
    bool    addToFavorites(QString);
    template<typename T>
    bool    downloadMultiMediaResources(QStringList,QList<T> &,int=MAX_DOWNLOAD_TRIES);
    template<typename T>
    bool    downloadMultiProfileResources(BadooUserProfileList,MediaContentsHash &,MediaContentsHash &,int=MAX_DOWNLOAD_TRIES);
    bool    getEncounters(BadooUserProfileList &,bool=false);
    void    getEncountersSettings(EncountersSettings &);
    bool    getFolderPage(BadooFolderType,BadooListSectionType,BadooListFilterList,int,BadooUserProfileList &,int &,int &,int &);
    bool    getFolderPage(FolderType,FolderFilterList,int,BadooUserProfileList &,int &,int &,int &);
    QString getLastError();
    bool    getLoggedInProfile(BadooUserProfile &);
    bool    getLoggedInProfilePhoto(QByteArray &);
    void    getPeopleNearbySettings(PeopleNearbySettings &);
    bool    getProfile(QString,BadooUserProfile &);
    void    getSessionDetails(SessionDetails &);
    bool    isLoggedIn();
    bool    loadSearchSettings();
    bool    login(QString,QString);
    bool    logout();
    bool    removeFromFavorites(QString);
    bool    saveSearchSettings();
    void    setEncountersSettings(EncountersSettings);
    void    setPeopleNearbySettings(PeopleNearbySettings);
    bool    showLogin();
    bool    showSearchSettings(BadooSettingsContextType);
    bool    vote(QString,bool,bool &);
    static QString getFolderName(FolderType);
    static QString getHTMLFromProfile(BadooUserProfile,bool=false,QString=QString(),QByteArrayList={},QByteArrayList={});
    static QString getTextFromBoolean(bool);
    static QString getTextFromSexType(BadooSexType);
    static QString getTextFromVote(BadooVote);
    static bool    isEncryptedUserId(QString);
    static bool    isValidUserId(QString);
private:
    QString              sLastError,
                         sLastEncountersId;
    QByteArray           abtSelfPhoto;
    BadooUserProfile     bupSelf;
    SessionDetails       sdSession;
    EncountersSettings   esEncounters;
    PeopleNearbySettings pnsPeopleNearby;
    void    clearEncountersSettings();
    void    clearPeopleNearbySettings();
    void    clearSessionDetails();
    void    setEncountersSettings(BadooSexTypeList,BadooIntRange,BadooIntRange,BadooIntRange,int);
    void    setPeopleNearbySettings(BadooSexTypeList,BadooIntRange,BadooIntRange,int,QString,BadooStrKeyStrValueHash,QString,BadooIntKeyStrValueHash,int);
    void    setSessionDetails(QString,QString,QString,QString,QString);
signals:
    void statusChanged(QString);
};

#endif // BADOOWRAPPER_H
