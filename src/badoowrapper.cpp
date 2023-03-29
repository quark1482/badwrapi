#include "badoowrapper.h"

#define MAX_DOWNLOAD_THREADS 32

#define MAX_USERS_BY_REQUEST 20

#define HTML_TABLE_PROFILE R"(
<table>
    <tr>
        <th>Name:</th>
        <td>%1</td>
        <th>Age:</th>
        <td>%2</td>
        <th>Appearance:</th>
        <td>%3</td>
    </tr>
    <tr>
        <th>Gender:</th>
        <td>%4</td>
        <th>Sexuality:</th>
        <td colspan=3>%5</td>
    </tr>
    <tr>
        <th>Country:</th>
        <td>%6</td>
        <th>Region:</th>
        <td>%7</td>
        <th>City:</th>
        <td>%8</td>
    </tr>
    <tr>
        <th>Online status:</th>
        <td align=center>%9</td>
        <th>Verified:</th>
        <td align=center>%10</td>
        <th>Quick-chat:</th>
        <td align=center>%11</td>
    </tr>
    <tr>
        <th>About:</th>
        <td colspan=5>%12</td>
    </tr>
    <tr>
        <th>Intent:</th>
        <td>%13</td>
        <th>Mood:</th>
        <td>%14</td>
        <th>Relationship status:</th>
        <td>%15</td>
    </tr>
    <tr>
        <th>Children:</th>
        <td>%16</td>
        <th>Smoking:</th>
        <td>%17</td>
        <th>Drinking:</th>
        <td>%18</td>
    </tr>
    <tr>
        <th>My vote:</th>
        <td align=center>%19</td>
        <th>Their vote:</th>
        <td align=center>%20</td>
        <th>Online profile:</th>
        <td align=center>%21</td>
    </tr>
</table>
)"

#define JS_MEDIA_ONCLICK R"(
<script>
    window.onload=function() {
        const img=document.querySelectorAll('img');
        img.forEach(
            function(i) {
                i.onclick=function() {
                    const doc=window.open('','_self').document;
                    const src=`src='${i.src}'`;
                    const clk=`onclick='location.reload()'`;
                    const stl=`style='cursor: pointer'`;
                    doc.write(`<img ${src} ${clk} ${stl}>`);
                };
            }
        );
        const vid=document.querySelectorAll('video');
        vid.forEach(
            function(v) {
                v.onclick=function() {
                    const doc=window.open('','_self').document;
                    const src=`src='${v.currentSrc}'`;
                    const atr=`controls autoplay loop muted width=100% height=100%`;
                    const clk=`onclick='location.reload()'`;
                    const stl=`style='cursor: pointer'`;
                    doc.write(`<video ${atr} ${clk} ${stl}><source ${src}>Video</video>`);
                };
            }
        );
    };
</script>
)"

BadooWrapper::BadooWrapper() {
    sLastError.clear();
    sLastEncountersId.clear();
    this->clearEncountersSettings();
    this->clearPeopleNearbySettings();
    this->clearSessionDetails();
}

template
bool BadooWrapper::downloadMultiMediaResources<QString>(QStringList,QStringList &,int);

template
bool BadooWrapper::downloadMultiMediaResources<QByteArray>(QStringList,QByteArrayList &,int);

