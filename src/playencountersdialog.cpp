#include "playencountersdialog.h"

#define DIALOG_TITLE "Play Encounters"

#define FILTER_HTML_FILES "HTML files (*.html;*.htm)"

#define EXTENSION_HTML ".html"

#define HTML_STYLE_PROFILE R"(
<style type='text/css'>
    th {
        border-radius: 15px;
        background-color: #6e3eff;
        color: white;
        font-weight: bold;
        padding: 5px;
    }
    td {
        border-radius: 15px;
        background-color: #ffbbd0;
        padding: 5px;
    }
    img {
        width: 100px;
        height: 100px;
        border-radius: 10px;
        background-color: black;
        object-fit: contain;
        cursor: pointer;
    }
    video {
        width: 100px;
        height: 100px;
        border-radius: 10px;
        background-color: black;
        cursor: pointer;
    }
</style>
)"

PlayEncountersDialog::PlayEncountersDialog(BadooWrapper *bwParent,
                                           QWidget      *wgtParent):QDialog(wgtParent) {
    bDialogReady=false;
    iCurrentProfileIndex=-1;
    mchPhotoContents.clear();
    mchVideoContents.clear();
    bwEncounters=bwParent;
    pvCurrentProfile=new ProfileViewer(this);
    vblMain.addWidget(pvCurrentProfile);
    this->loadMyProfile();
    this->setGeometry(0,0,640,480);
    this->setLayout(&vblMain);
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
    BadooUserProfileList buplBackupEcounters;
    MediaContentsHash    mchBackupPhotoContents,
                         mchBackupVideoContents;
    iBackupProfileIndex=iCurrentProfileIndex;
    buplBackupEcounters=buplEncounters;
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
            if(buplEncounters.count()) {
                int               iPhotoCounter,
                                  iVideoCounter;
                QStringList       slPhotoURLs,
                                  slVideoURLs;
                QByteArrayList    abtlPhotoContents,
                                  abtlVideoContents;
                slPhotoURLs.clear();
                for(const auto &u:buplEncounters) {
                    mchPhotoContents.insert(u.sUserId,{});
                    mchVideoContents.insert(u.sUserId,{});
                    slPhotoURLs.append(u.slPhotos);
                    slVideoURLs.append(u.slVideos);
                }
                if(bwEncounters->downloadMultiMediaResources<QByteArray>(
                    slPhotoURLs,
                    abtlPhotoContents
                ))
                    if(bwEncounters->downloadMultiMediaResources<QByteArray>(
                        slVideoURLs,
                        abtlVideoContents
                    )) {
                        iPhotoCounter=iVideoCounter=0;
                        for(const auto &u:buplEncounters) {
                            for(const auto &p:u.slPhotos)
                                mchPhotoContents[u.sUserId].append(
                                    abtlPhotoContents[iPhotoCounter++]
                                );
                            for(const auto &v:u.slVideos)
                                mchVideoContents[u.sUserId].append(
                                    abtlVideoContents[iVideoCounter++]
                                );
                        }
                        bResult=true;
                        break;
                    }
            }
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
            buplEncounters=buplBackupEcounters;
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
    if(-1<iCurrentProfileIndex) {
        QString sURL=QStringLiteral("%1/profile/0%2").
                     arg(ENDPOINT_BASE).
                     arg(this->getCurrentProfileId());
        QGuiApplication::clipboard()->setText(sURL);
        emit statusChanged(QStringLiteral("Profile URL copied to clipboard"));
    }
}

void PlayEncountersDialog::handleDownloadProfileButtonClick() {
    emit statusChanged(QString());
    if(-1<iCurrentProfileIndex) {
        QString sDefaultFolder,
                sTempPath,
                sHTML;
        QFile   fFile;
        sDefaultFolder=QStandardPaths::standardLocations(
            QStandardPaths::StandardLocation::DownloadLocation
        ).at(0);
        sTempPath=QFileDialog::getSaveFileName(
            this,
            QStringLiteral("Save profile as HTML"),
            sDefaultFolder,
            QStringLiteral(FILTER_HTML_FILES)
        );
        if(!sTempPath.isEmpty()) {
            sHTML=bwEncounters->getHTMLFromProfile(
                buplEncounters.at(iCurrentProfileIndex),
                true,
                QStringLiteral(HTML_STYLE_PROFILE),
                mchPhotoContents.value(this->getCurrentProfileId()),
                mchVideoContents.value(this->getCurrentProfileId())
            );
            if(QFileInfo(sTempPath).suffix().isEmpty())
                sTempPath.append(QStringLiteral(EXTENSION_HTML));
            fFile.setFileName(sTempPath);
            if(fFile.open(QFile::OpenModeFlag::WriteOnly))
                if(-1!=fFile.write(sHTML.toUtf8())) {
                    emit statusChanged(QStringLiteral("Profile saved to disk"));
                    QMessageBox::information(
                        this,
                        QStringLiteral(DIALOG_TITLE),
                        QStringLiteral("Profile saved to disk")
                    );
                }
            if(QFile::FileError::NoError!=fFile.error())
                QMessageBox::critical(
                    this,
                    QStringLiteral("Error"),
                    fFile.errorString()
                );
        }
    }
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
        bool bMatch;
        if(bwEncounters->vote(this->getCurrentProfileId(),false,bMatch)) {
            mchPhotoContents.remove(this->getCurrentProfileId());
            mchVideoContents.remove(this->getCurrentProfileId());
            buplEncounters.removeAt(iCurrentProfileIndex);
            if(iCurrentProfileIndex==buplEncounters.count()) {
                iCurrentProfileIndex--;
                this->getNewBatch();
            }
        }
        else
            QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                bwEncounters->getLastError()
            );
        this->showCurrentProfile();
    }
}

