///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////


#include "glwidget_for_data.h"
#include "structureRefinement.h"
#include "feature_space_radar.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>
#include <QProcess>
#include <QColorDialog>
#include <window.h>
#include <QVector2D>
#include <QPainter>
#include <QLineEdit>


#define PI 3.14159265358979323846


QVector3D GlWidgetForData::cross_product(QVector3D v1,QVector3D v2){
    QVector3D v;
    v.setX(v1.y()*v2.z()-v2.y()*v1.z());
    v.setY(v1.z()*v2.x()-v2.z()*v1.x());
    v.setZ(v1.x()*v2.y()-v2.x()*v1.y());
    return v;
}

GlWidgetForData::GlWidgetForData(Window *p, QWidget *parent)
    : QOpenGLWidget(parent),
      m_program(0)
{

    p_fps="";
    fps_start=-1;
    fps=0;

    c_window=p;

//    startTimer(30);
    QWidget::setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true);

}

GlWidgetForData::~GlWidgetForData()
{
    cleanup();
}

QSize GlWidgetForData::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize GlWidgetForData::sizeHint() const
{
    return QSize(1600, 1600);
}



void GlWidgetForData::cleanup()
{
    makeCurrent();
    delete m_program;
    m_program = 0;
    doneCurrent();
}



void GlWidgetForData::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GlWidgetForData::cleanup);
    initializeOpenGLFunctions();
    glClearColor(1, 1, 1,1);

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    vshader->compileSourceFile("Shaders/vshader_2D_view.glsl");

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    fshader->compileSourceFile("Shaders/fshader_2D_view.glsl");


    m_program = new QOpenGLShaderProgram;
    m_program->addShader(vshader);
    m_program->addShader(fshader);

    m_program->link();
    m_program->bind();

    con_structure_opacity=m_program->uniformLocation("structure_opacity");
    con_mitoLabel_opacity=m_program->uniformLocation("mitoLabel_opacity");
    con_correction_opacity=m_program->uniformLocation("correction_opacity");

    con_shift=m_program->uniformLocation("shift");
    con_scale=m_program->uniformLocation("scale");


    con_window_size=m_program->uniformLocation("window_size");
    con_background_color=m_program->uniformLocation("background_color");

    con_imageSize=m_program->uniformLocation("imageSize");

    con_tex_neuron=m_program->uniformLocation("tex_neuron");
    con_tex_mito=m_program->uniformLocation("tex_mito");
    con_tex_structure=m_program->uniformLocation("tex_structure");
    con_tex_connection=m_program->uniformLocation("tex_connection");
    con_tex_mitoLabel=m_program->uniformLocation("tex_mitoLabel");
    con_tex_mitoLabel_original=m_program->uniformLocation("tex_mitoLabel_original");
    con_tex_correction=m_program->uniformLocation("tex_correction");
    con_tex_subset=m_program->uniformLocation("tex_subset");

    con_color_table_neuron=m_program->uniformLocation("color_table_neuron");
    con_color_table_mito=m_program->uniformLocation("color_table_mito");

    con_mito_thresh=m_program->uniformLocation("mito_thresh");
    con_mito_alpha=m_program->uniformLocation("mito_alpha");

    con_mousePos=m_program->uniformLocation("mousePos");
    con_brushSize=m_program->uniformLocation("brushSize");
    con_brushColor=m_program->uniformLocation("brushColor");
    con_structure_thresh=m_program->uniformLocation("structure_thresh");
    con_structure_prev=m_program->uniformLocation("structure_prev");

    con_thresh_control=m_program->uniformLocation("thresh_control");

    con_grayscale=m_program->uniformLocation("grayscale");
    con_work_type=m_program->uniformLocation("work_type");

    m_program->release();

}
void GlWidgetForData::ReadLabelColor(){

}
void GlWidgetForData::ReadColorTable(){

}
void GlWidgetForData::interpolation_type_change(int a){

}
void GlWidgetForData::paintGL()
{
//    qDebug()<<"start glwidget for data";
//    return;

    fps++;

    if(c_window->curDataIndex==-1){
        glBegin(GL_LINES);
            glColor3f(0.6,0.6,0.6);
            glVertex3f(-0.99,-0.99,1);
            glVertex3f(-0.99,0.99,1);
            glVertex3f(-0.99,0.99,1);
            glVertex3f(0.99,0.99,1);
            glVertex3f(0.99,0.99,1);
            glVertex3f(0.99,-0.99,1);
            glVertex3f(0.99,-0.99,1);
            glVertex3f(-0.99,-0.99,1);

        glEnd();

        return;
    }

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    m_program->bind();

    if(c_window->curDataIndex!=prevDataIndex){
        c_window->focusSpine=-1;
        prevDataIndex=c_window->curDataIndex;
        if(!curData->IsGLInit){
            glGenTextures(1,&curData->tex_neuron);
            glBindTexture(GL_TEXTURE_2D,curData->tex_neuron);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RED,
                            curData->ImageW,
                            curData->ImageH,
                            0,GL_RED,GL_UNSIGNED_SHORT,
                            curData->imageData1);


            glGenTextures(1,&curData->tex_mito);
            glBindTexture(GL_TEXTURE_2D,curData->tex_mito);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RED,
                            curData->ImageW,
                            curData->ImageH,
                            0,GL_RED,GL_UNSIGNED_SHORT,
                            curData->imageData2);

            glGenTextures(1,&curData->tex_structure);
            glBindTexture(GL_TEXTURE_2D,curData->tex_structure);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,
                            curData->ImageW,
                            curData->ImageH,
                            0,GL_RGBA,GL_UNSIGNED_BYTE,
                            0);

            glGenTextures(1,&curData->tex_mitoLabel);
            glBindTexture(GL_TEXTURE_2D,curData->tex_mitoLabel);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,
                            curData->ImageW,
                            curData->ImageH,
                            0,GL_RGBA,GL_UNSIGNED_BYTE,
                            0);

            glGenTextures(1,&curData->tex_mitoLabel_original);
            glBindTexture(GL_TEXTURE_2D,curData->tex_mitoLabel_original);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RED,
                            curData->ImageW,
                            curData->ImageH,
                            0,GL_RED,GL_UNSIGNED_BYTE,
                            0);



            glGenTextures(1,&curData->tex_correction);
            glBindTexture(GL_TEXTURE_2D,curData->tex_correction);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,
                            curData->ImageW,
                            curData->ImageH,
                            0,GL_RGBA,GL_UNSIGNED_BYTE,
                            0);

            glGenTextures(1,&curData->tex_subset);
            glBindTexture(GL_TEXTURE_2D,curData->tex_subset);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,
                            curData->ImageW,
                            curData->ImageH,
                            0,GL_RGBA,GL_UNSIGNED_BYTE,
                            0);


            glGenTextures(1,&curData->tex_color_table_neuron);
            glBindTexture(GL_TEXTURE_2D,curData->tex_color_table_neuron);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RED,
                            256,
                            256,
                            0,GL_RED,GL_FLOAT,
                            0);

            glGenTextures(1,&curData->tex_color_table_mito);
            glBindTexture(GL_TEXTURE_2D,curData->tex_color_table_mito);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RED,
                            256,
                            256,
                            0,GL_RED,GL_FLOAT,
                            0);



            glGenTextures(1,&curData->tex_connection);
            glBindTexture(GL_TEXTURE_2D,curData->tex_connection);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,
                            curData->ImageW,
                            curData->ImageH,
                            0,GL_RGBA,GL_UNSIGNED_BYTE,
                            0);


//            glTexImage1D(GL_TEXTURE_1D,0,GL_RED,65536,0,GL_RED,GL_FLOAT,0);



            curData->IsGLInit=true;
        }
        setStructureTex();
        setMitoLabelTex();
        setSubsetTex();


    }

    if(structureChanged){
        setStructureTex();
        structureChanged=false;
    }
    if(mitoLabelChanged){
        setMitoLabelTex();
        mitoLabelChanged=false;
    }
    if(correctionChanged){
//        setCorrectionTex();
        correctionChanged=false;
    }
    if(subsetChanged){
        setSubsetTex();
        subsetChanged=false;
    }

    if(connectionChanged){
        setConnectionTex();
        connectionChanged=false;
    }

    if(colorTableChanged){
        glBindTexture(GL_TEXTURE_2D,curData->tex_color_table_neuron);
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,256,256,GL_RED,GL_FLOAT,curData->color_table_neuron);
        glBindTexture(GL_TEXTURE_2D,curData->tex_color_table_mito);
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,256,256,GL_RED,GL_FLOAT,curData->color_table_mito);

        for(int i=0;i<Insets_mito.size();i++){
            Insets_mito[i]->colorTableChanged=true;
        }


        colorTableChanged=false;
    }


    glActiveTexture(GL_TEXTURE0);
    glUniform1i(con_tex_neuron,0);
    glBindTexture(GL_TEXTURE_2D,curData->tex_neuron);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(con_tex_mito,1);
    glBindTexture(GL_TEXTURE_2D,curData->tex_mito);

    glActiveTexture(GL_TEXTURE2);
    glUniform1i(con_tex_structure,2);
    glBindTexture(GL_TEXTURE_2D,curData->tex_structure);


    glActiveTexture(GL_TEXTURE3);
    glUniform1i(con_tex_mitoLabel,3);
    glBindTexture(GL_TEXTURE_2D,curData->tex_mitoLabel);

    glActiveTexture(GL_TEXTURE4);
    glUniform1i(con_color_table_neuron,4);
    glBindTexture(GL_TEXTURE_2D,curData->tex_color_table_neuron);

    glActiveTexture(GL_TEXTURE5);
    glUniform1i(con_color_table_mito,5);
    glBindTexture(GL_TEXTURE_2D,curData->tex_color_table_mito);

    glActiveTexture(GL_TEXTURE6);
    glUniform1i(con_tex_correction,6);
    glBindTexture(GL_TEXTURE_2D,curData->tex_correction);

    glActiveTexture(GL_TEXTURE7);
    glUniform1i(con_tex_subset,7);
    glBindTexture(GL_TEXTURE_2D,curData->tex_subset);

    glActiveTexture(GL_TEXTURE8);
    glUniform1i(con_tex_connection,8);
    glBindTexture(GL_TEXTURE_2D,curData->tex_connection);

    glActiveTexture(GL_TEXTURE9);
    glUniform1i(con_tex_mitoLabel_original,9);
    glBindTexture(GL_TEXTURE_2D,curData->tex_mitoLabel_original);


    m_program->setUniformValue(con_window_size,QVector2D(this->width(),this->height()));
    m_program->setUniformValue(con_background_color,curData->background_color);
    m_program->setUniformValue(con_shift,curData->shift);
    m_program->setUniformValue(con_scale,curData->scale);
    m_program->setUniformValue(con_imageSize,QVector2D(curData->ImageW,curData->ImageH));
    m_program->setUniformValue(con_mousePos,mousePoint);
    m_program->setUniformValue(con_structure_thresh,curData->structure_thresh);

    if(c_window->imageControlPartLayout->currentIndex()==5 && curData->subsetInputType!=0 && !IsSpacePressed){
        m_program->setUniformValue(con_brushSize,curData->brushSize);
    }
//    else if(c_window->imageControlPartLayout->currentIndex()==3 && curData->correctorInputType!=-1
//            && curData->correctorInputType!=4 && !IsSpacePressed){
//        m_program->setUniformValue(con_brushSize,curData->brushSize);
//    }
    else m_program->setUniformValue(con_brushSize,int(0));

    if(c_window->imageControlPartLayout->currentIndex()==5){
        if(curData->subsetInputType==1){
            m_program->setUniformValue(con_brushColor,QVector3D(0,1,0));
        }
        else if(curData->subsetInputType==2){
            m_program->setUniformValue(con_brushColor,QVector3D(1,0,0));
        }
        else m_program->setUniformValue(con_brushColor,QVector3D(0,0,1));
    }
//    m_program->setUniformValue(con_structure_prev,int(0));
//    if(c_window->imageControlPartLayout->currentIndex()==3){
//        if(curData->correctorInputType==0){
//            m_program->setUniformValue(con_structure_prev,int(2));
//            m_program->setUniformValue(con_brushColor,QVector3D(1,0,0));
//        }
//        else if(curData->correctorInputType==1){
//            m_program->setUniformValue(con_structure_prev,int(1));
//            m_program->setUniformValue(con_brushColor,QVector3D(120.0/255,50.0/255,220.0/255));
//        }
//        else if(curData->correctorInputType==2){
//            m_program->setUniformValue(con_structure_prev,int(1));
//            m_program->setUniformValue(con_brushColor,QVector3D(255.0/255,142.0/255,138.0/255));
//        }
//        else if(curData->correctorInputType==3){
//            m_program->setUniformValue(con_structure_prev,int(1));
//            m_program->setUniformValue(con_brushColor,QVector3D(30.0/255,126.0/255,220.0/255));
//        }
//        else if(curData->correctorInputType==5){
//            m_program->setUniformValue(con_brushColor,QVector3D(0,220.0/255,220.0/255));
//        }
//        else if(curData->correctorInputType==6){
//            m_program->setUniformValue(con_brushColor,QVector3D(0,50.0/255,100.0/255));
//        }
//        else if(curData->correctorInputType==10){
//            m_program->setUniformValue(con_brushColor,QVector3D(0,0,0));
//        }
//        else if(curData->correctorInputType==11){
//            m_program->setUniformValue(con_brushColor,QVector3D(1,0,0));
//        }
//        else m_program->setUniformValue(con_brushColor,QVector3D(0,0,1));
//    }



    m_program->setUniformValue(con_thresh_control,thresh_control);

    m_program->setUniformValue(con_grayscale,c_window->grayscale_colormap->isChecked());
    m_program->setUniformValue(con_work_type,c_window->imageControlPartLayout->currentIndex());


    glBegin(GL_QUADS);
        glVertex3f(-1,-1,1);
        glVertex3f(-1,1,1);
        glVertex3f(1,1,1);
        glVertex3f(1,-1,1);
    glEnd();

    m_program->release();

    QPainter painter(this);
    painter.setPen(QColor(255-background_color.x()*255,255-background_color.y()*255,255-background_color.z()*255));
