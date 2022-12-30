#pragma once

#ifndef STRUCTUREREFINEMENT_H
#define STRUCTUREREFINEMENT_H


#include <QLinearGradient>
#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QWidget>
#include <QMatrix4x4>
#include <QVector2D>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QQueue>
#include <window.h>
#include <queue>




namespace mojo {
class network;
}

//class Window;
//class MitoDataset;

class structure{
public:
    unsigned short *data;
    int width;
    int height;
    int minx;
    int miny;

    int midx=0;
    int midy=0;
    int cnt=0;
    void save(QString path){

        int w=width;
        int h=height;
        while(w%4!=0){
            w++;
        }
        while(h%4!=0){
            h++;
        }

        unsigned char *stack=new unsigned char[w*h]();
        for(int iy=0;iy<height;iy++){
            for(int ix=0;ix<width;ix++){
                stack[iy*w+ix]=data[iy*width+ix];
            }
        }

        FILE *f=fopen((path+"_"+QString::number(w)+"_"+QString::number(h)+".raw").toStdString().c_str(),"wb");
        fwrite(stack,w*h,1,f);
        fclose(f);
        delete []stack;
    }
};




class structureRefinement :public QObject
{
    Q_OBJECT

public:
    structureRefinement(MitoDataset *p, QWidget *parent = 0);
    ~structureRefinement();


    QQueue <patchSet*> candidatePoints;


    bool onSegment(QVector2D p, QVector2D q, QVector2D r);
    int orientation(QVector2D p, QVector2D q, QVector2D r);
    bool doIntersect(QVector2D p1, QVector2D q1, QVector2D p2, QVector2D q2);
    bool isInside(QVector2D p,QVector2D *q);


    patchSet *mousePatch=NULL;

public slots:
    void getStartingPoints_minimap();
    void getStartingPoints_minimap_sub(int p);

    void getCandidatePoints();
    void initCnnModel();
    float getModelAccuracy(int n,std::vector<float*> test_data,std::vector<int> test_label);
    void run();
    void preTraining();
    QVector2D getCandidateMiddlePoint(int sx,int sy,bool *ck);
    int getCandidateMiddlePoint_tracing(int tx, int ty, int sx, int sy, bool *ck);
    int getCandidateMiddlePoint_minimap(int sx, int sy, bool *ck);
    int getStartMiddlePoint_minimap(int sx, int sy, bool *ck);

    QVector2D getMiddlePoint(int sx,int sy,int l);

    structure getStructure(int sx,int sy,int l,int maxPNum,unsigned char*structureData,bool eraseFlag);//l: -1 for all label
    std::vector<float*> getPatches(structure S);
    patchSet* getPatch(int sx,int sy,int l);//l: -1 for all label
    int getClass(float *data);
    float* getProbability(float *data);

    void modelTraining(int n,std::vector<float*> data,std::vector<int> label);
    void savePatches(QString path);
    void init();

    void patchTraining(patchSet patch);

    void trainingDataGeneration(patchSet *patch, int l, int nodePos);
    void reTraining();

    void saveProbabilityMap(QString path);

    void labelCleaning();


    void initialTracingFromCandidatePoints();
    void completeTracing();

    void doTracing(int startPos, int prevPos);

    void doPredictByTracing();


    void updateTracingMap(int curNodePos);
    void resetTracingMap(int curNodePos);

    void findNextNodes(int curNodePos,QQueue <int> *out);

    int findNextNodeForWeakSignal(int curNodePos,int prevNodePos);
    float node_cost(int pos,float direction_cost);


    void doPredict();

    float getShortestPathBetweenToPoint(int start_pos,int end_pos);

    int getMiddlePoint_on_fragmented_object(int pos);

    void tracingFromOneNode(int pos);

    int findNearestOutside(int pos);

    void handleUserCostBrushing();

    void handleStructureBrushing();

    void deleteNode(int pos,QQueue<int> *connected_nodes);

    void changeStructure(int pos, int label);
    void changeStructureOnCriticalPoint(int pos, int label);
    void changePredictData(int pos, int label);

    void doPredictWithOneTrace(int pos);
    int structureChangeWithOneTrace(int pos,int label);

    void removeTrainingSetOfNode(int curP);
    void userInteractFinished();
    void divideNeurite();

    bool checkNoise(int posx,int posy);
    void refineTracing();

signals:
    void structureChanged();
    void correctionChanged();
    void connection_graph_updated();

    void startWaiting();
    void endWaiting();

    void tracingFinished(int ind);

protected:
//    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

public:
    mojo::network *cnn;

    MitoDataset *curData;

    tracingNode* tracingMap;
    // if type is -1, needed to generate node,
    // if type is 0, posx and posy indicate near node.
    // if type is >=1, on the tracing line


    bool is_cnn_init=false;
    bool is_pre_trained=false;
    bool stopSignal=false;
    bool exitSignal=false;

    bool is_new_input=false;
    std::vector<float*> dendPatches;
    std::vector<float*> axonPatches;
    std::vector<float*> trainingData;
    std::vector<int> trainingLabel;

    QVector<float*> fixedTrainingData_axon;
    QVector<int> fixedTrainingLabel_axon;
    QVector<float*> fixedTrainingData_dend;
    QVector<int> fixedTrainingLabel_dend;

    QVector<int> index_trainingData_to_node_dend;
    QVector<int> index_trainingData_to_node_axon;

    int structureCnt=0;
    int cleanCnt=0;

    QQueue<int>tracingQ;

    bool isTrainingSetChanged=false;
    bool isStructureChanged=false;

    QQueue <int> deleteNodeList;


};

#endif // STRUCTUREREFINEMENT_H
