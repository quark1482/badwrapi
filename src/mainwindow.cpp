#include "mainwindow.h"
#include "./ui_mainwindow.h"

#define APP_TITLE "Badoo API wrapper demo"

MainWindow::MainWindow(QWidget *parent):
QMainWindow(parent),ui(new Ui::MainWindow) {
    ui->setupUi(this);
    dlgBrowseCustom=nullptr;
    dlgBrowseFavorites=nullptr;
    dlgBrowseLikes=nullptr;
    dlgBrowseMatches=nullptr;
    dlgBrowsePeopleNearby=nullptr;
    dlgBrowseVisitors=nullptr;
    dlgEncounters=nullptr;
    mdiArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setCentralWidget(&mdiArea);
    this->setWindowTitle(QStringLiteral(APP_TITLE));
    do {
        connect(
            ui->actEncounters,
            &QAction::triggered,
            this,
            &MainWindow::menuEncountersTriggered
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
            ui->actExit,
            &QAction::triggered,
            this,
            &MainWindow::close
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
            this->resize(800,600);
        }
    );
}

MainWindow::~MainWindow() {
    if(nullptr!=dlgBrowseCustom)
        delete dlgBrowseCustom;
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
    else {
        if(bwMain.isLoggedIn())
            bwMain.logout();
        evnE->accept();
    }
}

void MainWindow::dialogBrowseFolderDestroyed() {
    if(dlgBrowseCustom==QObject::sender())
        dlgBrowseCustom=nullptr;
    else if(dlgBrowseFavorites==QObject::sender())
        dlgBrowseFavorites=nullptr;
    else if(dlgBrowseLikes==QObject::sender())
        dlgBrowseLikes=nullptr;
    else if(dlgBrowseMatches==QObject::sender())
        dlgBrowseMatches=nullptr;
    else if(dlgBrowsePeopleNearby==QObject::sender())
        dlgBrowsePeopleNearby=nullptr;
    else if(dlgBrowseVisitors==QObject::sender())
        dlgBrowseVisitors=nullptr;
}

void MainWindow::dialogEncountersDestroyed() {
    dlgEncounters=nullptr;
}

