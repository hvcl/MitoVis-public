
#include "patchrecommender.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>
#include <QPainter>
#include <QtGui>
#include <QDockWidget>
#include <QColorDialog>

#include <iostream>
#include <QDebug>
#include <QCheckBox>



patchRecommender::patchRecommender(Window*p, QWidget *parent){
    c_window=p;
}
patchRecommender::~patchRecommender(){

}

void patchRecommender::init(){
    qDebug()<<"recommender init for dataset: "<<c_window->curDataIndex;
    curData=c_window->dataset[c_window->curDataIndex];
//    curData->tracingMap_neurite_based=new tracingNode_neurite_based[curData->ImageW * curData->ImageH];
//    generateNeurites();

    getCandidatePoints();

}

void patchRecommender::getCandidatePoints(){

    int axonN=0;
    int dendN=0;
    while(1){
        if(axonN>=30 && dendN>=30)break;

        int sx,sy,tP;
        sx=float(rand())/RAND_MAX*(curData->ImageW-40)+20;
        sy=float(rand())/RAND_MAX*(curData->ImageH-40)+20;
        tP=sy*curData->ImageW+sx;
        if(axonN>=30){
            while(curData->structureData[tP]!=2){
                sx=float(rand())/RAND_MAX*(curData->ImageW-40)+20;
                sy=float(rand())/RAND_MAX*(curData->ImageH-40)+20;
                tP=sy*curData->ImageW+sx;
            }

        }
        else if(dendN>=30){
            while(curData->structureData[tP]!=3){
                sx=float(rand())/RAND_MAX*(curData->ImageW-40)+20;
                sy=float(rand())/RAND_MAX*(curData->ImageH-40)+20;
                tP=sy*curData->ImageW+sx;
            }
        }
        else{
            while(curData->structureData[tP]!=2 && curData->structureData[tP]!=3){
                sx=float(rand())/RAND_MAX*(curData->ImageW-40)+20;
                sy=float(rand())/RAND_MAX*(curData->ImageH-40)+20;
                tP=sy*curData->ImageW+sx;
            }
        }

        if(curData->structureData[tP]==2){
            candidateStartPos_dend.push_back(QVector2D(sx-16,sy-16));
            candidateEndPos_dend.push_back(QVector2D(sx+16,sy+16));
            dendN++;
        }
        if(curData->structureData[tP]==3){
            candidateStartPos_axon.push_back(QVector2D(sx-16,sy-16));
            candidateEndPos_axon.push_back(QVector2D(sx+16,sy+16));
            axonN++;
        }
    }

}

void patchRecommender::run(int type){

    qDebug()<<"recommender run";

    if(type==0){ //axon -> dendrite
        int cnt=0;
        while(cnt<3){
            if(candidateStartPos_axon.size()==0)return;
            emit addNewPatch(candidateStartPos_axon[0],candidateEndPos_axon[0]);
            candidateStartPos_axon.pop_front();
            candidateEndPos_axon.pop_front();
            cnt++;
        }
    }
    else{ //dendrite -> axon
        int cnt=0;
        while(cnt<3){
            if(candidateStartPos_dend.size()==0)return;
            emit addNewPatch(candidateStartPos_dend[0],candidateEndPos_dend[0]);
            candidateStartPos_dend.pop_front();
            candidateEndPos_dend.pop_front();
            cnt++;
        }
    }
    qDebug()<<"patch generated";
}


//void patchRecommender::generateNeurites(){

//    bool *ck=new bool[curData->ImageW*curData->ImageH];
//    for(int i=0;i<curData->ImageW*curData->ImageH;i++){
//        ck[i]=false;
//    }

//    for(int iy=0;iy<curData->ImageH;iy++){
//        for(int ix=0;ix<curData->ImageW;ix++){
//            int p=iy*curData->ImageW+ix;
//            if(curData->tracingMap_neurite_based[p].patch==NULL && (curData->structureData[p]==2 || curData->structureData[p]==3)){
//                int mp=getMiddlePoint(ix,iy,ck);
//                QVector<patchSet*> neurite;
//                neurite=getNeuritesByTracing(mp,-1);