//    painter.drawText(30, 30, "FPS: " + p_fps);

    if(measure){
//        QPen tpen;
//        tpen.setWidth(2);
//        tpen.setColor(QColor(0,0,255));
        QVector2D p1=measureStart
                -QVector2D(this->width()-curData->ImageW*curData->scale,this->height()-curData->ImageH*curData->scale)/2
                -curData->shift*curData->scale;
        p1=p1/curData->scale;

        QVector2D p2=measureEnd
                -QVector2D(this->width()-curData->ImageW*curData->scale,this->height()-curData->ImageH*curData->scale)/2
                -curData->shift*curData->scale;
        p2=p2/curData->scale;

        if((p1-p2).length()>0.001){
            painter.drawLine(measureStart.x(),measureStart.y(),
                             measureEnd.x(),measureEnd.y());
            painter.drawText(measureStart.x(),measureStart.y(),QString::number((p1-p2).length()*curData->resolution,'g',4)+" micron");
        }
    }
    painter.end();

    if(c_window->imageControlPartLayout->currentIndex()==3 && curData->correctorInputType>=0 && curData->correctorInputType<4){
//        drawVisualGuideForStructure();
    }

//    drawSelectedLines();

//    if(curData->doTracing){
    if(curData->doTracing || c_window->show_tracing_map->isChecked()){
        drawTracingLines();
    }
    //qDebug()<<"end glwidget for data";

//    insetVisualization_mito();

    radarVisualization();


}
void GlWidgetForData::radarVisualization(){
//    qDebug()<<"radar vis";
    QPainter painter(this);

    if(c_window->curDataIndex==-1)return;
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    if(c_window->imageControlPartLayout->currentIndex()==3 && curData->correctorInputType>=0 && curData->correctorInputType<4){

        float radar_size=150;

        for(int i=0;i<curData->radars.size();i++){
            int p=curData->radars[i]->target_point;
            QVector2D target_pos=QVector2D(p%curData->ImageW,p/curData->ImageW);

            float dis=(mousePoint-target_pos).length();
            if(dis>radar_size*0.5){
                curData->radars[i]->is_focused=false;
            }
            QPoint parentPos=this->mapToGlobal(QPoint(0,0));


            QVector2D location=QVector2D(curData->radars[i]->target_point%curData->ImageW,curData->radars[i]->target_point/curData->ImageW)
                                         *curData->scale
                    + QVector2D(this->width()-curData->ImageW*curData->scale,this->height()-curData->ImageH*curData->scale)*0.5
                    + curData->shift*curData->scale;

            if(curData->focus_fixed_point!=curData->radars[i]->target_point){
                curData->radars[i]->is_focus_fixed=false;
            }

            if(location.x()<0 || location.x()>=this->width()
                    || location.y()<0 || location.y()>=this->height()){
                curData->radars[i]->is_focused=false;
                curData->radars[i]->is_focus_fixed=false;
                curData->radars[i]->hide();
                for(int j=0;j<4;j++){
                    if(curData->radars[i]->recommendation_insets[j]!=NULL){
                        curData->radars[i]->recommendation_insets[j]->hide();
                        curData->radars[i]->recommendation_insets[j]->update();
                    }
                }
            }
            else{

                //Middle point
                {
                    QPen t_pen;
                    t_pen.setColor(QColor(0,0,159,255));
                    if(curData->radars[i]->recommendation_insets[0]!=NULL){
                        t_pen.setColor(QColor(255,225,0,255));
                    }

                    int size=2;
                    t_pen.setWidth(2);
                    if(curData->radars[i]->is_focus_fixed){
                        size=4;
                        t_pen.setWidth(4);
                    }
                    painter.setPen(t_pen);
                    painter.drawLine(location.x()-size*2,location.y(),location.x()+size*2,location.y());
                    painter.drawLine(location.x()-size,location.y()+sqrt(3)*size,location.x()+size,location.y()-sqrt(3)*size);
                    painter.drawLine(location.x()-size,location.y()-sqrt(3)*size,location.x()+size,location.y()+sqrt(3)*size);

                }

                if(curData->radars[i]->is_focus_fixed){
                    curData->radars[i]->setGeometry(location.x() + parentPos.x() - radar_size * curData->scale*0.5,location.y()+parentPos.y()-radar_size * curData->scale*0.5
                                           ,radar_size * curData->scale,radar_size * curData->scale);
                    curData->radars[i]->show();
                    curData->radars[i]->update();
                }
                else{
                    curData->radars[i]->is_focused=false;
                    curData->radars[i]->is_focus_fixed=false;
                    curData->radars[i]->hide();
                    for(int j=0;j<4;j++){
                        if(curData->radars[i]->recommendation_insets[j]!=NULL){
                            curData->radars[i]->recommendation_insets[j]->hide();
                            curData->radars[i]->recommendation_insets[j]->update();
                        }
                    }
                }

//                curData->radars[i]->setGeometry(location.x() + parentPos.x() - radar_size * curData->scale*0.5,location.y()+parentPos.y()-radar_size * curData->scale*0.5
//                                       ,radar_size * curData->scale,radar_size * curData->scale);
//                curData->radars[i]->show();
            }
//            curData->radars[i]->update();

        }
    }
    else{
        curData->focus_fixed_point=-1;
        for(int i=0;i<curData->radars.size();i++){
            curData->radars[i]->is_focused=false;
            curData->radars[i]->is_focus_fixed=false;
            curData->radars[i]->hide();
            curData->radars[i]->update();
            for(int j=0;j<4;j++){
                if(curData->radars[i]->recommendation_insets[j]!=NULL){
                    curData->radars[i]->recommendation_insets[j]->hide();
                    curData->radars[i]->recommendation_insets[j]->update();
                }
            }

        }

    }
    painter.end();

}
void GlWidgetForData::insetClear(){

    for(int i=Insets_mito.size()-1;i>=0;i--){
        delete Insets_mito[i];
        Insets_mito.remove(i);
        InsetIndexs_mito.remove(i);
        InsetLocations_mito.remove(i);
        InsetSizes_mito.remove(i);
    }
}
void GlWidgetForData::insetVisualization_mito(){

    // check current insets' uncertainty
    // remove low uncertainty insets
    // check other mitochondria's uncertainity
    // generate new insets which have high uncertainty
    // move insets

//    qDebug()<<"inset vis start";

    if(c_window->curDataIndex==-1)return;
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];


    bool *inset_check=new bool[curData->mitoData.length()];
    float *uncertainty=new float[curData->mitoData.length()];

    for(int i=0;i<curData->mitoData.length();i++){
        uncertainty[i]=1-diceScore(i);
        inset_check[i]=false;
    }

//    qDebug()<<"uncertainty calculated";

    if(insetClearTrigger){
//        insetClear();
        insetClearTrigger=false;
        current_inset=-1;
    }

    for(int i=0;i<Insets_mito.size();i++){
        inset_check[InsetIndexs_mito[i]]=true;
        if(uncertainty[InsetIndexs_mito[i]]<curData->error_thresh){
            if(Insets_mito[i]->mitoIndex==current_inset){
                current_inset=-1;
            }
            delete Insets_mito[i];
            Insets_mito.remove(i);
            InsetIndexs_mito.remove(i);
            InsetLocations_mito.remove(i);
            InsetSizes_mito.remove(i);
            i--;
        }
        else{
//            InsetSizes_mito[i]=..;
        }
    }

//    qDebug()<<"current inset check";


    if(Insets_mito.size()<15){

        int max_index=-1;
        float max_error=0;
        for(int i=0;i<curData->mitoData.length();i++){
            if(inset_check[i]==false){
                if(uncertainty[i]>=curData->error_thresh){
                    if(uncertainty[i]>max_error){
                        max_error=uncertainty[i];
                        max_index=i;
                    }
                }
            }
        }
        if(max_index!=-1){
            Mito t=curData->mitoData[max_index];

            float new_size=sqrt((t.xmax-t.xmin)*(t.xmax-t.xmin)+(t.ymax-t.ymin)*(t.ymax-t.ymin))*0.5;
            int new_xmin=(t.xmax+t.xmin)*0.5 - new_size;
            int new_xmax=(t.xmax+t.xmin)*0.5 + new_size;
            int new_ymin=(t.ymax+t.ymin)*0.5 - new_size;
            int new_ymax=(t.ymax+t.ymin)*0.5 + new_size;
            if(new_xmin<0)new_xmin=0;
            if(new_ymin<0)new_ymin=0;
            if(new_xmax>=curData->ImageW)new_xmax=curData->ImageW-1;
            if(new_ymax>=curData->ImageH)new_ymax=curData->ImageH-1;

            while((new_xmax-new_xmin)%4!=0 || (new_xmax-new_xmin)<4){
                if(new_xmax<curData->ImageW-1)new_xmax++;
                else new_xmin--;
            }
            while((new_ymax-new_ymin)%4!=0 || (new_ymax-new_ymin)<4){
                if(new_ymax<curData->ImageH-1)new_ymax++;
                else new_ymin--;
            }
            QVector2D insetStart=QVector2D(new_xmin,new_ymin);
            QVector2D insetEnd=QVector2D(new_xmax,new_ymax);




            int insetSize=(insetEnd-insetStart).length()*0.5*2 * uncertainty[max_index];
            if(insetSize>100)insetSize=100;
            InsetSizes_mito.push_back(insetSize);


            GlWidgetForInset *newInset=new GlWidgetForInset(c_window,curData,t.index,1,insetStart,insetEnd,this);
            Insets_mito.push_back(newInset);

            newInset->setParent(this);


            QVector2D location=(insetStart+insetEnd)*0.5*curData->scale
                    + QVector2D(this->width()-curData->ImageW*curData->scale,this->height()-curData->ImageH*curData->scale)*0.5
                    + curData->shift*curData->scale;
            InsetLocations_mito.push_back(location);

            InsetIndexs_mito.push_back(max_index);
        }

    }
    //curData->mitoLabelImage[tP]==t.index
//    qDebug()<<"new inset generation";






    QPainter painter(this);
    QPen boundaryPen(QColor(255,255,0,150),2,Qt::DashLine);
    QPen leaderlinePen(QColor(255,255,0,200),3,Qt::SolidLine);
    QPen leaderlinePen_nofocus(QColor(255,255,0,50),3,Qt::SolidLine);

    painter.setBrush(QBrush(QColor(255,255,0,30),Qt::SolidPattern));
    QPoint parentPos=this->mapToGlobal(QPoint(0,0));

    for(int i=0;i<Insets_mito.size();i++){

//        InsetLocations_mito[i]=(Insets_mito[i]->inset_startPos+Insets_mito[i]->inset_endPos)*0.5*curData->scale
//                + QVector2D(this->width()-curData->ImageW*curData->scale,this->height()-curData->ImageH*curData->scale)*0.5
//                + curData->shift*curData->scale;

        Insets_mito[i]->setGeometry(InsetLocations_mito[i].x() + parentPos.x() -InsetSizes_mito[i],InsetLocations_mito[i].y()+parentPos.y()-InsetSizes_mito[i]
                               ,InsetSizes_mito[i]*2,InsetSizes_mito[i]*2);
        Insets_mito[i]->show();
        Insets_mito[i]->update();

        QVector2D p_start=Insets_mito[i]->inset_startPos*curData->scale
                + QVector2D(this->width()-curData->ImageW*curData->scale,this->height()-curData->ImageH*curData->scale)*0.5
                + curData->shift*curData->scale;

        QVector2D p_end=Insets_mito[i]->inset_endPos*curData->scale
                + QVector2D(this->width()-curData->ImageW*curData->scale,this->height()-curData->ImageH*curData->scale)*0.5
                + curData->shift*curData->scale;

        QVector2D p_middle=(p_start+p_end)*0.5;
        int p_size=(p_end-p_start).length()*0.5;

        if(Insets_mito[i]->mitoIndex==current_inset){
            painter.setPen(leaderlinePen);
        }
        else{
            painter.setPen(leaderlinePen_nofocus);
        }
        painter.drawLine(InsetLocations_mito[i].x(),InsetLocations_mito[i].y(),p_middle.x(),p_middle.y());
        painter.setPen(boundaryPen);
        painter.drawEllipse(QPoint(p_middle.x(),p_middle.y()),p_size,p_size);

    }
//    qDebug()<<"inset render";


    painter.end();

    delete []inset_check;
    delete []uncertainty;


}

