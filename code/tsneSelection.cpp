///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

#include "tsneSelection.h"
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
#include <QPushButton>
#include <QComboBox>
TSNE_worker::TSNE_worker(Window *p){
    c_window=p;
}

void TSNE_worker::selective_run(){

}


void TSNE_worker::run(){

    if(c_window->analysis_type->currentIndex()==0){

        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];


        int featureNum=0;
        FILE *Input=fopen("tsneInput.raw","wb");
        for(int i=0;i<curData->features.length();i++){
            featureNum=0;
            for(int j=1;j<c_window->featureNum;j++){
                    featureNum++;
                    float v=(curData->features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
                    fwrite(&v,4,1,Input);
            }
        }
        fclose(Input);

        char tt[30];
        std::string query;
        if(c_window->projection1->currentIndex()==0){
            query="python pca.py ";

        }
        else if(c_window->projection1->currentIndex()==1){
            query="python umap_test.py ";
        }
        query+=itoa(curData->features.length(),tt,10);
        query+=" ";
        query+=itoa(featureNum,tt,10);

        query+=" > tsneLog";

        qDebug()<<QString(query.c_str());


        system(query.c_str());
        //system("start \"\" cmd /c C:/\"Program Files\"/Wireshark/tshark.exe -a duration:130 -i 3 -T fields -e frame.number -e frame.time -e _ws.col.Info ^> c:/test/output.csv");


        qDebug()<<"11";

        emit resultReady();
    }
    else{
        int datacnt=0;
        int featureNum=0;
        FILE *Input=fopen("tsneInput.raw","wb");

        for(int g=0;g<c_window->dataset.length();g++){
            MitoDataset *curData=c_window->dataset[g];
            if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;

            for(int i=0;i<curData->features.length();i++){
                featureNum=0;
                for(int j=1;j<c_window->featureNum;j++){
                        featureNum++;
                        float v=(curData->features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
                        fwrite(&v,4,1,Input);
                }
                datacnt++;

            }
        }
        fclose(Input);

        char tt[30];
        std::string query;
        if(c_window->projection1->currentIndex()==0){
            query="python pca.py ";

        }
        else if(c_window->projection1->currentIndex()==1){
            query="python umap_test.py ";
        }
        query+=itoa(datacnt,tt,10);
        query+=" ";
        query+=itoa(featureNum,tt,10);

        query+=" > tsneLog";

        qDebug()<<QString(query.c_str());


        system(query.c_str());
        //system("start \"\" cmd /c C:/\"Program Files\"/Wireshark/tshark.exe -a duration:130 -i 3 -T fields -e frame.number -e frame.time -e _ws.col.Info ^> c:/test/output.csv");



        emit resultReady();
    }
}


void TSNE_worker::TrendPCArun(){

}


void TSNE_worker::TrendtSNErun(){


}


void tsneSelection::changedResult(){
    if(c_window->analysis_type->currentIndex()==0){

        qDebug()<<"12";


        FILE *Result=fopen("tsneResult.raw","rb");

        maxValue=QVector2D(-1000,-1000);
        minValue=QVector2D(1000,1000);

        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

        curData->plotCoord.clear();

        for(int i=0;i<curData->features.length();i++){
            float t[2];
            fread(t,4,2,Result);
            curData->plotCoord.push_back(QVector2D(t[0],t[1]));
            if(t[0]>maxValue.x()){
                maxValue.setX(t[0]);
            }
            if(t[1]>maxValue.y()){
                maxValue.setY(t[1]);
            }
            if(t[0]<minValue.x()){
                minValue.setX(t[0]);
            }
            if(t[1]<minValue.y()){
                minValue.setY(t[1]);
            }
        }

        fclose(Result);

        qDebug()<<"13";

        for(int i=0;i<curData->plotCoord.length();i++){
            curData->plotCoord[i].setX((curData->plotCoord[i].x()-minValue.x())/(maxValue.x()-minValue.x()));
            curData->plotCoord[i].setY((curData->plotCoord[i].y()-minValue.y())/(maxValue.y()-minValue.y()));
        }

        qDebug()<<"14";


        c_window->synchronization();
        c_window->arrangeRanderPart();

        qDebug()<<"15";

        //c_window->preOrdering();
    //    c_window->arranging=true;
    }
    else{

        FILE *Result=fopen("tsneResult.raw","rb");

        maxValue=QVector2D(-1000,-1000);
        minValue=QVector2D(1000,1000);

        for(int g=0;g<c_window->dataset.length();g++){
            MitoDataset *curData=c_window->dataset[g];
            if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;


            curData->plotCoord.clear();

            for(int i=0;i<curData->features.length();i++){
                float t[2];
                fread(t,4,2,Result);
                curData->plotCoord.push_back(QVector2D(t[0],t[1]));
                if(t[0]>maxValue.x()){
                    maxValue.setX(t[0]);
                }
                if(t[1]>maxValue.y()){
                    maxValue.setY(t[1]);
                }
                if(t[0]<minValue.x()){
                    minValue.setX(t[0]);
                }
                if(t[1]<minValue.y()){
                    minValue.setY(t[1]);
                }
            }

            //c_window->preOrdering();
        //    c_window->arranging=true;
        }
        fclose(Result);

        for(int g=0;g<c_window->dataset.length();g++){
            MitoDataset *curData=c_window->dataset[g];
            if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;

            for(int i=0;i<curData->plotCoord.length();i++){
                curData->plotCoord[i].setX((curData->plotCoord[i].x()-minValue.x())/(maxValue.x()-minValue.x()));
                curData->plotCoord[i].setY((curData->plotCoord[i].y()-minValue.y())/(maxValue.y()-minValue.y()));
            }
        }
        c_window->synchronization();
        c_window->arrangeRanderPart();

    }

}

void tsneSelection::changedTrendPCAResult(){


}

void tsneSelection::changedTrendtSNEResult(){

}


void tsneSelection::changedSelectiveResult(){


}

tsneSelection::tsneSelection(Window *p, QWidget *parent)
    : QWidget(parent)
{


    IsSelectGroupMode=false;


    c_window=p;

    doSelect=false;


    TSNE_worker *tsneWorker=new TSNE_worker(c_window);
    tsneWorker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, tsneWorker, &QObject::deleteLater);
    connect(tsneWorker,SIGNAL(resultReady()),this,SLOT(changedResult()));
    connect(c_window,SIGNAL(projectionRun()),tsneWorker,SLOT(run()));

    workerThread.start();

    setBackgroundRole(QPalette::Base);
    QWidget::setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);

    x_start=new QLineEdit("0.0",this);
    x_start->setStyleSheet(QString("QLineEdit {font: Bold;color: rgb(50,168,82);background-color: rgba(255,255,255,100)}"));
    x_start->setAutoFillBackground(false);
    x_start->setFrame(0);

    x_end=new QLineEdit("1.0",this);
    x_end->setStyleSheet(QString("QLineEdit {font: Bold;color: rgb(50,168,82);background-color: rgba(255,255,255,100)}"));
    x_end->setAutoFillBackground(false);
    x_end->setFrame(0);


    y_start=new QLineEdit("0.0",this);
    y_start->setStyleSheet(QString("QLineEdit {font: Bold;color: rgb(50,168,82);background-color: rgba(255,255,255,100)}"));
    y_start->setAutoFillBackground(false);
    y_start->setFrame(0);


    y_end=new QLineEdit("1.0",this);
    y_end->setStyleSheet(QString("QLineEdit {font: Bold;color: rgb(50,168,82);background-color: rgba(255,255,255,100)}"));
    y_end->setAutoFillBackground(false);
    y_end->setFrame(0);