//                c_window->neurites.push_back(neurite);
//            }
//        }
//    }
//    delete []ck;
//}

//QVector<patchSet*> patchRecommender::getNeuritesByTracing(int startPos){
//    int neurite_index=c_window->neurites.size();
//    QQueue <int> tracingQueue;
//    QQueue <int> prevNodeQueue;
//    tracingQueue.enqueue(startPos);
//    prevNodeQueue.enqueue(startPos);

//    while(!tracingQueue.empty()){
//        int prevP=prevNodeQueue.dequeue();

//        int curP=tracingQueue.dequeue();
//        int curx=curP%curData->ImageW;
//        int cury=curP/curData->ImageW;

//        patchSet *newPatch=getPatch(curx,cury,-1);

//        curData->tracingMap_neurite_based[curP].patch=newPatch;
//        curData->tracingMap_neurite_based[curP].neurite_index=neurite_index;

//        QQueue <int> tempQueue;
//        findNextNodes(curP,&tempQueue);



//        if(nodeType==-1 || nodeType==6 || curP==startPos){
//            QQueue <int> tempQueue;
//            findNextNodes(curP,&tempQueue);

//            int connected_size=tracingMap[curP].connected_nodes.size();
//            if(connected_size==0 ||connected_size==1)
//                tracingMap[curP].type=4;
//            else if(connected_size==2)
//                tracingMap[curP].type=1;
//            else if(connected_size>2)
//                tracingMap[curP].type=2;

//            if(tracingMap[curP].type==1 || tracingMap[curP].type==4 || curP==startPos || true){
//                while(!tempQueue.empty()){
//                    int t=tempQueue.dequeue();
//                    tracingQueue.enqueue(t);
//                    prevNodeQueue.enqueue(curP);

//                }
//                if(connected_size==1 || connected_size==0){
//                    for(int i=0;i<2-connected_size;i++){

//                        int nextP=findNextNodeForWeakSignal(curP,prevP);
//                        if(nextP!=-1){
//                            is_weak_path_updated=true;
//                            if(tracingMap[nextP].type==-1){
//                                bool okay=true;
//                                for(int k=0;k<tracingMap[curP].connected_nodes.size();k++){
//                                    if(tracingMap[curP].connected_nodes[k]==nextP)okay=false;
//                                }
//                                if(okay){
//                                    tracingMap[curP].connected_nodes.push_back(nextP);
//                                    tracingMap[nextP].connected_nodes.push_back(curP);
//                                    int connected_size=tracingMap[curP].connected_nodes.size();
//                                    if(connected_size==0 ||connected_size==1)
//                                        tracingMap[curP].type=4;
//                                    else if(connected_size==2)
//                                        tracingMap[curP].type=1;
//                                    else if(connected_size>2)
//                                        tracingMap[curP].type=2;
//                                }
//                                prevP=nextP;
//                                tracingQueue.enqueue(nextP);
//                                prevNodeQueue.enqueue(curP);

//                            }
//                            else{
//                                bool okay=true;
//                                for(int k=0;k<tracingMap[curP].connected_nodes.size();k++){
//                                    if(tracingMap[curP].connected_nodes[k]==nextP)okay=false;
//                                }
//                                if(okay){
//                                    tracingMap[curP].connected_nodes.push_back(nextP);
//                                    tracingMap[nextP].connected_nodes.push_back(curP);
//                                    tracingMap[curP].type=1;
//                                    int connected_size=tracingMap[nextP].connected_nodes.size();
//                                    if(connected_size==0 ||connected_size==1)
//                                        tracingMap[nextP].type=4;
//                                    else if(connected_size==2)
//                                        tracingMap[nextP].type=1;
//                                    else if(connected_size>2)
//                                        tracingMap[nextP].type=2;
//                                }
//                            }
//                        }
//                    }

