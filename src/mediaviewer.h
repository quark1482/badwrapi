#ifndef MEDIAVIEWER_H
#define MEDIAVIEWER_H

#include <QtCore>
#include <QtWidgets>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QVideoFrame>
#include <QVideoSink>
#include <QAudioOutput>

typedef enum {
    MEDIA_TYPE_PHOTO,
    MEDIA_TYPE_VIDEO
} MediaType;

class MediaViewer:public QWidget {
    Q_OBJECT
public:
    MediaViewer(MediaType=MEDIA_TYPE_PHOTO,QWidget * =nullptr);
    ~MediaViewer();
    static void getFrame(QByteArray,QPixmap &,qreal=0.5);
    qint64      getPosition();
    void        loadVideo(QByteArray);
    void        muteVideo(bool);
    void        playVideo();
    void        pauseVideo();
    void        setAutoHideOverlay(bool);
    void        setOverlay(QWidget *);
    void        setPosition(qint64);
    void        showPhoto(QByteArray);
    void        showVideo();
    void        stopVideo();
protected:
    void keyPressEvent(QKeyEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
private:
    bool                bAutoHideOverlay;
    QWidget             *wgtOverlay;
    QByteArray          abtMedia;
    QVBoxLayout         vblLayout;
    QGraphicsView       grvView;
    QGraphicsScene      grsScene;
    QGraphicsPixmapItem grpiPixmap;
    QGraphicsVideoItem  grviVideo;
    QMediaPlayer        mpPlayer;
    QAudioOutput        aoAudio;
    QBuffer             bufVideo;
    MediaType           mtType;
signals:
    void doubleClick();
    void hover(QPoint);
    void keyPress(int);
};

#endif // MEDIAVIEWER_H
