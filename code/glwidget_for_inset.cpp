///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////


#include "glwidget_for_inset.h"
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


QVector3D GlWidgetForInset::cross_product(QVector3D v1,QVector3D v2){
    QVector3D v;
    v.setX(v1.y()*v2.z()-v2.y()*v1.z());
    v.setY(v1.z()*v2.x()-v2.z()*v1.x());
    v.setZ(v1.x()*v2.y()-v2.x()*v1.y());
    return v;
}

GlWidgetForInset::GlWidgetForInset(Window *p, MitoDataset *M, int ind, int type,QVector2D startPos,QVector2D endPos, QWidget *parent)
    : QOpenGLWidget(parent),
      m_program(0)
{

//    QPalette pal = palette();
//    pal.setColor(QPalette::Background, QColor(0,0,0,0));
//    setAutoFillBackground(true);
//    setPalette(pal);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
//    setAttribute(Qt::WA_PaintOnScreen);
//    setAttribute(Qt::WA_TransparentForMouseEvents);


    inset_type=type;
    c_window=p;
    curData=M;

    inset_startPos=startPos;
    inset_endPos=endPos;

    inset_imageSize=inset_endPos-inset_startPos;

    mitoIndex=ind;
//    setAutoFillBackground(true);


    QWidget::setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true);
}

GlWidgetForInset::~GlWidgetForInset()
{
    cleanup();
}

QSize GlWidgetForInset::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize GlWidgetForInset::sizeHint() const
{
    return QSize(1600, 1600);
}



void GlWidgetForInset::cleanup()
{
    makeCurrent();
    delete m_program;
    m_program = 0;
    doneCurrent();
}



void GlWidgetForInset::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GlWidgetForInset::cleanup);
    initializeOpenGLFunctions();
    glClearColor(0,0,0,0);

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);


    if(inset_type==0){
        vshader->compileSourceFile("Shaders/vshader_inset_structure.glsl");
        fshader->compileSourceFile("Shaders/fshader_inset_structure.glsl");
    }

    if(inset_type==1){
        vshader->compileSourceFile("Shaders/vshader_inset_mito.glsl");
        fshader->compileSourceFile("Shaders/fshader_inset_mito.glsl");
    }



    m_program = new QOpenGLShaderProgram;
    m_program->addShader(vshader);
    m_program->addShader(fshader);

    m_program->link();
    m_program->bind();


    con_tex_background=m_program->uniformLocation("tex_background");
    con_tex_label=m_program->uniformLocation("tex_label");
    con_mousePos=m_program->uniformLocation("mousePos");
    con_brushSize=m_program->uniformLocation("brushSize");
    con_brushColor=m_program->uniformLocation("brushColor");


    con_window_size=m_program->uniformLocation("window_size");

    con_imageStart=m_program->uniformLocation("imageStart");
    con_imageEnd=m_program->uniformLocation("imageEnd");


    con_color_table=m_program->uniformLocation("color_table");


    glGenTextures(1,&tex_color_table);
    glBindTexture(GL_TEXTURE_2D,tex_color_table);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,
                    256,
                    256,
                    0,GL_RED,GL_FLOAT,
                    0);

