
#include "structureRefinement.h"
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
#include "mojo/mojo.h"



structureRefinement::structureRefinement(MitoDataset*p, QWidget *parent){
    curData=p;
    tracingMap=new tracingNode[curData->ImageW * curData->ImageH];
    curData->user_define_cost=new float[curData->ImageW * curData->ImageH];
    curData->user_brushing_buffer=new int[curData->ImageW * curData->ImageH];
    for(int i=0;i<curData->ImageW * curData->ImageH;i++){
        curData->user_define_cost[i]=1;
        curData->user_brushing_buffer[i]=-1;
    }

    curData->connection_path_forward=new unsigned int[curData->ImageW*curData->ImageH]();
    curData->connection_path_backward=new unsigned int[curData->ImageW*curData->ImageH]();


    curData->connection_object1=new int[curData->ImageW*curData->ImageH];
    curData->connection_object2=new int[curData->ImageW*curData->ImageH];
    for(int i=0;i<curData->ImageW * curData->ImageH;i++){
        curData->connection_object1[i]=-1;
        curData->connection_object2[i]=-1;
    }
}
void structureRefinement::init(){

//    initCnnModel();
    qDebug()<<"pre training";
//    preTraining();
//    savePatches("patches/");
    qDebug()<<"get candidate points";
    labelCleaning();

    getStartingPoints_minimap();
//    getCandidatePoints();
    qDebug()<<"end: get candidate points";

    curData->doTracing=true;

    initialTracingFromCandidatePoints();
    completeTracing();
    refineTracing();
    divideNeurite();
    curData->doTracing=false;

    curData->save_tracingMap(curData->path+"tracing_map.data");

    emit tracingFinished(curData->dataIndex);

//    doPredict();

//    saveProbabilityMap("probabilityMap/");
//    run();
}
void structureRefinement::refineTracing(){

    qDebug()<<"refineTracing";


    qDebug()<<"remove cycle";

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int curP=iy*curData->ImageW+ix;
            if(curData->tracingMap[curP].type>0){
                int nextP;
                int next_nextP;
                bool is_cycle=false;
                for(int i=0;i<curData->tracingMap[curP].connected_nodes.size();i++){
                    nextP=curData->tracingMap[curP].connected_nodes[i];
                    for(int j=0;j<curData->tracingMap[nextP].connected_nodes.size();j++){
                        next_nextP=curData->tracingMap[nextP].connected_nodes[j];
                        if(curP==next_nextP)continue;
                        for(int k=0;k<curData->tracingMap[next_nextP].connected_nodes.size();k++){
                            int next_next_nextP=curData->tracingMap[next_nextP].connected_nodes[k];
                            if(curP==next_next_nextP){
                                is_cycle=true;
                                break;
                            }
                        }
                        if(is_cycle)break;
                    }
                    if(is_cycle)break;
                }
                if(is_cycle){
                    if(curData->tracingMap[curP].connected_nodes.size()==2 || curData->tracingMap[nextP].connected_nodes.size()==2){
                        for(int i=0;i<curData->tracingMap[curP].connected_nodes.size();i++){
                            if(nextP==curData->tracingMap[curP].connected_nodes[i]){
                                curData->tracingMap[curP].connected_nodes.remove(i);
                                break;
                            }
                        }
                        for(int i=0;i<curData->tracingMap[nextP].connected_nodes.size();i++){
                            if(curP==curData->tracingMap[nextP].connected_nodes[i]){
                                curData->tracingMap[nextP].connected_nodes.remove(i);
                                break;
                            }
                        }

                    }
                    else{
                        for(int i=0;i<curData->tracingMap[curP].connected_nodes.size();i++){
                            if(next_nextP==curData->tracingMap[curP].connected_nodes[i]){
                                curData->tracingMap[curP].connected_nodes.remove(i);
                                break;
                            }
                        }
                        for(int i=0;i<curData->tracingMap[next_nextP].connected_nodes.size();i++){
                            if(curP==curData->tracingMap[next_nextP].connected_nodes[i]){
                                curData->tracingMap[next_nextP].connected_nodes.remove(i);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }


    qDebug()<<"ignore noise node";

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int curP=iy*curData->ImageW+ix;
            if(curData->tracingMap[curP].type>0 && curData->tracingMap[curP].connected_nodes.size()>0){
                int cnt=0;
                for(int i=0;i<curData->tracingMap[curP].connected_nodes.size();i++){
                    int n=curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].connected_nodes.size();
                    if(n>=2){
                        cnt++;
                    }
                }
                if(cnt>2)curData->tracingMap[curP].type=2;
                if(cnt==2)curData->tracingMap[curP].type=1;
                if(cnt==0 || cnt==1)curData->tracingMap[curP].type=4;
            }
        }
    }
    qDebug()<<"end refineTracing";

}
void structureRefinement::divideNeurite(){
    qDebug()<<"divideNeurite";


    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int curP=iy*curData->ImageW+ix;

            if(curData->tracingMap[curP].temp_ck)continue;

            QQueue<int> res;

            if(curData->tracingMap[curP].type==2 || curData->tracingMap[curP].type==3){
                curData->tracingMap[curP].temp_ck=true;
                res.enqueue(curP);
            }
            else if(curData->tracingMap[curP].type>0){
                QQueue<int> nodes;
                nodes.enqueue(curP);
                curData->tracingMap[curP].temp_ck=true;

                while(!nodes.isEmpty()){
                    int curP=nodes.dequeue();

                    if(curData->tracingMap[curP].type>0){
                        res.enqueue(curP);
                        for(int i=0;i<curData->tracingMap[curP].connected_nodes.size();i++){
                            if(curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].temp_ck==false
                                    && curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].type!=2 && curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].type!=3){
                                curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].temp_ck=true;
                                nodes.enqueue(curData->tracingMap[curP].connected_nodes[i]);
                            }
                        }
                    }
                }
            }

            bool ok=!res.isEmpty();
            while(!res.isEmpty()){
                int curP=res.dequeue();
                curData->tracingMap[curP].neurite_ind=*curData->global_neurite_ind;
                curData->patch_neurite_map[curData->tracingMap[curP].patch->index]=curData->tracingMap[curP].neurite_ind;
            }
            if(ok){
                curData->neurite_sample_point[*curData->global_neurite_ind]=iy*curData->ImageW+ix;

                (*curData->global_neurite_ind)++;
            }

        }
    }
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int curP=iy*curData->ImageW+ix;
            curData->tracingMap[curP].temp_ck=false;
        }
    }
    qDebug()<<"end divideNeurite";


}
void structureRefinement::saveProbabilityMap(QString path){

    unsigned char*axon_prob=new unsigned char[curData->ImageW*curData->ImageH]();
    unsigned char*dend_prob=new unsigned char[curData->ImageW*curData->ImageH]();
    unsigned char*axonWin_prob=new unsigned char[curData->ImageW*curData->ImageH]();
    unsigned char*dendWin_prob=new unsigned char[curData->ImageW*curData->ImageH]();

    int axonN=0;
    int dendN=0;
    while(1){
        if(axonN>=300 && dendN>=300)break;

        int sx,sy,tP;
        sx=float(rand())/RAND_MAX*(curData->ImageW-2)+1;
        sy=float(rand())/RAND_MAX*(curData->ImageH-2)+1;
        tP=sy*curData->ImageW+sx;
        if(axonN>=300){
            while(curData->structureData[tP]!=2){
                sx=float(rand())/RAND_MAX*(curData->ImageW-2)+1;
                sy=float(rand())/RAND_MAX*(curData->ImageH-2)+1;
                tP=sy*curData->ImageW+sx;
            }

        }
        else if(dendN>=300){
            while(curData->structureData[tP]!=3){
                sx=float(rand())/RAND_MAX*(curData->ImageW-2)+1;
                sy=float(rand())/RAND_MAX*(curData->ImageH-2)+1;
                tP=sy*curData->ImageW+sx;
            }
        }
        else{
            while(curData->structureData[tP]!=2 && curData->structureData[tP]!=3){
                sx=float(rand())/RAND_MAX*(curData->ImageW-2)+1;
                sy=float(rand())/RAND_MAX*(curData->ImageH-2)+1;
                tP=sy*curData->ImageW+sx;
            }
        }

        QVector2D midP=getMiddlePoint(sx,sy,curData->structureData[tP]);
//        structure S=getStructure(sx,sy,curData->structureData[tP],patchSize*patchSize/3,curData->structureData,false);
//        if(S.width<patchSize && S.height<patchSize)continue;
        tP=midP.y()*curData->ImageW+midP.x();
        patchSet *newPatch=getPatch(midP.x(),midP.y(),curData->structureData[tP]);
        if(newPatch->is_valid==false){
            delete newPatch;
            continue;
        }
//        delete []S.data;

        if(curData->structureData[tP]==2)dendN++;
        else if(curData->structureData[tP]==3)axonN++;


        float *probability=getProbability(newPatch->patch);

        probability[0]=(probability[0]-0.48) * 25;
        probability[1]=(probability[1]-0.48) * 25;
        if(probability[0]<0)probability[0]=0;
        if(probability[0]>1)probability[0]=1;
        if(probability[1]<0)probability[1]=0;
        if(probability[1]>1)probability[1]=1;


        int tW=newPatch->boundMax.x()-newPatch->boundMin.x()+1;
        int tH=newPatch->boundMax.y()-newPatch->boundMin.y()+1;

        bool *ck=new bool[tW*tH]();
        int dxylist[4][2]={{-1,0},{1,0},{0,-1},{0,1}};

        QQueue<int> quex,quey;
        quex.enqueue(midP.x());
        quey.enqueue(midP.y());
        dend_prob[int(midP.y()*curData->ImageW+midP.x())]=probability[0]*255;
        axon_prob[int(midP.y()*curData->ImageW+midP.x())]=probability[1]*255;
        if(probability[0]<probability[1])axonWin_prob[int(midP.y()*curData->ImageW+midP.x())]=255;
        else dendWin_prob[int(midP.y()*curData->ImageW+midP.x())]=255;

        ck[int((midP.y()-newPatch->boundMin.y())*tW+midP.x()-newPatch->boundMin.x())]=true;

        while(!quex.empty()){
            int curx=quex.dequeue();
            int cury=quey.dequeue();
            for(int d=0;d<4;d++){
                if(curx+dxylist[d][0]<newPatch->boundMin.x() || curx+dxylist[d][0]>newPatch->boundMax.x()
                        || cury+dxylist[d][1]<newPatch->boundMin.y() || cury+dxylist[d][1]>newPatch->boundMax.y()){
                    continue;
                }
                if(isInside(QVector2D(curx+dxylist[d][0], cury+dxylist[d][1]),newPatch->patchCorner)==false)continue;

                int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
                int tP2=(cury+dxylist[d][1]-newPatch->boundMin.y())*tW+curx+dxylist[d][0]-newPatch->boundMin.x();
                if(ck[tP2]==false){
                    ck[tP2]=true;
                    if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                        dend_prob[tP]=probability[0]*255;
                        axon_prob[tP]=probability[1]*255;
                        if(probability[0]<probability[1])axonWin_prob[tP]=255;
                        else dendWin_prob[tP]=255;
                        quex.enqueue(curx+dxylist[d][0]);
                        quey.enqueue(cury+dxylist[d][1]);
                    }
                }
            }
        }
        delete []ck;
        delete newPatch;
    }
    FILE *f=fopen((path+"probabilityMapDend_"+QString::number(curData->ImageW)+"_"+QString::number(curData->ImageH)+".raw").toStdString().c_str(),"wb");
    fwrite(dend_prob,curData->ImageW*curData->ImageH,1,f);
    fclose(f);
    f=fopen((path+"probabilityMapAxon_"+QString::number(curData->ImageW)+"_"+QString::number(curData->ImageH)+".raw").toStdString().c_str(),"wb");
    fwrite(axon_prob,curData->ImageW*curData->ImageH,1,f);
    fclose(f);
    f=fopen((path+"probabilityMapDendWin_"+QString::number(curData->ImageW)+"_"+QString::number(curData->ImageH)+".raw").toStdString().c_str(),"wb");
    fwrite(dendWin_prob,curData->ImageW*curData->ImageH,1,f);
    fclose(f);
    f=fopen((path+"probabilityMapAxonWin_"+QString::number(curData->ImageW)+"_"+QString::number(curData->ImageH)+".raw").toStdString().c_str(),"wb");
    fwrite(axonWin_prob,curData->ImageW*curData->ImageH,1,f);
    fclose(f);

    delete []axon_prob;
    delete []dend_prob;
    delete []axonWin_prob;
    delete []dendWin_prob;
}

structureRefinement::~structureRefinement(){

}

void structureRefinement::run(){
    if(exitSignal)return;
    stopSignal=false;

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            if(curData->correctionData[tP]==1)curData->correctionData[tP]=2;
        }
    }

    while(1){
        if(stopSignal){
            stopSignal=false;
            break;
        }
        if(is_new_input){
            is_new_input=false;
//            for(int iy=0;iy<curData->ImageH;iy++){
//                for(int ix=0;ix<curData->ImageW;ix++){
//                    if(curData->scribbleData[iy*curData->ImageW+ix]!=0){
//                        int l=curData->scribbleData[iy*curData->ImageW+ix];
//                        structure S=getStructure(ix,iy,l,999999,curData->scribbleData,true);
//                        //S.save(QString::number(structureCnt++)+".raw");
//                        std::vector<float*> newPatches=getPatches(S);
//                        delete []S.data;
//                        std::vector<int> newLabels;
//                        for(int i=0;i<newPatches.size();i++)newLabels.push_back(l);
//                        while(1){
//                            modelTraining(newPatches.size(),newPatches,newLabels);
//                            if (cnn->elvis_left_the_building())
//                            {
//                                qDebug()<<"Re-training accuracy with target set: "<<getModelAccuracy(trainingData.size(),trainingData,trainingLabel);
//                                break;
//                            }
//                        }
//                    }
//                }
//            }
        }
        qDebug()<<"correction loop start";
        int noChangeCount=0;
        while(!candidatePoints.empty()){
            cleanCnt++;
            if(cleanCnt>200){
                labelCleaning();
                cleanCnt=0;
            }
            if(noChangeCount>=candidatePoints.size()){
                stopSignal=true;
                break;
            }
            noChangeCount++;
            if(is_new_input)break;
            if(stopSignal)break;
            patchSet *patch=candidatePoints.dequeue();
            QVector2D curPoint=QVector2D(patch->midx,patch->midy);
            bool valid=false;
            //check whether the current target point is valid or not
            {
                int tP=curPoint.y()*curData->ImageW+curPoint.x();
                if(curData->structureData[tP]==2){
                    if(curData->structureData[tP-1]==3 ||
                        curData->structureData[tP+1]==3 ||
                        curData->structureData[tP-curData->ImageW]==3 ||
                        curData->structureData[tP+curData->ImageW]==3){
                        valid=true;
                    }
                }
                else if(curData->structureData[tP]==3){
                    if(curData->structureData[tP-1]==2 ||
                        curData->structureData[tP+1]==2 ||
                        curData->structureData[tP-curData->ImageW]==2 ||
                        curData->structureData[tP+curData->ImageW]==2){
                        valid=true;
                    }
                }
            }
            if(valid==false){
                delete patch;
                continue;
            }


//            qDebug()<<"patch generated : "<<patch->midx<<" "<<patch->midy;

            float *probability=getProbability(patch->patch);
            int newlabel;
            if(probability[0]>curData->correction_threshold && curData->correctorEnable_dend)newlabel=2;
            else if(probability[1]>curData->correction_threshold && curData->correctorEnable_axon)newlabel=3;
            else{
                candidatePoints.enqueue(patch);
//                emit correctionChanged();
                continue;
            }
//            qDebug()<<"new label: "<<newlabel;



            QQueue<int> quex2,quey2;
            //structure update by connected component and inclusion test
            {
                int tW=patch->boundMax.x()-patch->boundMin.x()+1;
                int tH=patch->boundMax.y()-patch->boundMin.y()+1;

                bool *ck=new bool[tW*tH]();
                int dxylist[4][2]={{-1,0},{1,0},{0,-1},{0,1}};

                QQueue<int> quex,quey;
                quex.enqueue(curPoint.x());
                quey.enqueue(curPoint.y());
                quex2.enqueue(curPoint.x());
                quey2.enqueue(curPoint.y());
                curData->structureData[int(curPoint.y()*curData->ImageW+curPoint.x())]=newlabel;
                ck[int((curPoint.y()-patch->boundMin.y())*tW+curPoint.x()-patch->boundMin.x())]=true;

                while(!quex.empty()){
                    int curx=quex.dequeue();
                    int cury=quey.dequeue();
                    for(int d=0;d<4;d++){
                        if(curx+dxylist[d][0]<patch->boundMin.x() || curx+dxylist[d][0]>patch->boundMax.x()
                                || cury+dxylist[d][1]<patch->boundMin.y() || cury+dxylist[d][1]>patch->boundMax.y()){
                            continue;
                        }
                        if(isInside(QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]),patch->patchCorner)==false)continue;

                        int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
                        int tP2=(cury+dxylist[d][1]-patch->boundMin.y())*tW+curx+dxylist[d][0]-patch->boundMin.x();
                        if(ck[tP2]==false){
                            ck[tP2]=true;
                            if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                                curData->structureData[tP]=newlabel;
                                quex.enqueue(curx+dxylist[d][0]);
                                quey.enqueue(cury+dxylist[d][1]);
                                quex2.enqueue(curx+dxylist[d][0]);
                                quey2.enqueue(cury+dxylist[d][1]);
                            }
                        }
                    }
                }
                delete []ck;
            }



            for(int iy=patch->boundMin.y();iy<=patch->boundMax.y();iy++){
                for(int ix=patch->boundMin.x();ix<=patch->boundMax.x();ix++){
                    if(isInside(QVector2D(ix,iy),patch->patchCorner)){
                        int tP=iy*curData->ImageW+ix;
                        if(curData->correctionData[tP]!=3)curData->correctionData[tP]=2;
                    }
                }
            }

            delete patch;
            qDebug()<<"structure corrected";

            bool *ck=new bool[curData->ImageW * curData->ImageH]();

            while(!quex2.empty()){
                int ix=quex2.dequeue();
                int iy=quey2.dequeue();
                if(ix==0 || iy==0|| ix==curData->ImageW-1 || iy==curData->ImageH-1){
                    qDebug()<<"oops";
                    continue;
                }
                int tP=iy*curData->ImageW+ix;
                if(ck[tP])continue;
                if(curData->structureData[tP]==2){
                    if(curData->structureData[tP-1]==3 ||
                        curData->structureData[tP+1]==3 ||
                        curData->structureData[tP-curData->ImageW]==3 ||
                        curData->structureData[tP+curData->ImageW]==3){
                        QVector2D newP=getCandidateMiddlePoint(ix,iy,ck);
                        patchSet *newPatch=getPatch(newP.x(),newP.y(),-1);
                        if(newPatch->is_valid)candidatePoints.push_back(newPatch);
                        else{
                            delete newPatch;
                        }
                    }
                }
                else if(curData->structureData[tP]==3){
                    if(curData->structureData[tP-1]==2 ||
                        curData->structureData[tP+1]==2 ||
                        curData->structureData[tP-curData->ImageW]==2 ||
                        curData->structureData[tP+curData->ImageW]==2){
                        QVector2D newP=getCandidateMiddlePoint(ix,iy,ck);
                        patchSet *newPatch=getPatch(newP.x(),newP.y(),-1);
                        if(newPatch->is_valid)candidatePoints.push_back(newPatch);
                        else{
                            delete newPatch;
                        }
//                        candidatePoints.push_back(getCandidateMiddlePoint(ix,iy,ck));
                    }
                }
            }

            qDebug()<<"found new target point";
            emit structureChanged();
            emit correctionChanged();


            noChangeCount=0;

