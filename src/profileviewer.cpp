#include "profileviewer.h"
#include "ui_profileviewer.h"

#define THUMBNAIL_SIDE 100

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

ProfileViewer::ProfileViewer(BadooWrapper *bwParent,
                             QWidget *wgtParent):
QWidget(wgtParent),ui(new Ui::ProfileViewer) {
    bVideoPausedByUser=false;
    iActiveActionButtons=-1;
    iCurrentPhotoIndex=-1;
    iCurrentVideoIndex=-1;
    BadooAPI::getFullFileContents(QStringLiteral(":img/photo-placeholder.png"),abtPlaceholderPhoto);
    BadooAPI::getFullFileContents(QStringLiteral(":img/video-placeholder.mp4"),abtPlaceholderVideo);
    if(!bwParent->getLoggedInProfilePhoto(abtOwnProfilePhoto))
        abtOwnProfilePhoto=abtPlaceholderPhoto;
    abtlProfilePhotos.clear();
    abtlProfileVideos.clear();
    bwProfile=bwParent;
    mvwPhoto=new MediaViewer;
    mvwVideo=new MediaViewer(MEDIA_TYPE_VIDEO);
    mctPhotoControls=new MediaControls(mvwPhoto);
    mctVideoControls=new MediaControls(mvwVideo,true);
    BadooAPI::clearUserProfile(bupProfileDetails);
    ui->setupUi(this);
    this->toggleMediaViewersIndependence();
    ui->grvPhotoGallery->setAlignment(Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);
    ui->grvPhotoGallery->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    ui->grvPhotoGallery->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    ui->grvPhotoGallery->setFrameShape(QFrame::Shape::NoFrame);
    ui->grvPhotoGallery->setSizeAdjustPolicy(QGraphicsView::SizeAdjustPolicy::AdjustIgnored);
    ui->grvPhotoGallery->setScene(&grsPhotoGallery);
    ui->grvVideoGallery->setAlignment(Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);
    ui->grvVideoGallery->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    ui->grvVideoGallery->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    ui->grvVideoGallery->setFrameShape(QFrame::Shape::NoFrame);
    ui->grvVideoGallery->setSizeAdjustPolicy(QGraphicsView::SizeAdjustPolicy::AdjustIgnored);
    ui->grvVideoGallery->setScene(&grsVideoGallery);
    grsPhotoGallery.installEventFilter(this);
    grsVideoGallery.installEventFilter(this);
    do {
        connect(
            &tmrDelayedResize,
            &QTimer::timeout,
            this,
            &ProfileViewer::delayedResizeTimeout
        );

        connect(
            ui->btnCopyURL,
            &QPushButton::clicked,
            this,
            &ProfileViewer::copyURLButtonClicked
        );
        connect(
            ui->btnDownloadProfile,
            &QPushButton::clicked,
            this,
            &ProfileViewer::downloadProfileButtonClicked
        );

        connect(
            ui->btnBack,
            &QPushButton::clicked,
            this,
            &ProfileViewer::backButtonClicked
        );
        connect(
            ui->btnNope,
            &QPushButton::clicked,
            this,
            &ProfileViewer::nopeButtonClicked
        );
        connect(
            ui->btnFavorite,
            &QPushButton::clicked,
            this,
            &ProfileViewer::favoriteButtonClicked
        );
        connect(
            ui->btnLike,
            &QPushButton::clicked,
            this,
            &ProfileViewer::likeButtonClicked
        );
        connect(
            ui->btnSkip,
            &QPushButton::clicked,
            this,
            &ProfileViewer::skipButtonClicked
        );

        connect(
            mctPhotoControls,
            &MediaControls::first,
            this,
            &ProfileViewer::firstPhotoButtonClicked
        );
        connect(
            mctPhotoControls,
            &MediaControls::previous,
            this,
            &ProfileViewer::previousPhotoButtonClicked
        );
        connect(
            mctPhotoControls,
            &MediaControls::next,
            this,
            &ProfileViewer::nextPhotoButtonClicked
        );
        connect(
            mctPhotoControls,
            &MediaControls::last,
            this,
            &ProfileViewer::lastPhotoButtonClicked
        );

        connect(
            mctVideoControls,
            &MediaControls::first,
            this,
            &ProfileViewer::firstVideoButtonClicked
        );
        connect(
            mctVideoControls,
            &MediaControls::previous,
            this,
            &ProfileViewer::previousVideoButtonClicked
        );
        connect(
            mctVideoControls,
            &MediaControls::next,
            this,
            &ProfileViewer::nextVideoButtonClicked
        );
        connect(
            mctVideoControls,
            &MediaControls::last,
            this,
            &ProfileViewer::lastVideoButtonClicked
        );
        connect(
            mctVideoControls,
            &MediaControls::pause,
            this,
            &ProfileViewer::pauseVideoButtonClicked
        );
        connect(
            mctVideoControls,
            &MediaControls::mute,
            this,
            &ProfileViewer::muteVideoButtonClicked
        );

        connect(
            mvwPhoto,
            &MediaViewer::doubleClick,
            this,
            &ProfileViewer::photoDoubleClicked
        );
        connect(
            mvwPhoto,
            &MediaViewer::keyPress,
            this,
            &ProfileViewer::photoKeyPressed
        );
        connect(
            mvwPhoto,
            &MediaViewer::hover,
            this,
            &ProfileViewer::photoMouseHover
        );

        connect(
            mvwVideo,
            &MediaViewer::doubleClick,
            this,
            &ProfileViewer::videoDoubleClicked
        );
        connect(
            mvwVideo,
            &MediaViewer::keyPress,
            this,
            &ProfileViewer::videoKeyPressed
        );
        connect(
            mvwVideo,
            &MediaViewer::hover,
            this,
            &ProfileViewer::videoMouseHover
        );

        connect(
            ui->tbwGalleries,
            &QTabWidget::currentChanged,
            this,
            &ProfileViewer::galleryTabWidgetChanged
        );
    } while(false);
}

