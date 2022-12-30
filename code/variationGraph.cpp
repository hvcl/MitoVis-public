///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

#include "variationGraph.h"
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




variationGraph::variationGraph(Window *p, QWidget *parent)
    : QWidget(parent)
{


    c_window=p;

    setBackgroundRole(QPalette::Base);
    QWidget::setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);


}

variationGraph::~variationGraph(){
}

void variationGraph::resizeEvent(QResizeEvent *event){

}

QSize variationGraph::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize variationGraph::sizeHint() const
{
    return QSize(800, 800);
}

void variationGraph::paintEvent(QPaintEvent *)
{

    qDebug()<<"start variation plot";

    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing,true);

    QPen t_pen;
    QBrush t_brush(Qt::SolidPattern);


    int enabledFeatureNum=6;
    t_pen.setWidth(this->width()*0.75/enabledFeatureNum);
    t_pen.setCapStyle(Qt::FlatCap);
    t_pen.setColor(QColor(245,245,245));
    painter.setPen(t_pen);

//    for(int i=0;i<6;i++){
//            painter.drawLine(this->width()*0.1+(this->width()*0.8)/(enabledFeatureNum-1)*i,0,
//                             this->width()*0.1+(this->width()*0.8)/(enabledFeatureNum-1)*i,this->height());
//    }


    QFont t_font;
    t_font.setBold(true);
    t_font.setUnderline(true);
    t_pen.setColor(QColor(50,50,50));

    painter.setFont(t_font);
    painter.setPen(t_pen);

    t_pen.setColor(QColor(150,150,150));
    t_pen.setWidth(2);
    t_pen.setCapStyle(Qt::RoundCap);

    painter.setPen(t_pen);
    painter.drawLine(0,this->height()*0.55,this->width(),this->height()*0.55);

    int pre=c_window->preGroup->value();
    int post=c_window->postGroup->value();
    if(pre!=post && pre<c_window->snapshot_count.size() && post<c_window->snapshot_count.size()){
        preTotal[0]=c_window->snapshot_count[pre];
        preTotal[1]=c_window->snapshot_density[pre];
        preTotal[2]=c_window->snapshot_features[pre][0];
        preTotal[3]=c_window->snapshot_features[pre][1];
        preTotal[4]=c_window->snapshot_features[pre][2];
        preTotal[5]=c_window->snapshot_features[pre][3];
        postTotal[0]=c_window->snapshot_count[post];
        postTotal[1]=c_window->snapshot_density[post];
        postTotal[2]=c_window->snapshot_features[post][0];
        postTotal[3]=c_window->snapshot_features[post][1];
        postTotal[4]=c_window->snapshot_features[post][2];
        postTotal[5]=c_window->snapshot_features[post][3];
        maxV=0;
        for(int i=0;i<6;i++){
            variation[i]=postTotal[i]-preTotal[i];
            if(preTotal[i]==0)continue;
            if(abs(variation[i]/preTotal[i])>maxV)maxV=abs(variation[i]/preTotal[i]);
        }
        if(maxV<1.0)maxV=1.0;

        t_pen.setWidth(30);
        t_pen.setCapStyle(Qt::FlatCap);

        if(maxV!=0){
            for(int i=0;i<6;i++){
                    float value=0;
                    if(variation[i]<0){
                        t_pen.setColor(QColor(29,125,220));
                    }
                    else{
                        t_pen.setColor(QColor(255,142,138));
                    }
                    if(preTotal[i]!=0)value=variation[i]/preTotal[i];
                    painter.setPen(t_pen);
                    painter.drawLine(this->width()*0.07+(this->width()*0.86)/(enabledFeatureNum-1)*i,this->height()*0.55,
                                     this->width()*0.07+(this->width()*0.86)/(enabledFeatureNum-1)*i,this->height()*0.55-this->height()*0.4*value/maxV);
            }
        }


        for(int i=0;i<6;i++){

            float value=0;
            int margin;
            if(variation[i]<0){
                margin=15;
            }
            else{
                margin=-5;
            }
            if(preTotal[i]!=0)value=variation[i]/preTotal[i];

            t_font.setUnderline(false);
            t_pen.setColor(QColor(100,100,100));
            painter.setPen(t_pen);
            painter.setFont(t_font);
            char tt[30];
            sprintf(tt,"%.1f%%",value*100);
            if(maxV!=0)painter.drawText(this->width()*0.07+(this->width()*0.86)/(enabledFeatureNum-1)*i-3.5*strlen(tt),this->height()*0.55-this->height()*0.4*value/maxV+margin,QString(tt));
            else painter.drawText(this->width()*0.07+(this->width()*0.86)/(enabledFeatureNum-1)*i-3.5*strlen(tt),this->height()*0.55-this->height()*0.4*value+margin,QString(tt));

        }

    }

}

void variationGraph::mousePressEvent(QMouseEvent *event)
{

}
void variationGraph::mouseReleaseEvent(QMouseEvent *event){

}
void variationGraph::mouseMoveEvent(QMouseEvent *event)
{


}

void variationGraph::keyPressEvent(QKeyEvent *event)
{

}


void variationGraph::timerEvent(QTimerEvent *event){
}


void variationGraph::readAnalysisFile(){


}
