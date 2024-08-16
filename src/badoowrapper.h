#ifndef BADOOWRAPPER_H
#define BADOOWRAPPER_H

#include "badooapi.h"
#include "badoolocationdialog.h"
#include "badoologindialog.h"
#include "badoosearchsettingsdialog.h"
#include "tageocoder.h"

#define MAX_DOWNLOAD_TRIES 3

#define MAX_PROFILE_IDLE_TIME 60    // After 60 min idle, they are definitely offline.
#define MAX_PROFILE_REAL_TIME 10080 // After 7 days offline, they are probably 'gone'.

#define MIN_USER_ID_LENGTH 30

#define PREFIX_NORMAL_USER_ID    "zAhMAC"
#define PREFIX_ENCRYPTED_USER_ID "zAgEAC"

typedef enum {
    FOLDER_TYPE_UNKNOWN,
    FOLDER_TYPE_FAVORITES,
    FOLDER_TYPE_LIKES,
    FOLDER_TYPE_MATCHES,
    FOLDER_TYPE_PEOPLE_NEARBY,
    FOLDER_TYPE_VISITORS,
    FOLDER_TYPE_BLOCKED
} FolderType; // High level forder type - because not every BadooFolderType is supported.

typedef QHash<QString,QByteArrayList> MediaContentsHash;

typedef struct {
    QString sSessionId;
    QString sDeviceId;
    QString sUserId;
    QString sAccountId;
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
    bool    addToBlocked(QString);
    bool    addToFavorites(QString);
    void    clearSessionDetails();
    template<typename T>
    bool    downloadMultiMediaResources(QStringList,QList<T> &,int=MAX_DOWNLOAD_TRIES);
    template<typename T>
    bool    downloadMultiProfileResources(BadooUserProfileList,MediaContentsHash &,MediaContentsHash &,int=MAX_DOWNLOAD_TRIES);
    bool    getEncounters(BadooUserProfileList &,bool=false);
    void    getEncountersSettings(EncountersSettings &);
    bool    getFolderPage(BadooFolderType,BadooListSectionType,BadooListFilterList,int,int,BadooUserProfileList &,int &,int &,int &);
    bool    getFolderPage(FolderType,BadooListFilterList,int,int,BadooUserProfileList &,int &,int &,int &);
    QString getLastError();
    bool    getLoggedInProfile(BadooUserProfile &);
    bool    getLoggedInProfilePhoto(QByteArray &);
    void    getPeopleNearbySettings(PeopleNearbySettings &);
    bool    getProfile(QString,BadooUserProfile &);
    void    getSessionDetails(SessionDetails &);
    void    getSessionDetails(QString &,QString &,QString &,QString &);
    bool    getVisibleStatus(bool &);
    bool    isLoggedIn();
    bool    loadOwnProfile();
    bool    loadSearchSettings();
    bool    login(QString,QString);
    bool    loginBack();
    bool    logout();
    bool    removeFromBlocked(QString);
    bool    removeFromFavorites(QString);
    bool    removeFromMatches(QString);
    bool    saveSearchSettings();
    void    setEncountersSettings(EncountersSettings);
    bool    setLocation(QString);
    void    setPeopleNearbySettings(PeopleNearbySettings);
    void    setSessionDetails(SessionDetails);
    void    setSessionDetails(QString,QString,QString,QString);
    bool    setVisibleStatus(bool);
    bool    showLocationDialog(QWidget * =nullptr);
    bool    showLoginDialog(QWidget * =nullptr);
    bool    showSearchSettingsDialog(BadooSettingsContextType,QWidget * =nullptr);
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
    void    setEncountersSettings(BadooSexTypeList,BadooIntRange,BadooIntRange,BadooIntRange,int);
    void    setPeopleNearbySettings(BadooSexTypeList,BadooIntRange,BadooIntRange,int,QString,BadooStrKeyStrValueHash,QString,BadooIntKeyStrValueHash,int);
signals:
    void statusChanged(QString);
};

#endif // BADOOWRAPPER_H
