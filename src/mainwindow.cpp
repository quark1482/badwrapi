#include "mainwindow.h"
#include "./ui_mainwindow.h"

#define APP_TITLE "Badoo API wrapper demo"

#define DB_NAME   "BadWrAPI-demo.db"

MainWindow::MainWindow(QWidget *parent):
QMainWindow(parent),ui(new Ui::MainWindow) {
    dbMain=new DB;
    pxyMain=new QNetworkProxy(QNetworkProxy::ProxyType::NoProxy);
    ui->setupUi(this);
    dlgBrowseCustom=nullptr;
    dlgBrowseChats=nullptr;
    dlgBrowseFavorites=nullptr;
    dlgBrowseLikes=nullptr;
    dlgBrowseMatches=nullptr;
    dlgBrowsePeopleNearby=nullptr;
    dlgBrowseVisitors=nullptr;
    dlgBrowseBlocked=nullptr;
    dlgEncounters=nullptr;
    mdiArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setCentralWidget(&mdiArea);
    this->setWindowTitle(QStringLiteral(APP_TITLE));
#if defined(HTTP_USER_AGENT)
    BadooAPI::setAgent(QStringLiteral(HTTP_USER_AGENT));
    TAGeoCoder::setAgent(QStringLiteral(HTTP_USER_AGENT));
#endif
#if defined(HTTP_PROXY_HOST)&&defined(HTTP_PROXY_PORT)
    pxyMain->setType(QNetworkProxy::ProxyType::HttpProxy);
    pxyMain->setHostName(QStringLiteral(HTTP_PROXY_HOST));
    pxyMain->setPort(HTTP_PROXY_PORT);
    BadooAPI::setProxy(pxyMain);
    TAGeoCoder::setProxy(pxyMain);
#endif
    do {
        connect(
            ui->actEncounters,
            &QAction::triggered,
            this,
            &MainWindow::menuEncountersTriggered
        );
        connect(
            ui->actChats,
            &QAction::triggered,
            this,
            &MainWindow::menuBrowseFolderTriggered
        );
        connect(
            ui->actFavorites,
            &QAction::triggered,
            this,
            &MainWindow::menuBrowseFolderTriggered
        );
        connect(
            ui->actLikes,
            &QAction::triggered,
            this,
            &MainWindow::menuBrowseFolderTriggered
        );
        connect(
            ui->actMatches,
            &QAction::triggered,
            this,
            &MainWindow::menuBrowseFolderTriggered
        );
        connect(
            ui->actAllPeopleNearby,
            &QAction::triggered,
            this,
            &MainWindow::menuBrowseFolderTriggered
        );
        connect(
            ui->actOnlinePeopleNearby,
            &QAction::triggered,
            this,
            &MainWindow::menuBrowseFolderTriggered
        );
        connect(
            ui->actNewPeopleNearby,
            &QAction::triggered,
            this,
            &MainWindow::menuBrowseFolderTriggered
        );
        connect(
            ui->actVisitors,
            &QAction::triggered,
            this,
            &MainWindow::menuBrowseFolderTriggered
        );
        connect(
            ui->actBlocked,
            &QAction::triggered,
            this,
            &MainWindow::menuBrowseFolderTriggered
        );
        connect(
            ui->actCustomFolder,
            &QAction::triggered,
            this,
            &MainWindow::menuBrowseFolderTriggered
        );
        connect(
            ui->actCustomProfile,
            &QAction::triggered,
            this,
            &MainWindow::menuBrowseProfileTriggered
        );
        connect(
            ui->actSessionDetails,
            &QAction::triggered,
            this,
            &MainWindow::menuEditSessionTriggered
        );
        connect(
            ui->actExit,
            &QAction::triggered,
            this,
            &MainWindow::close
        );
        connect(
            ui->actLocation,
            &QAction::triggered,
            this,
            &MainWindow::menuLocationTriggered
        );
        connect(
            ui->actLogin,
            &QAction::triggered,
            this,
            &MainWindow::menuLoginTriggered
        );
        connect(
            ui->actLogout,
            &QAction::triggered,
            this,
            &MainWindow::menuLogoutTriggered
        );
        connect(
            ui->actVisibility,
            &QAction::triggered,
            this,
            &MainWindow::menuVisibilityTriggered
        );
        connect(
            ui->actEncountersSettings,
            &QAction::triggered,
            this,
            &MainWindow::menuEncountersSettingsTriggered
        );
        connect(
            ui->actPeopleNearbySettings,
            &QAction::triggered,
            this,
            &MainWindow::menuPeopleNearbySettingsTriggered
        );
        connect(
            &bwMain,
            &BadooWrapper::statusChanged,
            this,
            &MainWindow::wrapperStatusChanged
        );
    } while(false);
    QTimer::singleShot(
        0,
        [=]() {
            if(!this->postInit())
                QApplication::quit();
        }
    );
}

MainWindow::~MainWindow() {
    WidgetGeometry wggGeometry;
    if(this->isMaximized())
        wggGeometry.wgsStatus=wgsMAXIMIZED;
    else if(this->isMinimized())
        wggGeometry.wgsStatus=wgsMINIMIZED;
    else
        wggGeometry.wgsStatus=wgsNORMAL;
    wggGeometry.recRect=this->normalGeometry();
    dbMain->saveSetting(stgMAIN,wggGeometry);
    delete dbMain;
    delete pxyMain;
    if(nullptr!=dlgBrowseCustom)
        delete dlgBrowseCustom;
    if(nullptr!=dlgBrowseChats)
        delete dlgBrowseChats;
    if(nullptr!=dlgBrowseFavorites)
        delete dlgBrowseFavorites;
    if(nullptr!=dlgBrowseLikes)
        delete dlgBrowseLikes;
    if(nullptr!=dlgBrowseMatches)
        delete dlgBrowseMatches;
    if(nullptr!=dlgBrowsePeopleNearby)
        delete dlgBrowsePeopleNearby;
    if(nullptr!=dlgBrowseVisitors)
        delete dlgBrowseVisitors;
    if(nullptr!=dlgBrowseBlocked)
        delete dlgBrowseBlocked;
    if(nullptr!=dlgEncounters)
        delete dlgEncounters;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *evnE) {
    if(this->anyChildrenActive()) {
        QMessageBox::critical(
            this,
            QStringLiteral("Error"),
            QStringLiteral("Close the active windows first")
        );
        evnE->ignore();
    }
    else
        evnE->accept();
}

bool MainWindow::eventFilter(QObject *objO,
                             QEvent  *evnE) {
    if(QEvent::Type::Close==evnE->type()) {
        QDialog        *dlgChild=qobject_cast<QDialog *>(objO);
        QMdiSubWindow  *mdsChild=qobject_cast<QMdiSubWindow *>(objO->parent());
        SettingsGroup  stgGroup=static_cast<SettingsGroup>(
            dlgChild->property(QStringLiteral("group").toUtf8()).toInt()
        );
        WidgetGeometry wggGeometry;
        if(mdsChild->isMaximized())
            wggGeometry.wgsStatus=wgsMAXIMIZED;
        else if(mdsChild->isMinimized())
            wggGeometry.wgsStatus=wgsMINIMIZED;
        else
            wggGeometry.wgsStatus=wgsNORMAL;
        // Among the bunch of annoyances that the MDI interface brings to the ...
        // ... table, is that the QRect returned by normalGeometry() is null, ...
        // ... so the child window needs to be restored to its 'normal' state ...
        // ... before getting that geometry.
        mdsChild->showNormal();
        wggGeometry.recRect=mdsChild->geometry();
        dbMain->saveSetting(stgGroup,wggGeometry);
        if(dlgBrowseCustom==dlgChild)
            dlgBrowseCustom=nullptr;
        else if(dlgBrowseChats==dlgChild)
            dlgBrowseChats=nullptr;
        else if(dlgBrowseFavorites==dlgChild)
            dlgBrowseFavorites=nullptr;
        else if(dlgBrowseLikes==dlgChild)
            dlgBrowseLikes=nullptr;
        else if(dlgBrowseMatches==dlgChild)
            dlgBrowseMatches=nullptr;
        else if(dlgBrowsePeopleNearby==dlgChild)
            dlgBrowsePeopleNearby=nullptr;
        else if(dlgBrowseVisitors==dlgChild)
            dlgBrowseVisitors=nullptr;
        else if(dlgBrowseBlocked==dlgChild)
            dlgBrowseBlocked=nullptr;
        else if(dlgEncounters==dlgChild)
            dlgEncounters=nullptr;
        delete dlgChild;
        return true; // 2024-10-20: This was missing. Potential crash.
    }
    return QObject::eventFilter(objO,evnE);
}

