
#include "structureConnector.h"
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

#include "rf/decision-trees.hxx"
#include "rf/marray.hxx"


structureConnector::structureConnector(MitoDataset*p, QWidget *parent){
    curData=p;
    curData->connection_graph=new unsigned char[curData->ImageW*curData->ImageH]();
//    curData->connection_object1=new unsigned short[curData->ImageW*curData->ImageH]();
//    curData->connection_object2=new unsigned short[curData->ImageW*curData->ImageH]();
    D=new nodeInfo[curData->ImageW*curData->ImageH];
    curData->objectIndex=new unsigned short[curData->ImageW*curData->ImageH];
}

void structureConnector::run(){
    while(1){
        if(finish_signal==true)break;
//        while(!curData->connection_work_list.empty()){
//            int i=curData->connection_work_list.dequeue();
//            cur_work_index=i;
//            qDebug()<<i;
//            find_paths(i,curData->connection_distance_threshold,curData->connection_background_threshold);
//            qDebug()<<"finish find path";
//            update_path_map();
//            qDebug()<<"path updated";
//        }
        if(cur_work_update_signal==true){
            cur_work_index=max_object_index;
            ind=1;
            cur_work_update_signal=false;
        }

        bool updated_flag=false;
        while(ind!=cur_work_index){
            if(finish_signal==true)break;
            if(cur_work_update_signal==true){
                cur_work_index=ind;
                cur_work_update_signal=false;
            }
            if(curData->doClassification){
                classification();
                curData->doClassification=false;
            }
            ind=ind%max_object_index;
            qDebug()<<ind;
            find_paths(ind,curData->connection_distance_threshold,curData->connection_background_threshold);
            qDebug()<<"finish find path";
            update_path_map();
            qDebug()<<"path updated";
            ind=(ind+1);
            if(ind==0)ind=1;
            updated_flag=true;

        }
        if(updated_flag || curData->doClassification){
            classification();
            curData->doClassification=false;
        }
//        for(int i=1;i<max_object_index;i++){
//            if(finish_signal==true)break;
//            qDebug()<<i;
//            find_paths(i,curData->connection_distance_threshold,curData->connection_background_threshold);
//            qDebug()<<"finish find path";
//            update_path_map();
//            qDebug()<<"path updated";
//        }
    }

}
void structureConnector::updateWorkList(){
    for(int i=1;i<cur_work_index;i++){
        curData->connection_work_list.enqueue(i);
    }
}
void structureConnector::classification(){

    qDebug() << "Random forest in";
    qDebug() << "Random forest start";


    int foregroundSize=0;
    int backgroundSize=0;
    int wholeSize=0;
    for(int i=1;i<max_object_index;i++){
        for(int j=i+1;j<max_object_index;j++){
            int path_len=paths[i][j].length();
            if(path_len!=0){
                if(curData->type_of_paths[i*max_object_index+j]==3
                        ||curData->type_of_paths[i*max_object_index+j]==5
                        || curData->type_of_paths[i*max_object_index+j]==6 ){
                    backgroundSize++;
                }
                if(curData->type_of_paths[i*max_object_index+j]==4
                        ||curData->type_of_paths[i*max_object_index+j]==7
                        || curData->type_of_paths[i*max_object_index+j]==8 ){
                    foregroundSize++;
                }
                wholeSize++;
            }
        }
    }
    if(foregroundSize+backgroundSize==0)return;
    const size_t numberOfSamples = foregroundSize+backgroundSize;
    const size_t numberOfFeatures = 3;
    const size_t shape[] = { numberOfSamples, numberOfFeatures };
    andres::Marray<float> features(shape, shape + 2);
    andres::Marray<unsigned char> labels(shape, shape + 1);

    int cnt = 0;
    for(int i=1;i<max_object_index;i++){
        for(int j=i+1;j<max_object_index;j++){
            int path_len=paths[i][j].length();
            if(path_len!=0){
                if(curData->type_of_paths[i*max_object_index+j]==3
                        ||curData->type_of_paths[i*max_object_index+j]==5
                        || curData->type_of_paths[i*max_object_index+j]==6 ){
                    features(cnt,0)=curData->distance1_of_paths[i*max_object_index+j];
                    features(cnt,1)=curData->distance2_of_paths[i*max_object_index+j];
                    features(cnt,2)=curData->mean_intensity_of_paths[i*max_object_index+j];
                    labels(cnt) = 0;
                    cnt++;
                }
                if(curData->type_of_paths[i*max_object_index+j]==4
                        ||curData->type_of_paths[i*max_object_index+j]==7
                        || curData->type_of_paths[i*max_object_index+j]==8 ){
                    features(cnt,0)=curData->distance1_of_paths[i*max_object_index+j];
                    features(cnt,1)=curData->distance2_of_paths[i*max_object_index+j];
                    features(cnt,2)=curData->mean_intensity_of_paths[i*max_object_index+j];
                    labels(cnt) = 1;
                    cnt++;
                }
            }
        }
    }

    andres::ml::DecisionForest<float,unsigned char,float> decisionForest;
    const size_t numberOfDecisionTrees = 50;
    decisionForest.learn(features, labels, numberOfDecisionTrees);

    qDebug() << "Random forest trained";


    const size_t shape2[] = { wholeSize, numberOfFeatures };
    andres::Marray<float> features2(shape2, shape2 + 2);

    cnt=0;
    for(int i=1;i<max_object_index;i++){
        for(int j=i+1;j<max_object_index;j++){
            int path_len=paths[i][j].length();
            if(path_len!=0){
                features2(cnt,0)=curData->distance1_of_paths[i*max_object_index+j];
                features2(cnt,1)=curData->distance2_of_paths[i*max_object_index+j];
                features2(cnt,2)=curData->mean_intensity_of_paths[i*max_object_index+j];
                cnt++;
            }
        }
    }


    andres::Marray<float> probabilities(shape2, shape2 + 2);
    decisionForest.predict(features2, probabilities);

    cnt=0;
    for(int i=1;i<max_object_index;i++){
        for(int j=i+1;j<max_object_index;j++){
            int path_len=paths[i][j].length();
            if(path_len!=0){
                int type=curData->type_of_paths[i*max_object_index+j];
                if(probabilities(cnt)<0.5){
                    if(type==0 || type==1 || type==2)
                        curData->type_of_paths[i*max_object_index+j]=2;
                    if(type==3 || type==5 || type==6)
                        curData->type_of_paths[i*max_object_index+j]=6;
                    if(type==4 || type==7 || type==8)
                        curData->type_of_paths[i*max_object_index+j]=8;
                }
                else{
                    int type=curData->type_of_paths[i*max_object_index+j];
                    if(type==0 || type==1 || type==2)
                        curData->type_of_paths[i*max_object_index+j]=1;
                    if(type==3 || type==5 || type==6)
                        curData->type_of_paths[i*max_object_index+j]=5;
                    if(type==4 || type==7 || type==8)
                        curData->type_of_paths[i*max_object_index+j]=7;
                }
                cnt++;
            }
        }
    }


    qDebug() << "Random forest end";

    emit connection_graph_updated();



}
structureConnector::~structureConnector(){

}
void structureConnector::update_path_map(){
    int imSize=curData->ImageW*curData->ImageH;
    for(int i=0;i<imSize;i++){
        curData->connection_graph[i]=0;
    }
    for(int i=1;i<max_object_index;i++){
        for(int j=i+1;j<max_object_index;j++){
            int path_len=paths[i][j].length();
            if(path_len!=0){
                int type1=curData->structureData[paths[i][j][0]];
                int type2=curData->structureData[paths[i][j][path_len-1]];
                int path_type=3;
                if(type1==2 && type2==2)path_type=2;
                for(int k=1;k<path_len-1;k++){
                    curData->connection_graph[paths[i][j][k]]=path_type;
//                    curData->connection_object1[paths[i][j][k]]=i;
//                    curData->connection_object2[paths[i][j][k]]=j;

                }
            }
        }
    }
    emit connection_graph_updated();

}
void structureConnector::find_paths(int start_object, float distance_thresh,float background_thresh){
    int imSize=curData->ImageW*curData->ImageH;
    for(int i=0;i<imSize;i++){
        D[i].distance=1000000;
        D[i].fromPos=-1;
    }

    for(int i=1;i<max_object_index;i++){
        paths[start_object][i].clear();
        paths[i][start_object].clear();
        distances[start_object*max_object_index + i]=1000000;
        distances[i*max_object_index + start_object]=1000000;
    }

    qDebug()<<"init";

    std::priority_queue <nodeInfo,std::vector<nodeInfo>,CompareFunctionObject> workingQ;


    for(int i=0;i<imSize;i++){
        if(curData->objectIndex[i]==start_object){
            D[i].distance=0;
            workingQ.push(nodeInfo(i,0));
        }
    }

    qDebug()<<"start setting";

    int dxylist[4][2]={{-1,0},{1,0},{0,-1},{0,1}};
    while(!workingQ.empty()){
        nodeInfo cur=workingQ.top();
//        qDebug()<<"top:"<<cur.fromPos%curData->ImageW<<cur.fromPos/curData->ImageW<<cur.distance;

        int curx=cur.fromPos%curData->ImageW;
        int cury=cur.fromPos/curData->ImageW;
        workingQ.pop();
        for(int d=0;d<4;d++){
            int newx=curx+dxylist[d][0];
            int newy=cury+dxylist[d][1];
            if(newx<0 || newx>=curData->ImageW ||newy<0 || newy>=curData->ImageH){
                continue;
            }
            int newP=newy*curData->ImageW + newx;
            int newS= curData->objectIndex[newP];
            if(newS!=0){
                if(newS!=start_object){
                    if(distances[start_object*max_object_index + newS]>cur.distance && cur.isDead==false){
                        //update path
//                        qDebug()<<"update path:";
                        distances[start_object*max_object_index + newS]=cur.distance;
                        distances[newS*max_object_index + start_object]=cur.distance;
                        paths[start_object][newS].clear();
                        paths[newS][start_object].clear();
                        paths[start_object][newS].push_back(newP);
                        paths[newS][start_object].push_back(newP);
                        int path_iter=cur.fromPos;
                        while(1){
                            paths[start_object][newS].push_back(path_iter);
                            paths[newS][start_object].push_back(path_iter);
                            if(curData->objectIndex[path_iter]==start_object)break;
                            path_iter=D[path_iter].fromPos;
                        }

                        curData->distance1_of_paths[newS*max_object_index+start_object]=paths[start_object][newS].length();
                        curData->distance1_of_paths[start_object*max_object_index+newS]=paths[start_object][newS].length();

                        curData->distance2_of_paths[newS*max_object_index+start_object]=cur.distance;
                        curData->distance2_of_paths[start_object*max_object_index+newS]=cur.distance;

                        curData->mean_intensity_of_paths[newS*max_object_index+start_object]=cur.distance/paths[start_object][newS].length();
                        curData->mean_intensity_of_paths[start_object*max_object_index+newS]=cur.distance/paths[start_object][newS].length();

                        curData->type_of_paths[newS*max_object_index+start_object]=0;
                        curData->type_of_paths[start_object*max_object_index+newS]=0;

                    }

                    if(D[newP].distance>D[cur.fromPos].distance){
                        D[newP].distance=D[cur.fromPos].distance;
                        D[newP].fromPos=cur.fromPos;
                        workingQ.push(nodeInfo(newP,D[cur.fromPos].distance,true));
                    }

                }
                else{
                    continue;
                }
            }
            float node_v=float(curData->imageData1[newP])/65535;
            if(node_v<background_thresh){
                continue;
            }
            float new_distance=D[cur.fromPos].distance+node_cost(node_v);
            if(new_distance>distance_thresh){
                continue;
            }
            if(D[newP].distance>new_distance){
                D[newP].distance=new_distance;
                D[newP].fromPos=cur.fromPos;
                workingQ.push(nodeInfo(newP,new_distance,cur.isDead));
//                qDebug()<<"push:"<<newP%curData->ImageW<<newP/curData->ImageW<<new_distance;

            }
        }
    }
}
float structureConnector::node_cost(float v){
    //return 1.0;
    return 1.0/(1+exp(10*v-5));
}
void structureConnector::object_indexing(){
    int imSize=curData->ImageW*curData->ImageH;
    for(int i=0;i<imSize;i++){
        curData->objectIndex[i]=0;
    }
    for(int i=0;i<imSize;i++){
        if(curData->objectIndex[i]==0 && curData->structureData[i]!=0){
            max_object_index++;
            curData->objectIndex[i]=max_object_index;
            connected_component(i%curData->ImageW,i/curData->ImageW,max_object_index);
        }
    }
    max_object_index++;
    distances=new float[max_object_index*max_object_index];
    for(int i=0;i<max_object_index*max_object_index;i++){
        int a=i%max_object_index;
        int b=i/max_object_index;
        if(a==0 || b==0)continue;
        if(a==b)distances[i]=0;
        else distances[i]=1000000;
    }
    paths=new QVector<int>*[max_object_index];
    for(int i=0;i<max_object_index;i++){
        paths[i]=new QVector<int>[max_object_index];
    }

    curData->type_of_paths=new unsigned char[max_object_index*max_object_index];
    curData->distance1_of_paths=new float[max_object_index*max_object_index];
    curData->distance2_of_paths=new float[max_object_index*max_object_index];
    curData->mean_intensity_of_paths=new float[max_object_index*max_object_index];

    cur_work_index=max_object_index;
    curData->max_object_index=max_object_index;
//    updateWorkList();
//    char tt[100];
//    sprintf(tt,"check_%d_%d.raw",curData->ImageW,curData->ImageH);
//    FILE *f=fopen(tt,"wb");
//    fwrite(objectIndex,2,curData->ImageW*curData->ImageW,f);
//    fclose(f);

}


