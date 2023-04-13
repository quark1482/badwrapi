#include "playencountersdialog.h"

#define DIALOG_TITLE "Play Encounters"

PlayEncountersDialog::PlayEncountersDialog(BadooWrapper *bwParent,
                                           QWidget      *wgtParent):QDialog(wgtParent) {
    bDialogReady=false;
    iCurrentProfileIndex=-1;
    buplEncounters.clear();
    mchPhotoContents.clear();
    mchVideoContents.clear();
    bwEncounters=bwParent;
    pvCurrentProfile=new ProfileViewer(bwEncounters,this);
    vblLayout.addWidget(pvCurrentProfile);
    this->loadMyProfile();
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
        bDialogReady=true;
        QTimer::singleShot(
            0,
            [=]() {
                this->showCurrentProfile();
            }
        );
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
        case PROFILE_VIEWER_BUTTON_LIKE:
            this->handleLikeButtonClick();
            break;
        case PROFILE_VIEWER_BUTTON_SKIP:
            this->handleSkipButtonClick();
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
                if(bwEncounters->downloadMultiProfileResources(
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
            emit statusChanged(QStringLiteral("Already at first profile"));
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

void PlayEncountersDialog::loadMyProfile() {
    QByteArray       abtMyProfilePhoto;
    BadooUserProfile bupMyProfile;
    pvCurrentProfile->getPlaceholderPhoto(abtMyProfilePhoto);
    if(bwEncounters->getLoggedInProfile(bupMyProfile)) {
        QStringList    slResources={bupMyProfile.sProfilePhotoURL};
        QByteArrayList abtlContents;
        if(bwEncounters->downloadMultiMediaResources(slResources,abtlContents))
            abtMyProfilePhoto=abtlContents.first();
    }
    pvCurrentProfile->setOwnProfilePhoto(abtMyProfilePhoto);
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