void MainWindow::menuBrowseFolderTriggered(bool) {
    BrowseFolderDialog  *dlgFolder=nullptr;
    FolderType          ftFolder;
    BadooListFilterList blflFilters;
    if(ui->actCustomFolder==QObject::sender()) {
        dlgFolder=dlgBrowseCustom;
        ftFolder=FOLDER_TYPE_UNKNOWN;
    }
    else if(ui->actChats==QObject::sender()) {
        dlgFolder=dlgBrowseChats;
        ftFolder=FOLDER_TYPE_CHATS;
    }
    else if(ui->actFavorites==QObject::sender()) {
        dlgFolder=dlgBrowseFavorites;
        ftFolder=FOLDER_TYPE_FAVORITES;
    }
    else if(ui->actLikes==QObject::sender()) {
        dlgFolder=dlgBrowseLikes;
        ftFolder=FOLDER_TYPE_LIKES;
    }
    else if(ui->actMatches==QObject::sender()) {
        dlgFolder=dlgBrowseMatches;
        ftFolder=FOLDER_TYPE_MATCHES;
    }
    else if(ui->actAllPeopleNearby==QObject::sender()) {
        dlgFolder=dlgBrowsePeopleNearby;
        ftFolder=FOLDER_TYPE_PEOPLE_NEARBY;
    }
    else if(ui->actOnlinePeopleNearby==QObject::sender()) {
        dlgFolder=dlgBrowsePeopleNearby;
        ftFolder=FOLDER_TYPE_PEOPLE_NEARBY;
        blflFilters={LIST_FILTER_ONLINE};
    }
    else if(ui->actNewPeopleNearby==QObject::sender()) {
        dlgFolder=dlgBrowsePeopleNearby;
        ftFolder=FOLDER_TYPE_PEOPLE_NEARBY;
        blflFilters={LIST_FILTER_NEW};
    }
    else if(ui->actVisitors==QObject::sender()) {
        dlgFolder=dlgBrowseVisitors;
        ftFolder=FOLDER_TYPE_VISITORS;
    }
    else if(ui->actBlocked==QObject::sender()) {
        dlgFolder=dlgBrowseBlocked;
        ftFolder=FOLDER_TYPE_BLOCKED;
    }
    if(bwMain.isLoggedIn())
        if(nullptr==dlgFolder) {
            if(FOLDER_TYPE_UNKNOWN==ftFolder) {
                int                  iMediaCount;
                BadooFolderType      bftFolder;
                BadooListSectionType blstSection;
                if(this->getCustomFolderParameters(
                    bftFolder,
                    blstSection,
                    blflFilters,
                    iMediaCount
                )) {
                    dlgFolder=new BrowseFolderDialog(
                        bftFolder,
                        blstSection,
                        blflFilters,
                        &bwMain,
                        iMediaCount,
                        dbMain,
                        this
                    );
                    if(!dlgFolder->isReady()) {
                        delete dlgFolder;
                        dlgFolder=nullptr;
                    }
                }
            }
            else {
                dlgFolder=new BrowseFolderDialog(
                    ftFolder,
                    blflFilters,
                    &bwMain,
                    0,
                    dbMain,
                    this
                );
                if(!dlgFolder->isReady()) {
                    delete dlgFolder;
                    dlgFolder=nullptr;
                }
            }
            if(nullptr!=dlgFolder) {
                switch(ftFolder) {
                    case FOLDER_TYPE_UNKNOWN:
                        dlgBrowseCustom=dlgFolder;
                        break;
                    case FOLDER_TYPE_CHATS:
                        dlgBrowseChats=dlgFolder;
                        break;
                    case FOLDER_TYPE_FAVORITES:
                        dlgBrowseFavorites=dlgFolder;
                        break;
                    case FOLDER_TYPE_LIKES:
                        dlgBrowseLikes=dlgFolder;
                        break;
                    case FOLDER_TYPE_MATCHES:
                        dlgBrowseMatches=dlgFolder;
                        break;
                    case FOLDER_TYPE_PEOPLE_NEARBY:
                        dlgBrowsePeopleNearby=dlgFolder;
                        break;
                    case FOLDER_TYPE_VISITORS:
                        dlgBrowseVisitors=dlgFolder;
                        break;
                    case FOLDER_TYPE_BLOCKED:
                        dlgBrowseBlocked=dlgFolder;
                        break;
                }
                connect(
                    dlgFolder,
                    &BrowseFolderDialog::statusChanged,
                    this,
                    &MainWindow::wrapperStatusChanged
                );
                this->showChildDialog(
                    (FOLDER_TYPE_UNKNOWN==ftFolder)?stgCUSTOM:stgFOLDERS,
                    dlgFolder
                );
            }
        }
        else
            QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                QStringLiteral("Already browsing %1").arg(bwMain.getFolderName(ftFolder))
            );
    else
        QMessageBox::critical(
            this,
            QStringLiteral("Error"),
            QStringLiteral("Not logged in")
        );
}

void MainWindow::menuBrowseProfileTriggered(bool) {
    if(bwMain.isLoggedIn()) {
        QInputDialog idProfile(this);
        QVBoxLayout  *vblMainLayout;
        QHBoxLayout  hblProfile,
                     hblWarning;
        QLabel       lblWarningIcon,
                     lblWarningText;
        QLineEdit    *ledProfile;
        QPushButton  btnPaste(&idProfile);
        // Forces the resize of the QInputDialog to something more adequate by ...
        // ... setting its QLabel to some maximum length string before showing ...
        // ... the dialog itself.
        idProfile.setLabelText(QStringLiteral("W").repeated(MIN_USER_ID_LENGTH));
        idProfile.setWindowFlag(Qt::WindowType::MSWindowsFixedSizeDialogHint);
        idProfile.setWindowModality(Qt::WindowModality::ApplicationModal);
        idProfile.setWindowTitle(QStringLiteral("Custom profile"));
        idProfile.show();
        vblMainLayout=qobject_cast<QVBoxLayout *>(idProfile.layout());
        // Moves the QLineEdit (Profile URL or Id) to its own QHBoxLayout ...
        // ... and adds a QPushButton (paste from clipboard) at the side.
        ledProfile=idProfile.findChild<QLineEdit *>();
        idProfile.layout()->removeWidget(ledProfile);
        hblProfile.addWidget(ledProfile);
        hblProfile.addWidget(&btnPaste);
        btnPaste.setIcon(QIcon(QStringLiteral(":/img/tool-paste.png")));
        vblMainLayout->insertLayout(1,&hblProfile);
        lblWarningIcon.setPixmap(
            QApplication::style()->standardPixmap(
                QStyle::StandardPixmap::SP_MessageBoxWarning
            )
        );
        lblWarningText.setText(
            QStringLiteral("<i>Warning: by browsing this profile directly, you become<br>"
                           "a visitor for them, completely losing your anonymity.</i>")
        );
        hblWarning.addWidget(&lblWarningIcon);
        hblWarning.addWidget(&lblWarningText);
        hblWarning.addStretch();
        vblMainLayout->addLayout(&hblWarning);
        idProfile.setLabelText(QStringLiteral("Profile URL or Id:"));
        idProfile.setTabOrder({ledProfile,&btnPaste});
        connect(
            &btnPaste,
            &QPushButton::clicked,
            [=](bool) {
                QClipboard *clpBoard=QApplication::clipboard();
                if(clpBoard->mimeData()->hasText()) {
                    ledProfile->setText(clpBoard->text().trimmed());
                    ledProfile->selectAll();
                    ledProfile->setFocus();
                }
            }
        );
        while(true)
            if(QDialog::DialogCode::Accepted==idProfile.exec()) {
                QString sProfile=idProfile.textValue().trimmed();
                if(sProfile.isEmpty())
                    break;
                QUrl urlProfile=QUrl(sProfile);
                if(urlProfile.isValid()) {
                    QString sSubdomain=QStringLiteral(".%1").arg(DOMAIN_BASE);
                    if(urlProfile.host()==DOMAIN_BASE||urlProfile.host().endsWith(sSubdomain))
                        sProfile=urlProfile.
                                 adjusted(QUrl::UrlFormattingOption::StripTrailingSlash).
                                 fileName();
                }
                if(sProfile.startsWith('0'))
                    sProfile.remove(0,1);
                if(BadooWrapper::isValidUserId(sProfile)) {
                    this->showCustomProfile(sProfile);
                    break;
                }
                else
                    QMessageBox::critical(
                        this,
                        QStringLiteral("Error"),
                        QStringLiteral("That does not seem like\n"
                                       "a profile URL or Id!")
                    );
            }
            else
                break;
    }
    else
        QMessageBox::critical(
            this,
            QStringLiteral("Error"),
            QStringLiteral("Not logged in")
        );
}

