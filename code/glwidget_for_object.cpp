///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////


#include "glwidget_for_object.h"
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


QVector3D GlWidgetForObject::cross_product(QVector3D v1,QVector3D v2){
    QVector3D v;
    v.setX(v1.y()*v2.z()-v2.y()*v1.z());
    v.setY(v1.z()*v2.x()-v2.z()*v1.x());
    v.setZ(v1.x()*v2.y()-v2.x()*v1.y());
    return v;
}

GlWidgetForObject::GlWidgetForObject(Window *p, QWidget *parent)
    : QOpenGLWidget(parent),
      m_program(0)
{

    p_fps="";
    fps_start=-1;
    fps=0;

    c_window=p;

    //startTimer(100);
    QWidget::setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true);

}

GlWidgetForObject::~GlWidgetForObject()
{
    cleanup();
}

QSize GlWidgetForObject::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize GlWidgetForObject::sizeHint() const
{
    return QSize(1600, 1600);
}



void GlWidgetForObject::cleanup()
{
    makeCurrent();
    delete m_program;
    m_program = 0;
    doneCurrent();
}



void GlWidgetForObject::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GlWidgetForObject::cleanup);
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

    m_program->release();

}
void GlWidgetForObject::ReadLabelColor(){

}
void GlWidgetForObject::ReadColorTable(){

}
void GlWidgetForObject::interpolation_type_change(int a){

}
void GlWidgetForObject::paintGL()
{
    qDebug()<<"start object gl";

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
    if(c_window->imageControlPartLayout->currentIndex()==4 && curData->subsetInputType!=0 && !IsSpacePressed){
        m_program->setUniformValue(con_brushSize,curData->brushSize);
    }
    else if(c_window->imageControlPartLayout->currentIndex()==3 && curData->correctorInputType!=-1
            && curData->correctorInputType!=4 && !IsSpacePressed){
        m_program->setUniformValue(con_brushSize,curData->brushSize);
    }
    else m_program->setUniformValue(con_brushSize,int(0));

    if(c_window->imageControlPartLayout->currentIndex()==4){
        if(curData->subsetInputType==1){
            m_program->setUniformValue(con_brushColor,QVector3D(0,1,0));
        }
        else if(curData->subsetInputType==2){
            m_program->setUniformValue(con_brushColor,QVector3D(1,0,0));
        }
        else m_program->setUniformValue(con_brushColor,QVector3D(0,0,1));
    }
    m_program->setUniformValue(con_structure_prev,int(0));
    if(c_window->imageControlPartLayout->currentIndex()==3){
        if(curData->correctorInputType==0){
            m_program->setUniformValue(con_brushColor,QVector3D(0,0,0));
        }
        else if(curData->correctorInputType==1){
            m_program->setUniformValue(con_structure_prev,int(1));
            m_program->setUniformValue(con_brushColor,QVector3D(120.0/255,50.0/255,220.0/255));
        }
        else if(curData->correctorInputType==2){
            m_program->setUniformValue(con_structure_prev,int(1));
            m_program->setUniformValue(con_brushColor,QVector3D(255.0/255,142.0/255,138.0/255));
        }
        else if(curData->correctorInputType==3){
            m_program->setUniformValue(con_structure_prev,int(1));
            m_program->setUniformValue(con_brushColor,QVector3D(30.0/255,126.0/255,220.0/255));
        }
        else if(curData->correctorInputType==5){
            m_program->setUniformValue(con_brushColor,QVector3D(0,220.0/255,220.0/255));
        }
        else if(curData->correctorInputType==6){
            m_program->setUniformValue(con_brushColor,QVector3D(0,50.0/255,100.0/255));
        }
        else m_program->setUniformValue(con_brushColor,QVector3D(0,0,1));
    }






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
        painter.drawLine(measureStart.x(),measureStart.y(),
                         measureEnd.x(),measureEnd.y());

        QVector2D p1=measureStart
                -QVector2D(this->width()-curData->ImageW*curData->scale,this->height()-curData->ImageH*curData->scale)/2
                -curData->shift*curData->scale;
        p1=p1/curData->scale;

        QVector2D p2=measureEnd
                -QVector2D(this->width()-curData->ImageW*curData->scale,this->height()-curData->ImageH*curData->scale)/2
                -curData->shift*curData->scale;
        p2=p2/curData->scale;


        painter.drawText(measureStart.x(),measureStart.y(),QString::number((p1-p2).length()*0.2094,'g',4)+" micron");
    }
    painter.end();