void GlWidgetForData::drawVisualGuideForStructure(){
    if(c_window->curDataIndex==-1)return;
    QPainter painter(this);
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
    painter.setPen(QPen(QColor(255,0,0,180*curData->structure_opacity),3,Qt::DotLine));
    bool *ck=new bool[curData->ImageW * curData->ImageH]();
    for(int iy=1;iy<curData->ImageH-1;iy++){
        for(int ix=1;ix<curData->ImageW-1;ix++){
            int tP=iy*curData->ImageW + ix;
            if(ck[tP])continue;
            if(curData->structureData[tP]==2){
                if(curData->structureData[tP-1]==3 ||
                    curData->structureData[tP+1]==3 ||
                    curData->structureData[tP-curData->ImageW]==3 ||
                    curData->structureData[tP+curData->ImageW]==3){
                    QVector2D newP=getCandidateMiddlePoint(ix,iy,ck);
                    float p_screen_x=newP.x()*curData->scale+curData->shift.x()*curData->scale+(this->width()-curData->ImageW*curData->scale)/2.0;
                    float p_screen_y=newP.y()*curData->scale+curData->shift.y()*curData->scale+(this->height()-curData->ImageH*curData->scale)/2.0;

                    painter.drawEllipse(QPointF(p_screen_x,p_screen_y),20,20);

                }
            }
            else if(curData->structureData[tP]==3){
                if(curData->structureData[tP-1]==2 ||
                    curData->structureData[tP+1]==2 ||
                    curData->structureData[tP-curData->ImageW]==2 ||
                    curData->structureData[tP+curData->ImageW]==2){
                    QVector2D newP=getCandidateMiddlePoint(ix,iy,ck);
                    float p_screen_x=newP.x()*curData->scale+curData->shift.x()*curData->scale+(this->width()-curData->ImageW*curData->scale)/2.0;
                    float p_screen_y=newP.y()*curData->scale+curData->shift.y()*curData->scale+(this->height()-curData->ImageH*curData->scale)/2.0;

                    painter.drawEllipse(QPointF(p_screen_x,p_screen_y),20,20);

                }
            }
        }
    }
    delete []ck;
    painter.end();

}
QVector2D GlWidgetForData::getCandidateMiddlePoint(int sx,int sy,bool *ck){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    int cnt=1;
    QVector2D midPoint=QVector2D(sx,sy);
    QQueue<int> quex,quey;
    quex.enqueue(sx);
    quey.enqueue(sy);
    QQueue<int> quex2,quey2;
    quex2.enqueue(sx);
    quey2.enqueue(sy);

    ck[sy*curData->ImageW+sx]=true;

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();

        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<=0 || curx+dxylist[d][0]>=curData->ImageW-1 || cury+dxylist[d][1]<=0 || cury+dxylist[d][1]>=curData->ImageH-1){
                continue;
            }
            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            if(ck[tP]==false){
                ck[tP]=true;
                if(curData->structureData[tP]==2){
                    if(curData->structureData[tP-1]==3 ||
                        curData->structureData[tP+1]==3 ||
                        curData->structureData[tP-curData->ImageW]==3 ||
                        curData->structureData[tP+curData->ImageW]==3){
                        quex.enqueue(curx+dxylist[d][0]);
                        quey.enqueue(cury+dxylist[d][1]);
                        quex2.enqueue(curx+dxylist[d][0]);
                        quey2.enqueue(cury+dxylist[d][1]);

                        midPoint+=QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]);
                        cnt++;

                    }
                }
                else if(curData->structureData[tP]==3){
                    if(curData->structureData[tP-1]==2 ||
                        curData->structureData[tP+1]==2 ||
                        curData->structureData[tP-curData->ImageW]==2 ||
                        curData->structureData[tP+curData->ImageW]==2){
                        quex.enqueue(curx+dxylist[d][0]);
                        quey.enqueue(cury+dxylist[d][1]);
                        quex2.enqueue(curx+dxylist[d][0]);
                        quey2.enqueue(cury+dxylist[d][1]);
                        midPoint+=QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]);
                        cnt++;
                    }
                }
            }
        }
    }
    midPoint=midPoint/cnt;
    int curx=quex2.dequeue();
    int cury=quey2.dequeue();
    QVector2D medianPoint=QVector2D(curx,cury);
    float minDis=(medianPoint-midPoint).length();
    while(!quex2.empty()){
        curx=quex2.dequeue();
        cury=quey2.dequeue();
        if(minDis>(QVector2D(curx,cury)-midPoint).length()){
            medianPoint=QVector2D(curx,cury);
            minDis=(medianPoint-midPoint).length();
        }
    }

    return medianPoint;

}


void GlWidgetForData::drawSelectedLines(){
    QPainter painter(this);

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    for(int l=0;l<curData->selected_neurite_on_minimap.size();l++){

        int tP=curData->selected_neurite_on_minimap[l];
        if(curData->tracingMap[tP].type>0){
            for(int i=0;i<curData->tracingMap[tP].connected_nodes.size();i++){
                float startx=tP%curData->ImageW + 0.5;
                float starty=tP/curData->ImageW + 0.5;
                float endx=curData->tracingMap[tP].connected_nodes[i]%curData->ImageW + 0.5;
                float endy=curData->tracingMap[tP].connected_nodes[i]/curData->ImageW + 0.5;

                float p_screen_x_start=startx*curData->scale+curData->shift.x()*curData->scale+(this->width()-curData->ImageW*curData->scale)/2.0;
                float p_screen_y_start=starty*curData->scale+curData->shift.y()*curData->scale+(this->height()-curData->ImageH*curData->scale)/2.0;
                float p_screen_x_end=endx*curData->scale+curData->shift.x()*curData->scale+(this->width()-curData->ImageW*curData->scale)/2.0;
                float p_screen_y_end=endy*curData->scale+curData->shift.y()*curData->scale+(this->height()-curData->ImageH*curData->scale)/2.0;
                QPen tpen;
                tpen.setColor(QColor(255,255,0,120));
                tpen.setWidth(30);
                painter.setPen(tpen);
                painter.drawLine(p_screen_x_start,p_screen_y_start,p_screen_x_end,p_screen_y_end);

            }

        }
    }
    painter.end();

}

void GlWidgetForData::drawTracingLines(){
    QPainter painter(this);

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            if(curData->tracingMap[tP].type>0){
                for(int i=0;i<curData->tracingMap[tP].connected_nodes.size();i++){

                    bool is_weak=false;
                    for(int j=0;j<curData->tracingMap[tP].weak_nodes.size();j++){
                        if(curData->tracingMap[tP].weak_nodes[j]==curData->tracingMap[tP].connected_nodes[i]){
                            is_weak=true;
                            QPen tpen;
                            tpen.setColor(QColor(255,0,0,curData->correction_opacity*255));
                            tpen.setWidth(2);
                            painter.setPen(tpen);
                            for(int k=1;k<curData->tracingMap[tP].weak_paths[j].size();k++){
                                float startx=curData->tracingMap[tP].weak_paths[j][k-1]%curData->ImageW + 0.5;
                                float starty=curData->tracingMap[tP].weak_paths[j][k-1]/curData->ImageW + 0.5;
                                float endx=curData->tracingMap[tP].weak_paths[j][k]%curData->ImageW + 0.5;
                                float endy=curData->tracingMap[tP].weak_paths[j][k]/curData->ImageW + 0.5;

                                float p_screen_x_start=startx*curData->scale+curData->shift.x()*curData->scale+(this->width()-curData->ImageW*curData->scale)/2.0;
                                float p_screen_y_start=starty*curData->scale+curData->shift.y()*curData->scale+(this->height()-curData->ImageH*curData->scale)/2.0;
                                float p_screen_x_end=endx*curData->scale+curData->shift.x()*curData->scale+(this->width()-curData->ImageW*curData->scale)/2.0;
                                float p_screen_y_end=endy*curData->scale+curData->shift.y()*curData->scale+(this->height()-curData->ImageH*curData->scale)/2.0;
                                painter.drawLine(p_screen_x_start,p_screen_y_start,p_screen_x_end,p_screen_y_end);

                            }


                            break;
                        }
                    }

                    if(is_weak==false){
                        float startx=ix + 0.5;
                        float starty=iy + 0.5;
                        float endx=curData->tracingMap[tP].connected_nodes[i]%curData->ImageW + 0.5;
                        float endy=curData->tracingMap[tP].connected_nodes[i]/curData->ImageW + 0.5;

                        float p_screen_x_start=startx*curData->scale+curData->shift.x()*curData->scale+(this->width()-curData->ImageW*curData->scale)/2.0;
                        float p_screen_y_start=starty*curData->scale+curData->shift.y()*curData->scale+(this->height()-curData->ImageH*curData->scale)/2.0;
                        float p_screen_x_end=endx*curData->scale+curData->shift.x()*curData->scale+(this->width()-curData->ImageW*curData->scale)/2.0;
                        float p_screen_y_end=endy*curData->scale+curData->shift.y()*curData->scale+(this->height()-curData->ImageH*curData->scale)/2.0;
                        QPen tpen;
                        tpen.setColor(QColor(255,226,39,curData->correction_opacity*255));
                        tpen.setWidth(2);
                        painter.setPen(tpen);
                        painter.drawLine(p_screen_x_start,p_screen_y_start,p_screen_x_end,p_screen_y_end);
                    }

                }
                float p_screen_x=(ix+0.5)*curData->scale+curData->shift.x()*curData->scale+(this->width()-curData->ImageW*curData->scale)/2.0;
                float p_screen_y=(iy+0.5)*curData->scale+curData->shift.y()*curData->scale+(this->height()-curData->ImageH*curData->scale)/2.0;
                QPen tpen;
                if(curData->tracingMap[tP].weak_nodes.size()>0){
                    tpen.setColor(QColor(0,255,225,curData->correction_opacity*255));
                }
                else{
                    tpen.setColor(QColor(239,247,225,curData->correction_opacity*255));
                }

                if(curData->tracingMap[tP].type==6){
                    tpen.setColor(QColor(255,0,0,curData->correction_opacity*255));
                }

                if(curData->tracingMap[tP].type==1 || curData->tracingMap[tP].type==4 || curData->tracingMap[tP].type==5){
                    tpen.setWidth(6);
                }
                else{
                    tpen.setWidth(12);
                }
                painter.setPen(tpen);
                painter.drawPoint(p_screen_x,p_screen_y);

            }
        }
    }
    painter.end();

}
void GlWidgetForData::setConnectionTex(){
//    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

//    unsigned char *tData=new unsigned char[curData->ImageW*curData->ImageH*4];

//    int objectIndex=0;
//    if(mousePoint.x()>=0 && mousePoint.x()<curData->ImageW
//            && mousePoint.y()>=0 && mousePoint.y()<curData->ImageH){
//        objectIndex=curData->objectIndex[int(mousePoint.y())*curData->ImageW+int(mousePoint.x())];
//    }

//    if(objectIndex!=0){
//        for(int iy=0;iy<curData->ImageH;iy++){
//            for(int ix=0;ix<curData->ImageW;ix++){
//                int tP=iy*curData->ImageW+ix;
//                int tS=curData->connection_graph[tP];
//                int tS1=curData->connection_object1[tP];
//                int tS2=curData->connection_object2[tP];
//                int type=-1;

//                if(tS!=0 && (tS1==objectIndex || tS2==objectIndex)){
//                    type=curData->type_of_paths[tS1*curData->max_object_index+tS2];
//                }
//                if(type==-1){
//                    tData[tP*4+0]=0;
//                    tData[tP*4+1]=0;
//                    tData[tP*4+2]=0;
//                    tData[tP*4+3]=0;

//                }
//                else if(type==2){
//                    tData[tP*4+0]=0;
//                    tData[tP*4+1]=255;
//                    tData[tP*4+2]=255;
//                    tData[tP*4+3]=255*curData->connection_visibility;

//                }
//                else if(type==0 || type==1){
//                    tData[tP*4+0]=0;
//                    tData[tP*4+1]=0;
//                    tData[tP*4+2]=255;
//                    tData[tP*4+3]=255*curData->connection_visibility;
//                }
//                else if(type==3 || type==5 || type==6){
//                    tData[tP*4+0]=255;
//                    tData[tP*4+1]=0;
//                    tData[tP*4+2]=0;
//                    tData[tP*4+3]=255*curData->connection_visibility;
//                }
//                else if(type==4 || type==7 || type==8){
//                    tData[tP*4+0]=0;
//                    tData[tP*4+1]=255;
//                    tData[tP*4+2]=255;
//                    tData[tP*4+3]=255*curData->connection_visibility;
//                }

//            }
//        }
//    }
//    else{
//        for(int iy=0;iy<curData->ImageH;iy++){
//            for(int ix=0;ix<curData->ImageW;ix++){
//                int tP=iy*curData->ImageW+ix;
//                int tS=curData->connection_graph[tP];
//                int tS1=curData->connection_object1[tP];
//                int tS2=curData->connection_object2[tP];
//                int type=-1;
//                if(tS!=0){
//                    type=curData->type_of_paths[tS1*curData->max_object_index+tS2];
//                }
//                if(type==-1){
//                    tData[tP*4+0]=0;
//                    tData[tP*4+1]=0;
//                    tData[tP*4+2]=0;
//                    tData[tP*4+3]=0;

//                }
//                else if(type==2){
//                    tData[tP*4+0]=0;
//                    tData[tP*4+1]=255;
//                    tData[tP*4+2]=255;
//                    tData[tP*4+3]=255*curData->connection_visibility;

//                }
//                else if(type==0 || type==1){
//                    tData[tP*4+0]=0;
//                    tData[tP*4+1]=0;
//                    tData[tP*4+2]=255;
//                    tData[tP*4+3]=255*curData->connection_visibility;
//                }
//                else if(type==3 || type==5 || type==6){
//                    tData[tP*4+0]=255;
//                    tData[tP*4+1]=0;
//                    tData[tP*4+2]=0;
//                    tData[tP*4+3]=255*curData->connection_visibility;
//                }
//                else if(type==4 || type==7 || type==8){
//                    tData[tP*4+0]=0;
//                    tData[tP*4+1]=255;
//                    tData[tP*4+2]=255;
//                    tData[tP*4+3]=255*curData->connection_visibility;
//                }
//            }
//        }
//    }
//    glBindTexture(GL_TEXTURE_2D,curData->tex_connection);
//    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,
//                    curData->ImageW,
//                    curData->ImageH,
//                    GL_RGBA,GL_UNSIGNED_BYTE,
//                    tData);
//    delete []tData;

}
void GlWidgetForData::setSubsetTex(){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    unsigned char *tData=new unsigned char[curData->ImageW*curData->ImageH*4]();
    if(c_window->imageControlPartLayout->currentIndex()==5){
        for(int iy=0;iy<curData->ImageH;iy++){
            for(int ix=0;ix<curData->ImageW;ix++){
                int tP=iy*curData->ImageW+ix;
                int tS=curData->subsetData[tP];
                if(tS==0){
                    tData[tP*4+0]=0;
                    tData[tP*4+1]=0;
                    tData[tP*4+2]=0;
                    tData[tP*4+3]=0;
                }
                else if(tS==1){
                    tData[tP*4+0]=0;
                    tData[tP*4+1]=255;
                    tData[tP*4+2]=0;
                    tData[tP*4+3]=128;
                }
            }
        }
    }
    glBindTexture(GL_TEXTURE_2D,curData->tex_subset);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,
                    curData->ImageW,
                    curData->ImageH,
                    GL_RGBA,GL_UNSIGNED_BYTE,
                    tData);
    delete []tData;

}
void GlWidgetForData::setStructureTex(){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    unsigned char *tData=new unsigned char[curData->ImageW*curData->ImageH*4];
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            int tS=curData->structureData[tP];

            if(tS==0){
                tS=curData->connectionStructureData[tP];
            }

            if(tS==0){
                tData[tP*4+0]=0;
                tData[tP*4+1]=0;
                tData[tP*4+2]=0;
                tData[tP*4+3]=0;
            }
//            else if(tS==100){
//                tData[tP*4+0]=0;
//                tData[tP*4+1]=255;
//                tData[tP*4+2]=255;
//                tData[tP*4+3]=100;
//            }
            else if(tS==10){
//                tData[tP*4+0]=100;
//                tData[tP*4+1]=200;
//                tData[tP*4+2]=255;
//                tData[tP*4+3]=curData->structure_opacity*255;

            }
            else{
                QColor tC=c_window->typeColors[tS];
                tData[tP*4+0]=tC.red();
                tData[tP*4+1]=tC.green();
                tData[tP*4+2]=tC.blue();
                tData[tP*4+3]=curData->structure_opacity*255;
            }
        }
    }


    //        if(thresh_control==1){
    //            if(value1<structure_thresh){
    //                cur_color=vec4(1,0,0,0.3);
    //                alpha=0.3;
    //            }
    //            else{
    //                cur_color=vec4(0,0,0.5,0.3);
    //                alpha=0.3;
    //            }
    //        }
    //        else if(thresh_control==2){
    //            if(value2<structure_thresh){
    //                cur_color=vec4(1,0,0,0.3);
    //                alpha=0.3;
    //            }
    //            else{
    //                cur_color=vec4(0,0,0.5,0.3);
    //                alpha=0.3;
    //            }
    //        }


    if(curData->correctorInputType>=0 && curData->correctorInputType<4 && c_window->imageControlPartLayout->currentIndex()==3 && !IsSpacePressed){
//        if(thresh_control==1){
//            for(int iy=0;iy<curData->ImageH;iy++){
//                for(int ix=0;ix<curData->ImageW;ix++){
//                    int tP=iy*curData->ImageW+ix;
//                    float v=curData->color_table_neuron[curData->imageData1[tP]];
//                    if(v<curData->structure_thresh){
//                        tData[tP*4+0]=tData[tP*4+0]*0.3;
//                        tData[tP*4+1]=tData[tP*4+1]*0.3;
//                        tData[tP*4+2]=tData[tP*4+2]*0.3;
//                        tData[tP*4+3]=curData->structure_opacity*255;
//                    }
//                    else{
//                        tData[tP*4+0]=tData[tP*4+0]*0.3;
//                        tData[tP*4+1]=tData[tP*4+1]*0.3;
//                        tData[tP*4+2]=tData[tP*4+2]*0.3 + 255*0.7;
//                        tData[tP*4+3]=curData->structure_opacity*255;
//                    }
//                }
//            }
//        }
//        else{
            QVector3D brushColor=QVector3D(0,0,0);
            if(curData->correctorInputType==0){
                brushColor=QVector3D(255,0,0);
            }
            else if(curData->correctorInputType==1){
                brushColor=QVector3D(120.0,50.0,220.0);
            }
            else if(curData->correctorInputType==2){
                brushColor=QVector3D(255.0,100.0,100.0);
            }
            else if(curData->correctorInputType==3){
                brushColor=QVector3D(30.0,126.05,255.0);
            }
            if(c_window->correction_type->currentIndex()==1){
                focusNeurite(mousePoint,tData,brushColor);
            }
            else{
                for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
                    for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                        if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                        int tP=iy*curData->ImageW+ix;

                        float dis=(QVector2D(ix,iy)-mousePoint).length();
                        if(dis<curData->brushSize){
                            if(curData->cc_buffer[tP]==2){
                                tData[tP*4+0]=tData[tP*4+0]*0.3 + brushColor.x()*0.7;
                                tData[tP*4+1]=tData[tP*4+1]*0.3 + brushColor.y()*0.7;
                                tData[tP*4+2]=tData[tP*4+2]*0.3 + brushColor.z()*0.7;
                                tData[tP*4+3]=curData->structure_opacity*255;
                            }
                            else{
                                tData[tP*4+0]=tData[tP*4+0]*0.8 + 255*0.2;
                                tData[tP*4+1]=tData[tP*4+1]*0.8 + 255*0.2;
                                tData[tP*4+2]=tData[tP*4+2]*0.8 + 255*0.2;
                                tData[tP*4+3]=curData->structure_opacity*255;
                            }
                        }
                    }
                }
            }
