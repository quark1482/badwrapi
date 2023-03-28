#include "playencountersdialog.h"
#include "ui_playencountersdialog.h"

#define DIALOG_TITLE "Play Encounters"

#define THUMBNAIL_SIZE 100

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
                                           QWidget      *wgtParent):
    QDialog(wgtParent),ui(new Ui::PlayEncountersDialog) {
    bVideoPausedByUser=false;
    iCurrentProfileIndex=0;
    iCurrentPhotoIndex=0;
    iCurrentVideoIndex=0;
    mchPhotoContents.clear();
    mchVideoContents.clear();
    bwEncounters=bwParent;
    mvwPhoto=new MediaViewer;
    mvwVideo=new MediaViewer(MEDIA_TYPE_VIDEO);
    mctPhotoControls=new MediaControls(mvwPhoto);
    mctVideoControls=new MediaControls(mvwVideo,true);
    if(!this->getNewBatch(true)) {
        QTimer::singleShot(
            0,
            [=]() {
                this->reject();
            }
        );
        return;
    }
    ui->setupUi(this);
    this->getFullFileContents(QStringLiteral(":img/photo-placeholder.png"),abtPlaceholderPhoto);
    this->getFullFileContents(QStringLiteral(":img/video-placeholder.mp4"),abtPlaceholderVideo);
    this->loadMyProfile();
    this->toggleMediaViewersIndependence();
    ui->grvPhotoGallery->setAlignment(Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);
    ui->grvPhotoGallery->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    ui->grvPhotoGallery->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    ui->grvPhotoGallery->setSizeAdjustPolicy(QGraphicsView::SizeAdjustPolicy::AdjustIgnored);
    ui->grvPhotoGallery->setScene(&grsPhotoGallery);
    ui->grvVideoGallery->setAlignment(Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);
    ui->grvVideoGallery->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    ui->grvVideoGallery->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    ui->grvVideoGallery->setSizeAdjustPolicy(QGraphicsView::SizeAdjustPolicy::AdjustIgnored);
    ui->grvVideoGallery->setScene(&grsVideoGallery);
    grsPhotoGallery.installEventFilter(this);
    grsVideoGallery.installEventFilter(this);
    this->installEventFilter(this);
    this->setWindowTitle(QStringLiteral(DIALOG_TITLE));
    connect(
        &tmrDelayedResize,
        &QTimer::timeout,
        this,
        &PlayEncountersDialog::delayedResizeTimeout
    );

    connect(
        ui->btnCopyURL,
        &QPushButton::clicked,
        this,
        &PlayEncountersDialog::copyURLButtonClicked
    );
    connect(
        ui->btnDownloadProfile,
        &QPushButton::clicked,
        this,
        &PlayEncountersDialog::downloadProfileButtonClicked
    );

    connect(
        ui->btnBack,
        &QPushButton::clicked,
        this,
        &PlayEncountersDialog::backButtonClicked
    );
    connect(
        ui->btnNope,
        &QPushButton::clicked,
        this,
        &PlayEncountersDialog::nopeButtonClicked
    );
    connect(
        ui->btnLike,
        &QPushButton::clicked,
        this,
        &PlayEncountersDialog::likeButtonClicked
    );
    connect(
        ui->btnSkip,
        &QPushButton::clicked,
        this,
        &PlayEncountersDialog::skipButtonClicked
    );

    connect(
        mctPhotoControls,
        &MediaControls::first,
        this,
        &PlayEncountersDialog::firstPhotoButtonClicked
    );
    connect(
        mctPhotoControls,
        &MediaControls::previous,
        this,
        &PlayEncountersDialog::previousPhotoButtonClicked
    );
    connect(
        mctPhotoControls,
        &MediaControls::next,
        this,
        &PlayEncountersDialog::nextPhotoButtonClicked
    );
    connect(
        mctPhotoControls,
        &MediaControls::last,
        this,
        &PlayEncountersDialog::lastPhotoButtonClicked
    );

    connect(
        mctVideoControls,
        &MediaControls::first,
        this,
        &PlayEncountersDialog::firstVideoButtonClicked
    );
    connect(
        mctVideoControls,
        &MediaControls::previous,
        this,
        &PlayEncountersDialog::previousVideoButtonClicked
    );
    connect(
        mctVideoControls,
        &MediaControls::next,
        this,
        &PlayEncountersDialog::nextVideoButtonClicked
    );
    connect(
        mctVideoControls,
        &MediaControls::last,
        this,
        &PlayEncountersDialog::lastVideoButtonClicked
    );
    connect(
        mctVideoControls,
        &MediaControls::pause,
        this,
        &PlayEncountersDialog::pauseVideoButtonClicked
    );
    connect(
        mctVideoControls,
        &MediaControls::mute,
        this,
        &PlayEncountersDialog::muteVideoButtonClicked
    );

    connect(
        mvwPhoto,
        &MediaViewer::doubleClick,
        this,
        &PlayEncountersDialog::photoDoubleClicked
    );
    connect(
        mvwPhoto,
        &MediaViewer::keyPress,
        this,
        &PlayEncountersDialog::photoKeyPressed
    );
    connect(
        mvwPhoto,
        &MediaViewer::hover,
        this,
        &PlayEncountersDialog::photoMouseHover
    );

    connect(
        mvwVideo,
        &MediaViewer::doubleClick,
        this,
        &PlayEncountersDialog::videoDoubleClicked
    );
    connect(
        mvwVideo,
        &MediaViewer::keyPress,
        this,
        &PlayEncountersDialog::videoKeyPressed
    );
    connect(
        mvwVideo,
        &MediaViewer::hover,
        this,
        &PlayEncountersDialog::videoMouseHover
    );

    connect(
        ui->tbwGalleries,
        &QTabWidget::currentChanged,
        this,
        &PlayEncountersDialog::galleryTabWidgetChanged
    );
    QTimer::singleShot(
        0,
        [=]() {
            this->resetProfileWidgets();
        }
    );
}

