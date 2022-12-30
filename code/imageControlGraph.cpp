///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

#include "imageControlGraph.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>
#include <QPainter>
#include <QtGui>
#include <QDockWidget>
#include <QColorDialog>

#include <window.h>
#include <iostream>
#include <QDebug>
#include <QCheckBox>




imageControlGraph::imageControlGraph(int type,Window *p, QWidget *parent)
    : QWidget(parent)
{
    image_type=type;

    c_window=p;

    setBackgroundRole(QPalette::Base);
    QWidget::setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);


}

imageControlGraph::~imageControlGraph(){
}

void imageControlGraph::resizeEvent(QResizeEvent *event){
//    int w=this->width();
//    int h=this->height();
//    if(w<h){
//        this->resize(w,w);
//    }
//    if(h<w){
//        this->resize(h,h);
//    }

}

QSize imageControlGraph::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize imageControlGraph::sizeHint() const
{
    return QSize(800, 800);
}

void imageControlGraph::paintEvent(QPaintEvent *)
{

    qDebug()<<"start img ctl gl";

    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing,true);

    QPen t_pen;
    QBrush t_brush(Qt::SolidPattern);
    t_brush.setColor(QColor(0,0,0));

    painter.setBrush(t_brush);
    painter.drawRect(0,0,this->width(),this->height());

    t_pen.setWidth(1);
    t_pen.setColor(QColor(255,255,255));
    painter.setPen(t_pen);

    if(c_window->curDataIndex!=-1){
        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
        for(int i=1;i<256;i++){
            QPoint startPos;
            QPoint endPos;
            startPos.setX(float(this->width()*0.9)/256*(i-1));
            endPos.setX(float(this->width()*0.9)/256*i);

            if(image_type==0){
                startPos.setY(this->height()-this->height()*curData->color_table_neuron[(i-1)*256]);
                endPos.setY(this->height()-this->height()*curData->color_table_neuron[i*256]);
            }
            else{
                startPos.setY(this->height()-this->height()*curData->color_table_mito[(i-1)*256]);
                endPos.setY(this->height()-this->height()*curData->color_table_mito[i*256]);
            }

            painter.drawLine(startPos,endPos);
        }
    }

    t_pen.setWidth(3);
    t_pen.setStyle(Qt::DashLine);
    painter.setPen(t_pen);
    if(image_type==0){
        painter.drawLine(0,this->height()-this->height()*float(c_window->imageBrightControl_neuron->value())/255.0,
                         this->width()*0.9,this->height()-this->height()*float(c_window->imageBrightControl_neuron->value())/255.0);

        painter.drawLine(this->width()*0.9*float(c_window->imageContrastPosControl_neuron->value())/255.0,0,
                         this->width()*0.9*float(c_window->imageContrastPosControl_neuron->value())/255.0,this->height());
    }
    else{
        painter.drawLine(0,this->height()-this->height()*float(c_window->imageBrightControl_mito->value())/255.0,
                         this->width()*0.9,this->height()-this->height()*float(c_window->imageBrightControl_mito->value())/255.0);

        painter.drawLine(this->width()*0.9*float(c_window->imageContrastPosControl_mito->value())/255.0,0,
                         this->width()*0.9*float(c_window->imageContrastPosControl_mito->value())/255.0,this->height());

    }



    QLinearGradient gradient;
    gradient.setStart(this->width()*0.95,this->height());
    gradient.setFinalStop(this->width()*0.95,0);

    if(image_type==0){
        gradient.setColorAt(0.0,QColor(0,0,0,0));
        gradient.setColorAt(1.0, QColor(31,255,0,255));
    }
    if(image_type==1){
        if(c_window->grayscale_colormap->isChecked()){
            gradient.setColorAt(0.0,QColor(0,0,0,0));
            gradient.setColorAt(1.0, QColor(255,255,255,255));

        }
        else{
            gradient.setColorAt(0.0,QColor(0,0,0,0));
            gradient.setColorAt(0.25, QColor(0,0,255,255*0.25));
            gradient.setColorAt(0.75, QColor(255,255,0,255*0.75));
            gradient.setColorAt(1.0, QColor(255,255,0,255));
        }

    }
    t_pen.setWidthF(this->width()*0.1);
    t_pen.setStyle(Qt::SolidLine);
    t_pen.setBrush(gradient);
    painter.setPen(t_pen);
    painter.drawLine(this->width()*0.95,this->height(),this->width()*0.95,0);

}

void imageControlGraph::mousePressEvent(QMouseEvent *event)
{

}
void imageControlGraph::mouseReleaseEvent(QMouseEvent *event){

}
void imageControlGraph::mouseMoveEvent(QMouseEvent *event)
{


}

void imageControlGraph::keyPressEvent(QKeyEvent *event)
{

}


void imageControlGraph::timerEvent(QTimerEvent *event){
}