void structureConnector::init_graph(){
//    int imSize=curData->ImageW*curData->ImageH;
//    for(int i=0;i<imSize;i++){
//        D[i].distance=1000000;
//        D[i].fromPos=-1;
//    }

//    QQueue <int> q;
//    for(int i=0;i<imSize;i++){
//        if(objectIndex[i]!=0){
//            q.enqueue(i);
//            D[i].distance=0;
//            break;
//        }
//    }
//    int dxylist[4][2]={{-1,0},{1,0},{0,-1},{0,1}};

//    while(!q.empty()){
//        int cur=q.dequeue();
//        int curx=cur%curData->ImageW;
//        int cury=cur/curData->ImageW;
//        for(int d=0;d<4;d++){
//            if(curx+dxylist[d][0]<0 || curx+dxylist[d][0]>=curData->ImageW || cury+dxylist[d][1]<0 || cury+dxylist[d][1]>=curData->ImageH){
//                continue;
//            }
//            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];

//            float newDistance=D[cur].distance+(1.0-float(curData->imageData1[tP])/65535);
//            if(D[tP].distance<=newDistance){
//                continue;
//            }

//            if(objectIndex[tP]!=0 && D[cur].fromObjectIndex==objectIndex[tP]){
//                D[tP].distance=0;
//                D[tP].fromObjectIndex=objectIndex[tP];
//                q.enqueue(tP);
//                continue;
//            }
//            if(objectIndex[tP]!=0 && D[cur].fromObjectIndex!=objectIndex[tP]){
//                D[tP].distance=0;
//                D[tP].fromObjectIndex=objectIndex[tP];
//                q.enqueue(tP);
//                continue;
//            }


//            if(curData->imageData1[tP]==0){
//                continue;
//            }


//            if(objectIndex[tP]!= && curData->structureData[tP]!=0){
//                q.enqueue(tP);

//            }
//        }

//    }


}
void structureConnector::connected_component(int startx, int starty, int ind){
    int dxylist[4][2]={{-1,0},{1,0},{0,-1},{0,1}};
    int cnt=1;
    QQueue<int> quex,quey;
    quex.enqueue(startx);
    quey.enqueue(starty);

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();

        for(int d=0;d<4;d++){
            if(curx+dxylist[d][0]<0 || curx+dxylist[d][0]>=curData->ImageW || cury+dxylist[d][1]<0 || cury+dxylist[d][1]>=curData->ImageH){
                continue;
            }
            int tP=(cury+dxylist[d][1])*curData->ImageW + curx+dxylist[d][0];
            if(curData->objectIndex[tP]==0 && curData->structureData[tP]!=0){
                curData->objectIndex[tP]=ind;
                quex.enqueue(curx+dxylist[d][0]);
                quey.enqueue(cury+dxylist[d][1]);
            }
        }
    }

}
