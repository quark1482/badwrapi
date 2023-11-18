#include "folderviewer.h"

#define THUMBNAIL_SIDE 200 // Side size of the square the profile photo goes in.

#define MARGIN_WIDTH    25 // Border size the profile photo is surrounded with.

#define SEPARATOR_WIDTH 10 // Empty space the profiles are separated with.

FolderViewer::FolderViewer(BadooWrapper *bwParent,
                           QWidget      *wgtParent):
QWidget(wgtParent) {
    dbFolder=nullptr;
    bwFolder=bwParent;
    buplPageDetails.clear();
    mchPagePhotos.clear();
    mchPageVideos.clear();
    ftType=FOLDER_TYPE_UNKNOWN;
    BadooAPI::getFullFileContents(QStringLiteral(":img/photo-placeholder.png"),abtPlaceholderPhoto);
    this->configurePageButton(
        &btnFirst,
        QStringLiteral(":img/action-first.png"),
        QStringLiteral("First page")
    );
    this->configurePageButton(
        &btnPrevious,
        QStringLiteral(":img/action-previous.png"),
        QStringLiteral("Previous page")
    );
    this->configurePageButton(
        &btnPage,
        QStringLiteral(":img/action-page.png"),
        QStringLiteral("Go to page")
    );
    this->configurePageButton(
        &btnNext,
        QStringLiteral(":img/action-next.png"),
        QStringLiteral("Next page")
    );
    this->configurePageButton(
        &btnLast,
        QStringLiteral(":img/action-last.png"),
        QStringLiteral("Last page")
    );
    hblLayout.addWidget(&lblPageTitle);
    hblLayout.addStretch();
    hblLayout.addWidget(&btnFirst);
    hblLayout.addWidget(&btnPrevious);
    hblLayout.addWidget(&btnPage);
    hblLayout.addWidget(&btnNext);
    hblLayout.addWidget(&btnLast);
    grvView.setAlignment(Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);
    grvView.setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    grvView.setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    grvView.setFrameShape(QFrame::Shape::NoFrame);
    grvView.setSizeAdjustPolicy(QGraphicsView::SizeAdjustPolicy::AdjustIgnored);
    grvView.setScene(&grsScene);
    grsScene.installEventFilter(this);
    vblLayout.setContentsMargins(0,0,0,0);
    vblLayout.addWidget(&grvView);
    vblLayout.addLayout(&hblLayout);
    this->setLayout(&vblLayout);
    do {
        connect(
            &btnFirst,
            &QPushButton::clicked,
            this,
            &FolderViewer::firstButtonClicked
        );
        connect(
            &btnPrevious,
            &QPushButton::clicked,
            this,
            &FolderViewer::previousButtonClicked
        );
        connect(
            &btnPage,
            &QPushButton::clicked,
            this,
            &FolderViewer::pageButtonClicked
        );
        connect(
            &btnNext,
            &QPushButton::clicked,
            this,
            &FolderViewer::nextButtonClicked
        );
        connect(
            &btnLast,
            &QPushButton::clicked,
            this,
            &FolderViewer::lastButtonClicked
        );
    } while(false);
}

bool FolderViewer::eventFilter(QObject *objO,
                               QEvent  *evnE) {
    if(QEvent::Type::GraphicsSceneMousePress==evnE->type()) {
        QGraphicsSceneMouseEvent *mevEvent=static_cast<QGraphicsSceneMouseEvent *>(evnE);
        QGraphicsScene           *grsScene=qobject_cast<QGraphicsScene *>(objO);
        QGraphicsItem            *griPhoto=grsScene->itemAt(
            mevEvent->scenePos().toPoint(),
            QTransform()
        );
        if(nullptr!=griPhoto) {
            if(griPhoto->data(0).isValid())
                this->showStandaloneProfile(griPhoto->data(0).toInt());
            return true;
        }
    }
    return QWidget::eventFilter(objO,evnE);
}

void FolderViewer::resizeEvent(QResizeEvent *) {
    this->updatePageWidgets();
}

void FolderViewer::load(BadooUserProfileList buplPage,
                        MediaContentsHash    mchPhotos,
                        MediaContentsHash    mchVideos) {
    buplPageDetails=buplPage;
    mchPagePhotos=mchPhotos;
    mchPageVideos=mchVideos;
    this->updatePageWidgets(true);
}

void FolderViewer::setDB(DB *dbNew) {
    dbFolder=dbNew;
}