//    x_start->hide();




}

tsneSelection::~tsneSelection(){
    workerThread.quit();
    workerThread.wait();
}

void tsneSelection::resizeEvent(QResizeEvent *event){
    is_axis_edit=true;

//    int w=event->size().width();
//    int h=event->size().height();
//    int t=w>h?h:w;
//    this->resize(w,h);
}

QSize tsneSelection::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize tsneSelection::sizeHint() const
{
    return QSize(300, 300);
}

void tsneSelection::paintEvent(QPaintEvent *)
{
    qDebug()<<"start projection plot";


    if(c_window->curDataIndex==-1){
        graphStart=QVector2D(this->width()*0.15,this->height()*0.8);
        graphEnd=QVector2D(this->width()*0.9,this->height()*0.1);

        QPainter painter(this);
        painter.setRenderHint(QPainter::HighQualityAntialiasing,true);
        //     painter.drawPixmap(0,0,this->width(),this->height(),background);//resolution

        QPen t_pen;
        QBrush t_brush(Qt::SolidPattern);

        ////Draw frame
        ///
        ///
        t_pen.setColor(QColor(150,150,150));
        t_pen.setWidth(2*c_window->WINDOW_SCALE);
        painter.setPen(t_pen);
        painter.drawLine(this->width()*0.1,this->height()*0.05,this->width()*0.95,this->height()*0.05);
        painter.drawLine(this->width()*0.95,this->height()*0.05,this->width()*0.95,this->height()*0.85);
        painter.drawLine(this->width()*0.95,this->height()*0.85,this->width()*0.1,this->height()*0.85);
        painter.drawLine(this->width()*0.1,this->height()*0.85,this->width()*0.1,this->height()*0.05);

        t_pen.setColor(QColor(200,200,200));
        t_pen.setWidthF(c_window->WINDOW_SCALE);
        t_pen.setStyle(Qt::DashLine);
        painter.setPen(t_pen);
        for(int i=0;i<=10;i++){
            painter.drawLine(this->width()*0.1,graphStart.y()+(graphEnd.y()-graphStart.y())/10.0*i,this->width()*0.95,graphStart.y()+(graphEnd.y()-graphStart.y())/10.0*i);
        }
        for(int i=0;i<=10;i++){
            painter.drawLine(graphStart.x()+(graphEnd.x()-graphStart.x())/10.0*i,this->height()*0.05,graphStart.x()+(graphEnd.x()-graphStart.x())/10.0*i,this->height()*0.85);
        }
        return;
    }




    if(c_window->analysis_type->currentIndex()==0){

        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];


        float pointW=2*c_window->WINDOW_SCALE;

        if(c_window->plot_expand->isChecked()){
            graphStart=QVector2D(this->width()*0.15-curData->plotXaxisRange.x()/(curData->plotXaxisRange.y()-curData->plotXaxisRange.x())*this->width()*0.75,
                                 this->height()*0.8+curData->plotYaxisRange.x()/(curData->plotYaxisRange.y()-curData->plotYaxisRange.x())*this->height()*0.7);
            graphEnd=QVector2D(this->width()*0.9+(1-curData->plotXaxisRange.y())/(curData->plotXaxisRange.y()-curData->plotXaxisRange.x())*this->width()*0.75,
                               this->height()*0.1-(1-curData->plotYaxisRange.y())/(curData->plotYaxisRange.y()-curData->plotYaxisRange.x())*this->height()*0.7);
        }
        else{
            graphStart=QVector2D(this->width()*0.15,this->height()*0.8);
            graphEnd=QVector2D(this->width()*0.9,this->height()*0.1);
        }


        QPainter painter(this);
        painter.setRenderHint(QPainter::HighQualityAntialiasing,true);
        //     painter.drawPixmap(0,0,this->width(),this->height(),background);//resolution

        QPen t_pen;
        QBrush t_brush(Qt::SolidPattern);

        ////Draw frame
        ///
        ///
        t_pen.setColor(QColor(150,150,150));
        t_pen.setWidth(2*c_window->WINDOW_SCALE);
        painter.setPen(t_pen);
        painter.drawLine(this->width()*0.1,this->height()*0.05,this->width()*0.95,this->height()*0.05);
        painter.drawLine(this->width()*0.95,this->height()*0.05,this->width()*0.95,this->height()*0.85);
        painter.drawLine(this->width()*0.95,this->height()*0.85,this->width()*0.1,this->height()*0.85);
        painter.drawLine(this->width()*0.1,this->height()*0.85,this->width()*0.1,this->height()*0.05);

        t_pen.setColor(QColor(200,200,200));
        t_pen.setWidthF(c_window->WINDOW_SCALE);
        t_pen.setStyle(Qt::DashLine);
        painter.setPen(t_pen);
        for(int i=0;i<=10;i++){
            painter.drawLine(this->width()*0.1,graphStart.y()+(graphEnd.y()-graphStart.y())/10.0*i,this->width()*0.95,graphStart.y()+(graphEnd.y()-graphStart.y())/10.0*i);
        }
        for(int i=0;i<=10;i++){
            painter.drawLine(graphStart.x()+(graphEnd.x()-graphStart.x())/10.0*i,this->height()*0.05,graphStart.x()+(graphEnd.x()-graphStart.x())/10.0*i,this->height()*0.85);
        }



        painter.setPen(Qt::NoPen);

        if(curData->plotCoord.length()==curData->features.length()){


            if(curData->focusItem!=-1){
                for(int i=0;i<curData->features.length();i++){
                    t_brush.setColor(QColor(50,50,50,15));
                    painter.setBrush(t_brush);
                    painter.drawEllipse(QPointF(graphStart.x()+curData->plotCoord[i].x()*(graphEnd.x()-graphStart.x())
                                                ,graphStart.y()+curData->plotCoord[i].y()*(graphEnd.y()-graphStart.y())),5*c_window->WINDOW_SCALE,5*c_window->WINDOW_SCALE);
                }
                int i=curData->focusItem;
                if(i<curData->features.length()){
                    if(c_window->coloringTypeSet->currentIndex()==0)
                        t_brush.setColor(c_window->typeColors[int(curData->features[i][0])]);
                    else
                        t_brush.setColor(c_window->groups[curData->group]->color);
                    painter.setBrush(t_brush);
                    painter.drawEllipse(QPointF(graphStart.x()+curData->plotCoord[i].x()*(graphEnd.x()-graphStart.x())
                                                ,graphStart.y()+curData->plotCoord[i].y()*(graphEnd.y()-graphStart.y())),5*c_window->WINDOW_SCALE,5*c_window->WINDOW_SCALE);
                }


            }
            else{

                for(int i=0;i<curData->features.length();i++){
                    if(!curData->enabled[i]){
                        t_brush.setColor(QColor(50,50,50,15));
                        painter.setBrush(t_brush);
                        painter.drawEllipse(QPointF(graphStart.x()+curData->plotCoord[i].x()*(graphEnd.x()-graphStart.x())
                                                    ,graphStart.y()+curData->plotCoord[i].y()*(graphEnd.y()-graphStart.y())),5*c_window->WINDOW_SCALE,5*c_window->WINDOW_SCALE);
                    }
                }


                for(int i=0;i<curData->features.length();i++){
                    if(curData->enabled[i]){
                        if(c_window->coloringTypeSet->currentIndex()==0)
                            t_brush.setColor(c_window->typeColors[int(curData->features[i][0])]);
                        else
                            t_brush.setColor(c_window->groups[curData->group]->color);
                        painter.setBrush(t_brush);
                        painter.drawEllipse(QPointF(graphStart.x()+curData->plotCoord[i].x()*(graphEnd.x()-graphStart.x())
                                                    ,graphStart.y()+curData->plotCoord[i].y()*(graphEnd.y()-graphStart.y())),5*c_window->WINDOW_SCALE,5*c_window->WINDOW_SCALE);
                    }
                }
            }
        }

        //// draw axis bar
        ///
        t_brush.setColor(QColor(50,168,82));
        t_pen.setColor(QColor(50,168,82));
        t_pen.setWidth(2);
        t_pen.setStyle(Qt::DotLine);
        painter.setBrush(t_brush);
        painter.setPen(Qt::NoPen);

        QPointF button[3];
        QPointF p=QPointF(this->width()*0.1,graphStart.y() + (graphEnd.y()-graphStart.y())*curData->plotYaxisRange.x());
        QPointF p2=QPointF(this->width()*0.95,graphStart.y() + (graphEnd.y()-graphStart.y())*curData->plotYaxisRange.x());

        button[0]=p;
        button[1]=p+QPointF(-10,10);
        button[2]=p+QPointF(-10,-10);
        painter.drawConvexPolygon(button,3);

        painter.setPen(t_pen);
        painter.drawLine(p,p2);
        painter.setPen(Qt::NoPen);

        if(is_axis_edit){
            y_start->setGeometry(p.x()-50,p.y()-10,40,20);
            y_start->setText(QString::number(minValue.y()+(maxValue.y()-minValue.y())*curData->plotYaxisRange.x(),'f',2));
        }

        p=QPointF(this->width()*0.1,graphStart.y() + (graphEnd.y()-graphStart.y())*curData->plotYaxisRange.y());
        p2=QPointF(this->width()*0.95,graphStart.y() + (graphEnd.y()-graphStart.y())*curData->plotYaxisRange.y());
        button[0]=p;
        button[1]=p+QPointF(-10,10);
        button[2]=p+QPointF(-10,-10);
        painter.drawConvexPolygon(button,3);


        painter.setPen(t_pen);
        painter.drawLine(p,p2);
        painter.setPen(Qt::NoPen);

        if(is_axis_edit){
            y_end->setGeometry(p.x()-50,p.y()-10,40,20);
            y_end->setText(QString::number(minValue.y()+(maxValue.y()-minValue.y())*curData->plotYaxisRange.y(),'f',2));
        }

        p=QPointF(graphStart.x() + (graphEnd.x()-graphStart.x())*curData->plotXaxisRange.x(),this->height()*0.85);
        p2=QPointF(graphStart.x() + (graphEnd.x()-graphStart.x())*curData->plotXaxisRange.x(),this->height()*0.05);
        button[0]=p;
        button[1]=p+QPointF(-10,10);
        button[2]=p+QPointF(10,10);
        painter.drawConvexPolygon(button,3);

        painter.setPen(t_pen);
        painter.drawLine(p,p2);
        painter.setPen(Qt::NoPen);

        if(is_axis_edit){
            x_start->setGeometry(p.x()-20,p.y()+10,40,20);
            x_start->setText(QString::number(minValue.x()+(maxValue.x()-minValue.x())*curData->plotXaxisRange.x(),'f',2));
        }

        p=QPointF(graphStart.x() + (graphEnd.x()-graphStart.x())*curData->plotXaxisRange.y(),this->height()*0.85);
        p2=QPointF(graphStart.x() + (graphEnd.x()-graphStart.x())*curData->plotXaxisRange.y(),this->height()*0.05);
        button[0]=p;
        button[1]=p+QPointF(-10,10);
        button[2]=p+QPointF(10,10);
        painter.drawConvexPolygon(button,3);

        painter.setPen(t_pen);
        painter.drawLine(p,p2);
        painter.setPen(Qt::NoPen);

        if(is_axis_edit){
            x_end->setGeometry(p.x()-20,p.y()+10,40,20);
            x_end->setText(QString::number(minValue.x()+(maxValue.x()-minValue.x())*curData->plotXaxisRange.y(),'f',2));


            if(c_window->projection1->currentIndex()-2==0){
                if(curData->plotXaxisRange.x()==0){
                    x_start->setText("Dend");
                }
                else x_start->setText("");
                if(curData->plotXaxisRange.y()==1){
                    x_end->setText("Axon");
                }
                else x_end->setText("");
            }
            if(c_window->projection2->currentIndex()-2==0){
                if(curData->plotYaxisRange.x()==0){
                    y_start->setText("Dend");
                }
                else y_start->setText("");
                if(curData->plotYaxisRange.y()==1){
                    y_end->setText("Axon");
                }
                else y_end->setText("");
            }
        }
        is_axis_edit=false;

    }
    else{

//        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];


        float pointW=2*c_window->WINDOW_SCALE;

        if(c_window->plot_expand->isChecked()){
            graphStart=QVector2D(this->width()*0.15-c_window->globalPlotXaxisRange.x()/(c_window->globalPlotXaxisRange.y()-c_window->globalPlotXaxisRange.x())*this->width()*0.75,
                                 this->height()*0.8+c_window->globalPlotYaxisRange.x()/(c_window->globalPlotYaxisRange.y()-c_window->globalPlotYaxisRange.x())*this->height()*0.7);
            graphEnd=QVector2D(this->width()*0.9+(1-c_window->globalPlotXaxisRange.y())/(c_window->globalPlotXaxisRange.y()-c_window->globalPlotXaxisRange.x())*this->width()*0.75,
                               this->height()*0.1-(1-c_window->globalPlotYaxisRange.y())/(c_window->globalPlotYaxisRange.y()-c_window->globalPlotYaxisRange.x())*this->height()*0.7);
        }
        else{
            graphStart=QVector2D(this->width()*0.15,this->height()*0.8);
            graphEnd=QVector2D(this->width()*0.9,this->height()*0.1);
        }


        QPainter painter(this);
        painter.setRenderHint(QPainter::HighQualityAntialiasing,true);
        //     painter.drawPixmap(0,0,this->width(),this->height(),background);//resolution

        QPen t_pen;
        QBrush t_brush(Qt::SolidPattern);

        ////Draw frame
        ///
        ///
        t_pen.setColor(QColor(150,150,150));
        t_pen.setWidth(2*c_window->WINDOW_SCALE);
        painter.setPen(t_pen);
        painter.drawLine(this->width()*0.1,this->height()*0.05,this->width()*0.95,this->height()*0.05);
        painter.drawLine(this->width()*0.95,this->height()*0.05,this->width()*0.95,this->height()*0.85);
        painter.drawLine(this->width()*0.95,this->height()*0.85,this->width()*0.1,this->height()*0.85);
        painter.drawLine(this->width()*0.1,this->height()*0.85,this->width()*0.1,this->height()*0.05);

        t_pen.setColor(QColor(200,200,200));
        t_pen.setWidthF(c_window->WINDOW_SCALE);
        t_pen.setStyle(Qt::DashLine);
        painter.setPen(t_pen);
        for(int i=0;i<=10;i++){
            painter.drawLine(this->width()*0.1,graphStart.y()+(graphEnd.y()-graphStart.y())/10.0*i,this->width()*0.95,graphStart.y()+(graphEnd.y()-graphStart.y())/10.0*i);
        }
        for(int i=0;i<=10;i++){
            painter.drawLine(graphStart.x()+(graphEnd.x()-graphStart.x())/10.0*i,this->height()*0.05,graphStart.x()+(graphEnd.x()-graphStart.x())/10.0*i,this->height()*0.85);
        }



        painter.setPen(Qt::NoPen);

        if(c_window->focus_group!=-1){
            for(int g=0;g<c_window->dataset.length();g++){
                MitoDataset *curData=c_window->dataset[g];
                if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;
                for(int i=0;i<curData->features.length();i++){
                    t_brush.setColor(QColor(50,50,50,15));
                    painter.setBrush(t_brush);
                    painter.drawEllipse(QPointF(graphStart.x()+curData->plotCoord[i].x()*(graphEnd.x()-graphStart.x())
                                                ,graphStart.y()+curData->plotCoord[i].y()*(graphEnd.y()-graphStart.y())),5*c_window->WINDOW_SCALE,5*c_window->WINDOW_SCALE);
                }
            }
            MitoDataset *curData=c_window->dataset[c_window->focus_group];
            int i=curData->focusItem;
            if(i<curData->features.length()){
                if(c_window->coloringTypeSet->currentIndex()==0)
                    t_brush.setColor(c_window->typeColors[int(curData->features[i][0])]);
                else
                    t_brush.setColor(c_window->groups[curData->group]->color);
                painter.setBrush(t_brush);
                painter.drawEllipse(QPointF(graphStart.x()+curData->plotCoord[i].x()*(graphEnd.x()-graphStart.x())
                                            ,graphStart.y()+curData->plotCoord[i].y()*(graphEnd.y()-graphStart.y())),5*c_window->WINDOW_SCALE,5*c_window->WINDOW_SCALE);
            }
        }
        else{
            for(int g=0;g<c_window->dataset.length();g++){
                MitoDataset *curData=c_window->dataset[g];
                if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;

                for(int i=0;i<curData->features.length();i++){
                    if(!curData->globalEnabled[i]){
                        t_brush.setColor(QColor(50,50,50,15));
                        painter.setBrush(t_brush);
                        painter.drawEllipse(QPointF(graphStart.x()+curData->plotCoord[i].x()*(graphEnd.x()-graphStart.x())
                                                    ,graphStart.y()+curData->plotCoord[i].y()*(graphEnd.y()-graphStart.y())),5*c_window->WINDOW_SCALE,5*c_window->WINDOW_SCALE);
                    }
                }
            }
            for(int g=0;g<c_window->dataset.length();g++){
                MitoDataset *curData=c_window->dataset[g];
                if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;

                for(int i=0;i<curData->features.length();i++){
                    if(curData->globalEnabled[i]){
                        if(c_window->coloringTypeSet->currentIndex()==0)
                            t_brush.setColor(c_window->typeColors[int(curData->features[i][0])]);
                        else
                            t_brush.setColor(c_window->groups[curData->group]->color);
                        painter.setBrush(t_brush);
                        painter.drawEllipse(QPointF(graphStart.x()+curData->plotCoord[i].x()*(graphEnd.x()-graphStart.x())
                                                    ,graphStart.y()+curData->plotCoord[i].y()*(graphEnd.y()-graphStart.y())),5*c_window->WINDOW_SCALE,5*c_window->WINDOW_SCALE);
                    }
                }
            }

        }

        //// draw axis bar
        ///
        t_brush.setColor(QColor(50,168,82));
        t_pen.setColor(QColor(50,168,82));
        t_pen.setWidth(2);
        t_pen.setStyle(Qt::DotLine);
        painter.setBrush(t_brush);
        painter.setPen(Qt::NoPen);

        QPointF button[3];
        QPointF p=QPointF(this->width()*0.1,graphStart.y() + (graphEnd.y()-graphStart.y())*c_window->globalPlotYaxisRange.x());
        QPointF p2=QPointF(this->width()*0.95,graphStart.y() + (graphEnd.y()-graphStart.y())*c_window->globalPlotYaxisRange.x());

        button[0]=p;
        button[1]=p+QPointF(-10,10);
        button[2]=p+QPointF(-10,-10);
        painter.drawConvexPolygon(button,3);

        painter.setPen(t_pen);
        painter.drawLine(p,p2);
        painter.setPen(Qt::NoPen);

        if(is_axis_edit){
            y_start->setGeometry(p.x()-50,p.y()-10,40,20);
            y_start->setText(QString::number(minValue.y()+(maxValue.y()-minValue.y())*c_window->globalPlotYaxisRange.x(),'f',2));
        }

        p=QPointF(this->width()*0.1,graphStart.y() + (graphEnd.y()-graphStart.y())*c_window->globalPlotYaxisRange.y());
        p2=QPointF(this->width()*0.95,graphStart.y() + (graphEnd.y()-graphStart.y())*c_window->globalPlotYaxisRange.y());
        button[0]=p;
        button[1]=p+QPointF(-10,10);
        button[2]=p+QPointF(-10,-10);
        painter.drawConvexPolygon(button,3);


        painter.setPen(t_pen);
        painter.drawLine(p,p2);
        painter.setPen(Qt::NoPen);

        if(is_axis_edit){
            y_end->setGeometry(p.x()-50,p.y()-10,40,20);
            y_end->setText(QString::number(minValue.y()+(maxValue.y()-minValue.y())*c_window->globalPlotYaxisRange.y(),'f',2));
        }

        p=QPointF(graphStart.x() + (graphEnd.x()-graphStart.x())*c_window->globalPlotXaxisRange.x(),this->height()*0.85);
        p2=QPointF(graphStart.x() + (graphEnd.x()-graphStart.x())*c_window->globalPlotXaxisRange.x(),this->height()*0.05);
        button[0]=p;
        button[1]=p+QPointF(-10,10);
        button[2]=p+QPointF(10,10);
        painter.drawConvexPolygon(button,3);

        painter.setPen(t_pen);
        painter.drawLine(p,p2);
        painter.setPen(Qt::NoPen);

        if(is_axis_edit){
            x_start->setGeometry(p.x()-20,p.y()+10,40,20);
            x_start->setText(QString::number(minValue.x()+(maxValue.x()-minValue.x())*c_window->globalPlotXaxisRange.x(),'f',2));
        }

        p=QPointF(graphStart.x() + (graphEnd.x()-graphStart.x())*c_window->globalPlotXaxisRange.y(),this->height()*0.85);
        p2=QPointF(graphStart.x() + (graphEnd.x()-graphStart.x())*c_window->globalPlotXaxisRange.y(),this->height()*0.05);
        button[0]=p;
        button[1]=p+QPointF(-10,10);
        button[2]=p+QPointF(10,10);
        painter.drawConvexPolygon(button,3);

        painter.setPen(t_pen);
        painter.drawLine(p,p2);
        painter.setPen(Qt::NoPen);

        if(is_axis_edit){
            x_end->setGeometry(p.x()-20,p.y()+10,40,20);
            x_end->setText(QString::number(minValue.x()+(maxValue.x()-minValue.x())*c_window->globalPlotXaxisRange.y(),'f',2));

            if(c_window->projection1->currentIndex()-2==0){
                if(c_window->globalPlotXaxisRange.x()==0){
                    x_start->setText("Dend");
                }
                else x_start->setText("");
                if(c_window->globalPlotXaxisRange.y()==1){
                    x_end->setText("Axon");
                }
                else  x_end->setText("");
            }
            if(c_window->projection2->currentIndex()-2==0){
                if(c_window->globalPlotXaxisRange.x()==0){
                    y_start->setText("Dend");
                }
                else y_start->setText("");
                if(c_window->globalPlotXaxisRange.y()==1){
                    y_end->setText("Axon");
                }
                else y_end->setText("");
            }
        }
    }

}
void tsneSelection::makeCoord(){
    if(c_window->curDataIndex==-1)return;
    int axis1=c_window->projection1->currentIndex()-2;
    int axis2=c_window->projection2->currentIndex()-2;

    if(c_window->analysis_type->currentIndex()==0){
        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

        curData->axis1=axis1;
        curData->axis2=axis2;

        curData->plotCoord.clear();

        maxValue=QVector2D(-1000,-1000);
        minValue=QVector2D(1000,1000);

        for(int i=0;i<curData->features.length();i++){
            float t[2];

            t[0]=curData->features[i][axis1];
            t[1]=curData->features[i][axis2];

            curData->plotCoord.push_back(QVector2D(t[0],t[1]));
            if(t[0]>maxValue.x()){
                maxValue.setX(t[0]);
            }
            if(t[1]>maxValue.y()){
                maxValue.setY(t[1]);
            }
            if(t[0]<minValue.x()){
                minValue.setX(t[0]);
            }
            if(t[1]<minValue.y()){
                minValue.setY(t[1]);
            }
        }

        if(minValue.x()==maxValue.x()){
            minValue.setX(minValue.x()-1);
            maxValue.setX(maxValue.x()+1);
        }
        if(minValue.y()==maxValue.y()){
            minValue.setY(minValue.y()-1);
            maxValue.setY(maxValue.y()+1);
        }


        for(int i=0;i<curData->plotCoord.length();i++){
            curData->plotCoord[i].setX((curData->plotCoord[i].x()-minValue.x())/(maxValue.x()-minValue.x()));
            curData->plotCoord[i].setY((curData->plotCoord[i].y()-minValue.y())/(maxValue.y()-minValue.y()));
        }
        c_window->synchronization();
        //c_window->arrangeRanderPart();
    }
    else{

        maxValue=QVector2D(-1000,-1000);
        minValue=QVector2D(1000,1000);

        for(int g=0;g<c_window->dataset.length();g++){

            MitoDataset *curData=c_window->dataset[g];
            if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;

            curData->axis1=axis1;
            curData->axis2=axis2;

            curData->plotCoord.clear();


            for(int i=0;i<curData->features.length();i++){
                float t[2];

                t[0]=curData->features[i][axis1];
                t[1]=curData->features[i][axis2];

                curData->plotCoord.push_back(QVector2D(t[0],t[1]));
                if(t[0]>maxValue.x()){
                    maxValue.setX(t[0]);
                }
                if(t[1]>maxValue.y()){
                    maxValue.setY(t[1]);
                }
                if(t[0]<minValue.x()){
                    minValue.setX(t[0]);
                }
                if(t[1]<minValue.y()){
                    minValue.setY(t[1]);
                }
            }

            if(minValue.x()==maxValue.x()){
                minValue.setX(minValue.x()-1);
                maxValue.setX(maxValue.x()+1);
            }
            if(minValue.y()==maxValue.y()){
                minValue.setY(minValue.y()-1);
                maxValue.setY(maxValue.y()+1);
            }
            //c_window->arrangeRanderPart();
        }
        for(int g=0;g<c_window->dataset.length();g++){
            MitoDataset *curData=c_window->dataset[g];
            if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;

            for(int i=0;i<curData->plotCoord.length();i++){
                curData->plotCoord[i].setX((curData->plotCoord[i].x()-minValue.x())/(maxValue.x()-minValue.x()));
                curData->plotCoord[i].setY((curData->plotCoord[i].y()-minValue.y())/(maxValue.y()-minValue.y()));
            }
        }
        c_window->synchronization();
    }
}