ProfileViewer::~ProfileViewer() {
    delete mctPhotoControls;
    delete mctVideoControls;
    delete mvwPhoto;
    delete mvwVideo;
    delete ui;
}

bool ProfileViewer::eventFilter(QObject *objO,
                                QEvent  *evnE) {
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
                        this->updateMediaButtons();
                        this->updateMediaTitle();
                        if(QEvent::Type::GraphicsSceneMouseDoubleClick==evnE->type())
                            this->toggleMediaViewersIndependence();
                        this->updatePhotoContent();
                    }
                    else if(1==iItemType) {
                        iCurrentVideoIndex=iItemIndex;
                        this->updateMediaButtons();
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
    return QWidget::eventFilter(objO,evnE);
}

void ProfileViewer::resizeEvent(QResizeEvent *) {
    tmrDelayedResize.start(500);
}

void ProfileViewer::load(BadooUserProfile bupProfile,
                         QByteArrayList   abtlPhotos,
                         QByteArrayList   abtlVideos) {
    bupProfileDetails=bupProfile;
    abtlProfilePhotos=abtlPhotos;
    abtlProfileVideos=abtlVideos;
    this->resetProfileWidgets();
}

void ProfileViewer::setActiveActionButtons(int iActive) {
    iActiveActionButtons=iActive;
}

void ProfileViewer::delayedResizeTimeout() {
    tmrDelayedResize.stop();
    this->updateMediaWidgets();
}

void ProfileViewer::copyURLButtonClicked() {
    if(!bupProfileDetails.sUserId.isEmpty()) {
        QString sURL=QStringLiteral("%1/profile/%2").
                     arg(ENDPOINT_BASE).
                     arg(bupProfileDetails.sUserId);
        QGuiApplication::clipboard()->setText(sURL);
        emit buttonClicked(PROFILE_VIEWER_BUTTON_COPY_URL);
    }
}

void ProfileViewer::downloadProfileButtonClicked() {
    if(!bupProfileDetails.sUserId.isEmpty()) {
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
            sHTML=BadooWrapper::getHTMLFromProfile(
                bupProfileDetails,
                true,
                QStringLiteral(HTML_STYLE_PROFILE),
                abtlProfilePhotos,
                abtlProfileVideos
            );
            if(QFileInfo(sTempPath).suffix().isEmpty())
                sTempPath.append(QStringLiteral(EXTENSION_HTML));
            fFile.setFileName(sTempPath);
            if(fFile.open(QFile::OpenModeFlag::WriteOnly))
                if(-1!=fFile.write(sHTML.toUtf8()))
                    emit buttonClicked(PROFILE_VIEWER_BUTTON_DOWNLOAD);
            if(QFile::FileError::NoError!=fFile.error())
                QMessageBox::critical(
                    this,
                    QStringLiteral("Error"),
                    fFile.errorString()
                );
        }
    }
}