//    drawTracingLines();
    //qDebug()<<"end glwidget for data";


}

void GlWidgetForObject::drawTracingLines(){
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

                if(curData->tracingMap[tP].connected_nodes.size()<=2){
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
void GlWidgetForObject::setConnectionTex(){
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
void GlWidgetForObject::setSubsetTex(){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    unsigned char *tData=new unsigned char[curData->ImageW*curData->ImageH*4]();
    if(c_window->imageControlPartLayout->currentIndex()==4){
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
void GlWidgetForObject::setStructureTex(){
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
                tData[tP*4+0]=100;
                tData[tP*4+1]=200;
                tData[tP*4+2]=255;
                tData[tP*4+3]=curData->structure_opacity*255;

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
    glBindTexture(GL_TEXTURE_2D,curData->tex_structure);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,
                    curData->ImageW,
                    curData->ImageH,
                    GL_RGBA,GL_UNSIGNED_BYTE,
                    tData);
    delete []tData;

}
void GlWidgetForObject::setCorrectionTex(){
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





    if((curData->correctorInputType==2||curData->correctorInputType==3) && c_window->imageControlPartLayout->currentIndex()==3){

    }


    glBindTexture(GL_TEXTURE_2D,curData->tex_correction);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,
                    curData->ImageW,
                    curData->ImageH,
                    GL_RGBA,GL_UNSIGNED_BYTE,
                    tData);
    delete []tData;

}
void GlWidgetForObject::setMitoLabelTex(){

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
    curData->saveMitoLabel(curData->path+"refined_mito_label.tif",tData);

    glBindTexture(GL_TEXTURE_2D,curData->tex_mitoLabel_original);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,
                    curData->ImageW,
                    curData->ImageH,
                    GL_RED,GL_UNSIGNED_BYTE,
                    tData);




    delete []tData;

}

void GlWidgetForObject::resizeGL(int w, int h)
{
//    int t=w>h?w:h;
//    this->resize(t,t);
}

void GlWidgetForObject::mousePressEvent(QMouseEvent *event)
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

//            if(curData->correctorInputType==5 && c_window->imageControlPartLayout->currentIndex()==3 && (event->buttons() & Qt::LeftButton)){
//                if(ck_brushing!=NULL){
//                    delete []ck_brushing;
//                }
//                ck_brushing=new bool[curData->ImageW * curData->ImageH]();
//                for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
//                    for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
//                        if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
//                        float dis=(QVector2D(ix,iy)-mousePoint).length();
//                        if(dis<=curData->brushSize){
//                            int tP=iy*curData->ImageW+ix;
//                            curData->correctionData[tP]=5;
//                            curData->user_define_cost[tP]=0.2;
//                            curData->user_brushing_buffer[tP]=5;
//                            ck_brushing[tP]=true;
//                        }
//                    }
//                }
////                emit curData->correctorUserCostBrushing();
//                correctionChanged=true;

//            }
//            else if(curData->correctorInputType==6 && c_window->imageControlPartLayout->currentIndex()==3 && (event->buttons() & Qt::LeftButton)){
//                if(ck_brushing!=NULL){
//                    delete []ck_brushing;
//                }
//                ck_brushing=new bool[curData->ImageW * curData->ImageH]();
//                for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
//                    for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
//                        if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
//                        float dis=(QVector2D(ix,iy)-mousePoint).length();
//                        if(dis<=curData->brushSize){
//                            int tP=iy*curData->ImageW+ix;
//                            curData->correctionData[tP]=6;
//                            curData->user_define_cost[tP]=5;
//                            curData->user_brushing_buffer[tP]=6;
//                            ck_brushing[tP]=true;
//                        }
//                    }
//                }
//                emit curData->correctorUserCostBrushing();
//                correctionChanged=true;

//            }
            if(((curData->correctorInputType>=0 && curData->correctorInputType<4) || curData->correctorInputType==7) && c_window->imageControlPartLayout->currentIndex()==3 && (event->buttons() & Qt::LeftButton)){
//                if(ck_brushing!=NULL){
//                    delete []ck_brushing;
//                }
//                ck_brushing=new bool[curData->ImageW * curData->ImageH]();
                mitoUpdate=true;
                for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
                    for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                        if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                        float dis=(QVector2D(ix,iy)-mousePoint).length();
                        if(dis<=curData->brushSize){
                            int tP=iy*curData->ImageW+ix;
                            if(curData->correctorInputType==0){
                                curData->structureData[tP]=curData->correctorInputType;
                            }
                            else{
                                float v=curData->color_table_neuron[curData->imageData1[tP]];
                                if(v>curData->structure_thresh){
                                    curData->structureData[tP]=curData->correctorInputType;
                                }
                            }
//                            curData->correctionData[tP]=2;
//                            curData->user_brushing_buffer[tP]=curData->correctorInputType;
//                            ck_brushing[tP]=true;
                        }
                    }
                }
//                emit curData->correctorStructureBrushing();
                correctionChanged=true;
                structureChanged=true;

            }


            if(curData->subsetInputType==1 && c_window->imageControlPartLayout->currentIndex()==4 && (event->buttons() & Qt::LeftButton)){
                for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
                    for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                        if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                        float dis=(QVector2D(ix,iy)-mousePoint).length();
                        if(dis<=curData->brushSize)curData->subsetData[iy*curData->ImageW+ix]=1;
                    }
                }
                subsetChanged=true;
            }
            else if(curData->subsetInputType==2 && c_window->imageControlPartLayout->currentIndex()==4 && (event->buttons() & Qt::LeftButton)){
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

//    c_window->ImageControlSliderChanged(0);

}

