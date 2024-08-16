#ifndef BADOOAPI_H
#define BADOOAPI_H

#include <QtCore>
#include <QApplication>
#include <QLabel>
#include <QInputDialog>
#include <QIntValidator>
#include <QNetworkCookie>
#include "badooenums.h"
#include "httprequest.h"

#define DOMAIN_BASE       "badoo.com"

#define PATH_API_ENDPOINT "/mwebapi.phtml" // 2023-09-07: new end-point

#define SIGNATURE_MAGIC   "whitetelevisionbulbelectionroofhorseflying"

#define VERSION_WEBAPP    "6.2467.0" // Formerly "1.0.00", the minimum acceptable.

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
    int         iLastOnline;
    bool        bIsVerified;
    bool        bIsMatch;
    bool        bIsFavorite;
    bool        bIsCrush;
    bool        bIsBlocked;
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

typedef QList<BadooUIScreenType> BadooUIScreenTypeList;

typedef QHash<int,BadooIndexedString> BadooIntKeyStrValueHash;

typedef QHash<QString,BadooIndexedString> BadooStrKeyStrValueHash;

typedef bool BadooCAPTCHASolver(QByteArray,QString &,QString &);

class BadooAPI {
public:
    static void clearUserProfile(BadooUserProfile &);
    static bool downloadMediaResource(QString,QString,QByteArray &,QString &);
    static bool downloadMediaResource(QString,QString,QString &,QString &);
    static void getFullFileContents(QString,QByteArray &);
    static bool getPreLoginParameters(QString &,QString &);
    static bool searchListSectionIdByType(QString,BadooFolderType,BadooListSectionType,QString &,QString &,BadooAPIError &);
    static bool sendAddPersonToFolder(QString,QString,BadooFolderType,BadooAPIError &);
    static bool sendCAPTCHAAttempt(QString,QString,QString,QString,bool &,BadooAPIError &);
    static bool sendCheckVerificationPin(QString,QString,QString &,QString &,QString &,BadooAPIError &);
    static bool sendConfirmScreenStory(QString,QString,BadooAPIError &);
    static bool sendEncountersVote(QString,QString,bool,bool &,BadooAPIError &);
    static bool sendGetAppSettings(QString,QVariantHash &,BadooAPIError &);
    static bool sendGetCAPTCHA(QString,QString,QString &,BadooAPIError &);
    static bool sendGetEncounters(QString,QString,int,BadooUserProfileList &,BadooAPIError &);
    static bool sendGetSearchSettings(QString,BadooSettingsContextType,BadooSearchSettings &,BadooIntRange &,BadooIntRange &,BadooStrKeyStrValueHash &,BadooIntKeyStrValueHash &,BadooAPIError &);
    static bool sendGetUser(QString,QString,BadooUserProfile &,BadooAPIError &);
    static bool sendGetUserList(QString,BadooListFilterList,BadooFolderType,QString,int,int,BadooUserProfileList &,int &,BadooAPIError &);
    static bool sendLogin(QString,QString,QString,QString &,QString &,QString &,BadooAPIError &);
    static bool sendLogout(QString,BadooAPIError &);
    static bool sendRemovePersonFromFolder(QString,QString,BadooFolderType,BadooAPIError &);
    static bool sendRemovePersonFromSection(QString,QString,BadooFolderType,BadooListSectionType,BadooSectionActionType,BadooAPIError &);
    static bool sendSaveAppSettings(QString,QVariantHash,BadooAPIError &);
    static bool sendSaveSearchSettings(QString,BadooSettingsContextType,BadooSearchSettings &,BadooIntRange &,BadooIntRange &,BadooStrKeyStrValueHash &,BadooIntKeyStrValueHash &,BadooAPIError &);
    static bool sendSearchLocations(QString,QString,BadooSearchLocationList &,BadooAPIError &);
    static bool sendStartup(QString,QString &,QString &,QString &,QString &,BadooUserProfile &,BadooAPIError &);
    static bool sendUpdateLocation(QString,float,float,BadooAPIError &);
    static void setAgent(QString);
    static void setProxy(QNetworkProxy *);
    static void setServer(QString);
    static void setSolver(BadooCAPTCHASolver *);
private:
    static bool    CAPTCHAHandler(QString,BadooAPIError &);
    static void    clearError(BadooAPIError &);
    static void    clearSearchSettings(BadooSearchSettings &);
    static QString fixURL(QString);
    static bool    getResponse(BadooMessagePair,QString,QJsonObject,RawHeadersHash &,QJsonObject &,BadooAPIError &);
    static bool    getResponse(BadooMessageType,QString,QJsonObject,RawHeadersHash &,BadooMessageResponseHash &,BadooAPIError &);
    static QString getSessionFromHeaders(RawHeadersHash);
    static QString getUSAStateNameFromCode(QString);
    static bool    manualCAPTCHASolver(QByteArray,QString &,QString &);
    static bool    noError(BadooAPIError);
    static void    parseAlbum(QJsonObject,QString &,QStringList &,QStringList &);
    static void    parseError(QJsonObject,BadooAPIError &);
    static void    parseFailure(QJsonObject,QString &);
    static void    parseHostChange(QJsonObject,QString &,bool &);
    static void    parseLocation(QJsonObject,BadooLocationType &,int &,QString &,int &,QString &,int &,QString &);
    static void    parseOnlineStatus(QString,int &);
    static bool    parseResponse(BadooMessageType,QString,QJsonObject &,QJsonObject &,QString &);
    static bool    parseResponse(QString,BadooMessageResponseHash &,QJsonObject &,QString &);
    static void    parseSearchSettings(QJsonObject,BadooSearchSettings &,BadooIntRange &,BadooIntRange &,BadooStrKeyStrValueHash &,BadooIntKeyStrValueHash &);
    static void    parseUserProfile(QJsonObject,BadooUserProfile &);
    static bool    sendMessage(BadooMessageType,QString,QJsonObject,RawHeadersHash &,QString &,QString &);
    static inline  QString            sAPIServer=QStringLiteral(DOMAIN_BASE),
                                      sAPIUserAgent=QStringLiteral("Mozilla/5.0");
    static inline  QNetworkProxy      *pxyAPIProxy=nullptr;
    static inline  BadooCAPTCHASolver *bcsCAPTCHASolver=manualCAPTCHASolver;
};

#endif // BADOOAPI_H