//                }

//            }
//            else{
//                QQueue <int> tempQueue2=tempQueue;
//                float min_angle=20;
//                int min_index=-1;
//                QVector2D dir(curx - prevP%curData->ImageW,
//                              cury - prevP/curData->ImageW);

//                while(!tempQueue2.empty()){
//                    int t=tempQueue2.dequeue();

//                    QVector2D newDir(t%curData->ImageW-curx,
//                            t/curData->ImageW-cury);

//                    float angle=acos((dir.x()*newDir.x()+dir.y()*newDir.y())/(dir.length()*newDir.length()))*180/M_PI;
//                    if(min_angle>angle){
//                        min_angle=angle;
//                        min_index=t;
//                    }
//                }
////                qDebug()<<"min angle: "<<min_angle;


//                while(!tempQueue.empty()){
//                    int t=tempQueue.dequeue();

//                    if(t==min_index){
//                        tracingQueue.enqueue(t);
//                        prevNodeQueue.enqueue(curP);
//                        continue;
//                    }


//                    patchSet *newPatch=getPatch(t%curData->ImageW,t/curData->ImageW,-1);
////                    if(!newPatch->is_valid){
////                        delete newPatch;
////                        continue;
////                    }
//                    tracingMap[t].patch=newPatch;
//                    tracingMap[t].distanceFromNode=0;
//                    tracingMap[t].posx=t%curData->ImageW;
//                    tracingMap[t].posy=t/curData->ImageW;
//                    tracingMap[t].type=6;

//                    updateTracingMap(t);

//                }

//            }

//        }
//        else if(nodeType==0){
//            for(int k=tracingMap[curP].connected_nodes.size()-1;k>=0;k--){
//                for(int kk=tracingMap[tracingMap[curP].connected_nodes[k]].connected_nodes.size()-1;kk>=0;kk--){
//                    if(tracingMap[tracingMap[curP].connected_nodes[k]].connected_nodes[kk]==curP){
//                        tracingMap[tracingMap[curP].connected_nodes[k]].connected_nodes.remove(kk);
//                    }
//                }
//                tracingMap[curP].connected_nodes.remove(k);
//            }
//        }

//    }

//    if(is_weak_path_updated){
//        curData->connectionStructureUpdate();
//        emit connection_graph_updated();
//    }
//}


//int patchRecommender::getMiddlePoint(int sx,int sy,bool *ck){

//    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

//    int cnt=1;
//    QVector2D midPoint=QVector2D(sx,sy);
//    QQueue<int> quex,quey;
//    quex.enqueue(sx);
//    quey.enqueue(sy);
//    QQueue<int> quex2,quey2;
//    quex2.enqueue(sx);
//    quey2.enqueue(sy);

//    ck[sy*curData->ImageW+sx]=true;

//    while(!quex.empty()){
//        int curx=quex.dequeue();
//        int cury=quey.dequeue();

//        for(int d=0;d<8;d++){
//            if(curx+dxylist[d][0]<=0 || curx+dxylist[d][0]>=curData->ImageW-1 || cury+dxylist[d][1]<=0 || cury+dxylist[d][1]>=curData->ImageH-1){
//                continue;
//            }
//            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
//            if(ck[tP]==false){
//                ck[tP]=true;
//                if((curData->structureData[tP]==2 || curData->structureData[tP]==3)
//                        && abs(cury+dxylist[d][1]-sy)<halfpatchSize && abs(curx+dxylist[d][0]-sx)<halfpatchSize){
//                    quex.enqueue(curx+dxylist[d][0]);
//                    quey.enqueue(cury+dxylist[d][1]);
//                    quex2.enqueue(curx+dxylist[d][0]);
//                    quey2.enqueue(cury+dxylist[d][1]);