void tsneSelection::mousePressEvent(QMouseEvent *event)
{
    if(c_window->curDataIndex==-1)return;

    if(event->x()<0 || event->x()>this->width() || event->y()<0 || event->y()>this->height())return;

    lastPos = event->pos();
    button=event->button();

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    clicked_thresh_bar=-1;
    if(event->button() ==Qt::LeftButton){
        if(c_window->analysis_type->currentIndex()==0){

            QPointF p=QPointF(this->width()*0.1,graphStart.y() + (graphEnd.y()-graphStart.y())*curData->plotYaxisRange.x());
            if(abs(p.x()-5-lastPos.x())<5 && abs(p.y()-lastPos.y())<10){
                clicked_thresh_bar=0;
            }

            p=QPointF(this->width()*0.1,graphStart.y() + (graphEnd.y()-graphStart.y())*curData->plotYaxisRange.y());
            if(abs(p.x()-5-lastPos.x())<5 && abs(p.y()-lastPos.y())<10){
                clicked_thresh_bar=1;
            }

            p=QPointF(graphStart.x()+ (graphEnd.x()-graphStart.x())*curData->plotXaxisRange.x(),this->height()*0.85);
            if(abs(p.x()-lastPos.x())<10 && abs(p.y()+5-lastPos.y())<5){
                clicked_thresh_bar=2;
            }

            p=QPointF(graphStart.x()+ (graphEnd.x()-graphStart.x())*curData->plotXaxisRange.y(),this->height()*0.85);
            if(abs(p.x()-lastPos.x())<10 && abs(p.y()+5-lastPos.y())<5){
                clicked_thresh_bar=3;
            }


            if(clicked_thresh_bar!=-1)
                c_window->dataset[c_window->curDataIndex]->focusItem=-1;
        }
        else{

            QPointF p=QPointF(this->width()*0.1,graphStart.y() + (graphEnd.y()-graphStart.y())*c_window->globalPlotYaxisRange.x());
            if(abs(p.x()-5-lastPos.x())<5 && abs(p.y()-lastPos.y())<10){
                clicked_thresh_bar=0;
            }

            p=QPointF(this->width()*0.1,graphStart.y() + (graphEnd.y()-graphStart.y())*c_window->globalPlotYaxisRange.y());
            if(abs(p.x()-5-lastPos.x())<5 && abs(p.y()-lastPos.y())<10){
                clicked_thresh_bar=1;
            }

            p=QPointF(graphStart.x()+ (graphEnd.x()-graphStart.x())*c_window->globalPlotXaxisRange.x(),this->height()*0.85);
            if(abs(p.x()-lastPos.x())<10 && abs(p.y()+5-lastPos.y())<5){
                clicked_thresh_bar=2;
            }

            p=QPointF(graphStart.x()+ (graphEnd.x()-graphStart.x())*c_window->globalPlotXaxisRange.y(),this->height()*0.85);
            if(abs(p.x()-lastPos.x())<10 && abs(p.y()+5-lastPos.y())<5){
                clicked_thresh_bar=3;
            }


            if(clicked_thresh_bar!=-1)
                c_window->focus_group=-1;

        }

    }
    update();
    c_window->synchronization();


}
void tsneSelection::mouseReleaseEvent(QMouseEvent *event){

    if(c_window->curDataIndex==-1)return;

    clicked_thresh_bar=-1;
    if(c_window->analysis_type->currentIndex()==0){

        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

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

                if(curData->focusItem!=-1){
                    Mito item=curData->mitoData[curData->focusItem];
                    QVector2D tP=QVector2D((item.xmax+item.xmin)/2.0,(item.ymax+item.ymin)/2.0);
                    curData->shift=QVector2D(curData->ImageW, curData->ImageH)/2.0-tP;
                    update();
                    c_window->synchronization();

                }
            }
        }

    }

}
void tsneSelection::mouseMoveEvent(QMouseEvent *event)
{
    if(c_window->curDataIndex==-1)return;

    if(c_window->analysis_type->currentIndex()==0){

        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

        setFocus();

        if(event->x()<0 || event->x()>this->width() || event->y()<0 || event->y()>this->height())return;

        float tx=event->x();
        float ty=event->y();

        if(clicked_thresh_bar!=-1){
            if(clicked_thresh_bar==0){
                float v=(event->y()-graphStart.y())/(graphEnd.y()-graphStart.y());
                curData->plotYaxisRange.setX(v);
                if(v<0)curData->plotYaxisRange.setX(0);
                if(v>1)curData->plotYaxisRange.setX(1);
            }
            else if(clicked_thresh_bar==1){
                float v=(event->y()-graphStart.y())/(graphEnd.y()-graphStart.y());
                curData->plotYaxisRange.setY(v);
                if(v<0)curData->plotYaxisRange.setY(0);
                if(v>1)curData->plotYaxisRange.setY(1);
            }
            else if(clicked_thresh_bar==2){
                float v=(event->x()-graphStart.x())/(graphEnd.x()-graphStart.x());
                curData->plotXaxisRange.setX(v);
                if(v<0)curData->plotXaxisRange.setX(0);
                if(v>1)curData->plotXaxisRange.setX(1);
            }
            else if(clicked_thresh_bar==3){
                float v=(event->x()-graphStart.x())/(graphEnd.x()-graphStart.x());
                curData->plotXaxisRange.setY(v);
                if(v<0)curData->plotXaxisRange.setY(0);
                if(v>1)curData->plotXaxisRange.setY(1);
            }

            is_axis_edit=true;
            curData->checkDataEnable(c_window);
            update();
            c_window->synchronization();
            return;
        }


        float minDis=10000;
        int prev=curData->focusItem;
        curData->focusItem=-1;

        for(int i=0;i<curData->features.length();i++){
            if(curData->enabled[i]==false){
                continue;
            }


            QPointF p=QPointF(graphStart.x()+curData->plotCoord[i].x()*(graphEnd.x()-graphStart.x())
                              ,graphStart.y()+curData->plotCoord[i].y()*(graphEnd.y()-graphStart.y()));

            float dis=sqrt((p.x()-tx)*(p.x()-tx)+(p.y()-ty)*(p.y()-ty));
            if(dis<5*c_window->WINDOW_SCALE){
                if(dis<minDis){
                    minDis=dis;
                    curData->focusItem=i;
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
//        MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

        setFocus();

        if(event->x()<0 || event->x()>this->width() || event->y()<0 || event->y()>this->height())return;

        float tx=event->x();
        float ty=event->y();

        if(clicked_thresh_bar!=-1){
            if(clicked_thresh_bar==0){
                float v=(event->y()-graphStart.y())/(graphEnd.y()-graphStart.y());
               c_window->globalPlotYaxisRange.setX(v);
                if(v<0)c_window->globalPlotYaxisRange.setX(0);
                if(v>1)c_window->globalPlotYaxisRange.setX(1);
            }
            else if(clicked_thresh_bar==1){
                float v=(event->y()-graphStart.y())/(graphEnd.y()-graphStart.y());
                c_window->globalPlotYaxisRange.setY(v);
                if(v<0)c_window->globalPlotYaxisRange.setY(0);
                if(v>1)c_window->globalPlotYaxisRange.setY(1);
            }
            else if(clicked_thresh_bar==2){
                float v=(event->x()-graphStart.x())/(graphEnd.x()-graphStart.x());
                c_window->globalPlotXaxisRange.setX(v);
                if(v<0)c_window->globalPlotXaxisRange.setX(0);
                if(v>1)c_window->globalPlotXaxisRange.setX(1);
            }
            else if(clicked_thresh_bar==3){
                float v=(event->x()-graphStart.x())/(graphEnd.x()-graphStart.x());
                c_window->globalPlotXaxisRange.setY(v);
                if(v<0)c_window->globalPlotXaxisRange.setY(0);
                if(v>1)c_window->globalPlotXaxisRange.setY(1);
            }
            for(int g=0;g<c_window->dataset.length();g++){
                MitoDataset *curData=c_window->dataset[g];
                if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;
                curData->checkDataEnable(c_window);
            }
            is_axis_edit=true;
            update();
            c_window->synchronization();
            return;
        }


        float minDis=10000;
        int prev=-1;
        int prev_group=-1;
        int new_focus_item=-1;
        if(c_window->focus_group!=-1){
            MitoDataset *curData=c_window->dataset[c_window->focus_group];
            prev=curData->focusItem;
            prev_group=c_window->focus_group;
            curData->focusItem=-1;
            c_window->focus_group=-1;
        }

        for(int g=0;g<c_window->dataset.length();g++){
            MitoDataset *curData=c_window->dataset[g];
            if(c_window->datagroupOnOff[curData->group]->isChecked()==false)continue;

            for(int i=0;i<curData->features.length();i++){
                if(curData->globalEnabled[i]==false){
                    continue;
                }


                QPointF p=QPointF(graphStart.x()+curData->plotCoord[i].x()*(graphEnd.x()-graphStart.x())
                                  ,graphStart.y()+curData->plotCoord[i].y()*(graphEnd.y()-graphStart.y()));

                float dis=sqrt((p.x()-tx)*(p.x()-tx)+(p.y()-ty)*(p.y()-ty));
                if(dis<5*c_window->WINDOW_SCALE){
                    if(dis<minDis){
                        minDis=dis;
                        new_focus_item=i;
                        c_window->focus_group=g;
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

void tsneSelection::keyPressEvent(QKeyEvent *event)
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

void tsneSelection::keyReleaseEvent(QKeyEvent *event)
{
//    if(event->key()==Qt::Key_Shift){
//        c_window->IsShift=false;
//    }
}

void tsneSelection::timerEvent(QTimerEvent *event){
}


void tsneSelection::readAnalysisFile(){


}
