///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

#include "parallelCoordinate.h"
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

ParallelCoordinate::ParallelCoordinate(Window *p, QWidget *parent)
    : QWidget(parent)
{

    c_window=p;

    selectionFrame=-1;
    doSelect=false;

    axisStarts=new QVector2D[30];
    axisEnds=new QVector2D[30];

    setBackgroundRole(QPalette::Base);
    QWidget::setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);

    for(int i=0;i<10;i++){

        QLineEdit *t_start=new QLineEdit("0.0",this);
        t_start->setStyleSheet(QString("QLineEdit {font: Bold;color: rgb(50,168,82);background-color: rgba(255,255,255,100)}"));
        t_start->setAutoFillBackground(false);
        t_start->setFrame(0);
        t_start->setObjectName(QString::number(i));
        t_start->hide();
        connect(t_start,SIGNAL(textEdited(QString)),this,SLOT(handleEditStart(QString)));
        connect(t_start,SIGNAL(editingFinished()),this,SLOT(handleEditFinish1()));


        QLineEdit *t_end=new QLineEdit("1.0",this);

        t_end->setStyleSheet(QString("QLineEdit {font: Bold;color: rgb(50,168,82);background-color: rgba(255,255,255,100)}"));
        t_end->setAutoFillBackground(false);
        t_end->setFrame(0);
        t_end->setObjectName(QString::number(i));
        t_end->hide();
        connect(t_end,SIGNAL(textEdited(QString)),this,SLOT(handleEditStart(QString)));
        connect(t_end,SIGNAL(editingFinished()),this,SLOT(handleEditFinish2()));

        thresh_bar_start.push_back(t_start);
        thresh_bar_end.push_back(t_end);

        c_window->globalAxisRange.push_back(QVector2D(0,1));
        c_window->globalSelectionRange.push_back(QVector2D(0,1));
        c_window->globalSelection.push_back(false);


    }

    //setValidator( new QDoubleValidator(0, 100, 2, this) );


}
void ParallelCoordinate::handleEditStart(QString b){
    qDebug()<<"edit start";
    setMouseTracking(false);
    editMode=true;
}
void ParallelCoordinate::handleEditFinish1(){
    qDebug()<<"edit finish";
    setMouseTracking(true);
    QObject *senderObj = sender(); // This will give Sender object
    int ind=senderObj->objectName().toInt();
    editMode=false;
    bool ck=false;
    float v=thresh_bar_start[ind]->text().toFloat(&ck);
    if(ck==true){
        v=(v-c_window->featureRanges[ind].x())/(c_window->featureRanges[ind].y()-c_window->featureRanges[ind].x());
        if(c_window->analysis_type->currentIndex()==0){
            c_window->dataset[c_window->curDataIndex]->axisRange[ind].setX(v);
            c_window->dataset[c_window->curDataIndex]->checkDataEnable(c_window);
        }
        else{
            c_window->globalAxisRange[ind].setX(v);
            for(int g=0;g<c_window->dataset.length();g++){
                   c_window->dataset[g]->checkDataEnable(c_window);
            }
        }
        is_axis_edit=true;
        emit synchronization();
        update();

    }
}
void ParallelCoordinate::handleEditFinish2(){
    qDebug()<<"edit finish";
    setMouseTracking(true);
    QObject *senderObj = sender(); // This will give Sender object
    int ind=senderObj->objectName().toInt();
    editMode=false;
    bool ck=false;
    float v=thresh_bar_end[ind]->text().toFloat(&ck);
    if(ck==true){
        v=(v-c_window->featureRanges[ind].x())/(c_window->featureRanges[ind].y()-c_window->featureRanges[ind].x());
        if(c_window->analysis_type->currentIndex()==0){

            c_window->dataset[c_window->curDataIndex]->axisRange[ind].setY(v);
            c_window->dataset[c_window->curDataIndex]->checkDataEnable(c_window);
        }
        else{
            c_window->globalAxisRange[ind].setY(v);
            for(int g=0;g<c_window->dataset.length();g++){
                   c_window->dataset[g]->checkDataEnable(c_window);
            }

        }
        is_axis_edit=true;
        update();
        emit synchronization();
    }
}
ParallelCoordinate::~ParallelCoordinate(){
}

void ParallelCoordinate::resizeEvent(QResizeEvent *event){
    is_axis_edit=true;
    //    int w=event->size().width();
    //    int h=event->size().height();
    //    this->resize(w,h);
}

QSize ParallelCoordinate::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize ParallelCoordinate::sizeHint() const
{
    return QSize(800, 800);
}

