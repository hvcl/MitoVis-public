///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

#include "circularCoordinate.h"
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
#include <QSpinBox>

CircularCoordinate::CircularCoordinate(Window *p, QWidget *parent)
    : QWidget(parent)
{

    c_window=p;
    readAnalysisFile();

    c_window->focusSpine=-1;
    c_window->clicked=false;

    selectionFrame=-1;
    doSelect=false;



    for(int i=0;i<30;i++){
        featurePoints.push_back(QVector2D(0,0));
    }

    setBackgroundRole(QPalette::Base);
    QWidget::setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);


}

CircularCoordinate::~CircularCoordinate(){
}

void CircularCoordinate::resizeEvent(QResizeEvent *event){
    //    int w=event->size().width();
    //    int h=event->size().height();
    //    this->resize(w,h);
}

QSize CircularCoordinate::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize CircularCoordinate::sizeHint() const
{
    return QSize(800, 800);
}

void CircularCoordinate::paintEvent(QPaintEvent *)
{



}

void CircularCoordinate::mousePressEvent(QMouseEvent *event)
{
//    if(event->x()<0 || event->x()>this->width() || event->y()<0 || event->y()>this->height())return;

//    lastPos = event->pos();
//    button=event->button();

//    if(c_window->spineEnableMode->isChecked()){
//        if(c_window->focusSpine!=-1){
//            c_window->displaySpines[c_window->focusSpine]=true;
//        }
//        return;
//    }


//    if(event->button()==Qt::LeftButton){
//        if(c_window->focusSpine!=-1){
//            if(c_window->IsShift){
//                c_window->addSpineToGroup(c_window->focusSpine,c_window->currentSelectGroup);
//            }
//            else{
//                c_window->clicked=true;
//                emit viewChange(c_window->focusSpine);
//            }
//        }
//    }
//    else if(event->button()==Qt::MidButton){
//    }
//    else if(event->button() == Qt::RightButton){
//        if(doSelect==false && selectionFrame!=-1){
//            doSelect=true;
//            c_window->selection[selectionFrame]=true;
//            c_window->focusSpine=-1;
//            if(c_window->clicked==true){
//                c_window->focusSpine=-1;
//                c_window->clicked=false;
//                emit viewChange(-1);
//            }
//            QVector2D p1(featurePoints[selectionFrame].x()*0.1+midPoint.x()*0.9,featurePoints[selectionFrame].y()*0.1+midPoint.y()*0.9);
//            float d1=(featurePoints[selectionFrame]-p1).length();
//            float d2=(p1-QVector2D(event->x(),event->y())).length();
//            float d3=(featurePoints[selectionFrame]-QVector2D(event->x(),event->y())).length();

//            startSelectValue=d2/d1;
//            if(d1<d3)startSelectValue=-startSelectValue;
//            c_window->selectionRange[selectionFrame]=QVector2D(d2/d1,d2/d1);
//        }
//    }
//    update();
//    emit synchronization();

}
void CircularCoordinate::mouseReleaseEvent(QMouseEvent *event){

//    if(event->button() == Qt::RightButton){
//        if(doSelect==true){
//            doSelect=false;
//            if(c_window->selectionRange[selectionFrame].y()-c_window->selectionRange[selectionFrame].x()<0.01){
//                c_window->selection[selectionFrame]=false;
//            }
//            c_window->checkSpineEnable();
//            update();
//            emit synchronization();
//            ////Todo:
//            ////update spine grid using selection info
//        }
//    }

}
void CircularCoordinate::mouseMoveEvent(QMouseEvent *event)
{
//    setFocus();

//    if(event->x()<0 || event->x()>this->width() || event->y()<0 || event->y()>this->height())return;

//    float tx=event->x();
//    float ty=event->y();

//    if(doSelect){
//        QPointF p1(featurePoints[selectionFrame].x()*0.1+midPoint.x()*0.9,featurePoints[selectionFrame].y()*0.1+midPoint.y()*0.9);
//        QPointF p2(featurePoints[selectionFrame].x(),featurePoints[selectionFrame].y());
//        if(abs(p1.x()-p2.x())<1.0){
//            if(((ty<=p1.y()+10 && ty>=p2.y()-10)||(ty<=p2.y()+10 && ty>=p1.y()-10))){
//                float dis=abs(tx-p1.x());
//                if(dis>25){
//                    doSelect=false;
//                    if(c_window->selectionRange[selectionFrame].y()-c_window->selectionRange[selectionFrame].x()<0.01){
//                        c_window->selection[selectionFrame]=false;
//                    }
//                    ////Todo:
//                    ////update spine grid using selection info
//                }
//                else{
//                    QVector2D p(featurePoints[selectionFrame].x()*0.1+midPoint.x()*0.9,featurePoints[selectionFrame].y()*0.1+midPoint.y()*0.9);
//                    float d1=(featurePoints[selectionFrame]-p).length();
//                    float d2=(p-QVector2D(event->x(),event->y())).length();
//                    float d3=(featurePoints[selectionFrame]-QVector2D(event->x(),event->y())).length();

//                    float v=d2/d1;
//                    if(d1<d3)v=-v;
//                    if(startSelectValue>v){
//                        c_window->selectionRange[selectionFrame].setX(v);
//                        c_window->selectionRange[selectionFrame].setY(startSelectValue);
//                    }
//                    if(startSelectValue<v){
//                        c_window->selectionRange[selectionFrame].setX(startSelectValue);
//                        c_window->selectionRange[selectionFrame].setY(v);
//                    }

//                }
//            }
//        }
//        else if(abs(p1.y()-p2.y())<1.0){
//            if(((tx<=p1.x()+10 && tx>=p2.x()-10)||(tx<=p2.x()+10 && tx>=p1.x()-10))){

//                float dis=abs(ty-p1.y());
//                if(dis>25){
//                    doSelect=false;
//                    if(c_window->selectionRange[selectionFrame].y()-c_window->selectionRange[selectionFrame].x()<0.01){
//                        c_window->selection[selectionFrame]=false;
//                    }
//                    ////Todo:
//                    ////update spine grid using selection info
//                }
//                else{
//                    QVector2D p(featurePoints[selectionFrame].x()*0.1+midPoint.x()*0.9,featurePoints[selectionFrame].y()*0.1+midPoint.y()*0.9);
//                    float d1=(featurePoints[selectionFrame]-p).length();
//                    float d2=(p-QVector2D(event->x(),event->y())).length();
//                    float d3=(featurePoints[selectionFrame]-QVector2D(event->x(),event->y())).length();

//                    float v=d2/d1;
//                    if(d1<d3)v=-v;
//                    if(startSelectValue>v){
//                        c_window->selectionRange[selectionFrame].setX(v);
//                        c_window->selectionRange[selectionFrame].setY(startSelectValue);
//                    }
//                    if(startSelectValue<v){
//                        c_window->selectionRange[selectionFrame].setX(startSelectValue);
//                        c_window->selectionRange[selectionFrame].setY(v);
//                    }

//                }
//            }
//        }
//        else{
//            if(((tx<=p1.x()+10 && tx>=p2.x()-10)||(tx<=p2.x()+10 && tx>=p1.x()-10))
//                    && ((ty<=p1.y()+10 && ty>=p2.y()-10)||(ty<=p2.y()+10 && ty>=p1.y()-10))){

//                float dis=abs(ty-p1.y()-((p2.y()-p1.y())/(p2.x()-p1.x()))*(tx-p1.x()))
//                        /sqrt(1+( (p2.y()-p1.y())/(p2.x()-p1.x()) )*( (p2.y()-p1.y())/(p2.x()-p1.x()) ));
//                if(dis>25){
//                    doSelect=false;
//                    if(c_window->selectionRange[selectionFrame].y()-c_window->selectionRange[selectionFrame].x()<0.01){
//                        c_window->selection[selectionFrame]=false;
//                    }
//                    ////Todo:
//                    ////update spine grid using selection info
//                }
//                else{
//                    QVector2D p(featurePoints[selectionFrame].x()*0.1+midPoint.x()*0.9,featurePoints[selectionFrame].y()*0.1+midPoint.y()*0.9);
//                    float d1=(featurePoints[selectionFrame]-p).length();
//                    float d2=(p-QVector2D(event->x(),event->y())).length();
//                    float d3=(featurePoints[selectionFrame]-QVector2D(event->x(),event->y())).length();

//                    float v=d2/d1;
//                    if(d1<d3)v=-v;
//                    if(startSelectValue>v){
//                        c_window->selectionRange[selectionFrame].setX(v);
//                        c_window->selectionRange[selectionFrame].setY(startSelectValue);
//                    }
//                    if(startSelectValue<v){
//                        c_window->selectionRange[selectionFrame].setX(startSelectValue);
//                        c_window->selectionRange[selectionFrame].setY(v);
//                    }

//                }
//            }
//        }
//        c_window->checkSpineEnable();
//        update();
//        emit synchronization();

//        return;
//    }


//    int prev=selectionFrame;
//    selectionFrame=-1;
//    float minDis=10000;
//    for(int i=0;i<c_window->featureNum;i++){

//        if(c_window->featureEnables[i]){
//            QPointF p1(featurePoints[i].x()*0.1+midPoint.x()*0.9,featurePoints[i].y()*0.1+midPoint.y()*0.9);
//            QPointF p2(featurePoints[i].x(),featurePoints[i].y());
//            if(abs(p1.x()-p2.x())<1.0){
//                if(((ty<=p1.y()+10 && ty>=p2.y()-10)||(ty<=p2.y()+10 && ty>=p1.y()-10))){
//                    float dis=abs(tx-p1.x());
//                    if(dis<minDis && dis<25){
//                        minDis=dis;
//                        selectionFrame=i;
//                    }
//                }
//            }
//            else if(abs(p1.y()-p2.y())<1.0){
//                if(((tx<=p1.x()+10 && tx>=p2.x()-10)||(tx<=p2.x()+10 && tx>=p1.x()-10))){

//                    float dis=abs(ty-p1.y());

//                    if(dis<minDis && dis<25){
//                        minDis=dis;
//                        selectionFrame=i;
//                    }
//                }
//            }
//            else{
//                if(((tx<=p1.x()+10 && tx>=p2.x()-10)||(tx<=p2.x()+10 && tx>=p1.x()-10))
//                        && ((ty<=p1.y()+10 && ty>=p2.y()-10)||(ty<=p2.y()+10 && ty>=p1.y()-10))){

//                    float dis=abs(ty-p1.y()-((p2.y()-p1.y())/(p2.x()-p1.x()))*(tx-p1.x()))
//                            /sqrt(1+( (p2.y()-p1.y())/(p2.x()-p1.x()) )*( (p2.y()-p1.y())/(p2.x()-p1.x()) ));

//                    if(dis<minDis && dis<25){
//                        minDis=dis;
//                        selectionFrame=i;
//                    }
//                }
//            }
//        }

//    }
//    if(prev!=selectionFrame){
//        update();
//        emit synchronization();
//    }


//    if(c_window->clicked)return;

//    minDis=10000;
//    prev=c_window->focusSpine;
//    c_window->focusSpine=-1;

//    if(!c_window->isFocus)return;
//    for(int i=0;i<c_window->data.length();i++){
//        if(c_window->spineEnable[i]==false){
//            continue;
//        }
//        for(int j=0;j<c_window->featureNum;j++){
//            if(c_window->featureEnables[j]){

//                float value1,value2;
//                QPointF p1,p2;

//                value1=(c_window->data[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
//                p1=QPointF(featurePoints[j].x()*value1+(featurePoints[j].x()*0.1+midPoint.x()*0.9)*(1-value1),
//                           featurePoints[j].y()*value1+(featurePoints[j].y()*0.1+midPoint.y()*0.9)*(1-value1));

//                for(int k=j+1;;k++){
//                    if(c_window->featureEnables[k%c_window->featureNum]){
//                        value2=(c_window->data[i][k%c_window->featureNum]-c_window->featureRanges[k%c_window->featureNum].x())/(c_window->featureRanges[k%c_window->featureNum].y()-c_window->featureRanges[k%c_window->featureNum].x());
//                        p2=QPointF(featurePoints[k%c_window->featureNum].x()*value2+(featurePoints[k%c_window->featureNum].x()*0.1+midPoint.x()*0.9)*(1-value2),
//                                featurePoints[k%c_window->featureNum].y()*value2+(featurePoints[k%c_window->featureNum].y()*0.1+midPoint.y()*0.9)*(1-value2));
//                        break;
//                    }
//                }


//                if(((tx<=p1.x()+10 && tx>=p2.x()-10)||(tx<=p2.x()+10 && tx>=p1.x()-10))
//                        && ((ty<=p1.y()+10 && ty>=p2.y()-10)||(ty<=p2.y()+10 && ty>=p1.y()-10))){

//                    float dis=abs(ty-p1.y()-((p2.y()-p1.y())/(p2.x()-p1.x()))*(tx-p1.x()))
//                            /sqrt(1+( (p2.y()-p1.y())/(p2.x()-p1.x()) )*( (p2.y()-p1.y())/(p2.x()-p1.x()) ));

//                    if(dis<minDis && dis<25){
//                        minDis=dis;
//                        c_window->focusSpine=i;
//                    }
//                }
//            }
//        }
//    }
//    if(prev!=c_window->focusSpine){
//        c_window->changeTitle();

//        update();
//        emit synchronization();
//    }

}