void MainWindow::menuBrowseFolderTriggered(bool) {
    BrowseFolderDialog *dlgFolder=nullptr;
    FolderType         ftFolder;
    FolderFilterList   fflFilters={FOLDER_FILTER_ALL};
    if(ui->actCustomFolder==QObject::sender()) {
        dlgFolder=dlgBrowseCustom;
        ftFolder=FOLDER_TYPE_UNKNOWN;
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
        fflFilters={FOLDER_FILTER_ONLINE};
    }
    else if(ui->actNewPeopleNearby==QObject::sender()) {
        dlgFolder=dlgBrowsePeopleNearby;
        ftFolder=FOLDER_TYPE_PEOPLE_NEARBY;
        fflFilters={FOLDER_FILTER_NEW};
    }
    else if(ui->actVisitors==QObject::sender()) {
        dlgFolder=dlgBrowseVisitors;
        ftFolder=FOLDER_TYPE_VISITORS;
    }
    if(bwMain.isLoggedIn())
        if(nullptr==dlgFolder) {
            if(FOLDER_TYPE_UNKNOWN==ftFolder) {
                BadooFolderType      bftFolder;
                BadooListSectionType blstSection;
                if(this->getCustomFolderParameters(bftFolder,blstSection))
                    dlgFolder=new BrowseFolderDialog(bftFolder,blstSection,&bwMain,this);
            }
            else
                dlgFolder=new BrowseFolderDialog(ftFolder,fflFilters,&bwMain,this);
            if(nullptr!=dlgFolder) {
                switch(ftFolder) {
                    case FOLDER_TYPE_UNKNOWN:
                        dlgBrowseCustom=dlgFolder;
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
                }
                connect(
                    dlgFolder,
                    &BrowseFolderDialog::statusChanged,
                    this,
                    &MainWindow::wrapperStatusChanged
                );
                connect(
                    dlgFolder,
                    &BrowseFolderDialog::destroyed,
                    this,
                    &MainWindow::dialogBrowseFolderDestroyed
                );
                // There seems to be a problem when a QDialog belongs to a QMdiArea and ...
                // ... QDialog::reject() is invoked by pressing the Escape key, causing ...
                // ... that the dialog remains visible. So, as a straightforward fix we ...
                // ... can call QMdiArea::closeActiveSubWindow() to close it by force.
                connect(
                    dlgFolder,
                    &BrowseFolderDialog::rejected,
                    &mdiArea,
                    &QMdiArea::closeActiveSubWindow
                );
                dlgFolder->setWindowIcon(QIcon(QStringLiteral(":img/bw.ico")));
                mdiArea.addSubWindow(dlgFolder)->setWindowFlag(
                    Qt::WindowType::WindowMinMaxButtonsHint
                );
                if(dlgFolder->isReady())
                    dlgFolder->exec();
                else
                    delete dlgFolder;
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
        QInputDialog idProfile;
        QHBoxLayout  hblWarning;
        QLabel       lblWarningIcon,
                     lblWarningText;
        idProfile.setLabelText(QStringLiteral("Profile URL or Id:"));
        idProfile.setWindowFlag(Qt::WindowType::MSWindowsFixedSizeDialogHint);
        idProfile.setWindowFlag(Qt::WindowType::CustomizeWindowHint);
        idProfile.setWindowFlag(Qt::WindowType::WindowSystemMenuHint,false);
        idProfile.setWindowModality(Qt::WindowModality::ApplicationModal);
        idProfile.setWindowTitle(QStringLiteral("Custom profile"));
        idProfile.show();
        lblWarningIcon.setPixmap(
            QApplication::style()->standardPixmap(
                QStyle::StandardPixmap::SP_MessageBoxWarning
            )
        );
        lblWarningText.setText(
            QStringLiteral("Warning: by browsing this profile directly, you become\n"
                           "a visitor for them, completely losing your anonymity.")
        );
        hblWarning.addWidget(&lblWarningIcon);
        hblWarning.addWidget(&lblWarningText);
        hblWarning.addStretch();
        qobject_cast<QVBoxLayout *>(idProfile.layout())->addLayout(&hblWarning);
        while(true)
            if(QDialog::DialogCode::Accepted==idProfile.exec()) {
                QString sProfile=idProfile.textValue().trimmed();
                if(sProfile.isEmpty())
                    break;
                if(sProfile.startsWith(ENDPOINT_BASE))
                    sProfile=QUrl(sProfile).
                             adjusted(QUrl::UrlFormattingOption::StripTrailingSlash).
                             fileName();
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

void MainWindow::menuEncountersTriggered(bool) {
    if(bwMain.isLoggedIn())
        if(nullptr==dlgEncounters) {
            dlgEncounters=new PlayEncountersDialog(&bwMain,this);
            connect(
                dlgEncounters,
                &PlayEncountersDialog::statusChanged,
                this,
                &MainWindow::wrapperStatusChanged
            );
            connect(
                dlgEncounters,
                &PlayEncountersDialog::destroyed,
                this,
                &MainWindow::dialogEncountersDestroyed
            );
            // There seems to be a problem when a QDialog belongs to a QMdiArea and ...
            // ... QDialog::reject() is invoked by pressing the Escape key, causing ...
            // ... that the dialog remains visible. So, as a straightforward fix we ...
            // ... call QMdiArea::closeActiveSubWindow() to close it by force.
            connect(
                dlgEncounters,
                &BrowseFolderDialog::rejected,
                &mdiArea,
                &QMdiArea::closeActiveSubWindow
            );
            dlgEncounters->setWindowIcon(QIcon(QStringLiteral(":img/bw.ico")));
            mdiArea.addSubWindow(dlgEncounters)->setWindowFlag(
                Qt::WindowType::WindowMinMaxButtonsHint
            );
            if(dlgEncounters->isReady())
                dlgEncounters->exec();
            else
                delete dlgEncounters;
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

void MainWindow::menuLoginTriggered(bool) {
    if(bwMain.isLoggedIn())
        QMessageBox::critical(
            this,
            QStringLiteral("Error"),
            QStringLiteral("Already logged in")
        );
    else
        if(bwMain.showLogin())
            ui->stbMain->showMessage(QStringLiteral("Logged in"));
}

void MainWindow::menuLogoutTriggered(bool) {
    if(bwMain.isLoggedIn())
        if(!this->anyChildrenActive())
            if(bwMain.logout())
                ui->stbMain->showMessage(QStringLiteral("Logged out"));
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

void MainWindow::wrapperStatusChanged(QString sStatus) {
    if(sStatus.isEmpty())
        ui->stbMain->clearMessage();
    else
        ui->stbMain->showMessage(sStatus);
}

bool MainWindow::anyChildrenActive() {
    // Checks for any (||) active dialog.
    return nullptr!=dlgBrowseCustom||
           nullptr!=dlgBrowseFavorites||
           nullptr!=dlgBrowseLikes||
           nullptr!=dlgBrowseMatches||
           nullptr!=dlgBrowsePeopleNearby||
           nullptr!=dlgBrowseVisitors||
           nullptr!=dlgEncounters;
}

bool MainWindow::getCustomFolderParameters(BadooFolderType      &bftFolder,
                                           BadooListSectionType &blstSection) {
    bool        bResult=false;
    QDialog     dlgCustom(this);
    QFormLayout fmlParams;
    QHBoxLayout vblButtons;
    QSpinBox    spbFolder,
                spbSection;
    QPushButton btnOK,
                btnCancel;
    spbFolder.setRange(0,99);
    spbFolder.setAlignment(Qt::AlignmentFlag::AlignRight);
    spbSection.setRange(0,99);
    spbSection.setAlignment(Qt::AlignmentFlag::AlignRight);
    btnOK.setText(QStringLiteral("OK"));
    btnCancel.setText(QStringLiteral("Cancel"));
    vblButtons.addWidget(&btnOK);
    vblButtons.addWidget(&btnCancel);
    fmlParams.addRow(QStringLiteral("Folder type [0-99]:"),&spbFolder);
    fmlParams.addRow(QStringLiteral("Section type [0-99]:"),&spbSection);
    fmlParams.addRow(QString(),&vblButtons);
    dlgCustom.setLayout(&fmlParams);
    dlgCustom.setWindowFlag(Qt::WindowType::MSWindowsFixedSizeDialogHint);
    dlgCustom.setWindowFlag(Qt::WindowType::CustomizeWindowHint);
    dlgCustom.setWindowFlag(Qt::WindowType::WindowSystemMenuHint,false);
    dlgCustom.setWindowModality(Qt::WindowModality::ApplicationModal);
    dlgCustom.setWindowTitle(QStringLiteral("Custom folder parameters"));
    connect(
        &btnOK,
        &QPushButton::clicked,
        &dlgCustom,
        &QDialog::accept
    );
    connect(
        &btnCancel,
        &QPushButton::clicked,
        &dlgCustom,
        &QDialog::reject
    );
    dlgCustom.show();
    spbFolder.selectAll();
    if(QDialog::DialogCode::Accepted==dlgCustom.exec()) {
        bftFolder=static_cast<BadooFolderType>(spbFolder.value());
        blstSection=static_cast<BadooListSectionType>(spbSection.value());
        bResult=true;
    }
    return bResult;
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
                            if(VOTE_YES==bupProfile.bvTheirVote)
                                bupProfile.bIsMatch=true;
                            break;
                        case PROFILE_VIEWER_BUTTON_SKIP:
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
            dlgProfile->exec();
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
            if(bwMain.showSearchSettings(bsctContext))
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
