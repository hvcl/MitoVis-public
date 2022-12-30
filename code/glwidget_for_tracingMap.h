///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

#ifndef GlWidgetForTracingMap_H
#define GlWidgetForTracingMap_H

#include <QMatrix4x4>
#include <QVector2D>
#include "tinytiffreader.h"
#include <QVector>
#include <QWidget>


class Window;
class MitoDataset;


class GlWidgetForTracingMap :public QWidget
{
    Q_OBJECT

public:
    GlWidgetForTracingMap(Window *p, QWidget *parent = 0);
    ~GlWidgetForTracingMap();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    QVector4D background_color;

    void mapSimplification();
    bool mapSimplification_sub(float distance_thresh);
    void node_combine(int v0,int v1);
    void node_move(int v0,int v1); //v0:to_v v1:from_v
    int find_nearest_node(int px,int py);
    void get_neurite(int p,QVector<int> *res);

public slots:


signals:

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;
    QVector3D cross_product(QVector3D v1,QVector3D v2);

    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;


public:

    QPoint m_lastPos;

    Window *c_window;

    QPoint mousePressedPos=QPoint(0,0);

    QVector2D mousePoint=QVector2D(0,0);
    bool IsSpacePressed=false;


    bool isFocused=false;

    float scale=0.3;
    QVector2D shift=QVector2D(0,0);
    bool scale_changed=false;

    QVector <int> selected_neurite;


};

#endif