void MainWindow::menuEditSessionTriggered(bool) {
    bool           bSuccess;
    QString        sSes,sDev,sUsr,sAcc;
    SessionDetails sdCopy;
    bwMain.getSessionDetails(sSes,sDev,sUsr,sAcc);
    bwMain.getSessionDetails(sdCopy);
    if(this->editSessionDetails(sSes,sDev,sUsr,sAcc)) {
        bwMain.setSessionDetails(sSes,sDev,sUsr,sAcc);
        bSuccess=false;
        do {
            if(bwMain.loginBack()) {
                bSuccess=true;
                break;
            }
            else if(QMessageBox::StandardButton::Cancel==QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                dbMain->getLastError(),
                QMessageBox::StandardButton::Retry|
                QMessageBox::StandardButton::Cancel,
                QMessageBox::StandardButton::Retry
            ))
                break;
        } while(true);
        if(bSuccess) {
            bSuccess=false;
            do {
                if(dbMain->saveSession(sSes,sDev,sUsr,sAcc)) {
                    bSuccess=true;
                    break;
                }
                else if(QMessageBox::StandardButton::Cancel==QMessageBox::critical(
                    this,
                    QStringLiteral("Error"),
                    dbMain->getLastError(),
                    QMessageBox::StandardButton::Retry|
                    QMessageBox::StandardButton::Cancel,
                    QMessageBox::StandardButton::Retry
                ))
                    break;
            } while(true);
        }
        if(bSuccess) {
            this->testAndSetLocation();
            this->testAndSetVisibility();
            ui->stbMain->showMessage(QStringLiteral("Session edited"));
        }
        else
            bwMain.setSessionDetails(sdCopy);
    }
}

void MainWindow::menuEncountersTriggered(bool) {
    if(bwMain.isLoggedIn())
        if(nullptr==dlgEncounters) {
            dlgEncounters=new PlayEncountersDialog(&bwMain,this);
            if(!dlgEncounters->isReady()) {
                delete dlgEncounters;
                dlgEncounters=nullptr;
            }
            else {
                connect(
                    dlgEncounters,
                    &PlayEncountersDialog::statusChanged,
                    this,
                    &MainWindow::wrapperStatusChanged
                );
                this->showChildDialog(stgENCOUNTERS,dlgEncounters);
            }
        }
        else
            QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                QStringLiteral("Already playing Encounters")
            );
    else
        QMessageBox::critical(
            this,
            QStringLiteral("Error"),
            QStringLiteral("Not logged in")
        );
}

void MainWindow::menuLocationTriggered(bool) {
    this->testAndSetLocation(true);
}

void MainWindow::menuLoginTriggered(bool) {
    if(bwMain.isLoggedIn())
        QMessageBox::critical(
            this,
            QStringLiteral("Error"),
            QStringLiteral("Already logged in")
        );
    else
        if(bwMain.showLoginDialog(this)) {
            QString sSes,sDev,sUsr,sAcc;
            bwMain.getSessionDetails(sSes,sDev,sUsr,sAcc);
            do {
                if(dbMain->saveSession(sSes,sDev,sUsr,sAcc))
                    break;
                else if(QMessageBox::StandardButton::Cancel==QMessageBox::critical(
                    this,
                    QStringLiteral("Error"),
                    dbMain->getLastError(),
                    QMessageBox::StandardButton::Retry|
                    QMessageBox::StandardButton::Cancel,
                    QMessageBox::StandardButton::Retry
                ))
                    break;
            } while(true);
            this->testAndSetLocation();
            this->testAndSetVisibility();
            ui->stbMain->showMessage(QStringLiteral("Logged in"));
        }
}

void MainWindow::menuLogoutTriggered(bool) {
    if(bwMain.isLoggedIn())
        if(!this->anyChildrenActive()) {
            if(QMessageBox::StandardButton::Yes==QMessageBox::warning(
                this,
                QStringLiteral("Warning"),
                QStringLiteral("Do you really want to end your session?"),
                QMessageBox::StandardButton::Yes|QMessageBox::StandardButton::No,
                QMessageBox::StandardButton::No
            ))
                if(bwMain.logout()) {
                    dbMain->saveSession(QString(),QString(),QString(),QString());
                    this->testAndSetLocation();
                    this->testAndSetVisibility();
                    ui->stbMain->showMessage(QStringLiteral("Logged out"));
                }
                else
                    QMessageBox::critical(
                        this,
                        QStringLiteral("Error"),
                        bwMain.getLastError()
                    );
        }
        else
            QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                QStringLiteral("Close the active windows first")
            );
    else
        QMessageBox::critical(
            this,
            QStringLiteral("Error"),
            QStringLiteral("Not logged in")
        );
}

void MainWindow::menuEncountersSettingsTriggered(bool) {
    showSettings(SETTINGS_CONTEXT_TYPE_ENCOUNTERS);
}

void MainWindow::menuPeopleNearbySettingsTriggered(bool) {
    showSettings(SETTINGS_CONTEXT_TYPE_PEOPLE_NEARBY);
}

void MainWindow::menuVisibilityTriggered(bool) {
    this->testAndSetVisibility(true);
}

void MainWindow::wrapperStatusChanged(QString sStatus) {
    if(sStatus.isEmpty())
        ui->stbMain->clearMessage();
    else
        ui->stbMain->showMessage(sStatus);
}

bool MainWindow::anyChildrenActive() {
    // Checks for any (||) active dialog.
    return nullptr!=dlgBrowseCustom||
           nullptr!=dlgBrowseChats||
           nullptr!=dlgBrowseFavorites||
           nullptr!=dlgBrowseLikes||
           nullptr!=dlgBrowseMatches||
           nullptr!=dlgBrowsePeopleNearby||
           nullptr!=dlgBrowseVisitors||
           nullptr!=dlgBrowseBlocked||
           nullptr!=dlgEncounters;
}