void ProfileViewer::backButtonClicked() {
    if(!bupProfileDetails.sUserId.isEmpty())
        emit buttonClicked(PROFILE_VIEWER_BUTTON_BACK);
}

void ProfileViewer::nopeButtonClicked() {
    if(!bupProfileDetails.sUserId.isEmpty()) {
        bool bMatch;
        this->showVote(false);
        if(bwProfile->vote(bupProfileDetails.sUserId,false,bMatch))
            emit buttonClicked(PROFILE_VIEWER_BUTTON_NOPE);
        else
            QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                bwProfile->getLastError()
            );
    }
}

void ProfileViewer::favoriteButtonClicked(bool bFavorite) {
    if(!bupProfileDetails.sUserId.isEmpty()) {
        bool bOK;
        if(bFavorite)
            bOK=bwProfile->addToFavorites(bupProfileDetails.sUserId);
        else
            bOK=bwProfile->removeFromFavorites(bupProfileDetails.sUserId);
        if(bOK)
            emit buttonClicked(PROFILE_VIEWER_BUTTON_FAVORITE);
        else {
            ui->btnFavorite->setChecked(!bFavorite);
            QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                bwProfile->getLastError()
            );
        }
    }
}

void ProfileViewer::likeButtonClicked() {
    if(!bupProfileDetails.sUserId.isEmpty()) {
        bool bMatch;
        this->showVote(true);
        if(bwProfile->vote(bupProfileDetails.sUserId,true,bMatch)) {
            if(bMatch)
                this->showMatch(
                    bupProfileDetails.sName,
                    abtlProfilePhotos.first(),
                    abtOwnProfilePhoto
                );
            emit buttonClicked(PROFILE_VIEWER_BUTTON_LIKE);
        }
        else
            QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                bwProfile->getLastError()
            );
    }
}

void ProfileViewer::skipButtonClicked() {
    if(!bupProfileDetails.sUserId.isEmpty())
        emit buttonClicked(PROFILE_VIEWER_BUTTON_SKIP);
}

void ProfileViewer::firstPhotoButtonClicked() {
    if(iCurrentPhotoIndex>0) {
        iCurrentPhotoIndex=0;
        this->updateMediaButtons();
        this->updateMediaTitle();
        this->updatePhotoContent();
    }
}

void ProfileViewer::previousPhotoButtonClicked() {
    if(iCurrentPhotoIndex>0) {
        iCurrentPhotoIndex--;
        this->updateMediaButtons();
        this->updateMediaTitle();
        this->updatePhotoContent();
    }
}

void ProfileViewer::nextPhotoButtonClicked() {
    if(iCurrentPhotoIndex<abtlProfilePhotos.count()-1) {
        iCurrentPhotoIndex++;
        this->updateMediaButtons();
        this->updateMediaTitle();
        this->updatePhotoContent();
    }
}

void ProfileViewer::lastPhotoButtonClicked() {
    if(iCurrentPhotoIndex<abtlProfilePhotos.count()-1) {
        iCurrentPhotoIndex=abtlProfilePhotos.count()-1;
        this->updateMediaButtons();
        this->updateMediaTitle();
        this->updatePhotoContent();
    }
}

void ProfileViewer::firstVideoButtonClicked() {
    if(iCurrentVideoIndex>0) {
        iCurrentVideoIndex=0;
        this->updateMediaButtons();
        this->updateMediaTitle();
        this->updateVideoContent();
    }
}

void ProfileViewer::previousVideoButtonClicked() {
    if(iCurrentVideoIndex>0) {
        iCurrentVideoIndex--;
        this->updateMediaButtons();
        this->updateMediaTitle();
        this->updateVideoContent();
    }
}

void ProfileViewer::nextVideoButtonClicked() {
    if(iCurrentVideoIndex<abtlProfileVideos.count()-1) {
        iCurrentVideoIndex++;
        this->updateMediaButtons();
        this->updateMediaTitle();
        this->updateVideoContent();
    }
}

void ProfileViewer::lastVideoButtonClicked() {
    if(iCurrentVideoIndex<abtlProfileVideos.count()-1) {
        iCurrentVideoIndex=abtlProfileVideos.count()-1;
        this->updateMediaButtons();
        this->updateMediaTitle();
        this->updateVideoContent();
    }
}

