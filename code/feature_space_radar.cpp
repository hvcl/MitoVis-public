///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

#include "feature_space_radar.h"
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

#include "glwidget_for_inset.h"

FSRadar::FSRadar(Window *p,MitoDataset *m,int mouse_p,QVector<int> top_sim, QVector<int> top_dissim, QWidget *parent)
    : QWidget(parent)
{

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);


    c_window=p;
    curData=m;
    for(int i=0;i<4;i++){
        recommendation_insets[i]=NULL;
    }

    target_point=mouse_p;
    target_neurite_ind=curData->tracingMap[target_point].neurite_ind;

    shift=QVector2D(curData->ImageW*0.5 - target_point%curData->ImageW, curData->ImageH*0.5 - target_point/curData->ImageW);
    scale=0.3;
    scale_changed=true;

    for(int i=0;i<top_sim.size();i++){
        top_similar_neurites.push_back(top_sim[i]);
    }
    for(int i=0;i<top_dissim.size();i++){
        top_dissimilar_neurites.push_back(top_dissim[i]);
    }


    if(is_ver3_init==false){
        ver3_init();
        is_ver3_init=true;
    }


//    QWidget::setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);



}
FSRadar::~FSRadar(){
}

void FSRadar::resizeEvent(QResizeEvent *event){
}

QSize FSRadar::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize FSRadar::sizeHint() const
{
    return QSize(800, 800);
}

void FSRadar::paintEvent(QPaintEvent *)
{
//    ver1();
//    ver2();

    ver3();


}

