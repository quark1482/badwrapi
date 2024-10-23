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
    abtSelfPhoto.clear();
    abtDefaultPhoto.clear();
    abtDefaultVideo.clear();
    BadooAPI::clearUserProfile(bupSelf);
    this->clearSessionDetails();
    this->clearEncountersSettings();
    this->clearPeopleNearbySettings();
}

bool BadooWrapper::addToBlocked(QString sProfileId) {
    bool          bResult=false;
    QString       sError;
    BadooAPIError baeError;
    sError.clear();
    emit statusChanged(QStringLiteral("Blocking profile..."));
    if(BadooAPI::sendAddPersonToFolder(
        sdSession.sSessionId,
        sProfileId,
        BLOCKED,
        baeError
    ))
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

bool BadooWrapper::addToFavorites(QString sProfileId) {
    bool          bResult=false;
    QString       sError;
    BadooAPIError baeError;
    sError.clear();
    emit statusChanged(QStringLiteral("Adding profile to favorites..."));
    if(BadooAPI::sendAddPersonToFolder(
        sdSession.sSessionId,
        sProfileId,
        FAVOURITES,
        baeError
    ))
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

void BadooWrapper::clearSessionDetails() {
    sdSession.sSessionId.clear();
    sdSession.sDeviceId.clear();
    sdSession.sUserId.clear();
    sdSession.sAccountId.clear();
}

template bool BadooWrapper::downloadMultiMediaResources<QString>(
    QStringList,QStringList &,QString,bool,int
);

template bool BadooWrapper::downloadMultiMediaResources<QByteArray>(
    QStringList,QByteArrayList &,QByteArray,bool,int
);

template<typename T>
bool BadooWrapper::downloadMultiMediaResources(QStringList slResourceURLs,
                                               QList<T>    &tlResourceContents,
                                               T           tDefaultContents,
                                               bool        bFailOnIndividualErrors,
                                               int         iMaxTriesPerResource) {
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
        ilResourceTries.append(iMaxTriesPerResource);
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
                qDebug() << "Unable to download resource" << ilWorkerIndexes.at(iIndex) <<
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
            // Exits the main loop if every resource has been downloaded, and with no errors.
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
                // Exits the main loop if an unrecoverable error has already occurred, ...
                // ... as continuing is pointless (no reason to download the remaining ...
                // ... resources, in case there's any).
                if(!sError.isEmpty())
                    if(bFailOnIndividualErrors)
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
    // Sets every empty download to a supplied default value.
    for(iR=0;iR<iTotalResources;iR++)
        if(tlResourceContents.at(iR).isEmpty())
            tlResourceContents[iR]=tDefaultContents;
    emit statusChanged(QString());
    if(bFailOnIndividualErrors)
        bResult=iProgress==iTotalResources;
    else
        bResult=true;
    if(bResult)
        sError.clear();
    else
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    sLastError=sError;
    return bResult;
}

template bool BadooWrapper::downloadMultiProfileResources<QString>(
    BadooUserProfileList,MediaContentsHash &,MediaContentsHash &,bool,int
);

template bool BadooWrapper::downloadMultiProfileResources<QByteArray>(
    BadooUserProfileList,MediaContentsHash &,MediaContentsHash &,bool,int
);

template<typename T>
bool BadooWrapper::downloadMultiProfileResources(BadooUserProfileList buplProfiles,
                                                 MediaContentsHash    &mchPhotoContents,
                                                 MediaContentsHash    &mchVideoContents,
                                                 bool                 bFailOnIndividualErrors,
                                                 int                  iMaxTriesPerResource) {
    bool        bResult=false;
    QString     sError;
    QStringList slPhotoURLs,
                slVideoURLs;
    QList<T>    tlPhotoContents,
                tlVideoContents;
    sError.clear();
    slPhotoURLs.clear();
    slVideoURLs.clear();
    mchPhotoContents.clear();
    mchVideoContents.clear();
    for(const auto &u:buplProfiles) {
        mchPhotoContents.insert(u.sUserId,{});
        mchVideoContents.insert(u.sUserId,{});
        slPhotoURLs.append(u.slPhotos);
        slVideoURLs.append(u.slVideos);
    }
    auto downloadMMR=[this](auto u,
                            auto &c,
                            auto d,
                            auto f,
                            auto t) {
        if constexpr(std::is_same_v<T,QString>)
            return this->downloadMultiMediaResources<T>(u,c,QString::fromUtf8(d),f,t);
        else if constexpr(std::is_same_v<T,QByteArray>)
            return this->downloadMultiMediaResources<T>(u,c,d,f,t);
        else
            return false;
    };
    if(downloadMMR(
        slPhotoURLs,
        tlPhotoContents,
        abtDefaultPhoto,
        bFailOnIndividualErrors,
        iMaxTriesPerResource
    ))
        if(downloadMMR(
            slVideoURLs,
            tlVideoContents,
            abtDefaultVideo,
            bFailOnIndividualErrors,
            iMaxTriesPerResource
        )) {
            int iPhotoCounter=0,
                iVideoCounter=0;
            // Deals with adding new content to a MediaContentHash, according ...
            // ... to the type the parent function template was invoked with.
            // This would allow a future caching of the profiles, since the ...
            // <QString> version saves the media contents to temporary files.
            auto appendMCH=[](auto key,
                              auto &hash,
                              auto item) {
                if constexpr(std::is_same_v<decltype(item),QString>)
                    hash[key].append({item.toUtf8()});
                else if constexpr(std::is_same_v<decltype(item),QByteArray>)
                    hash[key].append({item});
            };
            for(const auto &u:buplProfiles) {
                for(const auto &p:u.slPhotos)
                    appendMCH(u.sUserId,mchPhotoContents,tlPhotoContents.at(iPhotoCounter++));
                for(const auto &v:u.slVideos)
                    appendMCH(u.sUserId,mchVideoContents,tlVideoContents.at(iVideoCounter++));
            }
        }
        else
            sError=this->getLastError();
    else
        sError=this->getLastError();
    bResult=true;
    if(!sError.isEmpty()) {
        bResult=false;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

void BadooWrapper::getChatDirection(BadooUserProfile bupUser,
                                    ChatDirection    &cdDirection) {
   cdDirection=CHAT_DIRECTION_NONE;
    if(!bupUser.bcmLastMessage.sFromUserId.isEmpty())
        if(!bupUser.bcmLastMessage.sToUserId.isEmpty())
            if(bupUser.sUserId==bupUser.bcmLastMessage.sToUserId)
                cdDirection=CHAT_DIRECTION_PING;
            else if(bupUser.sUserId==bupUser.bcmLastMessage.sFromUserId)
                cdDirection=CHAT_DIRECTION_PONG;
}

bool BadooWrapper::getChatMessages(QString              sUserId,
                                   BadooChatMessageList &bcmlFullChat,
                                   int                  iHowMany) {
    bool                 bResult=false;
    QString              sError;
    int                  iOffset=0,
                         iTotal,
                         iRemainingTries;
    BadooAPIError        baeError;
    BadooChatMessageList bcmlMessages;
    sError.clear();
    bcmlFullChat.clear();
    emit statusChanged(QStringLiteral("Getting chat messages..."));
    iRemainingTries=MAX_DOWNLOAD_TRIES;
    while(true) {
        if(BadooAPI::sendGetChatMessages(
            sdSession.sSessionId,
            sUserId,
            iOffset,
            iHowMany,
            iTotal,
            bcmlMessages,
            baeError
        )) {
            if(bcmlMessages.isEmpty()) {
                bResult=true;
                break;
            }
            else {
                bcmlFullChat.append(bcmlMessages);
                if(iHowMany>0)
                    if(bcmlFullChat.count()>=iHowMany) {
                        if(bcmlFullChat.count()>iHowMany)
                            bcmlFullChat.resize(iHowMany);
                        bResult=true;
                        break;
                    }
                iOffset+=bcmlMessages.count();
                iRemainingTries=MAX_DOWNLOAD_TRIES;
                continue;
            }
        }
        else
            if(--iRemainingTries)
                continue;
        break;
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

bool BadooWrapper::getChatTotalMessages(QString sUserId,
                                        int     &iTotal) {
    bool                 bResult=false;
    int                  iRemainingTries;
    QString              sError;
    BadooAPIError        baeError;
    BadooChatMessageList bcmlMessages;
    sError.clear();
    iTotal=0;
    emit statusChanged(QStringLiteral("Getting chat total messages..."));
    iRemainingTries=MAX_DOWNLOAD_TRIES;
    while(true) {
        if(BadooAPI::sendGetChatMessages(
            sdSession.sSessionId,
            sUserId,
            0,
            0,
            iTotal,
            bcmlMessages,
            baeError
        )) {
            bResult=true;
            break;
        }
        else
            if(--iRemainingTries)
                continue;
        break;
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

QString BadooWrapper::getFolderName(FolderType ftType) {
    QString sResult;
    sResult.clear();
    switch(ftType) {
        case FOLDER_TYPE_UNKNOWN:
            sResult=QStringLiteral("Unknown folder");
            break;
        case FOLDER_TYPE_CHATS:
            sResult=QStringLiteral("Conversations");
            break;
        case FOLDER_TYPE_FAVORITES:
            sResult=QStringLiteral("Favorites");
            break;
        case FOLDER_TYPE_LIKES:
            sResult=QStringLiteral("Likes");
            break;
        case FOLDER_TYPE_MATCHES:
            sResult=QStringLiteral("Matches");
            break;
        case FOLDER_TYPE_PEOPLE_NEARBY:
            sResult=QStringLiteral("People nearby");
            break;
        case FOLDER_TYPE_VISITORS:
            sResult=QStringLiteral("Visitors");
            break;
        case FOLDER_TYPE_BLOCKED:
            sResult=QStringLiteral("Blocked people");
            break;
    }
    return sResult;
}

bool BadooWrapper::getFolderPage(BadooFolderType      bftFolder,
                                 BadooListSectionType blstSection,
                                 BadooListFilterList  blflFilters,
                                 int                  iPage,
                                 int                  iMaxProfileMediaCount,
                                 BadooUserProfileList &buplProfilesInPage,
                                 int                  &iTotalPagesInFolder,
                                 int                  &iTotalProfilesInFolder,
                                 int                  &iMaxProfilesByPage) {
    bool                 bResult=false;
    QString              sError,
                         sSectionId,
                         sSectionName;
    BadooAPIError        baeError;
    BadooUserProfileList buplProfilesInFolder;
    sError.clear();
    buplProfilesInPage.clear();
    iTotalPagesInFolder=0;
    iTotalProfilesInFolder=0;
    iMaxProfilesByPage=MAX_USERS_BY_REQUEST;
    emit statusChanged(QStringLiteral("Loading folder page #%1...").arg(iPage+1));
    if(BadooAPI::searchListSectionIdByType(
        sdSession.sSessionId,
        bftFolder,
        blstSection,
        sSectionId,
        sSectionName,
        baeError
    ))
        if(BadooAPI::sendGetUserList(
            sdSession.sSessionId,
            blflFilters,
            bftFolder,
            sSectionId,
            iPage*iMaxProfilesByPage,
            iMaxProfilesByPage,
            buplProfilesInPage,
            iTotalProfilesInFolder,
            baeError
        ))
            // Makes a second call to get the actual total of profiles in the folder.
            if(BadooAPI::sendGetUserList(
                sdSession.sSessionId,
                blflFilters,
                bftFolder,
                sSectionId,
                0,
                0,
                buplProfilesInFolder,
                iTotalProfilesInFolder,
                baeError
            )) {
                iTotalPagesInFolder=iTotalProfilesInFolder/iMaxProfilesByPage;
                if(iTotalProfilesInFolder%iMaxProfilesByPage)
                    iTotalPagesInFolder++;
                // Truncates both lists of media URLs for everyy user profile ...
                // ... when the respective counts are greater than a supplied ...
                // ... value, allowing the caller to optionally save time and ...
                // ... bandwidth in special cases.
                if(iMaxProfileMediaCount)
                    for(auto &p:buplProfilesInPage) {
                        if(p.slPhotos.count()>iMaxProfileMediaCount)
                            p.slPhotos.resize(iMaxProfileMediaCount);
                        if(p.slVideos.count()>iMaxProfileMediaCount)
                            p.slVideos.resize(iMaxProfileMediaCount);
                    }
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

bool BadooWrapper::getFolderPage(FolderType           ftType,
                                 BadooListFilterList  blflFilters,
                                 int                  iPage,
                                 int                  iMaxProfileMediaCount,
                                 BadooUserProfileList &buplProfilesInPage,
                                 int                  &iTotalPagesInFolder,
                                 int                  &iTotalProfilesInFolder,
                                 int                  &iMaxProfilesByPage) {
    bool                 bResult=false;
    QString              sError;
    BadooFolderType      bftFolder;
    BadooListSectionType blstSection;
    sError.clear();
    switch(ftType) {
        case FOLDER_TYPE_CHATS:
            bftFolder=ALL_MESSAGES;
            blstSection=LIST_SECTION_TYPE_ALL_MESSAGES;
            blflFilters.append(LIST_FILTER_CONVERSATIONS);
            break;
        case FOLDER_TYPE_FAVORITES:
            bftFolder=FAVOURITES;
            blstSection=LIST_SECTION_TYPE_FAVORITES;
            break;
        case FOLDER_TYPE_LIKES:
            bftFolder=WANT_TO_MEET_YOU;
            blstSection=LIST_SECTION_TYPE_WANT_TO_MEET_YOU_UNVOTED;
            break;
        case FOLDER_TYPE_MATCHES:
            // 2024-08-11: For a bunch of days, the folder type MATCHES was not ...
            // ... returning any profile. So I implemented this workaround, and ...
            // ... kept using it. Here, I'm grabbing all the profiles that were ...
            // ... included in a 'you got a match' notification.
            bftFolder=ALL_MESSAGES;
            blstSection=LIST_SECTION_TYPE_UNKNOWN;
            blflFilters.append(LIST_FILTER_MATCHED);
            break;
        case FOLDER_TYPE_PEOPLE_NEARBY:
            // 2024-10-13: Detected a problem with NEARBY_PEOPLE which disrupts ...
            // ... the quick_chat object in the responses of every request from ...
            // ... here on, causing that every profile comes with no quick chat ...
            // ... enabled, and the only known way to reset this behavior is by ...
            // ... logging out and back in.
            // 2024-10-13: Both the web and mobile apps experience this problem ...
            // ... as well, so it can be considered a real bug that their users ...
            // ... never report because they cannot see 'who can you chat with, ...
            // ... for free' in advance, but WE CAN because of the existence of ...
            // ... the folder type NEARBY_PEOPLE_WEB.
            // 2024-10-13: As previously discovered, there is a slight drawback ...
            // ... when we use this one: the Location in People nearby settings ...
            // ... is ignored, and the actual location of the logged-in profile ...
            // ... is used instead, which is not much of a hassle because there ...
            // ... is already a feature to change the Current location.
            bftFolder=NEARBY_PEOPLE_WEB;
            blstSection=LIST_SECTION_TYPE_UNKNOWN;
            break;
        case FOLDER_TYPE_VISITORS:
            bftFolder=PROFILE_VISITORS;
            blstSection=LIST_SECTION_TYPE_PROFILE_VISITORS;
            break;
        case FOLDER_TYPE_BLOCKED:
            bftFolder=BLOCKED;
            blstSection=LIST_SECTION_TYPE_UNKNOWN;
            break;
    }
    if(FOLDER_TYPE_UNKNOWN==ftType)
        sError=QStringLiteral("Unknown folder type");
    else
        bResult=this->getFolderPage(
            bftFolder,
            blstSection,
            blflFilters,
            iPage,
            iMaxProfileMediaCount,
            buplProfilesInPage,
            iTotalPagesInFolder,
            iTotalProfilesInFolder,
            iMaxProfilesByPage
        );
    if(!bResult) {
        if(sError.isEmpty())
            sError=this->getLastError();
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

QString BadooWrapper::getHTMLFromProfile(BadooUserProfile bupUser,
                                         bool             bFullHTML,
                                         QString          sStyle,
                                         QByteArrayList   abtlPhotos,
                                         QByteArrayList   abtlVideos) {
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
    sURL=QStringLiteral("https://%1/profile/%2").arg(DOMAIN_BASE).arg(bupUser.sUserId);
    sDetails=QStringLiteral(HTML_TABLE_PROFILE).
             arg(bupUser.sName).arg(bupUser.iAge).arg(bupUser.sAppearance).
             arg(
                 BadooWrapper::getTextFromSexType(
                     static_cast<BadooSexType>(bupUser.iGender)
                 ),
                 bupUser.sSexuality
             ).
             arg(
                 bupUser.sCountry,
                 bupUser.sRegion,
                 bupUser.sCity
             ).
             arg(
                 bupUser.sOnlineStatus,
                 BadooWrapper::getTextFromBoolean(bupUser.bIsVerified),
                 BadooWrapper::getTextFromBoolean(bupUser.bHasQuickChat)
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
                 BadooWrapper::getTextFromVote(bupUser.bvMyVote),
                 BadooWrapper::getTextFromVote(bupUser.bvTheirVote)
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
    BadooAPI::clearUserProfile(bupUser);
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

bool BadooWrapper::getLoggedInProfilePhoto(QByteArray &abtPhoto) {
    bool    bResult=false;
    QString sError;
    sError.clear();
    abtPhoto.clear();
    if(this->isLoggedIn()) {
        if(abtSelfPhoto.isEmpty()) {
            QStringList    slResources={bupSelf.sProfilePhotoURL};
            QByteArrayList abtlContents;
            if(this->downloadMultiMediaResources(slResources,abtlContents))
                abtSelfPhoto=abtlContents.first();
            else
                sError=this->getLastError();
        }
        if(!abtSelfPhoto.isEmpty()) {
            abtPhoto=abtSelfPhoto;
            bResult=true;
        }
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

bool BadooWrapper::getProfile(QString          sProfileId,
                              BadooUserProfile &bupUser) {
    bool          bResult=false;
    QString       sError;
    BadooAPIError baeError;
    sError.clear();
    BadooAPI::clearUserProfile(bupUser);
    emit statusChanged(QStringLiteral("Loading profile..."));
    if(BadooAPI::sendGetUser(sdSession.sSessionId,sProfileId,bupUser,baeError))
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

void BadooWrapper::getSessionDetails(SessionDetails &sdDetails) {
    sdDetails=sdSession;
}

void BadooWrapper::getSessionDetails(QString &sSessionId,
                                     QString &sDeviceId,
                                     QString &sUserId,
                                     QString &sAccountId) {
    sSessionId=sdSession.sSessionId;
    sDeviceId=sdSession.sDeviceId;
    sUserId=sdSession.sUserId;
    sAccountId=sdSession.sAccountId;
}

bool BadooWrapper::getVisibleStatus(bool &bVisible) {
    bool          bResult=false;
    QString       sError;
    QVariantHash  vhAppSettings;
    BadooAPIError baeError;
    sError.clear();
    bVisible=false;
    emit statusChanged(QStringLiteral("Querying visibility status..."));
    if(BadooAPI::sendGetAppSettings(sdSession.sSessionId,vhAppSettings,baeError)) {
        QString sKey=QStringLiteral("privacy_show_online_status");
        if(vhAppSettings.contains(sKey)&&vhAppSettings[sKey].typeId()==QMetaType::Type::Bool) {
            bVisible=vhAppSettings[sKey].toBool();
            bResult=true;
        }
        else
            sError=QStringLiteral("Could not find the visibility setting");
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
            sResult=QStringLiteral("UNKNOWN");
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

bool BadooWrapper::isEncryptedUserId(QString sUserId) {
    bool bResult=false;
    if(BadooWrapper::isValidUserId(sUserId))
        bResult=sUserId.startsWith(QStringLiteral(PREFIX_ENCRYPTED_USER_ID));
    return bResult;
}

bool BadooWrapper::isValidByRegEx(QString sText,
                                  QString sRegEx) {
    bool                    bResult=false;
    QRegularExpression      rxRegEx;
    QRegularExpressionMatch rxmMatch;
    rxRegEx.setPattern(sRegEx);
    rxmMatch=rxRegEx.match(sText);
    if(rxmMatch.hasMatch())
        bResult=rxmMatch.captured(0)==sText;
    return bResult;
}

bool BadooWrapper::isValidSessionId(QString sSessionId) {
    // As of 2024-10-11, I am not sure about the exact regex for a session id.
    // For now, I'll be using the Edit Session dialog to detect any conflicts.
    return BadooWrapper::isValidByRegEx(
        sSessionId,
        QStringLiteral(
            "s\\d:\\d{3}:[A-Za-z0-9]{40}"
        )
    );
}

bool BadooWrapper::isValidDeviceId(QString sDeviceId) {
    return BadooWrapper::isValidByRegEx(
        sDeviceId,
        QStringLiteral(
            "[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}"
        )
    );
}

bool BadooWrapper::isValidUserId(QString sUserId) {
    return BadooWrapper::isValidByRegEx(
        sUserId,
        QStringLiteral("(%2|%3)[A-Za-z0-9-_]{%1,}").
        arg(MIN_USER_ID_LENGTH).
        arg(
            QStringLiteral(PREFIX_NORMAL_USER_ID),
            QStringLiteral(PREFIX_ENCRYPTED_USER_ID)
        )
    );
}

bool BadooWrapper::isValidAccountId(QString sAccountId) {
    // As of 2024-10-11, I am not sure about the exact regex for an account id.
    // For now, I'll be using the Edit Session dialog to detect any conflicts.
    return BadooWrapper::isValidByRegEx(
        sAccountId,
        QStringLiteral(
            "\\d{10}"
        )
    );
}

bool BadooWrapper::isLoggedIn() {
    return !sdSession.sSessionId.isEmpty();
}

bool BadooWrapper::loadOwnProfile() {
    bool          bResult=false;
    QString       sError;
    BadooAPIError baeError;
    sError.clear();
    BadooAPI::clearUserProfile(bupSelf);
    emit statusChanged(QStringLiteral("Loading own profile..."));
    if(BadooAPI::sendGetUser(sdSession.sSessionId,sdSession.sUserId,bupSelf,baeError))
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
                     sStoryId,
                     sFlowId,
                     sError;
    BadooUserProfile bupUser;
    BadooAPIError    baeError;
    sAnonSessionId.clear();
    sError.clear();
    emit statusChanged(QStringLiteral("Logging in..."));
    this->clearSessionDetails();
    if(BadooAPI::getPreLoginParameters(sDeviceId,sError))
        if(BadooAPI::sendStartup(sDeviceId,sAnonSessionId,sAccountId,sStoryId,sFlowId,bupUser,baeError))
            if(BadooAPI::sendLogin(sAnonSessionId,sUsername,sPassword,sStoryId,sFlowId,sSessionId,baeError))
                if(BadooAPI::sendStartup(sDeviceId,sSessionId,sAccountId,sStoryId,sFlowId,bupUser,baeError))
                    bResult=true;
    emit statusChanged(QString());
    if(bResult) {
        bupSelf=bupUser;
        this->setSessionDetails(sSessionId,sDeviceId,bupUser.sUserId,sAccountId);
    }
    else {
        if(sError.isEmpty())
            sError=baeError.sErrorMessage;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

bool BadooWrapper::loginBack() {
    bool             bResult=false;
    QString          sSessionId,
                     sDeviceId,
                     sUserId,
                     sAccountId,
                     sStoryId,
                     sFlowId,
                     sError;
    BadooUserProfile bupUser;
    BadooAPIError    baeError;
    sError.clear();
    emit statusChanged(QStringLiteral("Logging back in..."));
    this->getSessionDetails(sSessionId,sDeviceId,sUserId,sAccountId);
    if(BadooAPI::sendStartup(sDeviceId,sSessionId,sAccountId,sStoryId,sFlowId,bupUser,baeError))
        bResult=true;
    emit statusChanged(QString());
    if(bResult) {
        // Pure convention. None of these values should change after the above startup.
        bupSelf=bupUser;
        this->setSessionDetails(sSessionId,sDeviceId,bupUser.sUserId,sAccountId);
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
    QString        sError;
    BadooAPIError  baeError;
    sError.clear();
    if(this->isLoggedIn()) {
        emit statusChanged(QStringLiteral("Logging out..."));
        bResult=BadooAPI::sendLogout(sdSession.sSessionId,baeError);
        emit statusChanged(QString());
    }
    else
        bResult=true;
    if(bResult)
        this->clearSessionDetails();
    else {
        if(sError.isEmpty())
            sError=baeError.sErrorMessage;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

bool BadooWrapper::removeFromBlocked(QString sProfileId) {
    bool          bResult=false;
    QString       sError;
    BadooAPIError baeError;
    sError.clear();
    emit statusChanged(QStringLiteral("Unblocking profile..."));
    if(BadooAPI::sendRemovePersonFromSection(
        sdSession.sSessionId,
        sProfileId,
        BLOCKED,
        LIST_SECTION_TYPE_GENERAL,
        SECTION_USER_DELETE,
        baeError
    ))
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

bool BadooWrapper::removeFromFavorites(QString sProfileId) {
    bool          bResult=false;
    QString       sError;
    BadooAPIError baeError;
    sError.clear();
    emit statusChanged(QStringLiteral("Removing profile from favorites..."));
    if(BadooAPI::sendRemovePersonFromSection(
        sdSession.sSessionId,
        sProfileId,
        FAVOURITES,
        LIST_SECTION_TYPE_UNKNOWN,
        SECTION_USER_DELETE,
        baeError
    ))
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

bool BadooWrapper::removeFromMatches(QString sProfileId) {
    bool          bResult=false;
    QString       sError;
    BadooAPIError baeError;
    sError.clear();
    emit statusChanged(QStringLiteral("Unmatching profile..."));
    // This is the correct and safest way of removing a match, with some unfortunate ...
    // ... side effects, as it's described in FolderViewer::showStandaloneProfile().
    if(BadooAPI::sendRemovePersonFromSection(
        sdSession.sSessionId,
        sProfileId,
        FOLDER_TYPE_COMBINED_CONNECTIONS_ALL,
        LIST_SECTION_TYPE_UNKNOWN,
        SECTION_ACTION_TYPE_USER_DELETE_FOR_ALL,
        baeError
    ))
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

bool BadooWrapper::sendChatMessage(QString sUserId,
                                   QString sMessage) {
    bool          bResult=false;
    QString       sError;
    BadooAPIError baeError;
    sError.clear();
    emit statusChanged(QStringLiteral("Sending message..."));
    // Only text messages are supported for now.
    bResult=BadooAPI::sendSendChatMessage(
        sdSession.sSessionId,
        sdSession.sUserId,
        sUserId,
        sMessage,
        baeError
    );
    emit statusChanged(QString());
    if(!bResult) {
        if(sError.isEmpty())
            sError=baeError.sErrorMessage;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

void BadooWrapper::setDefaultMedia(QByteArray abtNewDefaultPhoto,
                                   QByteArray abtNewDefaultVideo) {
    abtDefaultPhoto=abtNewDefaultPhoto;
    abtDefaultVideo=abtNewDefaultVideo;
}

void BadooWrapper::setEncountersSettings(EncountersSettings esNew) {
    esEncounters=esNew;
}

bool BadooWrapper::setLocation(QString sLocation) {
    bool             bResult=false;
    float            fLat,
                     fLng;
    QString          sSessionId,
                     sDeviceId,
                     sUserId,
                     sAccountId,
                     sStoryId,
                     sFlowId,
                     sError;
    BadooUserProfile bupUser;
    BadooAPIError    baeError;
    sError.clear();
    emit statusChanged(QStringLiteral("Changing location..."));
    this->getSessionDetails(sSessionId,sDeviceId,sUserId,sAccountId);
    if(TAGeoCoder::getLatLng(sLocation,fLat,fLng,sError))
        if(BadooAPI::sendUpdateLocation(sdSession.sSessionId,fLat,fLng,baeError))
            if(BadooAPI::sendStartup(sDeviceId,sSessionId,sAccountId,sStoryId,sFlowId,bupUser,baeError))
                bResult=true;
    emit statusChanged(QString());
    if(bResult) {
        bupSelf=bupUser;
        this->setSessionDetails(sSessionId,sDeviceId,bupUser.sUserId,sAccountId);
    }
    else {
        if(sError.isEmpty())
            sError=baeError.sErrorMessage;
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

void BadooWrapper::setPeopleNearbySettings(PeopleNearbySettings pnsNew) {
    pnsPeopleNearby=pnsNew;
}

void BadooWrapper::setSessionDetails(SessionDetails sdNewDetails) {
    sdSession=sdNewDetails;
}

void BadooWrapper::setSessionDetails(QString sNewSessionId,
                                     QString sNewDeviceId,
                                     QString sNewUserId,
                                     QString sNewAccountId) {
    sdSession.sSessionId=sNewSessionId;
    sdSession.sDeviceId=sNewDeviceId;
    sdSession.sUserId=sNewUserId;
    sdSession.sAccountId=sNewAccountId;
}

bool BadooWrapper::setVisibleStatus(bool bVisible) {
    bool          bResult=false;
    QString       sError;
    QVariantHash  vhAppSettings;
    BadooAPIError baeError;
    sError.clear();
    if(bVisible)
        emit statusChanged(QStringLiteral("Becoming visible..."));
    else
        emit statusChanged(QStringLiteral("Becoming invisible..."));
    vhAppSettings={{QStringLiteral("privacy_show_online_status"),bVisible}};
    if(BadooAPI::sendSaveAppSettings(sdSession.sSessionId,vhAppSettings,baeError))
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

bool BadooWrapper::showLocationDialog(QWidget *wgtParent) {
    bool                bResult=false;
    QString             sError;
    BadooLocationDialog bldLocation(this,wgtParent);
    if(bldLocation.show())
        bResult=true;
    else
        sError=QStringLiteral("Dialog canceled by user");
    if(!bResult) {
        if(sError.isEmpty())
            sError=this->getLastError();
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

bool BadooWrapper::showLoginDialog(QWidget *wgtParent) {
    bool             bResult=false;
    QString          sError;
    BadooLoginDialog bldLogin(this,wgtParent);
    sError.clear();
    if(bldLogin.show())
        bResult=true;
    else
        sError=QStringLiteral("Dialog canceled by user");
    if(!bResult) {
        if(sError.isEmpty())
            sError=this->getLastError();
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    sLastError=sError;
    return bResult;
}

bool BadooWrapper::showSearchSettingsDialog(BadooSettingsContextType bsctContext,
                                            QWidget                  *wgtParent) {
    bool                      bResult=false;
    QString                   sError;
    BadooSearchSettingsDialog bssdSettings(bsctContext,this,wgtParent);
    sError.clear();
    if(bssdSettings.show())
        bResult=true;
    else
        sError=QStringLiteral("Dialog canceled by user");
    if(!bResult) {
        if(sError.isEmpty())
            sError=this->getLastError();
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