//                    midPoint+=QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]);
//                    cnt++;
//                }
//            }
//        }
//    }
//    midPoint=midPoint/cnt;
//    int curx=quex2.dequeue();
//    int cury=quey2.dequeue();
//    int tP=cury*curData->ImageW + curx;
//    ck[tP]=false;
//    QVector2D medianPoint=QVector2D(curx,cury);
//    float minDis=(medianPoint-midPoint).length();
//    while(!quex2.empty()){
//        curx=quex2.dequeue();
//        cury=quey2.dequeue();
//        int tP=cury*curData->ImageW + curx;
//        ck[tP]=false;
//        if(minDis>(QVector2D(curx,cury)-midPoint).length()){
//            medianPoint=QVector2D(curx,cury);
//            minDis=(medianPoint-midPoint).length();
//        }
//    }

//    return medianPoint.y()*curData->ImageW + medianPoint.x();

//}


//patchSet *patchRecommender::getPatch(int sx,int sy,int l){

//    patchSet* patch=new patchSet();

//    structure S=getStructure(sx,sy,l,patchSize*patchSize,curData->structureData,false);

//    QVector <QVector2D> vertexs;
//    QVector <QVector2D> boundaries;
//    QVector <float> deep;
//    QVector <int> deepIndex;

//    QVector2D midPoint(0,0);
//    int startPointIndex=0;
//    int endPointIndex=0;

//    for(int iy=0;iy<S.height;iy++){
//        for(int ix=0;ix<S.width;ix++){
//            int tP=iy*S.width+ix;
//            if(S.data[tP]!=0){
//                if(iy==0 || iy==S.height-1 || ix==0 || ix==S.width-1){
//                    boundaries.push_back(QVector2D(ix,iy));
//                }
//                else{
//                    if(S.data[tP-1]==0 ||
//                        S.data[tP+1]==0 ||
//                        S.data[tP-S.width]==0 ||
//                        S.data[tP+S.width]==0){
//                        boundaries.push_back(QVector2D(ix,iy));
//                    }
//                    else{
//                        vertexs.push_back(QVector2D(ix,iy));
//                    }
//                }
//            }
//        }
//    }


//    for(int i=0;i<vertexs.length();i++){
//        deep.push_back(10000);
//        deepIndex.push_back(-1);
//        for(int j=0;j<boundaries.length();j++){
//            float dis=(vertexs[i]-boundaries[j]).length();
//            if(dis<deep[i]){
//                deep[i]=dis;
//                deepIndex[i]=j;
//            }
//        }
//    }

//    for(int i=0;i<vertexs.length();i++){
//        midPoint+=vertexs[i];
//    }
//    for(int i=0;i<boundaries.length();i++){
//        midPoint+=boundaries[i];
//    }
//    midPoint/=vertexs.length()+boundaries.length();

//    float maxDis=-1;
//    for(int i=0;i<boundaries.length();i++){
//        float dis=(boundaries[i]-midPoint).length();
//        if(dis>maxDis){
//            maxDis=dis;
//            startPointIndex=i;
//        }
//    }

//    maxDis=-1;
//    for(int i=0;i<boundaries.length();i++){
//        float dis=(boundaries[i]-boundaries[startPointIndex]).length();
//        if(dis>maxDis){
//            maxDis=dis;
//            endPointIndex=i;
//        }
//    }


//    if(startPointIndex==endPointIndex){
//        delete []S.data;
//        return patch;
//    }


//    QVector3D axis=QVector3D::crossProduct(QVector3D(boundaries[startPointIndex]-boundaries[endPointIndex],0),QVector3D(0,1,0));
//    axis.normalize();


//    float a=(boundaries[startPointIndex]-boundaries[endPointIndex]+QVector2D(0,1)).length();
//    float b=(boundaries[startPointIndex]-boundaries[endPointIndex]).length();
//    float c=1;
//    float angle=acos((b*b+1-a*a)/(2*b));
//    if(b==0)angle=0;