//            QThread::msleep(100);
            delete []ck;
        }

        labelCleaning();

        emit structureChanged();
        emit correctionChanged();

    }

    for(int i=0;i<candidatePoints.size();i++){
        patchSet* patch=candidatePoints[i];

        for(int iy=patch->boundMin.y();iy<=patch->boundMax.y();iy++){
            for(int ix=patch->boundMin.x();ix<=patch->boundMax.x();ix++){
                if(isInside(QVector2D(ix,iy),patch->patchCorner)){
                    int tP=iy*curData->ImageW+ix;
                    //if(curData->correctionData[tP]!=3)
                    curData->correctionData[tP]=1;
                }
            }
        }
    }
    emit correctionChanged();

}
QVector2D structureRefinement::getMiddlePoint(int sx, int sy,int l){

    QVector2D res(sx,sy);
    QVector2D cur(sx,sy);

    int maxCnt=-1;


    QVector2D up(sx,sy);
    QVector2D down(sx,sy);
    QVector2D right(sx,sy);
    QVector2D left(sx,sy);
    int cnt=0;
    while(1){
        up=up+QVector2D(0,1);
        down=down+QVector2D(0,-1);
        right=right+QVector2D(1,0);
        left=left+QVector2D(-1,0);
        bool go_up=false;
        bool go_down=false;
        bool go_right=false;
        bool go_left=false;
        if(up.y()<curData->ImageH){
            int tP=up.y()*curData->ImageW + up.x();
            if(curData->structureData[tP]==l || (curData->structureData[tP]==2 && l==-1) || (curData->structureData[tP]==3 && l==-1)){
                go_up=true;
            }
        }
        if(down.y()>=0){
            int tP=down.y()*curData->ImageW + down.x();
            if(curData->structureData[tP]==l || (curData->structureData[tP]==2 && l==-1) || (curData->structureData[tP]==3 && l==-1)){
                go_down=true;
            }
        }
        if(right.x()<curData->ImageW){
            int tP=right.y()*curData->ImageW + right.x();
            if(curData->structureData[tP]==l || (curData->structureData[tP]==2 && l==-1) || (curData->structureData[tP]==3 && l==-1)){
                go_right=true;
            }
        }
        if(left.x()>=0){
            int tP=left.y()*curData->ImageW + left.x();
            if(curData->structureData[tP]==l || (curData->structureData[tP]==2 && l==-1) || (curData->structureData[tP]==3 && l==-1)){
                go_left=true;
            }
        }
        if(go_up && go_down && go_right && go_left){
            cnt++;
        }
        else{
            if(cnt>maxCnt){
                res=cur;
                if(go_up==false && cur.y()>0)cur+=QVector2D(0,-1);
                if(go_down==false && cur.y()<curData->ImageH-1)cur+=QVector2D(0,1);
                if(go_right==false && cur.x()>0)cur+=QVector2D(-1,0);
                if(go_left==false && cur.x()<curData->ImageW-1)cur+=QVector2D(1,0);
                maxCnt=cnt;
                up=cur;
                down=cur;
                right=cur;
                left=cur;
                cnt=0;
            }
            else{
                return res;
            }
        }
    }
}


void structureRefinement::labelCleaning(){
    bool *ck=new bool[curData->ImageW * curData->ImageH]();

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    int i=curData->ImageW * curData->ImageH-1;
    while(1){
        int sx=-1,sy=-1;
        int l=-1;
        for(;i>=0;i--){
            if(ck[i]==false && (curData->structureData[i]==2 || curData->structureData[i]==3 )){
                sy=i/curData->ImageW;
                sx=i%curData->ImageW;
                l=curData->structureData[i];
                break;
            }
        }
        if(l==-1)break;
        QQueue<int> quex,quey;
        quex.enqueue(sx);
        quey.enqueue(sy);
        QQueue<int> quex2,quey2;
        quex2.enqueue(sx);
        quey2.enqueue(sy);
        ck[sy*curData->ImageW+sx]=true;
        bool touched=false;
        int cnt=1;

        while(!quex.empty()){
            int curx=quex.dequeue();
            int cury=quey.dequeue();

            for(int d=0;d<8;d++){
                if(curx+dxylist[d][0]<0 || curx+dxylist[d][0]>=curData->ImageW || cury+dxylist[d][1]<0 || cury+dxylist[d][1]>curData->ImageH){
                    continue;
                }
                int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
                if(ck[tP]==false){
                    if(curData->structureData[tP]==l){
                        ck[tP]=true;
                        quex.enqueue(curx+dxylist[d][0]);
                        quey.enqueue(cury+dxylist[d][1]);
                        quex2.enqueue(curx+dxylist[d][0]);
                        quey2.enqueue(cury+dxylist[d][1]);
                        cnt++;
                    }
                }
                if(l==2 && curData->structureData[tP]==3){
                    touched=true;
                }
                if(l==3 && curData->structureData[tP]==2){
                    touched=true;
                }
            }
        }
        if(cnt<10 && touched){
            while(!quex2.empty()){
                int curx=quex2.dequeue();
                int cury=quey2.dequeue();
                int tP=cury*curData->ImageW + curx;
                if(l==2)curData->structureData[tP]=3;
                else curData->structureData[tP]=2;
//                curData->correctionData[tP]=10;
            }
        }

    }
    delete []ck;

}

void structureRefinement::trainingDataGeneration(patchSet *patch,int l,int nodePos){
    float *data=new float[patchSize*patchSize];
    for(int i=0;i<patchSize*patchSize;i++)data[i]=patch->patch[i];

    float *xflip=new float[patchSize*patchSize];
    for(int iy=0;iy<patchSize;iy++){
        for(int ix=0;ix<patchSize;ix++){
            xflip[iy*patchSize+ix]=data[iy*patchSize+(patchSize-ix-1)];
        }
    }
    float *yflip=new float[patchSize*patchSize];
    for(int iy=0;iy<patchSize;iy++){
        for(int ix=0;ix<patchSize;ix++){
            yflip[iy*patchSize+ix]=data[(patchSize-iy-1)*patchSize+ix];
        }
    }
    float *xyflip=new float[patchSize*patchSize];
    for(int iy=0;iy<patchSize;iy++){
        for(int ix=0;ix<patchSize;ix++){
            xyflip[iy*patchSize+ix]=data[(patchSize-iy-1)*patchSize+(patchSize-ix-1)];
        }
    }

    if(l==2){ //dend
        fixedTrainingData_dend.push_back(data);
        fixedTrainingData_dend.push_back(xflip);
        fixedTrainingData_dend.push_back(yflip);
        fixedTrainingData_dend.push_back(xyflip);
        fixedTrainingLabel_dend.push_back(0);
        fixedTrainingLabel_dend.push_back(0);
        fixedTrainingLabel_dend.push_back(0);
        fixedTrainingLabel_dend.push_back(0);
        index_trainingData_to_node_dend.push_back(nodePos);
        index_trainingData_to_node_dend.push_back(nodePos);
        index_trainingData_to_node_dend.push_back(nodePos);
        index_trainingData_to_node_dend.push_back(nodePos);
    }
    else if(l==3){ //axon
        fixedTrainingData_axon.push_back(data);
        fixedTrainingData_axon.push_back(xflip);
        fixedTrainingData_axon.push_back(yflip);
        fixedTrainingData_axon.push_back(xyflip);
        fixedTrainingLabel_axon.push_back(1);
        fixedTrainingLabel_axon.push_back(1);
        fixedTrainingLabel_axon.push_back(1);
        fixedTrainingLabel_axon.push_back(1);
        index_trainingData_to_node_axon.push_back(nodePos);
        index_trainingData_to_node_axon.push_back(nodePos);
        index_trainingData_to_node_axon.push_back(nodePos);
        index_trainingData_to_node_axon.push_back(nodePos);
    }

}
void structureRefinement::reTraining(){
    int sizeDend=fixedTrainingData_dend.size();
    int sizeAxon=fixedTrainingData_axon.size();

    std::vector<float*>tempTrainingData;
    std::vector<int>tempLabelData;
    int cnt=0;
    while(1){
        if(cnt>=sizeDend && cnt>=sizeAxon)break;
        if(cnt<sizeDend)tempTrainingData.push_back(fixedTrainingData_dend[cnt]);
        else{
            int sx,sy,tP;
            sx=float(rand())/RAND_MAX*(curData->ImageW-2)+1;
            sy=float(rand())/RAND_MAX*(curData->ImageH-2)+1;
            tP=sy*curData->ImageW+sx;
            while(curData->structureData[tP]!=2){
                sx=float(rand())/RAND_MAX*(curData->ImageW-2)+1;
                sy=float(rand())/RAND_MAX*(curData->ImageH-2)+1;
                tP=sy*curData->ImageW+sx;
            }
            QVector2D midP=getMiddlePoint(sx,sy,curData->structureData[tP]);
            tP=midP.y()*curData->ImageW+midP.x();
            patchSet *newPatch=getPatch(midP.x(),midP.y(),curData->structureData[tP]);
            if(newPatch->is_valid==false){
                delete newPatch;
                continue;
            }

            float *buf=new float[patchSize*patchSize];
            for(int i=patchSize*patchSize-1;i>=0;i--){
                buf[i]=newPatch->patch[i];
            }
            tempTrainingData.push_back(buf);
            delete newPatch;
        }

        if(cnt<sizeAxon)tempTrainingData.push_back(fixedTrainingData_axon[cnt]);
        else{
            int sx,sy,tP;
            sx=float(rand())/RAND_MAX*(curData->ImageW-2)+1;
            sy=float(rand())/RAND_MAX*(curData->ImageH-2)+1;
            tP=sy*curData->ImageW+sx;
            while(curData->structureData[tP]!=3){
                sx=float(rand())/RAND_MAX*(curData->ImageW-2)+1;
                sy=float(rand())/RAND_MAX*(curData->ImageH-2)+1;
                tP=sy*curData->ImageW+sx;
            }
            QVector2D midP=getMiddlePoint(sx,sy,curData->structureData[tP]);
            tP=midP.y()*curData->ImageW+midP.x();
            patchSet *newPatch=getPatch(midP.x(),midP.y(),curData->structureData[tP]);
            if(newPatch->is_valid==false){
                delete newPatch;
                tempTrainingData.pop_back();
                continue;
            }

            float *buf=new float[patchSize*patchSize];
            for(int i=patchSize*patchSize-1;i>=0;i--){
                buf[i]=newPatch->patch[i];
            }
            tempTrainingData.push_back(buf);
            delete newPatch;
        }


        tempLabelData.push_back(0);
        tempLabelData.push_back(1);
        cnt++;

    }

    cnn->reset_smart_training();
    cnn->set_learning_rate(0.04);
    cnn->set_max_epochs(20);

    while(1){
        modelTraining(cnt*2,tempTrainingData,tempLabelData);
        qDebug()<<getModelAccuracy(cnt*2,tempTrainingData,tempLabelData);
        if (cnn->elvis_left_the_building())
        {
            qDebug()<<"re-training accuracy: "<<getModelAccuracy(cnt*2,tempTrainingData,tempLabelData);
            break;
        }
    }
    for(int i=0;i<cnt;i++){
        if(i>=sizeDend){
            delete []tempTrainingData[i*2+0];
        }
        if(i>=sizeAxon){
            delete []tempTrainingData[i*2+1];
        }

    }
}

void structureRefinement::initCnnModel(){
    cnn=new mojo::network("adam");
    cnn->set_mini_batch_size(10);
    cnn->set_smart_training(true); // automate training
    cnn->set_learning_rate(0.04);


//    cnn->push_back("I1", "input 11 11 1");				// 11x11x1
//    cnn->push_back("C1", "convolution 5 5 1 elu");		// 5x5 kernel, 5 maps. stride 1. out size is 11-5+1=7 (7x7x5)
////    cnn->push_back("P1", "semi_stochastic_pool 3 1");	// pool 3x3 blocks. stride 1. outsize is 5x5x5
//    cnn->push_back("C2", "convolution 5 10 1 elu");		// 5x5 kernel, 10 maps.  out size is 3x3x10
//    cnn->push_back("C3", "convolution 3 20 1 elu");		// 5x5 kernel, 2 maps.  out size is 1x1x20
//    cnn->push_back("FC", "softmax 2");					// 'flatten' of 1x1 input is inferred

    QString t;
//    t=QString("input ")+QString::number(patchSize)+" "+QString::number(patchSize)+" 1";
    cnn->push_back("I1", "input 21 21 1");				// 21x21x1
    cnn->push_back("C1", "convolution 11 5 1 elu");		// 11x11 kernel, 5 maps. stride 1. out size is 21-11+1=11 (11x11x5)
    cnn->push_back("C2", "convolution 5 5 1 elu");		// 5x5 kernel, 5 maps. stride 1. out size is 11-5+1=7 (7x7x5)
    cnn->push_back("C3", "convolution 5 10 1 elu");		// 5x5 kernel, 10 maps.  out size is 3x3x10
    cnn->push_back("C4", "convolution 3 20 1 elu");		// 3x3 kernel, 20 maps.  out size is 1x1x20
    cnn->push_back("FC", "softmax 2");					// 'flatten' of 1x1 input is inferred


    // connect all the layers. Call connect() manually for all layer connections if you need more exotic networks.
    cnn->connect_all();
    is_cnn_init=true;
}
void structureRefinement::preTraining(){
    int axonN=0;
    int dendN=0;
    while(1){
        if(axonN>=100 && dendN>=100)break;

        int sx,sy,tP;
        sx=float(rand())/RAND_MAX*(curData->ImageW-2)+1;
        sy=float(rand())/RAND_MAX*(curData->ImageH-2)+1;
        tP=sy*curData->ImageW+sx;
        if(axonN>=100){
            while(curData->structureData[tP]!=2){
                sx=float(rand())/RAND_MAX*(curData->ImageW-2)+1;
                sy=float(rand())/RAND_MAX*(curData->ImageH-2)+1;
                tP=sy*curData->ImageW+sx;
            }

        }
        else if(dendN>=100){
            while(curData->structureData[tP]!=3){
                sx=float(rand())/RAND_MAX*(curData->ImageW-2)+1;
                sy=float(rand())/RAND_MAX*(curData->ImageH-2)+1;
                tP=sy*curData->ImageW+sx;
            }
        }
        else{
            while(curData->structureData[tP]!=2 && curData->structureData[tP]!=3){
                sx=float(rand())/RAND_MAX*(curData->ImageW-2)+1;
                sy=float(rand())/RAND_MAX*(curData->ImageH-2)+1;
                tP=sy*curData->ImageW+sx;
            }
        }

//        structure S=getStructure(sx,sy,curData->structureData[tP],patchSize*patchSize/3,curData->structureData,false);
        QVector2D midP=getMiddlePoint(sx,sy,curData->structureData[tP]);
//        if(S.width<patchSize && S.height<patchSize)continue;
        tP=midP.y()*curData->ImageW+midP.x();
          patchSet *newPatch=getPatch(midP.x(),midP.y(),curData->structureData[tP]);
        if(newPatch->is_valid==false){
            delete newPatch;
            continue;
        }

        float *buf=new float[patchSize*patchSize];
        for(int i=patchSize*patchSize-1;i>=0;i--){
            buf[i]=newPatch->patch[i];
        }

        delete newPatch;
//        delete []S.data;
        if(curData->structureData[tP]==2){
            dendPatches.push_back(buf);
            dendN++;
        }
        if(curData->structureData[tP]==3){
//            for(int i=0;i<newPatches.size();i++){
//                axonPatches.push_back(newPatches[i]);
//            }
//            axonN+=newPatches.size();
            axonPatches.push_back(buf);
            axonN++;
        }
    }
    qDebug()<<"patch generated";


    int id=0,ia=0;
    while(1){
//        qDebug()<<id<<ia;
        if(id>=dendN && ia>=axonN)break;
        if(id<dendN){
            trainingData.push_back(dendPatches[id]);
            trainingLabel.push_back(0);
            id++;
        }
        if(ia<axonN){
            trainingData.push_back(axonPatches[ia]);
            trainingLabel.push_back(1);
            ia++;
        }
    }
    qDebug()<<"data generate finished";

//    qDebug()<<trainingData.size()<<trainingLabel.size();
    while(1){
        modelTraining(dendN+axonN,trainingData,trainingLabel);
        qDebug()<<getModelAccuracy(dendN+axonN,trainingData,trainingLabel);
        if (cnn->elvis_left_the_building())
        {
            qDebug()<<"Initial training accuracy with training set: "<<getModelAccuracy(dendN+axonN,trainingData,trainingLabel);
            break;
        }
    }
    is_pre_trained=true;
}


