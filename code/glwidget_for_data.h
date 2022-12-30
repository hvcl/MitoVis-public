///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

#ifndef GlWidgetForData_H
#define GlWidgetForData_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Compatibility>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QOpenGLTexture>
#include <QVector2D>
#include <QOpenGLShaderProgram>
#include "tinytiffreader.h"
#include <QVector>

#include <glwidget_for_inset.h>

class uchar4{
public:
    unsigned char x;
    unsigned char y;
    unsigned char z;
    unsigned char w;
};

class int3{
public:
    int x;
    int y;
    int z;
};

class Window;
class patchSet;

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GlWidgetForData : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Compatibility
{
    Q_OBJECT

public:
    GlWidgetForData(Window *p, QWidget *parent = 0);
    ~GlWidgetForData();

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
    void drawSelectedLines();

    void StructureChangeByCCf(QVector2D startP);
    void StructureChangeByCCb(QVector2D startP);

    void MitoExcluding();
    void MitoIncluding();
    void MitoSplitting();
    void MitoMerging();
    float diceScore(int i);
    void drawVisualGuideForStructure();
    QVector2D getCandidateMiddlePoint(int sx,int sy,bool *ck);

    void insetVisualization_mito();

    bool onSegment(QVector2D p, QVector2D q, QVector2D r);
    int orientation(QVector2D p, QVector2D q, QVector2D r);
    bool doIntersect(QVector2D p1, QVector2D q1, QVector2D p2, QVector2D q2);

    void insetClear();

    bool insetClearTrigger=false;


    void focusNeurite(QVector2D p,unsigned char *buffer,QVector3D brushColor);
    void correctNeurite(QVector2D p, int type);

    void radarVisualization();

    void getTopSimilarNeurites(int target_neurite, int target_type, QVector <int> *buffer);
    void getTopDissimilarNeurites(int target_neurite, int target_type,QVector <int> *buffer);
    int findNearestNode(int s_x,int s_y);

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

    int con_thresh_control;
    int thresh_control=0;

    int con_grayscale;
    int con_work_type;



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

    bool doMitoBrushing=false;

    QVector<GlWidgetForInset*> Insets_mito;
    QVector<int> InsetIndexs_mito;
    QVector<QVector2D> InsetLocations_mito;
    QVector<int> InsetSizes_mito;

    int current_inset=-1;


    QVector2D dxylist[9]={QVector2D(0,0),QVector2D(-1,0),QVector2D(1,0),QVector2D(0,-1),QVector2D(0,1),QVector2D(1,1),QVector2D(1,-1),QVector2D(-1,-1),QVector2D(-1,1)};

};

#endif