//        }
    }
    else if(curData->correctorInputType>=10 && curData->correctorInputType<14 && c_window->imageControlPartLayout->currentIndex()==4 && !IsSpacePressed){
        if(thresh_control==2){
            for(int iy=0;iy<curData->ImageH;iy++){
                for(int ix=0;ix<curData->ImageW;ix++){
                    int tP=iy*curData->ImageW+ix;
                    float v=curData->color_table_mito[curData->imageData2[tP]];
                    if(v<curData->structure_thresh){
                        tData[tP*4+0]=0;
                        tData[tP*4+1]=0;
                        tData[tP*4+2]=0;
                        tData[tP*4+3]=0.5*255;

                    }
                    else{
                        tData[tP*4+0]=255;
                        tData[tP*4+1]=0;
                        tData[tP*4+2]=0;
                        tData[tP*4+3]=0.7*255;
                    }
                }
            }
        }

    }


    //    m_program->setUniformValue(con_structure_prev,int(0));
    //    if(c_window->imageControlPartLayout->currentIndex()==3){
    //        if(curData->correctorInputType==0){
    //            m_program->setUniformValue(con_structure_prev,int(2));
    //            m_program->setUniformValue(con_brushColor,QVector3D(1,0,0));
    //        }
    //        else if(curData->correctorInputType==1){
    //            m_program->setUniformValue(con_structure_prev,int(1));
    //            m_program->setUniformValue(con_brushColor,QVector3D(120.0/255,50.0/255,220.0/255));
    //        }
    //        else if(curData->correctorInputType==2){
    //            m_program->setUniformValue(con_structure_prev,int(1));
    //            m_program->setUniformValue(con_brushColor,QVector3D(255.0/255,142.0/255,138.0/255));
    //        }
    //        else if(curData->correctorInputType==3){
    //            m_program->setUniformValue(con_structure_prev,int(1));
    //            m_program->setUniformValue(con_brushColor,QVector3D(30.0/255,126.0/255,220.0/255));
    //        }
    //        else if(curData->correctorInputType==5){
    //            m_program->setUniformValue(con_brushColor,QVector3D(0,220.0/255,220.0/255));
    //        }
    //        else if(curData->correctorInputType==6){
    //            m_program->setUniformValue(con_brushColor,QVector3D(0,50.0/255,100.0/255));
    //        }
    //        else if(curData->correctorInputType==10){
    //            m_program->setUniformValue(con_brushColor,QVector3D(0,0,0));
    //        }
    //        else if(curData->correctorInputType==11){
    //            m_program->setUniformValue(con_brushColor,QVector3D(1,0,0));
    //        }
    //        else m_program->setUniformValue(con_brushColor,QVector3D(0,0,1));
    //    }


    glBindTexture(GL_TEXTURE_2D,curData->tex_structure);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,
                    curData->ImageW,
                    curData->ImageH,
                    GL_RGBA,GL_UNSIGNED_BYTE,
                    tData);
    delete []tData;

}
void GlWidgetForData::focusNeurite(QVector2D p,unsigned char *buffer,QVector3D brushColor){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
    int curP=int(p.y())*curData->ImageW+int(p.x());
    int curx=p.x();
    int cury=p.y();

    if(curx<0 || cury<0 || curx>=curData->ImageW || cury>=curData->ImageH)return;

    if(curData->tracingMap[curP].type<0){
        curP=findNearestNode(curx,cury);
        if(curP==-1)return;
        curx=curP%curData->ImageW;
        cury=curP/curData->ImageW;
    }


    if(curData->tracingMap[curP].type==0){
        curx=curData->tracingMap[curP].posx;
        cury=curData->tracingMap[curP].posy;
    }
    curP=cury*curData->ImageW + curx;

    int target_neu=curData->tracingMap[curP].neurite_ind;

    QQueue<int> res;

    if(curData->tracingMap[curP].type==2 || curData->tracingMap[curP].type==3){
        res.enqueue(curP);
    }
    else if(curData->tracingMap[curP].type>0){
        QQueue<int> nodes;
        QQueue<int> nodes_recover;
        nodes.enqueue(curP);
        curData->tracingMap[curP].temp_ck=true;
        nodes_recover.enqueue(curP);

        while(!nodes.isEmpty()){
            int curP=nodes.dequeue();
            int curx=curP%curData->ImageW;
            int cury=curP/curData->ImageW;

            if(curData->tracingMap[curP].neurite_ind==target_neu){
                res.enqueue(cury*curData->ImageW + curx);
                for(int i=0;i<curData->tracingMap[curP].connected_nodes.size();i++){
                    if(curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].temp_ck==false){
                        curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].temp_ck=true;
                        nodes.enqueue(curData->tracingMap[curP].connected_nodes[i]);
                        nodes_recover.enqueue(curData->tracingMap[curP].connected_nodes[i]);
                    }
                }
            }
        }
        while(!nodes_recover.isEmpty()){
            int curP=nodes_recover.dequeue();
            curData->tracingMap[curP].temp_ck=false;
        }

    }

    while(!res.isEmpty()){
        int curP=res.dequeue();
        patchSet *patch=curData->tracingMap[curP].patch;
        int tW=patch->boundMax.x()-patch->boundMin.x()+1;
        int tH=patch->boundMax.y()-patch->boundMin.y()+1;

        bool *ck=new bool[tW*tH]();
        int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

        QQueue<int> quex,quey;
        quex.enqueue(patch->midx);
        quey.enqueue(patch->midy);
        ck[int((patch->midy-patch->boundMin.y())*tW+patch->midx-patch->boundMin.x())]=true;

        int tP=patch->midy*curData->ImageW + patch->midx;

        buffer[tP*4+0]=buffer[tP*4+0]*0.3 + brushColor.x()*0.7;
        buffer[tP*4+1]=buffer[tP*4+1]*0.3 + brushColor.y()*0.7;
        buffer[tP*4+2]=buffer[tP*4+2]*0.3 + brushColor.z()*0.7;
        buffer[tP*4+3]=curData->structure_opacity*255;


        while(!quex.empty()){
            int curx=quex.dequeue();
            int cury=quey.dequeue();
            for(int d=0;d<8;d++){
                int nextx=curx+dxylist[d][0];
                int nexty=cury+dxylist[d][1];
                if(nextx<patch->boundMin.x() || nextx>patch->boundMax.x()
                        || nexty<patch->boundMin.y() || nexty>patch->boundMax.y()){
                    continue;
                }
                if(curData->corrector->isInside(QVector2D(nextx,nexty),patch->patchCorner)==false)continue;

                int tP=nexty*curData->ImageW + nextx;
                int tP2=(nexty-patch->boundMin.y())*tW+nextx-patch->boundMin.x();
                if(ck[tP2]==false){
                    ck[tP2]=true;
                    if(curData->tracingMap[tP].type==0){
                        if(curData->tracingMap[tP].posy*curData->ImageW+curData->tracingMap[tP].posx==curP){
                            if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                                quex.enqueue(nextx);
                                quey.enqueue(nexty);
                                buffer[tP*4+0]=buffer[tP*4+0]*0.3 + brushColor.x()*0.7;
                                buffer[tP*4+1]=buffer[tP*4+1]*0.3 + brushColor.y()*0.7;
                                buffer[tP*4+2]=buffer[tP*4+2]*0.3 + brushColor.z()*0.7;
                                buffer[tP*4+3]=curData->structure_opacity*255;
                            }
                        }
                    }
                }
            }
        }
        delete []ck;
    }

}
int GlWidgetForData::findNearestNode(int s_x, int s_y){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    QQueue<int> quex,quey;
    quex.enqueue(s_x);
    quey.enqueue(s_y);
    QQueue<int> nodes_recover;

    QQueue<int> dis;
    dis.enqueue(0);
    curData->tracingMap[s_y*curData->ImageW + s_x].temp_ck=true;
    nodes_recover.enqueue(s_y*curData->ImageW + s_x);

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();
        int curdis=dis.dequeue();
        int nextdis=curdis+1;
        if(curdis>15){
            return -1;
        }

        for(int d=0;d<8;d++){
            int nextx=curx+dxylist[d][0];
            int nexty=cury+dxylist[d][1];
            if(nextx<=0 || nextx>=curData->ImageW-1 || nexty<=0 || nexty>=curData->ImageH-1){
                continue;
            }
            int tP=nexty*curData->ImageW + nextx;
            if(curData->tracingMap[tP].temp_ck){
                continue;
            }
            if(curData->tracingMap[tP].type>=0){
                while(!nodes_recover.isEmpty()){
                    int curP=nodes_recover.dequeue();
                    curData->tracingMap[curP].temp_ck=false;
                }
                return tP;
            }
            curData->tracingMap[tP].temp_ck=true;
            nodes_recover.enqueue(tP);
            quex.enqueue(nextx);
            quey.enqueue(nexty);
            dis.enqueue(nextdis);
        }
    }
    return -1;
}
void GlWidgetForData::correctNeurite(QVector2D p,int type){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
    int curP=int(p.y())*curData->ImageW+int(p.x());
    int curx=p.x();
    int cury=p.y();

    if(curx<0 || cury<0 || curx>=curData->ImageW || cury>=curData->ImageH)return;


    if(curData->tracingMap[curP].type<0){
        curP=findNearestNode(curx,cury);
        if(curP==-1)return;
        curx=curP%curData->ImageW;
        cury=curP/curData->ImageW;
    }


    if(curData->tracingMap[curP].type==0){
        curx=curData->tracingMap[curP].posx;
        cury=curData->tracingMap[curP].posy;
    }
    curP=cury*curData->ImageW + curx;
    int target_point=curP;
    int target_neu=curData->tracingMap[curP].neurite_ind;

    QQueue<int> res;

    if(curData->tracingMap[curP].type==2 || curData->tracingMap[curP].type==3){
        res.enqueue(curP);
    }
    else if(curData->tracingMap[curP].type>0){
        QQueue<int> nodes;
        QQueue<int> nodes_recover;
        nodes.enqueue(curP);
        curData->tracingMap[curP].temp_ck=true;
        nodes_recover.enqueue(curP);

        while(!nodes.isEmpty()){
            int curP=nodes.dequeue();
            int curx=curP%curData->ImageW;
            int cury=curP/curData->ImageW;

//            if(curData->tracingMap[curP].type==1 || curData->tracingMap[curP].type==4 || curData->tracingMap[curP].type==5){
            if(curData->tracingMap[curP].neurite_ind==target_neu){
                res.enqueue(cury*curData->ImageW + curx);
                for(int i=0;i<curData->tracingMap[curP].connected_nodes.size();i++){
                    if(curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].temp_ck==false){
                        curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].temp_ck=true;
                        nodes.enqueue(curData->tracingMap[curP].connected_nodes[i]);
                        nodes_recover.enqueue(curData->tracingMap[curP].connected_nodes[i]);
                    }
                }
            }
        }
        while(!nodes_recover.isEmpty()){
            int curP=nodes_recover.dequeue();
            curData->tracingMap[curP].temp_ck=false;
        }

    }

    float mean_x=0;
    float mean_y=0;
    int cnt=0;

    while(!res.isEmpty()){
        int curP=res.dequeue();
        curData->tracingMap[curP].corrected_type=type;
        patchSet *patch=curData->tracingMap[curP].patch;

        mean_x+=patch->midx;
        mean_y+=patch->midy;
        cnt++;


        int tW=patch->boundMax.x()-patch->boundMin.x()+1;
        int tH=patch->boundMax.y()-patch->boundMin.y()+1;

        bool *ck=new bool[tW*tH]();
        int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

        QQueue<int> quex,quey;
        quex.enqueue(patch->midx);
        quey.enqueue(patch->midy);



        ck[int((patch->midy-patch->boundMin.y())*tW+patch->midx-patch->boundMin.x())]=true;

        int tP=patch->midy*curData->ImageW + patch->midx;

        curData->structureData[tP]=type;
        curData->correctionData[tP]=type;



        while(!quex.empty()){
            int curx=quex.dequeue();
            int cury=quey.dequeue();
            for(int d=0;d<8;d++){
                int nextx=curx+dxylist[d][0];
                int nexty=cury+dxylist[d][1];
                if(nextx<patch->boundMin.x() ||nextx>patch->boundMax.x()
                        || nexty<patch->boundMin.y() || nexty>patch->boundMax.y()){
                    continue;
                }
                if(curData->corrector->isInside(QVector2D(nextx,nexty),patch->patchCorner)==false)continue;

                int tP=nexty*curData->ImageW + nextx;
                int tP2=(nexty-patch->boundMin.y())*tW+nextx-patch->boundMin.x();
                if(ck[tP2]==false){
                    ck[tP2]=true;
                    if(curData->tracingMap[tP].type==0){
                        if(curData->tracingMap[tP].posy*curData->ImageW+curData->tracingMap[tP].posx==curP){
                            if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                                quex.enqueue(nextx);
                                quey.enqueue(nexty);
                                curData->structureData[tP]=type;
                                curData->correctionData[tP]=type;
                            }
                        }
                    }
                }
            }
        }
        delete []ck;
    }

    if(cnt!=0){
        mean_x/=cnt;
        mean_y/=cnt;

        QVector <int> top_sim;
        QVector <int> top_dissim;

        int target_type=curData->structureData[target_point];
        getTopSimilarNeurites(curData->tracingMap[target_point].neurite_ind,target_type,&top_sim);
        getTopDissimilarNeurites(curData->tracingMap[target_point].neurite_ind,target_type,&top_dissim);


        if(curData->radar_indexs.contains(curData->tracingMap[target_point].neurite_ind)){
            FSRadar *curRadar=curData->radars[curData->radar_indexs[curData->tracingMap[target_point].neurite_ind]];
            curRadar->info_update(target_point,top_sim,top_dissim);
        }
        else{
            FSRadar *newRadar=new FSRadar(c_window,curData,target_point,top_sim,top_dissim,this);

            curData->radars.push_back(newRadar);

            newRadar->setParent(this);

            curData->radar_indexs[curData->tracingMap[target_point].neurite_ind]=curData->radars.size()-1;

            update();
        }

    }

}