template<typename T>
bool BadooWrapper::downloadMultiMediaResources(QStringList slResourceURLs,
                                               QList<T>    &tlResourceContents,
                                               int         iMaximumTriesPerResource) {
    bool             bResult=false;
    int              iR,
                     iW,
                     iTotalResources,
                     iTotalWorkers,
                     iProgress;
    QString          sError;
    QList<bool>      blResourceDownload,
                     blWorkerResults;
    QList<int>       ilWorkerIndexes,
                     ilResourceTries;
    QList<T>         tlWorkerContents;
    QList<QThread *> thlWorkerThreads;
    QStringList      slWorkerURLs,
                     slWorkerErrors;
    sError.clear();
    tlResourceContents.clear();
    iTotalResources=slResourceURLs.count();
    qDebug() << "Total resources" << iTotalResources;
    // Initializes the path/data contents, remaining download tries and the download status ...
    // ... for every resource.
    for(iR=0;iR<iTotalResources;iR++) {
        blResourceDownload.append(false);
        ilResourceTries.append(iMaximumTriesPerResource);
        tlResourceContents.append(T());
    }
    // Sets the most efficient value for the number of worker threads to use.
    iTotalWorkers=iTotalResources;
    if(iTotalWorkers>MAX_DOWNLOAD_THREADS)
        iTotalWorkers=MAX_DOWNLOAD_THREADS;
    // Initializes the lists of helpers corresponding to earch worker thread used: ...
    // ... blWorkerResults holds the results of invoking BadooAPI::downloadMediaResource(). ...
    // ... ilWorkerIndexes holds the index of each resource assigned to the respective worker. ...
    // ... tlWorkerContents holds the downloaded path/data contents for the respective worker. ...
    // ... thlWorkerThreads holds a pointer to the respective worker thread. ...
    // ... slWorkerURLs holds the resource URL to be downloaded by the respective worker. ...
    // ... slWorkerErrors holds the errors (if any) of invoking BadooAPI::downloadMediaResource().
    for(iW=0;iW<iTotalWorkers;iW++) {
        blWorkerResults.append(false);
        ilWorkerIndexes.append(-1);
        tlWorkerContents.append(T());
        thlWorkerThreads.append(nullptr);
        slWorkerURLs.append(QString());
        slWorkerErrors.append(QString());
    }
    // Saves the result of a given worker and resets its helpers (to make it able to be re-used).
    std::function<bool(int)> fnCompleteWorkersJob=[&](int iIndex) {
        bool bResult=false;
        if(blWorkerResults.at(iIndex)) {
            tlResourceContents[ilWorkerIndexes.at(iIndex)]=tlWorkerContents.at(iIndex);
            bResult=true;
            emit statusChanged(QStringLiteral("Resource %1 of %2 downloaded").arg(++iProgress).arg(iTotalResources));
        }
        else
            if(0<ilResourceTries.at(ilWorkerIndexes.at(iIndex))) {
                // If the resource download (corresponding to the given worker) failed ...
                // ... but still can be retried, sets the resource status to 'non downloaded yet'.
                blResourceDownload[ilWorkerIndexes.at(iIndex)]=false;
                qDebug() << "Download failed for resource" << ilWorkerIndexes.at(iIndex) <<
                            ": [" << slWorkerErrors.at(iIndex) << "]" <<
                            "Remaining tries" << ilResourceTries.at(ilWorkerIndexes.at(iIndex));
            }
            else {
                // Stores the LAST error that occurred, with no chance of retries.
                sError=slWorkerErrors.at(iIndex);
                qDebug() << "Download for resource" << ilWorkerIndexes.at(iIndex) << "failed";
            }
        // Resets the worker helpers.
        delete thlWorkerThreads.at(iIndex);
        blWorkerResults[iIndex]=false;
        ilWorkerIndexes[iIndex]=-1;
        tlWorkerContents[iIndex]=T();
        thlWorkerThreads[iIndex]=nullptr;
        slWorkerURLs[iIndex]=QString();
        slWorkerErrors[iIndex]=QString();
        return bResult;
    };
    iProgress=0;
    emit statusChanged(QStringLiteral("Downloading %1 resource(s)...").arg(iTotalResources));
    while(true) {
        // Finds a resource whose download has not been started yet.
        for(iR=0;iR<iTotalResources;iR++)
            if(!blResourceDownload.at(iR))
                break;
        if(iR==iTotalResources) {
            // If ALL the resources are found to be download(ing/ed), ...
            // ... waits until the remaining (if any) active downloads are complete.
            int iTotalErrors=0;
            while(true) {
                int iTotalPending=0;
                for(iW=0;iW<iTotalWorkers;iW++)
                    if(nullptr!=thlWorkerThreads.at(iW))
                        // Saves the result of the worker (if finished).
                        if(thlWorkerThreads.at(iW)->isFinished()) {
                            if(!fnCompleteWorkersJob(iW))
                                iTotalErrors++;
                        }
                        else
                            iTotalPending++;
                if(!iTotalPending)
                    break;
                QApplication::processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
            }
            if(!iTotalErrors)
                break;
        }
        else {
            // If one unprocessed resource is found, ...
            // ... searches for a free worker slot.
            for(iW=0;iW<iTotalWorkers;iW++)
                if(nullptr==thlWorkerThreads.at(iW))
                    break;
            if(iW==iTotalWorkers) {
                // If no free worker slot is found, waits until one of them ...
                // is free again (it's gonna be re-used in the next main loop iteration).
                while(true) {
                    int iTotalFinished=0;
                    for(iW=0;iW<iTotalWorkers;iW++)
                        if(nullptr!=thlWorkerThreads.at(iW))
                            // Saves the result of every worker that has finished.
                            if(thlWorkerThreads.at(iW)->isFinished()) {
                                // Frees the worker, no matter if its job failed or not.
                                fnCompleteWorkersJob(iW);
                                iTotalFinished++;
                            }
                    if(iTotalFinished)
                        break;
                    QApplication::processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
                }
                // Exits the main loop if there's already an unrecoverable error, ...
                // ... since there's no point in downloading the remaining resources.
                if(!sError.isEmpty())
                    break;
            }
            else {
                // If a free worker slot is found, ...
                // ... sets the unprocessed resource status to 'downloading', ...
                // ... decreases the resource remaining download tries, ...
                // ... sets the free worker resource index and resource URL ...
                // ... to the resource about to be downloaded, ...
                // ... and creates (and starts) a new thread for the download task.
                blResourceDownload[iR]=true;
                ilResourceTries[iR]--;
                ilWorkerIndexes[iW]=iR;
                slWorkerURLs[iW]=slResourceURLs.at(iR);
                thlWorkerThreads[iW]=QThread::create(
                    [&](int iIndex) {
                        blWorkerResults[iIndex]=BadooAPI::downloadMediaResource(
                            sdSession.sSessionId,
                            slWorkerURLs.at(iIndex),
                            tlWorkerContents[iIndex],
                            slWorkerErrors[iIndex]
                        );
                    },
                    iW
                );
                thlWorkerThreads.at(iW)->start();
            }
        }
    }
    // Terminates by force every worker still running outside the main loop, ...
    // ... since this means that an unrecoverable error has occurred.
    emit statusChanged(QStringLiteral("Terminating pending downloads..."));
    for(iW=0;iW<iTotalWorkers;iW++)
        if(nullptr!=thlWorkerThreads.at(iW))
            if(!thlWorkerThreads.at(iW)->isFinished()) {
                thlWorkerThreads.at(iW)->terminate();
                thlWorkerThreads.at(iW)->wait();
            }
    emit statusChanged(QString());
    bResult=true;
    if(!sError.isEmpty()) {
        bResult=false;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

bool BadooWrapper::getEncounters(BadooUserProfileList &buplEncounters,
                                 bool                 bReset) {
    bool          bResult=false;
    QString       sError;
    BadooAPIError baeError;
    sError.clear();
    if(bReset)
        sLastEncountersId.clear();
    emit statusChanged(QStringLiteral("Loading encounters batch..."));
    if(BadooAPI::sendGetEncounters(
        sdSession.sSessionId,
        sLastEncountersId,
        MAX_USERS_BY_REQUEST,
        buplEncounters,
        baeError
    )) {
        if(buplEncounters.isEmpty())
            sLastEncountersId.clear();
        else
            sLastEncountersId=buplEncounters.last().sUserId;
        bResult=true;
    }
    emit statusChanged(QString());
    if(!bResult) {
        if(sError.isEmpty())
            sError=baeError.sErrorMessage;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

void BadooWrapper::getEncountersSettings(EncountersSettings &esSettings) {
    esSettings=esEncounters;
}

QString BadooWrapper::getHTMLFromProfile(BadooUserProfile  bupUser,
                                         bool              bFullHTML,
                                         QString           sStyle,
                                         QByteArrayList    abtlPhotos,
                                         QByteArrayList    abtlVideos) {
    int         iIndex,
                iRows,
                iCols;
    QString     sResult,
                sTitle,
                sURL,
                sDetails,
                sPhotos,
                sVideos,
                sBody;
    QStringList slPhotos,
                slVideos;
    sTitle=QStringLiteral("%1, %2, %3").arg(bupUser.sName).arg(bupUser.iAge).arg(bupUser.sCity);
    sURL=QStringLiteral("%1/profile/0%2").arg(ENDPOINT_BASE).arg(bupUser.sUserId);
    sDetails=QStringLiteral(HTML_TABLE_PROFILE).
             arg(bupUser.sName).arg(bupUser.iAge).arg(bupUser.sAppearance).
             arg(
                 this->getTextFromSexType(static_cast<BadooSexType>(bupUser.iGender)),
                 bupUser.sSexuality
             ).
             arg(
                 bupUser.sCountry,
                 bupUser.sRegion,
                 bupUser.sCity
             ).
             arg(
                 bupUser.sOnlineStatus,
                 this->getTextFromBoolean(bupUser.bIsVerified),
                 this->getTextFromBoolean(bupUser.bHasQuickChat)
             ).
             arg(bupUser.sAbout).
             arg(
                 bupUser.sIntent,
                 bupUser.sMood,
                 bupUser.sRelationshipStatus
             ).
             arg(
                 bupUser.sChildren,
                 bupUser.sSmoking,
                 bupUser.sDrinking
             ).
             arg(
                 this->getTextFromVote(bupUser.bvMyVote),
                 this->getTextFromVote(bupUser.bvTheirVote)
             ).
             arg(QStringLiteral("<a href='%1'>Click to open</a>").arg(sURL));

    slPhotos.clear();
    if(abtlPhotos.isEmpty())
        slPhotos=bupUser.slPhotos;
    else
        for(const auto &p:abtlPhotos)
            slPhotos.append(QStringLiteral("data:image/jpg;base64,%1").arg(p.toBase64()));
    iIndex=0;
    iCols=10;
    iRows=slPhotos.count()/iCols;
    if(slPhotos.count()%iCols)
        iRows++;
    sPhotos=QStringLiteral("<table>");
    for(int iR=0;iR<iRows;iR++) {
        sPhotos.append(QStringLiteral("<tr>"));
        for(int iC=0;iC<iCols;iC++) {
            sPhotos.append(
                QStringLiteral("<td><img src='%1'></td>").arg(slPhotos.at(iIndex))
            );
            if(++iIndex==slPhotos.count())
                break;
        }
        sPhotos.append(QStringLiteral("</tr>"));
    }
    sPhotos.append(QStringLiteral("</table>"));

    slVideos.clear();
    if(abtlVideos.isEmpty())
        slVideos=bupUser.slVideos;
    else
        for(const auto &v:abtlVideos)
            slVideos.append(QStringLiteral("data:video/mp4;base64,%1").arg(v.toBase64()));
    iIndex=0;
    iCols=10;
    iRows=slVideos.count()/iCols;
    if(slVideos.count()%iCols)
        iRows++;
    sVideos=QStringLiteral("<table>");
    for(int iR=0;iR<iRows;iR++) {
        sVideos.append(QStringLiteral("<tr>"));
        for(int iC=0;iC<iCols;iC++) {
            sVideos.append(
                QStringLiteral("<td><video autoplay loop muted><source src='%1'>Video</video></td>").
                arg(slVideos.at(iIndex))
            );
            if(++iIndex==slVideos.count())
                break;
        }
        sVideos.append(QStringLiteral("</tr>"));
    }
    sVideos.append(QStringLiteral("</table>"));

    sBody=QStringLiteral(JS_MEDIA_ONCLICK);
    sBody.append(sStyle);
    sBody.append(QStringLiteral("<b>%1</b>").arg(sTitle));
    sBody.append(QStringLiteral("<br>%1<br>").arg(sDetails));
    sBody.append(QStringLiteral("Photos: %1<br>").arg(slPhotos.count()));
    sBody.append(QStringLiteral("%1<br>").arg(sPhotos));
    sBody.append(QStringLiteral("Videos: %1<br>").arg(slVideos.count()));
    sBody.append(QStringLiteral("%1<br><br>").arg(sVideos));
    if(bFullHTML) {
        sResult=QStringLiteral("<html><head><title>%1</title></head>").arg(sTitle);
        sResult.append(QStringLiteral("<body>%1</body></html>").arg(sBody));
    }
    else sResult=sBody;
    return sResult;
}

QString BadooWrapper::getLastError() {
    return sLastError;
}

bool BadooWrapper::getLoggedInProfile(BadooUserProfile &bupUser) {
    bool    bResult=false;
    QString sError;
    sError.clear();
    if(this->isLoggedIn()) {
        bupUser=bupSelf;
        bResult=true;
    }
    else
        sError=QStringLiteral("Not logged in");
    if(!bResult)
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    sLastError=sError;
    return bResult;
}

void BadooWrapper::getPeopleNearbySettings(PeopleNearbySettings &pnsSettings) {
    pnsSettings=pnsPeopleNearby;
}

void BadooWrapper::getSessionDetails(SessionDetails &sdDetails) {
    sdDetails=sdSession;
}

bool BadooWrapper::isLoggedIn() {
    return !sdSession.sSessionId.isEmpty()&&!sdSession.sResponseToken.isEmpty();
}

bool BadooWrapper::loadSearchSettings() {
    bool                    bResult=false;
    QString                 sError;
    BadooSearchSettings     bssSettings;
    BadooIntRange           birAgeRange,
                            birDistanceRange;
    BadooStrKeyStrValueHash bsksvhDistanceHash;
    BadooIntKeyStrValueHash biksvhOwnIntentHash;
    BadooAPIError           baeError;
    sError.clear();
    emit statusChanged(QStringLiteral("Loading search settings..."));
    if(BadooAPI::sendGetSearchSettings(
        sdSession.sSessionId,
        SETTINGS_CONTEXT_TYPE_ENCOUNTERS,
        bssSettings,
        birAgeRange,
        birDistanceRange,
        bsksvhDistanceHash,
        biksvhOwnIntentHash,
        baeError
    )) {
        this->setEncountersSettings(
            bssSettings.bstlGenders,
            birAgeRange,
            bssSettings.birAge,
            birDistanceRange,
            bssSettings.iDistanceAway
        );
        if(BadooAPI::sendGetSearchSettings(
            sdSession.sSessionId,
            SETTINGS_CONTEXT_TYPE_PEOPLE_NEARBY,
            bssSettings,
            birAgeRange,
            birDistanceRange,
            bsksvhDistanceHash,
            biksvhOwnIntentHash,
            baeError
        )) {
            this->setPeopleNearbySettings(
                bssSettings.bstlGenders,
                birAgeRange,
                bssSettings.birAge,
                bssSettings.iLocationId,
                bssSettings.sLocationName,
                bsksvhDistanceHash,
                bssSettings.sDistanceCode,
                biksvhOwnIntentHash,
                bssSettings.iOwnIntentId
            );
            bResult=true;
        }
    }
    emit statusChanged(QString());
    if(!bResult) {
        this->clearEncountersSettings();
        this->clearPeopleNearbySettings();
        if(sError.isEmpty())
            sError=baeError.sErrorMessage;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

bool BadooWrapper::login(QString sUsername,
                         QString sPassword) {
    bool             bResult=false;
    QString          sAnonSessionId,
                     sSessionId,
                     sDeviceId,
                     sAccountId,
                     sResponseToken,
                     sError;
    BadooUserProfile bupUser;
    BadooAPIError    baeError;
    this->clearSessionDetails();
    sError.clear();
    emit statusChanged(QStringLiteral("Logging in..."));
    if(BadooAPI::getPreLoginParameters(sAnonSessionId,sDeviceId,sError))
        if(BadooAPI::sendStartup(sAnonSessionId,sDeviceId,sAccountId,bupUser,baeError))
            if(BadooAPI::sendLogin(sAnonSessionId,sUsername,sPassword,sSessionId,baeError))
                if(BadooAPI::sendStartup(sSessionId,sDeviceId,sAccountId,bupUser,baeError))
                    if(BadooAPI::getPostLoginParameters(sSessionId,sResponseToken,sError))
                        bResult=true;
    emit statusChanged(QString());
    if(bResult) {
        bupSelf=bupUser;
        this->setSessionDetails(sSessionId,sDeviceId,bupUser.sUserId,sAccountId,sResponseToken);
    }
    else {
        if(sError.isEmpty())
            sError=baeError.sErrorMessage;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

bool BadooWrapper::logout() {
    bool           bResult=false;
    uint           uiResCode;
    QString        sContentType,
                   sError;
    QByteArray     abtResponse;
    QUrl           urlEndPoint;
    QUrlQuery      qryEndPoint;
    RawHeadersHash rhhHeaders;
    sError.clear();
    if(this->isLoggedIn()) {
        urlEndPoint=QUrl(QStringLiteral(ENDPOINT_SIGNOUT));
        qryEndPoint.addQueryItem(QStringLiteral("rt"),sdSession.sResponseToken);
        urlEndPoint.setQuery(qryEndPoint);
        emit statusChanged(QStringLiteral("Logging out..."));
        HTTPRequest::get(
            urlEndPoint,
            {
                {
                    QStringLiteral("cookie").toUtf8(),
                    QStringLiteral("session=%1").arg(sdSession.sSessionId).toUtf8()
                }
            },
            {
                {
                    QNetworkRequest::Attribute::RedirectPolicyAttribute,
                    QNetworkRequest::RedirectPolicy::ManualRedirectPolicy
                }
            },
            uiResCode,
            abtResponse,
            rhhHeaders,
            sError
        );
        emit statusChanged(QString());
        sContentType=rhhHeaders.value(QStringLiteral("content-type").toUtf8());
        if(HTTP_STATUS_FOUND==uiResCode)
            // A redirection is more than enough to detect the successful logout.
            // By pure convention, we also verify the Content-Type header.
            if(sContentType.startsWith(QStringLiteral(HTTP_HEADER_CONTENT_TYPE_HTML)))
                bResult=true;
            else
                sError=QStringLiteral("Unexpected content type: %1").
                       arg(sContentType);
        else if(HTTP_STATUS_INVALID!=uiResCode)
            sError=QStringLiteral("Unexpected response code: %1").
                   arg(uiResCode);
    }
    else
        bResult=true;
    if(bResult)
        this->clearSessionDetails();
    else
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    sLastError=sError;
    return bResult;
}

bool BadooWrapper::saveSearchSettings() {
    bool                    bResult=false;
    QString                 sError;
    BadooSearchSettings     bssSettings,
                            bssCopy;
    BadooIntRange           birAgeRange,
                            birDistanceRange;
    BadooStrKeyStrValueHash bsksvhDistanceHash;
    BadooIntKeyStrValueHash biksvhOwnIntentHash;
    BadooAPIError           baeError;
    sError.clear();
    // Creates an unified search settings struct with values taken from all search contexts.
    bssSettings.bstlGenders=esEncounters.bstlGenders;
    bssSettings.birAge=esEncounters.birAge;
    bssSettings.iLocationId=pnsPeopleNearby.iLocationId;
    bssSettings.sLocationName=pnsPeopleNearby.sLocationName;
    bssSettings.iDistanceAway=esEncounters.iDistanceAway;
    bssSettings.sDistanceCode=pnsPeopleNearby.sDistanceCode;
    bssSettings.iOwnIntentId=pnsPeopleNearby.iOwnIntentId;
    // Makes a copy since sendSaveSearchSettings() can modify the unified struct.
    bssCopy=bssSettings;
    emit statusChanged(QStringLiteral("Saving search settings..."));
    if(BadooAPI::sendSaveSearchSettings(
        sdSession.sSessionId,
        SETTINGS_CONTEXT_TYPE_ENCOUNTERS,
        bssSettings,
        birAgeRange,
        birDistanceRange,
        bsksvhDistanceHash,
        biksvhOwnIntentHash,
        baeError
    )) {
        // Updates the Encounters settings with the values actually saved.
        this->setEncountersSettings(
            bssSettings.bstlGenders,
            birAgeRange,
            bssSettings.birAge,
            birDistanceRange,
            bssSettings.iDistanceAway
        );
        // Restores the unified search settings struct.
        bssSettings=bssCopy;
        // Edits the fields specific to the People Nearby context.
        bssSettings.bstlGenders=pnsPeopleNearby.bstlGenders;
        bssSettings.birAge=pnsPeopleNearby.birAge;
        if(BadooAPI::sendSaveSearchSettings(
            sdSession.sSessionId,
            SETTINGS_CONTEXT_TYPE_PEOPLE_NEARBY,
            bssSettings,
            birAgeRange,
            birDistanceRange,
            bsksvhDistanceHash,
            biksvhOwnIntentHash,
            baeError
        )) {
            // Updates the People Nearby settings with the values actually saved.
            this->setPeopleNearbySettings(
                bssSettings.bstlGenders,
                birAgeRange,
                bssSettings.birAge,
                bssSettings.iLocationId,
                bssSettings.sLocationName,
                bsksvhDistanceHash,
                bssSettings.sDistanceCode,
                biksvhOwnIntentHash,
                bssSettings.iOwnIntentId
            );
            bResult=true;
        }
    }
    emit statusChanged(QString());
    if(!bResult) {
        if(sError.isEmpty())
            sError=baeError.sErrorMessage;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

void BadooWrapper::setEncountersSettings(EncountersSettings esNew) {
    esEncounters=esNew;
}

void BadooWrapper::setPeopleNearbySettings(PeopleNearbySettings pnsNew) {
    pnsPeopleNearby=pnsNew;
}

bool BadooWrapper::showLogin() {
    bool             bResult=false;
    QString          sError;
    BadooLoginDialog bldLogin(this);
    sError.clear();
    if(bldLogin.show())
        bResult=true;
    else
        sError=QStringLiteral("Dialog canceled by user");
    if(!bResult) {
        if(sError.isEmpty())
            sError=sLastError;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

bool BadooWrapper::showSearchSettings(BadooSettingsContextType bsctContext) {
    bool                      bResult=false;
    QString                   sError;
    BadooSearchSettingsDialog bssdSettings(bsctContext,this);
    sError.clear();
    if(bssdSettings.show())
        bResult=true;
    else
        sError=QStringLiteral("Dialog canceled by user");
    if(!bResult) {
        if(sError.isEmpty())
            sError=sLastError;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

bool BadooWrapper::vote(QString sProfileId,
                        bool    bLike,
                        bool    &bMatch) {
    bool          bResult=false;
    QString       sError;
    BadooAPIError baeError;
    sError.clear();
    emit statusChanged(QStringLiteral("Sending vote..."));
    if(BadooAPI::sendEncountersVote(sdSession.sSessionId,sProfileId,bLike,bMatch,baeError))
        bResult=true;
    emit statusChanged(QString());
    if(!bResult) {
        if(sError.isEmpty())
            sError=baeError.sErrorMessage;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

void BadooWrapper::clearEncountersSettings() {
    esEncounters.bstlGenders.clear();
    esEncounters.birAgeRange.first=0;
    esEncounters.birAgeRange.second=0;
    esEncounters.birAge.first=0;
    esEncounters.birAge.second=0;
    esEncounters.birDistanceAwayRange.first=0;
    esEncounters.birDistanceAwayRange.second=0;
    esEncounters.iDistanceAway=0;
}

void BadooWrapper::clearPeopleNearbySettings() {
    pnsPeopleNearby.bstlGenders.clear();
    pnsPeopleNearby.birAgeRange.first=0;
    pnsPeopleNearby.birAgeRange.second=0;
    pnsPeopleNearby.birAge.first=0;
    pnsPeopleNearby.birAge.second=0;
    pnsPeopleNearby.iLocationId=0;
    pnsPeopleNearby.sLocationName.clear();
    pnsPeopleNearby.bsksvhDistanceHash.clear();
    pnsPeopleNearby.sDistanceCode.clear();
    pnsPeopleNearby.biksvhOwnIntentHash.clear();
    pnsPeopleNearby.iOwnIntentId=0;
}

void BadooWrapper::clearSessionDetails() {
    sdSession.sSessionId.clear();
    sdSession.sDeviceId.clear();
    sdSession.sUserId.clear();
    sdSession.sAccountId.clear();
    sdSession.sResponseToken.clear();
}

QString BadooWrapper::getTextFromBoolean(bool bBoolean) {
    return bBoolean?QStringLiteral("YES"):QStringLiteral("NO");
}

QString BadooWrapper::getTextFromSexType(BadooSexType bstSexType) {
    QString sResult;
    switch(bstSexType) {
        case SEX_TYPE_MALE:
            sResult=QStringLiteral("Male");
            break;
        case SEX_TYPE_FEMALE:
            sResult=QStringLiteral("Female");
            break;
        case SEX_TYPE_UNKNOWN:
            sResult=QStringLiteral("Unknown");
            break;
        case SEX_TYPE_OTHER:
            sResult=QStringLiteral("Other");
            break;
    }
    return sResult;
}

QString BadooWrapper::getTextFromVote(BadooVote bvVote) {
    QString sResult;
    switch(bvVote) {
        case VOTE_UNKNOWN:
            sResult=QStringLiteral("UNKOWN");
            break;
        case VOTE_NONE:
            sResult=QStringLiteral("NONE");
            break;
        case VOTE_YES:
            sResult=QStringLiteral("YES");
            break;
        case VOTE_NO:
            sResult=QStringLiteral("NOPE");
            break;
        case VOTE_MAYBE:
            sResult=QStringLiteral("MAYBE");
            break;
        case VOTE_SKIP:
            sResult=QStringLiteral("SKIP");
            break;
        case VOTE_SUPER:
            sResult=QStringLiteral("SUPER");
            break;
        case VOTE_CRUSH:
            sResult=QStringLiteral("CRUSH");
            break;
    }
    return sResult;
}

void BadooWrapper::setEncountersSettings(BadooSexTypeList bstlNewGenders,
                                         BadooIntRange    birNewAgeRange,
                                         BadooIntRange    birNewAge,
                                         BadooIntRange    birNewDistanceAwayRange,
                                         int              iNewDistanceAway) {
    esEncounters.bstlGenders=bstlNewGenders;
    esEncounters.birAgeRange=birNewAgeRange;
    esEncounters.birAge=birNewAge;
    esEncounters.birDistanceAwayRange=birNewDistanceAwayRange;
    esEncounters.iDistanceAway=iNewDistanceAway;
}

void BadooWrapper::setPeopleNearbySettings(BadooSexTypeList        bstlNewGenders,
                                           BadooIntRange           birNewAgeRange,
                                           BadooIntRange           birNewAge,
                                           int                     iNewLocationId,
                                           QString                 sNewLocationName,
                                           BadooStrKeyStrValueHash bsksvhNewDistanceHash,
                                           QString                 sNewDistanceCode,
                                           BadooIntKeyStrValueHash biksvhNewOwnIntentHash,
                                           int                     iNewOwnIntentId) {
    pnsPeopleNearby.bstlGenders=bstlNewGenders;
    pnsPeopleNearby.birAgeRange=birNewAgeRange;
    pnsPeopleNearby.birAge=birNewAge;
    pnsPeopleNearby.iLocationId=iNewLocationId;
    pnsPeopleNearby.sLocationName=sNewLocationName;
    pnsPeopleNearby.bsksvhDistanceHash=bsksvhNewDistanceHash;
    pnsPeopleNearby.sDistanceCode=sNewDistanceCode;
    pnsPeopleNearby.biksvhOwnIntentHash=biksvhNewOwnIntentHash;
    pnsPeopleNearby.iOwnIntentId=iNewOwnIntentId;
}

void BadooWrapper::setSessionDetails(QString sNewSessionId,
                                     QString sNewDeviceId,
                                     QString sNewUserId,
                                     QString sNewAccountId,
                                     QString sNewResponseToken) {
    sdSession.sSessionId=sNewSessionId;
    sdSession.sDeviceId=sNewDeviceId;
    sdSession.sUserId=sNewUserId;
    sdSession.sAccountId=sNewAccountId;
    sdSession.sResponseToken=sNewResponseToken;
}
