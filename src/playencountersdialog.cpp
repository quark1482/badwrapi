#include "playencountersdialog.h"

#define DIALOG_TITLE "Play Encounters"

PlayEncountersDialog::PlayEncountersDialog(BadooWrapper *bwParent,
                                           QWidget      *wgtParent):
QDialog(wgtParent) {
    bDialogReady=false;
    iCurrentProfileIndex=-1;
    buplEncounters.clear();
    mchPhotoContents.clear();
    mchVideoContents.clear();
    bwEncounters=bwParent;
    pvCurrentProfile=new ProfileViewer(bwEncounters,this);
    vblLayout.addWidget(pvCurrentProfile);
    this->setGeometry(0,0,640,480);
    this->setLayout(&vblLayout);
    this->setMinimumSize(400,300);
    connect(
        pvCurrentProfile,
        &ProfileViewer::buttonClicked,
        this,
        &PlayEncountersDialog::buttonClicked
    );
    if(this->getNewBatch(true)) {
        // Adding someone to Favorites is not possible when playing Encounters.
        pvCurrentProfile->setActiveActionButtons(~PROFILE_VIEWER_BUTTON_FAVORITE);
        // This may be a costly operation (because of thumbnails and/or videos) ...
        // ... if the profile has too much media, so I've decided to execute it ...
        // ... before giving the control to the user to avoid possible problems ...
        // ... with the event loop (happened once and I lost 48 hours trying to ...
        // ... figure out the source of the bug).
        this->showCurrentProfile();
        bDialogReady=true;
    }
}

PlayEncountersDialog::~PlayEncountersDialog() {
    delete pvCurrentProfile;
}

bool PlayEncountersDialog::isReady() {
    return bDialogReady;
}

void PlayEncountersDialog::buttonClicked(ProfileViewerButton pvbButton) {
    switch(pvbButton) {
        case PROFILE_VIEWER_BUTTON_COPY_URL:
            this->handleCopyURLButtonClick();
            break;
        case PROFILE_VIEWER_BUTTON_DOWNLOAD:
            this->handleDownloadProfileButtonClick();
            break;
        case PROFILE_VIEWER_BUTTON_BACK:
            this->handleBackButtonClick();
            break;
        case PROFILE_VIEWER_BUTTON_NOPE:
            this->handleNopeButtonClick();
            break;
        case PROFILE_VIEWER_BUTTON_FAVORITE:
            this->handleFavoriteButtonClick();
            break;
        case PROFILE_VIEWER_BUTTON_LIKE:
            this->handleLikeButtonClick();
            break;
        case PROFILE_VIEWER_BUTTON_SKIP:
            this->handleSkipButtonClick();
            break;
        case PROFILE_VIEWER_BUTTON_BLOCK:
            this->handleBlockButtonClick();
            break;
        case PROFILE_VIEWER_BUTTON_UNBLOCK:
            this->handleUnblockButtonClick();
            break;
        case PROFILE_VIEWER_BUTTON_UNMATCH:
            this->handleUnmatchButtonClick();
            break;
    }
}

QString PlayEncountersDialog::getCurrentProfileId() {
    QString sResult;
    sResult.clear();
    if(-1<iCurrentProfileIndex)
        sResult=buplEncounters.at(iCurrentProfileIndex).sUserId;
    return sResult;
}

bool PlayEncountersDialog::getNewBatch(bool bReset) {
    bool                 bResult=false;
    int                  iBackupProfileIndex;
    QString              sError;
    BadooUserProfileList buplBackupEncounters;
    MediaContentsHash    mchBackupPhotoContents,
                         mchBackupVideoContents;
    iBackupProfileIndex=iCurrentProfileIndex;
    buplBackupEncounters=buplEncounters;
    mchBackupPhotoContents=mchPhotoContents;
    mchBackupVideoContents=mchVideoContents;
    iCurrentProfileIndex=0;
    mchPhotoContents.clear();
    mchVideoContents.clear();
    this->setEnabled(false);
    this->setCursor(Qt::CursorShape::BusyCursor);
    QApplication::processEvents();
    while(true) {
        sError.clear();
        if(bwEncounters->getEncounters(buplEncounters,bReset))
            if(buplEncounters.count())
                if(bwEncounters->downloadMultiProfileResources<QByteArray>(
                    buplEncounters,
                    mchPhotoContents,
                    mchVideoContents
                )) {
                    bResult=true;
                    break;
                }
                else
                    sError=bwEncounters->getLastError();
            else
                sError=QStringLiteral("No available profiles were\n"
                                      "found to play Encounters");
        else
            sError=bwEncounters->getLastError();
        if(QMessageBox::StandardButton::Cancel==QMessageBox::question(
            this,
            QStringLiteral("Error"),
            sError,
            QMessageBox::StandardButton::Retry|QMessageBox::StandardButton::Cancel
        )) {
            iCurrentProfileIndex=iBackupProfileIndex;
            buplEncounters=buplBackupEncounters;
            mchPhotoContents=mchBackupPhotoContents;
            mchVideoContents=mchBackupVideoContents;
            break;
        }
    }
    this->unsetCursor();
    this->setEnabled(true);
    return bResult;
}

void PlayEncountersDialog::handleCopyURLButtonClick() {
    emit statusChanged(QString());
    if(-1<iCurrentProfileIndex)
        emit statusChanged(QStringLiteral("Profile URL copied to clipboard"));
}