void FSRadar::ver1(){

    float transparency_v=0.3;

    if(is_focused){
        transparency_v=1.0;
    }

    int target_type=curData->structureData[target_point];


    QPainter painter(this);
    //painter.setRenderHint(QPainter::HighQualityAntialiasing,true);
    //     painter.drawPixmap(0,0,this->width(),this->height(),background);//resolution

    QPen t_pen1;
    QPen t_pen2;

    ////Draw frame
    ///
    ///
    if(is_focused){
        //background
        {
            QBrush t_brush(Qt::SolidPattern);

            t_brush.setColor(QColor(0,0,0,100));
            painter.setBrush(t_brush);
            QRectF bound(0,0,this->width(),this->height());
            painter.drawEllipse(bound);
        }

        painter.setBrush(Qt::NoBrush);
        // Outer circle
        {
            if(target_type==2){
                t_pen1.setColor(QColor(255,79,79,255*transparency_v));
                t_pen2.setColor(QColor(59,85,255,255*transparency_v));
            }
            else{
                t_pen2.setColor(QColor(255,79,79,255*transparency_v));
                t_pen1.setColor(QColor(59,85,255,255*transparency_v));
            }
            t_pen1.setWidth(3);
            t_pen2.setWidth(3);

            painter.setPen(t_pen1);
            QRectF upper_bound(3,3,this->width()-6,this->height()-6);
            painter.drawArc(upper_bound,0*16,180*16);

            painter.setPen(t_pen2);
            QRectF under_bound(3,3,this->width()-6,this->height()-6);
            painter.drawArc(under_bound,180*16,180*16);
        }

        // inner circles
        {
            QPen t_pen;
            t_pen.setColor(QColor(240,242,255,150*transparency_v));
            t_pen.setWidth(2);
            painter.setPen(t_pen);

            for(int i=1;i<=3;i++){
                QRectF bound(this->width()*0.25*0.5*i,
                             this->height()*0.25*0.5*i,
                             this->width()-this->width()*0.25*i,
                             this->height()-this->height()*0.25*i);
                painter.drawArc(bound,0*16,360*16);
            }
        }

        //Middle line
        {
            QPen t_pen;
            t_pen.setColor(QColor(250,250,250,150*transparency_v));
            t_pen.setWidth(2);
            painter.setPen(t_pen);
            painter.drawLine(0,this->height()*0.5,this->width(),this->height()*0.5);
        }


        //recommendation points
        {
            QBrush t_brush(Qt::SolidPattern);

            for(int layer=0;layer<3;layer++){
                int max_v=0;
                int min_v=1;
                for(int i=0;i<3;i++){
                    if(i+layer*3>=top_similar_neurites.size())break;
                    int neu=top_similar_neurites[i+layer*3];
                    int uncertainty=1.0 - abs(0.5-curData->neurite_probability_dend[neu]) * 2;
                    if(max_v<uncertainty)max_v=uncertainty;
                    if(min_v>uncertainty)min_v=uncertainty;
                }

                for(int i=0;i<3;i++){
                    if(i+layer*3>=top_similar_neurites.size())break;
                    int neu=top_similar_neurites[i+layer*3];
                    int type=curData->neurite_res[neu];
                    int uncertainty=1.0 - abs(0.5-curData->neurite_probability_dend[neu]) * 2;

                    if(type==2){
                        t_brush.setColor(QColor(255,79,79,255*transparency_v));
                    }
                    else{
                        t_brush.setColor(QColor(59,85,255,255*transparency_v));
                    }

                    float angle=(1.0-uncertainty) * M_PI;
                    if(max_v==min_v)angle=0.25*M_PI + 0.25*M_PI*i;
                    QVector2D loc=QVector2D(this->width()*0.125*(layer+1)*cos(angle),
                                      -this->width()*0.125*(layer+1)*sin(angle)) + QVector2D(this->width(),this->height())*0.5;


                    float size=this->width()*0.03+uncertainty*this->width()*0.03;



                    QRectF bound(loc.x()-size*0.5,loc.y()-size*0.5,size,size);

                    painter.setPen(Qt::NoPen);
                    painter.setBrush(t_brush);

                    painter.drawEllipse(bound);
                }


                max_v=0;
                min_v=1;
                for(int i=0;i<3;i++){
                    if(i+layer*3>=top_dissimilar_neurites.size())break;
                    int neu=top_dissimilar_neurites[i+layer*3];
                    int uncertainty=1.0 - abs(0.5-curData->neurite_probability_dend[neu]) * 2;
                    if(max_v<uncertainty)max_v=uncertainty;
                    if(min_v>uncertainty)min_v=uncertainty;
                }

                for(int i=0;i<3;i++){
                    if(i+layer*3>=top_dissimilar_neurites.size())break;
                    int neu=top_dissimilar_neurites[i+layer*3];
                    int type=curData->neurite_res[neu];
                    int uncertainty=1.0 - abs(0.5-curData->neurite_probability_dend[neu]) * 2;

                    if(type==2){
                        t_brush.setColor(QColor(255,79,79,255*transparency_v));
                    }
                    else{
                        t_brush.setColor(QColor(59,85,255,255*transparency_v));
                    }

                    float angle=(1.0-uncertainty) * M_PI;
                    if(max_v==min_v)angle=0.25*M_PI + 0.25*M_PI*i;
                    QVector2D loc=QVector2D(this->width()*0.125*(layer+1)*cos(angle),
                                      this->width()*0.125*(layer+1)*sin(angle)) + QVector2D(this->width(),this->height())*0.5;


                    float size=this->width()*0.03+uncertainty*this->width()*0.03;



                    QRectF bound(loc.x()-size*0.5,loc.y()-size*0.5,size,size);

                    painter.setPen(Qt::NoPen);
                    painter.setBrush(t_brush);

                    painter.drawEllipse(bound);
                }
            }

        }
    }


    //Middle point
    {
        QPen t_pen;
        t_pen.setColor(QColor(255,225,0,255));
        t_pen.setWidth(2);
        painter.setPen(t_pen);
        painter.drawLine(this->width()*0.5-4,this->height()*0.5,this->width()*0.5+4,this->height()*0.5);
        painter.drawLine(this->width()*0.5-2,this->height()*0.5+sqrt(3)*2,this->width()*0.5+2,this->height()*0.5-sqrt(3)*2);
        painter.drawLine(this->width()*0.5-2,this->height()*0.5-sqrt(3)*2,this->width()*0.5+2,this->height()*0.5+sqrt(3)*2);

    }
}

