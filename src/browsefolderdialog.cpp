#include "browsefolderdialog.h"

#define DIALOG_TITLE "Browsing"

BrowseFolderDialog::BrowseFolderDialog(FolderType   ftType,
                                       BadooWrapper *bwParent,
                                       QWidget      *wgtParent):QDialog(wgtParent) {
    bDialogReady=false;
    iCurrentPageIndex=-1;
    iTotalPages=0;
    iTotalProfiles=0;
    iMaxPageProfiles=0;
    buplBrowse.clear();
    mchPhotoContents.clear();
    mchVideoContents.clear();
    ftBrowse=ftType;
    bwBrowse=bwParent;
    fvCurrentPage=new FolderViewer(bwBrowse,this);
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
    if(this->getNewPage(true)) {
        bDialogReady=true;
        QTimer::singleShot(
            0,
            [=]() {
                this->showCurrentPage();
            }
        );
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
        case FOLDER_VIEWER_BUTTON_NEXT:
            this->handleNextButtonClick();
            break;
        case FOLDER_VIEWER_BUTTON_LAST:
            this->handleLastButtonClick();
            break;
    }
}

bool BrowseFolderDialog::getNewPage(bool bReset) {
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
    if(bReset)
        iCurrentPageIndex=0;
    iTotalPages=0;
    iTotalProfiles=0;
    iMaxPageProfiles=0;
    mchPhotoContents.clear();
    mchVideoContents.clear();
    this->setEnabled(false);
    this->setCursor(Qt::CursorShape::BusyCursor);
    QApplication::processEvents();
    while(true) {
        sError.clear();
        if(bwBrowse->getFolderPage(
            ftBrowse,
            iCurrentPageIndex,
            buplBrowse,
            iTotalPages,
            iTotalProfiles,
            iMaxPageProfiles
        ))
            if(buplBrowse.count())
                if(bwBrowse->downloadMultiProfileResources(
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
        if(iCurrentPageIndex) {
            iCurrentPageIndex=0;
            bUpdate=this->getNewPage();
        }
        else
            emit statusChanged(QStringLiteral("Already at first page"));
    if(bUpdate)
        this->showCurrentPage();
}

void BrowseFolderDialog::handlePreviousButtonClick() {
    bool bUpdate=false;
    emit statusChanged(QString());
    if(-1<iCurrentPageIndex)
        if(iCurrentPageIndex) {
            iCurrentPageIndex--;
            bUpdate=this->getNewPage();
        }
        else
            emit statusChanged(QStringLiteral("Already at first page"));
    if(bUpdate)
        this->showCurrentPage();
}

void BrowseFolderDialog::handleNextButtonClick() {
    bool bUpdate=false;
    emit statusChanged(QString());
    if(-1<iCurrentPageIndex)
        if(iCurrentPageIndex<iTotalPages-1) {
            iCurrentPageIndex++;
            bUpdate=this->getNewPage();
        }
        else
            emit statusChanged(QStringLiteral("Already at last page"));
    if(bUpdate)
        this->showCurrentPage();
}

void BrowseFolderDialog::handleLastButtonClick() {
    bool bUpdate=false;
    emit statusChanged(QString());
    if(-1<iCurrentPageIndex)
        if(iCurrentPageIndex<iTotalPages-1) {
            iCurrentPageIndex=iTotalPages-1;
            bUpdate=this->getNewPage();
        }
        else
            emit statusChanged(QStringLiteral("Already at last page"));
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
                     arg(bwBrowse->getFolderName(ftBrowse)).
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
                     arg(bwBrowse->getFolderName(ftBrowse));
        sPageTitle.clear();
    }
    this->setWindowTitle(sWindowTitle);
    fvCurrentPage->load(
        buplBrowse,
        mchPhotoContents,
        mchVideoContents
    );
    fvCurrentPage->setPageTitle(sPageTitle);
}
