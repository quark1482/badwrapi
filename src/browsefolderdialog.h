#ifndef BROWSEFOLDERDIALOG_H
#define BROWSEFOLDERDIALOG_H

#include <QDialog>
#include "folderviewer.h"

class BrowseFolderDialog:public QDialog {
    Q_OBJECT
public:
    BrowseFolderDialog(FolderType,BadooWrapper *,QWidget * =nullptr);
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
    BadooUserProfileList buplBrowse;
    MediaContentsHash    mchPhotoContents,
                         mchVideoContents;
    FolderType           ftBrowse;
    FolderViewer         *fvCurrentPage;
    QVBoxLayout          vblLayout;
    bool getNewPage(bool=false);
    void handleFirstButtonClick();
    void handlePreviousButtonClick();
    void handleNextButtonClick();
    void handleLastButtonClick();
    void showCurrentPage();
signals:
    void statusChanged(QString);
};

#endif // BROWSEFOLDERDIALOG_H
