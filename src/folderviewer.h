#ifndef FOLDERVIEWER_H
#define FOLDERVIEWER_H

#include "badoowrapper.h"
#include "profileviewer.h"

typedef enum {
    FOLDER_VIEWER_BUTTON_FIRST,
    FOLDER_VIEWER_BUTTON_PREVIOUS,
    FOLDER_VIEWER_BUTTON_PAGE,
    FOLDER_VIEWER_BUTTON_NEXT,
    FOLDER_VIEWER_BUTTON_LAST
} FolderViewerButton;

class FolderViewer:public QWidget {
    Q_OBJECT
public:
    FolderViewer(BadooWrapper *,QWidget * =nullptr);
    void load(BadooUserProfileList,MediaContentsHash,MediaContentsHash);
    void setFolderType(FolderType);
    void setPageTitle(QString);
protected:
    bool eventFilter(QObject *,QEvent *) override;
    void resizeEvent(QResizeEvent *) override;
private slots:
    void firstButtonClicked();
    void previousButtonClicked();
    void pageButtonClicked();
    void nextButtonClicked();
    void lastButtonClicked();
private:
    QByteArray           abtPlaceholderPhoto;
    QVBoxLayout          vblLayout;
    QHBoxLayout          hblLayout;
    QLabel               lblPageTitle;
    QPushButton          btnFirst,
                         btnPrevious,
                         btnPage,
                         btnNext,
                         btnLast;
    QGraphicsView        grvView;
    QGraphicsScene       grsScene;
    BadooWrapper         *bwFolder;
    BadooUserProfileList buplPageDetails;
    MediaContentsHash    mchPagePhotos,
                         mchPageVideos;
    FolderType           ftType;
    void configurePageButton(QPushButton *,QString,QString);
    void resetPageWidgets();
    void showStandaloneProfile(int);
    void updatePageGallery();
    void updatePageWidgets();
    void updateProfileBadges(QRect,bool,bool,bool,bool,bool,bool,bool,int,QString);
    void updateProfileMediaCounters(QRect,int,int);
    void updateProfilePhoto(QRect,QByteArray,int);
    void updateProfileTitle(QRect,QString,int,QString);
signals:
    void buttonClicked(FolderViewerButton);
};

#endif // FOLDERVIEWER_H