void structureRefinement::patchTraining(patchSet patch){

}

structure structureRefinement::getStructure(int sx, int sy, int l, int maxPNum, unsigned char*structureData, bool eraseFlag){  //l: -1 for all label

    bool *ck=new bool[curData->ImageW * curData->ImageH]();

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    int cnt=1;
    QQueue<int> quex,quey;
    quex.enqueue(sx);
    quey.enqueue(sy);
    QQueue<int> quex2,quey2;
    quex2.enqueue(sx);
    quey2.enqueue(sy);
    ck[sy*curData->ImageW+sx]=true;


    float midx=sx;
    float midy=sy;

    int minx=sx;
    int miny=sy;
    int maxx=sx;
    int maxy=sy;

    while(!quex.empty()){
        if(cnt>=maxPNum)break;
        if(maxx-minx>50 || maxy-miny>50)break;
        int curx=quex.dequeue();
        int cury=quey.dequeue();

        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<0 || curx+dxylist[d][0]>=curData->ImageW || cury+dxylist[d][1]<0 || cury+dxylist[d][1]>curData->ImageH){
                continue;
            }
            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            if(ck[tP]==false){
                ck[tP]=true;
                if(structureData[tP]==l || (structureData[tP]==2 && l==-1) || (structureData[tP]==3 && l==-1)){
                    quex.enqueue(curx+dxylist[d][0]);
                    quey.enqueue(cury+dxylist[d][1]);
                    quex2.enqueue(curx+dxylist[d][0]);
                    quey2.enqueue(cury+dxylist[d][1]);
                    midx+=curx+dxylist[d][0];
                    midy+=cury+dxylist[d][1];
                    cnt++;
                    if(curx+dxylist[d][0]>maxx)maxx=curx+dxylist[d][0];
                    if(curx+dxylist[d][0]<minx)minx=curx+dxylist[d][0];
                    if(cury+dxylist[d][1]>maxy)maxy=cury+dxylist[d][1];
                    if(cury+dxylist[d][1]<miny)miny=cury+dxylist[d][1];
                }
            }
        }
    }
    delete []ck;

    midx/=cnt;
    midy/=cnt;

    structure outStructure;

    outStructure.width=maxx-minx+1;
    outStructure.height=maxy-miny+1;
    outStructure.minx=minx;
    outStructure.miny=miny;
//    if(outStructure.width>15 && outStructure.height<15){
//        int l_pad=(15-outStructure.height)/2;
//        int r_pad=(15-outStructure.height)-l_pad;
//        unsigned char *newData=new unsigned char[outStructure.width*15]();
//        for(int iy=l_pad;iy<l_pad+outStructure.height;iy++){
//            for(int ix=0;ix<outStructure.width;ix++){
//                newData[iy*outStructure.width+ix]=outStructure.data[(iy-l_pad)*outStructure.width+ix];
//            }
//        }
//        outStructure.height=15;
//        delete []outStructure.data;
//        outStructure.data=newData;
//    }
    outStructure.cnt=cnt;
    float minD=100000;

    outStructure.data=new unsigned short[outStructure.width*outStructure.height]();
    while(!quex2.empty()){
        int ix=quex2.dequeue();
        int iy=quey2.dequeue();
        if(eraseFlag)structureData[iy*curData->ImageW+ix]=0;
        outStructure.data[(iy-miny)*outStructure.width+(ix-minx)]=curData->imageData1[iy*curData->ImageW+ix];
        if(abs(midx-ix)+abs(midy-iy)<minD){
            outStructure.midx=ix;
            outStructure.midy=iy;
            minD=abs(midx-ix)+abs(midy-iy);
        }
    }
    return outStructure;
}
std::vector<float*> structureRefinement::getPatches(structure S){ //not used for now

    std::vector<float*> patches;

    QVector <QVector2D> vertexs;
    QVector <QVector2D> boundaries;
    QVector <float> deep;
    QVector <int> deepIndex;

    QVector2D midPoint(0,0);
    int startPointIndex=0;
    int endPointIndex=0;

    for(int iy=0;iy<S.height;iy++){
        for(int ix=0;ix<S.width;ix++){
            int tP=iy*S.width+ix;
            if(S.data[tP]!=0){
                if(iy==0 || iy==S.height-1 || ix==0 || ix==S.width-1){
                    boundaries.push_back(QVector2D(ix,iy));
                }
                else{
                    if(S.data[tP-1]==0 ||
                        S.data[tP+1]==0 ||
                        S.data[tP-S.width]==0 ||
                        S.data[tP+S.width]==0){
                        boundaries.push_back(QVector2D(ix,iy));
                    }
                    else{
                        vertexs.push_back(QVector2D(ix,iy));
                    }
                }
            }
        }
    }


    for(int i=0;i<vertexs.length();i++){
        deep.push_back(10000);
        deepIndex.push_back(-1);
        for(int j=0;j<boundaries.length();j++){
            float dis=(vertexs[i]-boundaries[j]).length();
            if(dis<deep[i]){
                deep[i]=dis;
                deepIndex[i]=j;
            }
        }
    }

    for(int i=0;i<vertexs.length();i++){
        midPoint+=vertexs[i];
    }
    for(int i=0;i<boundaries.length();i++){
        midPoint+=boundaries[i];
    }
    midPoint/=vertexs.length()+boundaries.length();

    float maxDis=-1;
    for(int i=0;i<boundaries.length();i++){
        float dis=(boundaries[i]-midPoint).length();
        if(dis>maxDis){
            maxDis=dis;
            startPointIndex=i;
        }
    }

    maxDis=-1;
    for(int i=0;i<boundaries.length();i++){
        float dis=(boundaries[i]-boundaries[startPointIndex]).length();
        if(dis>maxDis){
            maxDis=dis;
            endPointIndex=i;
        }
    }


    if(startPointIndex==endPointIndex)return patches;


    QVector3D axis=QVector3D::crossProduct(QVector3D(boundaries[startPointIndex]-boundaries[endPointIndex],0),QVector3D(0,1,0));
    axis.normalize();


    float a=(boundaries[startPointIndex]-boundaries[endPointIndex]+QVector2D(0,1)).length();
    float b=(boundaries[startPointIndex]-boundaries[endPointIndex]).length();
    float c=1;
    float angle=acos((b*b+1-a*a)/(2*b));
    if(b==0)angle=0;

    //qDebug()<<axis.x()<<axis.y()<<axis.z();

    angle*=axis.z();

    if(axis.z()==0)angle=0;

    //qDebug()<<angle;

    QVector <forSort3> rotated_vertexs;
    float minY=1000;
    for(int i=0;i<boundaries.length();i++){
        forSort3 t;
        t.value=QVector2D(boundaries[i].x()*cos(angle)-boundaries[i].y()*sin(angle),
                          boundaries[i].y()*cos(angle)+boundaries[i].x()*sin(angle));
        t.yvalue=t.value.y();
        if(minY>t.yvalue)minY=t.yvalue;
        t.deep=0;
        t.ind1=i;
        rotated_vertexs.push_back(t);
    }
    for(int i=0;i<vertexs.length();i++){
        forSort3 t;
        t.value=QVector2D(vertexs[i].x()*cos(angle)-vertexs[i].y()*sin(angle),
                          vertexs[i].y()*cos(angle)+vertexs[i].x()*sin(angle));
        t.yvalue=t.value.y();
        if(minY>t.yvalue)minY=t.yvalue;
        t.deep=deep[i];
        t.ind2=i;
        rotated_vertexs.push_back(t);
    }
    qSort(rotated_vertexs);

    QVector2D prevP=boundaries[startPointIndex];
    QVector2D prevDirection=QVector2D(-123,-123);
    float maxDeep=-1;
    int ind=0;
    float curH=10;
    for(int i=0;i<rotated_vertexs.length();i++){
        //mitoLineImage[int((rotated_vertexs[i].value.y()+m.ymin))*ImageW+int(rotated_vertexs[i].value.x()+m.xmin)]=255;

        if(rotated_vertexs[i].yvalue>curH+minY){

            QVector2D direction;
            if(rotated_vertexs[ind].ind1==-1){
                direction=(prevDirection+(vertexs[rotated_vertexs[ind].ind2]-prevP).normalized())*0.5;
            }
            else{
                direction=(prevDirection+(boundaries[rotated_vertexs[ind].ind1]-prevP).normalized())*0.5;
            }

            if(prevDirection.x()!=-123){
                QVector3D axis=QVector3D::crossProduct(QVector3D(0,1,0),QVector3D(direction.x(),direction.y(),0));
                axis.normalize();

                float a=(QVector2D(0,1)-direction).length();
                float angle=acos((2-a*a)/2);

                //qDebug()<<axis.x()<<axis.y()<<axis.z();
                angle*=axis.z();
                if(axis.z()==0)angle=0;

//                QVector2D p1=QVector2D(5*cos(angle)-5*sin(angle),5*cos(angle)+5*sin(angle))+prevP;
//                QVector2D p2=QVector2D(5*cos(angle)+5*sin(angle),-5*cos(angle)+5*sin(angle))+prevP;
//                QVector2D p3=QVector2D(-5*cos(angle)-5*sin(angle),5*cos(angle)-5*sin(angle))+prevP;
//                QVector2D p4=QVector2D(-5*cos(angle)+5*sin(angle),-5*cos(angle)-5*sin(angle))+prevP;
//                if(p1.x()>=0 && p2.x()>=0 && p3.x()>=0 && p4.x()>=0
//                        && p1.x()<S.width && p2.x()<S.width && p3.x()<S.width && p4.x()<S.width
//                        && p1.y()>=0 && p2.y()>=0 && p3.y()>=0 && p4.y()>=0
//                        && p1.y()<S.height && p2.y()<S.height && p3.y()<S.height && p4.y()<S.height){
                    float *newPatch=new float[patchSize*patchSize]();
                    for(int iy=0;iy<patchSize;iy++){
                        for(int ix=0;ix<patchSize;ix++){
                            newPatch[iy*patchSize+ix]=0;
                            int px=(ix-halfpatchSize)*cos(angle)-(iy-halfpatchSize)*sin(angle)+prevP.x();
                            int py=(iy-halfpatchSize)*cos(angle)+(ix-halfpatchSize)*sin(angle)+prevP.y();
                            if(px>=0 && px<S.width && py>=0 && py<S.height){
//                                if(S.data[py*S.width+px]==1){
//                                    newPatch[iy*patchSize+ix]=1;
//                                }
                                newPatch[iy*patchSize+ix]=float(S.data[py*S.width+px])/255;

                            }
                        }
                    }
                    patches.push_back(newPatch);
//                }
            }

            if(rotated_vertexs[ind].ind1==-1){
                prevDirection=(vertexs[rotated_vertexs[ind].ind2]-prevP).normalized();
                prevP=vertexs[rotated_vertexs[ind].ind2];
            }
            else{
                prevDirection=(boundaries[rotated_vertexs[ind].ind1]-prevP).normalized();
                prevP=boundaries[rotated_vertexs[ind].ind1];
            }
            maxDeep=-1;
            curH+=10;
        }

        if(rotated_vertexs[i].deep>maxDeep){
            maxDeep=rotated_vertexs[i].deep;
            ind=i;
        }
    }
    return patches;
}