void FolderViewer::setFolderType(FolderType ftNew) {
    ftType=ftNew;
}

void FolderViewer::setPageTitle(QString sTitle) {
    lblPageTitle.setText(sTitle);
}

void FolderViewer::firstButtonClicked() {
    if(buplPageDetails.count())
        emit buttonClicked(FOLDER_VIEWER_BUTTON_FIRST);
}

void FolderViewer::previousButtonClicked() {
    if(buplPageDetails.count())
        emit buttonClicked(FOLDER_VIEWER_BUTTON_PREVIOUS);
}

void FolderViewer::pageButtonClicked() {
    if(buplPageDetails.count())
        emit buttonClicked(FOLDER_VIEWER_BUTTON_PAGE);
}

void FolderViewer::nextButtonClicked() {
    if(buplPageDetails.count())
        emit buttonClicked(FOLDER_VIEWER_BUTTON_NEXT);
}

void FolderViewer::lastButtonClicked() {
    if(buplPageDetails.count())
        emit buttonClicked(FOLDER_VIEWER_BUTTON_LAST);
}

void FolderViewer::configurePageButton(QPushButton *btnButton,
                                       QString     sImagePath,
                                       QString     sToolTip) {
    QIcon   icoNormal;
    QPixmap pxmNormal;
    pxmNormal.load(sImagePath);
    icoNormal.addPixmap(pxmNormal,QIcon::Mode::Normal,QIcon::State::On);
    btnButton->setCursor(Qt::CursorShape::PointingHandCursor);
    btnButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    btnButton->setIcon(icoNormal);
    btnButton->setSizePolicy(QSizePolicy::Policy::Fixed,QSizePolicy::Policy::Fixed);
    btnButton->setStyleSheet(QStringLiteral("background-color: transparent"));
    btnButton->setToolTip(sToolTip);
}