void GlWidgetForObject::mouseReleaseEvent(QMouseEvent *event)
{
//    if(c_window->curDataIndex!=-1){
//        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
//        if(((curData->correctorInputType>=0 && curData->correctorInputType<4) || curData->correctorInputType==7) && c_window->imageControlPartLayout->currentIndex()==3 && event->button()==Qt::LeftButton){
//            emit curData->userInteractFinished();
//        }
//    }

//    if((event->pos()-mousePressedPos).manhattanLength()<3 && event->button()==Qt::LeftButton){
//        if(c_window->curDataIndex!=-1){
//            MitoDataset *curData=c_window->dataset[c_window->curDataIndex];


//            if(curData->correctorInputType==4 && c_window->imageControlPartLayout->currentIndex()==3){
//                QVector2D curPoint=QVector2D(event->x()-(this->width()-curData->ImageW*curData->scale)/2.0-curData->shift.x()*curData->scale
//                                             ,event->y()-(this->height()-curData->ImageH*curData->scale)/2.0-curData->shift.y()*curData->scale);
//                curPoint=curPoint/curData->scale;
//                if(curPoint.x()>=0 && curPoint.y()>=0 && curPoint.x()<curData->ImageW-1 && curPoint.y()<curData->ImageH-1){
//                    emit curData->correctorTracingPointAdded(int(curPoint.y())*curData->ImageW+int(curPoint.x()));
//                }
//            }
//        }
//    }

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
    update();

}
void GlWidgetForObject::mouseMoveEvent(QMouseEvent *event)
{
    setFocus();

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

//    if(curData->correctorInputType==5 && c_window->imageControlPartLayout->currentIndex()==3 && (event->buttons() & Qt::LeftButton)&& !IsSpacePressed){
//        for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
//            for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
//                if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
//                float dis=(QVector2D(ix,iy)-mousePoint).length();
//                if(dis<=curData->brushSize){
//                    int tP=iy*curData->ImageW+ix;
//                    if(ck_brushing[tP]==false){
//                        curData->correctionData[tP]=5;
//                        curData->user_define_cost[tP]=0.2;
//                        curData->user_brushing_buffer[tP]=5;
//                        ck_brushing[tP]=true;
//                    }
//                }

//            }
//        }
//        emit curData->correctorUserCostBrushing();
//        correctionChanged=true;

//    }
//    else if(curData->correctorInputType==6 && c_window->imageControlPartLayout->currentIndex()==3 && (event->buttons() & Qt::LeftButton)&& !IsSpacePressed){
//        for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
//            for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
//                if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
//                float dis=(QVector2D(ix,iy)-mousePoint).length();
//                if(dis<=curData->brushSize){
//                    int tP=iy*curData->ImageW+ix;
//                    if(ck_brushing[tP]==false){
//                        curData->correctionData[tP]=6;
//                        curData->user_define_cost[tP]=5;
//                        curData->user_brushing_buffer[tP]=6;
//                        ck_brushing[tP]=true;
//                    }
//                }
//            }
//        }
//        emit curData->correctorUserCostBrushing();
//        correctionChanged=true;

//    }
    if(((curData->correctorInputType>=0 && curData->correctorInputType<4) || curData->correctorInputType==7) && c_window->imageControlPartLayout->currentIndex()==3 && (event->buttons() & Qt::LeftButton)&& !IsSpacePressed){
        mitoUpdate=true;
        for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
            for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                float dis=(QVector2D(ix,iy)-mousePoint).length();

                if(dis<=curData->brushSize){
                    int tP=iy*curData->ImageW+ix;
                    if(curData->correctorInputType==0){
                        curData->structureData[tP]=curData->correctorInputType;
                    }
                    else{
                        float v=curData->color_table_neuron[curData->imageData1[tP]];
                        if(v>curData->structure_thresh){
                            curData->structureData[tP]=curData->correctorInputType;
                        }
                    }
                }
            }
        }