patchSet *structureRefinement::getPatch(int sx,int sy,int l){

    patchSet* patch=new patchSet();

    patch->index=*curData->global_patch_ind;
    (*curData->global_patch_ind)+=1;

    structure S=getStructure(sx,sy,l,patchSize*patchSize*0.25,curData->structureData,false);

    QVector <QVector2D> vertexs;
    QVector <QVector2D> boundaries;
    QVector <float> deep;
    QVector <int> deepIndex;

    QVector2D midPoint(0,0);
    int startPointIndex=0;
    int endPointIndex=0;

    for(int iy=0;iy<S.height;iy++){
        for(int ix=0;ix<S.width;ix++){
            int tP=iy*S.width+ix;
            if(S.data[tP]!=0){
                if(iy==0 || iy==S.height-1 || ix==0 || ix==S.width-1){
                    boundaries.push_back(QVector2D(ix,iy));
                }
                else{
                    if(S.data[tP-1]==0 ||
                        S.data[tP+1]==0 ||
                        S.data[tP-S.width]==0 ||
                        S.data[tP+S.width]==0){
                        boundaries.push_back(QVector2D(ix,iy));
                    }
                    else{
                        vertexs.push_back(QVector2D(ix,iy));
                    }
                }
            }
        }
    }


    for(int i=0;i<vertexs.length();i++){
        deep.push_back(10000);
        deepIndex.push_back(-1);
        for(int j=0;j<boundaries.length();j++){
            float dis=(vertexs[i]-boundaries[j]).length();
            if(dis<deep[i]){
                deep[i]=dis;
                deepIndex[i]=j;
            }
        }
    }

    for(int i=0;i<vertexs.length();i++){
        midPoint+=vertexs[i];
    }
    for(int i=0;i<boundaries.length();i++){
        midPoint+=boundaries[i];
    }
    midPoint/=vertexs.length()+boundaries.length();

    float maxDis=-1;
    for(int i=0;i<boundaries.length();i++){
        float dis=(boundaries[i]-midPoint).length();
        if(dis>maxDis){
            maxDis=dis;
            startPointIndex=i;
        }
    }

    maxDis=-1;
    for(int i=0;i<boundaries.length();i++){
        float dis=(boundaries[i]-boundaries[startPointIndex]).length();
        if(dis>maxDis){
            maxDis=dis;
            endPointIndex=i;
        }
    }


    if(startPointIndex==endPointIndex){
        delete []S.data;
        return patch;
    }


    QVector3D axis=QVector3D::crossProduct(QVector3D(boundaries[startPointIndex]-boundaries[endPointIndex],0),QVector3D(0,1,0));
    axis.normalize();


    float a=(boundaries[startPointIndex]-boundaries[endPointIndex]-QVector2D(0,1)).length();
    float b=(boundaries[startPointIndex]-boundaries[endPointIndex]).length();
    float c=1;
    float angle=acos((b*b+1-a*a)/(2*b));
    if(b==0)angle=0;

    //qDebug()<<axis.x()<<axis.y()<<axis.z();

    if(axis.z()<0)angle*=-1;
//    angle*=axis.z();

//    if(axis.z()==0)angle=0;

    //qDebug()<<angle;

    QVector <forSort3> rotated_vertexs;
    float minY=1000;
    for(int i=0;i<boundaries.length();i++){
        forSort3 t;
        t.value=QVector2D(boundaries[i].x()*cos(angle)-boundaries[i].y()*sin(angle),
                          boundaries[i].y()*cos(angle)+boundaries[i].x()*sin(angle));
        t.yvalue=t.value.y();
        if(minY>t.yvalue)minY=t.yvalue;
        t.deep=0;
        t.ind1=i;
        rotated_vertexs.push_back(t);
    }
    for(int i=0;i<vertexs.length();i++){
        forSort3 t;
        t.value=QVector2D(vertexs[i].x()*cos(angle)-vertexs[i].y()*sin(angle),
                          vertexs[i].y()*cos(angle)+vertexs[i].x()*sin(angle));
        t.yvalue=t.value.y();
        if(minY>t.yvalue)minY=t.yvalue;
        t.deep=deep[i];
        t.ind2=i;
        rotated_vertexs.push_back(t);
    }
    qSort(rotated_vertexs);

    QVector2D O_targetP=QVector2D(sx-S.minx,sy-S.miny);


    QVector2D targetP=QVector2D(O_targetP.x()*cos(angle)-O_targetP.y()*sin(angle),
                                                O_targetP.y()*cos(angle)+O_targetP.x()*sin(angle));
    QVector2D P1,P2;

    float maxDeep=-1;
    int ind=0;

    for(int i=0;i<rotated_vertexs.length();i++){
        if(rotated_vertexs[i].yvalue>targetP.y()-10 && rotated_vertexs[i].yvalue<targetP.y()-5){
            if(rotated_vertexs[i].deep>maxDeep){
                maxDeep=rotated_vertexs[i].deep;
                ind=i;
            }
        }
    }
    if(rotated_vertexs[ind].ind1==-1){
        P1=vertexs[rotated_vertexs[ind].ind2];
    }
    else{
        P1=boundaries[rotated_vertexs[ind].ind1];
    }

    maxDeep=-1;
    ind=0;

    for(int i=0;i<rotated_vertexs.length();i++){
        if(rotated_vertexs[i].yvalue<targetP.y()+10 && rotated_vertexs[i].yvalue>targetP.y()+5){
            if(rotated_vertexs[i].deep>maxDeep){
                maxDeep=rotated_vertexs[i].deep;
                ind=i;
            }
        }
    }
    if(rotated_vertexs[ind].ind1==-1){
        P2=vertexs[rotated_vertexs[ind].ind2];
    }
    else{
        P2=boundaries[rotated_vertexs[ind].ind1];
    }



    {
        QVector2D direction=(O_targetP-P1).normalized()*0.5+(P2-O_targetP).normalized()*0.5;

        QVector3D axis=QVector3D::crossProduct(QVector3D(0,1,0),QVector3D(direction.x(),direction.y(),0));
        axis.normalize();

        float a=(QVector2D(0,1)-direction).length();
        float angle=acos((2-a*a)/2);

        //qDebug()<<axis.x()<<axis.y()<<axis.z();
        angle*=axis.z();
        if(axis.z()==0)angle=0;

        for(int iy=0;iy<patchSize;iy++){
            for(int ix=0;ix<patchSize;ix++){
                int px=(ix-halfpatchSize)*cos(angle)-(iy-halfpatchSize)*sin(angle)+O_targetP.x();
                int py=(iy-halfpatchSize)*cos(angle)+(ix-halfpatchSize)*sin(angle)+O_targetP.y();
                if(px>=0 && px<S.width && py>=0 && py<S.height){
//                    patch->patch[iy*patchSize+ix]=float(S.data[py*S.width+px])/65535;
//                    if(patch->patch[iy*patchSize+ix]!=0){
//                        int px=(ix-halfpatchSize)*cos(angle)-(iy-halfpatchSize)*sin(angle)+sx;
//                        int py=(iy-halfpatchSize)*cos(angle)+(ix-halfpatchSize)*sin(angle)+sy;
//                        if(patch->label_type==0)patch->label_type=curData->structureData[py*curData->ImageW+px];
//                        else if(patch->label_type==2 && curData->structureData[py*curData->ImageW+px]==3)patch->label_type=4;
//                        else if(patch->label_type==3 && curData->structureData[py*curData->ImageW+px]==2)patch->label_type=4;
//                    }

                    int px=(ix-halfpatchSize)*cos(angle)-(iy-halfpatchSize)*sin(angle)+sx;
                    int py=(iy-halfpatchSize)*cos(angle)+(ix-halfpatchSize)*sin(angle)+sy;
                    patch->patch[iy*patchSize+ix]=float(curData->imageData1[py*curData->ImageW+px])/65535;


                }
            }
        }
        for(int iy=0;iy<patchSize;iy++){
            for(int ix=0;ix<patchSize;ix++){
                int px=(ix-halfpatchSize)*cos(angle)-(iy-halfpatchSize)*sin(angle)+sx;
                int py=(iy-halfpatchSize)*cos(angle)+(ix-halfpatchSize)*sin(angle)+sy;
                if(px>=0 && px<curData->ImageW && py>=0 && py<curData->ImageH){
                    if(px<patch->boundMin.x())patch->boundMin.setX(px);
                    if(px>patch->boundMax.x())patch->boundMax.setX(px);
                    if(py<patch->boundMin.y())patch->boundMin.setY(py);
                    if(py>patch->boundMax.y())patch->boundMax.setY(py);

                }
            }
        }

        patch->patchCorner[0]=QVector2D((-halfpatchSize)*cos(angle)-(-halfpatchSize)*sin(angle)+sx,
                                 (-halfpatchSize)*cos(angle)+(-halfpatchSize)*sin(angle)+sy);

        patch->patchCorner[1]=QVector2D((-halfpatchSize)*cos(angle)-(halfpatchSize)*sin(angle)+sx,
                                 (halfpatchSize)*cos(angle)+(-halfpatchSize)*sin(angle)+sy);

        patch->patchCorner[2]=QVector2D((halfpatchSize)*cos(angle)-(halfpatchSize)*sin(angle)+sx,
                                 (halfpatchSize)*cos(angle)+(halfpatchSize)*sin(angle)+sy);

        patch->patchCorner[3]=QVector2D((halfpatchSize)*cos(angle)-(-halfpatchSize)*sin(angle)+sx,
                                 (-halfpatchSize)*cos(angle)+(halfpatchSize)*sin(angle)+sy);

        patch->angle=angle;
        patch->midx=sx;
        patch->midy=sy;

    }

    delete []S.data;

    patch->is_valid=true;

    return patch;
}


float structureRefinement::getModelAccuracy(int n,std::vector<float*> test_data,std::vector<int> test_label){
    int correct = 0;
    for (int k = 0; k<n; k++){
        int prediction = cnn->predict_class(test_data[k]);
        if (prediction == test_label[k]) correct += 1;
    }
    return (float)correct / n * 100;
}
int structureRefinement::getClass(float* data){
    return cnn->predict_class(data);
}
float* structureRefinement::getProbability(float* data){
    return cnn->forward(data);
}
void structureRefinement::modelTraining(int n,std::vector<float*> data,std::vector<int> label){
//    cnn->start_epoch("cross_entropy");
    qDebug()<<"training start";
    cnn->start_epoch("mse");
    for (int k = 0; k<n; k++){
//        qDebug()<<k;
        cnn->train_class(data[k], label[k]);
    }
    cnn->end_epoch();
}

// k, s, b, j, l
void structureRefinement::getCandidatePoints(){

    bool *ck=new bool[curData->ImageW * curData->ImageH]();
    for(int i=0;i<candidatePoints.size();i++){
        delete candidatePoints[i];
    }
    candidatePoints.clear();
    for(int iy=1;iy<curData->ImageH-1;iy++){
        for(int ix=1;ix<curData->ImageW-1;ix++){
            int tP=iy*curData->ImageW + ix;
            if(ck[tP])continue;
            if(curData->structureData[tP]==2){
                if(curData->structureData[tP-1]==3 ||
                    curData->structureData[tP+1]==3 ||
                    curData->structureData[tP-curData->ImageW]==3 ||
                    curData->structureData[tP+curData->ImageW]==3){
                    QVector2D newP=getCandidateMiddlePoint(ix,iy,ck);
                    patchSet *newPatch=getPatch(newP.x(),newP.y(),-1);
                    if(newPatch->is_valid)candidatePoints.enqueue(newPatch);
                    else{
                        delete newPatch;
                    }
//                    candidatePoints.enqueue(getCandidateMiddlePoint(ix,iy,ck));
                }
            }
            else if(curData->structureData[tP]==3){
                if(curData->structureData[tP-1]==2 ||
                    curData->structureData[tP+1]==2 ||
                    curData->structureData[tP-curData->ImageW]==2 ||
                    curData->structureData[tP+curData->ImageW]==2){
                    QVector2D newP=getCandidateMiddlePoint(ix,iy,ck);
                    patchSet *newPatch=getPatch(newP.x(),newP.y(),-1);
                    if(newPatch->is_valid)candidatePoints.enqueue(newPatch);
                    else{
                        delete newPatch;
                    }
//                    candidatePoints.enqueue(getCandidateMiddlePoint(ix,iy,ck));
                }
            }
        }
    }
    delete []ck;

}
void structureRefinement::doPredict(){

//    for(int i=0;i<candidatePoints.size();i++){
//        int curP=candidatePoints[i]->midy * curData->ImageW + candidatePoints[i]->midx;
//        if(tracingMap[curP].type==0){
//            curP=tracingMap[curP].posy * curData->ImageW + tracingMap[curP].posx;
//        }
//        for(int j=0;j<tracingMap[curP].connected_nodes.size();j++){



//        }
//    }


//        while(!candidatePoints.empty()){
//            cleanCnt++;
//            patchSet *patch=candidatePoints.dequeue();
//            QVector2D curPoint=QVector2D(patch->midx,patch->midy);
//            bool valid=false;
//            //check whether the current target point is valid or not
//            {
//                int tP=curPoint.y()*curData->ImageW+curPoint.x();
//                if(curData->structureData[tP]==2){
//                    if(curData->structureData[tP-1]==3 ||
//                        curData->structureData[tP+1]==3 ||
//                        curData->structureData[tP-curData->ImageW]==3 ||
//                        curData->structureData[tP+curData->ImageW]==3){
//                        valid=true;
//                    }
//                }
//                else if(curData->structureData[tP]==3){
//                    if(curData->structureData[tP-1]==2 ||
//                        curData->structureData[tP+1]==2 ||
//                        curData->structureData[tP-curData->ImageW]==2 ||
//                        curData->structureData[tP+curData->ImageW]==2){
//                        valid=true;
//                    }
//                }
//            }
//            if(valid==false){
//                delete patch;
//                continue;
//            }


////            qDebug()<<"patch generated : "<<patch->midx<<" "<<patch->midy;

//            float *probability=getProbability(patch->patch);
//            int newlabel;
//            if(probability[0]>curData->correction_threshold && curData->correctorEnable_dend)newlabel=2;
//            else if(probability[1]>curData->correction_threshold && curData->correctorEnable_axon)newlabel=3;
//            else{
//                candidatePoints.enqueue(patch);
////                emit correctionChanged();
//                continue;
//            }
////            qDebug()<<"new label: "<<newlabel;



//            QQueue<int> quex2,quey2;
//            //structure update by connected component and inclusion test
//            {
//                int tW=patch->boundMax.x()-patch->boundMin.x()+1;
//                int tH=patch->boundMax.y()-patch->boundMin.y()+1;

//                bool *ck=new bool[tW*tH]();
//                int dxylist[4][2]={{-1,0},{1,0},{0,-1},{0,1}};

//                QQueue<int> quex,quey;
//                quex.enqueue(curPoint.x());
//                quey.enqueue(curPoint.y());
//                quex2.enqueue(curPoint.x());
//                quey2.enqueue(curPoint.y());
//                curData->structureData[int(curPoint.y()*curData->ImageW+curPoint.x())]=newlabel;
//                ck[int((curPoint.y()-patch->boundMin.y())*tW+curPoint.x()-patch->boundMin.x())]=true;

//                while(!quex.empty()){
//                    int curx=quex.dequeue();
//                    int cury=quey.dequeue();
//                    for(int d=0;d<4;d++){
//                        if(curx+dxylist[d][0]<patch->boundMin.x() || curx+dxylist[d][0]>patch->boundMax.x()
//                                || cury+dxylist[d][1]<patch->boundMin.y() || cury+dxylist[d][1]>patch->boundMax.y()){
//                            continue;
//                        }
//                        if(isInside(QVector2D(curx,cury),patch->patchCorner)==false)continue;

//                        int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
//                        int tP2=(cury+dxylist[d][1]-patch->boundMin.y())*tW+curx+dxylist[d][0]-patch->boundMin.x();
//                        if(ck[tP2]==false){
//                            ck[tP2]=true;
//                            if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
//                                curData->structureData[tP]=newlabel;
//                                quex.enqueue(curx+dxylist[d][0]);
//                                quey.enqueue(cury+dxylist[d][1]);
//                                quex2.enqueue(curx+dxylist[d][0]);
//                                quey2.enqueue(cury+dxylist[d][1]);
//                            }
//                        }
//                    }
//                }
//                delete []ck;
//            }



//            for(int iy=patch->boundMin.y();iy<=patch->boundMax.y();iy++){
//                for(int ix=patch->boundMin.x();ix<=patch->boundMax.x();ix++){
//                    if(isInside(QVector2D(ix,iy),patch->patchCorner)){
//                        int tP=iy*curData->ImageW+ix;
//                        if(curData->correctionData[tP]!=3)curData->correctionData[tP]=2;
//                    }
//                }
//            }

//            delete patch;
//            qDebug()<<"structure corrected";

//            bool *ck=new bool[curData->ImageW * curData->ImageH]();

//            while(!quex2.empty()){
//                int ix=quex2.dequeue();
//                int iy=quey2.dequeue();
//                if(ix==0 || iy==0|| ix==curData->ImageW-1 || iy==curData->ImageH-1){
//                    qDebug()<<"oops";
//                    continue;
//                }
//                int tP=iy*curData->ImageW+ix;
//                if(ck[tP])continue;
//                if(curData->structureData[tP]==2){
//                    if(curData->structureData[tP-1]==3 ||
//                        curData->structureData[tP+1]==3 ||
//                        curData->structureData[tP-curData->ImageW]==3 ||
//                        curData->structureData[tP+curData->ImageW]==3){
//                        QVector2D newP=getCandidateMiddlePoint(ix,iy,ck);
//                        patchSet *newPatch=getPatch(newP.x(),newP.y(),-1);
//                        if(newPatch->is_valid)candidatePoints.push_back(newPatch);
//                        else{
//                            delete newPatch;
//                        }
//                    }
//                }
//                else if(curData->structureData[tP]==3){
//                    if(curData->structureData[tP-1]==2 ||
//                        curData->structureData[tP+1]==2 ||
//                        curData->structureData[tP-curData->ImageW]==2 ||
//                        curData->structureData[tP+curData->ImageW]==2){
//                        QVector2D newP=getCandidateMiddlePoint(ix,iy,ck);
//                        patchSet *newPatch=getPatch(newP.x(),newP.y(),-1);
//                        if(newPatch->is_valid)candidatePoints.push_back(newPatch);
//                        else{
//                            delete newPatch;
//                        }
////                        candidatePoints.push_back(getCandidateMiddlePoint(ix,iy,ck));
//                    }
//                }
//            }

//            qDebug()<<"found new target point";
//            emit structureChanged();
//            emit correctionChanged();


//            noChangeCount=0;

////            QThread::msleep(100);
//            delete []ck;
//        }

//        labelCleaning();

//        emit structureChanged();
//        emit correctionChanged();

//    }

//    for(int i=0;i<candidatePoints.size();i++){
//        patchSet* patch=candidatePoints[i];

//        for(int iy=patch->boundMin.y();iy<=patch->boundMax.y();iy++){
//            for(int ix=patch->boundMin.x();ix<=patch->boundMax.x();ix++){
//                if(isInside(QVector2D(ix,iy),patch->patchCorner)){
//                    int tP=iy*curData->ImageW+ix;
//                    //if(curData->correctionData[tP]!=3)
//                    curData->correctionData[tP]=1;
//                }
//            }
//        }
//    }
//    emit correctionChanged();

}
QVector2D structureRefinement::getCandidateMiddlePoint(int sx,int sy,bool *ck){

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    int cnt=1;
    QVector2D midPoint=QVector2D(sx,sy);
    QQueue<int> quex,quey;
    quex.enqueue(sx);
    quey.enqueue(sy);
    QQueue<int> quex2,quey2;
    quex2.enqueue(sx);
    quey2.enqueue(sy);

    ck[sy*curData->ImageW+sx]=true;

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();

        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<=0 || curx+dxylist[d][0]>=curData->ImageW-1 || cury+dxylist[d][1]<=0 || cury+dxylist[d][1]>=curData->ImageH-1){
                continue;
            }
            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            if(ck[tP]==false){
                ck[tP]=true;
                if(curData->structureData[tP]==2){
                    if(curData->structureData[tP-1]==3 ||
                        curData->structureData[tP+1]==3 ||
                        curData->structureData[tP-curData->ImageW]==3 ||
                        curData->structureData[tP+curData->ImageW]==3){
                        quex.enqueue(curx+dxylist[d][0]);
                        quey.enqueue(cury+dxylist[d][1]);
                        quex2.enqueue(curx+dxylist[d][0]);
                        quey2.enqueue(cury+dxylist[d][1]);

                        midPoint+=QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]);
                        cnt++;

                    }
                }
                else if(curData->structureData[tP]==3){
                    if(curData->structureData[tP-1]==2 ||
                        curData->structureData[tP+1]==2 ||
                        curData->structureData[tP-curData->ImageW]==2 ||
                        curData->structureData[tP+curData->ImageW]==2){
                        quex.enqueue(curx+dxylist[d][0]);
                        quey.enqueue(cury+dxylist[d][1]);
                        quex2.enqueue(curx+dxylist[d][0]);
                        quey2.enqueue(cury+dxylist[d][1]);
                        midPoint+=QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]);
                        cnt++;
                    }
                }
            }
        }
    }
    midPoint=midPoint/cnt;
    int curx=quex2.dequeue();
    int cury=quey2.dequeue();
    QVector2D medianPoint=QVector2D(curx,cury);
    float minDis=(medianPoint-midPoint).length();
    while(!quex2.empty()){
        curx=quex2.dequeue();
        cury=quey2.dequeue();
        if(minDis>(QVector2D(curx,cury)-midPoint).length()){
            medianPoint=QVector2D(curx,cury);
            minDis=(medianPoint-midPoint).length();
        }
    }

    return medianPoint;

}


