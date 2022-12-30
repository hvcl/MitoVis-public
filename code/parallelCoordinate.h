#ifndef PARALLELCOORDINATE_H
#define PARALLELCOORDINATE_H

#include <QLinearGradient>
#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QWidget>
#include <QMatrix4x4>
#include <QVector2D>
#include <QLineEdit>



class Window;
class MitoDataset;


class ParallelCoordinate :public QWidget
{
    Q_OBJECT

public:
    ParallelCoordinate(Window *p, QWidget *parent = 0);
    ~ParallelCoordinate();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

    QVector<float> points;
    QVector<QVector3D> colors;

    float max_value;
    QVector<QLineEdit *>thresh_bar_start;
    QVector<QLineEdit *>thresh_bar_end;

    int clicked_thresh_bar=-1;
    QString clicked_thresh_bar_type="";
    bool editMode=false;

public slots:

    void handleEditStart(QString b);
    void handleEditFinish1();
    void handleEditFinish2();

signals:
    void viewChange(int num);
    void synchronization();



protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;


    void readAnalysisFile();


private:

    Window *c_window;
    int prePost; //0: false /1: pre /2: post


    QPoint m_lastPos;

    QPixmap background;
    QPoint lastPos;
    Qt::MouseButton button;


    QVector2D *axisStarts; //each axis point
    QVector2D *axisEnds; //each axis point


    int selectionFrame;
    bool doSelect;
    float startSelectValue;

    bool is_axis_edit=true;



};


#endif // PARALLELCOORDINATE_H