void FSRadar::ver2(){
    int target_type=curData->structureData[target_point];


    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing,true);
    //     painter.drawPixmap(0,0,this->width(),this->height(),background);//resolution


    ////Draw frame
    ///
    ///
    if(is_focused){

        painter.setPen(Qt::NoPen);
        //background
        {
            QBrush t_brush(Qt::SolidPattern);

            t_brush.setColor(QColor(0,0,0,160));
            painter.setBrush(t_brush);
            QRectF bound(0,0,this->width(),this->height());
            painter.drawEllipse(bound);
        }
        painter.setBrush(Qt::NoBrush);


        // tracing map
        {
            drawTracingMap(&painter);
        }



        // inner circles
        {
            QPen t_pen;
            t_pen.setColor(QColor(240,242,255,150));
            t_pen.setWidth(2);
            painter.setPen(t_pen);

            for(int i=1;i<=3;i++){
                QRectF bound(this->width()*0.25*0.5*i,
                             this->height()*0.25*0.5*i,
                             this->width()-this->width()*0.25*i,
                             this->height()-this->height()*0.25*i);
                painter.drawArc(bound,0*16,360*16);
            }
        }

        //recommendation points
        {
        }

        // Outer circle
        {
            QPen t_pen;
            if(target_type==2){
                t_pen.setColor(QColor(255,79,79,255));
            }
            else{
                t_pen.setColor(QColor(59,85,255,255));
            }
            t_pen.setWidth(3);

            painter.setPen(t_pen);
            QRectF bound(3,3,this->width()-6,this->height()-6);
            painter.drawArc(bound,0*16,360*16);

        }

    }


    //Middle point
    {
        QPen t_pen;
        t_pen.setColor(QColor(255,225,0,255));
        int size=2;
        t_pen.setWidth(2);
        if(is_focus_fixed){
            size=4;
            t_pen.setWidth(4);
        }
        painter.setPen(t_pen);
        painter.drawLine(this->width()*0.5-size*2,this->height()*0.5,this->width()*0.5+size*2,this->height()*0.5);
        painter.drawLine(this->width()*0.5-size,this->height()*0.5+sqrt(3)*size,this->width()*0.5+size,this->height()*0.5-sqrt(3)*size);
        painter.drawLine(this->width()*0.5-size,this->height()*0.5-sqrt(3)*size,this->width()*0.5+size,this->height()*0.5+sqrt(3)*size);

    }

}
void FSRadar::drawTracingMap(QPainter *painter){


    QColor color_dend=QColor(255,79,79,180);
    QColor color_axon=QColor(59,85,255,180);



    if(scale_changed || curData->doTracing){
        mapSimplification();
        scale_changed=false;
    }

    float _scale=scale*curData->scale;

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            if(curData->tracingMap_simplified[tP].size()>0 && curData->tracingMap[tP].type!=-2){
                for(int i=0;i<curData->tracingMap_simplified[tP].size();i++){

                    int s_label=curData->structureData_simplified[tP];
                    int e_label=curData->structureData_simplified[curData->tracingMap_simplified[tP][i]];

                    QColor s_color=c_window->typeColors[s_label];
                    QColor e_color=c_window->typeColors[e_label];
                    if(s_label==2){
                        s_color=color_dend;
                    }
                    else{
                        s_color=color_axon;
                    }
                    if(e_label==2){
                        e_color=color_dend;
                    }
                    else{
                        e_color=color_axon;
                    }


                    float startx=ix + 0.5;
                    float starty=iy + 0.5;
                    float endx=curData->tracingMap_simplified[tP][i]%curData->ImageW + 0.5;
                    float endy=curData->tracingMap_simplified[tP][i]/curData->ImageW + 0.5;

                    float p_screen_x_start=startx*_scale+shift.x()*_scale+(this->width()-curData->ImageW*_scale)/2.0;
                    float p_screen_y_start=starty*_scale+shift.y()*_scale+(this->height()-curData->ImageH*_scale)/2.0;
                    float p_screen_x_end=endx*_scale+shift.x()*_scale+(this->width()-curData->ImageW*_scale)/2.0;
                    float p_screen_y_end=endy*_scale+shift.y()*_scale+(this->height()-curData->ImageH*_scale)/2.0;

                    QLinearGradient gradient;
                    gradient.setStart(p_screen_x_start,p_screen_y_start);
                    gradient.setFinalStop(p_screen_x_end,p_screen_y_end);
                    gradient.setColorAt(0.0,s_color);
                    gradient.setColorAt(1.0, e_color);

                    QPen tpen;
                    tpen.setBrush(gradient);
                    tpen.setWidth(3);
                    painter->setPen(tpen);

                    if((QVector2D(p_screen_x_start,p_screen_y_start)-QVector2D(this->width()*0.5,this->height()*0.5)).length()
                            <this->width()*0.5
                            &&
                        (QVector2D(p_screen_x_end,p_screen_y_end)-QVector2D(this->width()*0.5,this->height()*0.5)).length()
                                                    <this->width()*0.5){
                        painter->drawLine(p_screen_x_start,p_screen_y_start,p_screen_x_end,p_screen_y_end);
                    }

                }

            }
        }
    }


    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            if(curData->tracingMap[tP].type==-2){
                    float p_screen_x=(ix+0.5)*_scale+shift.x()*_scale+(this->width()-curData->ImageW*_scale)/2.0;
                    float p_screen_y=(iy+0.5)*_scale+shift.y()*_scale+(this->height()-curData->ImageH*_scale)/2.0;
                    QBrush tbrush;
                    tbrush.setColor(c_window->typeColors[1]);
                    tbrush.setStyle(Qt::SolidPattern);
                    painter->setBrush(tbrush);
                    painter->setPen(Qt::NoPen);

                    if((QVector2D(p_screen_x,p_screen_y)-QVector2D(this->width()*0.5,this->height()*0.5)).length()
                            <this->width()*0.5){

                        painter->drawEllipse(p_screen_x-50*_scale,p_screen_y-50*_scale,100*_scale,100*_scale);
                    }

            }
        }
    }
}