void GlWidgetForData::getTopSimilarNeurites(int target_neurite, int target_type,QVector<int> *buffer){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    int ind1=curData->neurite_index_map[target_neurite];

    QVector <forSort_base> temp;
    for(int i=0;i<curData->similarity_matrix_width;i++){
        if(ind1==i)continue;
        forSort_base t;
        t.index=i;
        t.value=curData->similarity_matrix[ind1*curData->similarity_matrix_width+i];
        temp.push_back(t);
    }
    qSort(temp);
    int cnt=0;

    for(int i=temp.size()-1;i>=0;i--){
        if(temp[i].value<0.7)break;
        int neu=curData->similarity_matrix_neu_ind[temp[i].index];
        if(curData->structureData[curData->neurite_sample_point[neu]]!=target_type){
            qDebug()<<"similarity:"<<temp[i].value;
            //if(curData->neurite_probability_dend[neu]>0.4 && curData->neurite_probability_dend[neu]<0.6){
                qDebug()<<"uncertainty:"<<curData->neurite_probability_dend[neu];
                buffer->push_back(neu);
                cnt++;
            //}
        }
        if(cnt>=9){
            break;
        }
    }


}
void GlWidgetForData::getTopDissimilarNeurites(int target_neurite, int target_type, QVector<int> *buffer){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    int ind1=curData->neurite_index_map[target_neurite];

    QVector <forSort_base> temp;
    for(int i=0;i<curData->similarity_matrix_width;i++){
        if(ind1==i)continue;
        forSort_base t;
        t.index=i;
        t.value=curData->similarity_matrix[ind1*curData->similarity_matrix_width+i];
        temp.push_back(t);
    }
    qSort(temp);
    int cnt=0;

    for(int i=0;i<temp.size();i++){
        int neu=curData->similarity_matrix_neu_ind[temp[i].index];
        if(curData->neurite_changed[neu] || curData->neurite_res[neu]==target_type){
            buffer->push_back(neu);
            cnt++;
        }
        if(cnt>=9){
            break;
        }
    }
}

void GlWidgetForData::setCorrectionTex(){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
    unsigned char *tData=new unsigned char[curData->ImageW*curData->ImageH*4]();
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            int tS=curData->correctionData[tP];
            if(tS==1){
                tData[tP*4+0]=255;
                tData[tP*4+1]=0;
                tData[tP*4+2]=0;
                tData[tP*4+3]=curData->correction_opacity*255;
            }
            else if(tS==2){
                tData[tP*4+0]=42;
                tData[tP*4+1]=139;
                tData[tP*4+2]=89;
                tData[tP*4+3]=curData->correction_opacity*255;
            }
            else if(tS==3){
                tData[tP*4+0]=0;
                tData[tP*4+1]=255;
                tData[tP*4+2]=255;
                tData[tP*4+3]=curData->correction_opacity*255;
            }

            else if(tS==5){
                tData[tP*4+0]=0;
                tData[tP*4+1]=220;
                tData[tP*4+2]=220;
                tData[tP*4+3]=curData->correction_opacity*255;
            }
            else if(tS==6){
                tData[tP*4+0]=0;
                tData[tP*4+1]=50;
                tData[tP*4+2]=100;
                tData[tP*4+3]=curData->correction_opacity*255;
            }
            else if(tS==11){ //corrected
                tData[tP*4+0]=52;
                tData[tP*4+1]=235;
                tData[tP*4+2]=143;
                tData[tP*4+3]=curData->correction_opacity*255;
            }

            tS=curData->predictData[tP];

            if(tS!=0){
                QColor tC=c_window->typeColors[tS];
                tData[tP*4+0]=tC.red();
                tData[tP*4+1]=tC.green();
                tData[tP*4+2]=tC.blue();
                tData[tP*4+3]=curData->correction_opacity*255;
            }
        }
    }

//    for(int iy=0;iy<curData->ImageH;iy++){
//        for(int ix=0;ix<curData->ImageW;ix++){
//            int tP=iy*curData->ImageW+ix;
//            bool tD=curData->tracingMap[tP].debug_middle_node;
//            if(tD){
//                tData[tP*4+0]=0;
//                tData[tP*4+1]=tD;
//                tData[tP*4+2]=tD;
//                tData[tP*4+3]=curData->correction_opacity*255;
//            }

////            int tS=curData->connection_path_node1[tP];
////            if(tS!=0){
////                tData[tP*4+0]=0;
////                tData[tP*4+1]=255;
////                tData[tP*4+2]=255;
////                tData[tP*4+3]=curData->correction_opacity*255;

////            }
//        }
//    }



//    for(int iy=0;iy<curData->ImageH;iy++){
//        for(int ix=0;ix<curData->ImageW;ix++){
//            int tP=iy*curData->ImageW+ix;
//            if(curData->tracingMap[tP].type>0){
//                for(int i=0;i<curData->tracingMap[tP].connected_nodes.size();i++){
//                    int startx=ix;
//                    int starty=iy;
//                    int endx=curData->tracingMap[tP].connected_nodes[i]%curData->ImageW;
//                    int endy=curData->tracingMap[tP].connected_nodes[i]/curData->ImageW;

//                    float a=endy-starty;
//                    float b=-endx+startx;
//                    float c=-a*startx-b*starty;

//                    if(startx>endx){
//                        int temp=startx;
//                        startx=endx;
//                        endx=temp;
//                    }
//                    if(starty>endy){
//                        int temp=starty;
//                        starty=endy;
//                        endy=temp;
//                    }
//                    for(int iiy=starty+1;iiy<endy;iiy++){
//                        for(int iix=startx+1;iix<endx;iix++){
//                            float dis=abs(a*iix + b*iiy + c)/sqrt(a*a+b*b);
//                            if(dis<0.3){
//                                int ttP=iiy*curData->ImageW + iix;
//                                tData[ttP*4+0]=0;
//                                tData[ttP*4+1]=255;
//                                tData[ttP*4+2]=255;
//                                tData[ttP*4+3]=curData->correction_opacity*255;

//                            }
//                        }
//                    }


//                }
//                tData[tP*4+0]=255;
//                tData[tP*4+1]=0;
//                tData[tP*4+2]=0;
//                tData[tP*4+3]=curData->correction_opacity*255;

//            }
//        }
//    }





    if((curData->correctorInputType==2 ||curData->correctorInputType==3) && c_window->imageControlPartLayout->currentIndex()==3){
        int posx=int(mousePoint.x());
        int posy=int(mousePoint.y());
        if(curData->corrector->tracingMap[posy*curData->ImageW + posx].type==0){
            posx=curData->corrector->tracingMap[int(mousePoint.y())*curData->ImageW + int(mousePoint.x())].posx;
            posy=curData->corrector->tracingMap[int(mousePoint.y())*curData->ImageW + int(mousePoint.x())].posy;
        }
        curData->corrector->mousePatch=curData->corrector->tracingMap[posy*curData->ImageW + posx].patch;

        if(curData->corrector->mousePatch!=NULL){
            for(int iy=curData->corrector->mousePatch->boundMin.y();iy<=curData->corrector->mousePatch->boundMax.y();iy++){
                for(int ix=curData->corrector->mousePatch->boundMin.x();ix<=curData->corrector->mousePatch->boundMax.x();ix++){
                    if(curData->corrector->isInside(QVector2D(ix,iy),curData->corrector->mousePatch->patchCorner)){
                        int tP=iy*curData->ImageW+ix;
                        if(curData->correctorInputType==2){
                            tData[tP*4+0]=255;
                            tData[tP*4+1]=142;
                            tData[tP*4+2]=138;
                            tData[tP*4+3]=curData->correction_opacity*255;
                        }
                        if(curData->correctorInputType==3){
                            tData[tP*4+0]=30;
                            tData[tP*4+1]=126;
                            tData[tP*4+2]=220;
                            tData[tP*4+3]=curData->correction_opacity*255;
                        }
                    }
                }
            }

            patchSet* patch=curData->corrector->mousePatch;
            int tW=patch->boundMax.x()-patch->boundMin.x()+1;
            int tH=patch->boundMax.y()-patch->boundMin.y()+1;

            bool *ck=new bool[tW*tH]();
            int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

            QQueue<int> quex,quey;
            quex.enqueue(patch->midx);
            quey.enqueue(patch->midy);
            ck[int((patch->midy-patch->boundMin.y())*tW+patch->midx-patch->boundMin.x())]=true;

            while(!quex.empty()){
                int curx=quex.dequeue();
                int cury=quey.dequeue();
                for(int d=0;d<8;d++){
                    if(curx+dxylist[d][0]<patch->boundMin.x() || curx+dxylist[d][0]>patch->boundMax.x()
                            || cury+dxylist[d][1]<patch->boundMin.y() || cury+dxylist[d][1]>patch->boundMax.y()){
                        continue;
                    }
                    if(curData->corrector->isInside(QVector2D(curx,cury),patch->patchCorner)==false)continue;

                    int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
                    int tP2=(cury+dxylist[d][1]-patch->boundMin.y())*tW+curx+dxylist[d][0]-patch->boundMin.x();
                    if(ck[tP2]==false){
                        ck[tP2]=true;
                        if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                            quex.enqueue(curx+dxylist[d][0]);
                            quey.enqueue(cury+dxylist[d][1]);
                            if(curData->correctorInputType==2){
                                tData[tP*4+0]=255;
                                tData[tP*4+1]=142;
                                tData[tP*4+2]=138;
                                tData[tP*4+3]=255;
                            }
                            if(curData->correctorInputType==3){
                                tData[tP*4+0]=30;
                                tData[tP*4+1]=126;
                                tData[tP*4+2]=220;
                                tData[tP*4+3]=255;
                            }
                        }
                    }
                }
            }
            delete []ck;


        }
    }


    glBindTexture(GL_TEXTURE_2D,curData->tex_correction);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,
                    curData->ImageW,
                    curData->ImageH,
                    GL_RGBA,GL_UNSIGNED_BYTE,
                    tData);
    delete []tData;

}
void GlWidgetForData::setMitoLabelTex(){

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
//    m_program->setUniformValue(con_mito_thresh,curData->mito_thresh);
    m_program->setUniformValue(con_mito_alpha,curData->mitoLabel_opacity);

    unsigned char *tData=new unsigned char[curData->ImageW*curData->ImageH*4]();
    if(curData->focusItem!=-1){
        Mito t=curData->mitoData[curData->focusItem];
        for(int iy=t.ymin;iy<=t.ymax;iy++){
            for(int ix=t.xmin;ix<=t.xmax;ix++){
                int tP=iy*curData->ImageW+ix;
                if((iy==t.ymin || iy==t.ymax || ix==t.xmax || ix==t.xmin) && c_window->boundaryOnOff->isChecked()){
                    if(t.enabled==false){
                        tData[tP*4+0]=0;
                        tData[tP*4+1]=0;
                        tData[tP*4+2]=0;
                        tData[tP*4+3]=255;

                    }
                    else{
                        tData[tP*4+0]=0;
                        tData[tP*4+1]=255;
                        tData[tP*4+2]=255;
                        tData[tP*4+3]=255;
                    }
                }
                else if(curData->mitoLabelImage[tP]==t.index){
                    if(t.enabled){
//                        tData[tP*4+0]=curData->imageData2[tP]/256;
//                        tData[tP*4+1]=0;
//                        tData[tP*4+2]=0;
//                        tData[tP*4+3]=curData->mitoLabel_opacity*255;
                    }
                }
            }
        }
    }
    else{
        for(int i=0;i<curData->mitoData.length();i++){
            Mito t=curData->mitoData[i];

            float error_prob=1-diceScore(i);

            for(int iy=t.ymin;iy<=t.ymax;iy++){
                for(int ix=t.xmin;ix<=t.xmax;ix++){
                    int tP=iy*curData->ImageW+ix;
                    bool cur_enabled=(c_window->analysis_type->currentIndex()==0 && curData->enabled[i])
                            || (c_window->analysis_type->currentIndex()!=0 && curData->globalEnabled[i]);

                    if((iy==t.ymin || iy==t.ymax || ix==t.xmax || ix==t.xmin) && c_window->boundaryOnOff->isChecked()){
                        if(t.enabled==false || cur_enabled==false){
                            tData[tP*4+0]=0;
                            tData[tP*4+1]=0;
                            tData[tP*4+2]=0;
                            tData[tP*4+3]=255;

                        }
                        else if(error_prob>curData->error_thresh){
                            tData[tP*4+0]=255;
                            tData[tP*4+1]=0;
                            tData[tP*4+2]=0;
                            tData[tP*4+3]=255;
                        }
                        else{
                            tData[tP*4+0]=0;
                            tData[tP*4+1]=255;
                            tData[tP*4+2]=255;
                            tData[tP*4+3]=255;
                        }
                    }
                    else if(cur_enabled){
                        if(curData->mitoLabelImage[tP]==t.index){
                            if(t.enabled){
//                                tData[tP*4+0]=255;
//                                tData[tP*4+1]=0;
//                                tData[tP*4+2]=0;
//                                tData[tP*4+3]=curData->mitoLabel_opacity*255;
                            }
                        }
                    }

                }
            }

        }
    }
    if(doMitoBrushing){
        int r=0,g=0,b=200;
        if(curData->correctorInputType==12 || curData->correctorInputType==13){
            r=255;
            g=100;
            b=85;
        }

        for(int i=0;i<curData->ImageW*curData->ImageH;i++){
            if(curData->mito_buffer[i]!=0){
                tData[i*4+0]=r;
                tData[i*4+1]=g;
                tData[i*4+2]=b;
                tData[i*4+3]=255;
            }
        }
    }
    if(curData->correctorInputType>=10 && curData->correctorInputType<14 && c_window->imageControlPartLayout->currentIndex()==4){
        int r=0,g=0,b=200;
        if(curData->correctorInputType==12 || curData->correctorInputType==13){
            r=255;
            g=100;
            b=85;
        }

        for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
            for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                int tP=iy*curData->ImageW+ix;

                float dis=(QVector2D(ix,iy)-mousePoint).length();
                if(dis<curData->brushSize){
                    tData[tP*4+0]=r;
                    tData[tP*4+1]=g;
                    tData[tP*4+2]=b;
                    tData[tP*4+3]=127;
                }
            }
        }
    }