//    //qDebug()<<axis.x()<<axis.y()<<axis.z();

//    angle*=axis.z();

//    if(axis.z()==0)angle=0;

//    //qDebug()<<angle;

//    QVector <forSort3> rotated_vertexs;
//    float minY=1000;
//    for(int i=0;i<boundaries.length();i++){
//        forSort3 t;
//        t.value=QVector2D(boundaries[i].x()*cos(angle)-boundaries[i].y()*sin(angle),
//                          boundaries[i].y()*cos(angle)+boundaries[i].x()*sin(angle));
//        t.yvalue=t.value.y();
//        if(minY>t.yvalue)minY=t.yvalue;
//        t.deep=0;
//        t.ind1=i;
//        rotated_vertexs.push_back(t);
//    }
//    for(int i=0;i<vertexs.length();i++){
//        forSort3 t;
//        t.value=QVector2D(vertexs[i].x()*cos(angle)-vertexs[i].y()*sin(angle),
//                          vertexs[i].y()*cos(angle)+vertexs[i].x()*sin(angle));
//        t.yvalue=t.value.y();
//        if(minY>t.yvalue)minY=t.yvalue;
//        t.deep=deep[i];
//        t.ind2=i;
//        rotated_vertexs.push_back(t);
//    }
//    qSort(rotated_vertexs);

//    QVector2D O_targetP=QVector2D(sx-S.minx,sy-S.miny);


//    QVector2D targetP=QVector2D(O_targetP.x()*cos(angle)-O_targetP.y()*sin(angle),
//                                                O_targetP.y()*cos(angle)+O_targetP.x()*sin(angle));
//    QVector2D P1,P2;

//    float maxDeep=-1;
//    int ind=0;

//    for(int i=0;i<rotated_vertexs.length();i++){
//        if(rotated_vertexs[i].yvalue>targetP.y()-10 && rotated_vertexs[i].yvalue<targetP.y()-5){
//            if(rotated_vertexs[i].deep>maxDeep){
//                maxDeep=rotated_vertexs[i].deep;
//                ind=i;
//            }
//        }
//    }
//    if(rotated_vertexs[ind].ind1==-1){
//        P1=vertexs[rotated_vertexs[ind].ind2];
//    }
//    else{
//        P1=boundaries[rotated_vertexs[ind].ind1];
//    }

//    maxDeep=-1;
//    ind=0;

//    for(int i=0;i<rotated_vertexs.length();i++){
//        if(rotated_vertexs[i].yvalue<targetP.y()+10 && rotated_vertexs[i].yvalue>targetP.y()+5){
//            if(rotated_vertexs[i].deep>maxDeep){
//                maxDeep=rotated_vertexs[i].deep;
//                ind=i;
//            }
//        }
//    }
//    if(rotated_vertexs[ind].ind1==-1){
//        P2=vertexs[rotated_vertexs[ind].ind2];
//    }
//    else{
//        P2=boundaries[rotated_vertexs[ind].ind1];
//    }



//    {
//        QVector2D direction=(O_targetP-P1).normalized()*0.5+(P2-O_targetP).normalized()*0.5;

//        QVector3D axis=QVector3D::crossProduct(QVector3D(0,1,0),QVector3D(direction.x(),direction.y(),0));
//        axis.normalize();

//        float a=(QVector2D(0,1)-direction).length();
//        float angle=acos((2-a*a)/2);

//        //qDebug()<<axis.x()<<axis.y()<<axis.z();
//        angle*=axis.z();
//        if(axis.z()==0)angle=0;

//        for(int iy=0;iy<patchSize;iy++){
//            for(int ix=0;ix<patchSize;ix++){
//                int px=(ix-halfpatchSize)*cos(angle)-(iy-halfpatchSize)*sin(angle)+O_targetP.x();
//                int py=(iy-halfpatchSize)*cos(angle)+(ix-halfpatchSize)*sin(angle)+O_targetP.y();
//                if(px>=0 && px<S.width && py>=0 && py<S.height){
//                    patch->patch[iy*patchSize+ix]=float(S.data[py*S.width+px])/65535;

