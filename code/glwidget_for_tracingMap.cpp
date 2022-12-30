///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////


#include "glwidget_for_tracingMap.h"
#include <QMouseEvent>
#include <QCoreApplication>
#include <math.h>
#include <QProcess>
#include <QColorDialog>
#include <window.h>
#include <QVector2D>
#include <QPainter>
#include <QLineEdit>


QVector3D GlWidgetForTracingMap::cross_product(QVector3D v1,QVector3D v2){
    QVector3D v;
    v.setX(v1.y()*v2.z()-v2.y()*v1.z());
    v.setY(v1.z()*v2.x()-v2.z()*v1.x());
    v.setZ(v1.x()*v2.y()-v2.x()*v1.y());
    return v;
}

GlWidgetForTracingMap::GlWidgetForTracingMap(Window *p,  QWidget *parent)
    :  QWidget(parent)
{


    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);

    c_window=p;


    QWidget::setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true);

    startTimer(100);
}

GlWidgetForTracingMap::~GlWidgetForTracingMap()
{
}

QSize GlWidgetForTracingMap::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize GlWidgetForTracingMap::sizeHint() const
{
    return QSize(1600, 1600);
}



void GlWidgetForTracingMap::paintEvent(QPaintEvent *event)
{

    qDebug()<<"start tracing gl";

    if(c_window->curDataIndex==-1){

        return;
    }

    QPainter painter(this);

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    if(scale_changed || curData->doTracing){
        mapSimplification();
        scale_changed=false;
    }

    for(int i=0;i<selected_neurite.size();i++){
        int tP=selected_neurite[i];
        for(int j=0;j<curData->tracingMap_simplified[tP].size();j++){
            float startx=tP%curData->ImageW + 0.5;
            float starty=tP/curData->ImageW + 0.5;
            float endx=curData->tracingMap_simplified[tP][j]%curData->ImageW + 0.5;
            float endy=curData->tracingMap_simplified[tP][j]/curData->ImageW + 0.5;

            float p_screen_x_start=startx*scale+shift.x()*scale+(this->width()-curData->ImageW*scale)/2.0;
            float p_screen_y_start=starty*scale+shift.y()*scale+(this->height()-curData->ImageH*scale)/2.0;
            float p_screen_x_end=endx*scale+shift.x()*scale+(this->width()-curData->ImageW*scale)/2.0;
            float p_screen_y_end=endy*scale+shift.y()*scale+(this->height()-curData->ImageH*scale)/2.0;

            QPen tpen;
            tpen.setColor(QColor(255,255,0,128));
            tpen.setWidth(8);
            painter.setPen(tpen);
            painter.drawLine(p_screen_x_start,p_screen_y_start,p_screen_x_end,p_screen_y_end);

        }

    }

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            if(curData->tracingMap_simplified[tP].size()>0 && curData->tracingMap[tP].type!=7){
                for(int i=0;i<curData->tracingMap_simplified[tP].size();i++){

                    int s_label=curData->structureData_simplified[tP];
                    int e_label=curData->structureData_simplified[curData->tracingMap_simplified[tP][i]];

                    QColor s_color=c_window->typeColors[s_label];
                    QColor e_color=c_window->typeColors[e_label];

                    float startx=ix + 0.5;
                    float starty=iy + 0.5;
                    float endx=curData->tracingMap_simplified[tP][i]%curData->ImageW + 0.5;
                    float endy=curData->tracingMap_simplified[tP][i]/curData->ImageW + 0.5;

                    float p_screen_x_start=startx*scale+shift.x()*scale+(this->width()-curData->ImageW*scale)/2.0;
                    float p_screen_y_start=starty*scale+shift.y()*scale+(this->height()-curData->ImageH*scale)/2.0;
                    float p_screen_x_end=endx*scale+shift.x()*scale+(this->width()-curData->ImageW*scale)/2.0;
                    float p_screen_y_end=endy*scale+shift.y()*scale+(this->height()-curData->ImageH*scale)/2.0;

                    QLinearGradient gradient;
                    gradient.setStart(p_screen_x_start,p_screen_y_start);
                    gradient.setFinalStop(p_screen_x_end,p_screen_y_end);
                    gradient.setColorAt(0.0,s_color);
                    gradient.setColorAt(1.0, e_color);

                    QPen tpen;
                    tpen.setBrush(gradient);
                    tpen.setWidth(3);
                    painter.setPen(tpen);
                    painter.drawLine(p_screen_x_start,p_screen_y_start,p_screen_x_end,p_screen_y_end);

                }

            }
        }
    }


    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            if(curData->tracingMap[tP].type==7){
                    float p_screen_x=(ix+0.5)*scale+shift.x()*scale+(this->width()-curData->ImageW*scale)/2.0;
                    float p_screen_y=(iy+0.5)*scale+shift.y()*scale+(this->height()-curData->ImageH*scale)/2.0;
                    QBrush tbrush;
                    tbrush.setColor(c_window->typeColors[1]);
                    tbrush.setStyle(Qt::SolidPattern);
                    painter.setBrush(tbrush);
                    painter.setPen(Qt::NoPen);
                    painter.drawEllipse(p_screen_x-50*scale,p_screen_y-50*scale,100*scale,100*scale);

            }
            else if(curData->tracingMap_simplified[tP].size()>0){
                for(int i=0;i<curData->tracingMap_simplified[tP].size();i++){

                        int s_label=curData->structureData_simplified[tP];
                        int e_label=curData->structureData_simplified[curData->tracingMap_simplified[tP][i]];


                        float startx=ix + 0.5;
                        float starty=iy + 0.5;
                        float endx=curData->tracingMap_simplified[tP][i]%curData->ImageW + 0.5;
                        float endy=curData->tracingMap_simplified[tP][i]/curData->ImageW + 0.5;

                        float p_screen_x_start=startx*scale+shift.x()*scale+(this->width()-curData->ImageW*scale)/2.0;
                        float p_screen_y_start=starty*scale+shift.y()*scale+(this->height()-curData->ImageH*scale)/2.0;
                        float p_screen_x_end=endx*scale+shift.x()*scale+(this->width()-curData->ImageW*scale)/2.0;
                        float p_screen_y_end=endy*scale+shift.y()*scale+(this->height()-curData->ImageH*scale)/2.0;

                        if(s_label!=e_label){
                            float p_screen_x=(p_screen_x_start+p_screen_x_end)/2.0;
                            float p_screen_y=(p_screen_y_start+p_screen_y_end)/2.0;

                            QPen tpen;
                            tpen.setColor(QColor(255,0,100,128));
                            tpen.setWidth(8);
                            painter.setPen(tpen);
                            painter.drawPoint(p_screen_x,p_screen_y);

                        }
                    }
//                }

                if(curData->tracingMap_simplified[tP].size()>2){
                    float p_screen_x=(ix+0.5)*scale+shift.x()*scale+(this->width()-curData->ImageW*scale)/2.0;
                    float p_screen_y=(iy+0.5)*scale+shift.y()*scale+(this->height()-curData->ImageH*scale)/2.0;

                    QPen tpen;
                    tpen.setColor(QColor(50,180,150,128));
                    tpen.setWidth(8);
                    painter.setPen(tpen);
                    painter.drawPoint(p_screen_x,p_screen_y);

                }
            }
        }
    }

    painter.end();



}