void CircularCoordinate::keyPressEvent(QKeyEvent *event)
{
//    if(event->key()==Qt::Key_Escape){
//        if(c_window->clicked==true){
//            c_window->clicked=false;
//            c_window->focusSpine=-1;
//            emit viewChange(-1);
//        }
//    }

//    if(event->key()==Qt::Key_Shift){
//        c_window->IsShift=true;
//    }
}
void CircularCoordinate::keyReleaseEvent(QKeyEvent *event)
{
//    if(event->key()==Qt::Key_Shift){
//        c_window->IsShift=false;
//    }
}


void CircularCoordinate::timerEvent(QTimerEvent *event){
}


void CircularCoordinate::readAnalysisFile(){
    c_window->typeEnable=false;
    c_window->testTrainEnable=false;
    QFile File("spineFeatureSetting.csv");

    if (File.open(QIODevice::ReadOnly))
    {
        QTextStream input(&File);
        QString headerLine = input.readLine();
        QStringList headers=headerLine.split(QChar(','),QString::KeepEmptyParts);

//        c_window->featureNames=new QString[headers.count()-1];

        for(int i=0;i<headers.count()-1;i++){
            c_window->featureNames.push_back(QString(""));
        }

        for(int i=1;i<headers.count();i++){
            c_window->featureNames[i-1]=headers[i];
            if(headers[i]=="Type"){
                c_window->typeEnable=true;
            }
            else if(headers[i]=="TrainTest"){
                c_window->testTrainEnable=true;
            }
        }
        if(c_window->typeEnable && c_window->testTrainEnable){
            c_window->featureNum=headers.count()-3;
        }
        else if(c_window->testTrainEnable){
            c_window->featureNum=headers.count()-2;
        }

        else if(c_window->typeEnable){
            c_window->featureNum=headers.count()-2;
        }
        else{
            c_window->featureNum=headers.count()-1;
        }

        for(int i=0;i<c_window->featureNum;i++){
            c_window->featureRanges.push_back(QVector2D(1000,-1000));
        }

        File.close();
    }


//    if(c_window->typeEnable && c_window->testTrainEnable){
//        c_window->featureNames.pop_back();
//        c_window->featureNames.pop_back();

//    }
//    else if(c_window->testTrainEnable){
//        c_window->featureNames.pop_back();
//    }

//    else if(c_window->typeEnable){
//        c_window->featureNames.pop_back();
//    }

}

