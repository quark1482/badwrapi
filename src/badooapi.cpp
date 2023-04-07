#include "badooapi.h"

#define DEFAULT_ERROR_MESSAGE "Unexpected response"

BadooMessageNameHash bmnhMessages={
    {CLIENT_SERVER_ERROR,QStringLiteral("server_error_message")},
    {SERVER_APP_STARTUP,QStringLiteral("server_app_startup")},
    {CLIENT_STARTUP,QStringLiteral("client_startup")},
    {SERVER_LOGIN_BY_PASSWORD,QStringLiteral("server_login_by_password")},
    {CLIENT_LOGIN_SUCCESS,QStringLiteral("client_login_success")},
    {CLIENT_LOGIN_FAILURE,"form_failure"},
    {SERVER_SEARCH_LOCATIONS,QStringLiteral("server_search_locations")},
    {CLIENT_LOCATIONS,QStringLiteral("client_locations")},
    {SERVER_ENCOUNTERS_VOTE,QStringLiteral("server_encounters_vote")},
    {CLIENT_ENCOUNTERS_VOTE,QStringLiteral("client_vote_response")},
    {SERVER_GET_ENCOUNTERS,QStringLiteral("server_get_encounters")},
    {CLIENT_NO_MORE_ENCOUNTERS,QStringLiteral("no_more_search_results")},
    {CLIENT_ENCOUNTERS,QStringLiteral("client_encounters")},
    {SERVER_GET_CAPTCHA,QStringLiteral("server_get_captcha")},
    {CLIENT_GET_CAPTCHA,QStringLiteral("client_get_captcha")},
    {SERVER_CAPTCHA_ATTEMPT,QStringLiteral("server_captcha_attempt")},
    {CLIENT_CAPTCHA_ATTEMPT,QStringLiteral("client_captcha_attempt")},
    {SERVER_GET_SEARCH_SETTINGS,QStringLiteral("server_get_search_settings")},
    {SERVER_SAVE_SEARCH_SETTINGS,"server_save_search_settings"},
    {CLIENT_SEARCH_SETTINGS,QStringLiteral("client_search_settings")},
    {CLIENT_SEARCH_SETTINGS_FAILURE,QStringLiteral("search_settings_failure")},
}; // Fun fact: the message names are not always a direct lower-case "translation" of the key constant names.

BadooUserFieldList buflProjection={
    USER_FIELD_COUNTRY,
    USER_FIELD_REGION,
    USER_FIELD_CITY,
    USER_FIELD_NAME,
    USER_FIELD_AGE,
    USER_FIELD_GENDER,
    USER_FIELD_IS_VERIFIED,
    USER_FIELD_PHOTO_COUNT,
    USER_FIELD_VIDEO_COUNT,
    USER_FIELD_ONLINE_STATUS,      // Online status numerical value [1:online 3:x_units_ago]
    USER_FIELD_ONLINE_STATUS_TEXT,
    USER_FIELD_PROFILE_PHOTO,
    USER_FIELD_ALBUMS,
    USER_FIELD_PROFILE_FIELDS,     // Relationship status, sexuality, appearance, children, etc.
    USER_FIELD_TIW_IDEA,           // "Open to chat" / "To date" / "Ready for a relationship".
    USER_FIELD_MY_VOTE,
    USER_FIELD_THEIR_VOTE,
    USER_FIELD_IS_MATCH,
    USER_FIELD_IS_FAVOURITE,
    USER_FIELD_QUICK_CHAT,         // Is free chat possible for the queried profile?
    USER_FIELD_MOOD_STATUS,        // 'Make me laugh', 'Looking for love', 'Thinking long-term', etc.
};

BadooAlbumTypeList batlAlbums={ // This is the 'request_albums' array content ...
    ALBUM_TYPE_PHOTOS_OF_ME,    // ... the app sends with the 'SERVER_GET_USER' message.
    ALBUM_TYPE_PRIVATE_PHOTOS,  // Not sure about what exactly are 'private' photos ...
    ALBUM_TYPE_ENCOUNTERS,      // ... or why is the 'encounters' type required here.
    ALBUM_TYPE_EXTERNAL_PHOTOS, // No idea about what are 'external' photos either.
};

BadooFeatureTypeList bftlFeatures={
    ALLOW_OPEN_PEOPLE_NEARBY,        // Required to get albums for the folder NEARBY_PEOPLE_XXX.
    ALLOW_OPEN_ENCOUNTERS,           // Required for requesting SERVER_GET_ENCOUNTERS.
    ALLOW_ENCOUNTERS_VOTE,           // Required for well, voting.
    ALLOW_MULTIMEDIA,                // Required for multimedia chat messages.
    ALLOW_CONTACTS_FOR_CREDITS,      // Required to make the 'quick_chat' user field valid.
    ALLOW_BADOO_PROFILE_MOOD_STATUS, // Required to include moods in user profiles.
};

BadooMinorFeatureList bmflMinorFeatures {
    MINOR_FEATURE_ENCOUNTER_SETTINGS_DISTANCE_SLIDER,  // Allows setting a 'distance away' in encounters.
    MINOR_FEATURE_REDESIGN_MERGE_ALBUMS,               // Puts everything in a single album.
    MINOR_FEATURE_CAN_PLAY_VIDEO,                      // Includes videos in album.
    MINOR_FEATURE_NO_ENCOUNTERS_ALBUM_IN_PROFILE_MODE, // Avoids a lot of repeated photos.
    MINOR_FEATURE_SEPARATE_MATCH_FOLDER,               // Allows querying the matches folder.
    MINOR_FEATURE_CHAT_PLAY_AUDIO,                     // Accepts audio clips in chat messages.
};

BadooCAPTCHASolver *bcsCAPTCHASolver=nullptr;

void BadooAPI::clearUserProfile(BadooUserProfile &bupProfile) {
    bupProfile.sUserId.clear();
    bupProfile.sName.clear();
    bupProfile.iAge=0;
    bupProfile.iGender=SEX_TYPE_UNKNOWN;
    bupProfile.bIsVerified=false;
    bupProfile.bIsMatch=false;
    bupProfile.bIsFavorite=false;
    bupProfile.bHasQuickChat=false;
    bupProfile.sCountry.clear();
    bupProfile.sRegion.clear();
    bupProfile.sCity.clear();
    bupProfile.sOnlineStatus.clear();
    bupProfile.sProfilePhotoURL.clear();
    bupProfile.slPhotos.clear();
    bupProfile.slVideos.clear();
    bupProfile.sIntent.clear();
    bupProfile.sMood.clear();
    bupProfile.bvMyVote=VOTE_UNKNOWN;
    bupProfile.bvTheirVote=VOTE_UNKNOWN;
    bupProfile.sAbout.clear();
    bupProfile.sRelationshipStatus.clear();
    bupProfile.sSexuality.clear();
    bupProfile.sAppearance.clear();
    bupProfile.sLiving.clear();
    bupProfile.sChildren.clear();
    bupProfile.sSmoking.clear();
    bupProfile.sDrinking.clear();
}

