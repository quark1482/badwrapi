#ifndef PROFILEVIEWER_H
#define PROFILEVIEWER_H

#include "badoowrapper.h"
#include "mediaviewer.h"
#include "mediacontrols.h"

namespace Ui {
class ProfileViewer;
}

typedef enum {
    PROFILE_VIEWER_BUTTON_COPY_URL,
    PROFILE_VIEWER_BUTTON_DOWNLOAD,
    PROFILE_VIEWER_BUTTON_BACK,
    PROFILE_VIEWER_BUTTON_NOPE,
    PROFILE_VIEWER_BUTTON_LIKE,
    PROFILE_VIEWER_BUTTON_SKIP
} ProfileViewerButton;

class ProfileViewer:public QWidget {
    Q_OBJECT
public:
    ProfileViewer(BadooWrapper *,QWidget * =nullptr);
    ~ProfileViewer();
    void getPlaceholderPhoto(QByteArray &);
    void getPlaceholderVideo(QByteArray &);
    void setOwnProfilePhoto(QByteArray);
    void load(BadooUserProfile,QByteArrayList,QByteArrayList);
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
    Ui::ProfileViewer *ui;
    bool              bVideoPausedByUser;
    int               iCurrentPhotoIndex,
                      iCurrentVideoIndex;
    QByteArray        abtOwnProfilePhoto,
                      abtPlaceholderPhoto,
                      abtPlaceholderVideo;
    QByteArrayList    abtlProfilePhotos,
                      abtlProfileVideos;
    QTimer            tmrDelayedResize;
    QGraphicsScene    grsPhotoGallery,
                      grsVideoGallery;
    BadooWrapper      *bwProfile;
    MediaViewer       *mvwPhoto,
                      *mvwVideo;
    MediaControls     *mctPhotoControls,
                      *mctVideoControls;
    BadooUserProfile  bupProfileDetails;
    void    resetProfileWidgets(int=0,int=0);
    void    showMatch(QString,QByteArray,QByteArray,int=640);
    void    showVote(bool);
    void    toggleMediaViewersIndependence();
    void    updateActionButtons();
    void    updateMediaButtons();
    void    updateMediaTitle();
    void    updateMediaWidgets();
    void    updatePhotoContent();
    void    updatePhotoGallery();
    void    updateProfileInfo();
    void    updateVideoContent();
    void    updateVideoGallery();
signals:
    void buttonClicked(ProfileViewerButton);
};

#endif // PROFILEVIEWER_H