void FolderViewer::showStandaloneProfile(int iIndex) {
    QDialog       *dlgProfile=new QDialog(this);
    QStatusBar    *stbProfile=new QStatusBar(this);
    QVBoxLayout   *vblProfile=new QVBoxLayout(dlgProfile);
    ProfileViewer *pvProfile=new ProfileViewer(bwFolder,dlgProfile);
    vblProfile->setContentsMargins(QMargins());
    vblProfile->addWidget(pvProfile);
    vblProfile->addWidget(stbProfile);
    dlgProfile->setGeometry(pvProfile->geometry());
    dlgProfile->setLayout(vblProfile);
    dlgProfile->setWindowFlag(Qt::WindowType::WindowMinMaxButtonsHint);
    dlgProfile->setWindowModality(Qt::WindowModality::ApplicationModal);
    dlgProfile->setWindowTitle(QStringLiteral("View profile"));
    if(FOLDER_TYPE_LIKES==ftType)
        pvProfile->setActiveActionButtons(~PROFILE_VIEWER_BUTTON_FAVORITE);
    pvProfile->load(
        buplPageDetails.at(iIndex),
        mchPagePhotos.value(buplPageDetails.at(iIndex).sUserId),
        mchPageVideos.value(buplPageDetails.at(iIndex).sUserId)
    );
    connect(
        pvProfile,
        &ProfileViewer::buttonClicked,
        [&](ProfileViewerButton pvbButton) {
            bool    bUpdateProfile=false,
                    bUpdatePage=false;
            QString sMessage=QString();
            switch(pvbButton) {
                case PROFILE_VIEWER_BUTTON_COPY_URL:
                    if(-1<iIndex)
                        sMessage=QStringLiteral("Profile URL copied to clipboard");
                    break;
                case PROFILE_VIEWER_BUTTON_DOWNLOAD:
                    if(-1<iIndex)
                        sMessage=QStringLiteral("Profile saved to disk");
                    break;
                case PROFILE_VIEWER_BUTTON_BACK:
                    if(-1<iIndex)
                        if(iIndex) {
                            iIndex--;
                            bUpdateProfile=true;
                        }
                        else
                            sMessage=QStringLiteral("Already at this page's first profile");
                    break;
                case PROFILE_VIEWER_BUTTON_NOPE:
                    // ToDo: (optional) remove profile from the page, if required.
                    buplPageDetails[iIndex].bvMyVote=VOTE_NO;
                    bUpdateProfile=true;
                    bUpdatePage=true;
                    break;
                case PROFILE_VIEWER_BUTTON_FAVORITE:
                    buplPageDetails[iIndex].bIsFavorite=!buplPageDetails[iIndex].bIsFavorite;
                    bUpdateProfile=true;
                    bUpdatePage=true;
                    break;
                case PROFILE_VIEWER_BUTTON_LIKE:
                    // ToDo: (optional) remove profile from the page, if required.
                    buplPageDetails[iIndex].bvMyVote=VOTE_YES;
                    bUpdateProfile=true;
                    bUpdatePage=true;
                    if(VOTE_YES==buplPageDetails.at(iIndex).bvTheirVote)
                        buplPageDetails[iIndex].bIsMatch=true;
                    break;
                case PROFILE_VIEWER_BUTTON_SKIP:
                    if(-1<iIndex)
                        if(iIndex<buplPageDetails.count()-1) {
                            iIndex++;
                            bUpdateProfile=true;
                        }
                        else
                            sMessage=QStringLiteral("Already at this page's last profile");
                    break;
            }
            if(sMessage.isEmpty())
                stbProfile->clearMessage();
            else
                stbProfile->showMessage(sMessage);
            if(bUpdateProfile) {
                pvProfile->load(
                    buplPageDetails.at(iIndex),
                    mchPagePhotos.value(buplPageDetails.at(iIndex).sUserId),
                    mchPageVideos.value(buplPageDetails.at(iIndex).sUserId)
                );
                if(bUpdatePage)
                    this->updatePageWidgets();
            }
        }
    );
    WidgetGeometry wggGeometry;
    if(nullptr!=dbFolder)
        if(dbFolder->loadSetting(stgPROFILE,wggGeometry))
            if(!wggGeometry.recRect.isNull()) {
                dlgProfile->setGeometry(wggGeometry.recRect);
                switch(wggGeometry.wgsStatus) {
                    case wgsNORMAL:
                        dlgProfile->showNormal();
                        break;
                    case wgsMINIMIZED:
                        dlgProfile->showMinimized();
                        break;
                    case wgsMAXIMIZED:
                        dlgProfile->showMaximized();
                        break;
                }
            }
    dlgProfile->exec();
    if(nullptr!=dbFolder) {
        if(dlgProfile->isMaximized())
            wggGeometry.wgsStatus=wgsMAXIMIZED;
        else if(dlgProfile->isMinimized())
            wggGeometry.wgsStatus=wgsMINIMIZED;
        else
            wggGeometry.wgsStatus=wgsNORMAL;
        wggGeometry.recRect=dlgProfile->normalGeometry();
        dbFolder->saveSetting(stgPROFILE,wggGeometry);
    }
    delete pvProfile;
    delete vblProfile;
    delete stbProfile;
    delete dlgProfile;
}