bool MainWindow::editSessionDetails(QString &sSessionId,
                                    QString &sDeviceId,
                                    QString &sUserId,
                                    QString &sAccountId) {
    bool        bResult=false;
    int         iMinEditWidth;
    QDialog     dlgESDetails(this);
    QVBoxLayout vblESDetails;
    QFormLayout fmlSession;
    QHBoxLayout hblSession,
                hblDevice,
                hblUser,
                hblAccount,
                hblButtons,
                hblWarning;
    QLabel      lblWarningIcon,
                lblWarningText;
    QLineEdit   ledSessionId,
                ledDeviceId,
                ledUserId,
                ledAccountId;
    QPushButton btnCopySessionId,
                btnPasteSessionId,
                btnCopyDeviceId,
                btnPasteDeviceId,
                btnCopyUserId,
                btnPasteUserId,
                btnCopyAccountId,
                btnPasteAccountId,
                btnOK,
                btnCancel;
    QFontMetrics ftmUserId(ledUserId.font());
    // Forces the resize of the dialog to something more adequate by ...
    // ... setting the minimum width of one of its QLineEdit widgets ...
    // ... to the minimum string size the widget should hold, so the ...
    // ... contents can be completely seen without having to scroll.
    iMinEditWidth=ftmUserId.horizontalAdvance(QStringLiteral("W"))*MIN_USER_ID_LENGTH;
    ledUserId.setMinimumWidth(iMinEditWidth);
    ledSessionId.setSizePolicy(
        QSizePolicy::Policy::Expanding,
        QSizePolicy::Policy::Preferred
    );
    ledDeviceId.setSizePolicy(
        QSizePolicy::Policy::Expanding,
        QSizePolicy::Policy::Preferred
    );
    ledUserId.setSizePolicy(
        QSizePolicy::Policy::Expanding,
        QSizePolicy::Policy::Preferred
    );
    ledAccountId.setSizePolicy(
        QSizePolicy::Policy::Expanding,
        QSizePolicy::Policy::Preferred
    );
    ledSessionId.setText(sSessionId);
    ledDeviceId.setText(sDeviceId);
    ledUserId.setText(sUserId);
    ledAccountId.setText(sAccountId);
    ledSessionId.setToolTip(
        QStringLiteral(
            "Found in:\n"
            "-Cookies as 'session'"
        )
    );
    ledDeviceId.setToolTip(
        QStringLiteral(
            "Found in:\n"
            "-Request cookies as 'device_id'\n"
            "-SERVER_APP_STARTUP request as 'open_udid.'"
        )
    );
    ledUserId.setToolTip(
        QStringLiteral(
            "Found in:\n"
            "-Cookies as 'uid', 'HDR-X-User-id' or 'X-User-id'\n"
            "-SERVER_APP_STARTUP response as 'client_login_success.user_info.user_id'\n"
            "-SERVER_APP_STARTUP response as 'user.user_id'"
        )
    );
    ledAccountId.setToolTip(
        QStringLiteral(
            "Found in:\n"
            "-SERVER_APP_STARTUP response as 'client_login_success.encrypted_user_id'"
        )
    );
    btnCopySessionId.setIcon(QIcon(QStringLiteral(":/img/tool-copy.png")));
    btnPasteSessionId.setIcon(QIcon(QStringLiteral(":/img/tool-paste.png")));
    btnCopyDeviceId.setIcon(QIcon(QStringLiteral(":/img/tool-copy.png")));
    btnPasteDeviceId.setIcon(QIcon(QStringLiteral(":/img/tool-paste.png")));
    btnCopyUserId.setIcon(QIcon(QStringLiteral(":/img/tool-copy.png")));
    btnPasteUserId.setIcon(QIcon(QStringLiteral(":/img/tool-paste.png")));
    btnCopyAccountId.setIcon(QIcon(QStringLiteral(":/img/tool-copy.png")));
    btnPasteAccountId.setIcon(QIcon(QStringLiteral(":/img/tool-paste.png")));
    btnOK.setDefault(true);
    btnOK.setText(QStringLiteral("OK"));
    btnCancel.setText(QStringLiteral("Cancel"));
    lblWarningIcon.setPixmap(
        QApplication::style()->standardPixmap(
            QStyle::StandardPixmap::SP_MessageBoxWarning
        )
    );
    lblWarningText.setText(
        QStringLiteral(
            "<i>Warning: hitting OK will overwrite the current session and all its details,"
            " both in memory and disk.<br>"
            "So have this into account if you plan to restore the previous session later,"
            " for whatever reason.</i>"
        )
    );
    hblSession.addWidget(&ledSessionId);
    hblSession.addWidget(&btnCopySessionId);
    hblSession.addWidget(&btnPasteSessionId);
    hblDevice.addWidget(&ledDeviceId);
    hblDevice.addWidget(&btnCopyDeviceId);
    hblDevice.addWidget(&btnPasteDeviceId);
    hblUser.addWidget(&ledUserId);
    hblUser.addWidget(&btnCopyUserId);
    hblUser.addWidget(&btnPasteUserId);
    hblAccount.addWidget(&ledAccountId);
    hblAccount.addWidget(&btnCopyAccountId);
    hblAccount.addWidget(&btnPasteAccountId);
    fmlSession.addRow(QStringLiteral("Session id:"),&hblSession);
    fmlSession.addRow(QStringLiteral("Device id:"),&hblDevice);
    fmlSession.addRow(QStringLiteral("User id:"),&hblUser);
    fmlSession.addRow(QStringLiteral("Account id:"),&hblAccount);
    hblButtons.addStretch();
    hblButtons.addWidget(&btnOK);
    hblButtons.addWidget(&btnCancel);
    hblWarning.addWidget(&lblWarningIcon);
    hblWarning.addWidget(&lblWarningText);
    hblWarning.addStretch();
    vblESDetails.addLayout(&fmlSession);
    vblESDetails.addLayout(&hblButtons);
    vblESDetails.addLayout(&hblWarning);
    dlgESDetails.setLayout(&vblESDetails);
    dlgESDetails.setWindowFlag(Qt::WindowType::MSWindowsFixedSizeDialogHint);
    dlgESDetails.setWindowModality(Qt::WindowModality::ApplicationModal);
    dlgESDetails.setWindowTitle(QStringLiteral("Edit session details"));
    auto fnCopy=[](QLineEdit *ledText) {
        QString sText=ledText->text().trimmed();
        if(!sText.isEmpty())
            QApplication::clipboard()->setText(sText);
    };
    auto fnPaste=[](QLineEdit *ledText) {
        QClipboard *clpBoard=QApplication::clipboard();
        if(clpBoard->mimeData()->hasText()) {
            ledText->setText(clpBoard->text().trimmed());
            ledText->selectAll();
            ledText->setFocus();
        }
    };
    connect(
        &btnCopySessionId,
        &QPushButton::clicked,
        [&](bool) {
            fnCopy(&ledSessionId);
        }
    );
    connect(
        &btnPasteSessionId,
        &QPushButton::clicked,
        [&](bool) {
            fnPaste(&ledSessionId);
        }
    );
    connect(
        &btnCopyDeviceId,
        &QPushButton::clicked,
        [&](bool) {
            fnCopy(&ledDeviceId);
        }
    );
    connect(
        &btnPasteDeviceId,
        &QPushButton::clicked,
        [&](bool) {
            fnPaste(&ledDeviceId);
        }
    );
    connect(
        &btnCopyUserId,
        &QPushButton::clicked,
        [&](bool) {
            fnCopy(&ledUserId);
        }
    );
    connect(
        &btnPasteUserId,
        &QPushButton::clicked,
        [&](bool) {
            fnPaste(&ledUserId);
        }
    );
    connect(
        &btnCopyAccountId,
        &QPushButton::clicked,
        [&](bool) {
            fnCopy(&ledAccountId);
        }
    );
    connect(
        &btnPasteAccountId,
        &QPushButton::clicked,
        [&](bool) {
            fnPaste(&ledAccountId);
        }
    );
    connect(
        &btnOK,
        &QPushButton::clicked,
        [&]() {
            QString sNewSessionId=ledSessionId.text().trimmed(),
                    sNewDeviceId=ledDeviceId.text().trimmed(),
                    sNewUserId=ledUserId.text().trimmed(),
                    sNewAccountId=ledAccountId.text().trimmed();
            if(!BadooWrapper::isValidSessionId(sNewSessionId)) {
                QMessageBox::critical(
                    this,
                    QStringLiteral("Error"),
                    QStringLiteral("Invalid Session id")
                );
                ledSessionId.selectAll();
                ledSessionId.setFocus();
            }
            else if(!BadooWrapper::isValidDeviceId(sNewDeviceId)) {
                QMessageBox::critical(
                    this,
                    QStringLiteral("Error"),
                    QStringLiteral("Invalid Device id")
                );
                ledDeviceId.selectAll();
                ledDeviceId.setFocus();
            }
            else if(!BadooWrapper::isValidUserId(sNewUserId)) {
                QMessageBox::critical(
                    this,
                    QStringLiteral("Error"),
                    QStringLiteral("Invalid User id")
                );
                ledUserId.selectAll();
                ledUserId.setFocus();
            }
            else if(!BadooWrapper::isValidAccountId(sNewAccountId)) {
                QMessageBox::critical(
                    this,
                    QStringLiteral("Error"),
                    QStringLiteral("Invalid Account id")
                );
                ledAccountId.selectAll();
                ledAccountId.setFocus();
            }
            else {
                sSessionId=sNewSessionId;
                sDeviceId=sNewDeviceId;
                sUserId=sNewUserId;
                sAccountId=sNewAccountId;
                dlgESDetails.accept();
            }
        }
    );
    connect(
        &btnCancel,
        &QPushButton::clicked,
        &dlgESDetails,
        &QDialog::reject
    );
    ledSessionId.selectAll();
    bResult=QDialog::DialogCode::Accepted==dlgESDetails.exec();
    return bResult;
}