void GlWidgetForTracingMap::mapSimplification(){
    if(c_window->curDataIndex==-1){

        return;
    }

    qDebug()<<"simplification start";

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

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

    float dis=1.5;

    for(int i=0;i<10;i++){
        bool is_changed=mapSimplification_sub(dis);
        dis+=1.5;
    }

//    while(1){
//        qDebug()<<"simplification sub";
//        bool is_changed=mapSimplification_sub(dis);
//        if(is_changed==false)break;
//    }



}
bool GlWidgetForTracingMap::mapSimplification_sub(float distance_thresh){

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

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
void GlWidgetForTracingMap::node_combine(int v0, int v1){
    if(v0==v1)return;
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];


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

void GlWidgetForTracingMap::node_move(int v0, int v1){
    if(v0==v1)return;
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

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

void GlWidgetForTracingMap::resizeEvent(QResizeEvent *event)
{
}

void GlWidgetForTracingMap::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
    mousePressedPos=event->pos();

    if(event->buttons() & Qt::MidButton){

    }

    if(c_window->curDataIndex!=-1){
    }
}

void GlWidgetForTracingMap::mouseReleaseEvent(QMouseEvent *event)
{
    if(c_window->curDataIndex==-1){

        return;
    }
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
    if((event->pos()-mousePressedPos).manhattanLength()<3 && event->button()==Qt::LeftButton){
        int p_image_x=(event->pos().x()-shift.x()*scale-(this->width()-curData->ImageW*scale)/2.0)/scale;
        int p_image_y=(event->pos().y()-shift.y()*scale-(this->height()-curData->ImageH*scale)/2.0)/scale;
        curData->shift=QVector2D(curData->ImageW*0.5-p_image_x,curData->ImageH*0.5-p_image_y);
        curData->scale=3;

    }

    update();

}
void GlWidgetForTracingMap::mouseMoveEvent(QMouseEvent *event)
{
    setFocus();

    isFocused=true;

    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();


    if(c_window->curDataIndex==-1){
        m_lastPos=event->pos();
        return;
    }
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    int p_image_x=(event->pos().x()-shift.x()*scale-(this->width()-curData->ImageW*scale)/2.0)/scale;
    int p_image_y=(event->pos().y()-shift.y()*scale-(this->height()-curData->ImageH*scale)/2.0)/scale;


    int nearest_node=find_nearest_node(p_image_x,p_image_y);
    if(nearest_node!=-1){
        curData->selected_neurite_on_minimap.clear();
        selected_neurite.clear();
        get_neurite(nearest_node,&curData->selected_neurite_on_minimap);
    }
    else{
        curData->selected_neurite_on_minimap.clear();
        selected_neurite.clear();
    }


    if (event->buttons() & Qt::RightButton) {
        if(scale>0.02){
            float t=1.0*(-dy)/300*scale;
            scale+=t;
        }
        else{
            scale=0.021;
        }
        scale_changed=true;
    }
    else if (event->buttons() & Qt::LeftButton ) {
        shift+=QVector2D(dx,dy)/scale;
    }

    update();
    m_lastPos=event->pos();

}
void GlWidgetForTracingMap::get_neurite(int p, QVector<int> *res){
    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];
    for(int i=0;i<curData->tracingMap_simplified_collections[p].size();i++){
        res->push_back(curData->tracingMap_simplified_collections[p][i]);
    }
    selected_neurite.push_back(p);
    bool *ck=new bool[curData->ImageW*curData->ImageH]();
    ck[p]=true;

    QQueue<int> nodes;
    nodes.enqueue(p);
    while(!nodes.isEmpty()){
        int tP=nodes.dequeue();
        for(int i=0;i<curData->tracingMap_simplified[tP].size();i++){
            int cP=curData->tracingMap_simplified[tP][i];
            if(ck[cP]==false && curData->tracingMap_simplified[cP].size()<=2){
                ck[cP]=true;
                nodes.enqueue(cP);
                for(int j=0;j<curData->tracingMap_simplified_collections[cP].size();j++){
                    res->push_back(curData->tracingMap_simplified_collections[cP][j]);
                }
                selected_neurite.push_back(cP);
            }
        }
    }
    delete []ck;


}
int GlWidgetForTracingMap::find_nearest_node(int px, int py){

    int res_node=-1;
    float min_dis=20;

    MitoDataset *curData=c_window->dataset[c_window->curDataIndex];

    float p_screen_x=(px+0.5)*scale+shift.x()*scale+(this->width()-curData->ImageW*scale)/2.0;
    float p_screen_y=(py+0.5)*scale+shift.y()*scale+(this->height()-curData->ImageH*scale)/2.0;


    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            if(curData->tracingMap_simplified[tP].size()==2 && curData->tracingMap[tP].type!=7){
                float i_screen_x=(ix+0.5)*scale+shift.x()*scale+(this->width()-curData->ImageW*scale)/2.0;
                float i_screen_y=(iy+0.5)*scale+shift.y()*scale+(this->height()-curData->ImageH*scale)/2.0;

                float cur_dis=sqrt((i_screen_x-p_screen_x)*(i_screen_x-p_screen_x)+(i_screen_y-p_screen_y)*(i_screen_y-p_screen_y));
                if(cur_dis<min_dis){
                    min_dis=cur_dis;
                    res_node=tP;
                }
            }
        }
    }
    return res_node;

}

void GlWidgetForTracingMap::wheelEvent(QWheelEvent *event)
{
    isFocused=true;
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numDegrees.isNull()) {
        QPoint numSteps = numDegrees / 15;
        if(c_window->curDataIndex!=-1){
        }

    }
    event->accept();
}

void GlWidgetForTracingMap::timerEvent(QTimerEvent *event){

    update();
}
void GlWidgetForTracingMap::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Space){
        IsSpacePressed=true;
    }
}
void GlWidgetForTracingMap::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Space){
        IsSpacePressed=false;
    }
}