void FolderViewer::updatePageWidgets(bool bReset) {
    int   iRow=0,
          iCol=0,
          iTRows,
          iTCols,
          iSideSize=THUMBNAIL_SIDE+MARGIN_WIDTH*2,
          vScrollPos=bReset?0:grvView.verticalScrollBar()->value();
    iTCols=(grvView.width()-SEPARATOR_WIDTH)/(iSideSize+SEPARATOR_WIDTH);
    if(!iTCols)
        iTCols++;
    iTRows=buplPageDetails.count()/iTCols;
    if(buplPageDetails.count()%iTCols)
        iTRows++;
    int iWidth=iTCols*(iSideSize+SEPARATOR_WIDTH)+SEPARATOR_WIDTH;
    int iHeight=iTRows*(iSideSize+SEPARATOR_WIDTH)+SEPARATOR_WIDTH;
    grsScene.clear();
    grsScene.setBackgroundBrush(QBrush(QColor(0x6e,0x3e,0xff)));
    grsScene.setSceneRect(0,0,iWidth,iHeight);
    for(const auto &p:buplPageDetails) {
        QPainterPath      pptProfile;
        QGraphicsPathItem *grptiProfile=grsScene.addPath(QPainterPath());
        QRect             recProfile(
            SEPARATOR_WIDTH+iCol*(iSideSize+SEPARATOR_WIDTH),
            SEPARATOR_WIDTH+iRow*(iSideSize+SEPARATOR_WIDTH),
            iSideSize,
            iSideSize
        );
        pptProfile.addRoundedRect(QRectF(recProfile),SEPARATOR_WIDTH,SEPARATOR_WIDTH);
        grptiProfile->setPath(pptProfile);
        grptiProfile->setPen(QPen(QColor(Qt::GlobalColor::white)));
        grptiProfile->setBrush(QBrush(QColor(Qt::GlobalColor::white)));
        grptiProfile->setZValue(0);
        QRect recTitle(
            SEPARATOR_WIDTH+iCol*(iSideSize+SEPARATOR_WIDTH),
            SEPARATOR_WIDTH+iRow*(iSideSize+SEPARATOR_WIDTH),
            iSideSize,
            MARGIN_WIDTH
        );
        this->updateProfileTitle(
            recTitle,
            p.sName,
            p.iAge,
            p.sCity
        );
        QByteArray abtPhoto;
        QRect      recPhoto(
            SEPARATOR_WIDTH+iCol*(iSideSize+SEPARATOR_WIDTH)+MARGIN_WIDTH,
            SEPARATOR_WIDTH+iRow*(iSideSize+SEPARATOR_WIDTH)+MARGIN_WIDTH,
            THUMBNAIL_SIDE,
            THUMBNAIL_SIDE
        );
        if(mchPagePhotos.value(p.sUserId).isEmpty())
            abtPhoto=abtPlaceholderPhoto;
        else
            abtPhoto=mchPagePhotos.value(p.sUserId).first();
        this->updateProfilePhoto(
            recPhoto,
            abtPhoto,
            iRow*iTCols+iCol
        );
        QRect recMediaInfo(
            SEPARATOR_WIDTH+iCol*(iSideSize+SEPARATOR_WIDTH),
            (iRow+1)*(iSideSize+SEPARATOR_WIDTH)-MARGIN_WIDTH,
            iSideSize/2,
            MARGIN_WIDTH
        );
        this->updateProfileMediaCounters(
            recMediaInfo,
            p.slPhotos.count(),
            p.slVideos.count()
        );
        QRect recBadges(
            SEPARATOR_WIDTH+iCol*(iSideSize+SEPARATOR_WIDTH)+iSideSize/2,
            (iRow+1)*(iSideSize+SEPARATOR_WIDTH)-MARGIN_WIDTH,
            iSideSize/2,
            MARGIN_WIDTH
        );
        this->updateProfileBadges(
            recBadges,
            recPhoto,
            p.bIsVerified,
            p.bIsMatch,
            p.bvMyVote,
            p.bvTheirVote,
            p.bIsFavorite,
            p.bIsCrush,
            p.bHasQuickChat,
            p.iLastOnline,
            p.sOnlineStatus
        );
        if(++iCol==iTCols) {
            iCol=0;
            iRow++;
        }
    }
    grvView.verticalScrollBar()->setValue(vScrollPos);
}