void structureRefinement::savePatches(QString path){
    int d=dendPatches.size()+axonPatches.size();
    unsigned char *patchStack=new unsigned char[patchSaveSize*patchSaveSize*d]();
    int cn=0;
    for(int i=0;i<dendPatches.size();i++){
        for(int iy=0;iy<patchSize;iy++){
            for(int ix=0;ix<patchSize;ix++){
                patchStack[cn*patchSaveSize*patchSaveSize+iy*patchSaveSize+ix]=int(dendPatches[i][iy*patchSize+ix]*255);
            }
        }
        cn++;
    }

    for(int i=0;i<axonPatches.size();i++){
        for(int iy=0;iy<patchSize;iy++){
            for(int ix=0;ix<patchSize;ix++){
                patchStack[cn*patchSaveSize*patchSaveSize+iy*patchSaveSize+ix]=int(axonPatches[i][iy*patchSize+ix]*255);
            }
        }
        cn++;
    }
    FILE *f=fopen((path+QString::number(d)+".raw").toStdString().c_str(),"wb");
    fwrite(patchStack,patchSaveSize*patchSaveSize*d,1,f);
    fclose(f);
    delete []patchStack;
}







bool structureRefinement::onSegment(QVector2D p, QVector2D q, QVector2D r)
{
    if (q.x() <= fmax(p.x(), r.x()) && q.x() >= fmin(p.x(), r.x()) &&
        q.y() <= fmax(p.y(), r.y()) && q.y() >= fmin(p.y(), r.y()))
        return true;
    return false;
}

int structureRefinement::orientation(QVector2D p, QVector2D q, QVector2D r)
{
    float val = (q.y() - p.y()) * (r.x() - q.x()) -
        (q.x() - p.x()) * (r.y() - q.y());

    if (abs(val)<0.0001) return 0;
    return (val > 0) ? 1 : 2;
}

bool structureRefinement::doIntersect(QVector2D p1, QVector2D q1, QVector2D p2, QVector2D q2)
{
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    if (o1 != o2 && o3 != o4)
        return true;

    if (o1 == 0 && onSegment(p1, p2, q1)) return true;
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false;
}

bool structureRefinement::isInside(QVector2D p,QVector2D *q)
{
    QVector2D extreme = QVector2D(10000,p.y());
    int count = 0;
    for(int i=0;i<4;i++){
        int next = (i + 1) % 4;
        if (doIntersect(q[i], q[next], p, extreme))
        {
            if (orientation(q[i], p, q[next]) == 0)
                return onSegment(q[i], p, q[next]);

            count++;
        }
    }
    if (count % 2 == 0)return false;
    else return true;
}

void structureRefinement::initialTracingFromCandidatePoints(){

    qDebug()<<"initialTracingFromCandidatePoints";

    for(int i=0;i<candidatePoints.size();i++){
        QVector2D midP=getMiddlePoint(candidatePoints[i]->midx,candidatePoints[i]->midy,-1);
        int curP=midP.y() * curData->ImageW + midP.x();
        patchSet *newPatch=getPatch(midP.x(),midP.y(),-1);
        if(!newPatch->is_valid){
            delete newPatch;
            continue;
        }
        tracingMap[curP].patch=newPatch;
        tracingMap[curP].distanceFromNode=0;
        tracingMap[curP].posx=midP.x();
        tracingMap[curP].posy=midP.y();
        tracingMap[curP].type=6;
        updateTracingMap(curP);

        tracingQ.enqueue(curP);

    }

    qDebug()<<"middle";

    while(!tracingQ.isEmpty()){
        int curP=tracingQ.dequeue();
        qDebug()<<"do tracing: "<<curP;
        doTracing(curP,-1);
    }

    qDebug()<<"end";

}
void structureRefinement::completeTracing(){
    qDebug()<<"complete tracing";
    bool *ck=new bool[curData->ImageW*curData->ImageH]();
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int p=iy*curData->ImageW+ix;
            if(tracingMap[p].type!=-1)ck[p]=true;
        }
    }

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int p=iy*curData->ImageW+ix;
            if(tracingMap[p].type==-1 && (curData->structureData[p]==2 || curData->structureData[p]==3)){
                QVector2D newP=getCandidateMiddlePoint(ix,iy,ck);
                if(newP.x()<0 || newP.y()<1 || newP.x()>=curData->ImageW || newP.y()>=curData->ImageH)continue;
                if(checkNoise(newP.x(),newP.y())){
                    int curP=newP.x()+newP.y()*curData->ImageW;
                    doTracing(curP,-1);
                }
//                for(int iy2=0;iy2<curData->ImageH;iy2++){
//                    for(int ix2=0;ix2<curData->ImageW;ix2++){
//                        int p=iy2*curData->ImageW+ix2;
//                        if(tracingMap[p].type!=-1)ck[p]=true;
//                    }
//                }
            }
        }
    }
    delete []ck;
}

bool structureRefinement::checkNoise(int posx, int posy){

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    int cnt=1;
    QQueue<int> quex,quey;
    quex.enqueue(posx);
    quey.enqueue(posy);

    QQueue<int> quex2,quey2;
    quex2.enqueue(posx);
    quey2.enqueue(posy);

    tracingMap[posy*curData->ImageW+posx].type=-2;

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();

        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<=0 || curx+dxylist[d][0]>=curData->ImageW-1 || cury+dxylist[d][1]<=0 || cury+dxylist[d][1]>=curData->ImageH-1){
                continue;
            }
            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            if(tracingMap[tP].type==-1 && (curData->structureData[tP]==2 || curData->structureData[tP]==3)){
                tracingMap[tP].type=-2;
                quex.enqueue(curx+dxylist[d][0]);
                quey.enqueue(cury+dxylist[d][1]);
                quex2.enqueue(curx+dxylist[d][0]);
                quey2.enqueue(cury+dxylist[d][1]);

                cnt++;
            }
        }
        if(cnt>15){
            break;
        }
    }


    while(!quex2.empty()){
        int curx=quex2.dequeue();
        int cury=quey2.dequeue();
        int curP=cury*curData->ImageW + curx;
        tracingMap[curP].type=-1;
    }
    if(cnt>15){
        return true;
    }
    else{
        return false;
    }
}

void structureRefinement::changeStructure(int pos, int label){
    qDebug()<<"structure correct: "<<pos;

    patchSet *patch=tracingMap[pos].patch;
    int startx=pos%curData->ImageW;
    int starty=pos/curData->ImageW;

    qDebug()<<"get patch. x:"<<startx<<",y:"<<starty;

    int tW=patch->boundMax.x()-patch->boundMin.x()+1;
    int tH=patch->boundMax.y()-patch->boundMin.y()+1;
    qDebug()<<"tw:"<<tW<<",th:"<<tH;

    if(tW<=0 || tH<=0){
        qDebug()<<"oops";
        return;
    }

    bool *ck=new bool[tW*tH]();
    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};
    qDebug()<<"1";

    QQueue<int> quex,quey;
    quex.enqueue(startx);
    quey.enqueue(starty);
    curData->structureData[pos]=label;
    ck[int((starty-patch->boundMin.y())*tW+startx-patch->boundMin.x())]=true;
    qDebug()<<"2";

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();
        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<patch->boundMin.x() || curx+dxylist[d][0]>patch->boundMax.x()
                    || cury+dxylist[d][1]<patch->boundMin.y() || cury+dxylist[d][1]>patch->boundMax.y()){
                continue;
            }
            if(isInside(QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]),patch->patchCorner)==false)continue;

            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            int tP2=(cury+dxylist[d][1]-patch->boundMin.y())*tW+curx+dxylist[d][0]-patch->boundMin.x();
            curData->predictData[tP]=0;
            if(ck[tP2]==false){
                ck[tP2]=true;
                if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                    if(tracingMap[tP].type==0 && tracingMap[tP].posx==startx && tracingMap[tP].posy==starty){
                        curData->structureData[tP]=label;
                        quex.enqueue(curx+dxylist[d][0]);
                        quey.enqueue(cury+dxylist[d][1]);
                    }
                }
            }
        }
    }
    delete []ck;

    qDebug()<<"3";


//    for(int iy=patch->boundMin.y();iy<=patch->boundMax.y();iy++){
//        for(int ix=patch->boundMin.x();ix<=patch->boundMax.x();ix++){
//            if(isInside(QVector2D(ix,iy),patch->patchCorner)){
//                int tP=iy*curData->ImageW+ix;
//                curData->correctionData[tP]=11;
//            }
//        }
//    }
    emit structureChanged();
    emit correctionChanged();
    qDebug()<<"structure corrected";
}


void structureRefinement::changeStructureOnCriticalPoint(int pos, int label){
    qDebug()<<"structure correct: "<<pos;

    patchSet *patch=tracingMap[pos].patch;
    int startx=pos%curData->ImageW;
    int starty=pos/curData->ImageW;

    qDebug()<<"get patch. x:"<<startx<<",y:"<<starty;

    int tW=patch->boundMax.x()-patch->boundMin.x()+1;
    int tH=patch->boundMax.y()-patch->boundMin.y()+1;
    qDebug()<<"tw:"<<tW<<",th:"<<tH;

    if(tW<=0 || tH<=0){
        qDebug()<<"oops";
        return;
    }

    bool *ck=new bool[tW*tH]();
    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};
    qDebug()<<"1";

    QQueue<int> quex,quey;
    quex.enqueue(startx);
    quey.enqueue(starty);
    curData->structureData[pos]=label;
    ck[int((starty-patch->boundMin.y())*tW+startx-patch->boundMin.x())]=true;
    qDebug()<<"2";

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();
        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<patch->boundMin.x() || curx+dxylist[d][0]>patch->boundMax.x()
                    || cury+dxylist[d][1]<patch->boundMin.y() || cury+dxylist[d][1]>patch->boundMax.y()){
                continue;
            }
            if(isInside(QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]),patch->patchCorner)==false)continue;

            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            int tP2=(cury+dxylist[d][1]-patch->boundMin.y())*tW+curx+dxylist[d][0]-patch->boundMin.x();
            curData->predictData[tP]=0;
            if(ck[tP2]==false){
                ck[tP2]=true;
                if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                    if(tracingMap[tP].type==0 && tracingMap[tP].posx==startx && tracingMap[tP].posy==starty){
                        curData->structureData[tP]=label;
                        quex.enqueue(curx+dxylist[d][0]);
                        quey.enqueue(cury+dxylist[d][1]);
                    }
                }
            }
        }
    }
    delete []ck;

    qDebug()<<"3";


//    for(int iy=patch->boundMin.y();iy<=patch->boundMax.y();iy++){
//        for(int ix=patch->boundMin.x();ix<=patch->boundMax.x();ix++){
//            if(isInside(QVector2D(ix,iy),patch->patchCorner)){
//                int tP=iy*curData->ImageW+ix;
//                curData->correctionData[tP]=11;
//            }
//        }
//    }
    emit structureChanged();
    emit correctionChanged();
    qDebug()<<"structure corrected";
}


