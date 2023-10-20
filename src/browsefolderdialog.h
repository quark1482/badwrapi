#ifndef BROWSEFOLDERDIALOG_H
#define BROWSEFOLDERDIALOG_H

#include <QDialog>
#include "folderviewer.h"

class BrowseFolderDialog:public QDialog {
    Q_OBJECT
public:
    BrowseFolderDialog(BadooFolderType,BadooListSectionType,BadooWrapper *,QWidget * =nullptr);
    BrowseFolderDialog(FolderType,FolderFilterList,BadooWrapper *,QWidget * =nullptr);
    ~BrowseFolderDialog();
    bool isReady();
private slots:
    void buttonClicked(FolderViewerButton);
private:
    bool                 bDialogReady;
    int                  iCurrentPageIndex,
                         iTotalPages,
                         iTotalProfiles,
                         iMaxPageProfiles;
    BadooWrapper         *bwBrowse;
    BadooFolderType      bftBrowsedFolder;
    BadooListSectionType blstBrowsedSection;
    BadooUserProfileList buplBrowse;
    MediaContentsHash    mchPhotoContents,
                         mchVideoContents;
    FolderType           ftBrowse;
    FolderFilterList     fflBrowse;
    FolderViewer         *fvCurrentPage;
    QVBoxLayout          vblLayout;
    bool getNewPage(int);
    void handleFirstButtonClick();
    void handlePreviousButtonClick();
    void handlePageButtonClick();
    void handleNextButtonClick();
    void handleLastButtonClick();
    void showCurrentPage();
signals:
    void statusChanged(QString);
};

#endif // BROWSEFOLDERDIALOG_H
