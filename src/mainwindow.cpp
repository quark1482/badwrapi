#include "mainwindow.h"
#include "./ui_mainwindow.h"

#define APP_TITLE "Badoo API wrapper demo"

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent),ui(new Ui::MainWindow) {
    ui->setupUi(this);
    dlgBrowseFolder=nullptr;
    dlgEncounters=nullptr;
    mdiArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setCentralWidget(&mdiArea);
    this->setWindowTitle(QStringLiteral(APP_TITLE));
    connect(
        ui->actEncounters,
        &QAction::triggered,
        this,
        &MainWindow::menuEncountersTriggered
    );
    connect(
        ui->actMatches,
        &QAction::triggered,
        this,
        &MainWindow::menuMatchesTriggered
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
    QTimer::singleShot(
        0,
        [=]() {
            this->resize(800,600);
        }
    );
}

MainWindow::~MainWindow() {
    if(nullptr!=dlgBrowseFolder)
        delete dlgBrowseFolder;
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
    dlgBrowseFolder=nullptr;
}

void MainWindow::dialogEncountersDestroyed() {
    dlgEncounters=nullptr;
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

void MainWindow::menuMatchesTriggered(bool) {
    if(bwMain.isLoggedIn())
        if(nullptr==dlgBrowseFolder) {
            dlgBrowseFolder=new BrowseFolderDialog(FOLDER_TYPE_MATCHES,&bwMain,this);
            connect(
                dlgBrowseFolder,
                &BrowseFolderDialog::statusChanged,
                this,
                &MainWindow::wrapperStatusChanged
            );
            connect(
                dlgBrowseFolder,
                &BrowseFolderDialog::destroyed,
                this,
                &MainWindow::dialogBrowseFolderDestroyed
            );
            // There seems to be a problem when a QDialog belongs to a QMdiArea and ...
            // ... QDialog::reject() is invoked by pressing the Escape key, causing ...
            // ... that the dialog remains visible. So, as a straightforward fix we ...
            // ... call QMdiArea::closeActiveSubWindow() to close it by force.
            connect(
                dlgBrowseFolder,
                &BrowseFolderDialog::rejected,
                &mdiArea,
                &QMdiArea::closeActiveSubWindow
            );
            dlgBrowseFolder->setWindowIcon(QIcon(QStringLiteral(":img/bw.ico")));
            mdiArea.addSubWindow(dlgBrowseFolder)->setWindowFlag(
                Qt::WindowType::WindowMinMaxButtonsHint
            );
            if(dlgBrowseFolder->isReady())
                dlgBrowseFolder->exec();
            else
                delete dlgBrowseFolder;
        }
        else
            ; // ToDo: Browsing folders of distinct types should be possible.
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
    return nullptr!=dlgBrowseFolder||nullptr!=dlgEncounters;
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