//                    if(patch->label_type==0)patch->label_type=S.label[py*S.width+px];
//                    else if(patch->label_type==2 && S.label[py*S.width+px]==3)patch->label_type=4;
//                    else if(patch->label_type==3 && S.label[py*S.width+px]==2)patch->label_type=4;

//                }
//            }
//        }
//        for(int iy=0;iy<patchSize;iy++){
//            for(int ix=0;ix<patchSize;ix++){
//                int px=(ix-halfpatchSize)*cos(angle)-(iy-halfpatchSize)*sin(angle)+sx;
//                int py=(iy-halfpatchSize)*cos(angle)+(ix-halfpatchSize)*sin(angle)+sy;
//                if(px>=0 && px<curData->ImageW && py>=0 && py<curData->ImageH){
//                    if(px<patch->boundMin.x())patch->boundMin.setX(px);
//                    if(px>patch->boundMax.x())patch->boundMax.setX(px);
//                    if(py<patch->boundMin.y())patch->boundMin.setY(py);
//                    if(py>patch->boundMax.y())patch->boundMax.setY(py);

//                }
//            }
//        }

//        patch->patchCorner[0]=QVector2D((-halfpatchSize)*cos(angle)-(-halfpatchSize)*sin(angle)+sx,
//                                 (-halfpatchSize)*cos(angle)+(-halfpatchSize)*sin(angle)+sy);

//        patch->patchCorner[1]=QVector2D((-halfpatchSize)*cos(angle)-(halfpatchSize)*sin(angle)+sx,
//                                 (halfpatchSize)*cos(angle)+(-halfpatchSize)*sin(angle)+sy);

//        patch->patchCorner[2]=QVector2D((halfpatchSize)*cos(angle)-(halfpatchSize)*sin(angle)+sx,
//                                 (halfpatchSize)*cos(angle)+(halfpatchSize)*sin(angle)+sy);

//        patch->patchCorner[3]=QVector2D((halfpatchSize)*cos(angle)-(-halfpatchSize)*sin(angle)+sx,
//                                 (-halfpatchSize)*cos(angle)+(halfpatchSize)*sin(angle)+sy);

//        patch->angle=angle;
//        patch->midx=sx;
//        patch->midy=sy;

//    }

//    delete []S.data;

//    patch->is_valid=true;

//    return patch;
//}



//structure patchRecommender::getStructure(int sx, int sy, int l, int maxPNum, unsigned char*structureData, bool eraseFlag){  //l: -1 for all label

//    bool *ck=new bool[curData->ImageW * curData->ImageH]();

//    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

//    int cnt=1;
//    QQueue<int> quex,quey;
//    quex.enqueue(sx);
//    quey.enqueue(sy);
//    QQueue<int> quex2,quey2;
//    quex2.enqueue(sx);
//    quey2.enqueue(sy);
//    ck[sy*curData->ImageW+sx]=true;


//    float midx=sx;
//    float midy=sy;

//    int minx=sx;
//    int miny=sy;
//    int maxx=sx;
//    int maxy=sy;

//    while(!quex.empty()){
//        if(cnt>=maxPNum)break;
//        if(maxx-minx>50 || maxy-miny>50)break;
//        int curx=quex.dequeue();
//        int cury=quey.dequeue();

