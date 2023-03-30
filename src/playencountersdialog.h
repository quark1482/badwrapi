#ifndef PLAYENCOUNTERSDIALOG_H
#define PLAYENCOUNTERSDIALOG_H

#include <QDialog>
#include <QMediaPlayer>
#include "badoowrapper.h"
#include "mediaviewer.h"
#include "mediacontrols.h"

namespace Ui {
class PlayEncountersDialog;
}

class PlayEncountersDialog:public QDialog {
    Q_OBJECT
public:
    PlayEncountersDialog(BadooWrapper *,QWidget * =nullptr);
    ~PlayEncountersDialog();
    bool isReady();
protected:
    bool eventFilter(QObject *,QEvent *) override;
    void resizeEvent(QResizeEvent *) override;
private slots:
    void delayedResizeTimeout();
    void copyURLButtonClicked();
    void downloadProfileButtonClicked();
    void backButtonClicked();
    void nopeButtonClicked();
    void likeButtonClicked();
    void skipButtonClicked();
    void firstPhotoButtonClicked();
    void previousPhotoButtonClicked();
    void nextPhotoButtonClicked();
    void lastPhotoButtonClicked();
    void firstVideoButtonClicked();
    void previousVideoButtonClicked();
    void nextVideoButtonClicked();
    void lastVideoButtonClicked();
    void pauseVideoButtonClicked(bool);
    void muteVideoButtonClicked(bool);
    void galleryTabWidgetChanged(int);
    void photoDoubleClicked();
    void photoKeyPressed(int);
    void photoMouseHover(QPoint);
    void videoDoubleClicked();
    void videoKeyPressed(int);
    void videoMouseHover(QPoint);
private:
    Ui::PlayEncountersDialog *ui;
    bool                     bDialogReady,
                             bVideoPausedByUser;
    int                      iCurrentProfileIndex,
                             iCurrentPhotoIndex,
                             iCurrentVideoIndex;
    QByteArray               abtPlaceholderPhoto,
                             abtPlaceholderVideo,
                             abtMyProfilePhoto;
    QTimer                   tmrDelayedResize;
    QGraphicsScene           grsPhotoGallery,
                             grsVideoGallery;
    BadooWrapper             *bwEncounters;
    BadooUserProfile         bupMyProfile;
    BadooUserProfileList     buplEncounters;
    MediaViewer              *mvwPhoto,
                             *mvwVideo;
    MediaControls            *mctPhotoControls,
                             *mctVideoControls;
    MediaContentsHash        mchPhotoContents,
                             mchVideoContents;
    QString getCurrentProfileId();
    void    getFullFileContents(QString,QByteArray &);
    bool    getNewBatch(bool=false);
    void    loadMyProfile();
    void    resetProfileWidgets(int=0,int=0);
    void    showMatch(QString,QByteArray,QByteArray,int=640);
    void    showVote(bool);
    void    toggleMediaViewersIndependence();
    void    updateMediaTitle();
    void    updateMediaWidgets();
    void    updatePhotoContent();
    void    updatePhotoGallery();
    void    updateProfileInfo();
    void    updateProfileTitle();
    void    updateVideoContent();
    void    updateVideoGallery();
signals:
    void statusChanged(QString);
};

#endif // PLAYENCOUNTERSDIALOG_H