//    for(int i=0;i<curData->ImageW*curData->ImageH;i++){
//        if(curData->mitoLineImage[i]==255){
//            tData[i*4+0]=255;
//            tData[i*4+1]=255;
//            tData[i*4+2]=255;
//            tData[i*4+3]=255;
//        }
//        else if(curData->mitoLineImage[i]!=0){
//            tData[i*4+0]=255;
//            tData[i*4+1]=0;
//            tData[i*4+2]=255;
//            tData[i*4+3]=curData->mitoLineImage[i]*2;
//        }
//    }

    glBindTexture(GL_TEXTURE_2D,curData->tex_mitoLabel);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,
                    curData->ImageW,
                    curData->ImageH,
                    GL_RGBA,GL_UNSIGNED_BYTE,
                    tData);
    delete []tData;



    tData=new unsigned char[curData->ImageW*curData->ImageH]();
    if(curData->focusItem!=-1){
        Mito t=curData->mitoData[curData->focusItem];
        for(int iy=t.ymin;iy<=t.ymax;iy++){
            for(int ix=t.xmin;ix<=t.xmax;ix++){
                int tP=iy*curData->ImageW+ix;
                if(curData->mitoLabelImage[tP]==t.index){
                    if(t.enabled){
                        tData[tP]=255;
                    }
                }
            }
        }
    }
    else{
        for(int i=0;i<curData->mitoData.length();i++){
            Mito t=curData->mitoData[i];
            for(int iy=t.ymin;iy<=t.ymax;iy++){
                for(int ix=t.xmin;ix<=t.xmax;ix++){
                    int tP=iy*curData->ImageW+ix;
                    bool cur_enabled=(c_window->analysis_type->currentIndex()==0 && curData->enabled[i])
                            || (c_window->analysis_type->currentIndex()!=0 && curData->globalEnabled[i]);
                    if(cur_enabled){
                        if(curData->mitoLabelImage[tP]==t.index){
                            if(t.enabled){
                                tData[tP]=255;
                            }
                        }
                    }

                }
            }

        }
    }
//    curData->saveMitoLabel(curData->path+"refined_mito_label.tif",tData);

    glBindTexture(GL_TEXTURE_2D,curData->tex_mitoLabel_original);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,
                    curData->ImageW,
                    curData->ImageH,
                    GL_RED,GL_UNSIGNED_BYTE,
                    tData);




    delete []tData;

}
float GlWidgetForData::diceScore(int i){
    if(c_window->curDataIndex==-1)return -1;
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
    Mito t=curData->mitoData[i];
    QVector <int> touched_Ob;
    float region1=0;
    float region2=0;
    float overlab_region=0;

    for(int iy=t.ymin;iy<=t.ymax;iy++){
        for(int ix=t.xmin;ix<=t.xmax;ix++){
            int tP=iy*curData->ImageW+ix;
            if(curData->mitoLabelImage[tP]==t.index){
                region1+=1;
                if(curData->mitoLabelImage_signal[tP]!=0){
                    overlab_region+=1;
                    bool is_found=false;
                    for(int j=0;j<touched_Ob.size();j++){
                        if(touched_Ob[j]==curData->mitoLabelImage_signal[tP]){
                            is_found=true;
                            break;
                        }
                    }
                    if(is_found==false)touched_Ob.push_back(curData->mitoLabelImage_signal[tP]);
                }
            }
        }
    }
    if(touched_Ob.size()>=2)return 0;

    for(int j=0;j<touched_Ob.size();j++){
        Mito Ob=curData->mitoData_signal[touched_Ob[j]-1];
        for(int iy=Ob.ymin;iy<=Ob.ymax;iy++){
            for(int ix=Ob.xmin;ix<=Ob.xmax;ix++){
                int tP=iy*curData->ImageW+ix;
                if(curData->mitoLabelImage_signal[tP]==Ob.index){
                    region2+=1;
                }
            }
        }
    }
    return 2*overlab_region/(region1+region2);
}
void GlWidgetForData::resizeGL(int w, int h)
{
//    int t=w>h?w:h;
//    this->resize(t,t);
}

void GlWidgetForData::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
    mousePressedPos=event->pos();

    if(event->buttons() & Qt::MidButton){
        measureStart=QVector2D(event->x(),event->y());
        measureEnd=measureStart;
        measure=true;
    }
    colorTableChanged=true;


    if(!IsSpacePressed){
        if(c_window->curDataIndex!=-1){
            MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

            QVector2D curPoint=QVector2D(event->x()-(this->width()-curData->ImageW*curData->scale)/2.0-curData->shift.x()*curData->scale
                                         ,event->y()-(this->height()-curData->ImageH*curData->scale)/2.0-curData->shift.y()*curData->scale);
            curPoint=curPoint/curData->scale;

            mousePoint=curPoint;

            if(((curData->correctorInputType>=0 && curData->correctorInputType<4) || curData->correctorInputType==7) && c_window->imageControlPartLayout->currentIndex()==3 && (event->buttons() & Qt::LeftButton)){
                mitoUpdate=true;
                if(curData->correctorInputType==0){
//                    StructureChangeByCCb(mousePoint);
                    for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
                        for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                            if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                            int tP=iy*curData->ImageW+ix;
                            if(curData->cc_buffer[tP]==2){
                                curData->structureData[tP]=curData->correctorInputType;
                                curData->correctionData[tP]=4;
                            }
                        }
                    }
                }
                else{
                    if(c_window->correction_type->currentIndex()==1){
                        bool is_radars=false;
                        for(int i=0;i<curData->radars.size();i++){
                            if(curData->radars[i]->recommendation_insets[0]==NULL)continue;
                            int p=curData->radars[i]->target_point;
                            QVector2D target_pos=QVector2D(p%curData->ImageW,p/curData->ImageW);

                            float dis=(mousePoint-target_pos).length();
                            if(dis<5){
                                curData->radars[i]->is_focus_fixed=!curData->radars[i]->is_focus_fixed;
                                if(curData->radars[i]->is_focus_fixed){
                                    curData->focus_fixed_point=curData->radars[i]->target_point;
                                }
                                else{
                                    curData->focus_fixed_point=-1;
                                }
                                is_radars=true;
                                break;
                            }
                        }

                        if(is_radars==false)
                            correctNeurite(mousePoint,curData->correctorInputType);
                    }
                    else{

    //                    StructureChangeByCCf(mousePoint);
                        for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
                            for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                                if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                                int tP=iy*curData->ImageW+ix;
                                if(curData->cc_buffer[tP]==2){
                                    curData->structureData[tP]=curData->correctorInputType;
                                    curData->correctionData[tP]=curData->correctorInputType;
                                }
                                else if(curData->cc_buffer[tP]==3){
    //                                curData->correctionData[tP]=4;
                                }
                            }
                        }
                    }
                }


                correctionChanged=true;
                structureChanged=true;
                for(int i=0;i<curData->radars.size();i++){
                    for(int j=0;j<4;j++){
                        if(curData->radars[i]->recommendation_insets[j]!=NULL){
                            curData->radars[i]->recommendation_insets[j]->structureChanged=true;
                            curData->radars[i]->recommendation_insets[j]->update();
                        }
                    }
                }
            }


            if(curData->correctorInputType>=10 && curData->correctorInputType<14 && c_window->imageControlPartLayout->currentIndex()==4 && (event->buttons() & Qt::LeftButton)){
                doMitoBrushing=true;
                memset(curData->mito_buffer,0,curData->ImageH*curData->ImageW);
                for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
                    for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                        if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                        float dis=(QVector2D(ix,iy)-mousePoint).length();
                        if(dis<curData->brushSize){
                            int tP=iy*curData->ImageW+ix;
                            curData->mito_buffer[tP]=curData->correctorInputType;
                        }
                    }
                }
                mitoLabelChanged=true;
            }




            if(curData->subsetInputType==1 && c_window->imageControlPartLayout->currentIndex()==5 && (event->buttons() & Qt::LeftButton)){
                for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
                    for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                        if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                        float dis=(QVector2D(ix,iy)-mousePoint).length();
                        if(dis<=curData->brushSize)curData->subsetData[iy*curData->ImageW+ix]=1;
                    }
                }
                subsetChanged=true;
            }
            else if(curData->subsetInputType==2 && c_window->imageControlPartLayout->currentIndex()==5 && (event->buttons() & Qt::LeftButton)){
                for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
                    for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                        if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                        float dis=(QVector2D(ix,iy)-mousePoint).length();
                        if(dis<=curData->brushSize)curData->subsetData[iy*curData->ImageW+ix]=0;
                    }
                }
                subsetChanged=true;
            }
        }
    }
    update();

//    c_window->ImageControlSliderChanged(0);

}
void GlWidgetForData::StructureChangeByCCf(QVector2D startP){
    if(c_window->curDataIndex==-1)return;

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
    memset(curData->cc_buffer,0,curData->ImageW*curData->ImageH);
    if(startP.x()<0 || startP.y()<0 || int(startP.x())>=curData->ImageW || int(startP.y())>=curData->ImageH)return;

    int sP=int(startP.y())*curData->ImageW + int(startP.x());
    int startv=curData->imageData1[sP];
    //if(curData->color_table_neuron[startv]<curData->structure_thresh)
    if(curData->structureData[sP]==0)
        return;


    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    QQueue<int> quex,quey;
    quex.enqueue(startP.x());
    quey.enqueue(startP.y());
    curData->cc_buffer[sP]=2;
//    curData->structureData[sP]=curData->correctorInputType;
//    curData->correctionData[sP]=4;

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();

        for(int d=0;d<8;d++){
            int new_x=curx+dxylist[d][0];
            int new_y=cury+dxylist[d][1];
            if(new_x<=0 || new_x>=curData->ImageW-1 ||new_y<=0 || new_y>=curData->ImageH-1){
                continue;
            }
            int tP=new_y*curData->ImageW + new_x;
            if(curData->cc_buffer[tP]==0){
//                curData->structureData[tP]=curData->correctorInputType;
//                curData->correctionData[tP]=curData->correctorInputType;
                curData->cc_buffer[tP]=1;

                float v=curData->color_table_neuron[curData->imageData1[tP]];
                float dis=(QVector2D(new_x,new_y)-startP).length();
                //if(v>=curData->structure_thresh && dis<curData->brushSize){
                if(curData->structureData[tP]!=0 && dis<curData->brushSize){
                    curData->cc_buffer[tP]=2;
                    quex.enqueue(new_x);
                    quey.enqueue(new_y);
                }
                else if(curData->structureData[tP]==0){
                    curData->cc_buffer[tP]=3;
                }
            }
        }
    }
}
void GlWidgetForData::StructureChangeByCCb(QVector2D startP){
    if(c_window->curDataIndex==-1)return;

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
    memset(curData->cc_buffer,0,curData->ImageW*curData->ImageH);
    if(startP.x()<0 || startP.y()<0 || int(startP.x())>=curData->ImageW || int(startP.y())>=curData->ImageH)return;

    int sP=int(startP.y())*curData->ImageW + int(startP.x());
    int startv=curData->imageData1[sP];

//    if(curData->color_table_neuron[startv]>=curData->structure_thresh)
    if(curData->structureData[sP]!=0)
        return;

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    QQueue<int> quex,quey;
    quex.enqueue(startP.x());
    quey.enqueue(startP.y());
    curData->cc_buffer[sP]=2;
//    curData->structureData[sP]=curData->correctorInputType;
//    curData->correctionData[sP]=4;

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();
        for(int d=0;d<8;d++){
            int new_x=curx+dxylist[d][0];
            int new_y=cury+dxylist[d][1];
            if(new_x<=0 || new_x>=curData->ImageW-1 ||new_y<=0 || new_y>=curData->ImageH-1){
                continue;
            }
            int tP=new_y*curData->ImageW + new_x;
            if(curData->cc_buffer[tP]==0){
//                curData->structureData[tP]=curData->correctorInputType;
//                curData->correctionData[tP]=4;
                curData->cc_buffer[tP]=1;
                float v=curData->color_table_neuron[curData->imageData1[tP]];
                float dis=(QVector2D(new_x,new_y)-startP).length();
//                if(v<curData->structure_thresh && dis<curData->brushSize){
                if(curData->structureData[tP]==0 && dis<curData->brushSize){
                    curData->cc_buffer[tP]=2;
                    quex.enqueue(new_x);
                    quey.enqueue(new_y);
                }
            }
        }
    }
}