void structureRefinement::changePredictData(int pos, int label){
    patchSet *patch=tracingMap[pos].patch;
    if(patch->is_valid==false)return;
    int startx=pos%curData->ImageW;
    int starty=pos/curData->ImageW;

    int tW=patch->boundMax.x()-patch->boundMin.x()+1;
    int tH=patch->boundMax.y()-patch->boundMin.y()+1;

    if(tW<=0 || tH<=0){
        return;
    }

    bool *ck=new bool[tW*tH]();
    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    QQueue<int> quex,quey;
    quex.enqueue(startx);
    quey.enqueue(starty);
    ck[int((starty-patch->boundMin.y())*tW+startx-patch->boundMin.x())]=true;
    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();
        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<patch->boundMin.x() || curx+dxylist[d][0]>patch->boundMax.x()
                    || cury+dxylist[d][1]<patch->boundMin.y() || cury+dxylist[d][1]>patch->boundMax.y()){
                continue;
            }
            if(isInside(QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]),patch->patchCorner)==false)continue;

            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            int tP2=(cury+dxylist[d][1]-patch->boundMin.y())*tW+curx+dxylist[d][0]-patch->boundMin.x();
            if(ck[tP2]==false){
                ck[tP2]=true;
                if((curData->structureData[tP]==2 || curData->structureData[tP]==3)
                        && (tracingMap[tP].type==0 && tracingMap[tP].posx==startx && tracingMap[tP].posy==starty)){
                    quex.enqueue(curx+dxylist[d][0]);
                    quey.enqueue(cury+dxylist[d][1]);
                }
                else{
                    curData->predictData[tP]=label;
                }
            }
        }
    }
    delete []ck;

    qDebug()<<"3";


    for(int iy=patch->boundMin.y();iy<=patch->boundMax.y();iy++){
        for(int ix=patch->boundMin.x();ix<=patch->boundMax.x();ix++){
            if(isInside(QVector2D(ix,iy),patch->patchCorner)){
                int tP=iy*curData->ImageW+ix;
                curData->correctionData[tP]=11;
            }
        }
    }
    emit correctionChanged();
    qDebug()<<"structure corrected";
}
void structureRefinement::doPredictByTracing(){
    qDebug()<<"do predict by tracing";

    int imageSize=curData->ImageW * curData->ImageH;
    for(int i=0;i<imageSize;i++){
        curData->predictData[i]=0;
        if(curData->correctionData[i]==11)curData->correctionData[i]=0;
    }

    bool *ck=new bool[curData->ImageW * curData->ImageH]();

    for(int i=0;i<imageSize;i++){
        int tP=i;
        if(tracingMap[tP].type<=0)continue;
        if(ck[tP])continue;
        if(tracingMap[tP].weak_nodes.size()!=0)continue;

        if(tracingMap[tP].type==1){
            ck[tP]=true;
            QQueue <int> tracingQueue;
            tracingQueue.enqueue(tracingMap[tP].posy * curData->ImageW + tracingMap[tP].posx);
            QQueue <int> segmentQueue;
            segmentQueue.enqueue(tracingMap[tP].posy * curData->ImageW + tracingMap[tP].posx);

            int dend_count=0;
            int axon_count=0;

            while(!tracingQueue.isEmpty()){
                int curP=tracingQueue.dequeue();

                int predicted_class=getClass(tracingMap[curP].patch->patch);
                if(predicted_class==0)dend_count++;
                else axon_count++;

                tracingMap[curP].predicted_label=predicted_class+2;
                for(int j=0;j<tracingMap[curP].connected_nodes.size();j++){
                    if(tracingMap[tracingMap[curP].connected_nodes[j]].connected_nodes.size()<=2){
//                    if(tracingMap[tracingMap[curP].connected_nodes[j]].type==1
//                       || tracingMap[tracingMap[curP].connected_nodes[j]].type==4){
                        if(ck[tracingMap[curP].connected_nodes[j]]==false){
                            ck[tracingMap[curP].connected_nodes[j]]=true;
                            tracingQueue.enqueue(tracingMap[curP].connected_nodes[j]);
                            segmentQueue.enqueue(tracingMap[curP].connected_nodes[j]);
                        }
                    }
                    else{ // critical point
                        int p0=curP;
                        int p1=tracingMap[curP].connected_nodes[j];
                        if(ck[p1])continue;
                        float min_angle=20;
                        int min_index=-1;
                        for(int k=0;k<tracingMap[p1].connected_nodes.size();k++){
                            int p2=tracingMap[p1].connected_nodes[k];
                            if(ck[p2])continue;
                            if(tracingMap[p2].connected_nodes.size()>2)continue;
                            QVector2D dir1(p1%curData->ImageW-p0%curData->ImageW,
                                    p1/curData->ImageW-p0/curData->ImageW);
                            QVector2D dir2(p2%curData->ImageW-p1%curData->ImageW,
                                    p2/curData->ImageW-p1/curData->ImageW);

                            float angle=acos((dir1.x()*dir2.x()+dir1.y()*dir2.y())/(dir1.length()*dir2.length()))*180/M_PI;
                            if(min_angle>angle){
                                min_angle=angle;
                                min_index=p2;
                            }
                        }
                        if(min_index!=-1){
                            segmentQueue.enqueue(p1);
                            segmentQueue.enqueue(min_index);
                            tracingQueue.enqueue(min_index);
                            ck[min_index]=true;
                        }
                    }
                }
            }
            float voted_probability=float(dend_count)/(dend_count+axon_count);
            if(voted_probability>0.9 || voted_probability<0.1){
                int new_label;
                if(voted_probability>0.9)new_label=2;
                else new_label=3;
                while(!segmentQueue.isEmpty()){
                    int curP=segmentQueue.dequeue();
                    if(tracingMap[curP].connected_nodes.size()<=2){
                        changeStructure(curP,new_label);
                        if(tracingMap[curP].weak_nodes.size()==0){
                            QQueue <int>buf;
                            deleteNode(curP,&buf);
                        }
                    }
                    else{
                        changeStructureOnCriticalPoint(curP,new_label);
                    }
                }

            }
            else{
                int new_label;
                if(voted_probability>0.5)new_label=2;
                else new_label=3;
                while(!segmentQueue.isEmpty()){
                    int curP=segmentQueue.dequeue();
//                    tracingMap[curP].predicted_label=new_label;
                    changePredictData(curP,new_label);
//                    patchSet *patch=tracingMap[curP].patch;
//                    for(int iy=patch->boundMin.y();iy<=patch->boundMax.y();iy++){
//                        for(int ix=patch->boundMin.x();ix<=patch->boundMax.x();ix++){
//                            if(isInside(QVector2D(ix,iy),patch->patchCorner)){
//                                int tPP=iy*curData->ImageW+ix;
//                                curData->predictData[tPP]=new_label+10;
//                            }
//                        }
//                    }
                }
                emit correctionChanged();

            }

        }
    }
    labelCleaning();
    getCandidatePoints();
    delete []ck;

}
void structureRefinement::doPredictWithOneTrace(int pos){
    qDebug()<<"doPredictWithOneTrace";


    bool *ck=new bool[curData->ImageW * curData->ImageH]();

    int tP=pos;

    if(tracingMap[tP].type==-1)doTracing(tP,-1);
    if(tracingMap[tP].type==-1)return;

    if(tracingMap[tP].type==1 ||  tracingMap[tP].type==0){
        QQueue <int> tracingQueue;
        tracingQueue.enqueue(tracingMap[tP].posy * curData->ImageW + tracingMap[tP].posx);

        QQueue <int> segmentQueue;
        segmentQueue.enqueue(tracingMap[tP].posy * curData->ImageW + tracingMap[tP].posx);

        ck[tracingMap[tP].posy * curData->ImageW + tracingMap[tP].posx]=true;

        int dend_count=0;
        int axon_count=0;

        while(!tracingQueue.isEmpty()){
            int curP=tracingQueue.dequeue();
            int predicted_class=getClass(tracingMap[curP].patch->patch);
            if(predicted_class==0)dend_count++;
            else axon_count++;

            tracingMap[curP].predicted_label=predicted_class+2;
            for(int j=0;j<tracingMap[curP].connected_nodes.size();j++){
                if(tracingMap[tracingMap[curP].connected_nodes[j]].connected_nodes.size()<=2){
                    if(ck[tracingMap[curP].connected_nodes[j]]==false){
                        ck[tracingMap[curP].connected_nodes[j]]=true;
                        tracingQueue.enqueue(tracingMap[curP].connected_nodes[j]);
                        segmentQueue.enqueue(tracingMap[curP].connected_nodes[j]);
                    }
                }
            }
        }
        float voted_probability=float(dend_count)/(dend_count+axon_count);
        if(voted_probability>0.9 || voted_probability<0.1){
            int new_label;
            if(voted_probability>0.9)new_label=2;
            else new_label=3;
            while(!segmentQueue.isEmpty()){
                int curP=segmentQueue.dequeue();
                changeStructure(curP,new_label);
                if(tracingMap[curP].weak_nodes.size()==0){
                    QQueue <int>buf;
                    deleteNode(curP,&buf);
                }
            }
        }
        else{
            int new_label;
            if(voted_probability>0.5)new_label=2;
            else new_label=3;
            while(!segmentQueue.isEmpty()){
                int curP=segmentQueue.dequeue();
                changePredictData(curP,new_label);
            }
            emit correctionChanged();

        }

    }
    delete []ck;

}
void structureRefinement::removeTrainingSetOfNode(int curP){
    qDebug()<<"removeTrainingSetOfNode:"<<curP;

    if(tracingMap[curP].trainingSetType==2){
        qDebug()<<"11";
        qDebug()<<tracingMap[curP].trainingSetIndex;
        qDebug()<<fixedTrainingData_dend.size();
        delete []fixedTrainingData_dend[tracingMap[curP].trainingSetIndex+3];
        delete []fixedTrainingData_dend[tracingMap[curP].trainingSetIndex+2];
        delete []fixedTrainingData_dend[tracingMap[curP].trainingSetIndex+1];
        delete []fixedTrainingData_dend[tracingMap[curP].trainingSetIndex+0];
        qDebug()<<"2";
        fixedTrainingData_dend.remove(tracingMap[curP].trainingSetIndex,4);
        qDebug()<<"3";
        fixedTrainingLabel_dend.remove(tracingMap[curP].trainingSetIndex,4);
        qDebug()<<"4";
        index_trainingData_to_node_dend.remove(tracingMap[curP].trainingSetIndex,4);
        qDebug()<<"5";

        for(int i=tracingMap[curP].trainingSetIndex;i<fixedTrainingData_dend.size();i+=4){
            tracingMap[index_trainingData_to_node_dend[i]].trainingSetIndex-=4;
        }
        qDebug()<<"6";

    }
    else if(tracingMap[curP].trainingSetType==3){
        qDebug()<<"1";
        qDebug()<<tracingMap[curP].trainingSetIndex;
        qDebug()<<fixedTrainingData_axon.size();
        delete []fixedTrainingData_axon[tracingMap[curP].trainingSetIndex+3];
        delete []fixedTrainingData_axon[tracingMap[curP].trainingSetIndex+2];
        delete []fixedTrainingData_axon[tracingMap[curP].trainingSetIndex+1];
        delete []fixedTrainingData_axon[tracingMap[curP].trainingSetIndex+0];
        qDebug()<<"2";
        fixedTrainingData_axon.remove(tracingMap[curP].trainingSetIndex,4);
        qDebug()<<"3";
        fixedTrainingLabel_axon.remove(tracingMap[curP].trainingSetIndex,4);
        qDebug()<<"4";
        index_trainingData_to_node_axon.remove(tracingMap[curP].trainingSetIndex,4);
        qDebug()<<"5";
        for(int i=tracingMap[curP].trainingSetIndex;i<fixedTrainingData_axon.size();i+=4){
            tracingMap[index_trainingData_to_node_axon[i]].trainingSetIndex-=4;
        }
        qDebug()<<"6";
    }
    tracingMap[curP].trainingSetType=-1;
    tracingMap[curP].trainingSetIndex=-1;
    qDebug()<<"end";

}
int structureRefinement::structureChangeWithOneTrace(int pos, int label){
    qDebug()<<"structureChangeWithOneTrace";

    bool is_structure_changed=false;

    bool is_training_set_changed=false;

    bool *ck=new bool[curData->ImageW * curData->ImageH]();

    int tP=pos;

//    if(tracingMap[tP].connected_nodes.size()<=2){
    QQueue <int> tracingQueue;
    tracingQueue.enqueue(tP);

    ck[tP]=true;

    while(!tracingQueue.isEmpty()){
        int curP=tracingQueue.dequeue();
        int predicted_class=tracingMap[curP].predicted_label;
        if(predicted_class==-1)continue;

        if(predicted_class!=label){ //wrong predict
            changeStructure(curP,label);
            deleteNodeList.enqueue(curP);
            is_structure_changed=true;
            if(tracingMap[curP].trainingSetIndex!=-1){
                if(tracingMap[curP].trainingSetType!=label){
                    removeTrainingSetOfNode(curP);
                    if(label==3){
                        tracingMap[curP].trainingSetType=label;
                        tracingMap[curP].trainingSetIndex=fixedTrainingLabel_axon.size();
                        trainingDataGeneration(tracingMap[curP].patch,label,curP);
                        is_training_set_changed=true;
                    }
                    else if(label==2){
                        tracingMap[curP].trainingSetType=label;
                        tracingMap[curP].trainingSetIndex=fixedTrainingLabel_dend.size();
                        trainingDataGeneration(tracingMap[curP].patch,label,curP);
                        is_training_set_changed=true;
                    }
                }
            }
            else{
                if(label==2){
                    tracingMap[curP].trainingSetIndex=fixedTrainingLabel_dend.size();
                    tracingMap[curP].trainingSetType=label;
                    trainingDataGeneration(tracingMap[curP].patch,label,curP);
                    is_training_set_changed=true;
                }
                else if(label==3){
                    tracingMap[curP].trainingSetIndex=fixedTrainingLabel_axon.size();
                    tracingMap[curP].trainingSetType=label;
                    trainingDataGeneration(tracingMap[curP].patch,label,curP);
                    is_training_set_changed=true;
                }
            }
        }
        else{ //corrected
            changeStructure(curP,label);
            deleteNodeList.enqueue(curP);
            is_structure_changed=true;
            if(tracingMap[curP].trainingSetIndex!=-1){
                removeTrainingSetOfNode(curP);
            }
        }
        for(int j=0;j<tracingMap[curP].connected_nodes.size();j++){
            if(tracingMap[tracingMap[curP].connected_nodes[j]].connected_nodes.size()<=2){
                if(ck[tracingMap[curP].connected_nodes[j]]==false){
                    ck[tracingMap[curP].connected_nodes[j]]=true;
                    tracingQueue.enqueue(tracingMap[curP].connected_nodes[j]);
                }
            }
            else{ // critical point
                int p0=curP;
                int p1=tracingMap[curP].connected_nodes[j];
                if(ck[p1])continue;
                ck[p1]=true;
                float min_angle=20;
                int min_index=-1;
                for(int k=0;k<tracingMap[p1].connected_nodes.size();k++){
                    int p2=tracingMap[p1].connected_nodes[k];
                    if(ck[p2])continue;
                    if(tracingMap[p2].connected_nodes.size()>2)continue;
                    QVector2D dir1(p1%curData->ImageW-p0%curData->ImageW,
                            p1/curData->ImageW-p0/curData->ImageW);
                    QVector2D dir2(p2%curData->ImageW-p1%curData->ImageW,
                            p2/curData->ImageW-p1/curData->ImageW);

                    float angle=acos((dir1.x()*dir2.x()+dir1.y()*dir2.y())/(dir1.length()*dir2.length()))*180/M_PI;
                    if(min_angle>angle){
                        min_angle=angle;
                        min_index=p2;
                    }
                }
                if(min_index!=-1){
                    changeStructureOnCriticalPoint(p1,label);
                    is_structure_changed=true;
                    tracingQueue.enqueue(min_index);
                    ck[min_index]=true;
                }
            }
        }
//            QQueue <int>buf;
//            deleteNode(curP,&buf);
    }
//    }
    delete []ck;
    if(is_structure_changed && is_training_set_changed){
        return 3;
    }
    if(is_structure_changed){
        return 1;
    }
    if(is_training_set_changed){
        return 2;
    }
    return 0;
}
void structureRefinement::doTracing(int startPos,int prevPos){

    QQueue <int> tracingQueue;
    QQueue <int> prevNodeQueue;
    tracingQueue.enqueue(startPos);
    prevNodeQueue.enqueue(startPos);



    bool is_weak_path_updated=false;
    while(!tracingQueue.empty()){
        int prevP=prevNodeQueue.dequeue();

        int curP=tracingQueue.dequeue();
        int curx=curP%curData->ImageW;
        int cury=curP/curData->ImageW;
        int nodeType=tracingMap[curP].type;

        if(nodeType==-1 || nodeType==6 || curP==startPos){
//            if(!newPatch->is_valid){
//                delete newPatch;
//                continue;
//            }
           if(nodeType!=6){
                patchSet *newPatch=getPatch(curx,cury,-1);
                if(!newPatch->is_valid){
                    delete newPatch;
                    continue;
                }
                tracingMap[curP].patch=newPatch;
                tracingMap[curP].distanceFromNode=0;
                tracingMap[curP].posx=curx;
                tracingMap[curP].posy=cury;

                updateTracingMap(curP);
            }
            QQueue <int> tempQueue;
            findNextNodes(curP,&tempQueue);

            int connected_size=tracingMap[curP].connected_nodes.size();
            if(connected_size==0 ||connected_size==1)
                tracingMap[curP].type=4;
            else if(connected_size==2)
                tracingMap[curP].type=1;
            else if(connected_size>2)
                tracingMap[curP].type=2;

            if(tracingMap[curP].type==1 || tracingMap[curP].type==4 || curP==startPos || true){
                while(!tempQueue.empty()){
                    int t=tempQueue.dequeue();
                    tracingQueue.enqueue(t);
                    prevNodeQueue.enqueue(curP);

                }
                if( (connected_size==1 || connected_size==0) && false){
                    for(int i=0;i<2-connected_size;i++){

                        int nextP=findNextNodeForWeakSignal(curP,prevP);
                        if(nextP!=-1){
                            is_weak_path_updated=true;
                            if(tracingMap[nextP].type==-1){
                                bool okay=true;
                                for(int k=0;k<tracingMap[curP].connected_nodes.size();k++){
                                    if(tracingMap[curP].connected_nodes[k]==nextP)okay=false;
                                }
                                if(okay){
                                    tracingMap[curP].connected_nodes.push_back(nextP);
                                    tracingMap[nextP].connected_nodes.push_back(curP);
                                    int connected_size=tracingMap[curP].connected_nodes.size();
                                    if(connected_size==0 ||connected_size==1)
                                        tracingMap[curP].type=4;
                                    else if(connected_size==2)
                                        tracingMap[curP].type=1;
                                    else if(connected_size>2)
                                        tracingMap[curP].type=2;
                                }
                                prevP=nextP;
                                tracingQueue.enqueue(nextP);
                                prevNodeQueue.enqueue(curP);

                            }
                            else{
                                bool okay=true;
                                for(int k=0;k<tracingMap[curP].connected_nodes.size();k++){
                                    if(tracingMap[curP].connected_nodes[k]==nextP)okay=false;
                                }
                                if(okay){
                                    tracingMap[curP].connected_nodes.push_back(nextP);
                                    tracingMap[nextP].connected_nodes.push_back(curP);
                                    tracingMap[curP].type=1;
                                    int connected_size=tracingMap[nextP].connected_nodes.size();
                                    if(connected_size==0 ||connected_size==1)
                                        tracingMap[nextP].type=4;
                                    else if(connected_size==2)
                                        tracingMap[nextP].type=1;
                                    else if(connected_size>2)
                                        tracingMap[nextP].type=2;
                                }
                            }
                        }
                    }

                }

            }
            else{
                QQueue <int> tempQueue2=tempQueue;
                float min_angle=20;
                int min_index=-1;
                QVector2D dir(curx - prevP%curData->ImageW,
                              cury - prevP/curData->ImageW);

                while(!tempQueue2.empty()){
                    int t=tempQueue2.dequeue();

                    QVector2D newDir(t%curData->ImageW-curx,
                            t/curData->ImageW-cury);

                    float angle=acos((dir.x()*newDir.x()+dir.y()*newDir.y())/(dir.length()*newDir.length()))*180/M_PI;
                    if(min_angle>angle){
                        min_angle=angle;
                        min_index=t;
                    }
                }
//                qDebug()<<"min angle: "<<min_angle;


                while(!tempQueue.empty()){
                    int t=tempQueue.dequeue();

                    if(t==min_index){
                        tracingQueue.enqueue(t);
                        prevNodeQueue.enqueue(curP);
                        continue;
                    }

                    patchSet *newPatch=getPatch(t%curData->ImageW,t/curData->ImageW,-1);
                    if(!newPatch->is_valid){
                        delete newPatch;
                        continue;
                    }
                    tracingMap[t].patch=newPatch;
                    tracingMap[t].distanceFromNode=0;
                    tracingMap[t].posx=t%curData->ImageW;
                    tracingMap[t].posy=t/curData->ImageW;
                    tracingMap[t].type=6;

                    updateTracingMap(t);

                }

            }

        }
        else if(nodeType==0){
            for(int k=tracingMap[curP].connected_nodes.size()-1;k>=0;k--){
                for(int kk=tracingMap[tracingMap[curP].connected_nodes[k]].connected_nodes.size()-1;kk>=0;kk--){
                    if(tracingMap[tracingMap[curP].connected_nodes[k]].connected_nodes[kk]==curP){
                        tracingMap[tracingMap[curP].connected_nodes[k]].connected_nodes.remove(kk);

                        int connected_size=tracingMap[tracingMap[curP].connected_nodes[k]].connected_nodes.size();
                        if(connected_size==0 ||connected_size==1)
                            tracingMap[tracingMap[curP].connected_nodes[k]].type=4;
                        else if(connected_size==2)
                            tracingMap[tracingMap[curP].connected_nodes[k]].type=1;
                        else if(connected_size>2)
                            tracingMap[tracingMap[curP].connected_nodes[k]].type=2;

                    }
                }
                tracingMap[curP].connected_nodes.remove(k);
            }
        }

    }

    if(is_weak_path_updated){
        curData->connectionStructureUpdate();
        emit connection_graph_updated();
    }
}

