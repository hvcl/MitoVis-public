///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

#ifndef GlWidgetForINSET_H
#define GlWidgetForINSET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Compatibility>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QOpenGLTexture>
#include <QVector2D>
#include <QOpenGLShaderProgram>
#include "tinytiffreader.h"
#include <QVector>


class Window;
class MitoDataset;

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GlWidgetForInset : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Compatibility
{
    Q_OBJECT

public:
    GlWidgetForInset(Window *p, MitoDataset *M, int ind, int type, QVector2D startPos, QVector2D endPos, QWidget *parent = 0);
    ~GlWidgetForInset();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    QVector4D background_color;


    int inset_type=0;   //0: structure
                        //1: mitochondria

    MitoDataset *curData;


    QVector2D inset_startPos=QVector2D(0,0); //pixel coordinate
    QVector2D inset_endPos=QVector2D(0,0); //pixel coordinate

    QVector2D inset_imageSize;

    int con_window_size;

    int con_imageStart;
    int con_imageEnd;



    int con_tex_background;
    int con_tex_label;
    int con_mousePos;
    int con_brushSize;
    int con_brushColor;
    int con_color_table;

    GLuint tex_background;
    GLuint tex_label;
    GLuint tex_color_table;

    unsigned short* background_data;
    unsigned char* label_data;


public slots:

    void cleanup();

    void setMitoLabelTex();
    void setStructureTex();

signals:

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;
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

    QOpenGLShaderProgram *m_program;


    bool correctionChanged=false;
    bool structureChanged=false;
    bool mitoUpdate=false;


    Window *c_window;

    QPoint mousePressedPos=QPoint(0,0);


    bool firstAdded=false;

    QVector2D mousePoint=QVector2D(0,0);
    bool IsSpacePressed=false;


    bool mitoLabelChanged=false;
    bool colorTableChanged=true;

    bool isFocused=false;

    int mitoIndex=0;
    bool doMitoBrushing=false;

    int sample_point=-1;


};

#endif
