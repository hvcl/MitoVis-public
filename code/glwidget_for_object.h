///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

#ifndef GlWidgetForOBJECT_H
#define GlWidgetForOBJECT_H

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

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GlWidgetForObject : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Compatibility
{
    Q_OBJECT

public:
    GlWidgetForObject(Window *p, QWidget *parent = 0);
    ~GlWidgetForObject();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    QVector4D background_color;


    int getObjectCoordfromMousePoint(QVector2D mousePoint); //mouse point: window coordinate


    void setStructureTex();
    void setMitoLabelTex();
    void setCorrectionTex();
    void setSubsetTex();
    void setConnectionTex();

    void drawTracingLines();

public slots:

    void interpolation_type_change(int a);
    void cleanup();

    void cut_plane(int r);

    void set_z_scale(int n);


    void ambi_change(int a);
    void diff_change(int a);
    void spec_change(int a);
    void shin_change(int a);
    void l_t_change(int a);


    void change_R1(int a);
    void change_R2(int a);
    void change_cut_d(int a);
    void change_cut_enable(bool a);
    void change_cut_alpha(int a);

    void change_phong_enable(bool a);
    void change_sampling_rate(int a);


signals:

    void backgroundColorChanged();
    void label_changed(int minx,int miny,int minz,int tw,int th,int td,unsigned char *tdata);

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
    void ReadColorTable();
    void ReadLabelColor();

    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;


public:

    QPoint m_lastPos;

    QOpenGLShaderProgram *m_program;


    QString p_fps;
    int fps;
    int fps_start;


    Window *c_window;

    int prevDataIndex=-1;
    QPoint mousePressedPos=QPoint(0,0);


    int con_tex_neuron;
    int con_tex_structure;
    int con_tex_mito;
    int con_tex_mitoLabel;
    int con_tex_mitoLabel_original;
    int con_tex_correction;
    int con_tex_subset;
    int con_structure_opacity;
    int con_mitoLabel_opacity;
    int con_correction_opacity;
    int con_tex_connection;

    int con_background_color;
    int con_shift;
    int con_scale;
    int con_window_size;
    int con_imageSize;
    int con_mousePos;
    int con_brushSize;
    int con_brushColor;

    int con_structure_thresh;

    int con_structure_prev;

    int con_color_table_neuron;
    int con_color_table_mito;

    bool structureChanged=false;
    bool mitoLabelChanged=false;
    bool colorTableChanged=false;
    bool correctionChanged=false;
    bool connectionChanged=false;
    bool subsetChanged=false;

    QVector2D measureStart;
    QVector2D measureEnd;
    float measureLen=0;
    bool measure=false;

    int con_mito_thresh;
    int con_mito_alpha;
    bool firstAdded=false;

    QVector2D mousePoint=QVector2D(0,0);
    bool IsSpacePressed=false;

    bool *ck_brushing=NULL;

    bool mitoUpdate=false;
};

#endif