QHash<BadooFolderType,QString>      hshBadooFolderNames;
QHash<BadooListSectionType,QString> hshBadooSectionNames;
QHash<BadooListFilter,QString>      hshBadooFilterNames;

void getCustomFolderParameters_Helper() {
#define ADD_CONSTANT_TO_HASH(h,c) (h.insert(c,QStringLiteral(#c)))
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,ALL_MESSAGES);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FRIENDS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FAVOURITES);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,WANT_TO_MEET_YOU);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,YOU_WANT_TO_MEET);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,PROFILE_VISITORS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,BLOCKED);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,NEARBY_PEOPLE);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,PRIVATE_ALBUM_ACCESS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,SPOTLIGHT);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,MATCHES);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,NEARBY_PEOPLE_4);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,RATED_ME);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,PROFILE_SEARCH);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,NEARBY_PEOPLE_WEB);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,VERIFICATION_ACCESS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_COMBINED_CONNECTIONS_ALL);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_BELL);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_LIVESTREAMERS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_LIVESTREAM_SUBSCRIPTIONS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_LIVESTREAM_FOLLOWERS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_LIVESTREAM_VIEWERS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_CHAT_REQUEST_LIST);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_JOINED_GROUP_CHATS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_AVAILABLE_GROUP_CHATS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_NO_VOTES);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_GROUP_CHAT_MEMBERS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_MATCH_BAR);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_ARCHIVED);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_FAVOURITED_ME);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_FAVOURITED_BY_ME);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_ACTIVITY);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_CONVERSATIONS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_CRUSHES);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_MESSAGES_AND_ACTIVITY);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_BEST_BETS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_BFF_CONTACTS);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_DISCOVER_FOR_YOU);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_COMPATIBLE_PEOPLE);
    ADD_CONSTANT_TO_HASH(hshBadooFolderNames,FOLDER_TYPE_COMPLIMENTS);
    for(const auto &k:hshBadooFolderNames.keys())
        hshBadooFolderNames[k].remove(QStringLiteral("FOLDER_TYPE_"));
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_UNKNOWN);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_GENERAL);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_TEMPORAL_MATCH);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_BUMPED_INTO);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_ALL_MESSAGES);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_FAVORITES);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_FAVORITED_YOU);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_FAVORITES_MUTUAL);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_WANT_TO_MEET_YOU_UNVOTED);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_WANT_TO_MEET_YOU_MUTUAL);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_WANT_TO_MEET_YOU_REJECTED);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_PROFILE_VISITORS);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_BLOCKED);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_PRIVATE_ALBUM_ACCESS);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_SPOTLIGHT);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_PROFILE_SEARCH);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_COMMON_PLACE);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_PEOPLE_NEARBY);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_FRIENDS);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_MESSENGER_MINI_GAME);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_COMBINED_CONNECTIONS_ALL);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_TOP_LIVESTREAMERS);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_POPULAR_LIVESTREAMERS);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_TOP_SPENDERS);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_BEELINE_IN_RANGE);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_BEELINE_OUT_OF_RANGE);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_LIVESTREAM_FOLLOWING);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_LOCKED);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_ACTIVITY);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_ACTIVITY_HIGHLIGHTS);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_BEELINE_ALL);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_BEELINE_NEW);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_BEELINE_NEARBY);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_BEELINE_RECENTLY_ACTIVE);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_PNB_CLIPS);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_COLLECTION);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_CONNECTIONS_CHAT_REQUESTS);
    ADD_CONSTANT_TO_HASH(hshBadooSectionNames,LIST_SECTION_TYPE_FREEMIUM);
    for(const auto &k:hshBadooSectionNames.keys())
        hshBadooSectionNames[k].remove(QStringLiteral("LIST_SECTION_TYPE_"));
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_ONLINE);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_NEW);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_UNREAD);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_CONVERSATIONS);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_ACTIVE_USERS);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_NEARBY);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_FAVOURITES);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_MATCHED);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_MATCH_MODE);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_UNREPLIED);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_MESSENGER_MINI_GAME);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_HOT);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_STREAMING);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_ENCOUNTERS);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_MOST_COMPATIBLE);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_ALIGNED_INTENTIONS);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_ALIGNED_LIFESTYLE);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_ALIGNED_MUSIC);
    ADD_CONSTANT_TO_HASH(hshBadooFilterNames,LIST_FILTER_ALIGNED_LANGUAGE);
    for(const auto &k:hshBadooFilterNames.keys())
        hshBadooFilterNames[k].remove(QStringLiteral("LIST_FILTER_"));
}