bool BadooAPI::downloadMediaResource(QString    sSessionId,
                                     QString    sResourceURL,
                                     QByteArray &abtResourceData,
                                     QString    &sError) {
    bool           bResult=false;
    uint           uiResCode;
    quint64        ui64ContentLength;
    QString        sContentType;
    QByteArray     abtResponse;
    RawHeadersHash rhhHeaders;
    abtResourceData.clear();
    sError.clear();
    HTTPRequest::get(
        QUrl(sResourceURL),
        {
            {
                QStringLiteral("cookie").toUtf8(),
                QStringLiteral("session=%1").arg(sSessionId).toUtf8()
            }
        },
        {},
        uiResCode,
        abtResponse,
        rhhHeaders,
        sError
    );
    ui64ContentLength=rhhHeaders.value(QStringLiteral("Content-Length").toUtf8()).toULongLong();
    sContentType=rhhHeaders.value(QStringLiteral("content-type").toUtf8());
    if(HTTP_STATUS_OK==uiResCode)
        if(sContentType.startsWith(QStringLiteral("image/"))||
           sContentType.startsWith(QStringLiteral("video/"))) {
            abtResourceData=abtResponse;
            if(abtResourceData.isEmpty())
                sError=QStringLiteral("Downloaded resource is empty");
            else
                if(ui64ContentLength&&ui64ContentLength!=abtResourceData.size())
                    sError=QStringLiteral("Downloaded size mismatches resource headers");
                else
                    bResult=true;
        }
        else
            sError=QStringLiteral("Unexpected content type: %1").
                   arg(sContentType);
    else if(HTTP_STATUS_INVALID!=uiResCode)
        sError=QStringLiteral("Unexpected response code: %1").
               arg(uiResCode);
    if(!bResult) {
        if(sError.isEmpty())
            sError=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    return bResult;
}

bool BadooAPI::downloadMediaResource(QString sSessionId,
                                     QString sResourceURL,
                                     QString &sFilePath,
                                     QString &sError) {
    bool       bResult=false;
    QByteArray abtResourceData;
    sFilePath.clear();
    sError.clear();
    if(downloadMediaResource(sSessionId,sResourceURL,abtResourceData,sError)) {
        QTemporaryFile tfResource;
        tfResource.setAutoRemove(false);
        if(tfResource.open()) {
            if(0<tfResource.write(abtResourceData)) {
                sFilePath=tfResource.fileName();
                bResult=true;
            }
            else
                sError=tfResource.errorString();
            tfResource.close();
        }
        else
            sError=tfResource.errorString();
    }
    if(!bResult) {
        if(sError.isEmpty())
            sError=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    return bResult;
}

bool BadooAPI::getPreLoginParameters(QString &sAnonSessionId,
                                     QString &sDeviceId,
                                     QString &sError) {
    bool           bResult=false;
    uint           uiResCode;
    QString        sContentType;
    QByteArray     abtResponse;
    RawHeadersHash rhhHeaders;
    sAnonSessionId.clear();
    sDeviceId.clear();
    sError.clear();
    HTTPRequest::get(
        QUrl(QStringLiteral(ENDPOINT_BASE)),
        {},
        {},
        uiResCode,
        abtResponse,
        rhhHeaders,
        sError
    );
    sContentType=rhhHeaders.value(QStringLiteral("content-type").toUtf8());
    // Searches for a Device Id and a temporary Session Id in the returned cookies.
    if(rhhHeaders.contains(QStringLiteral("set-cookie").toUtf8())) {
        QList<QNetworkCookie> nclCookies=QNetworkCookie::parseCookies(
            rhhHeaders.value(QStringLiteral("set-cookie").toUtf8())
        );
        for(const auto &c:nclCookies)
            if(!c.name().compare(
                QStringLiteral("device_id").toUtf8(),
                Qt::CaseSensitivity::CaseInsensitive
            ))
                sDeviceId=QByteArray::fromPercentEncoding(c.value());
            else if(!c.name().compare(
                QStringLiteral("session").toUtf8(),
                Qt::CaseSensitivity::CaseInsensitive
            ))
                sAnonSessionId=QByteArray::fromPercentEncoding(c.value());
    }
    if(HTTP_STATUS_OK==uiResCode)
        if(sContentType.startsWith(QStringLiteral(HTTP_HEADER_CONTENT_TYPE_HTML)))
            bResult=!sDeviceId.isEmpty()&&!sAnonSessionId.isEmpty();
        else
            sError=QStringLiteral("Unexpected content type: %1").
                   arg(sContentType);
    else if(HTTP_STATUS_INVALID!=uiResCode)
        sError=QStringLiteral("Unexpected response code: %1").
               arg(uiResCode);
    if(!bResult) {
        if(sError.isEmpty())
            sError=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    return bResult;
}

bool BadooAPI::getPostLoginParameters(QString sSessionId,
                                      QString &sResponseToken,
                                      QString &sError) {
    bool           bResult=false;
    uint           uiResCode;
    QString        sRequest,
                   sContentType;
    QByteArray     abtResponse;
    RawHeadersHash rhhHeaders;
    sResponseToken.clear();
    sError.clear();
    sRequest=QStringLiteral("new_session_id=%1").arg(sSessionId);
    HTTPRequest::post(
        QUrl(QStringLiteral(ENDPOINT_SESSION)),
        sRequest.toUtf8(),
        {
            {
                QStringLiteral("content-type").toUtf8(),
                QStringLiteral(HTTP_HEADER_CONTENT_TYPE_FORM).toUtf8()
            },
            {
                QStringLiteral("cookie").toUtf8(),
                QStringLiteral("session=%1").arg(sSessionId).toUtf8()
            }
        },
        {},
        uiResCode,
        abtResponse,
        rhhHeaders,
        sError
    );
    sContentType=rhhHeaders.value(QStringLiteral("content-type").toUtf8());
    if(HTTP_STATUS_OK==uiResCode)
        if(sContentType.startsWith(QStringLiteral("text/"))) {
            QJsonDocument jsnDoc;
            QJsonObject   jsnObj;
            jsnDoc=QJsonDocument::fromJson(abtResponse);
            if(!jsnDoc.isNull()) {
                jsnObj=jsnDoc.object();
                if(jsnObj.value(QStringLiteral("success")).isBool()) {
                    bResult=jsnObj.value(QStringLiteral("success")).toBool();
                    if(bResult)
                        // No idea about what does 'rt' mean, so I just named it 'Response Token'.
                        sResponseToken=jsnObj.value(QStringLiteral("rt")).toString();
                }
            }
        }
        else
            sError=QStringLiteral("Unexpected content type: %1").
                   arg(sContentType);
    else if(HTTP_STATUS_INVALID!=uiResCode)
        sError=QStringLiteral("Unexpected response code: %1").
               arg(uiResCode);
    if(!bResult) {
        if(sError.isEmpty())
            sError=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    return bResult;
}

bool BadooAPI::sendCAPTCHAAttempt(QString       sSessionId,
                                  QString       sCAPTCHAId,
                                  QString       sCAPTCHAURL,
                                  QString       sAnswer,
                                  bool          &bSuccess,
                                  BadooAPIError &baeError) {
    bool        bResult=false;
    QJsonObject jsnMessage,
                jsnResponse;
    bSuccess=false;
    clearError(baeError);
    jsnMessage.insert(QStringLiteral("uid"),sCAPTCHAId);
    jsnMessage.insert(QStringLiteral("image_id"),sCAPTCHAURL);
    jsnMessage.insert(QStringLiteral("answer"),sAnswer);
    const BadooMessagePair bmpMessage={SERVER_CAPTCHA_ATTEMPT,CLIENT_CAPTCHA_ATTEMPT};
    if(getResponse(bmpMessage,sSessionId,jsnMessage,jsnResponse,baeError)) {
        if(jsnResponse.value(QStringLiteral("success")).isBool()) {
            bSuccess=jsnResponse.value(QStringLiteral("success")).toBool();
            bResult=true;
        }
    }
    if(!bResult) {
        if(noError(baeError))
            baeError.sErrorMessage=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        baeError.sErrorMessage=QStringLiteral("[%1()] %2").arg(__FUNCTION__,baeError.sErrorMessage);
    }
    return bResult;
}

bool BadooAPI::sendEncountersVote(QString       sSessionId,
                                  QString       sUserId,
                                  bool          bLike,
                                  bool          &bMatch,
                                  BadooAPIError &baeError) {
    bool        bResult=false;
    QJsonObject jsnMessage,
                jsnResponse;
    bMatch=false;
    clearError(baeError);
    jsnMessage.insert(QStringLiteral("person_id"),sUserId);
    jsnMessage.insert(QStringLiteral("vote"),bLike?VOTE_YES:VOTE_NO);
    jsnMessage.insert(QStringLiteral("vote_source"),CLIENT_SOURCE_ENCOUNTERS);
    const BadooMessagePair bmpMessage={SERVER_ENCOUNTERS_VOTE,CLIENT_ENCOUNTERS_VOTE};
    // Voting again is possible, but doesn't do anything (most probably reserved to premium access).
    // Voting for profiles from another country is also possible, but doesn't do anything either.
    // Voting for local profiles with restricted game settings (age, etc.) IS possible.
    if(getResponse(bmpMessage,sSessionId,jsnMessage,jsnResponse,baeError)) {
        // Ignores over-parsing the response because at this point, it's safe to assume ...
        // ... that the vote was successfully registered (see VOTE_ enums for exceptions) ...
        // ... and only checks for the match condition.
        if(GAME_SUCCESS==jsnResponse.value(QStringLiteral("vote_response_type")).toInt())
            bMatch=true;
        bResult=true;
    }
    if(!bResult) {
        if(noError(baeError))
            baeError.sErrorMessage=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        baeError.sErrorMessage=QStringLiteral("[%1()] %2").arg(__FUNCTION__,baeError.sErrorMessage);
    }
    return bResult;
}

bool BadooAPI::sendGetCAPTCHA(QString       sSessionId,
                              QString       sCAPTCHAId,
                              QString       &sCAPTCHAURL,
                              BadooAPIError &baeError) {
    bool        bResult=false;
    QJsonObject jsnMessage,
                jsnResponse;
    sCAPTCHAURL.clear();
    clearError(baeError);
    jsnMessage.insert(QStringLiteral("uid"),sCAPTCHAId);
    jsnMessage.insert(QStringLiteral("force_new"),true);
    jsnMessage.insert(QStringLiteral("context"),0);
    const BadooMessagePair bmpMessage={SERVER_GET_CAPTCHA,CLIENT_GET_CAPTCHA};
    if(getResponse(bmpMessage,sSessionId,jsnMessage,jsnResponse,baeError)) {
        if(jsnResponse.value(QStringLiteral("captcha")).isObject()) {
            QJsonObject jsnObj=jsnResponse.value(QStringLiteral("captcha")).toObject();
            sCAPTCHAURL=jsnObj.value(QStringLiteral("image_id")).toString();
        }
        bResult=!sCAPTCHAURL.isEmpty();
    }
    if(!bResult) {
        if(noError(baeError))
            baeError.sErrorMessage=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        baeError.sErrorMessage=QStringLiteral("[%1()] %2").arg(__FUNCTION__,baeError.sErrorMessage);
    }
    return bResult;
}

bool BadooAPI::sendGetEncounters(QString              sSessionId,
                                 QString              sLastUserId,
                                 int                  iCount,
                                 BadooUserProfileList &buplUsers,
                                 BadooAPIError        &baeError) {
    bool                     bResult=false;
    QJsonObject              jsnMessage,
                             jsnFieldFilter;
    QJsonArray               jsnProjection,
                             jsnAlbums;
    BadooMessageResponseHash bmrhResponses;
    buplUsers.clear();
    clearError(baeError);
    jsnMessage.insert(QStringLiteral("number"),iCount);
    if(!sLastUserId.isEmpty())
        jsnMessage.insert(QStringLiteral("last_person_id"),sLastUserId);
    for(const auto &f:buflProjection)
        jsnProjection.append(f);
    jsnFieldFilter.insert(QStringLiteral("projection"),jsnProjection);
    for(const auto &t:batlAlbums) {
        QJsonObject jsnObj;
        jsnObj.insert(QStringLiteral("album_type"),t);
        jsnAlbums.append(jsnObj);
    }
    jsnFieldFilter.insert(QStringLiteral("request_albums"),jsnAlbums);
    jsnMessage.insert(QStringLiteral("user_field_filter"),jsnFieldFilter);
    if(getResponse(SERVER_GET_ENCOUNTERS,sSessionId,jsnMessage,bmrhResponses,baeError)) {
        if(bmrhResponses.contains(CLIENT_ENCOUNTERS)) {
            if(bmrhResponses.value(CLIENT_ENCOUNTERS).value(QStringLiteral("results")).isArray()) {
                QJsonObject      jsnUser;
                QJsonArray       jsnUsers;
                BadooUserProfile bupUser;
                jsnUsers=bmrhResponses.value(CLIENT_ENCOUNTERS).value(QStringLiteral("results")).toArray();
                for(const auto &u:qAsConst(jsnUsers))
                    if(u.isObject()) {
                        if(u.toObject().value(QStringLiteral("user")).isObject()) {
                            jsnUser=u.toObject().value(QStringLiteral("user")).toObject();
                            parseUserProfile(jsnUser,bupUser);
                            buplUsers.append(bupUser);
                        }
                    }
                bResult=true;
            }
        }
        else if(bmrhResponses.contains(CLIENT_NO_MORE_ENCOUNTERS))
            bResult=true;
    }
    if(!bResult) {
        if(noError(baeError))
            baeError.sErrorMessage=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        baeError.sErrorMessage=QStringLiteral("[%1()] %2").arg(__FUNCTION__,baeError.sErrorMessage);
    }
    return bResult;
}

bool BadooAPI::sendGetSearchSettings(QString                  sSessionId,
                                     BadooSettingsContextType bsctContext,
                                     BadooSearchSettings      &bssSettings,
                                     BadooIntRange            &birAgeRange,
                                     BadooIntRange            &birDistanceRange,
                                     BadooStrKeyStrValueHash  &bsksvhDistanceHash,
                                     BadooIntKeyStrValueHash  &biksvhIntentHash,
                                     BadooAPIError            &baeError) {
    bool        bResult=false;
    QJsonObject jsnMessage,
                jsnResponse;
    clearSearchSettings(bssSettings);
    birAgeRange.first=0;
    birAgeRange.second=0;
    birDistanceRange.first=0;
    birDistanceRange.second=0;
    bsksvhDistanceHash.clear();
    biksvhIntentHash.clear();
    clearError(baeError);
    jsnMessage.insert(QStringLiteral("context_type"),bsctContext);
    const BadooMessagePair bmpMessage={SERVER_GET_SEARCH_SETTINGS,CLIENT_SEARCH_SETTINGS};
    if(getResponse(bmpMessage,sSessionId,jsnMessage,jsnResponse,baeError)) {
        // Depending on the supplied settings context type, the returned search settings struct ...
        // ... will have either the 'distance away' or the 'distance code' field values filled ...
        // ... and the other set to its default (zero or an empty string)
        parseSearchSettings(
            jsnResponse,
            bssSettings,
            birAgeRange,
            birDistanceRange,
            bsksvhDistanceHash,
            biksvhIntentHash
        );
        bResult=true;
    }
    if(!bResult) {
        if(noError(baeError))
            baeError.sErrorMessage=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        baeError.sErrorMessage=QStringLiteral("[%1()] %2").arg(__FUNCTION__,baeError.sErrorMessage);
    }
    return bResult;
}

bool BadooAPI::sendLogin(QString       sSessionId,
                         QString       sUsername,
                         QString       sPassword,
                         QString       &sNewSessionId,
                         BadooAPIError &baeError) {
    bool                     bResult=false;
    QJsonObject              jsnMessage;
    BadooMessageResponseHash bmrhResponses;
    sNewSessionId.clear();
    clearError(baeError);
    jsnMessage.insert(QStringLiteral("user"),sUsername);
    jsnMessage.insert(QStringLiteral("password"),sPassword);
    jsnMessage.insert(QStringLiteral("remember_me"),true);
    if(getResponse(SERVER_LOGIN_BY_PASSWORD,sSessionId,jsnMessage,bmrhResponses,baeError)) {
        if(bmrhResponses.contains(CLIENT_LOGIN_SUCCESS))
            sNewSessionId=bmrhResponses.value(CLIENT_LOGIN_SUCCESS).value(QStringLiteral("session_id")).toString();
        else if(bmrhResponses.contains(CLIENT_LOGIN_FAILURE))
            parseFailure(bmrhResponses.value(CLIENT_LOGIN_FAILURE),baeError.sErrorMessage);
        bResult=!sNewSessionId.isEmpty();
    }
    if(!bResult) {
        if(noError(baeError))
            baeError.sErrorMessage=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        baeError.sErrorMessage=QStringLiteral("[%1()] %2").arg(__FUNCTION__,baeError.sErrorMessage);
    }
    return bResult;
}

bool BadooAPI::sendSaveSearchSettings(QString                  sSessionId,
                                      BadooSettingsContextType bsctContext,
                                      BadooSearchSettings      &bssSettings,
                                      BadooIntRange            &birAgeRange,
                                      BadooIntRange            &birDistanceRange,
                                      BadooStrKeyStrValueHash  &bsksvhDistanceHash,
                                      BadooIntKeyStrValueHash  &biksvhIntentHash,
                                      BadooAPIError            &baeError) {
    bool                     bResult=false;
    QJsonObject              jsnMessage,
                             jsnSettings,
                             jsnAge,
                             jsnDistance;
    QJsonArray               jsnGender;
    BadooMessageResponseHash bmrhResponses;
    birAgeRange.first=0;
    birAgeRange.second=0;
    birDistanceRange.first=0;
    birDistanceRange.second=0;
    bsksvhDistanceHash.clear();
    biksvhIntentHash.clear();
    clearError(baeError);
    jsnMessage.insert(QStringLiteral("context_type"),bsctContext);
    for(const auto &g:bssSettings.bstlGenders)
        jsnGender.append(g);
    jsnSettings.insert(QStringLiteral("gender"),jsnGender);
    jsnAge.insert(QStringLiteral("start"),bssSettings.birAge.first);
    jsnAge.insert(QStringLiteral("end"),bssSettings.birAge.second);
    jsnSettings.insert(QStringLiteral("age"),jsnAge);
    // For the context type 'encounters', a 'location id' can be passed and the response shows ...
    // ... it was modified, but that's false. A following SERVER_GET_SEARCH_SETTINGS request ...
    // ... in the same context will include the actual location of the logged-in account.
    jsnSettings.insert(QStringLiteral("location_id"),bssSettings.iLocationId);
    // For this request, a 'location name' must not be passed. This setting is only required ...
    // ... when the caller invokes sendSearchLocations() to find the right 'location id'.
    if(SETTINGS_CONTEXT_TYPE_ENCOUNTERS==bsctContext) // These two fields cannot co-exist in the same request.
        jsnDistance.insert(QStringLiteral("end"),bssSettings.iDistanceAway);
    else
        jsnDistance.insert(QStringLiteral("fixed_end"),bssSettings.sDistanceCode);
    jsnSettings.insert(QStringLiteral("distance"),jsnDistance);
    jsnSettings.insert(QStringLiteral("tiw_phrase_id"),bssSettings.iOwnIntentId);
    jsnMessage.insert(QStringLiteral("settings"),jsnSettings);
    if(getResponse(SERVER_SAVE_SEARCH_SETTINGS,sSessionId,jsnMessage,bmrhResponses,baeError)) {
        if(bmrhResponses.contains(CLIENT_SEARCH_SETTINGS)) {
            // A copy of the saved settings is returned here. The supplied search settings ...
            // ... struct is updated (just to be sure) along with the ranges and hash tables.
            parseSearchSettings(
                bmrhResponses.value(CLIENT_SEARCH_SETTINGS),
                bssSettings,
                birAgeRange,
                birDistanceRange,
                bsksvhDistanceHash,
                biksvhIntentHash
            );
            bResult=true;
        }
        else if(bmrhResponses.contains(CLIENT_SEARCH_SETTINGS_FAILURE))
            parseFailure(bmrhResponses.value(CLIENT_SEARCH_SETTINGS_FAILURE),baeError.sErrorMessage);
    }
    if(!bResult) {
        if(noError(baeError))
            baeError.sErrorMessage=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        baeError.sErrorMessage=QStringLiteral("[%1()] %2").arg(__FUNCTION__,baeError.sErrorMessage);
    }
    return bResult;
}

bool BadooAPI::sendSearchLocations(QString                 sSessionId,
                                   QString                 sQuery,
                                   BadooSearchLocationList &bsllLocations,
                                   BadooAPIError           &baeError) {
    bool        bResult=false;
    QJsonObject jsnMessage,
                jsnResponse;
    bsllLocations.clear();
    clearError(baeError);
    jsnMessage.insert(QStringLiteral("query"),sQuery);
    // There is a query modifier boolean field named 'with_countries' which makes ...
    // ... the response include full countries as well. It's not worth the attention.
    const BadooMessagePair bmpMessage={SERVER_SEARCH_LOCATIONS,CLIENT_LOCATIONS};
    if(getResponse(bmpMessage,sSessionId,jsnMessage,jsnResponse,baeError)) {
        if(jsnResponse.value(QStringLiteral("locations")).isArray()) {
            QJsonArray jsnLocations=jsnResponse.value(QStringLiteral("locations")).toArray();
            for(const auto &l:qAsConst(jsnLocations))
                if(l.isObject()) {
                    BadooLocationType bltLocationType;
                    int               iCountryId,
                                      iRegionId,
                                      iCityId,
                                      iLocationId;
                    QString           sCountryName,
                                      sRegionName,
                                      sCityName,
                                      sLocationCode,
                                      sLocationName;
                    parseLocation(l.toObject(),
                                  bltLocationType,
                                  iCountryId,
                                  sCountryName,
                                  iRegionId,
                                  sRegionName,
                                  iCityId,
                                  sCityName
                    );
                    switch(bltLocationType) {
                        case LOCATION_TYPE_COUNTRY:
                            iLocationId=iCountryId;
                            break;
                        case LOCATION_TYPE_REGION:
                            iLocationId=iRegionId;
                            break;
                        case LOCATION_TYPE_CITY:
                            iLocationId=iCityId;
                            break;
                        default:
                            iLocationId=0;
                    }
                    if(iLocationId) {
                        sLocationCode=QStringLiteral("%1_%2_%3").
                                      arg(iCountryId).arg(iRegionId).arg(iCityId);
                        sLocationName.clear();
                        if(!sCityName.isEmpty()) {
                            if(!sLocationName.isEmpty())
                                sLocationName.append(QStringLiteral(", "));
                            sLocationName.append(sCityName);
                        }
                        if(!sRegionName.isEmpty()) {
                            if(!sLocationName.isEmpty())
                                sLocationName.append(QStringLiteral(", "));
                            sLocationName.append(sRegionName);
                        }
                        if(!sCountryName.isEmpty()) {
                            if(!sLocationName.isEmpty())
                                sLocationName.append(QStringLiteral(", "));
                            sLocationName.append(sCountryName);
                        }
                        if(!sLocationName.isEmpty())
                            bsllLocations.append({
                                bltLocationType,
                                iLocationId,
                                sLocationCode,
                                sLocationName
                            });
                    }
                }
        }
        bResult=true;
    }
    if(!bResult) {
        if(noError(baeError))
            baeError.sErrorMessage=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        baeError.sErrorMessage=QStringLiteral("[%1()] %2").arg(__FUNCTION__,baeError.sErrorMessage);
    }
    return bResult;
}

bool BadooAPI::sendStartup(QString          sSessionId,
                           QString          sDeviceId,
                           QString          &sAccountId,
                           BadooUserProfile &bupUser,
                           BadooAPIError    &baeError) {
    bool                     bResult=false;
    QJsonObject              jsnMessage,
                             jsnFieldFilter;
    QJsonArray               jsnFeatures,
                             jsnMinorFeatures,
                             jsnProjection;
    BadooMessageResponseHash bmrhResponses;
    sAccountId.clear();
    clearUserProfile(bupUser);
    clearError(baeError);
    jsnMessage.insert(QStringLiteral("app_build"),QStringLiteral("Badoo"));
    jsnMessage.insert(QStringLiteral("app_name"),QStringLiteral("hotornot"));
    jsnMessage.insert(QStringLiteral("app_version"),QStringLiteral("1.0.00"));
    jsnMessage.insert(QStringLiteral("user_agent"),QString());
    jsnMessage.insert(QStringLiteral("screen_width"),1280);
    jsnMessage.insert(QStringLiteral("screen_height"),720);
    jsnMessage.insert(QStringLiteral("language"),0);
    for(const auto &f:bftlFeatures)
        jsnFeatures.append(f);
    jsnMessage.insert(QStringLiteral("supported_features"),jsnFeatures);
    for(const auto &f:bmflMinorFeatures)
        jsnMinorFeatures.append(f);
    jsnMessage.insert(QStringLiteral("supported_minor_features"),jsnMinorFeatures);
    for(const auto &f:buflProjection)
        jsnProjection.append(f);
    jsnFieldFilter.insert(QStringLiteral("projection"),jsnProjection);
    jsnMessage.insert(QStringLiteral("user_field_filter_client_login_success"),jsnFieldFilter);
    jsnMessage.insert(QStringLiteral("device_id"),sDeviceId);
    if(getResponse(SERVER_APP_STARTUP,sSessionId,jsnMessage,bmrhResponses,baeError)) {
        if(bmrhResponses.contains(CLIENT_STARTUP)) {
            // We don't need anything from this response ATM.
        }
        if(bmrhResponses.contains(CLIENT_LOGIN_SUCCESS)) {
            if(bmrhResponses.value(CLIENT_LOGIN_SUCCESS).value(QStringLiteral("user_info")).isObject())
                parseUserProfile(
                    bmrhResponses.value(CLIENT_LOGIN_SUCCESS).value(QStringLiteral("user_info")).toObject(),
                    bupUser
                );
            sAccountId=bmrhResponses.value(CLIENT_LOGIN_SUCCESS).value(QStringLiteral("encrypted_user_id")).toString();
        }
        bResult=true; // Result depends on the stage SERVER_APP_STARTUP is requested.
    }
    if(!bResult) {
        if(noError(baeError))
            baeError.sErrorMessage=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        baeError.sErrorMessage=QStringLiteral("[%1()] %2").arg(__FUNCTION__,baeError.sErrorMessage);
    }
    return bResult;
}

bool BadooAPI::CAPTCHAHandler(QString       sSessionId,
                              BadooAPIError &baeError) {
    bool               bResult=false;
    QString            sCAPTCHAId,
                       sCAPTCHAURL,
                       sAnswer,
                       sError;
    QByteArray         abtCAPTCHA;
    BadooCAPTCHASolver *bcsSolver=bcsCAPTCHASolver;
    if(nullptr==bcsSolver)
        bcsSolver=manualCAPTCHASolver;
    sCAPTCHAId=baeError.sErrorId;
    clearError(baeError);
    while(true) {
        if(sendGetCAPTCHA(sSessionId,sCAPTCHAId,sCAPTCHAURL,baeError))
            if(downloadMediaResource(sSessionId,sCAPTCHAURL,abtCAPTCHA,sError))
                if(bcsSolver(abtCAPTCHA,sAnswer,sError))
                    if(sendCAPTCHAAttempt(sSessionId,sCAPTCHAId,sCAPTCHAURL,sAnswer,bResult,baeError)) {
                        if(!bResult)
                            continue; // Only retries when it's a wrong CAPTCHA answer.
                    }
        break;
    }
    if(!bResult) {
        if(noError(baeError))
            baeError.sErrorMessage=sError;
        baeError.sErrorMessage=QStringLiteral("[%1()] %2").arg(__FUNCTION__,baeError.sErrorMessage);
    }
    return bResult;
}

void BadooAPI::clearError(BadooAPIError &baeError) {
    baeError.iErrorCode=0;
    baeError.sErrorId.clear();
    baeError.sErrorMessage.clear();
}

void BadooAPI::clearSearchSettings(BadooSearchSettings &bssSettings) {
    bssSettings.bstlGenders.clear();
    bssSettings.birAge.first=0;
    bssSettings.birAge.second=0;
    bssSettings.iLocationId=0;
    bssSettings.sLocationName.clear();
    bssSettings.iDistanceAway=0;
    bssSettings.sDistanceCode.clear();
    bssSettings.iOwnIntentId=0;
}

QString BadooAPI::fixURL(QString sURL) {
    QString sResult;
    QUrl    urlURL;
    sResult.clear();
    if(!sURL.isEmpty()) {
        urlURL.setUrl(sURL);
        if(urlURL.scheme().isEmpty())
            urlURL.setScheme(QStringLiteral("https"));
        sResult=urlURL.url();
    }
    return sResult;
}

bool BadooAPI::getResponse(BadooMessagePair bmpServerClient,
                           QString          sSessionId,
                           QJsonObject      jsnMessage,
                           QJsonObject      &jsnResponse,
                           BadooAPIError    &baeError) {
    bool        bResult=false;
    QString     sResponse,
                sError;
    QJsonObject jsnError;
    while(true) {
        jsnResponse=QJsonObject();
        clearError(baeError);
        if(sendMessage(bmpServerClient.first,sSessionId,jsnMessage,sResponse,sError))
            if(parseResponse(bmpServerClient.second,sResponse,jsnResponse,jsnError,sError)) {
                if(!jsnError.isEmpty()) {
                    parseError(jsnError,baeError);
                    if(SERVER_ERROR_TYPE_CAPTCHA_REQUIRED==baeError.iErrorCode)
                        if(CAPTCHAHandler(sSessionId,baeError))
                            continue;
                }
                else
                    bResult=true;
            }
        break;
    }
    if(!bResult) {
        if(noError(baeError))
            baeError.sErrorMessage=sError;
        baeError.sErrorMessage=QStringLiteral("[%1()] %2").arg(__FUNCTION__,baeError.sErrorMessage);
    }
    return bResult;
}

bool BadooAPI::getResponse(BadooMessageType         bmtMessage,
                           QString                  sSessionId,
                           QJsonObject              jsnMessage,
                           BadooMessageResponseHash &bmrhResponses,
                           BadooAPIError            &baeError) {
    bool        bResult=false;
    QString     sResponse,
                sError;
    QJsonObject jsnError;
    while(true) {
        bmrhResponses.clear();
        clearError(baeError);
        if(sendMessage(bmtMessage,sSessionId,jsnMessage,sResponse,sError))
            if(parseResponse(sResponse,bmrhResponses,jsnError,sError)) {
                if(!jsnError.isEmpty()) {
                    parseError(jsnError,baeError);
                    if(SERVER_ERROR_TYPE_CAPTCHA_REQUIRED==baeError.iErrorCode)
                        if(CAPTCHAHandler(sSessionId,baeError))
                            continue;
                }
                else
                    bResult=true;
            }
        break;
    }
    if(!bResult) {
        if(noError(baeError))
            baeError.sErrorMessage=sError;
        baeError.sErrorMessage=QStringLiteral("[%1()] %2").arg(__FUNCTION__,baeError.sErrorMessage);
    }
    return bResult;
}

bool BadooAPI::manualCAPTCHASolver(QByteArray abtCAPTCHA,
                                   QString    &sAnswer,
                                   QString    &sError) {
    bool         bResult=false;
    QInputDialog idCAPTCHA;
    sAnswer.clear();
    sError.clear();
    idCAPTCHA.setWindowFlag(Qt::WindowType::MSWindowsFixedSizeDialogHint);
    idCAPTCHA.setWindowFlag(Qt::WindowType::CustomizeWindowHint);
    idCAPTCHA.setWindowFlag(Qt::WindowType::WindowSystemMenuHint,false);
    idCAPTCHA.setWindowModality(Qt::WindowModality::ApplicationModal);
    idCAPTCHA.setWindowTitle(QStringLiteral("Solve CAPTCHA"));
    idCAPTCHA.adjustSize();
    idCAPTCHA.show();
    idCAPTCHA.findChild<QLabel *>()->setPixmap(
        QPixmap::fromImage(
            QImage::fromData(abtCAPTCHA)
        )
    );
    if(QDialog::DialogCode::Rejected==idCAPTCHA.exec())
        sError=QStringLiteral("CAPTCHA ignored by user");
    else {
        sAnswer=idCAPTCHA.textValue().trimmed();
        if(sAnswer.isEmpty())
            sError=QStringLiteral("CAPTCHA ignored by user");
        else
            bResult=true;
    }
    if(!bResult) {
        if(sError.isEmpty())
            sError=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    return bResult;
}

bool BadooAPI::noError(BadooAPIError baeError) {
    bool bResult=false;
    if(!baeError.iErrorCode)
        if(baeError.sErrorId.isEmpty())
            if(baeError.sErrorMessage.isEmpty())
                bResult=true;
    return bResult;
}

void BadooAPI::parseAlbum(QJsonObject jsnAlbum,
                          QString     &sOwnerId,
                          QStringList &slPhotos,
                          QStringList &slVideos) {
    QJsonArray jsnPhotos;
    sOwnerId.clear();
    slPhotos.clear();
    slVideos.clear();
    sOwnerId=jsnAlbum.value(QStringLiteral("owner_id")).toString();
    if(jsnAlbum.value(QStringLiteral("photos")).isArray()) {
        jsnPhotos=jsnAlbum.value(QStringLiteral("photos")).toArray();
        for(const auto &p:qAsConst(jsnPhotos))
            if(p.isObject()) {
                QString     sURL;
                QJsonObject jsnObj;
                jsnObj=p.toObject();
                // If the video object exists, adds the video url and ignores the image ...
                // ... because it will be just a frame (large_url) of the whole video.
                if(jsnObj.value(QStringLiteral("video")).isObject()) {
                    jsnObj=jsnObj.value(QStringLiteral("video")).toObject();
                    sURL=jsnObj.value(QStringLiteral("url")).toString();
                    if(!sURL.isEmpty())
                        slVideos.append(fixURL(sURL));
                }
                else {
                    sURL=jsnObj.value(QStringLiteral("large_url")).toString();
                    if(!sURL.isEmpty())
                        slPhotos.append(fixURL(sURL));
                }
            }
    }
}

void BadooAPI::parseError(QJsonObject   jsnError,
                          BadooAPIError &baeError) {
    baeError.iErrorCode=jsnError.value(QStringLiteral("type")).toInt();
    baeError.sErrorId=jsnError.value(QStringLiteral("error_id")).toString();
    baeError.sErrorMessage=jsnError.value(QStringLiteral("error_message")).toString();
}

void BadooAPI::parseFailure(QJsonObject jsnFailure,
                            QString     &sFailure) {
    QJsonArray jsnErrors;
    sFailure.clear();
    if(jsnFailure.value(QStringLiteral("errors")).isArray()) {
        jsnErrors=jsnFailure.value(QStringLiteral("errors")).toArray();
        for(const auto &e:qAsConst(jsnErrors))
            if(e.isObject()) {
                sFailure=e.toObject().value(QStringLiteral("error")).toString();
                break;
            }
    }
}

void BadooAPI::parseLocation(QJsonObject       jsnLocation,
                             BadooLocationType &bltType,
                             int               &iCountryId,
                             QString           &sCountryName,
                             int               &iRegionId,
                             QString           &sRegionName,
                             int               &iCityId,
                             QString           &sCityName) {
    QJsonObject jsnObj;
    bltType=LOCATION_TYPE_UNKNOWN;
    iCountryId=0;
    iRegionId=0;
    iCityId=0;
    sCountryName.clear();
    sRegionName.clear();
    sCityName.clear();
    bltType=static_cast<BadooLocationType>(
        jsnLocation.value(QStringLiteral("type")).toInt()
    );
    if(jsnLocation.value(QStringLiteral("country")).isObject()) {
        jsnObj=jsnLocation.value(QStringLiteral("country")).toObject();
        iCountryId=jsnObj.value(QStringLiteral("id")).toInt();
        sCountryName=jsnObj.value(QStringLiteral("name")).toString();
    }
    if(jsnLocation.value(QStringLiteral("region")).isObject()) {
        jsnObj=jsnLocation.value(QStringLiteral("region")).toObject();
        iRegionId=jsnObj.value(QStringLiteral("id")).toInt();
        sRegionName=jsnObj.value(QStringLiteral("name")).toString();
    }
    if(jsnLocation.value(QStringLiteral("city")).isObject()) {
        jsnObj=jsnLocation.value(QStringLiteral("city")).toObject();
        iCityId=jsnObj.value(QStringLiteral("id")).toInt();
        sCityName=jsnObj.value(QStringLiteral("name")).toString();
    }
}

bool BadooAPI::parseResponse(BadooMessageType bmtMessage,
                             QString          sResponse,
                             QJsonObject      &jsnResponse,
                             QJsonObject      &jsnError,
                             QString          &sError) {
    bool          bResult=false;
    QJsonDocument jsnDoc;
    QJsonObject   jsnObj;
    QJsonArray    jsnArr;
    jsnResponse=QJsonObject();
    jsnError=QJsonObject();
    sError.clear();
    jsnDoc=QJsonDocument::fromJson(sResponse.toUtf8());
    if(!jsnDoc.isNull()) {
        jsnObj=jsnDoc.object();
        if(jsnObj.value(QStringLiteral("body")).isArray()) {
            jsnArr=jsnObj.value(QStringLiteral("body")).toArray();
            for(const auto &o:qAsConst(jsnArr))
                if(o.isObject()) {
                    jsnObj=o.toObject();
                    if(jsnObj.value(bmnhMessages.value(CLIENT_SERVER_ERROR)).isObject()) {
                        jsnError=jsnObj.value(bmnhMessages.value(CLIENT_SERVER_ERROR)).toObject();
                        bResult=true;
                        break;
                    }
                    else if(bmnhMessages.contains(bmtMessage)) {
                        if(bmnhMessages.value(bmtMessage).isEmpty()) {
                            if(bmtMessage==jsnObj.value(QStringLiteral("message_type")).toInt())
                                bResult=true;
                            break;
                        }
                        else if(jsnObj.value(bmnhMessages.value(bmtMessage)).isObject()) {
                            jsnResponse=jsnObj.value(bmnhMessages.value(bmtMessage)).toObject();
                            if(bmtMessage==jsnObj.value(QStringLiteral("message_type")).toInt())
                                bResult=true;
                            break;
                        }
                    }
                }
        }
    }
    if(!bResult) {
        if(sError.isEmpty())
            sError=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    return bResult;
}

bool BadooAPI::parseResponse(QString                  sResponse,
                             BadooMessageResponseHash &bmrhResponses,
                             QJsonObject              &jsnError,
                             QString                  &sError) {
    bool          bResult=false;
    QJsonDocument jsnDoc;
    QJsonObject   jsnObj;
    QJsonArray    jsnArr;
    bmrhResponses.clear();
    jsnError=QJsonObject();
    sError.clear();
    jsnDoc=QJsonDocument::fromJson(sResponse.toUtf8());
    if(!jsnDoc.isNull()) {
        jsnObj=jsnDoc.object();
        if(jsnObj.value(QStringLiteral("body")).isArray()) {
            jsnArr=jsnObj.value(QStringLiteral("body")).toArray();
            for(const auto &o:qAsConst(jsnArr))
                if(o.isObject()) {
                    jsnObj=o.toObject();
                    if(jsnObj.value(bmnhMessages.value(CLIENT_SERVER_ERROR)).isObject())
                        jsnError=jsnObj.value(bmnhMessages.value(CLIENT_SERVER_ERROR)).toObject();
                    else {
                        BadooMessageType bmtMessage;
                        QJsonObject      jsnResponse;
                        bmtMessage=static_cast<BadooMessageType>(
                            jsnObj.value(QStringLiteral("message_type")).toInt()
                        );
                        if(bmnhMessages.contains(bmtMessage))
                            if(jsnObj.value(bmnhMessages.value(bmtMessage)).isObject()) {
                                jsnResponse=jsnObj.value(bmnhMessages.value(bmtMessage)).toObject();
                                bmrhResponses.insert(bmtMessage,jsnResponse);
                            }
                    }
                }
            bResult=true;
        }
    }
    if(!bResult) {
        if(sError.isEmpty())
            sError=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    return bResult;
}

void BadooAPI::parseSearchSettings(QJsonObject             jsnSettings,
                                   BadooSearchSettings     &bssSettings,
                                   BadooIntRange           &birAgeRange,
                                   BadooIntRange           &birDistanceRange,
                                   BadooStrKeyStrValueHash &bsksvhDistanceHash,
                                   BadooIntKeyStrValueHash &biksvhIntentHash) {
    QJsonObject jsnSet,
                jsnObj;
    QJsonArray  jsnArr;
    clearSearchSettings(bssSettings);
    birAgeRange.first=0;
    birAgeRange.second=0;
    birDistanceRange.first=0;
    birDistanceRange.second=0;
    bsksvhDistanceHash.clear();
    biksvhIntentHash.clear();
    if(jsnSettings.value(QStringLiteral("current_settings")).isObject()) {
        jsnSet=jsnSettings.value(QStringLiteral("current_settings")).toObject();
        if(jsnSet.value(QStringLiteral("gender")).isArray()) {
            jsnArr=jsnSet.value(QStringLiteral("gender")).toArray();
            for(const auto &g:qAsConst(jsnArr))
                if(g.isDouble())
                    bssSettings.bstlGenders.append(
                        static_cast<BadooSexType>(g.toInt())
                    );
        }
        if(jsnSet.value(QStringLiteral("age")).isObject()) {
            jsnObj=jsnSet.value(QStringLiteral("age")).toObject();
            bssSettings.birAge.first=jsnObj.value(QStringLiteral("start")).toInt();
            bssSettings.birAge.second=jsnObj.value(QStringLiteral("end")).toInt();
        }
        bssSettings.iLocationId=jsnSet.value(QStringLiteral("location_id")).toInt();
        if(jsnSet.value(QStringLiteral("distance")).isObject()) {
            jsnObj=jsnSet.value(QStringLiteral("distance")).toObject();
            bssSettings.iDistanceAway=jsnObj.value(QStringLiteral("end")).toInt();
            bssSettings.sDistanceCode=jsnObj.value(QStringLiteral("fixed_end")).toString();
        }
        bssSettings.iOwnIntentId=jsnSet.value(QStringLiteral("tiw_phrase_id")).toInt();
    }
    if(jsnSettings.value(QStringLiteral("settings_form")).isObject()) {
        jsnSet=jsnSettings.value(QStringLiteral("settings_form")).toObject();
        bssSettings.sLocationName=jsnSet.value(QStringLiteral("location_name")).toString();
        if(jsnSet.value(QStringLiteral("age_slider_settings")).isObject()) {
            jsnObj=jsnSet.value(QStringLiteral("age_slider_settings")).toObject();
            birAgeRange.first=jsnObj.value(QStringLiteral("min_value")).toInt();
            birAgeRange.second=jsnObj.value(QStringLiteral("max_value")).toInt();
        }
        if(jsnSet.value(QStringLiteral("distance_slider_settings")).isObject()) {
            jsnObj=jsnSet.value(QStringLiteral("distance_slider_settings")).toObject();
            birDistanceRange.first=jsnObj.value(QStringLiteral("min_value")).toInt();
            birDistanceRange.second=jsnObj.value(QStringLiteral("max_value")).toInt();
            if(jsnObj.value(QStringLiteral("fixed_values")).isArray()) {
                jsnArr=jsnObj.value(QStringLiteral("fixed_values")).toArray();
                for(const auto &d:qAsConst(jsnArr))
                    if(d.isObject()) {
                        jsnObj=d.toObject();
                        bsksvhDistanceHash.insert(
                            // The field, 'value' is actually a key (alphanumeric distance code).
                            jsnObj.value(QStringLiteral("value")).toString(),
                            {
                                jsnObj.value(QStringLiteral("position")).toInt(),
                                jsnObj.value(QStringLiteral("name")).toString()
                            }
                        );
                    }
            }
        }
        if(jsnSet.value(QStringLiteral("tiw_ideas")).isArray()) {
            jsnArr=jsnSet.value(QStringLiteral("tiw_ideas")).toArray();
            for(const auto &i:qAsConst(jsnArr))
                if(i.isObject()) {
                    jsnObj=i.toObject();
                    biksvhIntentHash.insert(
                        jsnObj.value(QStringLiteral("tiw_phrase_id")).toInt(),
                        {
                            // The field, 'tiw_phrase_id' is good enough to act as a sorting index.
                            jsnObj.value(QStringLiteral("tiw_phrase_id")).toInt(),
                            jsnObj.value(QStringLiteral("tiw_phrase")).toString()
                        }
                    );
                }
        }
    }
}

void BadooAPI::parseUserProfile(QJsonObject      jsnUser,
                                BadooUserProfile &bupUser) {
    QJsonObject jsnObj;
    QJsonArray  jsnArr;
    clearUserProfile(bupUser);
    bupUser.sUserId=jsnUser.value(QStringLiteral("user_id")).toString();
    bupUser.sName=jsnUser.value(QStringLiteral("name")).toString();
    bupUser.iAge=jsnUser.value(QStringLiteral("age")).toInt();
    bupUser.iGender=jsnUser.value(QStringLiteral("gender")).toInt();
    bupUser.bIsVerified=jsnUser.value(QStringLiteral("is_verified")).toBool();
    bupUser.bIsMatch=jsnUser.value(QStringLiteral("is_match")).toBool();
    bupUser.bIsFavorite=jsnUser.value(QStringLiteral("is_favourite")).toBool();
    bupUser.bHasQuickChat=jsnUser.value(QStringLiteral("quick_chat")).isObject();
    if(jsnUser.value(QStringLiteral("country")).isObject()) {
        jsnObj=jsnUser.value(QStringLiteral("country")).toObject();
        bupUser.sCountry=jsnObj.value(QStringLiteral("name")).toString();
    }
    if(jsnUser.value(QStringLiteral("region")).isObject()) {
        jsnObj=jsnUser.value(QStringLiteral("region")).toObject();
        bupUser.sRegion=jsnObj.value(QStringLiteral("name")).toString();
    }
    if(jsnUser.value(QStringLiteral("city")).isObject()) {
        jsnObj=jsnUser.value(QStringLiteral("city")).toObject();
        bupUser.sCity=jsnObj.value(QStringLiteral("name")).toString();
    }
    bupUser.sOnlineStatus=jsnUser.value(QStringLiteral("online_status_text")).toString();
    if(jsnUser.value(QStringLiteral("profile_photo")).isObject()) {
        QString sURL;
        jsnObj=jsnUser.value(QStringLiteral("profile_photo")).toObject();
        sURL=jsnObj.value(QStringLiteral("large_url")).toString();
        if(!sURL.isEmpty())
            bupUser.sProfilePhotoURL=fixURL(sURL);
    }
    if(jsnUser.value(QStringLiteral("albums")).isArray()) {
        jsnArr=jsnUser.value(QStringLiteral("albums")).toArray();
        for(const auto &a:qAsConst(jsnArr))
            if(a.isObject()) {
                QString     sOwnerId;
                QStringList slPhotos,
                            slVideos;
                jsnObj=a.toObject();
                parseAlbum(jsnObj,sOwnerId,slPhotos,slVideos);
                // This little fix (trick?) allows us to get a working id for profiles queried under ...
                // ... special conditions. E.g., sendGetUserList(WANT_TO_MEET_YOU) -AKA 'likes you'- ...
                // ... which returns unusable ids for some reason. Other queries may benefit from this.
                // I don't know what's the point of having the attribute 'owner_id' in each album ...
                // ... inside the user's JSON response, since those albums belong to the queried user.
                if(!sOwnerId.isEmpty())
                    bupUser.sUserId=sOwnerId;
                bupUser.slPhotos.append(slPhotos);
                bupUser.slVideos.append(slVideos);
            }
    }
    if(jsnUser.value(QStringLiteral("tiw_idea")).isObject()) {
        jsnObj=jsnUser.value(QStringLiteral("tiw_idea")).toObject();
        bupUser.sIntent=jsnObj.value(QStringLiteral("tiw_phrase")).toString();
    }
    if(jsnUser.value(QStringLiteral("mood_status")).isObject()) {
        jsnObj=jsnUser.value(QStringLiteral("mood_status")).toObject();
        bupUser.sMood=jsnObj.value(QStringLiteral("name")).toString();
    }
    bupUser.bvMyVote=static_cast<BadooVote>(
        jsnUser.value(QStringLiteral("my_vote")).toInt()
    );
    bupUser.bvTheirVote=static_cast<BadooVote>(
        jsnUser.value(QStringLiteral("their_vote")).toInt()
    );
    if(jsnUser.value(QStringLiteral("profile_fields")).isArray()) {
        jsnArr=jsnUser.value(QStringLiteral("profile_fields")).toArray();
        for(const auto &a:qAsConst(jsnArr))
            if(a.isObject()) {
                int     iType;
                QString sValue;
                jsnObj=a.toObject();
                iType=jsnObj.value(QStringLiteral("type")).toInt();
                sValue=jsnObj.value(QStringLiteral("display_value")).toString();
                switch(iType) {
                    case PROFILE_OPTION_TYPE_ABOUT_ME:
                        bupUser.sAbout=sValue;
                        break;
                    case PROFILE_OPTION_TYPE_RELATIONSHIP:
                        bupUser.sRelationshipStatus=sValue;
                        break;
                    case PROFILE_OPTION_TYPE_SEXUALITY:
                        bupUser.sSexuality=sValue;
                        break;
                    case PROFILE_OPTION_TYPE_APPEARANCE:
                        bupUser.sAppearance=sValue;
                        break;
                    case PROFILE_OPTION_TYPE_LIVING:
                        bupUser.sLiving=sValue;
                        break;
                    case PROFILE_OPTION_TYPE_CHILDREN:
                        bupUser.sChildren=sValue;
                        break;
                    case PROFILE_OPTION_TYPE_SMOKING:
                        bupUser.sSmoking=sValue;
                        break;
                    case PROFILE_OPTION_TYPE_DRINKING:
                        bupUser.sDrinking=sValue;
                        break;
                }
            }
    }
}

bool BadooAPI::sendMessage(BadooMessageType bmtMessage,
                           QString          sSessionId,
                           QJsonObject      jsnMessage,
                           QString          &sResponse,
                           QString          &sError) {
    bool           bResult=false;
    uint           uiResCode;
    QString        sRequest,
                   sSignature,
                   sContentType;
    QByteArray     abtResponse;
    QJsonDocument  jsnDoc;
    QJsonObject    jsnObj,
                   jsnBody;
    QJsonArray     jsnArr;
    QUrl           urlEndPoint;
    RawHeadersHash rhhHeaders;
    sResponse.clear();
    sError.clear();
    if(bmnhMessages.contains(bmtMessage)) {
        jsnBody.insert(QStringLiteral("message_type"),bmtMessage);
        jsnBody.insert(bmnhMessages.value(bmtMessage),jsnMessage);
        jsnArr.append(jsnBody);
        jsnObj.insert(QStringLiteral("$gpb"),QStringLiteral("badoo.bma.BadooMessage"));
        jsnObj.insert(QStringLiteral("body"),jsnArr);
        jsnObj.insert(QStringLiteral("message_type"),bmtMessage);
        jsnObj.insert(QStringLiteral("version"),1);
        jsnDoc.setObject(jsnObj);
        sRequest=jsnDoc.toJson(QJsonDocument::JsonFormat::Compact);
        sSignature=QCryptographicHash::hash(
            QStringLiteral("%1%2").arg(sRequest,QStringLiteral(SIGNATURE_MAGIC)).toUtf8(),
            QCryptographicHash::Algorithm::Md5
        ).toHex();
        urlEndPoint=QUrl(QStringLiteral(ENDPOINT_WEBAPI));
        // The parameter is not required at all, but identifies the request during debug.
        urlEndPoint.setQuery(bmnhMessages.value(bmtMessage).toUpper());
        HTTPRequest::post(
            urlEndPoint,
            sRequest.toUtf8(),
            {
                {
                    QStringLiteral("content-type").toUtf8(),
                    QStringLiteral(HTTP_HEADER_CONTENT_TYPE_JSON).toUtf8()
                },
                {
                    QStringLiteral("cookie").toUtf8(),
                    QStringLiteral("session=%1").arg(sSessionId).toUtf8()
                },
                {
                    QStringLiteral("x-use-session-cookie").toUtf8(),
                    QStringLiteral("1").toUtf8()
                },
                {
                    QStringLiteral("x-pingback").toUtf8(),
                    sSignature.toUtf8()
                }
            },
            {},
            uiResCode,
            abtResponse,
            rhhHeaders,
            sError
        );
        sContentType=rhhHeaders.value(QStringLiteral("content-type").toUtf8());
        if(HTTP_STATUS_OK==uiResCode)
            if(sContentType.startsWith(QStringLiteral(HTTP_HEADER_CONTENT_TYPE_JSON))) {
                sResponse=abtResponse;
                bResult=true;
            }
            else
                sError=QStringLiteral("Unexpected content type: %1").
                       arg(sContentType);
        else if(HTTP_STATUS_INVALID!=uiResCode)
            sError=QStringLiteral("Unexpected response code: %1").
                   arg(uiResCode);
    }
    else
        sError=QStringLiteral("Unknown message type: %1").
               arg(bmtMessage);
    if(!bResult) {
        if(sError.isEmpty())
            sError=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    return bResult;
}