//    if(inset_type==0){
//        con_color_table=m_program->uniformLocation("color_table");
//    }
//    if(inset_type==1){
//        con_color_table=m_program->uniformLocation("color_table");
//    }



    background_data=new unsigned short[inset_imageSize.x()*inset_imageSize.y()];
    label_data=new unsigned char[inset_imageSize.x()*inset_imageSize.y()];
    for(int iy=inset_startPos.y();iy<inset_endPos.y();iy++){
        for(int ix=inset_startPos.x();ix<inset_endPos.x();ix++){
            if(inset_type==1){
                background_data[int((iy-inset_startPos.y())*inset_imageSize.x() + (ix-inset_startPos.x()))]=curData->imageData2[iy*curData->ImageW + ix];
                if(curData->mitoLabelImage[iy*curData->ImageW + ix]==mitoIndex){
                    label_data[int((iy-inset_startPos.y())*inset_imageSize.x() + (ix-inset_startPos.x()))]=255;
                }
                else{
                    label_data[int((iy-inset_startPos.y())*inset_imageSize.x() + (ix-inset_startPos.x()))]=0;
                }
            }
            else{
                if(curData->tracingMap[iy*curData->ImageW + ix].neurite_ind==mitoIndex){
                    sample_point=iy*curData->ImageW + ix;
                }
                background_data[int((iy-inset_startPos.y())*inset_imageSize.x() + (ix-inset_startPos.x()))]=curData->imageData1[iy*curData->ImageW + ix];
                label_data[int((iy-inset_startPos.y())*inset_imageSize.x() + (ix-inset_startPos.x()))]=curData->structureData[iy*curData->ImageW + ix];
            }
        }
    }

    glGenTextures(1,&tex_background);
    glBindTexture(GL_TEXTURE_2D,tex_background);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,
                    inset_imageSize.x(),
                    inset_imageSize.y(),
                    0,GL_RED,GL_UNSIGNED_SHORT,
                    background_data);

    glGenTextures(1,&tex_label);
    glBindTexture(GL_TEXTURE_2D,tex_label);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    if(inset_type==1){
        glTexImage2D(GL_TEXTURE_2D,0,GL_RED,
                        inset_imageSize.x(),
                        inset_imageSize.y(),
                        0,GL_RED,GL_UNSIGNED_BYTE,
                        label_data);
    }
    else{
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,
                        inset_imageSize.x(),
                        inset_imageSize.y(),
                        0,GL_RGBA,GL_UNSIGNED_BYTE,
                        0);
        setStructureTex();
    }




    m_program->release();

}
void GlWidgetForInset::paintGL()
{
    qDebug()<<"start inset gl";

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

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    m_program->bind();


    m_program->setUniformValue(con_window_size,QVector2D(this->width(),this->height()));
    m_program->setUniformValue(con_mousePos,mousePoint);


    if(c_window->dataRendering->current_inset==mitoIndex){
        m_program->setUniformValue(con_brushSize,curData->brushSize);
//        isFocused=false;
    }
    else{
        m_program->setUniformValue(con_brushSize,0);
    }
    m_program->setUniformValue(con_brushColor,QVector3D(1,0,0));

    m_program->setUniformValue(con_imageStart,inset_startPos);
    m_program->setUniformValue(con_imageEnd,inset_endPos);


    if(mitoLabelChanged){
        setMitoLabelTex();
        mitoLabelChanged=false;
    }


    if(structureChanged){
        setStructureTex();
        structureChanged=false;
    }


    if(colorTableChanged){
        glBindTexture(GL_TEXTURE_2D,tex_color_table);
        if(inset_type==0){
            glTexSubImage2D(GL_TEXTURE_2D,0,0,0,256,256,GL_RED,GL_FLOAT,curData->color_table_neuron);
        }
        if(inset_type==1){
            glTexSubImage2D(GL_TEXTURE_2D,0,0,0,256,256,GL_RED,GL_FLOAT,curData->color_table_mito);
        }

        colorTableChanged=false;
    }


    glActiveTexture(GL_TEXTURE0);
    glUniform1i(con_tex_background,0);
    glBindTexture(GL_TEXTURE_2D,tex_background);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(con_tex_label,1);
    glBindTexture(GL_TEXTURE_2D,tex_label);


    glActiveTexture(GL_TEXTURE2);
    glUniform1i(con_color_table,2);
    glBindTexture(GL_TEXTURE_2D,tex_color_table);



    glBegin(GL_QUADS);
        glVertex3f(-1,-1,1);
        glVertex3f(-1,1,1);
        glVertex3f(1,1,1);
        glVertex3f(1,-1,1);
    glEnd();

    m_program->release();


    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing,true);

    QPen boundaryPen(QColor(255,255,0,150),3,Qt::DashLine);
    painter.setPen(boundaryPen);
    painter.drawEllipse(QPoint(this->width()*0.5,this->height()*0.5),int(this->width()*0.5),int(this->height()*0.5));

    painter.end();
}