bool MainWindow::getCustomFolderParameters(BadooFolderType      &bftFolder,
                                           BadooListSectionType &blstSection,
                                           BadooListFilterList  &blflFilters,
                                           int                  &iMediaCount) {
    static QHash<BadooListFilter,boolean> hCarriedFilterValues={};
    static int  iCarriedFolderType=0,
                iCarriedSectionType=0,
                iCarriedMediaCount=-0;
    bool        bResult=false;
    QDialog     dlgCFParams(this);
    QVBoxLayout vblCFParams;
    QFormLayout fmlFolder;
    QGridLayout grlFilter;
    QHBoxLayout hblFolder,
                hblSection,
                hblMediaCount,
                hblButtons;
    QSpinBox    spbFolder,
                spbSection,
                spbMediaCount;
    QLabel      lblFilter;
    QComboBox   cboFolder,
                cboSection,
                cboMediaCount;
    QWidgetList wglFilters;
    QPushButton btnOK,
                btnCancel;
    getCustomFolderParameters_Helper();
    // Configures the Folder spinner.
    spbFolder.setRange(0,99);
    spbFolder.setAlignment(Qt::AlignmentFlag::AlignRight);
    spbFolder.setSizePolicy(
        QSizePolicy::Policy::Fixed,
        QSizePolicy::Policy::Preferred
    );
    connect(
        &spbFolder,
        &QSpinBox::valueChanged,
        [&](int iV) {
            cboFolder.blockSignals(true);
            cboFolder.setCurrentIndex(cboFolder.findData(iV));
            cboFolder.blockSignals(false);
        }
    );
    // Configures the Section spinner.
    spbSection.setRange(0,99);
    spbSection.setAlignment(Qt::AlignmentFlag::AlignRight);
    spbSection.setSizePolicy(
        QSizePolicy::Policy::Fixed,
        QSizePolicy::Policy::Preferred
    );
    connect(
        &spbSection,
        &QSpinBox::valueChanged,
        [&](int iV) {
            cboSection.blockSignals(true);
            cboSection.setCurrentIndex(cboSection.findData(iV));
            cboSection.blockSignals(false);
        }
    );
    // Configures the Media Count spinner.
    spbMediaCount.setRange(0,100);
    spbMediaCount.setAlignment(Qt::AlignmentFlag::AlignRight);
    spbMediaCount.setSizePolicy(
        QSizePolicy::Policy::Fixed,
        QSizePolicy::Policy::Preferred
    );
    connect(
        &spbMediaCount,
        &QSpinBox::valueChanged,
        [&](int iV) {
            cboMediaCount.blockSignals(true);
            cboMediaCount.setCurrentIndex(cboMediaCount.findData(iV));
            cboMediaCount.blockSignals(false);
        }
    );
    spbFolder.setMinimumWidth(spbMediaCount.sizeHint().width());
    spbSection.setMinimumWidth(spbMediaCount.sizeHint().width());
    // Configures the Folder combo-box.
    auto folderKeys=hshBadooFolderNames.keys();
    std::sort(folderKeys.begin(),folderKeys.end());
    for(const auto &k:folderKeys) {
        if(FOLDER_TYPE_COMBINED_CONNECTIONS_ALL==k) {
            auto *simModel=qobject_cast<QStandardItemModel *>(cboFolder.model());
            cboFolder.addItem(QStringLiteral("------- Extended folder types -------"),-1);
            simModel->item(cboFolder.count()-1,0)->setFlags(Qt::ItemFlag::NoItemFlags);
        }
        cboFolder.addItem(hshBadooFolderNames.value(k),k);
    }
    cboFolder.setPlaceholderText(QStringLiteral("Folder type shortcuts"));
    cboFolder.setSizePolicy(
        QSizePolicy::Policy::Expanding,
        QSizePolicy::Policy::Preferred
    );
    connect(
        &cboFolder,
        &QComboBox::currentIndexChanged,
        [&](int) {
            spbFolder.blockSignals(true);
            spbFolder.setValue(cboFolder.currentData().toInt());
            spbFolder.blockSignals(false);
        }
    );
    // Configures the Section combo-box.
    auto sectionKeys=hshBadooSectionNames.keys();
    std::sort(sectionKeys.begin(),sectionKeys.end());
    for(const auto &k:sectionKeys)
        cboSection.addItem(hshBadooSectionNames.value(k),k);
    cboSection.setPlaceholderText(QStringLiteral("Section type shortcuts"));
    cboSection.setSizePolicy(
        QSizePolicy::Policy::Expanding,
        QSizePolicy::Policy::Preferred
    );
    connect(
        &cboSection,
        &QComboBox::currentIndexChanged,
        [&](int) {
            spbSection.blockSignals(true);
            spbSection.setValue(cboSection.currentData().toInt());
            spbSection.blockSignals(false);
        }
    );
    // Configures the Media Count combo-box.
    cboMediaCount.addItem(QStringLiteral("All available images and videos"),0);
    cboMediaCount.addItem(QStringLiteral("Only one image / one video per profile"),1);
    cboMediaCount.addItem(QStringLiteral("Up to 10 images / 10 videos per profile"),10);
    cboMediaCount.addItem(QStringLiteral("Up to 100 images / 100 videos per profile"),100);
    cboMediaCount.setPlaceholderText(QStringLiteral("Media count shortcuts"));
    cboMediaCount.setSizePolicy(
        QSizePolicy::Policy::Expanding,
        QSizePolicy::Policy::Preferred
    );
    connect(
        &cboMediaCount,
        &QComboBox::currentIndexChanged,
        [&](int) {
            spbMediaCount.blockSignals(true);
            spbMediaCount.setValue(cboMediaCount.currentData().toInt());
            spbMediaCount.blockSignals(false);
        }
    );
    // Configures the Filter check-boxes.
    lblFilter.setText(QStringLiteral("Choose filters:"));
    auto filterKeys=hshBadooFilterNames.keys();
    std::sort(filterKeys.begin(),filterKeys.end());
    for(int iK=0;iK<filterKeys.count();iK++) {
        int       iRow=iK/3,
                  iCol=iK%3;
        QCheckBox *chkFilter=new QCheckBox(hshBadooFilterNames.value(filterKeys.at(iK)));
        if(hCarriedFilterValues.contains(filterKeys.at(iK)))
            chkFilter->setChecked(hCarriedFilterValues.value(filterKeys.at(iK)));
        wglFilters.append(chkFilter);
        grlFilter.addWidget(wglFilters.at(iK),iRow,iCol);
    }
    // Completes the dialog design.
    btnOK.setText(QStringLiteral("OK"));
    btnCancel.setText(QStringLiteral("Cancel"));
    hblFolder.addWidget(&spbFolder);
    hblFolder.addWidget(&cboFolder);
    hblSection.addWidget(&spbSection);
    hblSection.addWidget(&cboSection);
    hblMediaCount.addWidget(&spbMediaCount);
    hblMediaCount.addWidget(&cboMediaCount);
    fmlFolder.addRow(QStringLiteral("Folder type [0-99]:"),&hblFolder);
    fmlFolder.addRow(QStringLiteral("Section type [0-99]:"),&hblSection);
    fmlFolder.addRow(QStringLiteral("Media count [0-100]:"),&hblMediaCount);
    hblButtons.addStretch();
    hblButtons.addWidget(&btnOK);
    hblButtons.addWidget(&btnCancel);
    vblCFParams.addLayout(&fmlFolder);
    vblCFParams.addWidget(&lblFilter,0,Qt::AlignmentFlag::AlignHCenter);
    vblCFParams.addLayout(&grlFilter);
    vblCFParams.addLayout(&hblButtons);
    dlgCFParams.setLayout(&vblCFParams);
    dlgCFParams.setWindowFlag(Qt::WindowType::MSWindowsFixedSizeDialogHint);
    dlgCFParams.setWindowModality(Qt::WindowModality::ApplicationModal);
    dlgCFParams.setWindowTitle(QStringLiteral("Custom folder parameters"));
    connect(
        &btnOK,
        &QPushButton::clicked,
        &dlgCFParams,
        &QDialog::accept
    );
    connect(
        &btnCancel,
        &QPushButton::clicked,
        &dlgCFParams,
        &QDialog::reject
    );
    spbFolder.setValue(iCarriedFolderType);
    spbSection.setValue(iCarriedSectionType);
    spbMediaCount.setValue(iCarriedMediaCount);
    spbFolder.selectAll();
    if(QDialog::DialogCode::Accepted==dlgCFParams.exec()) {
        iCarriedFolderType=spbFolder.value();
        iCarriedSectionType=spbSection.value();
        iCarriedMediaCount=spbMediaCount.value();
        blflFilters.clear();
        for(int iK=0;iK<filterKeys.count();iK++) {
            QCheckBox *chkFilter=qobject_cast<QCheckBox *>(wglFilters[iK]);
            hCarriedFilterValues[filterKeys.at(iK)]=chkFilter->isChecked();
            if(chkFilter->isChecked())
                blflFilters.append(filterKeys.at(iK));
        }
        bftFolder=static_cast<BadooFolderType>(iCarriedFolderType);
        blstSection=static_cast<BadooListSectionType>(iCarriedSectionType);
        iMediaCount=iCarriedMediaCount;
        bResult=true;
    }
    while(!wglFilters.isEmpty())
        delete wglFilters.takeFirst();
    return bResult;
}

bool MainWindow::postInit() {
    int        iButton;
    QString    sError;
    QByteArray abtPhotoFail,
               abtVideoFail;
    // Loads the default media files, to be shown on download errors.
    BadooAPI::getFullFileContents(QStringLiteral(":img/photo-failure.png"),abtPhotoFail);
    BadooAPI::getFullFileContents(QStringLiteral(":img/video-failure.mp4"),abtVideoFail);
    bwMain.setDefaultMedia(abtPhotoFail,abtVideoFail);
    // Initializes the DB.
    do {
        if(this->setupDB(sError)) {
            iButton=QMessageBox::StandardButton::NoButton;
            break;
        }
        else {
            iButton=QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                sError,
                QMessageBox::StandardButton::Abort|
                QMessageBox::StandardButton::Retry|
                QMessageBox::StandardButton::Ignore,
                QMessageBox::StandardButton::Retry
            );
            if(QMessageBox::StandardButton::Abort==iButton)
                break;
            else if(QMessageBox::StandardButton::Ignore==iButton) {
                QMessageBox::information(
                    this,
                    QStringLiteral(APP_TITLE),
                    QStringLiteral("The program will continue now,\n"
                                   "but it's not gonna be possible\n"
                                   "to save the interface settings\n"
                                   "nor the logged-in session.")
                );
                break;
            }
        }
    } while(true);
    if(QMessageBox::StandardButton::Abort==iButton)
        return false;
    this->resize(800,600);
    WidgetGeometry wggGeometry;
    // Restores the interface settings, when available.
    if(dbMain->loadSetting(stgMAIN,wggGeometry))
        if(!wggGeometry.recRect.isNull()) {
            this->setGeometry(wggGeometry.recRect);
            switch(wggGeometry.wgsStatus) {
                case wgsNORMAL:
                    this->showNormal();
                    break;
                case wgsMINIMIZED:
                    this->showMinimized();
                    break;
                case wgsMAXIMIZED:
                    this->showMaximized();
                    break;
            }
        }
    QString sSes,sDev,sUsr,sAcc;
    // Restores the saved session, when available.
    if(dbMain->loadSession(sSes,sDev,sUsr,sAcc)) {
        bwMain.setSessionDetails(sSes,sDev,sUsr,sAcc);
        if(bwMain.isLoggedIn()) {
            do {
                if(bwMain.loginBack()) {
                    iButton=QMessageBox::StandardButton::NoButton;
                    break;
                }
                else {
                    iButton=QMessageBox::critical(
                        this,
                        QStringLiteral("Error"),
                        bwMain.getLastError(),
                        QMessageBox::StandardButton::Abort|
                        QMessageBox::StandardButton::Retry|
                        QMessageBox::StandardButton::Ignore,
                        QMessageBox::StandardButton::Retry
                    );
                    if(QMessageBox::StandardButton::Abort==iButton)
                        break;
                    else if(QMessageBox::StandardButton::Ignore==iButton) {
                        QMessageBox::information(
                            this,
                            QStringLiteral(APP_TITLE),
                            QStringLiteral("The program will continue now,\n"
                                           "but the saved session will be\n"
                                           "cleared, so you need to log in\n"
                                           "again.")
                        );
                        break;
                    }
                }
            } while(true);
            if(QMessageBox::StandardButton::Abort==iButton)
                return false;
            else if(QMessageBox::StandardButton::Ignore==iButton) {
                if(bwMain.logout())
                    dbMain->saveSession(QString(),QString(),QString(),QString());
                // Clears the session even if the API or DB operations failed.
                bwMain.clearSessionDetails();
            }
        }
    }
    // Initializes the variable menu icons and captions.
    this->testAndSetLocation();
    this->testAndSetVisibility();
    if(bwMain.isLoggedIn())
        ui->stbMain->showMessage(QStringLiteral("Logged in"));
    return true;
}