void ProfileViewer::pauseVideoButtonClicked(bool bPause) {
    bVideoPausedByUser=bPause;
    if(bVideoPausedByUser)
        mvwVideo->pauseVideo();
    else
        mvwVideo->playVideo();
}

void ProfileViewer::muteVideoButtonClicked(bool bMute) {
    mvwVideo->muteVideo(bMute);
}

void ProfileViewer::galleryTabWidgetChanged(int iIndex) {
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

void ProfileViewer::photoDoubleClicked() {
    this->toggleMediaViewersIndependence();
    this->updatePhotoContent();
}

void ProfileViewer::photoKeyPressed(int iKey) {
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

void ProfileViewer::photoMouseHover(QPoint pntP) {
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

void ProfileViewer::videoDoubleClicked() {
    this->toggleMediaViewersIndependence();
    this->updateVideoContent();
}

void ProfileViewer::videoKeyPressed(int iKey) {
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

void ProfileViewer::videoMouseHover(QPoint pntP) {
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

void ProfileViewer::resetProfileWidgets(int iGoToPhotoIndex,
                                        int iGoToVideoIndex) {
    if(abtlProfilePhotos.count())
        iCurrentPhotoIndex=iGoToPhotoIndex;
    if(abtlProfileVideos.count())
        iCurrentVideoIndex=iGoToVideoIndex;
    ui->tbwGalleries->setCurrentWidget(ui->wgtPhotoGallery);
    ui->stwMediaContent->setCurrentWidget(ui->wgtPhoto);
    this->updateProfileInfo();
    this->updateMediaButtons();
    this->updateMediaTitle();
    this->updateMediaWidgets();
    this->updateActionButtons();
}

void ProfileViewer::showMatch(QString    sName,
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
    mbMatch.setWindowTitle(mbMatch.text());
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

void ProfileViewer::showVote(bool bVote) {
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
    if(abtlProfilePhotos.count())
        abtNormalImage=abtlProfilePhotos.at(iCurrentPhotoIndex);
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

void ProfileViewer::toggleMediaViewersIndependence() {
    if(mvwPhoto->isWindow()) {
        ui->wgtPhoto->layout()->addWidget(mvwPhoto);
        mctPhotoControls->setButtonSizeRatio(0.25);
        mctPhotoControls->setVisible(true);
        this->activateWindow();
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
        this->activateWindow();
    }
    else {
        ui->wgtVideo->layout()->removeWidget(mvwVideo);
        mvwVideo->setParent(nullptr);
        mctVideoControls->setButtonSizeRatio(0.5);
        mctVideoControls->setVisible(false);
    }
}

void ProfileViewer::updateActionButtons() {
    bool bCopyURLEnabled= iActiveActionButtons&PROFILE_VIEWER_BUTTON_COPY_URL,
         bDownloadEnabled=iActiveActionButtons&PROFILE_VIEWER_BUTTON_DOWNLOAD,
         bBackEnabled=    iActiveActionButtons&PROFILE_VIEWER_BUTTON_BACK,
         bNopeEnabled=    iActiveActionButtons&PROFILE_VIEWER_BUTTON_NOPE,
         bFavoriteEnabled=iActiveActionButtons&PROFILE_VIEWER_BUTTON_FAVORITE,
         bLikeEnabled=    iActiveActionButtons&PROFILE_VIEWER_BUTTON_LIKE,
         bSkipEnabled=    iActiveActionButtons&PROFILE_VIEWER_BUTTON_SKIP,
         bVoteEnabled=    VOTE_NONE==bupProfileDetails.bvMyVote;
    ui->btnCopyURL->setEnabled(bCopyURLEnabled);
    ui->btnDownloadProfile->setEnabled(bDownloadEnabled);
    ui->btnBack->setEnabled(bBackEnabled);
    ui->btnNope->setEnabled(bVoteEnabled&bNopeEnabled);
    ui->btnFavorite->setEnabled(bFavoriteEnabled);
    ui->btnLike->setEnabled(bVoteEnabled&bLikeEnabled);
    ui->btnSkip->setEnabled(bSkipEnabled);
    for(const auto &b:this->findChildren<QPushButton *>())
        if(!b->isEnabled())
            b->setToolTip(QStringLiteral("Disabled in this mode"));
    if(!bVoteEnabled) {
        QString sToolTip=QStringLiteral("You already voted %1 to this profile").
                         arg(BadooWrapper::getTextFromVote(bupProfileDetails.bvMyVote));
        ui->btnNope->setToolTip(sToolTip);
        ui->btnLike->setToolTip(sToolTip);
    }
    ui->btnFavorite->setChecked(bupProfileDetails.bIsFavorite);
    if(bFavoriteEnabled)
        if(bupProfileDetails.bIsFavorite)
            ui->btnFavorite->setToolTip(QStringLiteral("Remove from Favorites"));
        else
            ui->btnFavorite->setToolTip(QStringLiteral("Add to Favorites"));
}

void ProfileViewer::updateMediaButtons() {
    mctPhotoControls->setButtonsEnabling(iCurrentPhotoIndex,abtlProfilePhotos.count());
    mctVideoControls->setButtonsEnabling(iCurrentVideoIndex,abtlProfileVideos.count());
}

void ProfileViewer::updateMediaTitle() {
    if(ui->tbwGalleries->currentWidget()==ui->wgtPhotoGallery)
        if(abtlProfilePhotos.count())
            ui->lblMediaTitle->setText(
                QStringLiteral("Photo %1 of %2").
                arg(iCurrentPhotoIndex+1).
                arg(abtlProfilePhotos.count())
            );
        else
            ui->lblMediaTitle->setText(QStringLiteral("No available photos"));
    else if(ui->tbwGalleries->currentWidget()==ui->wgtVideoGallery)
        if(abtlProfileVideos.count())
            ui->lblMediaTitle->setText(
                QStringLiteral("Video %1 of %2").
                arg(iCurrentVideoIndex+1).
                arg(abtlProfileVideos.count())
            );
        else
            ui->lblMediaTitle->setText(QStringLiteral("No available videos"));
}

void ProfileViewer::updateMediaWidgets() {
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

void ProfileViewer::updatePhotoContent() {
    QByteArray abtContent;
    if(abtlProfilePhotos.count())
        abtContent=abtlProfilePhotos.at(iCurrentPhotoIndex);
    else
        abtContent=abtPlaceholderPhoto;
    mvwPhoto->showPhoto(abtContent);
    mctPhotoControls->resetVisualStatus();
}

void ProfileViewer::updatePhotoGallery() {
    int iRow=0,
        iCol=0,
        iTRows,
        iTCols;
    ui->tbwGalleries->setTabText(
        ui->tbwGalleries->indexOf(ui->wgtPhotoGallery),
        QStringLiteral("Photos (%1)").arg(abtlProfilePhotos.count())
    );
    iTCols=ui->grvPhotoGallery->width()/THUMBNAIL_SIDE;
    if(!iTCols)
        iTCols++;
    iTRows=abtlProfilePhotos.count()/iTCols;
    if(abtlProfilePhotos.count()%iTCols)
        iTRows++;
    int iWidth=iTCols*THUMBNAIL_SIDE;
    int iHeight=iTRows*THUMBNAIL_SIDE;
    grsPhotoGallery.clear();
    grsPhotoGallery.setSceneRect(0,0,iWidth,iHeight);
    for(const auto &p:abtlProfilePhotos) {
        QPixmap             pxmPhoto;
        QGraphicsPixmapItem *grpiPhoto=grsPhotoGallery.addPixmap(QPixmap());
        pxmPhoto.loadFromData(p);
        if(!pxmPhoto.isNull()) {
            grpiPhoto->setCursor(Qt::CursorShape::PointingHandCursor);
            grpiPhoto->setPixmap(
                pxmPhoto.scaled(
                    THUMBNAIL_SIDE,
                    THUMBNAIL_SIDE,
                    Qt::AspectRatioMode::KeepAspectRatio,
                    Qt::TransformationMode::SmoothTransformation
                )
            );
            grpiPhoto->setPos(
                iCol*THUMBNAIL_SIDE+THUMBNAIL_SIDE/2.0-grpiPhoto->pixmap().width()/2.0,
                iRow*THUMBNAIL_SIDE+THUMBNAIL_SIDE/2.0-grpiPhoto->pixmap().height()/2.0
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

void ProfileViewer::updateProfileInfo() {
    int     iGender=bupProfileDetails.iGender;
    QString sTitle=bupProfileDetails.sName,
            sLocation=bupProfileDetails.sCity;
    if(BadooSexType::SEX_TYPE_MALE==iGender||BadooSexType::SEX_TYPE_FEMALE==iGender) {
        sTitle.append(QStringLiteral(", "));
        if(BadooSexType::SEX_TYPE_MALE==iGender)
            sTitle.append(QStringLiteral("Male"));
        else
            sTitle.append(QStringLiteral("Female"));
    }
    if(bupProfileDetails.iAge)
        sTitle.append(QStringLiteral(", %1").arg(bupProfileDetails.iAge));
    if(!bupProfileDetails.sRegion.isEmpty()) {
        if(!sLocation.isEmpty())
            sLocation.append(QStringLiteral(", "));
        sLocation.append(bupProfileDetails.sRegion);
    }
    if(!bupProfileDetails.sCountry.isEmpty()) {
        if(!sLocation.isEmpty())
            sLocation.append(QStringLiteral(", "));
        sLocation.append(bupProfileDetails.sCountry);
    }
    while(auto i=ui->hblInfo->takeAt(0)) {
        if(i->widget())
            delete i->widget();
        delete i;
    }
    QLabel *lblTitle=new QLabel(QStringLiteral("<b>%1</b>").arg(sTitle));
    QSize  sizBadge;
    sizBadge.setHeight(lblTitle->fontMetrics().height());
    sizBadge.setWidth(sizBadge.height());
    ui->hblInfo->addWidget(lblTitle);
    std::function<void(QString,QString)> fnAddBadge=[=](QString sImg,QString sMsg) {
        QPixmap pxmBadge;
        QLabel  *lblBadge;
        pxmBadge.load(sImg);
        lblBadge=new QLabel;
        lblBadge->setMaximumSize(sizBadge);
        lblBadge->setPixmap(pxmBadge);
        lblBadge->setScaledContents(true);
        lblBadge->setToolTip(sMsg);
        ui->hblInfo->addWidget(lblBadge);
    };
    if(-1==bupProfileDetails.iLastOnline) {
        fnAddBadge(
            QStringLiteral(":img/badge-offline-hidden.svg"),
            QStringLiteral("Hidden online status")
        );
    }
    else if(!bupProfileDetails.iLastOnline) {
        fnAddBadge(
            QStringLiteral(":img/badge-online.svg"),
            bupProfileDetails.sOnlineStatus
        );
    }
    else if(MAX_PROFILE_IDLE_TIME>=bupProfileDetails.iLastOnline) {
        fnAddBadge(
            QStringLiteral(":img/badge-online-idle.svg"),
            bupProfileDetails.sOnlineStatus
        );
    }
    else if(MAX_PROFILE_REAL_TIME<=bupProfileDetails.iLastOnline) {
        fnAddBadge(
            QStringLiteral(":img/badge-offline-unknown.svg"),
            bupProfileDetails.sOnlineStatus
        );
    }
    else {
        fnAddBadge(
            QStringLiteral(":img/badge-offline.svg"),
            bupProfileDetails.sOnlineStatus
        );
    }
    if(bupProfileDetails.bIsVerified) {
        fnAddBadge(
            QStringLiteral(":img/badge-verification.svg"),
            QStringLiteral("Verifed profile")
        );
    }
    if(bupProfileDetails.bIsMatch) {
        fnAddBadge(
            QStringLiteral(":img/badge-match.svg"),
            QStringLiteral("It's a match!")
        );
    }
    else if(bupProfileDetails.bIsCrush) {
        fnAddBadge(
            QStringLiteral(":img/badge-crush.svg"),
            QStringLiteral("They have a crush on you!")
        );
    }
    else if(BadooVote::VOTE_YES==bupProfileDetails.bvTheirVote) {
        fnAddBadge(
            QStringLiteral(":img/badge-liked-you.svg"),
            QStringLiteral("They already liked you!")
        );
    }
    else if(BadooVote::VOTE_NO==bupProfileDetails.bvTheirVote) {
        if(BadooVote::VOTE_NO==bupProfileDetails.bvMyVote)
            fnAddBadge(
                QStringLiteral(":img/badge-disliked-mutual.svg"),
                QStringLiteral("The hate is mutual")
            );
        else
            fnAddBadge(
                QStringLiteral(":img/badge-disliked-you.svg"),
                QStringLiteral("They voted against you")
            );
    }
    if(bupProfileDetails.bIsFavorite) {
        fnAddBadge(
            QStringLiteral(":img/badge-favorite.svg"),
            QStringLiteral("Your favorite")
        );
    }
    if(bupProfileDetails.bHasQuickChat) {
        fnAddBadge(
            QStringLiteral(":img/badge-quick-chat.svg"),
            QStringLiteral("Quick-chat enabled!")
        );
    }
    ui->hblInfo->addStretch();
    while(ui->fmlInfo->rowCount())
        ui->fmlInfo->removeRow(0);
    QTextEdit *txtAbout=new QTextEdit(bupProfileDetails.sAbout);
    txtAbout->setReadOnly(true);
    txtAbout->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    txtAbout->viewport()->setAutoFillBackground(false);
    ui->fmlInfo->addRow(
        QStringLiteral("About me:"),
        txtAbout
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Relationship:"),
        new QLabel(bupProfileDetails.sRelationshipStatus)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Sexuality:"),
        new QLabel(bupProfileDetails.sSexuality)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Appearance:"),
        new QLabel(bupProfileDetails.sAppearance)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("I'm here to:"),
        new QLabel(bupProfileDetails.sIntent)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Mood:"),
        new QLabel(bupProfileDetails.sMood)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Kids:"),
        new QLabel(bupProfileDetails.sChildren)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Smoking:"),
        new QLabel(bupProfileDetails.sSmoking)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Drinking:"),
        new QLabel(bupProfileDetails.sDrinking)
    );
    ui->fmlInfo->addRow(
        QStringLiteral("Location:"),
        new QLabel(sLocation)
    );
    ui->scaInfo->ensureVisible(0,0);
}

void ProfileViewer::updateVideoContent() {
    static int     iPreviousVideoIndex=-1;
    static QString sPreviousProfileId;
    // Avoids unnecessarily reloading (and restarting) the video.
    if(sPreviousProfileId!=bupProfileDetails.sUserId||iPreviousVideoIndex!=iCurrentVideoIndex) {
        QByteArray abtContent;
        if(abtlProfileVideos.count())
            abtContent=abtlProfileVideos.at(iCurrentVideoIndex);
        else
            abtContent=abtPlaceholderVideo;
        mvwVideo->loadVideo(abtContent);
        mvwVideo->playVideo();
        sPreviousProfileId=bupProfileDetails.sUserId;
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

void ProfileViewer::updateVideoGallery() {
    int iRow=0,
        iCol=0,
        iTRows,
        iTCols;
    ui->tbwGalleries->setTabText(
        ui->tbwGalleries->indexOf(ui->wgtVideoGallery),
        QStringLiteral("Videos (%1)").arg(abtlProfileVideos.count())
    );
    iTCols=ui->grvVideoGallery->width()/THUMBNAIL_SIDE;
    if(!iTCols)
        iTCols++;
    iTRows=abtlProfileVideos.count()/iTCols;
    if(abtlProfileVideos.count()%iTCols)
        iTRows++;
    int iWidth=iTCols*THUMBNAIL_SIDE;
    int iHeight=iTRows*THUMBNAIL_SIDE;
    grsVideoGallery.clear();
    grsVideoGallery.setSceneRect(0,0,iWidth,iHeight);
    int iK=0;
    for(const auto &p:abtlProfileVideos) {
        QPixmap             pxmVideo;
        QGraphicsPixmapItem *grpiVideo=grsVideoGallery.addPixmap(QPixmap());
        MediaViewer::getFrame(p,pxmVideo);
        if(!pxmVideo.isNull()) {
            grpiVideo->setCursor(Qt::CursorShape::PointingHandCursor);
            grpiVideo->setPixmap(
                pxmVideo.scaled(
                    THUMBNAIL_SIDE,
                    THUMBNAIL_SIDE,
                    Qt::AspectRatioMode::KeepAspectRatio,
                    Qt::TransformationMode::SmoothTransformation
                )
            );
            grpiVideo->setPos(
                iCol*THUMBNAIL_SIDE+THUMBNAIL_SIDE/2.0-grpiVideo->pixmap().width()/2.0,
                iRow*THUMBNAIL_SIDE+THUMBNAIL_SIDE/2.0-grpiVideo->pixmap().height()/2.0
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