void GlWidgetForInset::setStructureTex(){
    unsigned char *tData=new unsigned char[int(inset_imageSize.x()*inset_imageSize.y()*4)];
    for(int iy=inset_startPos.y();iy<inset_endPos.y();iy++){
        for(int ix=inset_startPos.x();ix<inset_endPos.x();ix++){
            int tP=iy*curData->ImageW+ix;
            int tS=curData->structureData[tP];
            int tP2=int(iy-inset_startPos.y())*inset_imageSize.x() + (ix-inset_startPos.x());
            tData[tP2*4+0]=0;
            tData[tP2*4+1]=0;
            tData[tP2*4+2]=0;
            tData[tP2*4+3]=0;

            if(tS==0){
                tS=curData->connectionStructureData[tP];
            }

            if(tS==0){
                tData[tP2*4+0]=0;
                tData[tP2*4+1]=0;
                tData[tP2*4+2]=0;
                tData[tP2*4+3]=0;
            }
            else if(tS==10){

            }
            else{
                QColor tC=c_window->typeColors[tS];
                tData[tP2*4+0]=tC.red();
                tData[tP2*4+1]=tC.green();
                tData[tP2*4+2]=tC.blue();
                tData[tP2*4+3]=curData->structure_opacity*255;
            }

            if(curData->tracingMap[tP].type>=0){
                int neu=curData->tracingMap[curData->tracingMap[tP].posy*curData->ImageW + curData->tracingMap[tP].posx].neurite_ind;
                int target_neurite=mitoIndex;

                if(target_neurite==neu){
                    sample_point=curData->tracingMap[tP].posy*curData->ImageW + curData->tracingMap[tP].posx;
                    bool is_boundary=false;
                    int leftx=ix-1;
                    int rightx=ix+1;
                    int upy=iy+1;
                    int downy=iy-1;
                    if(leftx>=inset_startPos.x() && leftx<inset_endPos.x()){
                        int newP=iy*curData->ImageW+leftx;
                        if(curData->tracingMap[newP].type<0){
                            is_boundary=true;
                        }
                        else if(target_neurite!=curData->tracingMap[curData->tracingMap[newP].posy*curData->ImageW + curData->tracingMap[newP].posx].neurite_ind){
                            is_boundary=true;
                        }
                    }
                    if(rightx>=inset_startPos.x() && rightx<inset_endPos.x()){
                        int newP=iy*curData->ImageW+rightx;
                        if(curData->tracingMap[newP].type<0){
                            is_boundary=true;
                        }
                        else if(target_neurite!=curData->tracingMap[curData->tracingMap[newP].posy*curData->ImageW + curData->tracingMap[newP].posx].neurite_ind){
                            is_boundary=true;
                        }
                    }
                    if(upy>=inset_startPos.y() && upy<inset_endPos.y()){
                        int newP=upy*curData->ImageW+ix;
                        if(curData->tracingMap[newP].type<0){
                            is_boundary=true;
                        }
                        else if(target_neurite!=curData->tracingMap[curData->tracingMap[newP].posy*curData->ImageW + curData->tracingMap[newP].posx].neurite_ind){
                            is_boundary=true;
                        }
                    }
                    if(downy>=inset_startPos.y() && downy<inset_endPos.y()){
                        int newP=downy*curData->ImageW+ix;
                        if(curData->tracingMap[newP].type<0){
                            is_boundary=true;
                        }
                        else if(target_neurite!=curData->tracingMap[curData->tracingMap[newP].posy*curData->ImageW + curData->tracingMap[newP].posx].neurite_ind){
                            is_boundary=true;
                        }
                    }

                    if(is_boundary){
                        QColor tC;
                        if(tS==2){
                            tC=QColor(0,0,255);
                        }
                        if(tS==3){
                            tC=QColor(255,0,0);
                        }
                        tData[tP2*4+0]=tC.red();
                        tData[tP2*4+1]=tC.green();
                        tData[tP2*4+2]=tC.blue();
                        tData[tP2*4+3]=255;
                    }
                }
            }
        }
    }


    glBindTexture(GL_TEXTURE_2D,tex_label);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,
                    inset_imageSize.x(),
                    inset_imageSize.y(),
                    GL_RGBA,GL_UNSIGNED_BYTE,
                    tData);
    delete []tData;

    c_window->dataRendering->structureChanged=true;



}



void GlWidgetForInset::setMitoLabelTex(){
    return;

    for(int iy=inset_startPos.y();iy<inset_endPos.y();iy++){
        for(int ix=inset_startPos.x();ix<inset_endPos.x();ix++){
            if(curData->mitoLabelImage[iy*curData->ImageW + ix]==mitoIndex){
                label_data[int((iy-inset_startPos.y())*inset_imageSize.x() + (ix-inset_startPos.x()))]=255;
            }
            else{
                label_data[int((iy-inset_startPos.y())*inset_imageSize.x() + (ix-inset_startPos.x()))]=0;
            }
        }
    }
    glBindTexture(GL_TEXTURE_2D,tex_label);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,
                    inset_imageSize.x(),
                    inset_imageSize.y(),
                    GL_RED,GL_UNSIGNED_BYTE,
                    label_data);
}



void GlWidgetForInset::resizeGL(int w, int h)
{
    int t=w>h?h:w;
    this->resize(t,t);
}