void GlWidgetForData::MitoExcluding(){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
    QVector <int> labelIndex;
    QVector <int> count;
    for(int i=0;i<curData->ImageW*curData->ImageH;i++){
        if(curData->mito_buffer[i]!=0){
            if(curData->mitoLabelImage[i]!=0){
                bool isFound=false;
                for(int j=0;j<labelIndex.count();j++){
                    if(labelIndex[j]==curData->mitoLabelImage[i]){
                        count[j]+=1;
                        isFound=true;
                        break;
                    }
                }
                if(isFound==false){
                    labelIndex.push_back(curData->mitoLabelImage[i]);
                    count.push_back(1);
                }
            }
        }
    }
    int targetIndex=-1;
    int maxCount=0;
    for(int j=0;j<labelIndex.count();j++){
        if(maxCount<count[j]){
            targetIndex=labelIndex[j];
            maxCount=count[j];
        }
    }
    for(int i=0;i<curData->ImageW*curData->ImageH;i++){
        if(curData->mitoLabelImage[i]==targetIndex){
            curData->mitoCorrectionData[i]=10;
        }
    }
}
void GlWidgetForData::MitoSplitting(){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    QVector <int> touched_object;

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            if(curData->mito_buffer[iy*curData->ImageW + ix]!=0 && curData->mito_buffer[iy*curData->ImageW + ix]!=50){
                QVector2D startP=QVector2D(ix,iy);
                QQueue<int> quex,quey;
                quex.enqueue(ix);
                quey.enqueue(iy);
                curData->mito_buffer[iy*curData->ImageW + ix]=50;
                float startv=curData->color_table_mito[curData->imageData2[iy*curData->ImageW + ix]];
                if(startv<curData->structure_thresh)curData->mitoCorrectionData[iy*curData->ImageW + ix]=11;
            //    curData->structureData[sP]=curData->correctorInputType;
            //    curData->correctionData[sP]=4;

                while(!quex.empty()){
                    int curx=quex.dequeue();
                    int cury=quey.dequeue();

                    for(int d=0;d<8;d++){
                        int new_x=curx+dxylist[d][0];
                        int new_y=cury+dxylist[d][1];
                        if(new_x<=0 || new_x>=curData->ImageW-1 ||new_y<=0 || new_y>=curData->ImageH-1){
                            continue;
                        }
                        int tP=new_y*curData->ImageW + new_x;
                        if(curData->mito_buffer[tP]!=50){
            //                curData->structureData[tP]=curData->correctorInputType;
            //                curData->correctionData[tP]=curData->correctorInputType;

                            float v=curData->color_table_mito[curData->imageData2[tP]];
                            float dis=(QVector2D(new_x,new_y)-startP).length();
                            if(v<curData->structure_thresh && dis<2){
                                curData->mito_buffer[tP]=50;
                                curData->mitoCorrectionData[tP]=11;
                                quex.enqueue(new_x);
                                quey.enqueue(new_y);
                                if(curData->mitoLabelImage[tP]!=0){
                                    bool is_found=false;
                                    for(int i=0;i<touched_object.size();i++){
                                        if(touched_object[i]==curData->mitoLabelImage[tP]){
                                            is_found=true;
                                            break;
                                        }
                                    }
                                    if(is_found==false){
                                        touched_object.push_back(curData->mitoLabelImage[tP]);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for(int i=0;i<touched_object.size();i++){
        Mito t=curData->mitoData[touched_object[i]-1];
        for(int iy=t.ymin;iy<=t.ymax;iy++){
            for(int ix=t.xmin;ix<=t.xmax;ix++){
                int tP=iy*curData->ImageW+ix;
                if(curData->mitoLabelImage[tP]==t.index && curData->mitoCorrectionData[tP]==0){
                    curData->mitoCorrectionData[tP]=13;
                }
            }
        }
    }
}
void GlWidgetForData::MitoMerging(){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            if(curData->mito_buffer[iy*curData->ImageW + ix]!=0 && curData->mito_buffer[iy*curData->ImageW + ix]!=50){
                QVector2D startP=QVector2D(ix,iy);
                QQueue<int> quex,quey;
                quex.enqueue(ix);
                quey.enqueue(iy);
                float startv=curData->color_table_mito[curData->imageData2[iy*curData->ImageW + ix]];
                if(startv>=curData->structure_thresh)curData->mitoCorrectionData[iy*curData->ImageW + ix]=12;
                curData->mito_buffer[iy*curData->ImageW + ix]=50;
            //    curData->structureData[sP]=curData->correctorInputType;
            //    curData->correctionData[sP]=4;

                while(!quex.empty()){
                    int curx=quex.dequeue();
                    int cury=quey.dequeue();

                    for(int d=0;d<8;d++){
                        int new_x=curx+dxylist[d][0];
                        int new_y=cury+dxylist[d][1];
                        if(new_x<=0 || new_x>=curData->ImageW-1 ||new_y<=0 || new_y>=curData->ImageH-1){
                            continue;
                        }
                        int tP=new_y*curData->ImageW + new_x;
                        if(curData->mito_buffer[tP]!=50){
            //                curData->structureData[tP]=curData->correctorInputType;
            //                curData->correctionData[tP]=curData->correctorInputType;

                            float v=curData->color_table_mito[curData->imageData2[tP]];
                            float dis=(QVector2D(new_x,new_y)-startP).length();
                            if(v>=curData->structure_thresh && dis<2){
                                curData->mito_buffer[tP]=50;
                                curData->mitoCorrectionData[tP]=12;
                                quex.enqueue(new_x);
                                quey.enqueue(new_y);
                            }
                        }
                    }
                }
            }
        }
    }
}
void GlWidgetForData::MitoIncluding(){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            if(curData->mito_buffer[iy*curData->ImageW + ix]!=0){
                 float startv=curData->color_table_mito[curData->imageData2[iy*curData->ImageW + ix]];
                 if(startv<curData->structure_thresh)continue;
                QVector2D startP=QVector2D(ix,iy);
                QQueue<int> quex,quey;
                quex.enqueue(ix);
                quey.enqueue(iy);
                curData->mito_buffer[iy*curData->ImageW + ix]=50;
                curData->mitoCorrectionData[iy*curData->ImageW + ix]=13;
            //    curData->structureData[sP]=curData->correctorInputType;
            //    curData->correctionData[sP]=4;

                while(!quex.empty()){
                    int curx=quex.dequeue();
                    int cury=quey.dequeue();

                    for(int d=0;d<8;d++){
                        int new_x=curx+dxylist[d][0];
                        int new_y=cury+dxylist[d][1];
                        if(new_x<=0 || new_x>=curData->ImageW-1 ||new_y<=0 || new_y>=curData->ImageH-1){
                            continue;
                        }
                        int tP=new_y*curData->ImageW + new_x;
                        if(curData->mito_buffer[tP]!=50){
            //                curData->structureData[tP]=curData->correctorInputType;
            //                curData->correctionData[tP]=curData->correctorInputType;
                            curData->mito_buffer[tP]=50;

                            float v=curData->color_table_mito[curData->imageData2[tP]];
                            float dis=(QVector2D(new_x,new_y)-startP).length();
                            if(v>=curData->structure_thresh && dis<5){
                                curData->mitoCorrectionData[tP]=13;
                                quex.enqueue(new_x);
                                quey.enqueue(new_y);
                            }
                        }
                    }
                }
            }
        }
    }
}

void GlWidgetForData::mouseReleaseEvent(QMouseEvent *event)
{

    if(doMitoBrushing){
        if(c_window->curDataIndex!=-1){
            MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
            if(curData->correctorInputType==10 && c_window->correction_type_mito->currentIndex()==1){
                MitoExcluding();
            }
            if(curData->correctorInputType==11 && c_window->correction_type_mito->currentIndex()==1){
                MitoSplitting();
            }
            if(curData->correctorInputType==12 && c_window->correction_type_mito->currentIndex()==1){
                MitoMerging();
            }
            if(curData->correctorInputType==13 && c_window->correction_type_mito->currentIndex()==1){
                MitoIncluding();
            }
            mitoUpdate=true;
        }
        doMitoBrushing=false;
    }

    if(mitoUpdate){
        emit c_window->mitoUpdate();
        mitoUpdate=false;
    }

    if((event->pos()-mousePressedPos).manhattanLength()<3 && event->button()==Qt::RightButton){
        if(c_window->curDataIndex!=-1){
            MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
            int curItem=-1;
            float minDis=1000000;
            for(int i=0;i<curData->mitoData.length();i++){
                Mito item=curData->mitoData[i];
                QVector2D tmax=QVector2D(item.xmax,item.ymax);
                QVector2D tmin=QVector2D(item.xmin,item.ymin);
                QVector2D p_render=QVector2D(event->x()-(this->width()-curData->ImageW*curData->scale)/2.0-curData->shift.x()*curData->scale
                                             ,event->y()-(this->height()-curData->ImageH*curData->scale)/2.0-curData->shift.y()*curData->scale);
                p_render=p_render/curData->scale;
                float curDis=((tmax+tmin)*0.5-p_render).length();
                if(p_render.x()<tmax.x() && p_render.x()>tmin.x()
                        && p_render.y()<tmax.y() && p_render.y()>tmin.y()){
                    if(curDis<minDis){
                        curItem=i;
                        minDis=curDis;
                    }
                }
            }

            if(curItem!=-1){
                curData->mitoData[curItem].enabled=!curData->mitoData[curItem].enabled;
                curData->checkDataEnable(c_window);
                c_window->synchronization();

            }
        }
    }


    if(event->buttons() & Qt::MidButton){
        measureEnd=QVector2D(event->x(),event->y());
        if((measureStart-measureEnd).length()<10)measure=false;
    }


//    if(structureChanged){
//        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
        emit c_window->recommenderRun(0);

//        if(curData->correctorInputType==2)
//            emit c_window->recommenderRun(0);
//        if(curData->correctorInputType==3)
//            emit c_window->recommenderRun(1);
//    }

    update();


}
void GlWidgetForData::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->x()>=0 && event->x()<this->width() && event->y()>=0 && event->y()<this->height())){
        return;
    }
    setFocus();
    this->activateWindow();

    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    if(c_window->curDataIndex==-1){
        m_lastPos=event->pos();
        return;
    }

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    QVector2D curPoint=QVector2D(event->x()-(this->width()-curData->ImageW*curData->scale)/2.0-curData->shift.x()*curData->scale
                                 ,event->y()-(this->height()-curData->ImageH*curData->scale)/2.0-curData->shift.y()*curData->scale);
    curPoint=curPoint/curData->scale;

    mousePoint=curPoint;


    if(curData->correctorInputType>=10 && curData->correctorInputType<14 && c_window->imageControlPartLayout->currentIndex()==4 && !IsSpacePressed){
        mitoLabelChanged=true;
    }


    if(((curData->correctorInputType>=0 && curData->correctorInputType<4) || curData->correctorInputType==7) && c_window->imageControlPartLayout->currentIndex()==3 && !IsSpacePressed){

        if(curData->correctorInputType==0){
            StructureChangeByCCb(mousePoint);
        }
        else{
            if(c_window->correction_type->currentIndex()==1){

            }
            else{
                StructureChangeByCCf(mousePoint);
            }
        }
        structureChanged=true;
        for(int i=0;i<curData->radars.size();i++){
            for(int j=0;j<4;j++){
                if(curData->radars[i]->recommendation_insets[j]!=NULL){
                    curData->radars[i]->recommendation_insets[j]->structureChanged=true;
                    curData->radars[i]->recommendation_insets[j]->update();
                }
            }
        }
    }


    if(((curData->correctorInputType>=0 && curData->correctorInputType<4) || curData->correctorInputType==7) && c_window->imageControlPartLayout->currentIndex()==3 && (event->buttons() & Qt::LeftButton)&& !IsSpacePressed){
        mitoUpdate=true;
        if(curData->correctorInputType==0){
            for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
                for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                    if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                    int tP=iy*curData->ImageW+ix;
                    if(curData->cc_buffer[tP]==2){
                        curData->structureData[tP]=curData->correctorInputType;
                        curData->correctionData[tP]=4;
                    }
                }
            }
        }
        else{
            if(c_window->correction_type->currentIndex()==1){
                correctNeurite(mousePoint,curData->correctorInputType);
            }
            else{

                for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
                    for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                        if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                        int tP=iy*curData->ImageW+ix;
                        if(curData->cc_buffer[tP]==2){
                            curData->structureData[tP]=curData->correctorInputType;
                            curData->correctionData[tP]=curData->correctorInputType;
                        }
                        else if(curData->cc_buffer[tP]==3){
    //                        curData->correctionData[tP]=4;
                        }
                    }
                }
            }
        }