void FSRadar::ver3(){


    if(is_ver3_init==false){
        ver3_init();
        is_ver3_init=true;
    }


    int target_type=curData->structureData[target_point];


    QPainter painter(this);
    //painter.setRenderHint(QPainter::HighQualityAntialiasing,true);
    //     painter.drawPixmap(0,0,this->width(),this->height(),background);//resolution



    //Middle point
    {
        QPen t_pen;
        t_pen.setColor(QColor(255,225,0,255));
        int size=2;
        t_pen.setWidth(2);
        if(is_focus_fixed){
            size=4;
            t_pen.setWidth(4);
        }
        painter.setPen(t_pen);
        painter.drawLine(this->width()*0.5-size*2,this->height()*0.5,this->width()*0.5+size*2,this->height()*0.5);
        painter.drawLine(this->width()*0.5-size,this->height()*0.5+sqrt(3)*size,this->width()*0.5+size,this->height()*0.5-sqrt(3)*size);
        painter.drawLine(this->width()*0.5-size,this->height()*0.5-sqrt(3)*size,this->width()*0.5+size,this->height()*0.5+sqrt(3)*size);

    }

    if(is_focus_fixed){


        QPen leaderlinePen(QColor(255,255,0,200),1,Qt::SolidLine);

        QPoint parentPos=this->mapToGlobal(QPoint(this->width()*0.5,this->height()*0.5));

        int inset_size=150;
        int offset=30;

        QVector2D inset_location[4]={QVector2D(inset_size*0.5+offset,inset_size*0.5+offset),QVector2D(inset_size*0.5+offset,-(inset_size*0.5+offset)),
                                    QVector2D(-(inset_size*0.5+offset),-(inset_size*0.5+offset)),QVector2D(-(inset_size*0.5+offset),inset_size*0.5+offset)};
        QVector2D line_location[4]={QVector2D(1,1),QVector2D(1,-1),
                                    QVector2D(-1,-1),QVector2D(-1,1)};

        for(int i=0;i<4;i++){
            if(recommendation_insets[i]!=NULL){

                QVector2D cur_location=inset_location[i];
                recommendation_insets[i]->setGeometry(cur_location.x() + parentPos.x()-inset_size*0.5,cur_location.y()+parentPos.y()-inset_size*0.5
                                       ,inset_size,inset_size);
                recommendation_insets[i]->show();
                recommendation_insets[i]->update();

                float line_len=(sqrt((inset_size*0.5+offset)*(inset_size*0.5+offset) + (inset_size*0.5+offset)*(inset_size*0.5+offset))-inset_size*0.5)/sqrt(2);

                painter.setPen(leaderlinePen);
                painter.drawLine(this->width()*0.5+line_location[i].x()*8,this->height()*0.5+line_location[i].y()*8,
                                 this->width()*0.5+line_location[i].x()*line_len,this->height()*0.5+line_location[i].y()*line_len);
            }

        }
    }
    else{
//        curData->focus_fixed_point=-1;
        for(int i=0;i<4;i++){
            if(recommendation_insets[i]!=NULL){
                recommendation_insets[i]->hide();
                recommendation_insets[i]->update();
            }
        }

    }

    painter.end();

}