//        emit curData->correctorStructureBrushing();
        correctionChanged=true;
        structureChanged=true;

    }
    else if(curData->subsetInputType==1 && c_window->imageControlPartLayout->currentIndex()==4 && (event->buttons() & Qt::LeftButton) && !IsSpacePressed){
        for(int iy=mousePoint.y()-curData->brushSize;iy<mousePoint.y()+curData->brushSize;iy++){
            for(int ix=mousePoint.x()-curData->brushSize;ix<mousePoint.x()+curData->brushSize;ix++){
                if(iy<0 || ix<0 || iy>=curData->ImageH || ix>=curData->ImageW)continue;
                float dis=(QVector2D(ix,iy)-mousePoint).length();
                if(dis<=curData->brushSize)curData->subsetData[iy*curData->ImageW+ix]=1;
            }
        }
        subsetChanged=true;
    }
    else if(curData->subsetInputType==2 && c_window->imageControlPartLayout->currentIndex()==4 && (event->buttons() & Qt::LeftButton) && !IsSpacePressed){
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


    update();
    m_lastPos=event->pos();

}

void GlWidgetForObject::wheelEvent(QWheelEvent *event)
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
                update();
//            }
        }

//        c_window->roi_size+=numSteps.x()+numSteps.y();
//        if(c_window->roi_size<1)c_window->roi_size=1;
    }
    event->accept();
}

void GlWidgetForObject::timerEvent(QTimerEvent *event){

    if(fps_start==-1){
        fps_start=GetTickCount();
    }
    else{
        if(GetTickCount()-fps_start>1000){
            char itoa_t[10];
            p_fps=QString(itoa(fps,itoa_t,10));
            fps=0;
            fps_start=GetTickCount();
        }
    }

    update();
}
void GlWidgetForObject::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Space){
        IsSpacePressed=true;
    }
}
void GlWidgetForObject::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Space){
        IsSpacePressed=false;
    }
}



void GlWidgetForObject::cut_plane(int r){
}


void GlWidgetForObject::set_z_scale(int n){
}



void GlWidgetForObject::ambi_change(int a){
}

void GlWidgetForObject::diff_change(int a){
}

void GlWidgetForObject::spec_change(int a){
}
void GlWidgetForObject::shin_change(int a){
}
void GlWidgetForObject::l_t_change(int a){
}

void GlWidgetForObject::change_R1(int a){
}

void GlWidgetForObject::change_R2(int a){
}

void GlWidgetForObject::change_cut_d(int a){

}
void GlWidgetForObject::change_cut_enable(bool a){
}
void GlWidgetForObject::change_cut_alpha(int a){
}
void GlWidgetForObject::change_phong_enable(bool a){
}
void GlWidgetForObject::change_sampling_rate(int a){

}
int GlWidgetForObject::getObjectCoordfromMousePoint(QVector2D mousePoint){



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