//        for(int d=0;d<8;d++){
//            if(curx+dxylist[d][0]<0 || curx+dxylist[d][0]>=curData->ImageW || cury+dxylist[d][1]<0 || cury+dxylist[d][1]>curData->ImageH){
//                continue;
//            }
//            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
//            if(ck[tP]==false){
//                ck[tP]=true;
//                if(structureData[tP]==l || (structureData[tP]==2 && l==-1) || (structureData[tP]==3 && l==-1)){
//                    quex.enqueue(curx+dxylist[d][0]);
//                    quey.enqueue(cury+dxylist[d][1]);
//                    quex2.enqueue(curx+dxylist[d][0]);
//                    quey2.enqueue(cury+dxylist[d][1]);
//                    midx+=curx+dxylist[d][0];
//                    midy+=cury+dxylist[d][1];
//                    cnt++;
//                    if(curx+dxylist[d][0]>maxx)maxx=curx+dxylist[d][0];
//                    if(curx+dxylist[d][0]<minx)minx=curx+dxylist[d][0];
//                    if(cury+dxylist[d][1]>maxy)maxy=cury+dxylist[d][1];
//                    if(cury+dxylist[d][1]<miny)miny=cury+dxylist[d][1];
//                }
//            }
//        }
//    }
//    delete []ck;

//    midx/=cnt;
//    midy/=cnt;

//    structure outStructure;

//    outStructure.width=maxx-minx+1;
//    outStructure.height=maxy-miny+1;
//    outStructure.minx=minx;
//    outStructure.miny=miny;

//    outStructure.cnt=cnt;
//    float minD=100000;

//    outStructure.data=new unsigned short[outStructure.width*outStructure.height];
//    outStructure.label=new unsigned char[outStructure.width*outStructure.height];
//    for(int i=0;i<outStructure.width*outStructure.height;i++){
//        outStructure.data[i]=0;
//        outStructure.label[i]=0;
//    }

//    while(!quex2.empty()){
//        int ix=quex2.dequeue();
//        int iy=quey2.dequeue();
//        if(eraseFlag)structureData[iy*curData->ImageW+ix]=0;
//        outStructure.data[(iy-miny)*outStructure.width+(ix-minx)]=curData->imageData1[iy*curData->ImageW+ix];
//        outStructure.label[(iy-miny)*outStructure.width+(ix-minx)]=curData->structureData[iy*curData->ImageW+ix];
//        if(abs(midx-ix)+abs(midy-iy)<minD){
//            outStructure.midx=ix;
//            outStructure.midy=iy;
//            minD=abs(midx-ix)+abs(midy-iy);
//        }
//    }
//    return outStructure;
//}

//void patchRecommender::findNextNodes(int curNodePos, QQueue<int> *out){
//    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};
//    bool *ck=new bool[curData->ImageW * curData->ImageH]();
//    bool *ck2=new bool[curData->ImageW * curData->ImageH]();
//    int startx=curNodePos%curData->ImageW;
//    int starty=curNodePos/curData->ImageW;
//    QQueue <int> quex;
//    QQueue <int> quey;
//    quex.enqueue(startx);
//    quey.enqueue(starty);
//    while(!quex.empty()){
//        int curx=quex.dequeue();
//        int cury=quey.dequeue();
//        ck[cury*curData->ImageW + curx]=true;
//        for(int d=0;d<8;d++){
//            if(curx+dxylist[d][0]<=0 || curx+dxylist[d][0]>=curData->ImageW-1 || cury+dxylist[d][1]<=0 || cury+dxylist[d][1]>=curData->ImageH-1){
//                continue;
//            }
//            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
//            if(ck[tP])continue;

//            if(tracingMap[tP].type==0 && tracingMap[tP].posx==startx && tracingMap[tP].posy==starty){
//                quex.enqueue(curx+dxylist[d][0]);
//                quey.enqueue(cury+dxylist[d][1]);
//            }
//            if(tracingMap[tP].type==0 && (tracingMap[tP].posx!=startx || tracingMap[tP].posy!=starty)){
//                int newP=tracingMap[tP].posy * curData->ImageW + tracingMap[tP].posx;
//                bool okay=true;
//                for(int k=0;k<tracingMap[curNodePos].connected_nodes.size();k++){
//                    if(tracingMap[curNodePos].connected_nodes[k]==newP)okay=false;
//                }
//                if(okay){
//                    tracingMap[curNodePos].connected_nodes.push_back(newP);
//                    tracingMap[newP].connected_nodes.push_back(curNodePos);
//                    int connected_size=tracingMap[newP].connected_nodes.size();
//                    if(tracingMap[newP].type==6){
//                        out->enqueue(newP);
//                    }
//                    else if(connected_size==0 ||connected_size==1)
//                        tracingMap[newP].type=4;
//                    else if(connected_size==2)
//                        tracingMap[newP].type=1;
//                    else if(connected_size>2)
//                        tracingMap[newP].type=2;