void FSRadar::ver3_init(){
    for(int i=0;i<4;i++){
        if(top_similar_neurites.size()==i)break;
        QVector2D minP=QVector2D(curData->ImageW-1,curData->ImageH-1);
        QVector2D maxP=QVector2D(0,0);
        for(int iy=0;iy<curData->ImageH;iy++){
            for(int ix=0;ix<curData->ImageW;ix++){
                if(curData->tracingMap[iy*curData->ImageW+ix].neurite_ind==top_similar_neurites[i]){
                    if(minP.x()>ix)minP.setX(ix);
                    if(minP.y()>iy)minP.setY(iy);
                    if(maxP.x()<ix)maxP.setX(ix);
                    if(maxP.y()<iy)maxP.setY(iy);
                }
            }
        }
        minP-=QVector2D(30,30);
        maxP+=QVector2D(30,30);
        if(minP.x()<0)minP.setX(0);
        if(minP.y()<0)minP.setY(0);
        if(maxP.x()>=curData->ImageW)maxP.setX(curData->ImageW-1);
        if(maxP.y()>=curData->ImageH)maxP.setY(curData->ImageH-1);

        while(int(maxP.y()-minP.y())!=int(maxP.x()-minP.x())){
            if(int(maxP.y()-minP.y())<int(maxP.x()-minP.x())){
                if(maxP.y()<curData->ImageH-1)maxP.setY(maxP.y()+1);
                else minP.setY(minP.y()-1);
            }
            else{
                if(maxP.x()<curData->ImageW-1)maxP.setX(maxP.x()+1);
                else minP.setX(minP.x()-1);
            }
        }

        while(int(maxP.x()-minP.x())%4!=0 || int(maxP.x()-minP.x())<4){
            if(maxP.x()<curData->ImageW-1)maxP.setX(maxP.x()+1);
            else minP.setX(minP.x()-1);
        }
        while(int(maxP.y()-minP.y())%4!=0 || int(maxP.y()-minP.y())<4){
            if(maxP.y()<curData->ImageH-1)maxP.setY(maxP.y()+1);
            else minP.setY(minP.y()-1);
        }


        recommendation_insets[i]=new GlWidgetForInset(c_window,curData,top_similar_neurites[i],0,minP,maxP,c_window->dataRendering);
    }
}
void FSRadar::mapSimplification(){

    qDebug()<<"simplification start";

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            curData->tracingMap_simplified[tP].clear();
            for(int k=0;k<curData->tracingMap[tP].connected_nodes.size();k++){
                curData->tracingMap_simplified[tP].push_back(curData->tracingMap[tP].connected_nodes[k]);
            }
            curData->tracingMap_simplified_collections[tP].clear();
            curData->tracingMap_simplified_collections[tP].push_back(tP);

            curData->structureData_simplified[tP]=curData->structureData[tP];
        }
    }

//    float dis=1.5;

//    for(int i=0;i<10;i++){
//        bool is_changed=mapSimplification_sub(dis);
//        dis+=1.5;
//    }

