#include "browsefolderdialog.h"

#define DIALOG_TITLE "Browsing"

BrowseFolderDialog::BrowseFolderDialog(BadooFolderType      bftFolder,
                                       BadooListSectionType blstSection,
                                       BadooListFilterList  blflFilters,
                                       BadooWrapper         *bwParent,
                                       int                  iMediaLimit,
                                       DB                   *dbDatabase,
                                       QWidget              *wgtParent):
BrowseFolderDialog(FOLDER_TYPE_UNKNOWN,blflFilters,bwParent,iMediaLimit,dbDatabase,wgtParent) {
    bftBrowsedFolder=bftFolder;
    blstBrowsedSection=blstSection;
    if(this->getNewPage(0)) {
        bDialogReady=true;
        QTimer::singleShot(
            0,
            [=]() {
                this->showCurrentPage();
            }
        );
    }
}

BrowseFolderDialog::BrowseFolderDialog(FolderType          ftType,
                                       BadooListFilterList blflFilters,
                                       BadooWrapper        *bwParent,
                                       int                 iMediaLimit,
                                       DB                  *dbDatabase,
                                       QWidget             *wgtParent):
QDialog(wgtParent) {
    bDialogReady=false;
    iCurrentPageIndex=-1;
    iTotalPages=0;
    iTotalProfiles=0;
    iMaxPageProfiles=0;
    iMaxMediaCount=iMediaLimit;
    bftBrowsedFolder=static_cast<BadooFolderType>(0);
    blstBrowsedSection=static_cast<BadooListSectionType>(0);
    blflBrowseFilters=blflFilters;
    buplBrowse.clear();
    mchPhotoContents.clear();
    mchVideoContents.clear();
    ftBrowse=ftType;
    bwBrowse=bwParent;
    fvCurrentPage=new FolderViewer(bwBrowse,this);
    fvCurrentPage->setDB(dbDatabase);
    vblLayout.addWidget(fvCurrentPage);
    this->setGeometry(0,0,640,480);
    this->setLayout(&vblLayout);
    this->setMinimumSize(400,300);
    connect(
        fvCurrentPage,
        &FolderViewer::buttonClicked,
        this,
        &BrowseFolderDialog::buttonClicked
    );
    if(FOLDER_TYPE_UNKNOWN!=ftBrowse)
        if(this->getNewPage(0)) {
            // This may be a costly operation (along with calling getNewPage(0)), ...
            // ... so I've decided to execute it before giving the control to the ...
            // ... user to avoid possible problems with the event loop.
            this->showCurrentPage();
            bDialogReady=true;
        }
}

BrowseFolderDialog::~BrowseFolderDialog() {
    delete fvCurrentPage;
}

bool BrowseFolderDialog::isReady() {
    return bDialogReady;
}

void BrowseFolderDialog::buttonClicked(FolderViewerButton fvbButton) {
    switch(fvbButton) {
        case FOLDER_VIEWER_BUTTON_FIRST:
            this->handleFirstButtonClick();
            break;
        case FOLDER_VIEWER_BUTTON_PREVIOUS:
            this->handlePreviousButtonClick();
            break;
        case FOLDER_VIEWER_BUTTON_PAGE:
            this->handlePageButtonClick();;
            break;
        case FOLDER_VIEWER_BUTTON_NEXT:
            this->handleNextButtonClick();
            break;
        case FOLDER_VIEWER_BUTTON_LAST:
            this->handleLastButtonClick();
            break;
    }
}

bool BrowseFolderDialog::getNewPage(int iPage) {
    bool                 bResult=false;
    int                  iBackupPageIndex,
                         iBackupTotalPages,
                         iBackupTotalProfiles,
                         iBackupMaxPageProfiles;
    QString              sError;
    BadooUserProfileList buplBackupBrowse;
    MediaContentsHash    mchBackupPhotoContents,
                         mchBackupVideoContents;
    iBackupPageIndex=iCurrentPageIndex;
    iBackupTotalPages=iTotalPages;
    iBackupTotalProfiles=iTotalProfiles;
    iBackupMaxPageProfiles=iMaxPageProfiles;
    buplBackupBrowse=buplBrowse;
    mchBackupPhotoContents=mchPhotoContents;
    mchBackupVideoContents=mchVideoContents;
    iCurrentPageIndex=iPage;
    iTotalPages=0;
    iTotalProfiles=0;
    iMaxPageProfiles=0;
    mchPhotoContents.clear();
    mchVideoContents.clear();
    this->setEnabled(false);
    this->setCursor(Qt::CursorShape::BusyCursor);
    QApplication::processEvents();
    while(true) {
        bool bOK;
        sError.clear();
        if(FOLDER_TYPE_UNKNOWN==ftBrowse)
            bOK=bwBrowse->getFolderPage(
                bftBrowsedFolder,
                blstBrowsedSection,
                blflBrowseFilters,
                iCurrentPageIndex,
                iMaxMediaCount,
                buplBrowse,
                iTotalPages,
                iTotalProfiles,
                iMaxPageProfiles
            );
        else
            bOK=bwBrowse->getFolderPage(
                ftBrowse,
                blflBrowseFilters,
                iCurrentPageIndex,
                iMaxMediaCount,
                buplBrowse,
                iTotalPages,
                iTotalProfiles,
                iMaxPageProfiles
            );
        if(bOK)
            if(buplBrowse.count())
                if(bwBrowse->downloadMultiProfileResources<QByteArray>(
                    buplBrowse,
                    mchPhotoContents,
                    mchVideoContents
                )) {
                    bResult=true;
                    break;
                }
                else
                    sError=bwBrowse->getLastError();
            else
                sError=QStringLiteral("No available profiles were\n"
                                      "found in the requested page");
        else
            sError=bwBrowse->getLastError();
        if(QMessageBox::StandardButton::Cancel==QMessageBox::question(
            this,
            QStringLiteral("Error"),
            sError,
            QMessageBox::StandardButton::Retry|QMessageBox::StandardButton::Cancel
        )) {
            iCurrentPageIndex=iBackupPageIndex;
            iTotalPages=iBackupTotalPages;
            iTotalProfiles=iBackupTotalProfiles;
            iMaxPageProfiles=iBackupMaxPageProfiles;
            buplBrowse=buplBackupBrowse;
            mchPhotoContents=mchBackupPhotoContents;
            mchVideoContents=mchBackupVideoContents;
            break;
        }
    }
    this->unsetCursor();
    this->setEnabled(true);
    return bResult;
}

