#include "mediacontrols.h"

MediaControls::MediaControls(QWidget         *wgtParent,
                             bool            bIncludeVideoButtons,
                             Qt::Orientation ornOrientation):
QWidget(wgtParent) {
    QLayout *layLayout;
    rButtonSizeRatio=1.0;
    alnHAlignment=Qt::AlignmentFlag::AlignHCenter;
    alnVAlignment=Qt::AlignmentFlag::AlignBottom;
    this->configureMediaButton(
        &btnFirst,
        QStringLiteral(":img/media-first.png")
    );
    this->configureMediaButton(
        &btnPrevious,
        QStringLiteral(":img/media-previous.png")
    );
    this->configureMediaButton(
        &btnNext,
        QStringLiteral(":img/media-next.png")
    );
    this->configureMediaButton(
        &btnLast,
        QStringLiteral(":img/media-last.png")
    );
    this->configureMediaButton(
        &btnPause,
        QStringLiteral(":img/media-play.png"),
        QStringLiteral(":img/media-pause.png")
    );
    this->configureMediaButton(
        &btnMute,
        QStringLiteral(":img/media-sound-off.png"),
        QStringLiteral(":img/media-sound-on.png")
    );
    if(Qt::Orientation::Horizontal==ornOrientation)
        layLayout=&hblLayout;
    else
        layLayout=&vblLayout;
    layLayout->setContentsMargins(0,0,0,0);
    layLayout->setSpacing(0);
    layLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    layLayout->addWidget(&btnFirst);
    layLayout->addWidget(&btnPrevious);
    if(bIncludeVideoButtons)
        layLayout->addWidget(&btnPause);
    layLayout->addWidget(&btnNext);
    layLayout->addWidget(&btnLast);
    if(bIncludeVideoButtons)
        layLayout->addWidget(&btnMute);
    this->setLayout(layLayout);
    this->resetVisualStatus();
    this->setPauseButtonState(false);
    this->setMuteButtonState(false);
    do {
        connect(
            &btnFirst,
            &QPushButton::clicked,
            this,
            &MediaControls::firstButtonClicked
        );
        connect(
            &btnPrevious,
            &QPushButton::clicked,
            this,
            &MediaControls::previousButtonClicked
        );
        connect(
            &btnNext,
            &QPushButton::clicked,
            this,
            &MediaControls::nextButtonClicked
        );
        connect(
            &btnLast,
            &QPushButton::clicked,
            this,
            &MediaControls::lastButtonClicked
        );
        connect(
            &btnPause,
            &QPushButton::clicked,
            this,
            &MediaControls::pauseButtonClicked
        );
        connect(
            &btnMute,
            &QPushButton::clicked,
            this,
            &MediaControls::muteButtonClicked
        );
    } while(false);
}

void MediaControls::resizeEvent(QResizeEvent *) {
    this->resetVisualStatus();
}

void MediaControls::firstButtonClicked() {
    emit first();
}

void MediaControls::previousButtonClicked() {
    emit previous();
}

void MediaControls::nextButtonClicked() {
    emit next();
}

void MediaControls::lastButtonClicked() {
    emit last();
}

void MediaControls::pauseButtonClicked(bool bChecked) {
    emit pause(bChecked);
}

void MediaControls::muteButtonClicked(bool bChecked) {
    emit mute(bChecked);
}

void MediaControls::configureMediaButton(QPushButton *btnButton,
                                         QString     sImagePathOn,
                                         QString     sImagePathOff) {
    QIcon   icoNormal;
    QPixmap pxmNormal;
    pxmNormal.load(sImagePathOn);
    icoNormal.addPixmap(pxmNormal,QIcon::Mode::Normal,QIcon::State::On);
    btnButton->setCheckable(false);
    if(!sImagePathOff.isEmpty()) {
        pxmNormal.load(sImagePathOff);
        icoNormal.addPixmap(pxmNormal,QIcon::Mode::Normal,QIcon::State::Off);
        btnButton->setCheckable(true);
        btnButton->setChecked(false);
    }
    btnButton->setCursor(Qt::CursorShape::PointingHandCursor);
    btnButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    btnButton->setIcon(icoNormal);
    btnButton->setIconSize(pxmNormal.size());
    btnButton->setProperty(
        QStringLiteral("base_icon_size").toUtf8(),
        pxmNormal.size()
    );
    btnButton->setStyleSheet(QStringLiteral("background-color: transparent"));
}

void MediaControls::setAlignment(Qt::AlignmentFlag alnHNew,
                                 Qt::AlignmentFlag alnVNew) {
    alnHAlignment=alnHNew;
    alnVAlignment=alnVNew;
    if(nullptr!=this->parent()) {
        int     iX=0,
                iY=0;
        QWidget *wgtParent=qobject_cast<QWidget *>(this->parent());
        if(Qt::AlignmentFlag::AlignLeft==alnHAlignment)
            ;
        else if(Qt::AlignmentFlag::AlignHCenter==alnHAlignment)
            iX=wgtParent->width()/2-this->width()/2;
        else if(Qt::AlignmentFlag::AlignRight==alnHAlignment)
            iX=wgtParent->width()-this->width();
        if(Qt::AlignmentFlag::AlignTop==alnVAlignment)
            ;
        else if(Qt::AlignmentFlag::AlignVCenter==alnVAlignment)
            iY=wgtParent->height()/2-this->height()/2;
        else if(Qt::AlignmentFlag::AlignBottom==alnVAlignment)
            iY=wgtParent->height()-this->height();
        this->move(iX,iY);
    }
}

void MediaControls::setButtonsEnabling(int iIndex,
                                       int iTotal) {
    btnFirst.setEnabled(false);
    btnPrevious.setEnabled(false);
    btnNext.setEnabled(false);
    btnLast.setEnabled(false);
    btnPause.setEnabled(false);
    btnMute.setEnabled(false);
    if(0<iTotal)
        if(0<=iIndex&&iIndex<iTotal) {
            btnPause.setEnabled(true);
            btnMute.setEnabled(true);
            if(iIndex) {
                btnFirst.setEnabled(true);
                btnPrevious.setEnabled(true);
            }
            if(iIndex<iTotal-1) {
                btnNext.setEnabled(true);
                btnLast.setEnabled(true);
            }
        }
}

void MediaControls::setButtonSizeRatio(qreal rNewButtonSizeRatio) {
    rButtonSizeRatio=rNewButtonSizeRatio;
    for(const auto b:this->findChildren<QPushButton *>()) {
        QSize sizButton=b->property(
            QStringLiteral("base_icon_size").toUtf8()
        ).toSize();
        sizButton.setWidth(sizButton.width()*rButtonSizeRatio);
        sizButton.setHeight(sizButton.height()*rButtonSizeRatio);
        b->setIconSize(sizButton);
    }
}

void MediaControls::setPauseButtonState(bool bState) {
    btnPause.setChecked(bState);
}

void MediaControls::setMuteButtonState(bool bState) {
    btnMute.setChecked(bState);
}

void MediaControls::resetVisualStatus() {
    this->setButtonSizeRatio(rButtonSizeRatio);
    this->setAlignment(alnHAlignment,alnVAlignment);
}
