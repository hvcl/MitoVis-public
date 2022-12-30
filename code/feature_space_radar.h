///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

#ifndef FSRadar_H
#define FSRadar_H

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
class GlWidgetForInset;

class FSRadar :public QWidget
{
    Q_OBJECT

public:
    FSRadar(Window *p, MitoDataset *m, int mouse_p, QVector<int> top_sim, QVector<int> top_dissim, QWidget *parent = 0);
    ~FSRadar();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

    void info_update(int p,QVector<int> top_sim, QVector<int> top_dissim);

    void ver1();
    void ver2();
    void ver3();
    void ver3_init();

    void drawTracingMap(QPainter *painter);

    int target_point=0;
    int target_neurite_ind=0;
    QVector <int> top_similar_neurites;
    QVector <int> top_dissimilar_neurites;

    bool is_focused=false;

    bool is_focus_fixed=false;

    GlWidgetForInset *recommendation_insets[4];

    bool is_ver3_init=false;


public slots:
    void mapSimplification();
    bool mapSimplification_sub(float distance_thresh);
    void node_combine(int v0,int v1);
    void node_move(int v0,int v1); //v0:to_v v1:from_v

signals:


protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;



private:

    Window *c_window;
    MitoDataset *curData;
    QPoint m_lastPos;

    QPixmap background;
    QPoint lastPos;
    Qt::MouseButton button;

    bool scale_changed=false;

    QVector2D shift;
    float scale=1.0;



};

#endif
