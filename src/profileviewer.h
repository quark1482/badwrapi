#ifndef PROFILEVIEWER_H
#define PROFILEVIEWER_H

#include "badoowrapper.h"
#include "mediaviewer.h"
#include "mediacontrols.h"

namespace Ui {
class ProfileViewer;
}

typedef enum {
    PROFILE_VIEWER_BUTTON_COPY_URL=  1,
    PROFILE_VIEWER_BUTTON_DOWNLOAD=  2,
    PROFILE_VIEWER_BUTTON_BACK    =  4,
    PROFILE_VIEWER_BUTTON_NOPE    =  8,
    PROFILE_VIEWER_BUTTON_FAVORITE= 16,
    PROFILE_VIEWER_BUTTON_LIKE    = 32,
    PROFILE_VIEWER_BUTTON_SKIP    = 64,
    PROFILE_VIEWER_BUTTON_BLOCK   =128,
    PROFILE_VIEWER_BUTTON_UNBLOCK =256,
    PROFILE_VIEWER_BUTTON_UNMATCH =512
} ProfileViewerButton;

typedef enum {
    PROFILE_VIEWER_DANGER_BUTTON_INDEX_BLOCK,
    PROFILE_VIEWER_DANGER_BUTTON_INDEX_UNBLOCK,
    PROFILE_VIEWER_DANGER_BUTTON_INDEX_UNMATCH
} ProfileViewerDangerButtonIndex;

typedef enum {
    ITEM_DATA_KEY_INDEX,
    ITEM_DATA_KEY_TYPE,
    ITEM_DATA_KEY_ACTION
} ItemDataKey;

typedef enum {
    BADGE_ACTION_NONE,
    BADGE_ACTION_FAST_MESSAGE,
    BADGE_ACTION_OPEN_CHAT
} BadgeAction;

class ProfileViewer:public QWidget {
    Q_OBJECT
public:
    ProfileViewer(BadooWrapper *,QWidget * =nullptr);
    ~ProfileViewer();
    void load(BadooUserProfile,QByteArrayList,QByteArrayList);
    void setActiveActionButtons(int);
protected:
    bool eventFilter(QObject *,QEvent *) override;
    void resizeEvent(QResizeEvent *) override;
private slots:
    void delayedResizeTimeout();
    void copyURLButtonClicked();
    void downloadProfileButtonClicked();
    void backButtonClicked();
    void nopeButtonClicked();
    void favoriteButtonClicked(bool);
    void likeButtonClicked();
    void skipButtonClicked();
    void blockButtonClicked();
    void unblockButtonClicked();
    void unmatchButtonClicked();
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
    int               iActiveActionButtons,
                      iCurrentPhotoIndex,
                      iCurrentVideoIndex,
                      iPreviousVideoIndex;
    QString           sPreviousProfileId;
    QByteArray        abtPlaceholderPhoto,
                      abtPlaceholderVideo,
                      abtOwnProfilePhoto;
    QByteArrayList    abtlProfilePhotos,
                      abtlProfileVideos;
    QTimer            tmrDelayedResize;
    QGraphicsScene    grsPhotoGallery,
                      grsVideoGallery;
    QToolBar          tlbDanger;
    QToolButton       tbtBlock,
                      tbtUnblock,
                      tbtUnmatch;
    BadooWrapper      *bwProfile;
    MediaViewer       *mvwPhoto,
                      *mvwVideo;
    MediaControls     *mctPhotoControls,
                      *mctVideoControls;
    BadooUserProfile  bupProfileDetails;
    void clearDynamicWidgets();
    void resetProfileWidgets(int=0,int=0);
    bool showChatWithProfile();
    bool showQuickChatWithProfile();
    void showMatch(QString,QByteArray,QByteArray,int=640);
    void showVote(bool);
    void toggleMediaViewersIndependence();
    void updateActionButtons();
    void updateMediaButtons();
    void updateMediaTitle();
    void updateMediaWidgets();
    void updatePhotoContent();
    void updatePhotoGallery();
    void updateProfileInfo();
    void updateVideoContent();
    void updateVideoGallery();
signals:
    void badgeClicked(BadgeAction);
    void buttonClicked(ProfileViewerButton);
};

#endif // PROFILEVIEWER_H
