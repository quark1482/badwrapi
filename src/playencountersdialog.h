#ifndef PLAYENCOUNTERSDIALOG_H
#define PLAYENCOUNTERSDIALOG_H

#include <QDialog>
#include "profileviewer.h"

class PlayEncountersDialog:public QDialog {
    Q_OBJECT
public:
    PlayEncountersDialog(BadooWrapper *,QWidget * =nullptr);
    ~PlayEncountersDialog();
    bool isReady();
private slots:
    void buttonClicked(ProfileViewerButton);
private:
    bool                 bDialogReady;
    int                  iCurrentProfileIndex;
    QByteArray           abtMyProfilePhoto;
    BadooWrapper         *bwEncounters;
    BadooUserProfileList buplEncounters;
    MediaContentsHash    mchPhotoContents,
                         mchVideoContents;
    ProfileViewer        *pvCurrentProfile;
    QVBoxLayout          vblMain;
    QString getCurrentProfileId();
    bool    getNewBatch(bool=false);
    void    handleCopyURLButtonClick();
    void    handleDownloadProfileButtonClick();
    void    handleBackButtonClick();
    void    handleNopeButtonClick();
    void    handleLikeButtonClick();
    void    handleSkipButtonClick();
    void    loadMyProfile();
    void    showCurrentProfile();
    void    showMatch(QString,QByteArray,QByteArray,int=640);
signals:
    void statusChanged(QString);
};

#endif // PLAYENCOUNTERSDIALOG_H