void FolderViewer::updateProfileBadges(QRect     recBadges,
                                       QRect     recPhoto,
                                       bool      bVerified,
                                       bool      bMatch,
                                       BadooVote bvMyVote,
                                       BadooVote bvTheirVote,
                                       bool      bFavorite,
                                       bool      bCrush,
                                       bool      bQuickChat,
                                       int       iLastOnline,
                                       QString   sOnlineStatus) {
    int            iXOffset;
    QPixmap        pxmBadge;
    QList<QPixmap> pxmlBadges;
    QStringList    slBadgeToolTips;
    if(bQuickChat) {
        pxmBadge.load(QStringLiteral(":img/badge-quick-chat.svg"));
        pxmlBadges.append(pxmBadge);
        slBadgeToolTips.append(QStringLiteral("Quick-chat enabled!"));
    }
    if(bFavorite) {
        pxmBadge.load(QStringLiteral(":img/badge-favorite.svg"));
        pxmlBadges.append(pxmBadge);
        slBadgeToolTips.append(QStringLiteral("Your favorite"));
    }
    if(bMatch) {
        pxmBadge.load(QStringLiteral(":img/badge-match.svg"));
        pxmlBadges.append(pxmBadge);
        slBadgeToolTips.append(QStringLiteral("It's a match!"));
    }
    else if(bCrush) {
        pxmBadge.load(QStringLiteral(":img/badge-crush.svg"));
        pxmlBadges.append(pxmBadge);
        slBadgeToolTips.append(QStringLiteral("They have a crush on you!"));
    }
    else if(BadooVote::VOTE_YES==bvTheirVote) {
        pxmBadge.load(QStringLiteral(":img/badge-liked-you.svg"));
        pxmlBadges.append(pxmBadge);
        slBadgeToolTips.append(QStringLiteral("They already liked you!"));
    }
    else if(BadooVote::VOTE_NO==bvTheirVote) {
        if(BadooVote::VOTE_NO==bvMyVote) {
            pxmBadge.load(QStringLiteral(":img/badge-disliked-mutual.svg"));
            pxmlBadges.append(pxmBadge);
            slBadgeToolTips.append(QStringLiteral("The hate is mutual"));
        }
        else {
            pxmBadge.load(QStringLiteral(":img/badge-disliked-you.svg"));
            pxmlBadges.append(pxmBadge);
            slBadgeToolTips.append(QStringLiteral("They voted against you"));
        }
    }
    if(bVerified) {
        pxmBadge.load(QStringLiteral(":img/badge-verification.svg"));
        pxmlBadges.append(pxmBadge);
        slBadgeToolTips.append(QStringLiteral("Verifed profile"));
    }
    if(-1==iLastOnline) {
        pxmBadge.load(QStringLiteral(":img/badge-offline-hidden.svg"));
        pxmlBadges.append(pxmBadge);
        slBadgeToolTips.append(QStringLiteral("Hidden online status"));
    } else if(!iLastOnline) {
        pxmBadge.load(QStringLiteral(":img/badge-online.svg"));
        pxmlBadges.append(pxmBadge);
        slBadgeToolTips.append(sOnlineStatus);
    } else if(MAX_PROFILE_IDLE_TIME>=iLastOnline) {
        pxmBadge.load(QStringLiteral(":img/badge-online-idle.svg"));
        pxmlBadges.append(pxmBadge);
        slBadgeToolTips.append(sOnlineStatus);
    } else if(MAX_PROFILE_REAL_TIME<=iLastOnline) {
        pxmBadge.load(QStringLiteral(":img/badge-offline-unknown.svg"));
        pxmlBadges.append(pxmBadge);
        slBadgeToolTips.append(sOnlineStatus);
    } else {
        pxmBadge.load(QStringLiteral(":img/badge-offline.svg"));
        pxmlBadges.append(pxmBadge);
        slBadgeToolTips.append(sOnlineStatus);
    }
    QGraphicsPixmapItem *grpiBadge;
    if(BadooVote::VOTE_YES==bvMyVote||BadooVote::VOTE_NO==bvMyVote) {
        QString sToolTip=QStringLiteral("You already voted %1 to this profile").
                         arg(BadooWrapper::getTextFromVote(bvMyVote));
        grpiBadge=grsScene.addPixmap(QPixmap());
        grpiBadge->setToolTip(sToolTip);
        if(BadooVote::VOTE_YES==bvMyVote)
            pxmBadge.load(QStringLiteral(":img/badge-you-liked.svg"));
        else
            pxmBadge.load(QStringLiteral(":img/badge-you-disliked.svg"));
        grpiBadge->setPixmap(
            pxmBadge.scaled(
                recBadges.height(),
                recBadges.height(),
                Qt::AspectRatioMode::KeepAspectRatio,
                Qt::TransformationMode::SmoothTransformation
            )
        );
        grpiBadge->setPos(recPhoto.x(),recPhoto.y());
    }
    iXOffset=-recBadges.height();
    for(const auto &b:pxmlBadges) {
        grpiBadge=grsScene.addPixmap(QPixmap());
        grpiBadge->setToolTip(slBadgeToolTips.takeFirst());
        if(!b.isNull()) {
            grpiBadge->setPixmap(
                b.scaled(
                    recBadges.height(),
                    recBadges.height(),
                    Qt::AspectRatioMode::KeepAspectRatio,
                    Qt::TransformationMode::SmoothTransformation
                )
            );
            grpiBadge->setPos(recBadges.x()+recBadges.width()+iXOffset,recBadges.y());
            iXOffset-=recBadges.height();
            if(0>recBadges.width()+iXOffset)
                break;
        }
    }
}