void BrowseFolderDialog::handleFirstButtonClick() {
    bool bUpdate=false;
    emit statusChanged(QString());
    if(-1<iCurrentPageIndex)
        if(iCurrentPageIndex)
            bUpdate=this->getNewPage(0);
        else
            emit statusChanged(QStringLiteral("Already at the first page"));
    if(bUpdate)
        this->showCurrentPage();
}

void BrowseFolderDialog::handlePreviousButtonClick() {
    bool bUpdate=false;
    emit statusChanged(QString());
    if(-1<iCurrentPageIndex)
        if(iCurrentPageIndex)
            bUpdate=this->getNewPage(iCurrentPageIndex-1);
        else
            emit statusChanged(QStringLiteral("Already at the first page"));
    if(bUpdate)
        this->showCurrentPage();
}

void BrowseFolderDialog::handlePageButtonClick() {
    bool bUpdate=false;
    emit statusChanged(QString());
    if(-1<iCurrentPageIndex)
        if(iTotalPages>1) {
            QInputDialog idPage;
            idPage.setInputMode(QInputDialog::InputMode::IntInput);
            idPage.setIntRange(1,iTotalPages);
            idPage.setIntValue(iCurrentPageIndex+1);
            idPage.setLabelText(
                QStringLiteral("Page number [1 - %1]:").arg(iTotalPages)
            );
            idPage.setWindowFlag(Qt::WindowType::MSWindowsFixedSizeDialogHint);
            idPage.setWindowFlag(Qt::WindowType::CustomizeWindowHint);
            idPage.setWindowFlag(Qt::WindowType::WindowSystemMenuHint,false);
            idPage.setWindowModality(Qt::WindowModality::ApplicationModal);
            idPage.setWindowTitle(QStringLiteral("Go to page"));
            if(QDialog::DialogCode::Accepted==idPage.exec())
                if(iCurrentPageIndex!=idPage.intValue()-1)
                    bUpdate=this->getNewPage(idPage.intValue()-1);
        }
        else if(iTotalPages)
            emit statusChanged(QStringLiteral("Already at the only page"));
    if(bUpdate)
        this->showCurrentPage();
}

void BrowseFolderDialog::handleNextButtonClick() {
    bool bUpdate=false;
    emit statusChanged(QString());
    if(-1<iCurrentPageIndex)
        if(iCurrentPageIndex<iTotalPages-1)
            bUpdate=this->getNewPage(iCurrentPageIndex+1);
        else
            emit statusChanged(QStringLiteral("Already at the last page"));
    if(bUpdate)
        this->showCurrentPage();
}

void BrowseFolderDialog::handleLastButtonClick() {
    bool bUpdate=false;
    emit statusChanged(QString());
    if(-1<iCurrentPageIndex)
        if(iCurrentPageIndex<iTotalPages-1)
            bUpdate=this->getNewPage(iTotalPages-1);
        else
            emit statusChanged(QStringLiteral("Already at the last page"));
    if(bUpdate)
        this->showCurrentPage();
}

void BrowseFolderDialog::showCurrentPage() {
    QString sWindowTitle,
            sPageTitle;
    if(-1<iCurrentPageIndex) {
        int iFirstProfileInPage,
            iLastProfileInPage;
        sWindowTitle=QStringLiteral("%1 %2 - Page %3 of %4").
                     arg(QStringLiteral(DIALOG_TITLE)).
                     arg(BadooWrapper::getFolderName(ftBrowse)).
                     arg(iCurrentPageIndex+1).
                     arg(iTotalPages);
        iFirstProfileInPage=iCurrentPageIndex*iMaxPageProfiles;
        if(iCurrentPageIndex<iTotalPages-1)
            iLastProfileInPage=iFirstProfileInPage+iMaxPageProfiles-1;
        else
            iLastProfileInPage=iTotalProfiles-1;
        sPageTitle=QStringLiteral("Showing profiles %1 to %2 of %3 - Page %4 of %5").
                   arg(iFirstProfileInPage+1).
                   arg(iLastProfileInPage+1).
                   arg(iTotalProfiles).
                   arg(iCurrentPageIndex+1).
                   arg(iTotalPages);
    }
    else {
        sWindowTitle=QStringLiteral("%1 %2 - No results").
                     arg(QStringLiteral(DIALOG_TITLE)).
                     arg(BadooWrapper::getFolderName(ftBrowse));
        sPageTitle.clear();
    }
    this->setWindowTitle(sWindowTitle);
    fvCurrentPage->load(
        buplBrowse,
        mchPhotoContents,
        mchVideoContents
    );
    fvCurrentPage->setFolderType(ftBrowse);
    fvCurrentPage->setPageTitle(sPageTitle);
}
