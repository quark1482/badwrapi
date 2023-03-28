#ifndef MEDIACONTROLS_H
#define MEDIACONTROLS_H

#include <QtCore>
#include <QtWidgets>

class MediaControls:public QWidget {
    Q_OBJECT
public:
    MediaControls(QWidget * =nullptr,bool=false,Qt::Orientation=Qt::Orientation::Horizontal);
    void setAlignment(Qt::AlignmentFlag,Qt::AlignmentFlag);
    void setButtonSizeRatio(qreal);
    void setPauseButtonState(bool);
    void setMuteButtonState(bool);
    void resetVisualStatus();
protected:
    void resizeEvent(QResizeEvent *) override;
private slots:
    void firstButtonClicked();
    void previousButtonClicked();
    void nextButtonClicked();
    void lastButtonClicked();
    void pauseButtonClicked(bool);
    void muteButtonClicked(bool);
private:
    qreal             rButtonSizeRatio;
    QHBoxLayout       hblLayout;
    QVBoxLayout       vblLayout;
    QPushButton       btnFirst,
                      btnPrevious,
                      btnNext,
                      btnLast,
                      btnPause,
                      btnMute;
    Qt::AlignmentFlag alnHAlignment,
                      alnVAlignment;
    void configureMediaButton(QPushButton *,QString,QString=QString());
signals:
    void first();
    void previous();
    void next();
    void last();
    void pause(bool);
    void mute(bool);
};

#endif // MEDIACONTROLS_H