void PlayEncountersDialog::handleLikeButtonClick() {
    emit statusChanged(QString());
    if(-1<iCurrentProfileIndex) {
        bool bMatch;
        if(bwEncounters->vote(this->getCurrentProfileId(),true,bMatch)) {
            if(bMatch)
                this->showMatch(
                    buplEncounters.at(iCurrentProfileIndex).sName,
                    mchPhotoContents.value(this->getCurrentProfileId()).first(),
                    abtMyProfilePhoto
                );
            mchPhotoContents.remove(this->getCurrentProfileId());
            mchVideoContents.remove(this->getCurrentProfileId());
            buplEncounters.removeAt(iCurrentProfileIndex);
            if(iCurrentProfileIndex==buplEncounters.count()) {
                iCurrentProfileIndex--;
                this->getNewBatch();
            }
        }
        else
            QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                bwEncounters->getLastError()
            );
        this->showCurrentProfile();
    }
}

void PlayEncountersDialog::handleSkipButtonClick() {
    bool bUpdate=false;
    emit statusChanged(QString());
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
    BadooUserProfile bupMyProfile;
    if(bwEncounters->getLoggedInProfile(bupMyProfile)) {
        QStringList    slResources={bupMyProfile.sProfilePhotoURL};
        QByteArrayList abtlContents;
        if(bwEncounters->downloadMultiMediaResources(slResources,abtlContents))
            abtMyProfilePhoto=abtlContents.first();
        else {
            QByteArray abtPlaceholder;
            pvCurrentProfile->getPlaceholderPhoto(abtPlaceholder);
            abtMyProfilePhoto=abtPlaceholder;
        }
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

void PlayEncountersDialog::showMatch(QString    sName,
                                     QByteArray abtPhoto1,
                                     QByteArray abtPhoto2,
                                     int        iCanvasHeight) {
    int         iPhotoSide=iCanvasHeight/4,
                iMatchTextFlags=Qt::TextFlag::TextWordWrap|Qt::TextFlag::TextJustificationForced,
                iMaxFontSize;
    QString     sMatchText;
    QMessageBox mbMatch;
    QPixmap     pxmMatch(iCanvasHeight*0.75,iCanvasHeight),
                pxmPhoto1=QPixmap::fromImage(
                    QImage::fromData(abtPhoto1)
                ).scaled(iPhotoSide,iPhotoSide,Qt::AspectRatioMode::KeepAspectRatio),
                pxmPhoto2=QPixmap::fromImage(
                    QImage::fromData(abtPhoto2)
                ).scaled(iPhotoSide,iPhotoSide,Qt::AspectRatioMode::KeepAspectRatio),
                pxmPhotoBox1(iPhotoSide,iPhotoSide),
                pxmPhotoBox2(iPhotoSide,iPhotoSide);
    QPainter    pntMatch;
    QFont       fntText;
    QPointF     ptfPhotosTopLeft(
                    pxmMatch.width()/2-iPhotoSide,
                    pxmMatch.height()/2-iPhotoSide
                ),
                ptfPhoto1Center(
                    ptfPhotosTopLeft.x()+iPhotoSide/2,
                    ptfPhotosTopLeft.y()+iPhotoSide/2
                ),
                ptfPhoto2Center(
                    ptfPhotosTopLeft.x()+iPhotoSide+iPhotoSide/2,
                    ptfPhotosTopLeft.y()+iPhotoSide/2
                );
    QRectF      rcfMatchTextArea(
                    ptfPhotosTopLeft.x(),
                    pxmMatch.height()-ptfPhotosTopLeft.y()-iPhotoSide/2,
                    iPhotoSide*2,
                    iPhotoSide
                );
    sMatchText=QStringLiteral("%1 likes you too! Will you be the one to make the first move?").
               arg(sName);
    fntText.setFamily(QStringLiteral("Arial"));
    fntText.setBold(true);
    iMaxFontSize=0;
    while(true) {
        fntText.setPointSize(iMaxFontSize+1);
        int iNewHeight=QFontMetrics(fntText,&pxmMatch).boundingRect(
            rcfMatchTextArea.toRect(),
            iMatchTextFlags,
            sMatchText
        ).height();
        if(rcfMatchTextArea.height()<iNewHeight)
            break;
        iMaxFontSize++;
    }
    fntText.setPointSize(iMaxFontSize);

    pntMatch.begin(&pxmPhotoBox1);
    pntMatch.fillRect(
        pxmPhotoBox1.rect(),
        Qt::GlobalColor::black
    );
    pntMatch.drawPixmap(
        iPhotoSide/2-pxmPhoto1.width()/2,
        iPhotoSide/2-pxmPhoto1.height()/2,
        pxmPhoto1
    );
    pntMatch.end();

    pntMatch.begin(&pxmPhotoBox2);
    pntMatch.fillRect(
        pxmPhotoBox2.rect(),
        Qt::GlobalColor::black
    );
    pntMatch.drawPixmap(
        iPhotoSide/2-pxmPhoto2.width()/2,
        iPhotoSide/2-pxmPhoto2.height()/2,
        pxmPhoto2
    );
    pntMatch.end();

    pntMatch.begin(&pxmMatch);
    pntMatch.fillRect(pxmMatch.rect(),QColor(QStringLiteral("#6e3eff")));
    pntMatch.setFont(fntText);
    pntMatch.setPen(Qt::GlobalColor::white);
    pntMatch.save();
    pntMatch.translate(ptfPhoto2Center);
    pntMatch.rotate(25);
    pntMatch.translate(-ptfPhoto2Center);
    pntMatch.drawPixmap(ptfPhotosTopLeft+QPointF(iPhotoSide,0),pxmPhotoBox2);
    pntMatch.restore();
    pntMatch.save();
    pntMatch.translate(ptfPhoto1Center);
    pntMatch.rotate(-25);
    pntMatch.translate(-ptfPhoto1Center);
    pntMatch.drawPixmap(ptfPhotosTopLeft,pxmPhotoBox1);
    pntMatch.restore();
    pntMatch.drawText(rcfMatchTextArea,iMatchTextFlags,sMatchText);
    pntMatch.end();

    mbMatch.setIconPixmap(pxmMatch);
    mbMatch.setText(QStringLiteral("It's a match!"));
    mbMatch.setWindowTitle(QStringLiteral(DIALOG_TITLE));
    QGridLayout *glyLayout=qobject_cast<QGridLayout *>(mbMatch.layout());
    QLayoutItem *lyiItem=glyLayout->takeAt(2);
    QLabel      *lblText=qobject_cast<QLabel *>(lyiItem->widget());
    QPalette    palText;
    palText.setColor(QPalette::ColorRole::WindowText,Qt::GlobalColor::green);
    lblText->setAlignment(Qt::AlignmentFlag::AlignCenter);
    lblText->setFont(fntText);
    lblText->setPalette(palText);
    glyLayout->addItem(lyiItem,0,1);
    glyLayout->addItem(glyLayout->takeAt(0),0,1);
    iMaxFontSize=0;
    while(true) {
        fntText.setPointSize(iMaxFontSize+1);
        if(pxmMatch.width()*0.9<QFontMetrics(fntText).boundingRect(mbMatch.text()).width())
            break;
        iMaxFontSize++;
    }

    QThread *thMatch=QThread::create(
        [lblText](int iSize) {
            while(true)
                if(lblText->isVisible()) {
                    // Animates the QMessageBox text by changing the font size up and ...
                    // ... down within the dialog area (the reason behind the complex ...
                    // ... call to QMetaObject::invokeMethod() is to avoid updating ...
                    // ... the UI from this thread and causing havoc as a result; ...
                    // ... and the reason for using setStyleSheet() for changing ...
                    // ... the font size instead of setFont(), is because the ...
                    // ... former is a slot. i.e., Q_INVOKABLE).
                    QString sFamily=lblText->font().family();
                    for(int iK=iSize/2;iK<=iSize;iK++) {
                        QString sStyle=QStringLiteral("font-family: \"%1\"; font-size: %2pt;").
                                       arg(sFamily).arg(iK);
                        QMetaObject::invokeMethod(
                            lblText,
                            QStringLiteral("setStyleSheet").toUtf8(),
                            Q_ARG(QString,sStyle)
                        );
                        QThread::msleep(10);
                    }
                    for(int iK=iSize;iK>=iSize/2;iK--) {
                        QString sStyle=QStringLiteral("font-family: \"%1\"; font-size: %2pt;").
                                       arg(sFamily).arg(iK);
                        QMetaObject::invokeMethod(
                            lblText,
                            QStringLiteral("setStyleSheet").toUtf8(),
                            Q_ARG(QString,sStyle)
                        );
                        QThread::msleep(10);
                    }
                }
        },
        iMaxFontSize
    );
    QObject::connect(
        &mbMatch,
        &QMessageBox::finished,
        [thMatch](int) {
            thMatch->terminate();
            thMatch->wait();
        }
    );
    thMatch->start();
    mbMatch.exec();
}