//                }

//            }
//            if((tracingMap[tP].type==-1 && (curData->structureData[tP]==2 || curData->structureData[tP]==3))
//                    || (tracingMap[tP].type==0 && (tracingMap[tP].posx!=startx || tracingMap[tP].posy!=starty))){
////            if(tracingMap[tP].type==-1 && (curData->structureData[tP]==2 || curData->structureData[tP]==3)){
//                int newP=getCandidateMiddlePoint_tracing(startx,starty,curx+dxylist[d][0],cury+dxylist[d][1],ck);
//                if(tracingMap[newP].type==-1){
//                    bool okay=true;
//                    for(int k=0;k<tracingMap[curNodePos].connected_nodes.size();k++){
//                        if(tracingMap[curNodePos].connected_nodes[k]==newP)okay=false;
//                    }
//                    if(okay){
//                        out->enqueue(newP);
//                        tracingMap[curNodePos].connected_nodes.push_back(newP);
//                        tracingMap[newP].connected_nodes.push_back(curNodePos);
//                    }
//                }
//            }
//        }
//    }
//    delete []ck;
//    delete []ck2;

//}
//void patchRecommender::updateTracingMap(int curNodePos){

//    patchSet *patch=curData->tracingMap_neurite_based[curNodePos].patch;

//    if(patch->is_valid==false){
//        return;
//    }

//    int posx=curNodePos%curData->ImageW;
//    int posy=curNodePos/curData->ImageW;


//    int curP=posy*curData->ImageW+posx;

////    bool *ck=new bool[tW*tH]();
//    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};


//    QQueue<int> quex,quey;
//    QQueue<float> distance;
//    quex.enqueue(posx);
//    quey.enqueue(posy);

//    while(!quex.empty()){
//        int curx=quex.dequeue();
//        int cury=quey.dequeue();
//        float curDis=distance.dequeue();
//        for(int d=0;d<8;d++){
//            int new_x=curx+dxylist[d][0];
//            int new_y=cury+dxylist[d][1];
//            if(new_x<patch->boundMin.x() || new_x>patch->boundMax.x()
//                    || new_y<patch->boundMin.y() || new_y>patch->boundMax.y()){
//                continue;
//            }
//            if(isInside(QVector2D(new_x,new_y),patch->patchCorner)==false)continue;

//            int tP=new_y*curData->ImageW + new_x;
//            if(curData->structureData[tP]!=2 && curData->structureData[tP]!=3)continue;

//            float v=float(curData->imageData1[tP])/65535;
//            float newDis=curDis+1.0/(1+exp(10*v-5));
//            if(tracingMap[tP].distanceFromNode>newDis){
//                if(tracingMap[tP].type==-1 || tracingMap[tP].type==0){
//                    quex.enqueue(curx+dxylist[d][0]);
//                    quey.enqueue(cury+dxylist[d][1]);
//                    distance.enqueue(newDis);
//                    tracingMap[tP].type=0;
//                    tracingMap[tP].posx=posx;
//                    tracingMap[tP].posy=posy;
//                    tracingMap[tP].distanceFromNode=newDis;//getShortestPathBetweenToPoint(curP,tP);//(posx-curx)*(posx-curx)+(posy-cury)*(posy-cury);
//                }
//            }
//        }
//    }
//}