//    while(1){
//        qDebug()<<"simplification sub";
//        bool is_changed=mapSimplification_sub(dis);
//        if(is_changed==false)break;
//    }



}
bool FSRadar::mapSimplification_sub(float distance_thresh){


    bool is_changed=false;
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int v0=iy*curData->ImageW+ix;
            bool is_mixed_v0=false;
            for(int k=0;k<curData->tracingMap_simplified[v0].size();k++){
                int v1=curData->tracingMap_simplified[v0][k];
                int s_label=curData->structureData_simplified[v0];
                int e_label=curData->structureData_simplified[v1];
                if(s_label!=e_label){
                    is_mixed_v0=true;
                    break;
                }

            }

            for(int k=0;k<curData->tracingMap_simplified[v0].size();k++){
                int v1=curData->tracingMap_simplified[v0][k];
                bool is_mixed_v1=false;
                for(int m=0;m<curData->tracingMap_simplified[v1].size();m++){
                    int v2=curData->tracingMap_simplified[v1][m];
                    int s_label=curData->structureData_simplified[v1];
                    int e_label=curData->structureData_simplified[v2];
                    if(s_label!=e_label){
                        is_mixed_v1=true;
                        break;
                    }
                }


                if(curData->structureData_simplified[v0]!=curData->structureData_simplified[v1]){
                    continue;
                }
                QVector2D v0_screen=
                        QVector2D((v0%curData->ImageW)*scale+shift.x()*scale+(this->width()-curData->ImageW*scale)/2.0,
                                    (v0/curData->ImageW)*scale+shift.y()*scale+(this->height()-curData->ImageH*scale)/2.0);
                QVector2D v1_screen=
                        QVector2D((v1%curData->ImageW)*scale+shift.x()*scale+(this->width()-curData->ImageW*scale)/2.0,
                                    (v1/curData->ImageW)*scale+shift.y()*scale+(this->height()-curData->ImageH*scale)/2.0);

                if(v0_screen.distanceToPoint(v1_screen)<distance_thresh){
                    if(is_mixed_v0==false && is_mixed_v1==false){
                        node_combine(v0,v1);
                        int new_x=(v0%curData->ImageW) + (v1%curData->ImageW);
                        new_x*=0.5;
                        int new_y=(v0/curData->ImageW) + (v1/curData->ImageW);
                        new_y*=0.5;
                        node_move(new_y*curData->ImageW+new_x,v0);
                        is_changed=true;
                    }
                    else if(is_mixed_v1==false){
                        node_combine(v0,v1);
                        is_changed=true;
                    }
                    else if(is_mixed_v0==false){
                        node_combine(v1,v0);
                        is_changed=true;
                    }
                }
            }
        }
    }
    return is_changed;

}
void FSRadar::node_combine(int v0, int v1){
    if(v0==v1)return;


    for(int i=curData->tracingMap_simplified[v0].size()-1;i>=0;i--){
        if(curData->tracingMap_simplified[v0][i]==v1){
            curData->tracingMap_simplified[v0].remove(i);
        }
    }
    for(int i=curData->tracingMap_simplified[v1].size()-1;i>=0;i--){
        if(curData->tracingMap_simplified[v1][i]==v0){
            curData->tracingMap_simplified[v1].remove(i);
        }
    }

    for(int i=0;i<curData->tracingMap_simplified[v1].size();i++){
        int v2=curData->tracingMap_simplified[v1][i];
        for(int j=curData->tracingMap_simplified[v2].size()-1;j>=0;j--){
            if(curData->tracingMap_simplified[v2][j]==v1){
                curData->tracingMap_simplified[v2].remove(j);
            }
        }
        curData->tracingMap_simplified[v2].push_back(v0);
        curData->tracingMap_simplified[v0].push_back(v2);
    }

    curData->tracingMap_simplified[v1].clear();
    curData->tracingMap_simplified_collections[v0].push_back(v1);
    curData->tracingMap_simplified_collections[v1].clear();


}