bool MainWindow::setupDB(QString &sError) {
    bool    bResult=false;
    QString sFilename,
            sLocation=QStandardPaths::standardLocations(
                QStandardPaths::StandardLocation::AppLocalDataLocation
            ).first();
    QDir    dirDB(sLocation);
    sError.clear();
    if(!dirDB.exists())
        dirDB.mkpath(QStringLiteral("."));
    sFilename=QStringLiteral("%1/%2").arg(sLocation,QStringLiteral(DB_NAME));
    dbMain->setFilename(sFilename);
    if(dbMain->exists())
        bResult=true;
    else if(dbMain->create())
        bResult=true;
    else {
        (void)QDir().remove(sFilename);
        sError=QStringLiteral("Unable to create the DB:\n%1").
               arg(dbMain->getLastError());
    }
    if(bResult)
        if(!dbMain->open()) {
            sError=QStringLiteral("Unable to open the DB:\n%1").
                   arg(dbMain->getLastError());
            bResult=false;
        }
    return bResult;
}

void MainWindow::showChildDialog(SettingsGroup stgGroup,
                                 QDialog       *dlgChild) {
    QMdiSubWindow  *mdsChild;
    WidgetGeometry wggGeometry;
    // Identifies the dialog with its settings group through a custom property.
    dlgChild->setProperty(QStringLiteral("group").toUtf8(),stgGroup);
    dlgChild->setWindowIcon(QIcon(QStringLiteral(":img/bw.ico")));
    mdsChild=mdiArea.addSubWindow(dlgChild);
    // Restores the dialog status and geometry, if possible.
    if(dbMain->loadSetting(stgGroup,wggGeometry))
        if(!wggGeometry.recRect.isNull()) {
            mdsChild->setGeometry(wggGeometry.recRect);
            switch(wggGeometry.wgsStatus) {
                case wgsNORMAL:
                    mdsChild->showNormal();
                    break;
                case wgsMINIMIZED:
                    mdsChild->showMinimized();
                    break;
                case wgsMAXIMIZED:
                    mdsChild->showMaximized();
                    break;
            }
        }
    // There seems to be a problem when a QDialog belongs to a QMdiArea and ...
    // ... QDialog::reject() is invoked by pressing the Escape key, causing ...
    // ... that its QMdiSubWindow remains visible. So, as a straightforward ...
    // ... fix, we can call QMdiArea::closeActiveSubWindow() to close it by ...
    // ... force as soon as the dialog finishes.
    connect(
        dlgChild,
        &QDialog::finished,
        &mdiArea,
        &QMdiArea::closeActiveSubWindow
    );
    dlgChild->installEventFilter(this);
    dlgChild->show();
}

void MainWindow::showCustomProfile(QString sProfileId) {
    BadooUserProfile  bupProfile;
    MediaContentsHash mchPhotoContents,
                      mchVideoContents;
    if(bwMain.getProfile(sProfileId,bupProfile))
        if(bwMain.downloadMultiProfileResources<QByteArray>(
            {bupProfile},
            mchPhotoContents,
            mchVideoContents
        )) {
            // The first message of every profile needs to be retrieved separately ...
            // ... because, for some reason, the SERVER_GET_USER response does not ...
            // ... come with the last_message object, unlike SERVER_GET_USER_LIST.
            // It seems that there's nothing that can be done to fix this specific ...
            // ... behavior. However, the next call to getChatMessages() completes ...
            // ... the profile details quickly and effortlessly.
            BadooChatMessageList bcmlChat;
            if(bwMain.getChatMessages(sProfileId,bcmlChat,1))
                if(!bcmlChat.isEmpty())
                    bupProfile.bcmLastMessage=bcmlChat.first();
            int           iActionButtons;
            QDialog       *dlgProfile=new QDialog(this);
            QStatusBar    *stbProfile=new QStatusBar(this);
            QVBoxLayout   *vblProfile=new QVBoxLayout(dlgProfile);
            ProfileViewer *pvProfile=new ProfileViewer(&bwMain,dlgProfile);
            vblProfile->setContentsMargins(QMargins());
            vblProfile->addWidget(pvProfile);
            vblProfile->addWidget(stbProfile);
            dlgProfile->setGeometry(pvProfile->geometry());
            dlgProfile->setLayout(vblProfile);
            dlgProfile->setWindowFlag(Qt::WindowType::WindowMinMaxButtonsHint);
            dlgProfile->setWindowModality(Qt::WindowModality::ApplicationModal);
            dlgProfile->setWindowTitle(QStringLiteral("View profile"));
            iActionButtons=PROFILE_VIEWER_BUTTON_COPY_URL|
                           PROFILE_VIEWER_BUTTON_DOWNLOAD|
                           PROFILE_VIEWER_BUTTON_NOPE|
                           PROFILE_VIEWER_BUTTON_LIKE;
            if(!BadooWrapper::isEncryptedUserId(sProfileId))
                iActionButtons|=PROFILE_VIEWER_BUTTON_FAVORITE;
            pvProfile->setActiveActionButtons(iActionButtons);
            pvProfile->load(
                bupProfile,
                mchPhotoContents.value(sProfileId),
                mchVideoContents.value(sProfileId)
            );
            connect(
                pvProfile,
                &ProfileViewer::badgeClicked,
                [&](BadgeAction baAction) {
                    bool           bUpdateProfile=false;
                    QString        sMessage=QString();
                    SessionDetails sdDetails;
                    switch(baAction) {
                        case BADGE_ACTION_FAST_MESSAGE:
                            sMessage=QStringLiteral("Message sent");
                            // Updates the current profile's last message by force, ...
                            // ... so there is no need of calling the API to update ...
                            // ... these details. -Notice that we're only modifying ...
                            // ... the minimum required that actually cause changes ...
                            // ... in the UI, but this may vary in the future-.
                            bwMain.getSessionDetails(sdDetails);
                            bupProfile.bHasQuickChat=false;
                            bupProfile.bcmLastMessage.sFromUserId=sdDetails.sUserId;
                            bupProfile.bcmLastMessage.sToUserId=bupProfile.sUserId;
                            bUpdateProfile=true;
                            break;
                        case BADGE_ACTION_OPEN_CHAT:
                            break;
                    }
                    if(sMessage.isEmpty())
                        stbProfile->clearMessage();
                    else
                        stbProfile->showMessage(sMessage);
                    if(bUpdateProfile)
                        pvProfile->load(
                            bupProfile,
                            mchPhotoContents.value(sProfileId),
                            mchVideoContents.value(sProfileId)
                        );
                }
            );
            connect(
                pvProfile,
                &ProfileViewer::buttonClicked,
                [&](ProfileViewerButton pvbButton) {
                    bool    bUpdateProfile=false;
                    QString sMessage=QString();
                    switch(pvbButton) {
                        case PROFILE_VIEWER_BUTTON_COPY_URL:
                            sMessage=QStringLiteral("Profile URL copied to clipboard");
                            break;
                        case PROFILE_VIEWER_BUTTON_DOWNLOAD:
                            sMessage=QStringLiteral("Profile saved to disk");
                            break;
                        case PROFILE_VIEWER_BUTTON_BACK:
                            break;
                        case PROFILE_VIEWER_BUTTON_NOPE:
                            bupProfile.bvMyVote=VOTE_NO;
                            bUpdateProfile=true;
                            break;
                        case PROFILE_VIEWER_BUTTON_FAVORITE:
                            bupProfile.bIsFavorite=!bupProfile.bIsFavorite;
                            bUpdateProfile=true;
                            break;
                        case PROFILE_VIEWER_BUTTON_LIKE:
                            bupProfile.bvMyVote=VOTE_YES;
                            bUpdateProfile=true;
                            if(VOTE_YES==bupProfile.bvTheirVote) {
                                bupProfile.bIsMatch=true;
                                if(!bupProfile.bHasConversation)
                                    bupProfile.bHasQuickChat=true;
                            }
                            break;
                        case PROFILE_VIEWER_BUTTON_SKIP:
                            break;
                        case PROFILE_VIEWER_BUTTON_BLOCK:
                            sMessage=QStringLiteral("Profile blocked");
                            bupProfile.bIsBlocked=true;
                            bUpdateProfile=true;
                            break;
                        case PROFILE_VIEWER_BUTTON_UNBLOCK:
                            sMessage=QStringLiteral("Profile unblocked");
                            bupProfile.bIsBlocked=false;
                            bUpdateProfile=true;
                            break;
                        case PROFILE_VIEWER_BUTTON_UNMATCH:
                            sMessage=QStringLiteral("Profile unmatched");
                            bupProfile.bIsMatch=false;
                            bUpdateProfile=true;
                            break;
                    }
                    if(sMessage.isEmpty())
                        stbProfile->clearMessage();
                    else
                        stbProfile->showMessage(sMessage);
                    if(bUpdateProfile)
                        pvProfile->load(
                            bupProfile,
                            mchPhotoContents.value(sProfileId),
                            mchVideoContents.value(sProfileId)
                        );
                }
            );
            WidgetGeometry wggGeometry;
            if(dbMain->loadSetting(stgPROFILE,wggGeometry))
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
            if(dlgProfile->isMaximized())
                wggGeometry.wgsStatus=wgsMAXIMIZED;
            else if(dlgProfile->isMinimized())
                wggGeometry.wgsStatus=wgsMINIMIZED;
            else
                wggGeometry.wgsStatus=wgsNORMAL;
            wggGeometry.recRect=dlgProfile->normalGeometry();
            dbMain->saveSetting(stgPROFILE,wggGeometry);
            delete pvProfile;
            delete vblProfile;
            delete stbProfile;
            delete dlgProfile;
        }
        else
            QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                bwMain.getLastError()
            );
    else
        QMessageBox::critical(
            this,
            QStringLiteral("Error"),
            bwMain.getLastError()
        );
    return;
}