PlayEncountersDialog::~PlayEncountersDialog() {
    delete mctPhotoControls;
    delete mctVideoControls;
    delete mvwPhoto;
    delete mvwVideo;
    delete ui;
}

bool PlayEncountersDialog::eventFilter(QObject *objO,QEvent *evnE) {
    if(QEvent::Type::GraphicsSceneMousePress==evnE->type()||
       QEvent::Type::GraphicsSceneMouseDoubleClick==evnE->type()) {
        QGraphicsSceneMouseEvent *mevEvent=static_cast<QGraphicsSceneMouseEvent *>(evnE);
        QGraphicsScene           *grsScene=qobject_cast<QGraphicsScene *>(objO);
        QGraphicsItem            *griPhoto=grsScene->itemAt(
            mevEvent->scenePos().toPoint(),
            QTransform()
        );
        if(nullptr!=griPhoto) {
            if(griPhoto->data(0).isValid()) {
                if(griPhoto->data(1).isValid()) {
                    int iItemIndex=griPhoto->data(0).toInt(),
                        iItemType=griPhoto->data(1).toInt();
                    if(0==iItemType) {
                        iCurrentPhotoIndex=iItemIndex;
                        this->updateMediaTitle();
                        if(QEvent::Type::GraphicsSceneMouseDoubleClick==evnE->type())
                            this->toggleMediaViewersIndependence();
                        this->updatePhotoContent();
                    }
                    else if(1==iItemType) {
                        iCurrentVideoIndex=iItemIndex;
                        this->updateMediaTitle();
                        if(QEvent::Type::GraphicsSceneMouseDoubleClick==evnE->type())
                            this->toggleMediaViewersIndependence();
                        this->updateVideoContent();
                    }
                }
            }
            return true;
        }
    }
    return QDialog::eventFilter(objO,evnE);
}

void PlayEncountersDialog::resizeEvent(QResizeEvent *) {
    if(this->isVisible())
        tmrDelayedResize.start(500);
}

void PlayEncountersDialog::delayedResizeTimeout() {
    tmrDelayedResize.stop();
    this->updateMediaWidgets();
}

void PlayEncountersDialog::copyURLButtonClicked() {
    emit statusChanged(QString());
    if(buplEncounters.count()) {
        QString sURL=QStringLiteral("%1/profile/0%2").
                     arg(ENDPOINT_BASE).
                     arg(this->getCurrentProfileId());
        QGuiApplication::clipboard()->setText(sURL);
        emit statusChanged(QStringLiteral("Profile URL copied to clipboard"));
    }
}

