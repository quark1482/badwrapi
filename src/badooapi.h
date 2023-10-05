#ifndef BADOOAPI_H
#define BADOOAPI_H

#include <QtCore>
#include <QApplication>
#include <QLabel>
#include <QInputDialog>
#include <QNetworkCookie>
#include "badooenums.h"
#include "httprequest.h"

#define ENDPOINT_BASE    "https://badoo.com"

#define ENDPOINT_WEBAPI  "https://badoo.com/mwebapi.phtml" // New end-point (Sep/2023).

#define ENDPOINT_SESSION "https://badoo.com/ws/set_session_cookie.phtml"

#define ENDPOINT_SIGNOUT "https://badoo.com/signout"

#define SIGNATURE_MAGIC  "whitetelevisionbulbelectionroofhorseflying"

typedef QPair<int,int> BadooIntRange;

typedef QPair<BadooMessageType,BadooMessageType> BadooMessagePair;

typedef QList<BadooSexType> BadooSexTypeList;

typedef QHash<BadooMessageType,QString> BadooMessageNameHash;

typedef QHash<BadooMessageType,QJsonObject> BadooMessageResponseHash;

typedef struct {
    BadooSexTypeList bstlGenders;
    BadooIntRange    birAge;
    int              iLocationId;   // Only for context 'people nearby'.
    QString          sLocationName; // Only for context 'people nearby'.
    int              iDistanceAway; // Only for context 'encounters'. Otherwise, the next field is used.
    QString          sDistanceCode; // Code for "in the whole city" ... "within xx km" ... "in the whole country".
    int              iOwnIntentId;  // Only for context 'people nearby'.
} BadooSearchSettings;

typedef struct {
    int     iErrorCode;
    QString sErrorId;
    QString sErrorMessage;
} BadooAPIError;

typedef struct {
    QString     sUserId;
    QString     sName;
    int         iAge;
    int         iGender;
    bool        bIsVerified;
    bool        bIsMatch;
    bool        bIsFavorite;
    bool        bHasQuickChat;
    QString     sCountry;
    QString     sRegion;
    QString     sCity;
    QString     sOnlineStatus;
    QString     sProfilePhotoURL;
    QStringList slPhotos;
    QStringList slVideos;
    QString     sIntent;
    QString     sMood;
    BadooVote   bvMyVote;
    BadooVote   bvTheirVote;
    // Profile options go here ...
    QString     sAbout;
    QString     sRelationshipStatus;
    QString     sSexuality;
    QString     sAppearance;
    QString     sLiving;
    QString     sChildren;
    QString     sSmoking;
    QString     sDrinking;
} BadooUserProfile;

typedef struct {
    BadooLocationType bltLocationType;
    int               iLocationId;
    QString           sLocationCode;
    QString           sLocationName;
} BadooSearchLocation;

typedef struct {
    int     iIndex;
    QString sValue;
} BadooIndexedString; // Helper struct that allows having a hash of optionally-sorted string values.

typedef QList<BadooUserProfile> BadooUserProfileList;

typedef QList<BadooSearchLocation> BadooSearchLocationList;

typedef QList<BadooUserField> BadooUserFieldList;

typedef QList<BadooListFilter> BadooListFilterList;

typedef QList<BadooAlbumType> BadooAlbumTypeList;

typedef QList<BadooFeatureType> BadooFeatureTypeList;

typedef QList<BadooMinorFeature> BadooMinorFeatureList;

typedef QHash<int,BadooIndexedString> BadooIntKeyStrValueHash;

typedef QHash<QString,BadooIndexedString> BadooStrKeyStrValueHash;

typedef bool BadooCAPTCHASolver(QByteArray,QString &,QString &);

extern BadooCAPTCHASolver *bcsCAPTCHASolver;

class BadooAPI {
public:
    static void clearUserProfile(BadooUserProfile &);
    static bool downloadMediaResource(QString,QString,QByteArray &,QString &);
    static bool downloadMediaResource(QString,QString,QString &,QString &);
    static void getFullFileContents(QString,QByteArray &);
    static bool getPreLoginParameters(QString &,QString &);
    static bool getPostLoginParameters(QString,QString &,QString &);
    static bool searchListSectionIdByType(QString,BadooFolderType,BadooListSectionType,QString &,QString &,BadooAPIError &);
    static bool sendAddPersonToFolder(QString,QString,BadooFolderType,BadooAPIError &);
    static bool sendCAPTCHAAttempt(QString,QString,QString,QString,bool &,BadooAPIError &);
    static bool sendEncountersVote(QString,QString,bool,bool &,BadooAPIError &);
    static bool sendGetCAPTCHA(QString,QString,QString &,BadooAPIError &);
    static bool sendGetEncounters(QString,QString,int,BadooUserProfileList &,BadooAPIError &);
    static bool sendGetSearchSettings(QString,BadooSettingsContextType,BadooSearchSettings &,BadooIntRange &,BadooIntRange &,BadooStrKeyStrValueHash &,BadooIntKeyStrValueHash &,BadooAPIError &);
    static bool sendGetUser(QString,QString,BadooUserProfile &,BadooAPIError &);
    static bool sendGetUserList(QString,BadooListFilterList,BadooFolderType,QString,int,int,BadooUserProfileList &,int &,BadooAPIError &);
    static bool sendLogin(QString,QString,QString,QString &,BadooAPIError &);
    static bool sendRemovePersonFromFolder(QString,QString,BadooFolderType,BadooAPIError &);
    static bool sendSaveSearchSettings(QString,BadooSettingsContextType,BadooSearchSettings &,BadooIntRange &,BadooIntRange &,BadooStrKeyStrValueHash &,BadooIntKeyStrValueHash &,BadooAPIError &);
    static bool sendSearchLocations(QString,QString,BadooSearchLocationList &,BadooAPIError &);
    static bool sendStartup(QString,QString &,QString &,BadooUserProfile &,BadooAPIError &);
private:
    static bool    CAPTCHAHandler(QString,BadooAPIError &);
    static void    clearError(BadooAPIError &);
    static void    clearSearchSettings(BadooSearchSettings &);
    static QString fixURL(QString);
    static bool    getResponse(BadooMessagePair,QString,QJsonObject,RawHeadersHash &,QJsonObject &,BadooAPIError &);
    static bool    getResponse(BadooMessageType,QString,QJsonObject,RawHeadersHash &,BadooMessageResponseHash &,BadooAPIError &);
    static bool    manualCAPTCHASolver(QByteArray,QString &,QString &);
    static bool    noError(BadooAPIError);
    static void    parseAlbum(QJsonObject,QString &,QStringList &,QStringList &);
    static void    parseError(QJsonObject,BadooAPIError &);
    static void    parseFailure(QJsonObject,QString &);
    static void    parseLocation(QJsonObject,BadooLocationType &,int &,QString &,int &,QString &,int &,QString &);
    static bool    parseResponse(BadooMessageType,QString,QJsonObject &,QJsonObject &,QString &);
    static bool    parseResponse(QString,BadooMessageResponseHash &,QJsonObject &,QString &);
    static void    parseSearchSettings(QJsonObject,BadooSearchSettings &,BadooIntRange &,BadooIntRange &,BadooStrKeyStrValueHash &,BadooIntKeyStrValueHash &);
    static void    parseUserProfile(QJsonObject,BadooUserProfile &);
    static bool    sendMessage(BadooMessageType,QString,QJsonObject,RawHeadersHash &,QString &,QString &);
};

#endif // BADOOAPI_H