void GlWidgetForInset::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
    mousePressedPos=event->pos();

    if(event->buttons() & Qt::LeftButton){
        if(sample_point==-1){
            qDebug()<<"??";
            return;
        }
        int cur_type=curData->structureData[sample_point];
        if(cur_type==2)
            c_window->dataRendering->correctNeurite(QVector2D(sample_point%curData->ImageW,sample_point/curData->ImageW),3);
        if(cur_type==3)
            c_window->dataRendering->correctNeurite(QVector2D(sample_point%curData->ImageW,sample_point/curData->ImageW),2);
        structureChanged=true;
//        setStructureTex();
        update();
    }

    if(event->buttons() & Qt::RightButton){
        if(sample_point==-1){
            qDebug()<<"??";
            return;
        }

        QVector2D curPoint=QVector2D(event->x()-(this->width()-curData->ImageW*curData->scale)/2.0-curData->shift.x()*curData->scale
                                     ,event->y()-(this->height()-curData->ImageH*curData->scale)/2.0-curData->shift.y()*curData->scale);
        curPoint=curPoint/curData->scale;


        QVector2D midP=QVector2D(this->width()*0.5-(this->width()-curData->ImageW*curData->scale)/2.0,
                                 this->height()*0.5-(this->height()-curData->ImageH*curData->scale)/2.0);
        midP=midP/curData->scale;



        curData->shift=QVector2D(midP.x()-sample_point%curData->ImageW,midP.y()-sample_point/curData->ImageW);
        curData->scale=3.0;

        curData->focus_fixed_point=-1;
        for(int i=0;i<curData->radars.size();i++){
            curData->radars[i]->is_focused=false;
            curData->radars[i]->is_focus_fixed=false;
//            curData->radars[i]->hide();
            curData->radars[i]->update();
        }

        c_window->dataRendering->update();
        update();
    }


    return;

    if(c_window->curDataIndex!=-1){
        QVector2D curPoint=inset_startPos+inset_imageSize*QVector2D(float(event->x())/this->width(),float(event->y())/this->height());
        mousePoint=curPoint;

        if(curData->correctorInputType>=10 && curData->correctorInputType<14 && c_window->imageControlPartLayout->currentIndex()==3 && (event->buttons() & Qt::LeftButton)){
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
    }
}

void GlWidgetForInset::mouseReleaseEvent(QMouseEvent *event)
{

    return;


    if(doMitoBrushing){
        if(c_window->curDataIndex!=-1){
            MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
            if(curData->correctorInputType==10){
                c_window->dataRendering->MitoExcluding();
            }
            if(curData->correctorInputType==11){
                c_window->dataRendering->MitoSplitting();
            }
            if(curData->correctorInputType==12){
                c_window->dataRendering->MitoMerging();
            }
            if(curData->correctorInputType==13){
                c_window->dataRendering->MitoIncluding();
            }
            mitoUpdate=true;
        }
        doMitoBrushing=false;
    }

    if(mitoUpdate){
        emit c_window->mitoUpdate();
        mitoUpdate=false;
    }

    update();

}
void GlWidgetForInset::mouseMoveEvent(QMouseEvent *event)
{

    return;


    c_window->dataRendering->current_inset=mitoIndex;
    //setFocus();

    isFocused=true;

    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    if(c_window->curDataIndex==-1){
        m_lastPos=event->pos();
        return;
    }

    QVector2D curPoint=inset_startPos+inset_imageSize*QVector2D(float(event->x())/this->width(),float(event->y())/this->height());
    mousePoint=curPoint;

    if(curData->correctorInputType>=10 && curData->correctorInputType<14 && c_window->imageControlPartLayout->currentIndex()==3 &&(event->buttons() & Qt::LeftButton)&& !IsSpacePressed){
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


//    if(event->buttons() & Qt::LeftButton){
//        mitoUpdate=true;
//        correctionChanged=true;
//        structureChanged=true;

//    }

    update();
    m_lastPos=event->pos();

}

void GlWidgetForInset::wheelEvent(QWheelEvent *event)
{
    return;
    isFocused=true;
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numDegrees.isNull()) {
        QPoint numSteps = numDegrees / 15;
        if(c_window->curDataIndex!=-1){
 //           if(curData->subsetInputType!=0 && c_window->imageControlPartLayout->currentIndex()==4){
                curData->brushSize+=numSteps.x()+numSteps.y();
                if(curData->brushSize<1)curData->brushSize=1;
                if(curData->brushSize>50)curData->brushSize=50;
                c_window->subsetBrushSize->setValue(curData->brushSize);
                update();
//            }
        }

    }
    event->accept();
}

void GlWidgetForInset::timerEvent(QTimerEvent *event){

    update();
}
void GlWidgetForInset::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Space){
        IsSpacePressed=true;
    }
}
void GlWidgetForInset::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Space){
        IsSpacePressed=false;
    }
}