void PlayEncountersDialog::downloadProfileButtonClicked() {
    emit statusChanged(QString());
    if(buplEncounters.count()) {
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

void PlayEncountersDialog::backButtonClicked() {
    bool bUpdate=false;
    emit statusChanged(QString());
    if(iCurrentProfileIndex) {
        iCurrentProfileIndex--;
        bUpdate=true;
    }
    else
        emit statusChanged(QStringLiteral("Already at first profile"));
    if(bUpdate)
        this->resetProfileWidgets();
}

void PlayEncountersDialog::nopeButtonClicked() {
    emit statusChanged(QString());
    if(buplEncounters.count()) {
        bool bMatch;
        this->showVote(false);
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
        this->resetProfileWidgets();
    }
}

void PlayEncountersDialog::likeButtonClicked() {
    emit statusChanged(QString());
    if(buplEncounters.count()) {
        bool bMatch;
        this->showVote(true);
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
        this->resetProfileWidgets();
    }
}

void PlayEncountersDialog::skipButtonClicked() {
    bool bUpdate=false;
    emit statusChanged(QString());
    if(iCurrentProfileIndex<buplEncounters.count()-1) {
        iCurrentProfileIndex++;
        bUpdate=true;
    }
    else
        bUpdate=this->getNewBatch();
    if(bUpdate)
        this->resetProfileWidgets();
}

void PlayEncountersDialog::firstPhotoButtonClicked() {
    emit statusChanged(QString());
    if(iCurrentPhotoIndex>0) {
        iCurrentPhotoIndex=0;
        this->updateMediaTitle();
        this->updatePhotoContent();
    }
}

void PlayEncountersDialog::previousPhotoButtonClicked() {
    emit statusChanged(QString());
    if(iCurrentPhotoIndex>0) {
        iCurrentPhotoIndex--;
        this->updateMediaTitle();
        this->updatePhotoContent();
    }
    else
        emit statusChanged(QStringLiteral("Already at first photo"));
}

void PlayEncountersDialog::nextPhotoButtonClicked() {
    emit statusChanged(QString());
    if(iCurrentPhotoIndex<mchPhotoContents.value(this->getCurrentProfileId()).count()-1) {
        iCurrentPhotoIndex++;
        this->updateMediaTitle();
        this->updatePhotoContent();
    }
    else
        emit statusChanged(QStringLiteral("Already at last photo"));
}

void PlayEncountersDialog::lastPhotoButtonClicked() {
    emit statusChanged(QString());
    if(iCurrentPhotoIndex<mchPhotoContents.value(this->getCurrentProfileId()).count()-1) {
        iCurrentPhotoIndex=mchPhotoContents.value(this->getCurrentProfileId()).count()-1;
        this->updateMediaTitle();
        this->updatePhotoContent();
    }
}

void PlayEncountersDialog::firstVideoButtonClicked() {
    emit statusChanged(QString());
    if(iCurrentVideoIndex>0) {
        iCurrentVideoIndex=0;
        this->updateMediaTitle();
        this->updateVideoContent();
    }
}

void PlayEncountersDialog::previousVideoButtonClicked() {
    emit statusChanged(QString());
    if(iCurrentVideoIndex>0) {
        iCurrentVideoIndex--;
        this->updateMediaTitle();
        this->updateVideoContent();
    }
    else
        emit statusChanged(QStringLiteral("Already at first video"));
}

void PlayEncountersDialog::nextVideoButtonClicked() {
    emit statusChanged(QString());
    if(iCurrentVideoIndex<mchVideoContents.value(this->getCurrentProfileId()).count()-1) {
        iCurrentVideoIndex++;
        this->updateMediaTitle();
        this->updateVideoContent();
    }
    else
        emit statusChanged(QStringLiteral("Already at last video"));
}

void PlayEncountersDialog::lastVideoButtonClicked() {
    emit statusChanged(QString());
    if(iCurrentVideoIndex<mchVideoContents.value(this->getCurrentProfileId()).count()-1) {
        iCurrentVideoIndex=mchVideoContents.value(this->getCurrentProfileId()).count()-1;
        this->updateMediaTitle();
        this->updateVideoContent();
    }
}

void PlayEncountersDialog::pauseVideoButtonClicked(bool bPause) {
    bVideoPausedByUser=bPause;
    if(bVideoPausedByUser)
        mvwVideo->pauseVideo();
    else
        mvwVideo->playVideo();
}

void PlayEncountersDialog::muteVideoButtonClicked(bool bMute) {
    mvwVideo->muteVideo(bMute);
}

void PlayEncountersDialog::galleryTabWidgetChanged(int iIndex) {
    if(iIndex==ui->tbwGalleries->indexOf(ui->wgtPhotoGallery)) {
        mvwVideo->pauseVideo();
        ui->stwMediaContent->setCurrentWidget(ui->wgtPhoto);
        this->updateMediaTitle();
    }
    else if(iIndex==ui->tbwGalleries->indexOf(ui->wgtVideoGallery)) {
        if(!bVideoPausedByUser)
            mvwVideo->playVideo();
        ui->stwMediaContent->setCurrentWidget(ui->wgtVideo);
        this->updateMediaTitle();
    }
}

void PlayEncountersDialog::photoDoubleClicked() {
    this->toggleMediaViewersIndependence();
    this->updatePhotoContent();
}

void PlayEncountersDialog::photoKeyPressed(int iKey) {
    switch(iKey) {
        case Qt::Key::Key_Escape:
            this->toggleMediaViewersIndependence();
            this->updatePhotoContent();
            break;
        case Qt::Key::Key_Up:
            this->firstPhotoButtonClicked();
            break;
        case Qt::Key::Key_Down:
            this->lastPhotoButtonClicked();
            break;
        case Qt::Key::Key_Left:
            this->previousPhotoButtonClicked();
            break;
        case Qt::Key::Key_Right:
            this->nextPhotoButtonClicked();
            break;
    }
}

void PlayEncountersDialog::photoMouseHover(QPoint pntP) {
    if(mvwPhoto->isWindow())
        mctPhotoControls->setVisible(
            pntP.y()>=mctPhotoControls->y()&&
            pntP.y()<=mctPhotoControls->y()+mctPhotoControls->height()&&
            pntP.x()>=mctPhotoControls->x()&&
            pntP.x()<=mctPhotoControls->x()+mctPhotoControls->width()
        );
    else
        mctPhotoControls->setVisible(true);
}

void PlayEncountersDialog::videoDoubleClicked() {
    this->toggleMediaViewersIndependence();
    this->updateVideoContent();
}

void PlayEncountersDialog::videoKeyPressed(int iKey) {
    switch(iKey) {
        case Qt::Key::Key_Escape:
            this->toggleMediaViewersIndependence();
            this->updateVideoContent();
            break;
        case Qt::Key::Key_Space:
            this->pauseVideoButtonClicked(!bVideoPausedByUser);
            mctVideoControls->setPauseButtonState(bVideoPausedByUser);
            break;
        case Qt::Key::Key_Up:
            this->firstVideoButtonClicked();
            break;
        case Qt::Key::Key_Down:
            this->lastVideoButtonClicked();
            break;
        case Qt::Key::Key_Left:
            this->previousVideoButtonClicked();
            break;
        case Qt::Key::Key_Right:
            this->nextVideoButtonClicked();
            break;
    }
}

void PlayEncountersDialog::videoMouseHover(QPoint pntP) {
    if(mvwVideo->isWindow())
        mctVideoControls->setVisible(
            pntP.y()>=mctVideoControls->y()&&
            pntP.y()<=mctVideoControls->y()+mctVideoControls->height()&&
            pntP.x()>=mctVideoControls->x()&&
            pntP.x()<=mctVideoControls->x()+mctVideoControls->width()
        );
    else
        mctVideoControls->setVisible(true);
}

QString PlayEncountersDialog::getCurrentProfileId() {
    return buplEncounters.at(iCurrentProfileIndex).sUserId;
}

void PlayEncountersDialog::getFullFileContents(QString    sPath,
                                               QByteArray &abtContents) {
    QFile   fFile;
    QBuffer bufContents;
    abtContents.clear();
    fFile.setFileName(sPath);
    if(fFile.open(QFile::OpenModeFlag::ReadOnly)) {
        bufContents.setBuffer(&abtContents);
        bufContents.open(QBuffer::OpenModeFlag::WriteOnly);
        bufContents.write(fFile.readAll());
        bufContents.close();
        fFile.close();
    }
}

bool PlayEncountersDialog::getNewBatch(bool bReset) {
    bool              bResult=false;
    int               iBackupProfileIndex,
                      iBackupPhotoIndex,
                      iBackupVideoIndex;
    MediaContentsHash mchBackupPhotoContents,
                      mchBackupVideoContents;
    iBackupProfileIndex=iCurrentProfileIndex;
    iBackupPhotoIndex=iCurrentPhotoIndex;
    iBackupVideoIndex=iCurrentVideoIndex;
    mchBackupPhotoContents=mchPhotoContents;
    mchBackupVideoContents=mchVideoContents;
    iCurrentProfileIndex=0;
    iCurrentPhotoIndex=0;
    iCurrentVideoIndex=0;
    mchPhotoContents.clear();
    mchVideoContents.clear();
    this->setCursor(Qt::CursorShape::BusyCursor);
    while(true) {
        if(bwEncounters->getEncounters(buplEncounters,bReset)) {
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
                            mchPhotoContents[u.sUserId].append(abtlPhotoContents[iPhotoCounter++]);
                        for(const auto &v:u.slVideos)
                            mchVideoContents[u.sUserId].append(abtlVideoContents[iVideoCounter++]);
                    }
                    bResult=true;
                    break;
                }
        }
        if(QMessageBox::StandardButton::Cancel==QMessageBox::question(
            this,
            QStringLiteral("Error"),
            bwEncounters->getLastError(),
            QMessageBox::StandardButton::Retry|QMessageBox::StandardButton::Cancel
        )) {
            iCurrentProfileIndex=iBackupProfileIndex;
            iCurrentPhotoIndex=iBackupPhotoIndex;
            iCurrentVideoIndex=iBackupVideoIndex;
            mchPhotoContents=mchBackupPhotoContents;
            mchVideoContents=mchBackupVideoContents;
            break;
        }
    }
    this->unsetCursor();
    return bResult;
}