//        emit curData->correctorStructureBrushing();
        correctionChanged=true;
        structureChanged=true;
        for(int i=0;i<curData->radars.size();i++){
            for(int j=0;j<4;j++){
                if(curData->radars[i]->recommendation_insets[j]!=NULL){
                    curData->radars[i]->recommendation_insets[j]->structureChanged=true;
                    curData->radars[i]->recommendation_insets[j]->update();
                }
            }
        }

    }
    else if(curData->correctorInputType>=10 && curData->correctorInputType<14 && c_window->imageControlPartLayout->currentIndex()==4 && (event->buttons() & Qt::LeftButton)&& !IsSpacePressed){
        doMitoBrushing=true;
        for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
            for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                float dis=(QVector2D(ix,iy)-mousePoint).length();
                if(dis<curData->brushSize){
                    int tP=iy*curData->ImageW+ix;
                    curData->mito_buffer[tP]=curData->correctorInputType;
                }
            }
        }
        mitoLabelChanged=true;
    }
    else if(curData->subsetInputType==1 && c_window->imageControlPartLayout->currentIndex()==5 && (event->buttons() & Qt::LeftButton) && !IsSpacePressed){
        for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
            for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                float dis=(QVector2D(ix,iy)-mousePoint).length();
                if(dis<=curData->brushSize)curData->subsetData[iy*curData->ImageW+ix]=1;
            }
        }
        subsetChanged=true;
    }
    else if(curData->subsetInputType==2 && c_window->imageControlPartLayout->currentIndex()==5 && (event->buttons() & Qt::LeftButton) && !IsSpacePressed){
        for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
            for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                float dis=(QVector2D(ix,iy)-mousePoint).length();
                if(dis<=curData->brushSize)curData->subsetData[iy*curData->ImageW+ix]=0;
            }
        }
        subsetChanged=true;
    }
    else if(event->buttons() & Qt::MidButton){
        measureEnd=QVector2D(event->x(),event->y());
    }
    else if (event->buttons() & Qt::LeftButton ) {
        curData->shift+=QVector2D(dx,dy)/curData->scale;
    }
    else if (event->buttons() & Qt::RightButton) {
        if(curData->scale>0.02){
            float t=1.0*(-dy)/1000*curData->scale;
            curData->scale+=t;
        }
        else{
            curData->scale=0.021;
        }
    }
    else{
        if(curData->correctorInputType!=-1 && c_window->imageControlPartLayout->currentIndex()==4 && !IsSpacePressed){
            int prev=curData->focusItem;
            curData->focusItem=-1;
            if(prev!=curData->focusItem){
                mitoLabelChanged=true;
                c_window->synchronization();
            }

        }
        else{
            int prev=curData->focusItem;
            bool flag=false;
            float minDis=1000000;

            for(int i=0;i<curData->mitoData.length();i++){
                if(c_window->analysis_type->currentIndex()==0 && curData->enabled[i]==false)continue;
                if(c_window->analysis_type->currentIndex()!=0 && curData->globalEnabled[i]==false)continue;
                Mito item=curData->mitoData[i];
                QVector2D tmax=QVector2D(item.xmax,item.ymax);
                QVector2D tmin=QVector2D(item.xmin,item.ymin);
                QVector2D p_render=QVector2D(event->x()-(this->width()-curData->ImageW*curData->scale)/2.0-curData->shift.x()*curData->scale
                                             ,event->y()-(this->height()-curData->ImageH*curData->scale)/2.0-curData->shift.y()*curData->scale);
                p_render=p_render/curData->scale;

                float curDis=((tmax+tmin)*0.5-p_render).length();

                if(p_render.x()<tmax.x() && p_render.x()>tmin.x()
                        && p_render.y()<tmax.y() && p_render.y()>tmin.y()){
                    if(curDis<minDis){
                        minDis=curDis;
                        curData->focusItem=i;
                        flag=true;
                    }
                }
            }
            if(flag==false){
                curData->focusItem=-1;
                c_window->focus_group=-1;
            }

            if(curData->focusItem!=-1)c_window->focus_group=c_window->curDataIndex;

            if(prev!=curData->focusItem){
                mitoLabelChanged=true;
                c_window->synchronization();
            }
        }
    }


    update();
    m_lastPos=event->pos();

}

void GlWidgetForData::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numDegrees.isNull()) {
        QPoint numSteps = numDegrees / 15;
        if(c_window->curDataIndex!=-1){
            MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

 //           if(curData->subsetInputType!=0 && c_window->imageControlPartLayout->currentIndex()==4){
                curData->brushSize+=numSteps.x()+numSteps.y();
                if(curData->brushSize<1)curData->brushSize=1;
                if(curData->brushSize>50)curData->brushSize=50;
                c_window->subsetBrushSize->setValue(curData->brushSize);
                structureChanged=true;
                mitoLabelChanged=true;
                update();
//            }
        }

//        c_window->roi_size+=numSteps.x()+numSteps.y();
//        if(c_window->roi_size<1)c_window->roi_size=1;
    }
    event->accept();
}

void GlWidgetForData::timerEvent(QTimerEvent *event){

    if(c_window->curDataIndex!=-1){
        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

        for(int i=0;i<Insets_mito.size();i++){

            if(InsetLocations_mito[i].x()<0)InsetLocations_mito[i].setX(0);
            if(InsetLocations_mito[i].y()<0)InsetLocations_mito[i].setY(0);
            if(InsetLocations_mito[i].x()>this->width())InsetLocations_mito[i].setX(this->width());
            if(InsetLocations_mito[i].y()>this->height())InsetLocations_mito[i].setY(this->height());

            QVector2D p0=(Insets_mito[i]->inset_startPos+Insets_mito[i]->inset_endPos)*0.5*curData->scale
                            + QVector2D(this->width()-curData->ImageW*curData->scale,this->height()-curData->ImageH*curData->scale)*0.5
                            + curData->shift*curData->scale;
            float min_cost=1000000;
            int min_index=0;
            for(int j=0;j<9;j++){
                QVector2D p=InsetLocations_mito[i]+dxylist[j];
                if(p.x()<0 || p.y()<0 || p.x()>this->width() || p.y()>this->height())continue;
                float dis_from_x=fmin(abs(p.x()),abs(this->width()-p.x()));
                float dis_from_y=fmin(abs(p.y()),abs(this->height()-p.y()));

                float overlap_cost=0;
                float intersect_cost=0;
                for(int k=0;k<Insets_mito.size();k++){
                    if(k==i)continue;
                    QVector2D q=InsetLocations_mito[k];
                    QVector2D q0=(Insets_mito[k]->inset_startPos+Insets_mito[k]->inset_endPos)*0.5*curData->scale
                                    + QVector2D(this->width()-curData->ImageW*curData->scale,this->height()-curData->ImageH*curData->scale)*0.5
                                    + curData->shift*curData->scale;

                    overlap_cost+=fmax(0,InsetSizes_mito[i]+InsetSizes_mito[k]-(p-q).length());
                    intersect_cost+=doIntersect(p0,p,q0,q);

                }

                float cur_cost=(p-p0).length()*0.01 + fmin(dis_from_x,dis_from_y) + overlap_cost*100 + intersect_cost;

                if(cur_cost<min_cost){
                    min_cost=cur_cost;
                    min_index=j;
                }

            }
            InsetLocations_mito[i]=InsetLocations_mito[i]+dxylist[min_index];
        }
        update();
    }
}

bool GlWidgetForData::onSegment(QVector2D p, QVector2D q, QVector2D r)
{
    if (q.x() <= fmax(p.x(), r.x()) && q.x() >= fmin(p.x(), r.x()) &&
        q.y() <= fmax(p.y(), r.y()) && q.y() >= fmin(p.y(), r.y()))
        return true;
    return false;
}

int GlWidgetForData::orientation(QVector2D p, QVector2D q, QVector2D r)
{
    float val = (q.y() - p.y()) * (r.x() - q.x()) -
        (q.x() - p.x()) * (r.y() - q.y());

    if (abs(val)<0.0001) return 0;
    return (val > 0) ? 1 : 2;
}

bool GlWidgetForData::doIntersect(QVector2D p1, QVector2D q1, QVector2D p2, QVector2D q2)
{
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    if (o1 != o2 && o3 != o4)
        return true;

    if (o1 == 0 && onSegment(p1, p2, q1)) return true;
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false;
}


void GlWidgetForData::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Space){
        IsSpacePressed=true;
    }

    if(event->key()==Qt::Key_I){
        if(c_window->curDataIndex!=-1){
//            MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
//            int insetSize=qrand()%10*4+28;
//            InsetSizes.push_back(insetSize);

//            QVector2D insetStart=QVector2D(qrand()%(curData->ImageW-insetSize*2),qrand()%(curData->ImageH-insetSize*2));
//            QVector2D insetEnd=insetStart+QVector2D(insetSize*2,insetSize*2);
//            GlWidgetForInset *newInset=new GlWidgetForInset(c_window,curData,1,insetStart,insetEnd,this);
//            Insets.push_back(newInset);

//            newInset->setParent(this);

//            QVector2D location=QVector2D(qrand()%this->width(),qrand()%this->height());
//            InsetLocations.push_back(location);

//            update();
        }
    }

    if(event->key()==Qt::Key_S){
        qDebug()<<"glwidget data key s pressed";
        if(c_window->curDataIndex!=-1){
            //qDebug()<<"save patch based dataset";
//            c_window->save_dataset_for_training_all();

        }

    }


}

void GlWidgetForData::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Space){
        IsSpacePressed=false;
    }
}



void GlWidgetForData::cut_plane(int r){
}


void GlWidgetForData::set_z_scale(int n){
}



void GlWidgetForData::ambi_change(int a){
}

void GlWidgetForData::diff_change(int a){
}

void GlWidgetForData::spec_change(int a){
}
void GlWidgetForData::shin_change(int a){
}
void GlWidgetForData::l_t_change(int a){
}

void GlWidgetForData::change_R1(int a){
}

void GlWidgetForData::change_R2(int a){
}

void GlWidgetForData::change_cut_d(int a){

}
void GlWidgetForData::change_cut_enable(bool a){
}
void GlWidgetForData::change_cut_alpha(int a){
}
void GlWidgetForData::change_phong_enable(bool a){
}
void GlWidgetForData::change_sampling_rate(int a){

}
int GlWidgetForData::getObjectCoordfromMousePoint(QVector2D mousePoint){



//    return QVector3D(0,0,0);



//    if(c_window->curDataIndex==-1)return -1;
//    if(dataList[c_window->curDataIndex]->basePath=="")return -1;

//    int w=dataList[c_window->curDataIndex]->w;
//    int h=dataList[c_window->curDataIndex]->h;
//    int d=dataList[c_window->curDataIndex]->d;

//    QVector2D global_mousePoint=QVector2D(mousePoint.x()/this->width()*2-1,-mousePoint.y()/this->height()*2+1);

//    QVector3D right=cross_product(-eye_position,up).normalized();



//    QVector3D view_eye_position=eye_position+right;
//    QVector3D view_right=cross_product(-view_eye_position,up).normalized();
//    view_eye_position=view_eye_position+up;
//    QVector3D view_up=cross_product(view_right,-view_eye_position).normalized();

//    QVector3D t_pixel_position=view_eye_position*0.5+
//                               view_right*global_mousePoint.x()*this->width()/800.0+
//                               view_up*global_mousePoint.y()*this->height()/800.0;

//    QVector3D direction=-view_eye_position.normalized();
//    float alpha=0;

//    int cn=0;
//    QVector3D a[2];
//    float k;
//    QVector3D point;
//    if(cn<2 && direction.x()!=0){
//        k=(box_min.x()-t_pixel_position.x())/direction.x();
//        point=t_pixel_position+k*direction;
//        if(point.y()>=box_min.y() && point.y()<=box_max.y()
//            && point.z()>=box_min.z() && point.z()<=box_max.z()){
//                a[cn++]=point;
//        }
//    }
//    if(cn<2&& direction.x()!=0){
//        k=(box_max.x()-t_pixel_position.x())/direction.x();
//        point=t_pixel_position+k*direction;
//        if(point.y()>=box_min.y() && point.y()<=box_max.y()
//            && point.z()>=box_min.z() && point.z()<=box_max.z()){
//                a[cn++]=point;
//        }
//    }
//    if(cn<2 && direction.y()!=0){
//        k=(box_min.y()-t_pixel_position.y())/direction.y();
//        point=t_pixel_position+k*direction;
//        if(point.x()>=box_min.x() && point.x()<=box_max.x()
//             && point.z()>=box_min.z() && point.z()<=box_max.z()){
//                 a[cn++]=point;
//        }
//    }
//    if(cn<2 && direction.y()!=0){
//        k=(box_max.y()-t_pixel_position.y())/direction.y();
//        point=t_pixel_position+k*direction;
//        if(point.x()>=box_min.x() && point.x()<=box_max.x()
//             && point.z()>=box_min.z() && point.z()<=box_max.z()){
//                 a[cn++]=point;
//        }
//    }
//    if(cn<2 && direction.z()!=0){
//        k=(box_min.z()-t_pixel_position.z())/direction.z();
//        point=t_pixel_position+k*direction;
//        if(point.x()>=box_min.x() && point.x()<=box_max.x()
//            && point.y()>=box_min.y() && point.y()<=box_max.y()){
//                 a[cn++]=point;
//        }
//    }
//    if(cn<2 && direction.z()!=0){
//        k=(box_max.z()-t_pixel_position.z())/direction.z();
//        point=t_pixel_position+k*direction;
//        if(point.x()>=box_min.x() && point.x()<=box_max.x()
//             && point.y()>=box_min.y() && point.y()<=box_max.y()){
//                 a[cn++]=point;
//        }
//    }
//    if(cn!=2){
//        return -1;
//    }
//    if((a[0]-view_eye_position).length()>(a[1]-view_eye_position).length()){
//        QVector3D t=a[0];
//        a[0]=a[1];
//        a[1]=t;
//    }

//    int sampling_num=int((a[1]-a[0]).length()*sample/scale*sampling_rate);
//    if(sampling_num==0){
//        return -1;
//    }
//    QVector3D normalize_box=QVector3D(1.0/(box_max.x()-box_min.x()),1.0/(box_max.y()-box_min.y()),1.0/(box_max.z()-box_min.z()));
//    QVector3D dir=(a[1]-a[0])/sampling_num;
//    QVector3D cur_location=a[0];

//    for(int i=0;i<sampling_num;i++){
//        cur_location=cur_location+dir;
//        QVector3D object_location=(cur_location-box_min)*normalize_box*QVector3D(w-1,h-1,d-1);
//        int data_location=int(object_location.z())*w*h+int(object_location.y())*w+int(object_location.x());
//        int value=dataList[c_window->curDataIndex]->label_data[data_location];

//        if(value!=0){
//            int len=dataList[c_window->curDataIndex]->labelIndex.size();
//            for(int j=0;j<len;j++){
//                if(dataList[c_window->curDataIndex]->labelIndex[j]==value){
//                    return dataList[c_window->curDataIndex]->spineIndex[j];
//                }
//            }
//        }
//    }
    return -1;
}


