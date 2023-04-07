#include "profileviewer.h"
#include "ui_profileviewer.h"

#define THUMBNAIL_SIZE 100

ProfileViewer::ProfileViewer(QWidget *wgtParent):
    QWidget(wgtParent),ui(new Ui::ProfileViewer) {
    bVideoPausedByUser=false;
    iCurrentPhotoIndex=-1;
    iCurrentVideoIndex=-1;
    abtProfilePhotos.clear();
    abtProfileVideos.clear();
    BadooAPI::clearUserProfile(bupProfileDetails);
    mvwPhoto=new MediaViewer;
    mvwVideo=new MediaViewer(MEDIA_TYPE_VIDEO);
    mctPhotoControls=new MediaControls(mvwPhoto);
    mctVideoControls=new MediaControls(mvwVideo,true);
    ui->setupUi(this);
    this->getFullFileContents(QStringLiteral(":img/photo-placeholder.png"),abtPlaceholderPhoto);
    this->getFullFileContents(QStringLiteral(":img/video-placeholder.mp4"),abtPlaceholderVideo);
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
}

ProfileViewer::~ProfileViewer() {
    delete mctPhotoControls;
    delete mctVideoControls;
    delete mvwPhoto;
    delete mvwVideo;
    delete ui;
}

void ProfileViewer::getPlaceholderPhoto(QByteArray &abtPlaceholder) {
    abtPlaceholder=abtPlaceholderPhoto;
}

void ProfileViewer::getPlaceholderVideo(QByteArray &abtPlaceholder) {
    abtPlaceholder=abtPlaceholderVideo;
}