void FolderViewer::updateProfileMediaCounters(QRect recMediaInfo,
                                              int   iTotalPhotos,
                                              int   iTotalVideos) {
    std::function<int(int,int,QRect,QString,QString)> fnUpdateMediaCounter=
        [this](int     iCount,
               int     iXOffset,
               QRect   recWorkArea,
               QString sIconPath,
               QString sToolTip) {
            QString             sMediaCount;
            QPixmap             pxmMediaCountIcon;
            QRect               recMediaCount;
            QGraphicsTextItem   *grtiInfo=grsScene.addText(QString());
            QGraphicsPixmapItem *grpiInfo=grsScene.addPixmap(QPixmap());
            grpiInfo->setToolTip(sToolTip);
            pxmMediaCountIcon.load(sIconPath);
            if(!pxmMediaCountIcon.isNull()) {
                grpiInfo->setPixmap(
                    pxmMediaCountIcon.scaled(
                        recWorkArea.height(),
                        recWorkArea.height(),
                        Qt::AspectRatioMode::KeepAspectRatio,
                        Qt::TransformationMode::SmoothTransformation
                    )
                );
                grpiInfo->setPos(recWorkArea.x()+iXOffset,recWorkArea.y());
                iXOffset+=recWorkArea.height();
            }
            sMediaCount=QStringLiteral(" %1").arg(iCount,-3);
            recMediaCount=QFontMetrics(grtiInfo->font(),&grvView).boundingRect(
                recWorkArea,
                0,
                sMediaCount
            );
            grtiInfo->document()->setDocumentMargin(0);
            grtiInfo->setPlainText(sMediaCount);
            grtiInfo->setPos(
                recWorkArea.x()+iXOffset,
                recWorkArea.y()+(recWorkArea.height()-recMediaCount.height())/2
            );
            iXOffset+=recMediaCount.width()+recWorkArea.height()/2;
            return iXOffset;
        };
    int iX=0;
    if(iTotalPhotos)
        iX=fnUpdateMediaCounter(
            iTotalPhotos,
            iX,
            recMediaInfo,
            QStringLiteral(":img/info-photo.svg"),
            QStringLiteral("Total photos")
        );
    if(iTotalVideos)
        iX=fnUpdateMediaCounter(
            iTotalVideos,
            iX,
            recMediaInfo,
            QStringLiteral(":img/info-video.svg"),
            QStringLiteral("Total videos")
        );
}

void FolderViewer::updateProfilePhoto(QRect      recPhoto,
                                      QByteArray abtPhoto,
                                      int        iData) {
    QPixmap             pxmPhoto;
    QGraphicsPixmapItem *grpiPhoto=grsScene.addPixmap(QPixmap());
    pxmPhoto.loadFromData(abtPhoto);
    if(!pxmPhoto.isNull()) {
        grpiPhoto->setCursor(Qt::CursorShape::PointingHandCursor);
        grpiPhoto->setPixmap(
            pxmPhoto.scaled(
                recPhoto.width(),
                recPhoto.height(),
                Qt::AspectRatioMode::KeepAspectRatio,
                Qt::TransformationMode::SmoothTransformation
            )
        );
        grpiPhoto->setPos(
            recPhoto.x()+recPhoto.width()/2.0-grpiPhoto->pixmap().width()/2.0,
            recPhoto.y()+recPhoto.height()/2.0-grpiPhoto->pixmap().height()/2.0
        );
    }
    grpiPhoto->setData(0,iData);
}

void FolderViewer::updateProfileTitle(QRect   recTitle,
                                      QString sName,
                                      int     iAge,
                                      QString sCity) {
    int                 iYOffset;
    QString             sTitle;
    QFont               fntTitle;
    QTextOption         txoTitle;
    QGraphicsTextItem   *grtiTitle=grsScene.addText(QString());
    sTitle=sName;
    if(iAge)
        sTitle.append(QStringLiteral(", %1").arg(iAge));
    sTitle.append(QStringLiteral(", %1").arg(sCity));
    txoTitle.setAlignment(Qt::AlignmentFlag::AlignCenter);
    txoTitle.setWrapMode(QTextOption::WrapMode::WordWrap);
    grtiTitle->document()->setDefaultTextOption(txoTitle);
    grtiTitle->document()->setDocumentMargin(0);
    grtiTitle->setTextWidth(recTitle.width());
    grtiTitle->setPlainText(sTitle);
    fntTitle=grtiTitle->font();
    while(true) {
        int iFontHeight=QFontMetrics(fntTitle,&grvView).boundingRect(
            recTitle,
            Qt::AlignmentFlag::AlignCenter|Qt::TextFlag::TextWordWrap,
            sTitle
        ).height();
        iYOffset=(recTitle.height()-iFontHeight)/2;
        if(iFontHeight<=recTitle.height())
            break;
        else if(2>=fntTitle.pointSize())
            break;
        else {
            fntTitle.setPointSize(fntTitle.pointSize()-1);
            grtiTitle->setFont(fntTitle);
        }
    }
    grtiTitle->setPos(recTitle.x(),recTitle.y()+iYOffset);
}