void ParallelCoordinate::paintEvent(QPaintEvent *)
{


    qDebug()<<"start parallelcoord";


    if(c_window->curDataIndex==-1){
        for(int i=0;i<c_window->featureNum;i++){
            axisStarts[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,this->height()*0.9);
            axisEnds[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,this->height()*0.3);
        }
        QPainter painter(this);
        painter.setRenderHint(QPainter::HighQualityAntialiasing,true);
        //     painter.drawPixmap(0,0,this->width(),this->height(),background);//resolution

        QPen t_pen;
        QBrush t_brush(Qt::SolidPattern);

        ////Draw frame

        for(int i=0;i<c_window->featureNum;i++){

            QLinearGradient gradient;
            gradient.setStart(axisStarts[i].x(),axisStarts[i].y());
            gradient.setFinalStop(axisEnds[i].x(),axisEnds[i].y());
            gradient.setColorAt(0.0,QColor(200,200,200));
            gradient.setColorAt(1.0, QColor(100,100,100));
            t_pen=QPen(gradient,2);
            t_pen.setStyle(Qt::DotLine);
            painter.setPen(t_pen);
            painter.drawLine(axisStarts[i].x(),axisStarts[i].y()
                             ,axisEnds[i].x(),axisEnds[i].y());
        }
        return;
    }

    float lineW=2*c_window->WINDOW_SCALE;
    float pointW=2*c_window->WINDOW_SCALE;


    if(c_window->analysis_type->currentIndex()==0){
        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

        ////Get coordinate
        ///

        if(c_window->pcp_expand->isChecked()){
            for(int i=0;i<c_window->featureNum;i++){
                axisStarts[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,
                                        this->height()*0.9+curData->axisRange[i].x()/(curData->axisRange[i].y()-curData->axisRange[i].x())*this->height()*0.6);
                axisEnds[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,
                                      this->height()*0.3-((1-curData->axisRange[i].y())/(curData->axisRange[i].y()-curData->axisRange[i].x())*this->height()*0.6));
            }
        }
        else{
            for(int i=0;i<c_window->featureNum;i++){
                axisStarts[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,this->height()*0.9);
                axisEnds[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,this->height()*0.3);
            }
        }

        QPainter painter(this);
        painter.setRenderHint(QPainter::HighQualityAntialiasing,true);
        //     painter.drawPixmap(0,0,this->width(),this->height(),background);//resolution

        QPen t_pen;
        QBrush t_brush(Qt::SolidPattern);

        ////Draw frame

        for(int i=0;i<c_window->featureNum;i++){

            QLinearGradient gradient;
            gradient.setStart(axisStarts[i].x(),axisStarts[i].y());
            gradient.setFinalStop(axisEnds[i].x(),axisEnds[i].y());
            gradient.setColorAt(0.0,QColor(200,200,200));
            gradient.setColorAt(1.0, QColor(100,100,100));
            t_pen=QPen(gradient,2);
            t_pen.setStyle(Qt::DotLine);
            painter.setPen(t_pen);
            painter.drawLine(axisStarts[i].x(),axisStarts[i].y()
                             ,axisEnds[i].x(),axisEnds[i].y());
        }
    //    thresh_bar->setGeometry(axisStarts[0].x(),axisStarts[0].y(),30,30);

        QFont tFont;

        tFont.setBold(true);
        painter.setFont(tFont);

        //qDebug()<<"parallelcoord 0";






        t_pen.setColor(QColor(0,0,0));
        t_pen.setCapStyle(Qt::RoundCap);
        painter.setPen(t_pen);
        painter.setBrush(Qt::NoBrush);


        ////Draw each data graph

        if(curData->focusItem==-1){

            t_pen.setStyle(Qt::SolidLine);
            for(int i=0;i<curData->features.length();i++){
                for(int j=0;j<c_window->featureNum-1;j++){
                    //qDebug()<<curData->features[i][j]<<c_window->featureRanges[j];
                    if(!curData->enabled[i]){

                        float value1, value2;
                        QPointF p1,p2;
                        value1=(curData->features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
                        p1=QPointF(axisEnds[j].x()*value1+axisStarts[j].x()*(1-value1),
                                   axisEnds[j].y()*value1+axisStarts[j].y()*(1-value1));
                        int k=j+1;
                        value2=(curData->features[i][k]-c_window->featureRanges[k].x())/(c_window->featureRanges[k].y()-c_window->featureRanges[k].x());
                        p2=QPointF(axisEnds[k].x()*value2+axisStarts[k].x()*(1-value2),
                                   axisEnds[k].y()*value2+axisStarts[k].y()*(1-value2));

                        //qDebug()<<p1<<p2<<value1<<value2;

                        t_pen.setWidth(lineW);
                        t_pen.setColor(QColor(50,50,50,15));

                        painter.setPen(t_pen);
                        painter.drawEllipse(p1,pointW,pointW);
                        painter.drawLine(p1,p2);
                        if(k==c_window->featureNum-1){
                            painter.drawEllipse(p2,pointW,pointW);
                            break;
                        }
                    }
                }
            }

            for(int i=0;i<curData->features.length();i++){
                for(int j=0;j<c_window->featureNum-1;j++){
                    //qDebug()<<curData->features[i][j]<<c_window->featureRanges[j];
                    if(curData->enabled[i]){

                        float value1, value2;
                        QPointF p1,p2;
                        value1=(curData->features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
                        p1=QPointF(axisEnds[j].x()*value1+axisStarts[j].x()*(1-value1),
                                   axisEnds[j].y()*value1+axisStarts[j].y()*(1-value1));
                        int k=j+1;
                        value2=(curData->features[i][k]-c_window->featureRanges[k].x())/(c_window->featureRanges[k].y()-c_window->featureRanges[k].x());
                        p2=QPointF(axisEnds[k].x()*value2+axisStarts[k].x()*(1-value2),
                                   axisEnds[k].y()*value2+axisStarts[k].y()*(1-value2));

                        //qDebug()<<p1<<p2<<value1<<value2;

                        t_pen.setWidth(lineW);
                        if(c_window->coloringTypeSet->currentIndex()==0)
                            t_pen.setColor(c_window->typeColors[int(curData->features[i][0])]);
                        else
                            t_pen.setColor(c_window->groups[curData->group]->color);


                        painter.setPen(t_pen);
                        painter.drawEllipse(p1,pointW,pointW);
                        painter.drawLine(p1,p2);
                        if(k==c_window->featureNum-1){
                            painter.drawEllipse(p2,pointW,pointW);
                            break;
                        }
                    }
                }
            }
        }
        else{
            t_pen.setStyle(Qt::SolidLine);
            for(int i=0;i<curData->features.length();i++){
                for(int j=0;j<c_window->featureNum-1;j++){
                    float value1, value2;
                    QPointF p1,p2;
                    value1=(curData->features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
                    p1=QPointF(axisEnds[j].x()*value1+axisStarts[j].x()*(1-value1),
                               axisEnds[j].y()*value1+axisStarts[j].y()*(1-value1));
                    int k=j+1;
                    value2=(curData->features[i][k]-c_window->featureRanges[k].x())/(c_window->featureRanges[k].y()-c_window->featureRanges[k].x());
                    p2=QPointF(axisEnds[k].x()*value2+axisStarts[k].x()*(1-value2),
                               axisEnds[k].y()*value2+axisStarts[k].y()*(1-value2));


                    t_pen.setWidth(lineW);
                    t_pen.setColor(QColor(50,50,50,15));

                    if(i==curData->focusItem){
                        if(c_window->coloringTypeSet->currentIndex()==0)
                            t_pen.setColor(c_window->typeColors[int(curData->features[i][0])]);
                        else
                            t_pen.setColor(c_window->groups[curData->group]->color);
                    }

                    painter.setPen(t_pen);
                    painter.drawEllipse(p1,pointW,pointW);
                    painter.drawLine(p1,p2);
                    if(k==c_window->featureNum-1){
                        painter.drawEllipse(p2,pointW,pointW);
                        break;
                    }
                }
            }
            int i=curData->focusItem;
            for(int j=0;j<c_window->featureNum-1;j++){
                float value1, value2;
                QPointF p1,p2;
                value1=(curData->features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
                p1=QPointF(axisEnds[j].x()*value1+axisStarts[j].x()*(1-value1),
                           axisEnds[j].y()*value1+axisStarts[j].y()*(1-value1));
                int k=j+1;
                value2=(curData->features[i][k]-c_window->featureRanges[k].x())/(c_window->featureRanges[k].y()-c_window->featureRanges[k].x());
                p2=QPointF(axisEnds[k].x()*value2+axisStarts[k].x()*(1-value2),
                           axisEnds[k].y()*value2+axisStarts[k].y()*(1-value2));


                t_pen.setWidth(lineW);
                t_pen.setColor(QColor(50,50,50,15));

                if(i==curData->focusItem){
                    if(c_window->coloringTypeSet->currentIndex()==0)
                        t_pen.setColor(c_window->typeColors[int(curData->features[i][0])]);
                    else
                        t_pen.setColor(c_window->groups[curData->group]->color);
                }

                painter.setPen(t_pen);
                painter.drawEllipse(p1,pointW,pointW);
                painter.drawLine(p1,p2);
                if(k==c_window->featureNum-1){
                    painter.drawEllipse(p2,pointW,pointW);
                    break;
                }
            }
        }

        //qDebug()<<"parallelcoord 1";


            ////Draw Selection frame & selection information

        if(selectionFrame!=-1){
            t_pen.setColor(QColor(0,255,0,40));
            t_pen.setWidth(50*c_window->WINDOW_SCALE);
            t_pen.setCapStyle(Qt::RoundCap);
            painter.setPen(t_pen);
            painter.drawLine(axisStarts[selectionFrame].x(),axisStarts[selectionFrame].y()
                             ,axisEnds[selectionFrame].x(),axisEnds[selectionFrame].y());
        }

        //qDebug()<<"parallelcoord 2";


        for(int i=0;i<c_window->featureNum;i++){

            if(curData->selection[i]){
                t_pen.setColor(QColor(35,255,159,200));
                t_pen.setWidth(20*c_window->WINDOW_SCALE);
                t_pen.setCapStyle(Qt::RoundCap);
                painter.setPen(t_pen);
                QPointF p1(axisStarts[i].x(),axisStarts[i].y());
                QPointF p2(axisEnds[i].x(),axisEnds[i].y());
                painter.drawLine(p1*(1-curData->selectionRange[i].x())+p2*curData->selectionRange[i].x(),p1*(1-curData->selectionRange[i].y())+p2*curData->selectionRange[i].y());

            }
        }


        // draw axis range bar

        for(int i=0;i<c_window->featureNum;i++){

            t_brush.setColor(QColor(50,168,82));
            painter.setBrush(t_brush);

    //        t_pen.setColor(QColor(50,168,82));
    //        t_pen.setWidth(10*c_window->WINDOW_SCALE);
    //        t_pen.setCapStyle(Qt::FlatCap);
            painter.setPen(Qt::NoPen);

            QPointF button[3];

            QPointF p1(axisStarts[i].x(),axisEnds[i].y()+(axisStarts[i].y()-axisEnds[i].y())*(1-curData->axisRange[i].x()));

            button[0]=p1;
            button[1]=p1+QPointF(-10,10);
            button[2]=p1+QPointF(10,10);
            painter.drawConvexPolygon(button,3);
            if(is_axis_edit){
                thresh_bar_start[i]->setGeometry(p1.x()-20,p1.y()+10,40,20);
                if(editMode==false)
                    thresh_bar_start[i]->setText(QString::number(c_window->featureRanges[i].x()+(c_window->featureRanges[i].y()-c_window->featureRanges[i].x())*curData->axisRange[i].x(),'f',2));
                thresh_bar_start[i]->show();
                if(i==0){
                    if(curData->axisRange[i].x()==0){
                        thresh_bar_start[i]->setGeometry(p1.x()-30,p1.y()+10,60,20);
                        thresh_bar_start[i]->setText("Dendrite");
                    }
                    else thresh_bar_start[i]->setText("");
                }
            }

            QPointF p2(axisEnds[i].x(),axisEnds[i].y()+(axisStarts[i].y()-axisEnds[i].y())*(1-curData->axisRange[i].y()));
            button[0]=p2;
            button[1]=p2+QPointF(-10,-10);
            button[2]=p2+QPointF(10,-10);
            painter.drawConvexPolygon(button,3);

            if(is_axis_edit){
                thresh_bar_end[i]->setGeometry(p2.x()-20,p2.y()-30,40,20);
                if(editMode==false)
                    thresh_bar_end[i]->setText(QString::number(c_window->featureRanges[i].x()+(c_window->featureRanges[i].y()-c_window->featureRanges[i].x())*curData->axisRange[i].y(),'f',2));
                thresh_bar_end[i]->show();

                if(i==0){
                    if(curData->axisRange[i].y()==1){
                        thresh_bar_end[i]->setText("Axon");
                    }
                    else thresh_bar_end[i]->setText("");
                }
            }


        }



        for(int i=0;i<c_window->featureNum;i++){
            axisStarts[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,this->height()*0.9);
            axisEnds[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,this->height()*0.3);
        }


        ////Draw Label
        t_pen.setColor(QColor(0,0,0));
        painter.setPen(t_pen);
        tFont.setBold(true);
        tFont.setPixelSize(12);
        painter.setFont(tFont);
        for(int i=0;i<c_window->featureNum;i++){
            if(i==1){
                painter.drawText(axisEnds[i].x()-50,axisEnds[i].y()-45,100,20,Qt::AlignCenter,c_window->featureNames[i]+"("+QChar(181)+"m"+0xB2+")");
            }
            else if(i==2){
                painter.drawText(axisEnds[i].x()-50,axisEnds[i].y()-45,100,20,Qt::AlignCenter,c_window->featureNames[i]+"("+QChar(181)+QString("m)"));
            }
            else painter.drawText(axisEnds[i].x()-50,axisEnds[i].y()-45,100,20,Qt::AlignCenter,c_window->featureNames[i]);

        }

        //// draw values
        t_pen.setColor(QColor(0,0,0));
        t_pen.setCapStyle(Qt::RoundCap);
        painter.setPen(t_pen);
        painter.setBrush(Qt::NoBrush);


        if(curData->focusItem==-1){
    //        painter.drawText(0,axisEnds[0].y()-this->height()*0.1,tr("min"));
            painter.drawText(0,axisEnds[0].y()-this->height()*0.15,tr("avg"));
    //        painter.drawText(0,axisEnds[0].y()-this->height()*0.2,tr("max"));
            tFont.setBold(false);
            painter.setFont(tFont);

            for(int i=0;i<c_window->featureNum;i++){
                char tt[10];
    //            sprintf(tt,"%0.2f",c_window->minValues[i]);
    //            painter.drawText(axisEnds[i].x()-10,axisEnds[i].y()-this->height()*0.1,tr(tt));
                sprintf(tt,"%0.2f",c_window->avgValues[i]);
                painter.drawText(axisEnds[i].x()-10,axisEnds[i].y()-this->height()*0.15,tr(tt));
    //            sprintf(tt,"%0.2f",c_window->maxValues[i]);
    //            painter.drawText(axisEnds[i].x()-10,axisEnds[i].y()-this->height()*0.2,tr(tt));
            }
        }
        else{
            painter.drawText(0,axisEnds[0].y()-this->height()*0.15,tr("val"));
            tFont.setBold(false);
            painter.setFont(tFont);

            for(int i=0;i<c_window->featureNum;i++){
                char tt[10];
                sprintf(tt,"%0.2f",curData->features[curData->focusItem][i]);
                painter.drawText(axisEnds[i].x()-10,axisEnds[i].y()-this->height()*0.15,tr(tt));

            }
        }

        if(c_window->pcp_expand->isChecked()){
            for(int i=0;i<c_window->featureNum;i++){
                axisStarts[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,
                                        this->height()*0.9+curData->axisRange[i].x()/(curData->axisRange[i].y()-curData->axisRange[i].x())*this->height()*0.6);
                axisEnds[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,
                                      this->height()*0.3-((1-curData->axisRange[i].y())/(curData->axisRange[i].y()-curData->axisRange[i].x())*this->height()*0.6));
            }
        }
    }
    else{

        ////Get coordinate
        ///

        if(c_window->pcp_expand->isChecked()){
            for(int i=0;i<c_window->featureNum;i++){
                axisStarts[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,
                                        this->height()*0.9+c_window->globalAxisRange[i].x()/(c_window->globalAxisRange[i].y()-c_window->globalAxisRange[i].x())*this->height()*0.6);
                axisEnds[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,
                                      this->height()*0.3-((1-c_window->globalAxisRange[i].y())/(c_window->globalAxisRange[i].y()-c_window->globalAxisRange[i].x())*this->height()*0.6));
            }
        }
        else{
            for(int i=0;i<c_window->featureNum;i++){
                axisStarts[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,this->height()*0.9);
                axisEnds[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,this->height()*0.3);
            }
        }

        QPainter painter(this);
        painter.setRenderHint(QPainter::HighQualityAntialiasing,true);
        //     painter.drawPixmap(0,0,this->width(),this->height(),background);//resolution

        QPen t_pen;
        QBrush t_brush(Qt::SolidPattern);

        ////Draw frame

        for(int i=0;i<c_window->featureNum;i++){

            QLinearGradient gradient;
            gradient.setStart(axisStarts[i].x(),axisStarts[i].y());
            gradient.setFinalStop(axisEnds[i].x(),axisEnds[i].y());
            gradient.setColorAt(0.0,QColor(200,200,200));
            gradient.setColorAt(1.0, QColor(100,100,100));
            t_pen=QPen(gradient,2);
            t_pen.setStyle(Qt::DotLine);
            painter.setPen(t_pen);
            painter.drawLine(axisStarts[i].x(),axisStarts[i].y()
                             ,axisEnds[i].x(),axisEnds[i].y());
        }
    //    thresh_bar->setGeometry(axisStarts[0].x(),axisStarts[0].y(),30,30);

        QFont tFont;

        tFont.setBold(true);
        painter.setFont(tFont);

        //qDebug()<<"parallelcoord 0";






        t_pen.setColor(QColor(0,0,0));
        t_pen.setCapStyle(Qt::RoundCap);
        painter.setPen(t_pen);
        painter.setBrush(Qt::NoBrush);


        ////Draw each data graph

        for(int g=0;g<c_window->dataset.length();g++){
            MitoDataset *curData=c_window->dataset[g];
            if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;


            if(curData->focusItem==-1){

                t_pen.setStyle(Qt::SolidLine);
                for(int i=0;i<curData->features.length();i++){
                    for(int j=0;j<c_window->featureNum-1;j++){
                        //qDebug()<<curData->features[i][j]<<c_window->featureRanges[j];
                        if(!curData->globalEnabled[i]){

                            float value1, value2;
                            QPointF p1,p2;
                            value1=(curData->features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
                            p1=QPointF(axisEnds[j].x()*value1+axisStarts[j].x()*(1-value1),
                                       axisEnds[j].y()*value1+axisStarts[j].y()*(1-value1));
                            int k=j+1;
                            value2=(curData->features[i][k]-c_window->featureRanges[k].x())/(c_window->featureRanges[k].y()-c_window->featureRanges[k].x());
                            p2=QPointF(axisEnds[k].x()*value2+axisStarts[k].x()*(1-value2),
                                       axisEnds[k].y()*value2+axisStarts[k].y()*(1-value2));

                            //qDebug()<<p1<<p2<<value1<<value2;

                            t_pen.setWidth(lineW);
                            t_pen.setColor(QColor(50,50,50,15));

                            painter.setPen(t_pen);
                            painter.drawEllipse(p1,pointW,pointW);
                            painter.drawLine(p1,p2);
                            if(k==c_window->featureNum-1){
                                painter.drawEllipse(p2,pointW,pointW);
                                break;
                            }
                        }
                    }
                }
            }
            else{
                t_pen.setStyle(Qt::SolidLine);
                for(int i=0;i<curData->features.length();i++){
                    for(int j=0;j<c_window->featureNum-1;j++){
                        float value1, value2;
                        QPointF p1,p2;
                        value1=(curData->features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
                        p1=QPointF(axisEnds[j].x()*value1+axisStarts[j].x()*(1-value1),
                                   axisEnds[j].y()*value1+axisStarts[j].y()*(1-value1));
                        int k=j+1;
                        value2=(curData->features[i][k]-c_window->featureRanges[k].x())/(c_window->featureRanges[k].y()-c_window->featureRanges[k].x());
                        p2=QPointF(axisEnds[k].x()*value2+axisStarts[k].x()*(1-value2),
                                   axisEnds[k].y()*value2+axisStarts[k].y()*(1-value2));


                        t_pen.setWidth(lineW);
                        t_pen.setColor(QColor(50,50,50,15));

                        if(i==curData->focusItem){
                            if(c_window->coloringTypeSet->currentIndex()==0)
                                t_pen.setColor(c_window->typeColors[int(curData->features[i][0])]);
                            else
                                t_pen.setColor(c_window->groups[curData->group]->color);
                        }

                        painter.setPen(t_pen);
                        painter.drawEllipse(p1,pointW,pointW);
                        painter.drawLine(p1,p2);
                        if(k==c_window->featureNum-1){
                            painter.drawEllipse(p2,pointW,pointW);
                            break;
                        }
                    }
                }
            }
        }


        for(int g=0;g<c_window->dataset.length();g++){
            MitoDataset *curData=c_window->dataset[g];
            if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;


            if(c_window->focus_group==-1){

                for(int i=0;i<curData->features.length();i++){
                    for(int j=0;j<c_window->featureNum-1;j++){
                        //qDebug()<<curData->features[i][j]<<c_window->featureRanges[j];
                        if(curData->globalEnabled[i]){

                            float value1, value2;
                            QPointF p1,p2;
                            value1=(curData->features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
                            p1=QPointF(axisEnds[j].x()*value1+axisStarts[j].x()*(1-value1),
                                       axisEnds[j].y()*value1+axisStarts[j].y()*(1-value1));
                            int k=j+1;
                            value2=(curData->features[i][k]-c_window->featureRanges[k].x())/(c_window->featureRanges[k].y()-c_window->featureRanges[k].x());
                            p2=QPointF(axisEnds[k].x()*value2+axisStarts[k].x()*(1-value2),
                                       axisEnds[k].y()*value2+axisStarts[k].y()*(1-value2));

                            //qDebug()<<p1<<p2<<value1<<value2;

                            t_pen.setWidth(lineW);
                            if(c_window->coloringTypeSet->currentIndex()==0)
                                t_pen.setColor(c_window->typeColors[int(curData->features[i][0])]);
                            else
                                t_pen.setColor(c_window->groups[curData->group]->color);


                            painter.setPen(t_pen);
                            painter.drawEllipse(p1,pointW,pointW);
                            painter.drawLine(p1,p2);
                            if(k==c_window->featureNum-1){
                                painter.drawEllipse(p2,pointW,pointW);
                                break;
                            }
                        }
                    }
                }
            }
        }


        if(c_window->focus_group!=-1){
            t_pen.setStyle(Qt::SolidLine);
            MitoDataset *curData=c_window->dataset[c_window->focus_group];

            int i=curData->focusItem;
            for(int j=0;j<c_window->featureNum-1;j++){
                float value1, value2;
                QPointF p1,p2;
                value1=(curData->features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
                p1=QPointF(axisEnds[j].x()*value1+axisStarts[j].x()*(1-value1),
                           axisEnds[j].y()*value1+axisStarts[j].y()*(1-value1));
                int k=j+1;
                value2=(curData->features[i][k]-c_window->featureRanges[k].x())/(c_window->featureRanges[k].y()-c_window->featureRanges[k].x());
                p2=QPointF(axisEnds[k].x()*value2+axisStarts[k].x()*(1-value2),
                           axisEnds[k].y()*value2+axisStarts[k].y()*(1-value2));


                t_pen.setWidth(lineW);
                t_pen.setColor(QColor(50,50,50,15));

                if(c_window->coloringTypeSet->currentIndex()==0)
                    t_pen.setColor(c_window->typeColors[int(curData->features[i][0])]);
                else
                    t_pen.setColor(c_window->groups[curData->group]->color);

                painter.setPen(t_pen);
                painter.drawEllipse(p1,pointW,pointW);
                painter.drawLine(p1,p2);
                if(k==c_window->featureNum-1){
                    painter.drawEllipse(p2,pointW,pointW);
                    break;
                }
            }
        }

        //qDebug()<<"parallelcoord 1";


            ////Draw Selection frame & selection information

        if(selectionFrame!=-1){
            t_pen.setColor(QColor(0,255,0,40));
            t_pen.setWidth(50*c_window->WINDOW_SCALE);
            t_pen.setCapStyle(Qt::RoundCap);
            painter.setPen(t_pen);
            painter.drawLine(axisStarts[selectionFrame].x(),axisStarts[selectionFrame].y()
                             ,axisEnds[selectionFrame].x(),axisEnds[selectionFrame].y());
        }

        //qDebug()<<"parallelcoord 2";


        for(int i=0;i<c_window->featureNum;i++){

            if(c_window->globalSelection[i]){
                t_pen.setColor(QColor(35,255,159,200));
                t_pen.setWidth(20*c_window->WINDOW_SCALE);
                t_pen.setCapStyle(Qt::RoundCap);
                painter.setPen(t_pen);
                QPointF p1(axisStarts[i].x(),axisStarts[i].y());
                QPointF p2(axisEnds[i].x(),axisEnds[i].y());
                painter.drawLine(p1*(1-c_window->globalSelectionRange[i].x())+p2*c_window->globalSelectionRange[i].x(),
                                 p1*(1-c_window->globalSelectionRange[i].y())+p2*c_window->globalSelectionRange[i].y());

            }
        }


        // draw axis range bar

        for(int i=0;i<c_window->featureNum;i++){

            t_brush.setColor(QColor(50,168,82));
            painter.setBrush(t_brush);

    //        t_pen.setColor(QColor(50,168,82));
    //        t_pen.setWidth(10*c_window->WINDOW_SCALE);
    //        t_pen.setCapStyle(Qt::FlatCap);
            painter.setPen(Qt::NoPen);

            QPointF button[3];

            QPointF p1(axisStarts[i].x(),axisEnds[i].y()+(axisStarts[i].y()-axisEnds[i].y())*(1-c_window->globalAxisRange[i].x()));

            button[0]=p1;
            button[1]=p1+QPointF(-10,10);
            button[2]=p1+QPointF(10,10);
            painter.drawConvexPolygon(button,3);
            if(is_axis_edit){
                thresh_bar_start[i]->setGeometry(p1.x()-20,p1.y()+10,40,20);
                if(editMode==false)
                    thresh_bar_start[i]->setText(QString::number(c_window->featureRanges[i].x()+(c_window->featureRanges[i].y()-c_window->featureRanges[i].x())*c_window->globalAxisRange[i].x(),'f',2));
                thresh_bar_start[i]->show();

                if(i==0){
                    if(c_window->globalAxisRange[i].x()==0){
                        thresh_bar_start[i]->setGeometry(p1.x()-30,p1.y()+10,60,20);
                        thresh_bar_start[i]->setText("Dendrite");
                    }
                    else thresh_bar_start[i]->setText("");
                }
            }


            QPointF p2(axisEnds[i].x(),axisEnds[i].y()+(axisStarts[i].y()-axisEnds[i].y())*(1-c_window->globalAxisRange[i].y()));
            button[0]=p2;
            button[1]=p2+QPointF(-10,-10);
            button[2]=p2+QPointF(10,-10);
            painter.drawConvexPolygon(button,3);

            thresh_bar_end[i]->setGeometry(p2.x()-20,p2.y()-30,40,20);
            if(editMode==false)
                thresh_bar_end[i]->setText(QString::number(c_window->featureRanges[i].x()+(c_window->featureRanges[i].y()-c_window->featureRanges[i].x())*c_window->globalAxisRange[i].y(),'f',2));
            thresh_bar_end[i]->show();

            if(i==0){
                if(c_window->globalAxisRange[i].y()==1){
                    thresh_bar_end[i]->setText("Axon");
                }
                else thresh_bar_end[i]->setText("");
            }

        }



        for(int i=0;i<c_window->featureNum;i++){
            axisStarts[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,this->height()*0.9);
            axisEnds[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,this->height()*0.3);
        }


        ////Draw Label
        t_pen.setColor(QColor(0,0,0));
        painter.setPen(t_pen);
        tFont.setBold(true);
        painter.setFont(tFont);
        for(int i=0;i<c_window->featureNum;i++){
            painter.drawText(axisEnds[i].x()-c_window->featureNames[i].length()*5,axisEnds[i].y()-this->height()*0.1,c_window->featureNames[i]);

        }

        //// draw values
        t_pen.setColor(QColor(0,0,0));
        t_pen.setCapStyle(Qt::RoundCap);
        painter.setPen(t_pen);
        painter.setBrush(Qt::NoBrush);



        if(c_window->pcp_expand->isChecked()){
            for(int i=0;i<c_window->featureNum;i++){
                axisStarts[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,
                                        this->height()*0.9+c_window->globalAxisRange[i].x()/(c_window->globalAxisRange[i].y()-c_window->globalAxisRange[i].x())*this->height()*0.6);
                axisEnds[i]=QVector2D(this->width()*0.1+this->width()*0.8/(c_window->featureNum-1)*i,
                                      this->height()*0.3-((1-c_window->globalAxisRange[i].y())/(c_window->globalAxisRange[i].y()-c_window->globalAxisRange[i].x())*this->height()*0.6));
            }
        }


    }


    is_axis_edit=false;
    //qDebug()<<"end parallel";


}


void ParallelCoordinate::mousePressEvent(QMouseEvent *event)
{
    if(c_window->curDataIndex==-1)return;

    if(event->x()<0 || event->x()>this->width() || event->y()<0 || event->y()>this->height())return;

    lastPos = event->pos();
    button=event->button();

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];


    if(event->button() == Qt::RightButton){
        if(doSelect==false && selectionFrame!=-1){
            doSelect=true;
            if(c_window->analysis_type->currentIndex()==0){
                curData->selection[selectionFrame]=true;
                c_window->dataset[c_window->curDataIndex]->focusItem=-1;
                c_window->focus_group=-1;
                float d=(event->y()-axisStarts[selectionFrame].y())/(axisEnds[selectionFrame].y()-axisStarts[selectionFrame].y());
                curData->selectionRange[selectionFrame]=QVector2D(d,d);
                startSelectValue=d;

            }
            else{
                c_window->globalSelection[selectionFrame]=true;
                c_window->focus_group=-1;
                float d=(event->y()-axisStarts[selectionFrame].y())/(axisEnds[selectionFrame].y()-axisStarts[selectionFrame].y());
                c_window->globalSelectionRange[selectionFrame]=QVector2D(d,d);
                startSelectValue=d;

            }
        }
    }

    clicked_thresh_bar=-1;
    if(event->button() ==Qt::LeftButton){
        if(c_window->analysis_type->currentIndex()==0){
            for(int i=0;i<c_window->featureNum;i++){

                QPointF p1(axisStarts[i].x(),axisEnds[i].y()+(axisStarts[i].y()-axisEnds[i].y())*(1-curData->axisRange[i].x()));
                if(abs(lastPos.x()-p1.x())<10 && abs(lastPos.y()-(p1.y()+5))<5){
                    clicked_thresh_bar_type="start";
                    clicked_thresh_bar=i;
                }
                QPointF p2(axisEnds[i].x(),axisEnds[i].y()+(axisStarts[i].y()-axisEnds[i].y())*(1-curData->axisRange[i].y()));
                if(abs(lastPos.x()-p2.x())<10 && abs(lastPos.y()-(p2.y()-5))<5){
                    clicked_thresh_bar_type="end";
                    clicked_thresh_bar=i;
                }

            }
            if(clicked_thresh_bar!=-1){
                c_window->dataset[c_window->curDataIndex]->focusItem=-1;
                c_window->focus_group=-1;
            }
        }
        else{
            for(int i=0;i<c_window->featureNum;i++){

                QPointF p1(axisStarts[i].x(),axisEnds[i].y()+(axisStarts[i].y()-axisEnds[i].y())*(1-c_window->globalAxisRange[i].x()));
                if(abs(lastPos.x()-p1.x())<10 && abs(lastPos.y()-(p1.y()+5))<5){
                    clicked_thresh_bar_type="start";
                    clicked_thresh_bar=i;
                }
                QPointF p2(axisEnds[i].x(),axisEnds[i].y()+(axisStarts[i].y()-axisEnds[i].y())*(1-c_window->globalAxisRange[i].y()));
                if(abs(lastPos.x()-p2.x())<10 && abs(lastPos.y()-(p2.y()-5))<5){
                    clicked_thresh_bar_type="end";
                    clicked_thresh_bar=i;
                }

            }
            if(clicked_thresh_bar!=-1){
                c_window->focus_group=-1;
            }
        }

    }
    update();
    c_window->synchronization();

}
void ParallelCoordinate::mouseReleaseEvent(QMouseEvent *event){

    if(c_window->curDataIndex==-1)return;

    clicked_thresh_bar=-1;

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    if(event->button() == Qt::RightButton){
        if(doSelect==true){
            doSelect=false;
            if(c_window->analysis_type->currentIndex()==0){

                if(curData->selectionRange[selectionFrame].y()-curData->selectionRange[selectionFrame].x()<0.01){
                    curData->selection[selectionFrame]=false;
                }
                curData->checkDataEnable(c_window);
            }
            else{
                if(c_window->globalSelectionRange[selectionFrame].y()-c_window->globalSelectionRange[selectionFrame].x()<0.01){
                    c_window->globalSelection[selectionFrame]=false;
                }
                for(int g=0;g<c_window->dataset.length();g++){
                       c_window->dataset[g]->checkDataEnable(c_window);
                }
            }
            update();
            c_window->synchronization();
        }
    }

    if(c_window->analysis_type->currentIndex()==0){
        QPoint diff=lastPos-event->pos();
        if(event->button() == Qt::LeftButton && abs(diff.x())+abs(diff.y())<2){
            if(curData->focusItem!=-1){
                Mito item=curData->mitoData[curData->focusItem];
                QVector2D tP=QVector2D((item.xmax+item.xmin)/2.0,(item.ymax+item.ymin)/2.0);
                curData->shift=QVector2D(curData->ImageW, curData->ImageH)/2.0-tP;
                update();
                c_window->synchronization();

            }
        }
    }
    else{
        QPoint diff=lastPos-event->pos();
        if(event->button() == Qt::LeftButton && abs(diff.x())+abs(diff.y())<2){
            if(c_window->focus_group!=-1){
                MitoDataset *curData=c_window->dataset[c_window->focus_group];
                Mito item=curData->mitoData[curData->focusItem];
                QVector2D tP=QVector2D((item.xmax+item.xmin)/2.0,(item.ymax+item.ymin)/2.0);
                curData->shift=QVector2D(curData->ImageW, curData->ImageH)/2.0-tP;
                update();
                c_window->synchronization();

            }
        }
    }



}
void ParallelCoordinate::mouseMoveEvent(QMouseEvent *event)
{
    if(c_window->curDataIndex==-1)return;

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    setFocus();

    if(event->x()<0 || event->x()>this->width() || event->y()<0 || event->y()>this->height())return;

    float tx=event->x();
    float ty=event->y();

    if(clicked_thresh_bar!=-1){
        if(clicked_thresh_bar_type=="start"){
            float v=(event->y()-axisStarts[clicked_thresh_bar].y())/(axisEnds[clicked_thresh_bar].y()-axisStarts[clicked_thresh_bar].y());
            if(c_window->analysis_type->currentIndex()==0){
                curData->axisRange[clicked_thresh_bar].setX(v);
                if(v<0)curData->axisRange[clicked_thresh_bar].setX(0);
                if(v>1)curData->axisRange[clicked_thresh_bar].setX(1);
            }
            else{
                c_window->globalAxisRange[clicked_thresh_bar].setX(v);
                if(v<0) c_window->globalAxisRange[clicked_thresh_bar].setX(0);
                if(v>1) c_window->globalAxisRange[clicked_thresh_bar].setX(1);

            }

        }
        else{
            float v=(event->y()-axisStarts[clicked_thresh_bar].y())/(axisEnds[clicked_thresh_bar].y()-axisStarts[clicked_thresh_bar].y());
            if(c_window->analysis_type->currentIndex()==0){
                curData->axisRange[clicked_thresh_bar].setY(v);
                if(v<0)curData->axisRange[clicked_thresh_bar].setY(0);
                if(v>1)curData->axisRange[clicked_thresh_bar].setY(1);
            }
            else{
                c_window->globalAxisRange[clicked_thresh_bar].setY(v);
                if(v<0) c_window->globalAxisRange[clicked_thresh_bar].setY(0);
                if(v>1) c_window->globalAxisRange[clicked_thresh_bar].setY(1);

            }
        }
        if(c_window->analysis_type->currentIndex()==0){
            curData->checkDataEnable(c_window);
        }
        else{
            for(int g=0;g<c_window->dataset.length();g++){
                   c_window->dataset[g]->checkDataEnable(c_window);
            }
        }
        is_axis_edit=true;
        update();
        c_window->synchronization();
        return;
    }

    if(doSelect){
        QPointF p1(axisStarts[selectionFrame].x(),axisStarts[selectionFrame].y());
        QPointF p2(axisEnds[selectionFrame].x(),axisEnds[selectionFrame].y());
        if(ty<=p1.y()+10 && ty>=p2.y()-10){
            float dis=abs(tx-p1.x());
            if(dis>25){
                doSelect=false;
                if(c_window->analysis_type->currentIndex()==0){
                    if(curData->selectionRange[selectionFrame].y()-curData->selectionRange[selectionFrame].x()<0.01){
                        curData->selection[selectionFrame]=false;
                    }
                }
                else{
                    if(c_window->globalSelectionRange[selectionFrame].y()-c_window->globalSelectionRange[selectionFrame].x()<0.01){
                        c_window->globalSelection[selectionFrame]=false;
                    }
                }
            }
            else{
                float v=(event->y()-axisStarts[selectionFrame].y())/(axisEnds[selectionFrame].y()-axisStarts[selectionFrame].y());
                if(c_window->analysis_type->currentIndex()==0){
                    if(startSelectValue>v){
                        curData->selectionRange[selectionFrame].setX(v);
                        curData->selectionRange[selectionFrame].setY(startSelectValue);
                    }
                    if(startSelectValue<v){
                        curData->selectionRange[selectionFrame].setX(startSelectValue);
                        curData->selectionRange[selectionFrame].setY(v);
                    }
                }
                else{
                    if(startSelectValue>v){
                        c_window->globalSelectionRange[selectionFrame].setX(v);
                        c_window->globalSelectionRange[selectionFrame].setY(startSelectValue);
                    }
                    if(startSelectValue<v){
                        c_window->globalSelectionRange[selectionFrame].setX(startSelectValue);
                        c_window->globalSelectionRange[selectionFrame].setY(v);
                    }
                }

            }
        }

        if(c_window->analysis_type->currentIndex()==0){
            curData->checkDataEnable(c_window);
        }
        else{
            for(int g=0;g<c_window->dataset.length();g++){
                   c_window->dataset[g]->checkDataEnable(c_window);
            }
        }
        update();
        c_window->synchronization();

        return;
    }


    int prev=selectionFrame;
    selectionFrame=-1;
    float minDis=10000;
    for(int i=0;i<c_window->featureNum;i++){

        QPointF p1(axisStarts[i].x(),axisStarts[i].y());
        QPointF p2(axisEnds[i].x(),axisEnds[i].y());
        if(ty<=p1.y()+10 && ty>=p2.y()-10){
            float dis=abs(tx-p1.x());
            if(dis<minDis && dis<25){
                minDis=dis;
                selectionFrame=i;
            }
        }
    }
    if(prev!=selectionFrame){
        update();
        c_window->synchronization();
    }

    if(c_window->analysis_type->currentIndex()==0){

        minDis=10000;
        prev=curData->focusItem;
        curData->focusItem=-1;

        for(int i=0;i<curData->features.length();i++){
            if(curData->enabled[i]==false){
                continue;
            }
            for(int j=0;j<c_window->featureNum-1;j++){


                float value1, value2;
                QPointF p1,p2;

                value1=(curData->features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
                p1=QPointF(axisEnds[j].x()*value1+axisStarts[j].x()*(1-value1),
                           axisEnds[j].y()*value1+axisStarts[j].y()*(1-value1));
                int k=j+1;
                value2=(curData->features[i][k]-c_window->featureRanges[k].x())/(c_window->featureRanges[k].y()-c_window->featureRanges[k].x());
                p2=QPointF(axisEnds[k].x()*value2+axisStarts[k].x()*(1-value2),
                           axisEnds[k].y()*value2+axisStarts[k].y()*(1-value2));

                if(k!=c_window->featureNum){

                    if(((tx<=p1.x()+10 && tx>=p2.x()-10)||(tx<=p2.x()+10 && tx>=p1.x()-10))
                            && ((ty<=p1.y()+10 && ty>=p2.y()-10)||(ty<=p2.y()+10 && ty>=p1.y()-10))){

                        float dis=abs(ty-p1.y()-((p2.y()-p1.y())/(p2.x()-p1.x()))*(tx-p1.x()))
                                /sqrt(1+( (p2.y()-p1.y())/(p2.x()-p1.x()) )*( (p2.y()-p1.y())/(p2.x()-p1.x()) ));

                        if(dis<minDis && dis<25){
                            minDis=dis;
                            curData->focusItem=i;
                        }
                    }
                }
            }
        }
        if(prev!=curData->focusItem){
            c_window->dataRendering->mitoLabelChanged=true;
            update();
            c_window->synchronization();
        }
    }
    else{
        minDis=10000;
        int new_focus_item=-1;
        int prev_group=c_window->focus_group;
        c_window->focus_group=-1;
        if(prev_group==-1){
            prev=-1;
        }
        else{
            prev=curData->focusItem;
            curData->focusItem=-1;
        }

        for(int g=0;g<c_window->dataset.length();g++){

            MitoDataset *curData=c_window->dataset[g];
            if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;

            for(int i=0;i<curData->features.length();i++){
                if(curData->globalEnabled[i]==false){
                    continue;
                }
                for(int j=0;j<c_window->featureNum-1;j++){


                    float value1, value2;
                    QPointF p1,p2;

                    value1=(curData->features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
                    p1=QPointF(axisEnds[j].x()*value1+axisStarts[j].x()*(1-value1),
                               axisEnds[j].y()*value1+axisStarts[j].y()*(1-value1));
                    int k=j+1;
                    value2=(curData->features[i][k]-c_window->featureRanges[k].x())/(c_window->featureRanges[k].y()-c_window->featureRanges[k].x());
                    p2=QPointF(axisEnds[k].x()*value2+axisStarts[k].x()*(1-value2),
                               axisEnds[k].y()*value2+axisStarts[k].y()*(1-value2));

                    if(k!=c_window->featureNum){

                        if(((tx<=p1.x()+10 && tx>=p2.x()-10)||(tx<=p2.x()+10 && tx>=p1.x()-10))
                                && ((ty<=p1.y()+10 && ty>=p2.y()-10)||(ty<=p2.y()+10 && ty>=p1.y()-10))){

                            float dis=abs(ty-p1.y()-((p2.y()-p1.y())/(p2.x()-p1.x()))*(tx-p1.x()))
                                    /sqrt(1+( (p2.y()-p1.y())/(p2.x()-p1.x()) )*( (p2.y()-p1.y())/(p2.x()-p1.x()) ));

                            if(dis<minDis && dis<25){
                                minDis=dis;
                                c_window->focus_group=g;
                                new_focus_item=i;
                            }
                        }
                    }
                }
            }
        }
        if(c_window->focus_group!=-1){
            c_window->dataset[c_window->focus_group]->focusItem=new_focus_item;
        }
        if(prev!=new_focus_item || prev_group!=c_window->focus_group){
            c_window->dataRendering->mitoLabelChanged=true;
            update();
            c_window->synchronization();
        }
    }

}

void ParallelCoordinate::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Escape){
    }
    if(event->key()==Qt::Key_Shift){
        c_window->IsShift=true;
    }
}
void ParallelCoordinate::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Shift){
        c_window->IsShift=false;
    }
}

void ParallelCoordinate::timerEvent(QTimerEvent *event){
}


void ParallelCoordinate::readAnalysisFile(){


}