void PlayEncountersDialog::loadMyProfile() {
    if(bwEncounters->getLoggedInProfile(bupMyProfile)) {
        QStringList    slResources={bupMyProfile.sProfilePhotoURL};
        QByteArrayList abtlContents;
        if(bwEncounters->downloadMultiMediaResources(slResources,abtlContents))
            abtMyProfilePhoto=abtlContents.first();
        else
            abtMyProfilePhoto=abtPlaceholderPhoto;
    }
}

void PlayEncountersDialog::resetProfileWidgets(int iGoToPhotoIndex,
                                               int iGoToVideoIndex) {
    iCurrentPhotoIndex=iGoToPhotoIndex;
    iCurrentVideoIndex=iGoToVideoIndex;
    ui->tbwGalleries->setCurrentWidget(ui->wgtPhotoGallery);
    ui->stwMediaContent->setCurrentWidget(ui->wgtPhoto);
    this->updateProfileTitle();
    this->updateProfileInfo();
    this->updateMediaTitle();
    this->updateMediaWidgets();
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
                    // Animates a pulsating QMessageBox text by changing the font size ...
                    // up and down within the dialog area (the reason of the complex ...
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

void PlayEncountersDialog::showVote(bool bVote) {
    const QString asVotes[]={
        QStringLiteral("LIKE"),
        QStringLiteral("NOPE")
    };
    int           iVoteWidthSize,
                  iMaxFontSize;
    QString       sImageFormat;
    QByteArray    abtNormalImage,
                  abtVotedImage;
    QImageReader  imrImage;
    QBuffer       bufImage;
    QRect         recText;
    QPixmap       pxmVoted;
    QPainter      pntVoted;
    QFont         fntText;

    iVoteWidthSize=QFontMetrics(fntText).boundingRect(asVotes[0]).width()-
                   QFontMetrics(fntText).boundingRect(asVotes[1]).width();
    if(mchPhotoContents.value(this->getCurrentProfileId()).count())
        abtNormalImage=mchPhotoContents.value(this->getCurrentProfileId()).at(iCurrentPhotoIndex);
    else
        abtNormalImage=abtPlaceholderPhoto;
    bufImage.setBuffer(&abtNormalImage);
    imrImage.setDevice(&bufImage);
    bufImage.open(QBuffer::OpenModeFlag::ReadOnly);
    sImageFormat=imrImage.format();
    bufImage.close();
    pxmVoted=QPixmap::fromImage(
        QImage::fromData(abtNormalImage)
    );

    fntText.setFamily(QStringLiteral("Arial"));
    fntText.setBold(true);
    iMaxFontSize=0;
    while(true) {
        fntText.setPointSize(iMaxFontSize+1);
        recText=QFontMetrics(fntText,&pxmVoted).boundingRect(
            pxmVoted.rect(),
            0,
            iVoteWidthSize<0?asVotes[1]:asVotes[0]
        );
        if(pxmVoted.width()<recText.width())
            break;
        iMaxFontSize++;
    }
    recText.moveTo(pxmVoted.rect().center()-recText.center());
    fntText.setPointSize(iMaxFontSize);

    pntVoted.begin(&pxmVoted);
    pntVoted.setFont(fntText);
    pntVoted.setPen(QPen(bVote?QColor(Qt::GlobalColor::green):QColor(Qt::GlobalColor::red),5));
    pntVoted.translate(pxmVoted.rect().center());
    pntVoted.rotate(-45);
    pntVoted.translate(-pxmVoted.rect().center());
    pntVoted.drawRect(recText);
    pntVoted.drawText(recText,Qt::AlignmentFlag::AlignCenter,bVote?asVotes[0]:asVotes[1]);
    pntVoted.end();

    bufImage.setBuffer(&abtVotedImage);
    bufImage.open(QBuffer::OpenModeFlag::WriteOnly);
    pxmVoted.save(&bufImage,sImageFormat.toUtf8());
    bufImage.close();

    ui->tbwGalleries->setCurrentWidget(ui->wgtPhotoGallery);
    mvwPhoto->showPhoto(abtVotedImage);
}

void PlayEncountersDialog::toggleMediaViewersIndependence() {
    if(mvwPhoto->isWindow()) {
        ui->wgtPhoto->layout()->addWidget(mvwPhoto);
        mctPhotoControls->setButtonSizeRatio(0.25);
        mctPhotoControls->setVisible(true);
    }
    else {
        ui->wgtPhoto->layout()->removeWidget(mvwPhoto);
        mvwPhoto->setParent(nullptr);
        mctPhotoControls->setButtonSizeRatio(0.5);
        mctPhotoControls->setVisible(false);
    }
    if(mvwVideo->isWindow()) {
        ui->wgtVideo->layout()->addWidget(mvwVideo);
        mctVideoControls->setButtonSizeRatio(0.25);
        mctVideoControls->setVisible(true);
    }
    else {
        ui->wgtVideo->layout()->removeWidget(mvwVideo);
        mvwVideo->setParent(nullptr);
        mctVideoControls->setButtonSizeRatio(0.5);
        mctVideoControls->setVisible(false);
    }
}

void PlayEncountersDialog::updateMediaTitle() {
    if(ui->tbwGalleries->currentWidget()==ui->wgtPhotoGallery)
        if(mchPhotoContents.value(this->getCurrentProfileId()).count())
            ui->lblMediaTitle->setText(
                QStringLiteral("Photo %1 of %2").
                arg(iCurrentPhotoIndex+1).
                arg(mchPhotoContents.value(this->getCurrentProfileId()).count())
            );
        else
            ui->lblMediaTitle->setText(QStringLiteral("No available photos"));
    else if(ui->tbwGalleries->currentWidget()==ui->wgtVideoGallery)
        if(mchVideoContents.value(this->getCurrentProfileId()).count())
            ui->lblMediaTitle->setText(
                QStringLiteral("Video %1 of %2").
                arg(iCurrentVideoIndex+1).
                arg(mchVideoContents.value(this->getCurrentProfileId()).count())
            );
        else
            ui->lblMediaTitle->setText(QStringLiteral("No available videos"));
}

void PlayEncountersDialog::updateMediaWidgets() {
    // There seems to be a problem with QGraphicsView when it's covered ...
    // ... behind another widget. Automatic resizing inherited from its ...
    // ... container is ignored. So here we force the resizing from the ...
    // ... visible counterpart.
    if(ui->wgtPhotoGallery==ui->tbwGalleries->currentWidget()) {
        mvwVideo->resize(mvwPhoto->size());
        ui->grvVideoGallery->resize(ui->grvPhotoGallery->size());
    }
    else if(ui->wgtVideoGallery==ui->tbwGalleries->currentWidget()) {
        mvwPhoto->resize(mvwVideo->size());
        ui->grvPhotoGallery->resize(ui->grvVideoGallery->size());
    }
    this->updatePhotoContent();
    this->updateVideoContent();
    this->updatePhotoGallery();
    this->updateVideoGallery();
}

void PlayEncountersDialog::updatePhotoContent() {
    QByteArray abtContent;
    if(mchPhotoContents.value(this->getCurrentProfileId()).count())
        abtContent=mchPhotoContents.value(this->getCurrentProfileId()).at(iCurrentPhotoIndex);
    else
        abtContent=abtPlaceholderPhoto;
    mvwPhoto->showPhoto(abtContent);
    mctPhotoControls->resetVisualStatus();
}

void PlayEncountersDialog::updatePhotoGallery() {
    int iRow=0,
        iCol=0,
        iTRows,
        iTCols;
    ui->tbwGalleries->setTabText(
        ui->tbwGalleries->indexOf(ui->wgtPhotoGallery),
        QStringLiteral("Photos (%1)").arg(mchPhotoContents.value(this->getCurrentProfileId()).count())
    );
    iTCols=ui->grvPhotoGallery->width()/THUMBNAIL_SIZE;
    if(!iTCols)
        iTCols++;
    iTRows=mchPhotoContents.value(this->getCurrentProfileId()).count()/iTCols;
    if(mchPhotoContents.value(this->getCurrentProfileId()).count()%iTCols)
        iTRows++;
    int iWidth=iTCols*THUMBNAIL_SIZE;
    int iHeight=iTRows*THUMBNAIL_SIZE;
    grsPhotoGallery.clear();
    grsPhotoGallery.setSceneRect(0,0,iWidth,iHeight);
    for(const auto &p:mchPhotoContents.value(this->getCurrentProfileId())) {
        QPixmap             pxmPhoto;
        QGraphicsPixmapItem *grpiPhoto=grsPhotoGallery.addPixmap(QPixmap());
        pxmPhoto.loadFromData(p);
        if(!pxmPhoto.isNull()) {
            grpiPhoto->setCursor(Qt::CursorShape::PointingHandCursor);
            grpiPhoto->setPixmap(
                pxmPhoto.scaled(
                    THUMBNAIL_SIZE,
                    THUMBNAIL_SIZE,
                    Qt::AspectRatioMode::KeepAspectRatio,
                    Qt::TransformationMode::SmoothTransformation
                )
            );
            grpiPhoto->setPos(
                iCol*THUMBNAIL_SIZE+THUMBNAIL_SIZE/2.0-grpiPhoto->pixmap().width()/2.0,
                iRow*THUMBNAIL_SIZE+THUMBNAIL_SIZE/2.0-grpiPhoto->pixmap().height()/2.0
            );
        }
        grpiPhoto->setData(0,iRow*iTCols+iCol);
        grpiPhoto->setData(1,0);
        if(++iCol==iTCols) {
            iCol=0;
            iRow++;
        }
    }
    ui->grvPhotoGallery->ensureVisible(QRectF());
}

void PlayEncountersDialog::updateProfileInfo() {
    int     iGender=buplEncounters.at(iCurrentProfileIndex).iGender;
    QString sTitle=buplEncounters.at(iCurrentProfileIndex).sName,
            sLocation=buplEncounters.at(iCurrentProfileIndex).sCity;
    if(BadooSexType::SEX_TYPE_MALE==iGender||BadooSexType::SEX_TYPE_FEMALE==iGender) {
        sTitle.append(QStringLiteral(", "));
        if(BadooSexType::SEX_TYPE_MALE==iGender)
            sTitle.append(QStringLiteral("Male"));
        else
            sTitle.append(QStringLiteral("Female"));
    }
    sTitle.append(QStringLiteral(", %1").arg(buplEncounters.at(iCurrentProfileIndex).iAge));
    if(!buplEncounters.at(iCurrentProfileIndex).sRegion.isEmpty()) {
        if(!sLocation.isEmpty())
            sLocation.append(QStringLiteral(", "));
        sLocation.append(buplEncounters.at(iCurrentProfileIndex).sRegion);
    }
    if(!buplEncounters.at(iCurrentProfileIndex).sCountry.isEmpty()) {
        if(!sLocation.isEmpty())
            sLocation.append(QStringLiteral(", "));
        sLocation.append(buplEncounters.at(iCurrentProfileIndex).sCountry);
    }
    while(auto i=ui->hblInfo->takeAt(0)) {
        if(i->widget())
            delete i->widget();
        delete i;
    }
    QLabel  *lblTitle=new QLabel(sTitle),
            *lblBadge;
    QSize   sizBadge;
    QPixmap pxmBadge;
    sizBadge.setHeight(lblTitle->fontMetrics().height());
    sizBadge.setWidth(sizBadge.height());
    ui->hblInfo->addWidget(lblTitle);
    if(buplEncounters.at(iCurrentProfileIndex).bIsVerified) {
        pxmBadge.load(QStringLiteral(":img/badge-verification.svg"));
        lblBadge=new QLabel;
        lblBadge->setMaximumSize(sizBadge);
        lblBadge->setPixmap(pxmBadge);
        lblBadge->setScaledContents(true);
        lblBadge->setToolTip(QStringLiteral("Verifed profile"));
        ui->hblInfo->addWidget(lblBadge);
    }
    if(buplEncounters.at(iCurrentProfileIndex).bHasQuickChat) {
        pxmBadge.load(QStringLiteral(":img/badge-quick-chat.svg"));
        lblBadge=new QLabel;
        lblBadge->setMaximumSize(sizBadge);
        lblBadge->setPixmap(pxmBadge);
        lblBadge->setScaledContents(true);
        lblBadge->setToolTip(QStringLiteral("Quick-chat enabled!"));
        ui->hblInfo->addWidget(lblBadge);
    }
    if(BadooVote::VOTE_YES==buplEncounters.at(iCurrentProfileIndex).bvTheirVote) {
        pxmBadge.load(QStringLiteral(":img/badge-liked-you.svg"));
        lblBadge=new QLabel;
        lblBadge->setMaximumSize(sizBadge);
        lblBadge->setPixmap(pxmBadge);
        lblBadge->setScaledContents(true);
        lblBadge->setToolTip(QStringLiteral("They already liked you!"));
        ui->hblInfo->addWidget(lblBadge);
    }
    ui->hblInfo->addStretch();
    while(ui->fmlInfo->rowCount())
        ui->fmlInfo->removeRow(0);
    QTextEdit *txtAbout=new QTextEdit(buplEncounters.at(iCurrentProfileIndex).sAbout);
    txtAbout->setReadOnly(true);
    txtAbout->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    txtAbout->viewport()->setAutoFillBackground(false);
    ui->fmlInfo->addRow(
        QStringLiteral("About me:"),
        txtAbout
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Relationship:"),
        new QLabel(buplEncounters.at(iCurrentProfileIndex).sRelationshipStatus)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Sexuality:"),
        new QLabel(buplEncounters.at(iCurrentProfileIndex).sSexuality)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Appearance:"),
        new QLabel(buplEncounters.at(iCurrentProfileIndex).sAppearance)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("I'm here to:"),
        new QLabel(buplEncounters.at(iCurrentProfileIndex).sIntent)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Mood:"),
        new QLabel(buplEncounters.at(iCurrentProfileIndex).sMood)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Kids:"),
        new QLabel(buplEncounters.at(iCurrentProfileIndex).sChildren)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Smoking:"),
        new QLabel(buplEncounters.at(iCurrentProfileIndex).sSmoking)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Drinking:"),
        new QLabel(buplEncounters.at(iCurrentProfileIndex).sDrinking)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Location:"),
        new QLabel(sLocation)
    );
    ui->scaInfo->ensureVisible(0,0);
}

void PlayEncountersDialog::updateProfileTitle() {
    if(buplEncounters.count())
        ui->lblProfileTitle->setText(
            QStringLiteral("Profile %1 of %2").
            arg(iCurrentProfileIndex+1).arg(buplEncounters.count())
        );
    else
        ui->lblProfileTitle->setText(QStringLiteral("No available profiles"));
}

void PlayEncountersDialog::updateVideoContent() {
    static int     iPreviousVideoIndex=-1;
    static QString sPreviousProfileId;
    // Avoids unnecessarily reloading (and restarting) the video.
    if(sPreviousProfileId!=this->getCurrentProfileId()||iPreviousVideoIndex!=iCurrentVideoIndex) {
        QByteArray abtContent;
        if(mchVideoContents.value(this->getCurrentProfileId()).count())
            abtContent=mchVideoContents.value(this->getCurrentProfileId()).at(iCurrentVideoIndex);
        else
            abtContent=abtPlaceholderVideo;
        mvwVideo->loadVideo(abtContent);
        mvwVideo->playVideo();
        sPreviousProfileId=this->getCurrentProfileId();
        iPreviousVideoIndex=iCurrentVideoIndex;
    }
    else
        mvwVideo->showVideo();
    if(ui->wgtPhotoGallery==ui->tbwGalleries->currentWidget())
        mvwVideo->pauseVideo();
    else if(ui->wgtVideoGallery==ui->tbwGalleries->currentWidget())
        if(bVideoPausedByUser)
            mvwVideo->pauseVideo();
    mctVideoControls->resetVisualStatus();
}

void PlayEncountersDialog::updateVideoGallery() {
    int iRow=0,
        iCol=0,
        iTRows,
        iTCols;
    ui->tbwGalleries->setTabText(
        ui->tbwGalleries->indexOf(ui->wgtVideoGallery),
        QStringLiteral("Videos (%1)").arg(mchVideoContents.value(this->getCurrentProfileId()).count())
    );
    iTCols=ui->grvVideoGallery->width()/THUMBNAIL_SIZE;
    if(!iTCols)
        iTCols++;
    iTRows=mchVideoContents.value(this->getCurrentProfileId()).count()/iTCols;
    if(mchVideoContents.value(this->getCurrentProfileId()).count()%iTCols)
        iTRows++;
    int iWidth=iTCols*THUMBNAIL_SIZE;
    int iHeight=iTRows*THUMBNAIL_SIZE;
    grsVideoGallery.clear();
    grsVideoGallery.setSceneRect(0,0,iWidth,iHeight);
    int iK=0;
    for(const auto &p:mchVideoContents.value(this->getCurrentProfileId())) {
        QPixmap             pxmVideo;
        QGraphicsPixmapItem *grpiVideo=grsVideoGallery.addPixmap(QPixmap());
        MediaViewer::getFrame(p,pxmVideo);
        if(!pxmVideo.isNull()) {
            grpiVideo->setCursor(Qt::CursorShape::PointingHandCursor);
            grpiVideo->setPixmap(
                pxmVideo.scaled(
                    THUMBNAIL_SIZE,
                    THUMBNAIL_SIZE,
                    Qt::AspectRatioMode::KeepAspectRatio,
                    Qt::TransformationMode::SmoothTransformation
                )
            );
            grpiVideo->setPos(
                iCol*THUMBNAIL_SIZE+THUMBNAIL_SIZE/2.0-grpiVideo->pixmap().width()/2.0,
                iRow*THUMBNAIL_SIZE+THUMBNAIL_SIZE/2.0-grpiVideo->pixmap().height()/2.0
            );
        }
        grpiVideo->setData(0,iRow*iTCols+iCol);
        grpiVideo->setData(1,1);
        if(++iCol==iTCols) {
            iCol=0;
            iRow++;
        }
    }
    ui->grvVideoGallery->ensureVisible(QRectF());
}