int structureRefinement::getCandidateMiddlePoint_tracing(int tx,int ty,int sx,int sy,bool *ck){

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    int cnt=1;
    QVector2D midPoint=QVector2D(sx,sy);
    QQueue<int> quex,quey;
    quex.enqueue(sx);
    quey.enqueue(sy);
    QQueue<int> quex2,quey2;
    quex2.enqueue(sx);
    quey2.enqueue(sy);
    tracingMap[sy*curData->ImageW+sx].debug_middle_node=true;

    ck[sy*curData->ImageW+sx]=true;

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();

        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<=0 || curx+dxylist[d][0]>=curData->ImageW-1 || cury+dxylist[d][1]<=0 || cury+dxylist[d][1]>=curData->ImageH-1){
                continue;
            }
            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            if(ck[tP]==false){
                ck[tP]=true;
                if((tracingMap[tP].type==-1 && (curData->structureData[tP]==2 || curData->structureData[tP]==3))
                        || (tracingMap[tP].type==0 && (tracingMap[tP].posx!=tx || tracingMap[tP].posy!=ty ))){
                    bool is_okay=false;
                    for(int d2=0;d2<8;d2++){
                        int nP=(cury+dxylist[d][1]+dxylist[d2][1])*curData->ImageW + curx+dxylist[d][0]+dxylist[d2][0];
                        if(tracingMap[nP].type==0 && tracingMap[nP].posx==tx && tracingMap[nP].posy==ty  && (curData->structureData[nP]==2 || curData->structureData[nP]==3)){
                            is_okay=true;
                        }
                    }

                    if(is_okay){
                        quex.enqueue(curx+dxylist[d][0]);
                        quey.enqueue(cury+dxylist[d][1]);
                        quex2.enqueue(curx+dxylist[d][0]);
                        quey2.enqueue(cury+dxylist[d][1]);
                        tracingMap[(cury+dxylist[d][1])*curData->ImageW+curx+dxylist[d][0]].debug_middle_node=true;

                        midPoint+=QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]);
                        cnt++;
                    }
                }
            }
        }
    }
    midPoint=midPoint/cnt;
    int curx=quex2.dequeue();
    int cury=quey2.dequeue();
    QVector2D medianPoint=QVector2D(curx,cury);
    float minDis=(medianPoint-midPoint).length();
    while(!quex2.empty()){
        curx=quex2.dequeue();
        cury=quey2.dequeue();
        if(minDis>(QVector2D(curx,cury)-midPoint).length()){
            medianPoint=QVector2D(curx,cury);
            minDis=(medianPoint-midPoint).length();
        }
    }

    return medianPoint.y()*curData->ImageW + medianPoint.x();

}

int structureRefinement::findNearestOutside(int pos){

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};
    QQueue<int> q;
    q.enqueue(pos);

    bool *ck=new bool[curData->ImageW * curData->ImageH]();
    ck[pos]=true;

    while(!q.empty()){
        int curP=q.dequeue();
        int curx=curP%curData->ImageW;
        int cury=curP/curData->ImageW;

        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<=0 || curx+dxylist[d][0]>=curData->ImageW-1 || cury+dxylist[d][1]<=0 || cury+dxylist[d][1]>=curData->ImageH-1){
                continue;
            }
            int new_x=curx+dxylist[d][0];
            int new_y=cury+dxylist[d][1];
            int tP=new_y*curData->ImageW + new_x;
            if(ck[tP]==false){
                ck[tP]=true;
                if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                    q.enqueue(tP);
                }
                else{
                    delete[]ck;
                    return tP;
                }
            }
        }
    }
    delete []ck;
    return -1;
}

int structureRefinement::getStartMiddlePoint_minimap(int sx,int sy,bool *ck){

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    int cnt=1;
    QVector2D midPoint=QVector2D(sx,sy);
    QQueue<int> quex,quey;
    quex.enqueue(sx);
    quey.enqueue(sy);
    QQueue<int> quex2,quey2;
    quex2.enqueue(sx);
    quey2.enqueue(sy);

    ck[sy*curData->ImageW+sx]=true;

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();

        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<=0 || curx+dxylist[d][0]>=curData->ImageW-1 || cury+dxylist[d][1]<=0 || cury+dxylist[d][1]>=curData->ImageH-1){
                continue;
            }
            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            if(ck[tP]==false){
                ck[tP]=true;
                if(curData->structureData[tP]==1){
                    quex.enqueue(curx+dxylist[d][0]);
                    quey.enqueue(cury+dxylist[d][1]);
                    quex2.enqueue(curx+dxylist[d][0]);
                    quey2.enqueue(cury+dxylist[d][1]);
                    midPoint+=QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]);
                    cnt++;
                }
            }
        }
    }
    midPoint=midPoint/cnt;
    int curx=quex2.dequeue();
    int cury=quey2.dequeue();
    QVector2D medianPoint=QVector2D(curx,cury);
    float minDis=(medianPoint-midPoint).length();
    while(!quex2.empty()){
        curx=quex2.dequeue();
        cury=quey2.dequeue();
        if(minDis>(QVector2D(curx,cury)-midPoint).length()){
            medianPoint=QVector2D(curx,cury);
            minDis=(medianPoint-midPoint).length();
        }
    }

    return medianPoint.y()*curData->ImageW + medianPoint.x();

}

int structureRefinement::getCandidateMiddlePoint_minimap(int sx,int sy,bool *ck){

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    int cnt=1;
    QVector2D midPoint=QVector2D(sx,sy);
    QQueue<int> quex,quey;
    quex.enqueue(sx);
    quey.enqueue(sy);
    QQueue<int> quex2,quey2;
    quex2.enqueue(sx);
    quey2.enqueue(sy);

    ck[sy*curData->ImageW+sx]=true;

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();



        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<=0 || curx+dxylist[d][0]>=curData->ImageW-1 || cury+dxylist[d][1]<=0 || cury+dxylist[d][1]>=curData->ImageH-1){
                continue;
            }
            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            if(ck[tP]==false){
                ck[tP]=true;
                if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                    bool is_okay=false;
                    for(int d2=0;d2<8;d2++){
                        int nP=(cury+dxylist[d][1]+dxylist[d2][1])*curData->ImageW + curx+dxylist[d][0]+dxylist[d2][0];
                        if(curData->structureData[nP]==1){
                            is_okay=true;
                        }
                    }

                    if(is_okay){
                        quex.enqueue(curx+dxylist[d][0]);
                        quey.enqueue(cury+dxylist[d][1]);
                        quex2.enqueue(curx+dxylist[d][0]);
                        quey2.enqueue(cury+dxylist[d][1]);

                        midPoint+=QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]);
                        cnt++;
                    }
                }

            }
        }
    }
    midPoint=midPoint/cnt;
    int curx=quex2.dequeue();
    int cury=quey2.dequeue();
    QVector2D medianPoint=QVector2D(curx,cury);
    float minDis=(medianPoint-midPoint).length();
    while(!quex2.empty()){
        curx=quex2.dequeue();
        cury=quey2.dequeue();
        if(minDis>(QVector2D(curx,cury)-midPoint).length()){
            medianPoint=QVector2D(curx,cury);
            minDis=(medianPoint-midPoint).length();
        }
    }

    return medianPoint.y()*curData->ImageW + medianPoint.x();

}


void structureRefinement::getStartingPoints_minimap_sub(int p){

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    int sx=p%curData->ImageW;
    int sy=p/curData->ImageW;
    QQueue<int> quex,quey;
    quex.enqueue(sx);
    quey.enqueue(sy);

    bool *ck=new bool[curData->ImageW * curData->ImageH]();
    ck[sy*curData->ImageW+sx]=true;

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();

        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<=0 || curx+dxylist[d][0]>=curData->ImageW-1 || cury+dxylist[d][1]<=0 || cury+dxylist[d][1]>=curData->ImageH-1){
                continue;
            }
            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            if(ck[tP]==false){
                ck[tP]=true;
                if(curData->structureData[tP]==1){
                    quex.enqueue(curx+dxylist[d][0]);
                    quey.enqueue(cury+dxylist[d][1]);
                }
                else if(curData->structureData[tP]==2 ||curData->structureData[tP]==3 ){
                    int newP=getCandidateMiddlePoint_minimap(curx+dxylist[d][0],cury+dxylist[d][1],ck);
                    tracingMap[p].connected_nodes.push_back(newP);
                    patchSet *newPatch=getPatch(newP%curData->ImageW,newP/curData->ImageW,-1);
                    if(newPatch->is_valid)candidatePoints.enqueue(newPatch);
                    else{
                        delete newPatch;
                    }

                }
            }
        }
    }

    delete []ck;
}
void structureRefinement::getStartingPoints_minimap(){

    bool *ck=new bool[curData->ImageW * curData->ImageH]();
    for(int i=0;i<candidatePoints.size();i++){
        delete candidatePoints[i];
    }
    candidatePoints.clear();
    for(int iy=1;iy<curData->ImageH-1;iy++){
        for(int ix=1;ix<curData->ImageW-1;ix++){
            int tP=iy*curData->ImageW + ix;
            if(ck[tP])continue;
            if(curData->structureData[tP]==1){
                int newP=getStartMiddlePoint_minimap(ix,iy,ck);
                tracingMap[newP].type=-2;
                getStartingPoints_minimap_sub(newP);
            }
        }
    }
    delete []ck;
}
int structureRefinement::findNextNodeForWeakSignal(int curNodePos,int prevNodePos){

//    return -1;

    float distance_thresh=10;

    QVector2D dir(curNodePos%curData->ImageW - prevNodePos%curData->ImageW,
            curNodePos/curData->ImageW - prevNodePos/curData->ImageW);
    if(curNodePos!=prevNodePos)dir=dir.normalized();
    else dir=QVector2D(0,0);

//    int ddx=round(dir.x());
//    int ddy=round(dir.y());

    int startx=curNodePos%curData->ImageW;
    int starty=curNodePos/curData->ImageW;

//    int startx_outside=curNodePos%curData->ImageW;
//    int starty_outside=curNodePos/curData->ImageW;


    qDebug()<<"weak find";
//    if(ddx!=0 || ddy!=0){
//        while(curData->structureData[starty_outside*curData->ImageW + startx_outside]!=0){
//            startx_outside+=ddx;
//            starty_outside+=ddy;
//            if(startx_outside<0 || startx_outside>=curData->ImageW
//                    || starty_outside<0 || starty_outside>=curData->ImageH){
//                return -1;
//            }
//        }
//    }
//    else{
//        int outP=findNearestOutside(curNodePos);
//        if(outP==-1)return -1;

//        startx_outside=outP%curData->ImageW;
//        starty_outside=outP/curData->ImageW;
//    }


    int imSize=curData->ImageW*curData->ImageH;
    nodeInfo *D=new nodeInfo[curData->ImageW*curData->ImageH];
    for(int i=0;i<imSize;i++){
        D[i].distance=1000000;
        D[i].fromPos=-1;
    }
    D[curNodePos].distance=0;
    D[curNodePos].fromPos=curNodePos;

//    int outsideP=starty_outside*curData->ImageW + startx_outside;
//    D[outsideP].distance=0;
//    D[outsideP].fromPos=curNodePos;


    std::priority_queue <nodeInfo,std::vector<nodeInfo>,CompareFunctionObject> workingQ;
//    workingQ.push(nodeInfo(outsideP,0));
    workingQ.push(nodeInfo(curNodePos,0));

//    qDebug()<<"start setting";

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};
//    float direction_cost[8];
//    for(int i=0;i<8;i++){
//        float d_l;
//        if(i<4)d_l=1;
//        else d_l=sqrt(2);
//        float angle=acos((dir.x()*dxylist[i][0]+dir.y()*dxylist[i][1])/(dir.length()*d_l));
//        direction_cost[i]=angle;
//        if(angle*180/M_PI>80)
//            direction_cost[i]=angle*angle*angle;
//    }

    while(!workingQ.empty()){
        nodeInfo cur=workingQ.top();
        workingQ.pop();
//        qDebug()<<"top:"<<cur.fromPos%curData->ImageW<<cur.fromPos/curData->ImageW<<cur.distance;

        int curP=cur.fromPos;
        int curx=curP%curData->ImageW;
        int cury=curP/curData->ImageW;

        int curS= curData->structureData[curP];
        if(curS==2 || curS==3){
//            if(tracingMap[curP].posx==startx && tracingMap[curP].posy==starty){
//                continue;
//            }

            if(tracingMap[curP].type==-1){

                QVector2D t=getMiddlePoint(curP%curData->ImageW,curP/curData->ImageW,-1);
                int ttP=t.y()*curData->ImageW+t.x();

                int midP=getMiddlePoint_on_fragmented_object(curP);
                if(midP!=-1){
                    ttP=midP;
                }

                if(ttP!=curP && tracingMap[ttP].type==-1){
                    D[ttP].fromPos=curP;
                    curP=ttP;
                }
                qDebug()<<"1";

            }

            if(tracingMap[curP].type!=-1 && (tracingMap[curP].posx!=startx || tracingMap[curP].posy!=starty)){
                int tempP=tracingMap[curP].posy*curData->ImageW + tracingMap[curP].posx;

                bool is_connected=false;
                for(int i=0;i<tracingMap[curNodePos].connected_nodes.size();i++){
                    if(tempP==tracingMap[curNodePos].connected_nodes[i])is_connected=true;
                }

               qDebug()<<"2";

                if(is_connected)continue;

                bool is_cycle=false;
                for(int i=0;i<tracingMap[curNodePos].connected_nodes.size();i++){
                    for(int j=0;j<tracingMap[tempP].connected_nodes.size();j++){
                        if(tracingMap[tempP].connected_nodes[j]==tracingMap[curNodePos].connected_nodes[i])is_cycle=true;
                    }
                }

                if(is_cycle){
                    qDebug()<<"cycle";
                    delete []D;
                    return -1;

                }
                qDebug()<<"3";

                QVector <int> new_path;
                new_path.push_back(tempP);
//                curData->connection_path_forward[tempP]=curP;
//                curData->connection_path_backward[tempP]=tempP;
//                int prev=tempP;
                int path_iter=curP;
                while(1){
//                    curData->connection_path_forward[path_iter]=D[path_iter].fromPos;
//                    curData->connection_path_backward[path_iter]=prev;
                    new_path.push_back(path_iter);
                    if(path_iter==curNodePos)break;
                    curData->connection_object1[path_iter]=tempP;
                    curData->connection_object2[path_iter]=curNodePos;
 //                   prev=path_iter;
                    path_iter=D[path_iter].fromPos;
                }
                qDebug()<<"4";

                tracingMap[curNodePos].weak_nodes.push_back(tempP);
                tracingMap[tempP].weak_nodes.push_back(curNodePos);
                tracingMap[curNodePos].weak_paths.push_back(new_path);
                tracingMap[tempP].weak_paths.push_back(new_path);
                qDebug()<<tempP;

                delete []D;
                return tempP;
            }
            else if(tracingMap[curP].type==-1){
                qDebug()<<"123";
                QVector <int> new_path;
                new_path.push_back(curP);
//                int prev=curP;
                int path_iter=D[curP].fromPos;
                while(1){
//                    curData->connection_path_forward[path_iter]=D[path_iter].fromPos;
//                    curData->connection_path_backward[path_iter]=prev;
                    new_path.push_back(path_iter);
                    if(path_iter==curNodePos)break;
                    curData->connection_object1[path_iter]=curP;
                    curData->connection_object2[path_iter]=curNodePos;
//                    prev=path_iter;
                    path_iter=D[path_iter].fromPos;
                }
                tracingMap[curNodePos].weak_nodes.push_back(curP);
                tracingMap[curP].weak_nodes.push_back(curNodePos);
                tracingMap[curNodePos].weak_paths.push_back(new_path);
                tracingMap[curP].weak_paths.push_back(new_path);
                qDebug()<<curP;

                delete []D;
                return curP;
            }

        }

        qDebug()<<"6";

        for(int d=0;d<8;d++){
            int newx=curx+dxylist[d][0];
            int newy=cury+dxylist[d][1];
            if(newx<0 || newx>=curData->ImageW ||newy<0 || newy>=curData->ImageH){
                continue;
            }
            int newP=newy*curData->ImageW + newx;
            QVector2D newDir(newP%curData->ImageW-startx,
                    newP/curData->ImageW-starty);

            float angle=acos((dir.x()*newDir.x()+dir.y()*newDir.y())/(dir.length()*newDir.length())); // 0: 0, 90: 1.57
            //qDebug()<<angle;

            if(dir.x()==0 && dir.y()==0)angle=0;

            float new_distance=D[curP].distance+node_cost(newP,angle);
            if(tracingMap[newP].posx==startx && tracingMap[newP].posy==starty){
                new_distance=D[curP].distance+0.000001 + 0.001 * angle * curData->user_define_cost[newP];
            }
            if(new_distance>distance_thresh){
                continue;
            }
            if(D[newP].distance>new_distance){
                D[newP].distance=new_distance;
                D[newP].fromPos=curP;
                workingQ.push(nodeInfo(newP,new_distance,cur.isDead));
//                qDebug()<<"push:"<<newP%curData->ImageW<<newP/curData->ImageW<<new_distance;

            }
        }
    }
    qDebug()<<"no";
    delete []D;
    return -1;

}
int structureRefinement::getMiddlePoint_on_fragmented_object(int pos){

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    int max_x=pos%curData->ImageW;
    int min_x=pos%curData->ImageW;
    int max_y=pos/curData->ImageW;
    int min_y=pos/curData->ImageW;

    QVector2D res(int(pos%curData->ImageW),int(pos/curData->ImageW));
    int cnt=1;
    QQueue<int> q,q2;
    q.enqueue(pos);
    q2.enqueue(pos);

    bool *ck=new bool[curData->ImageW * curData->ImageH]();
    ck[pos]=true;

    while(!q.empty()){

        if(max_x-min_x>15 || max_y-min_y>15){
            delete []ck;
            return -1;
        }

        int curP=q.dequeue();
        int curx=curP%curData->ImageW;
        int cury=curP/curData->ImageW;

        for(int d=0;d<8;d++){
            int new_x=curx+dxylist[d][0];
            int new_y=cury+dxylist[d][1];
            if(new_x<=0 || new_x>=curData->ImageW-1 ||new_y<=0 || new_y>=curData->ImageH-1){
                continue;
            }
            int tP=new_y*curData->ImageW + new_x;
            if(ck[tP]==false){
                ck[tP]=true;
                if(tracingMap[tP].type==-1){
                    if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                        q.enqueue(tP);
                        q2.enqueue(tP);
                        if(new_x<min_x)min_x=new_x;
                        if(new_x>max_x)max_x=new_x;
                        if(new_y<min_y)min_y=new_y;
                        if(new_y>max_y)max_y=new_y;

                        res+=QVector2D(new_x,new_y);
                        cnt++;
                    }
                }
            }
        }
    }
    res=res/cnt;
    int curp=q2.dequeue();
    int curx=curp%curData->ImageW;
    int cury=curp/curData->ImageW;
    QVector2D medianPoint=QVector2D(curx,cury);
    float minDis=(medianPoint-res).length();
    while(!q2.empty()){
        curp=q2.dequeue();
        curx=curp%curData->ImageW;
        cury=curp/curData->ImageW;
        if(minDis>(QVector2D(curx,cury)-res).length()){
            medianPoint=QVector2D(curx,cury);
            minDis=(medianPoint-res).length();
        }
    }
    delete []ck;
    return medianPoint.y()*curData->ImageW + medianPoint.x();
}
float structureRefinement::node_cost(int pos, float direction_cost){
    //return 1.0;
    float v=float(curData->imageData1[pos])/65535;
    if(v<0.05){
        return 100 * curData->user_define_cost[pos];
    }
    if(direction_cost>1.5){
        return (1.0/(1+exp(10*v-5)) + 3) * curData->user_define_cost[pos];
    }
    //return 1.0/(1+exp(10*v-5)) + curData->user_define_cost[pos];
    return (1.0/(1+exp(10*v-5)) + 0.01*direction_cost) * curData->user_define_cost[pos];
}