void ProfileViewer::load(BadooUserProfile bupProfile,
                         QByteArrayList   abtlPhotos,
                         QByteArrayList   abtlVideos) {
    bupProfileDetails=bupProfile;
    abtProfilePhotos=abtlPhotos;
    abtProfileVideos=abtlVideos;
    this->resetProfileWidgets();
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

void ProfileViewer::delayedResizeTimeout() {
    tmrDelayedResize.stop();
    this->updateMediaWidgets();
}

void ProfileViewer::copyURLButtonClicked() {
    if(!bupProfileDetails.sUserId.isEmpty())
        emit buttonClicked(PROFILE_VIEWER_BUTTON_COPY_URL);
}

void ProfileViewer::downloadProfileButtonClicked() {
    if(!bupProfileDetails.sUserId.isEmpty())
        emit buttonClicked(PROFILE_VIEWER_BUTTON_DOWNLOAD);
}

void ProfileViewer::backButtonClicked() {
    if(!bupProfileDetails.sUserId.isEmpty())
        emit buttonClicked(PROFILE_VIEWER_BUTTON_BACK);
}

void ProfileViewer::nopeButtonClicked() {
    if(!bupProfileDetails.sUserId.isEmpty()) {
        this->showVote(false);
        emit buttonClicked(PROFILE_VIEWER_BUTTON_NOPE);
    }
}

void ProfileViewer::likeButtonClicked() {
    if(!bupProfileDetails.sUserId.isEmpty()) {
        this->showVote(true);
        emit buttonClicked(PROFILE_VIEWER_BUTTON_LIKE);
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
    if(iCurrentPhotoIndex<abtProfilePhotos.count()-1) {
        iCurrentPhotoIndex++;
        this->updateMediaButtons();
        this->updateMediaTitle();
        this->updatePhotoContent();
    }
}

void ProfileViewer::lastPhotoButtonClicked() {
    if(iCurrentPhotoIndex<abtProfilePhotos.count()-1) {
        iCurrentPhotoIndex=abtProfilePhotos.count()-1;
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
    if(iCurrentVideoIndex<abtProfileVideos.count()-1) {
        iCurrentVideoIndex++;
        this->updateMediaButtons();
        this->updateMediaTitle();
        this->updateVideoContent();
    }
}

void ProfileViewer::lastVideoButtonClicked() {
    if(iCurrentVideoIndex<abtProfileVideos.count()-1) {
        iCurrentVideoIndex=abtProfileVideos.count()-1;
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

void ProfileViewer::getFullFileContents(QString    sPath,
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

void ProfileViewer::resetProfileWidgets(int iGoToPhotoIndex,
                                        int iGoToVideoIndex) {
    if(abtProfilePhotos.count())
        iCurrentPhotoIndex=iGoToPhotoIndex;
    if(abtProfileVideos.count())
        iCurrentVideoIndex=iGoToVideoIndex;
    ui->tbwGalleries->setCurrentWidget(ui->wgtPhotoGallery);
    ui->stwMediaContent->setCurrentWidget(ui->wgtPhoto);
    this->updateProfileInfo();
    this->updateMediaButtons();
    this->updateMediaTitle();
    this->updateMediaWidgets();
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
    if(abtProfilePhotos.count())
        abtNormalImage=abtProfilePhotos.at(iCurrentPhotoIndex);
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

void ProfileViewer::updateMediaButtons() {
    mctPhotoControls->setButtonsEnabling(iCurrentPhotoIndex,abtProfilePhotos.count());
    mctVideoControls->setButtonsEnabling(iCurrentVideoIndex,abtProfileVideos.count());
}

void ProfileViewer::updateMediaTitle() {
    if(ui->tbwGalleries->currentWidget()==ui->wgtPhotoGallery)
        if(abtProfilePhotos.count())
            ui->lblMediaTitle->setText(
                QStringLiteral("Photo %1 of %2").
                arg(iCurrentPhotoIndex+1).
                arg(abtProfilePhotos.count())
            );
        else
            ui->lblMediaTitle->setText(QStringLiteral("No available photos"));
    else if(ui->tbwGalleries->currentWidget()==ui->wgtVideoGallery)
        if(abtProfileVideos.count())
            ui->lblMediaTitle->setText(
                QStringLiteral("Video %1 of %2").
                arg(iCurrentVideoIndex+1).
                arg(abtProfileVideos.count())
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
    if(abtProfilePhotos.count())
        abtContent=abtProfilePhotos.at(iCurrentPhotoIndex);
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
        QStringLiteral("Photos (%1)").arg(abtProfilePhotos.count())
    );
    iTCols=ui->grvPhotoGallery->width()/THUMBNAIL_SIZE;
    if(!iTCols)
        iTCols++;
    iTRows=abtProfilePhotos.count()/iTCols;
    if(abtProfilePhotos.count()%iTCols)
        iTRows++;
    int iWidth=iTCols*THUMBNAIL_SIZE;
    int iHeight=iTRows*THUMBNAIL_SIZE;
    grsPhotoGallery.clear();
    grsPhotoGallery.setSceneRect(0,0,iWidth,iHeight);
    for(const auto &p:abtProfilePhotos) {
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
    QLabel  *lblTitle=new QLabel(sTitle),
            *lblBadge;
    QSize   sizBadge;
    QPixmap pxmBadge;
    sizBadge.setHeight(lblTitle->fontMetrics().height());
    sizBadge.setWidth(sizBadge.height());
    ui->hblInfo->addWidget(lblTitle);
    if(bupProfileDetails.bIsVerified) {
        pxmBadge.load(QStringLiteral(":img/badge-verification.svg"));
        lblBadge=new QLabel;
        lblBadge->setMaximumSize(sizBadge);
        lblBadge->setPixmap(pxmBadge);
        lblBadge->setScaledContents(true);
        lblBadge->setToolTip(QStringLiteral("Verifed profile"));
        ui->hblInfo->addWidget(lblBadge);
    }
    if(bupProfileDetails.bIsMatch) {
        pxmBadge.load(QStringLiteral(":img/badge-match.svg"));
        lblBadge=new QLabel;
        lblBadge->setMaximumSize(sizBadge);
        lblBadge->setPixmap(pxmBadge);
        lblBadge->setScaledContents(true);
        lblBadge->setToolTip(QStringLiteral("It's a match!"));
        ui->hblInfo->addWidget(lblBadge);
    }
    if(bupProfileDetails.bIsFavorite) {
        pxmBadge.load(QStringLiteral(":img/badge-favorite.svg"));
        lblBadge=new QLabel;
        lblBadge->setMaximumSize(sizBadge);
        lblBadge->setPixmap(pxmBadge);
        lblBadge->setScaledContents(true);
        lblBadge->setToolTip(QStringLiteral("Your favorite"));
        ui->hblInfo->addWidget(lblBadge);
    }
    if(bupProfileDetails.bHasQuickChat) {
        pxmBadge.load(QStringLiteral(":img/badge-quick-chat.svg"));
        lblBadge=new QLabel;
        lblBadge->setMaximumSize(sizBadge);
        lblBadge->setPixmap(pxmBadge);
        lblBadge->setScaledContents(true);
        lblBadge->setToolTip(QStringLiteral("Quick-chat enabled!"));
        ui->hblInfo->addWidget(lblBadge);
    }
    if(BadooVote::VOTE_YES==bupProfileDetails.bvTheirVote) {
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
        if(abtProfileVideos.count())
            abtContent=abtProfileVideos.at(iCurrentVideoIndex);
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
        QStringLiteral("Videos (%1)").arg(abtProfileVideos.count())
    );
    iTCols=ui->grvVideoGallery->width()/THUMBNAIL_SIZE;
    if(!iTCols)
        iTCols++;
    iTRows=abtProfileVideos.count()/iTCols;
    if(abtProfileVideos.count()%iTCols)
        iTRows++;
    int iWidth=iTCols*THUMBNAIL_SIZE;
    int iHeight=iTRows*THUMBNAIL_SIZE;
    grsVideoGallery.clear();
    grsVideoGallery.setSceneRect(0,0,iWidth,iHeight);
    int iK=0;
    for(const auto &p:abtProfileVideos) {
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
