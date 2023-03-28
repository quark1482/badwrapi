#include "mediaviewer.h"

MediaViewer::MediaViewer(MediaType mtNew,QWidget *wgtParent):QWidget(wgtParent) {
    bAutoHideOverlay=false;
    wgtOverlay=nullptr;
    mtType=mtNew;
    abtMedia.clear();
    if(MEDIA_TYPE_PHOTO==mtType) {
        grsScene.addItem(&grpiPixmap);
    }
    else if(MEDIA_TYPE_VIDEO==mtType) {
        grsScene.addItem(&grviVideo);
        aoAudio.setMuted(false);
        aoAudio.setVolume((float)0.1);
        mpPlayer.setAudioOutput(&aoAudio);
        mpPlayer.setLoops(QMediaPlayer::Loops::Infinite);
        mpPlayer.setVideoOutput(&grviVideo);
        grviVideo.setAspectRatioMode(Qt::AspectRatioMode::KeepAspectRatio);
    }
    grsScene.setBackgroundBrush(QBrush(QColor(Qt::GlobalColor::black)));
    // Allows the main widget to detect the mouse hovering.
    grvView.setAttribute(Qt::WidgetAttribute::WA_TransparentForMouseEvents);
    grvView.setAlignment(Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);
    // Allows the main widget to detect the arrow-keys presses.
    grvView.setFocusPolicy(Qt::FocusPolicy::NoFocus);
    grvView.setFrameShape(QFrame::Shape::NoFrame);
    grvView.setScene(&grsScene);
    vblLayout.setContentsMargins(0,0,0,0);
    vblLayout.addWidget(&grvView);
    this->setLayout(&vblLayout);
    this->setMouseTracking(true);
    this->setWindowModality(Qt::WindowModality::ApplicationModal);
}

void MediaViewer::keyPressEvent(QKeyEvent *evnE) {
    emit keyPress(evnE->key());
    evnE->accept();
}

void MediaViewer::mouseDoubleClickEvent(QMouseEvent *evnE) {
    emit doubleClick();
    evnE->accept();
}

void MediaViewer::mouseMoveEvent(QMouseEvent *evnE) {
    emit hover(evnE->pos());
    evnE->accept();
}

