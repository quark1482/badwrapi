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
    BrowseFolderDialog  *dlgFolder=nullptr;
    FolderType          ftFolder;
    BadooListFilterList blflFilters;
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
                dlgFolder->exec();
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
                dlgEncounters->exec();
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
    for(const auto &k:hshBadooFilterNames.keys())
        hshBadooFilterNames[k].remove(QStringLiteral("LIST_FILTER_"));
}

bool MainWindow::getCustomFolderParameters(BadooFolderType      &bftFolder,
                                           BadooListSectionType &blstSection,
                                           BadooListFilterList  &blflFilters,
                                           int                  &iMediaCount) {
    bool        bResult=false;
    QDialog     dlgCustom(this);
    QVBoxLayout vblCustom;
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
    spbFolder.setRange(0,99);
    spbFolder.setAlignment(Qt::AlignmentFlag::AlignRight);
    spbFolder.setSizePolicy(
        QSizePolicy::Policy::Fixed,
        QSizePolicy::Policy::Preferred
    );
    spbSection.setRange(0,99);
    spbSection.setAlignment(Qt::AlignmentFlag::AlignRight);
    spbSection.setSizePolicy(
        QSizePolicy::Policy::Fixed,
        QSizePolicy::Policy::Preferred
    );
    spbMediaCount.setRange(0,100);
    spbMediaCount.setAlignment(Qt::AlignmentFlag::AlignRight);
    spbMediaCount.setSizePolicy(
        QSizePolicy::Policy::Fixed,
        QSizePolicy::Policy::Preferred
    );
    spbFolder.setMinimumWidth(spbMediaCount.sizeHint().width());
    spbSection.setMinimumWidth(spbMediaCount.sizeHint().width());
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
    cboFolder.setCurrentIndex(-1);
    cboFolder.setPlaceholderText(QStringLiteral("Folder type shortcuts"));
    cboFolder.setSizePolicy(
        QSizePolicy::Policy::Expanding,
        QSizePolicy::Policy::Preferred
    );
    connect(
        &cboFolder,
        &QComboBox::currentIndexChanged,
        [&](int) {
            spbFolder.setValue(cboFolder.currentData().toInt());
        }
    );
    auto sectionKeys=hshBadooSectionNames.keys();
    std::sort(sectionKeys.begin(),sectionKeys.end());
    for(const auto &k:sectionKeys)
        cboSection.addItem(hshBadooSectionNames.value(k),k);
    cboSection.setCurrentIndex(-1);
    cboSection.setPlaceholderText(QStringLiteral("Section type shortcuts"));
    cboSection.setSizePolicy(
        QSizePolicy::Policy::Expanding,
        QSizePolicy::Policy::Preferred
    );
    connect(
        &cboSection,
        &QComboBox::currentIndexChanged,
        [&](int) {
            spbSection.setValue(cboSection.currentData().toInt());
        }
    );
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
            spbMediaCount.setValue(cboMediaCount.currentData().toInt());
        }
    );
    lblFilter.setText(QStringLiteral("Choose filters:"));
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
    auto filterKeys=hshBadooFilterNames.keys();
    std::sort(filterKeys.begin(),filterKeys.end());
    for(int iK=0;iK<filterKeys.count();iK++) {
        int iRow=iK/3,
            iCol=iK%3;
        wglFilters.append(new QCheckBox(hshBadooFilterNames.value(filterKeys.at(iK))));
        grlFilter.addWidget(wglFilters.at(iK),iRow,iCol);
    }
    hblButtons.addStretch();
    hblButtons.addWidget(&btnOK);
    hblButtons.addWidget(&btnCancel);
    vblCustom.addLayout(&fmlFolder);
    vblCustom.addWidget(&lblFilter,0,Qt::AlignmentFlag::AlignHCenter);
    vblCustom.addLayout(&grlFilter);
    vblCustom.addLayout(&hblButtons);
    dlgCustom.setLayout(&vblCustom);
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
        blflFilters.clear();
        for(int iK=0;iK<wglFilters.count();iK++) {
            if(qobject_cast<QCheckBox *>(wglFilters.at(iK))->isChecked())
                blflFilters.append(static_cast<BadooListFilter>(iK));
            delete wglFilters.at(iK);
        }
        iMediaCount=spbMediaCount.value();
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