void FSRadar::node_move(int v0, int v1){
    if(v0==v1)return;

    curData->structureData_simplified[v0]=curData->structureData_simplified[v1];

    for(int i=curData->tracingMap_simplified[v0].size()-1;i>=0;i--){
        if(curData->tracingMap_simplified[v0][i]==v1){
            curData->tracingMap_simplified[v0].remove(i);
        }
    }
    for(int i=curData->tracingMap_simplified[v1].size()-1;i>=0;i--){
        if(curData->tracingMap_simplified[v1][i]==v0){
            curData->tracingMap_simplified[v1].remove(i);
        }
    }

    for(int i=0;i<curData->tracingMap_simplified[v1].size();i++){
        int v2=curData->tracingMap_simplified[v1][i];
        for(int j=curData->tracingMap_simplified[v2].size()-1;j>=0;j--){
            if(curData->tracingMap_simplified[v2][j]==v1){
                curData->tracingMap_simplified[v2].remove(j);
            }
        }
        curData->tracingMap_simplified[v2].push_back(v0);
        curData->tracingMap_simplified[v0].push_back(v2);
    }

    curData->tracingMap_simplified[v1].clear();

    for(int i=0;i<curData->tracingMap_simplified_collections[v1].size();i++){
        curData->tracingMap_simplified_collections[v0].push_back(curData->tracingMap_simplified_collections[v1][i]);
    }
}



void FSRadar::info_update(int p,QVector<int> top_sim, QVector<int> top_dissim){
    target_point=p;

    top_similar_neurites.clear();
    for(int i=0;i<top_sim.size();i++){
        top_similar_neurites.push_back(top_sim[i]);
    }

    top_dissimilar_neurites.clear();
    for(int i=0;i<top_dissim.size();i++){
        top_dissimilar_neurites.push_back(top_dissim[i]);
    }

    for(int i=0;i<4;i++){
        if(recommendation_insets[i]!=NULL){
            delete recommendation_insets[i];
            recommendation_insets[i]=NULL;
        }
    }
    ver3_init();

}

void FSRadar::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numDegrees.isNull()) {
        QPoint numSteps = numDegrees / 15;

        scale+=(numSteps.x()+numSteps.y())*0.01;
        if(scale<0.02)scale=0.02;
        if(scale>1)scale=1;
        scale_changed=true;
        update();
    }
    event->accept();
}



void FSRadar::mousePressEvent(QMouseEvent *event)
{
    if(c_window->curDataIndex==-1)return;

    if(event->x()<0 || event->x()>this->width() || event->y()<0 || event->y()>this->height())return;

    lastPos = event->pos();
    button=event->button();

    float tx=event->x();
    float ty=event->y();


    if(event->button() == Qt::RightButton){
    }

    if(event->button() ==Qt::LeftButton){
        if(is_focused && sqrt((this->width()*0.5-tx)*(this->width()*0.5-tx)+(this->height()*0.5-ty)*(this->height()*0.5-ty))<5){
            is_focus_fixed=!is_focus_fixed;
            if(is_focus_fixed){
                curData->focus_fixed_point=target_point;
            }
        }
    }
    update();
}
void FSRadar::mouseReleaseEvent(QMouseEvent *event){

    if(c_window->curDataIndex==-1)return;


    if(event->button() == Qt::RightButton){
    }



}
void FSRadar::mouseMoveEvent(QMouseEvent *event)
{
    if(c_window->curDataIndex==-1)return;

//    setFocus();

    if(event->x()<0 || event->x()>this->width() || event->y()<0 || event->y()>this->height())return;

    float tx=event->x();
    float ty=event->y();

    if(sqrt((this->width()*0.5-tx)*(this->width()*0.5-tx)+(this->height()*0.5-ty)*(this->height()*0.5-ty))<5){
        is_focused=true;
        update();
    }


}

void FSRadar::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Escape){
    }
    if(event->key()==Qt::Key_Shift){
        c_window->IsShift=true;
    }
}
void FSRadar::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Shift){
        c_window->IsShift=false;
    }
}

void FSRadar::timerEvent(QTimerEvent *event){
}