void MediaViewer::getFrame(QByteArray abtContent,
                           QPixmap    &pxmFrame,
                           qreal      rPosition) {
    QEventLoop   evlLoop;
    QMediaPlayer mpPlayer;
    QVideoSink   vskSink;
    QBuffer      bufSource;
    qint64       i64Position;
    pxmFrame=QPixmap();
    rPosition=fabs(rPosition);
    if(rPosition>1)
        rPosition=QRandomGenerator::global()->generateDouble();
    mpPlayer.setVideoSink(&vskSink);
    bufSource.setBuffer(&abtContent);
    if(bufSource.open(QBuffer::OpenModeFlag::ReadOnly)) {
        i64Position=0;
        mpPlayer.setSourceDevice(&bufSource);
        bufSource.seek(0);
        if(QMediaPlayer::Error::NoError==mpPlayer.error()) {
            QObject::connect(
                &mpPlayer,
                &QMediaPlayer::mediaStatusChanged,
                [&](QMediaPlayer::MediaStatus mpmsStatus) {
                    if(QMediaPlayer::MediaStatus::BufferedMedia==mpmsStatus) {
                        i64Position=mpPlayer.duration()*rPosition;
                        mpPlayer.setPosition(i64Position);
                    }
                    else if(QMediaPlayer::MediaStatus::StalledMedia==mpmsStatus)
                        evlLoop.exit();
                    else if(QMediaPlayer::MediaStatus::InvalidMedia==mpmsStatus)
                        evlLoop.exit();
                }
            );
            QObject::connect(
                &mpPlayer,
                &QMediaPlayer::errorOccurred,
                [&](QMediaPlayer::Error,const QString &) {
                    evlLoop.exit();
                }
            );
            QObject::connect(
                &vskSink,
                &QVideoSink::videoFrameChanged,
                [&](const QVideoFrame &vfrFrame) {
                    qint64 iFrameTime=vfrFrame.startTime()/1000;
                    if(pxmFrame.isNull())
                        if(!mpPlayer.isSeekable()||!rPosition||i64Position&&iFrameTime>=i64Position) {
                            pxmFrame=QPixmap::fromImage(vfrFrame.toImage());
                            evlLoop.exit();
                        }
                }
            );
            mpPlayer.play();
            evlLoop.exec(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
            mpPlayer.stop();
        }
        mpPlayer.setSourceDevice(nullptr);
        bufSource.close();
    }
    mpPlayer.setVideoSink(nullptr);
}

qint64 MediaViewer::getPosition() {
    return mpPlayer.position();
}

void MediaViewer::loadVideo(QByteArray abtContent) {
    if(MEDIA_TYPE_VIDEO==mtType) {
        this->stopVideo();
        abtMedia.clear();
        if(nullptr!=mpPlayer.sourceDevice())
            mpPlayer.setSourceDevice(nullptr);
        if(bufVideo.isOpen())
            bufVideo.close();
        if(!abtContent.isNull()) {
            abtMedia=abtContent;
            bufVideo.setBuffer(&abtMedia);
            if(bufVideo.open(QBuffer::OpenModeFlag::ReadOnly)) {
                mpPlayer.setSourceDevice(&bufVideo);
                bufVideo.seek(0);
            }
        }
    }
}

void MediaViewer::muteVideo(bool bMute) {
    aoAudio.setMuted(bMute);
}

void MediaViewer::playVideo() {
    if(MEDIA_TYPE_VIDEO==mtType) {
        this->showVideo();
        if(bufVideo.isOpen())
            if(QMediaPlayer::Error::NoError==mpPlayer.error()) {
                // Confirms that the video is actually playing before returning to the caller.
                // Given the asynchronous nature of the QMediaPlayer::play() operation and ...
                // ... the amount of things that happens before a single frame is shown, ...
                // ... this approach frees the caller of the responsibility of figuring ...
                // ... out the video playing status.
                if(QMediaPlayer::MediaStatus::BufferedMedia==mpPlayer.mediaStatus())
                    mpPlayer.play();
                else {
                    QEventLoop evlLoop;
                    auto c1=QObject::connect(
                        &mpPlayer,
                        &QMediaPlayer::mediaStatusChanged,
                        [&](QMediaPlayer::MediaStatus mpmsStatus) {
                            if(QMediaPlayer::MediaStatus::BufferedMedia==mpmsStatus)
                                evlLoop.exit();
                            else if(QMediaPlayer::MediaStatus::StalledMedia==mpmsStatus)
                                evlLoop.exit();
                            else if(QMediaPlayer::MediaStatus::InvalidMedia==mpmsStatus)
                                evlLoop.exit();
                        }
                    );
                    auto c2=QObject::connect(
                        &mpPlayer,
                        &QMediaPlayer::errorOccurred,
                        [&](QMediaPlayer::Error,const QString &) {
                            evlLoop.exit();
                        }
                    );
                    mpPlayer.play();
                    evlLoop.exec(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
                    QObject::disconnect(c1);
                    QObject::disconnect(c2);
                }
            }
    }
}

void MediaViewer::pauseVideo() {
    if(MEDIA_TYPE_VIDEO==mtType)
        if(QMediaPlayer::PlaybackState::PlayingState==mpPlayer.playbackState())
            mpPlayer.pause();
}

void MediaViewer::setAutoHideOverlay(bool bNew) {
    bAutoHideOverlay=bNew;
}

void MediaViewer::setOverlay(QWidget *wgtNew) {
    wgtOverlay=wgtNew;
}

void MediaViewer::setPosition(qint64 i64New) {
    mpPlayer.setPosition(i64New);
}

void MediaViewer::showPhoto(QByteArray abtContent) {
    if(MEDIA_TYPE_PHOTO==mtType) {
        QPixmap pxmPhoto;
        abtMedia.clear();
        if(this->isWindow())
            this->showFullScreen();
        else
            this->showNormal();
        grsScene.setSceneRect(this->rect());
        // Removes any pre-existing graphical leftover if the viewer is being resized.
        grsScene.update(this->rect());
        pxmPhoto.loadFromData(abtContent);
        if(!pxmPhoto.isNull()) {
            abtMedia=abtContent;
            grpiPixmap.setPixmap(
                pxmPhoto.scaled(
                    this->size(),
                    Qt::AspectRatioMode::KeepAspectRatio,
                    Qt::TransformationMode::SmoothTransformation
                )
            );
            grpiPixmap.setPos(
                grsScene.width()/2.0-grpiPixmap.pixmap().width()/2.0,
                grsScene.height()/2.0-grpiPixmap.pixmap().height()/2.0
            );
        }
        else
            grpiPixmap.setPixmap(pxmPhoto);
    }
}

void MediaViewer::showVideo() {
    if(MEDIA_TYPE_VIDEO==mtType) {
        if(this->isWindow())
            this->showFullScreen();
        else
            this->showNormal();
        grsScene.setSceneRect(this->rect());
        // Removes any pre-existing graphical leftover if the viewer is being resized.
        grsScene.update(this->rect());
        grviVideo.setSize(QSizeF(grsScene.width(),grsScene.height()));
        grviVideo.setPos(
            grsScene.width()/2.0-grviVideo.size().width()/2.0,
            grsScene.height()/2.0-grviVideo.size().height()/2.0
        );
    }
}

void MediaViewer::stopVideo() {
    if(MEDIA_TYPE_VIDEO==mtType)
        if(QMediaPlayer::PlaybackState::StoppedState!=mpPlayer.playbackState())
            mpPlayer.stop();
}