void PlayEncountersDialog::handleDownloadProfileButtonClick() {
    emit statusChanged(QString());
    if(-1<iCurrentProfileIndex)
        emit statusChanged(QStringLiteral("Profile saved to disk"));
}

void PlayEncountersDialog::handleBackButtonClick() {
    bool bUpdate=false;
    emit statusChanged(QString());
    if(-1<iCurrentProfileIndex)
        if(iCurrentProfileIndex) {
            iCurrentProfileIndex--;
            bUpdate=true;
        }
        else
            emit statusChanged(QStringLiteral("Already at this batch's first profile"));
    if(bUpdate)
        this->showCurrentProfile();
}

void PlayEncountersDialog::handleNopeButtonClick() {
    emit statusChanged(QString());
    if(-1<iCurrentProfileIndex) {
        mchPhotoContents.remove(this->getCurrentProfileId());
        mchVideoContents.remove(this->getCurrentProfileId());
        buplEncounters.removeAt(iCurrentProfileIndex);
        if(iCurrentProfileIndex==buplEncounters.count()) {
            iCurrentProfileIndex--;
            this->getNewBatch();
        }
        this->showCurrentProfile();
    }
}

void PlayEncountersDialog::handleFavoriteButtonClick() {
    // Just a placeholder - this action is not gonna happen in Encounters.
    emit statusChanged(QString());
    if(-1<iCurrentProfileIndex) {
        this->showCurrentProfile();
    }
}

void PlayEncountersDialog::handleLikeButtonClick() {
    emit statusChanged(QString());
    if(-1<iCurrentProfileIndex) {
        mchPhotoContents.remove(this->getCurrentProfileId());
        mchVideoContents.remove(this->getCurrentProfileId());
        buplEncounters.removeAt(iCurrentProfileIndex);
        if(iCurrentProfileIndex==buplEncounters.count()) {
            iCurrentProfileIndex--;
            this->getNewBatch();
        }
        this->showCurrentProfile();
    }
}

void PlayEncountersDialog::handleSkipButtonClick() {
    bool bUpdate=false;
    emit statusChanged(QString());
    if(-1<iCurrentProfileIndex)
        if(iCurrentProfileIndex<buplEncounters.count()-1) {
            iCurrentProfileIndex++;
            bUpdate=true;
        }
        else
            bUpdate=this->getNewBatch();
    if(bUpdate)
        this->showCurrentProfile();
}

void PlayEncountersDialog::handleBlockButtonClick() {
    emit statusChanged(QString());
    if(-1<iCurrentProfileIndex) {
        // Blocking a profile in Encounters also implies voting Nope ...
        // ... to the profile. Hence, the following line is useless, ...
        // ... since the profile is being thrown away from the feed.
        buplEncounters[iCurrentProfileIndex].bIsBlocked=true;
        mchPhotoContents.remove(this->getCurrentProfileId());
        mchVideoContents.remove(this->getCurrentProfileId());
        buplEncounters.removeAt(iCurrentProfileIndex);
        if(iCurrentProfileIndex==buplEncounters.count()) {
            iCurrentProfileIndex--;
            this->getNewBatch();
        }
        emit statusChanged(QStringLiteral("Profile blocked"));
        this->showCurrentProfile();
    }
}

void PlayEncountersDialog::handleUnblockButtonClick() {
    // Just a placeholder - this action is not gonna happen in Encounters.
    emit statusChanged(QString());
    if(-1<iCurrentProfileIndex) {
        buplEncounters[iCurrentProfileIndex].bIsBlocked=false;
        emit statusChanged(QStringLiteral("Profile unblocked"));
        this->showCurrentProfile();
    }
}

void PlayEncountersDialog::handleUnmatchButtonClick() {
    // Just a placeholder - this action is not gonna happen in Encounters.
    emit statusChanged(QString());
    if(-1<iCurrentProfileIndex) {
        buplEncounters[iCurrentProfileIndex].bIsMatch=false;
        emit statusChanged(QStringLiteral("Profile unmatched"));
        this->showCurrentProfile();
    }
}

void PlayEncountersDialog::showCurrentProfile() {
    BadooUserProfile  buplCurrentProfile;
    QByteArrayList    abtlCurrentPhotos,
                      abtlCurrentVideos;
    BadooAPI::clearUserProfile(buplCurrentProfile);
    abtlCurrentPhotos.clear();
    abtlCurrentVideos.clear();
    if(-1<iCurrentProfileIndex) {
        buplCurrentProfile=buplEncounters.at(iCurrentProfileIndex);
        abtlCurrentPhotos=mchPhotoContents.value(this->getCurrentProfileId());
        abtlCurrentVideos=mchVideoContents.value(this->getCurrentProfileId());
        this->setWindowTitle(
            QStringLiteral("%1 - Profile %2 of %3").
            arg(QStringLiteral(DIALOG_TITLE)).
            arg(iCurrentProfileIndex+1).
            arg(buplEncounters.count())
        );
    }
    else
        this->setWindowTitle(QStringLiteral(DIALOG_TITLE));
    pvCurrentProfile->load(
        buplCurrentProfile,
        abtlCurrentPhotos,
        abtlCurrentVideos
    );
}