void structureRefinement::updateTracingMap(int curNodePos){

    patchSet *patch=tracingMap[curNodePos].patch;

    if(patch->is_valid==false){
        return;
    }

    int posx=curNodePos%curData->ImageW;
    int posy=curNodePos/curData->ImageW;


    int curP=posy*curData->ImageW+posx;

//    bool *ck=new bool[tW*tH]();
    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};


    QQueue<int> quex,quey;
    QQueue<float> distance;
    quex.enqueue(posx);
    quey.enqueue(posy);
    distance.enqueue(0);
    //ck[int((posy-patch->boundMin.y())*tW+posx-patch->boundMin.x())]=true;

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();
        float curDis=distance.dequeue();
        for(int d=0;d<8;d++){
            int new_x=curx+dxylist[d][0];
            int new_y=cury+dxylist[d][1];
            if(new_x<patch->boundMin.x() || new_x>patch->boundMax.x()
                    || new_y<patch->boundMin.y() || new_y>patch->boundMax.y()){
                continue;
            }
            if(isInside(QVector2D(new_x,new_y),patch->patchCorner)==false)continue;

            int tP=new_y*curData->ImageW + new_x;
            if(curData->structureData[tP]!=2 && curData->structureData[tP]!=3)continue;

            float v=float(curData->imageData1[tP])/65535;
            float newDis=curDis+1.0/(1+exp(10*v-5));
            if(tracingMap[tP].distanceFromNode>newDis){
                if(tracingMap[tP].type==-1 || tracingMap[tP].type==0){
                    quex.enqueue(curx+dxylist[d][0]);
                    quey.enqueue(cury+dxylist[d][1]);
                    distance.enqueue(newDis);
                    tracingMap[tP].type=0;
                    tracingMap[tP].posx=posx;
                    tracingMap[tP].posy=posy;
                    tracingMap[tP].distanceFromNode=newDis;//getShortestPathBetweenToPoint(curP,tP);//(posx-curx)*(posx-curx)+(posy-cury)*(posy-cury);
                }
            }
        }
    }
}
void structureRefinement::resetTracingMap(int curNodePos){

    qDebug()<<"reset tracing Map: "<<curNodePos;
    patchSet *patch=tracingMap[curNodePos].patch;

    if(patch==NULL)return;

    if(patch->is_valid==false){
        return;
    }

    int posx=curNodePos%curData->ImageW;
    int posy=curNodePos/curData->ImageW;


//    bool *ck=new bool[tW*tH]();
    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    QQueue<int> quex,quey;
    quex.enqueue(posx);
    quey.enqueue(posy);

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();
        for(int d=0;d<8;d++){
            if(curx+dxylist[d][0]<patch->boundMin.x() || curx+dxylist[d][0]>patch->boundMax.x()
                    || cury+dxylist[d][1]<patch->boundMin.y() || cury+dxylist[d][1]>patch->boundMax.y()){
                continue;
            }
            if(isInside(QVector2D(curx+dxylist[d][0],cury+dxylist[d][1]),patch->patchCorner)==false)continue;

            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            if(curData->structureData[tP]!=2 && curData->structureData[tP]!=3)continue;

            if(tracingMap[tP].type==0 && tracingMap[tP].posx==posx && tracingMap[tP].posy==posy){
                quex.enqueue(curx+dxylist[d][0]);
                quey.enqueue(cury+dxylist[d][1]);
                tracingMap[tP].type=-1;
                tracingMap[tP].distanceFromNode=1000000;
            }
        }
    }
    qDebug()<<"end";
}


float structureRefinement::getShortestPathBetweenToPoint(int start_pos, int end_pos){


    float distance_thresh=halfpatchSize;

    int imSize=curData->ImageW*curData->ImageH;
    nodeInfo *D=new nodeInfo[curData->ImageW*curData->ImageH];
    for(int i=0;i<imSize;i++){
        D[i].distance=1000000;
    }
    D[start_pos].distance=0;

    std::priority_queue <nodeInfo,std::vector<nodeInfo>,CompareFunctionObject> workingQ;
    workingQ.push(nodeInfo(start_pos,0));


    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    while(!workingQ.empty()){
        nodeInfo cur=workingQ.top();
        workingQ.pop();
//        qDebug()<<"top:"<<cur.fromPos%curData->ImageW<<cur.fromPos/curData->ImageW<<cur.distance;
        int curP=cur.fromPos;
        int curx=curP%curData->ImageW;
        int cury=curP/curData->ImageW;
        int curS= curData->structureData[curP];
        if(curS!=2 && curS!=3)continue;

        if(curP==end_pos){
            float res=D[curP].distance;
            delete []D;
            return res;
        }

        for(int d=0;d<8;d++){
            int newx=curx+dxylist[d][0];
            int newy=cury+dxylist[d][1];
            if(newx<0 || newx>=curData->ImageW ||newy<0 || newy>=curData->ImageH){
                continue;
            }
            int newP=newy*curData->ImageW + newx;

            float v=float(curData->imageData1[newP])/65535;
            float new_distance=D[curP].distance+1.0/(1+exp(10*v-5));
            if(new_distance>distance_thresh){
                delete[]D;
                return 1000000;
            }
            if(D[newP].distance>new_distance){
                D[newP].distance=new_distance;
                D[newP].fromPos=curP;
                workingQ.push(nodeInfo(newP,new_distance,cur.isDead));
            }
        }
    }
    delete []D;
    return 1000000;
}
void structureRefinement::findNextNodes(int curNodePos, QQueue<int> *out){
    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};
    bool *ck=new bool[curData->ImageW * curData->ImageH]();
    int startx=curNodePos%curData->ImageW;
    int starty=curNodePos/curData->ImageW;
    QQueue <int> quex;
    QQueue <int> quey;
    quex.enqueue(startx);
    quey.enqueue(starty);
    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();
        ck[cury*curData->ImageW + curx]=true;
        for(int d=0;d<8;d++){
            int nextx=curx+dxylist[d][0];
            int nexty=cury+dxylist[d][1];
            if(nextx<=0 || nextx>=curData->ImageW-2 || nexty<=0 || nexty>=curData->ImageH-2){
                continue;
            }
            int tP=nexty*curData->ImageW + nextx;
            if(ck[tP])continue;

            if(tracingMap[tP].type==0 && tracingMap[tP].posx==startx && tracingMap[tP].posy==starty){
                quex.enqueue(nextx);
                quey.enqueue(nexty);
            }
            if(tracingMap[tP].type==0 && (tracingMap[tP].posx!=startx || tracingMap[tP].posy!=starty)){
                int newP=tracingMap[tP].posy * curData->ImageW + tracingMap[tP].posx;
                bool okay=true;
                for(int k=0;k<tracingMap[curNodePos].connected_nodes.size();k++){
                    if(tracingMap[curNodePos].connected_nodes[k]==newP)okay=false;
                }
                if(okay){
                    tracingMap[curNodePos].connected_nodes.push_back(newP);
                    tracingMap[newP].connected_nodes.push_back(curNodePos);
                    int connected_size=tracingMap[newP].connected_nodes.size();
                    if(tracingMap[newP].type==6){
                        out->enqueue(newP);
                    }
                    else if(connected_size==0 ||connected_size==1)
                        tracingMap[newP].type=4;
                    else if(connected_size==2)
                        tracingMap[newP].type=1;
                    else if(connected_size>2)
                        tracingMap[newP].type=2;

                }

            }
            if((tracingMap[tP].type==-1 && (curData->structureData[tP]==2 || curData->structureData[tP]==3))){
//            if(tracingMap[tP].type==-1 && (curData->structureData[tP]==2 || curData->structureData[tP]==3)){
                int newP=getCandidateMiddlePoint_tracing(startx,starty,nextx,nexty,ck);
                if(tracingMap[newP].type==-1){
                    bool okay=true;
                    for(int k=0;k<tracingMap[curNodePos].connected_nodes.size();k++){
                        if(tracingMap[curNodePos].connected_nodes[k]==newP)okay=false;
                    }
                    if(okay){
                        out->enqueue(newP);
                        tracingMap[curNodePos].connected_nodes.push_back(newP);
                        tracingMap[newP].connected_nodes.push_back(curNodePos);
                    }
                }
            }
        }
    }
    delete []ck;

}
void structureRefinement::tracingFromOneNode(int pos){
    if(curData->structureData[pos]!=2 && curData->structureData[pos]!=3)return;
    doTracing(int(pos/curData->ImageW) * curData->ImageW + pos%curData->ImageW,-1);
    doPredictByTracing();
}

void structureRefinement::handleUserCostBrushing(){
    qDebug()<<"user interaction5 start";

    QQueue <int> tracingQueue;
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            if(curData->user_brushing_buffer[tP]==5 || curData->user_brushing_buffer[tP]==6){
                curData->user_brushing_buffer[tP]=-1;
                //for weak node, reset path
                //delete nodes and add connected nodes to queue
                if(tracingMap[tP].type>0){
                    deleteNode(tP,&tracingQueue);
                }
                else if(tracingMap[tP].type==0){
                    deleteNode(tracingMap[tP].posy*curData->ImageW+tracingMap[tP].posx,&tracingQueue);
                }
                else if(curData->connection_object1[tP]!=-1){
                    int node1=tracingMap[curData->connection_object1[tP]].posy*curData->ImageW+tracingMap[curData->connection_object1[tP]].posx;
                    int node2=tracingMap[curData->connection_object2[tP]].posy*curData->ImageW+tracingMap[curData->connection_object2[tP]].posx;
                    deleteNode(node1,&tracingQueue);
                    deleteNode(node2,&tracingQueue);
                }
            }
        }
    }

//    qDebug()<<"node clear end";

    while(!tracingQueue.isEmpty()){
        int tP=tracingQueue.dequeue();
        if(tracingMap[tP].type>0){
            doTracing(tP,-1);
        }
    }
    curData->connectionStructureUpdate();
    emit connection_graph_updated();
//    qDebug()<<"tracing end";

    // do tracing with the nodes which is still type>0 in the queue
}
void structureRefinement::deleteNode(int pos, QQueue<int> *connected_nodes){
    qDebug()<<"node delete: "<<pos;

    if(tracingMap[pos].type<=0)return;

    for(int k=tracingMap[pos].connected_nodes.size()-1;k>=0;k--){
        int pos2=tracingMap[pos].connected_nodes[k];

        //reset weak path
        for(int i=0;i<tracingMap[pos].weak_nodes.size();i++){
            if(tracingMap[pos].weak_nodes[i]==pos2){
                tracingMap[pos].weak_nodes.remove(i);
                for(int j=1;j<tracingMap[pos].weak_paths[i].size()-1;j++){
                    curData->connection_object1[tracingMap[pos].weak_paths[i][j]]=-1;
                    curData->connection_object2[tracingMap[pos].weak_paths[i][j]]=-1;
                }
                tracingMap[pos].weak_paths.remove(i);

                for(int j=0;j<tracingMap[pos2].weak_nodes.size();j++){
                    if(tracingMap[pos2].weak_nodes[j]==pos){
                        tracingMap[pos2].weak_nodes.remove(j);
                        for(int w=1;w<tracingMap[pos2].weak_paths[j].size()-1;w++){
                            curData->connection_object1[tracingMap[pos2].weak_paths[j][w]]=-1;
                            curData->connection_object2[tracingMap[pos2].weak_paths[j][w]]=-1;
                        }
                        tracingMap[pos2].weak_paths.remove(j);
                        break;
                    }
                }

                break;
            }
        }

        for(int i=0;i<tracingMap[pos2].connected_nodes.size();i++){
            if(tracingMap[pos2].connected_nodes[i]==pos){
                tracingMap[pos2].connected_nodes.remove(i);
                break;
            }
        }
        tracingMap[pos].connected_nodes.remove(k);
        tracingMap[pos2].type=6;
        resetTracingMap(pos2);
        updateTracingMap(pos2);
        connected_nodes->push_back(pos2);
    }



    tracingMap[pos].predicted_label=-1;

    removeTrainingSetOfNode(pos);

    resetTracingMap(pos);
    if(tracingMap[pos].patch!=NULL)
        delete tracingMap[pos].patch;
    tracingMap[pos].distanceFromNode=1000000;
    tracingMap[pos].type=-1;


    qDebug()<<"end";
}

void structureRefinement::handleStructureBrushing(){
    qDebug()<<"user interaction1 start";

    bool structureChanged=false;
    bool trainingSetChanged=false;
    QQueue <int> buffer;
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            if(curData->user_brushing_buffer[tP]==0 || curData->user_brushing_buffer[tP]==1){
                if(curData->user_brushing_buffer[tP]==0 || curData->structureData[tP]>0)
                        curData->structureData[tP]=curData->user_brushing_buffer[tP];
                curData->user_brushing_buffer[tP]=-1;
                if(tracingMap[tP].type>0){
                    deleteNode(tP,&buffer);
                }
            }
            else if(curData->user_brushing_buffer[tP]==2 || curData->user_brushing_buffer[tP]==3){
                if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                    int nodeP=-1;
                    if(tracingMap[tP].type>0)nodeP=tP;
                    else if(tracingMap[tP].type==0)nodeP=tracingMap[tP].posy * curData->ImageW + tracingMap[tP].posx;

                    if(nodeP==-1){ // no node
                        if(curData->structureData[tP]!=curData->user_brushing_buffer[tP]){
                            structureChanged=true;
                        }
                        curData->structureData[tP]=curData->user_brushing_buffer[tP];
                    }
                    else{ //node existed
                        if(tracingMap[nodeP].predicted_label==-1){ // no predicted label
                            if(curData->structureData[tP]!=curData->user_brushing_buffer[tP]){
                                structureChanged=true;
                            }
                            curData->structureData[tP]=curData->user_brushing_buffer[tP];
                        }
                        else {//predicted label existed
                            int sc=structureChangeWithOneTrace(nodeP, curData->user_brushing_buffer[tP]);
                            if(sc==3){
                                structureChanged=true;
                                trainingSetChanged=true;
                            }
                            else if(sc==1){
                                structureChanged=true;
                            }
                            else if(sc==2){
                                trainingSetChanged=true;
                            }
                        }
                    }
                }
            }
            else if(curData->user_brushing_buffer[tP]==7){
                curData->user_brushing_buffer[tP]=-1;
                if(tracingMap[tP].type>0){
                    deleteNode(tP,&buffer);
                }
            }
        }
    }

    if(trainingSetChanged){
        isTrainingSetChanged=true;
//        reTraining();
    }
    if(structureChanged){
        isStructureChanged=true;
//        getCandidatePoints();
//        doPredictByTracing();
    }

    emit curData->structureChanged();



}
void structureRefinement::userInteractFinished(){
    emit startWaiting();
    if(isTrainingSetChanged){
        isTrainingSetChanged=false;
        reTraining();
        while(!deleteNodeList.isEmpty()){
            int curP=deleteNodeList.dequeue();
            if(tracingMap[curP].weak_nodes.size()==0){
                QQueue <int> buf;
                deleteNode(curP,&buf);
            }
        }
        doPredictByTracing();
    }
    if(isStructureChanged){
        isStructureChanged=false;
//        getCandidatePoints();
    }
    curData->connectionStructureUpdate();
    emit connection_graph_updated();

    emit endWaiting();

}