void MainWindow::showSettings(BadooSettingsContextType bsctContext) {
    if(bwMain.isLoggedIn())
        if(bwMain.loadSearchSettings()) {
            if(bwMain.showSearchSettingsDialog(bsctContext,this))
                ui->stbMain->showMessage(QStringLiteral("Settings saved"));
        }
        else
            QMessageBox::critical(
                this,
                QStringLiteral("Error"),
                bwMain.getLastError()
            );
    else
        QMessageBox::critical(
            this,
            QStringLiteral("Error"),
            QStringLiteral("Not logged in")
        );
}

void MainWindow::testAndSetLocation(bool bChange) {
    ui->actLocation->setText(QStringLiteral("Current location: UNKNOWN"));
    if(bwMain.isLoggedIn()) {
        bool bTestOK=false,
             bSetOK=false;
        do {
            if(bwMain.loadOwnProfile()) {
                bTestOK=true;
                break;
            }
            else {
                int iButton=QMessageBox::critical(
                    this,
                    QStringLiteral("Error"),
                    bwMain.getLastError(),
                    QMessageBox::StandardButton::Retry|
                    QMessageBox::StandardButton::Ignore,
                    QMessageBox::StandardButton::Retry
                );
                if(QMessageBox::StandardButton::Ignore==iButton)
                    break;
            }
        } while(true);
        if(bTestOK)
            if(bChange)
                if(bwMain.showLocationDialog(this))
                    do {
                        if(bwMain.loadOwnProfile()) {
                            bSetOK=true;
                            break;
                        }
                        else {
                            int iButton=QMessageBox::critical(
                                this,
                                QStringLiteral("Error"),
                                bwMain.getLastError(),
                                QMessageBox::StandardButton::Retry|
                                QMessageBox::StandardButton::Ignore,
                                QMessageBox::StandardButton::Retry
                            );
                            if(QMessageBox::StandardButton::Ignore==iButton)
                                break;
                        }
                    } while(true);
        if(bTestOK) {
            BadooUserProfile bupUser;
            (void)bwMain.getLoggedInProfile(bupUser);
            if(bChange)
                if(bSetOK)
                    QMessageBox::information(
                        this,
                        QStringLiteral(APP_TITLE),
                        QStringLiteral("You are now in %1.").arg(bupUser.sCity)
                    );
            ui->actLocation->setText(
                QStringLiteral("Current location: %1").arg(bupUser.sCity)
            );
        }
    }
    else if(bChange)
        QMessageBox::critical(
            this,
            QStringLiteral("Error"),
            QStringLiteral("Not logged in")
        );
}

void MainWindow::testAndSetVisibility(bool bToggle) {
    ui->actVisibility->setText(QStringLiteral("Online status: OFFLINE"));
    ui->actVisibility->setIcon(
        QIcon(QStringLiteral(":img/badge-offline.svg"))
    );
    if(bwMain.isLoggedIn()) {
        bool bTestOK=false,
             bSetOK=false,
             bIsVisible=false;
        do {
            if(bwMain.getVisibleStatus(bIsVisible)) {
                bTestOK=true;
                break;
            }
            else {
                int iButton=QMessageBox::critical(
                    this,
                    QStringLiteral("Error"),
                    bwMain.getLastError(),
                    QMessageBox::StandardButton::Retry|
                    QMessageBox::StandardButton::Ignore,
                    QMessageBox::StandardButton::Retry
                );
                if(QMessageBox::StandardButton::Ignore==iButton)
                    break;
            }
        } while(true);
        if(bTestOK) {
            if(bToggle) {
                do {
                    if(bwMain.setVisibleStatus(!bIsVisible)) {
                        bSetOK=true;
                        break;
                    }
                    else {
                        int iButton=QMessageBox::critical(
                            this,
                            QStringLiteral("Error"),
                            bwMain.getLastError(),
                            QMessageBox::StandardButton::Retry|
                            QMessageBox::StandardButton::Ignore,
                            QMessageBox::StandardButton::Retry
                        );
                        if(QMessageBox::StandardButton::Ignore==iButton)
                            break;
                    }
                } while(true);
            }
        }
        if(bTestOK) {
            if(bToggle)
                if(bSetOK) {
                    QString sVisibility;
                    bIsVisible=!bIsVisible;
                    sVisibility=bIsVisible?QStringLiteral("VISIBLE"):QStringLiteral("HIDDEN");
                    QMessageBox::information(
                        this,
                        QStringLiteral(APP_TITLE),
                        QStringLiteral("You are %1 now.").arg(sVisibility)
                    );
                }
            if(bIsVisible) {
                ui->actVisibility->setText(QStringLiteral("Online status: VISIBLE"));
                ui->actVisibility->setIcon(
                    QIcon(QStringLiteral(":img/badge-online.svg"))
                );
            }
            else {
                ui->actVisibility->setText(QStringLiteral("Online status: HIDDEN"));
                ui->actVisibility->setIcon(
                    QIcon(QStringLiteral(":img/badge-offline-hidden.svg"))
                );
            }
        }
        else {
            ui->actVisibility->setText(QStringLiteral("Online status: UNKNOWN"));
            ui->actVisibility->setIcon(
                QIcon(QStringLiteral(":img/badge-offline-unknown.svg"))
            );
        }
    }
    else if(bToggle)
        QMessageBox::critical(
            this,
            QStringLiteral("Error"),
            QStringLiteral("Not logged in")
        );
}
