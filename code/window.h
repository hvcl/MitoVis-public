///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////
#pragma once

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QMatrix4x4>
#include <QVector2D>
#include <QSplitter>
#include "glwidget_for_data.h"
#include "glwidget_for_object.h"

//#include "glwidget.h"
#include "parallelCoordinate.h"
#include "tsneSelection.h"
#include "colorBar.h"
#include "tinytiffwriter.h"
#include "imageControlGraph.h"
#include "variationGraph.h"
#include "glwidget_for_tracingMap.h"


#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QDesktopWidget>
#include <QApplication>
#include <QMessageBox>
#include <QTabWidget>
#include <QSpinBox>
#include <QDockWidget>
#include <QTextEdit>
#include <QTime>
#include <QCheckBox>
#include <QLabel>
#include <QSpacerItem>
#include <QComboBox>
#include <QDebug>
#include <QTimer>
#include <QGroupBox>
#include <QGraphicsDropShadowEffect>
#include "DarkStyle.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QFileDialog>
#include <QSplitterHandle>
#include <QColorDialog>
#include <QProgressBar>
#include <QThread>
#include <QQueue>
#include "Eigen/Core"
#include "Eigen/Eigenvalues"
#include <QMovie>

#include <QMatrix>
#include <QProcess>
#include <QDialog>

class MainWindow;
class structureRefinement;
class structureConnector;
class patchRecommender;
class FSRadar;


#define MITOTHRESH 65535/2
#define CELL_BODY 1

#define patchSaveSize 24
#define patchSize 21
#define halfpatchSize 10


#define MaxSearchSpace 100

class nodeInfo{
public:
    nodeInfo(){

    }
    nodeInfo(int p,float d){
        fromPos=p;
        distance=d;
    }
    nodeInfo(int p,float d,bool id){
        fromPos=p;
        distance=d;
        isDead=id;
    }

    int fromPos;
    float distance;
    bool isDead=false;

};
class CompareFunctionObject
{
public:
    int operator()(nodeInfo t, nodeInfo u){
        return t.distance > u.distance;
    }
};

class patchSet{
public:
    patchSet(){
        boundMin=QVector2D(1000000,1000000);
        boundMax=QVector2D(-1,-1);
        for(int i=0;i<patchSize*patchSize;i++)patch[i]=0;
    }
    ~patchSet(){

    }
    float patch[patchSize*patchSize];
    int midx;
    int midy;
    float angle;
    bool is_valid=false;
    QVector2D patchCorner[4];
    QVector2D boundMin;
    QVector2D boundMax;
    
    float n_included=0;

    int label_type=0; //2: dendrite 3: axon 4: mixed

    int index=-1;
};

class tracingNode_neurite_based{
public:
    patchSet *patch=NULL;
    int neurite_index=-1;
};

class tracingNode{
  public:
    int posx=0;
    int posy=0;
    int type=-1;
    float distanceFromNode=1000000;
    // -1: none, 0: included near node, 1: normal, 2: crossing, 3: branch, 4: fragmented end, 5: real end, 6: waiting
    QVector <int> connected_nodes;
    patchSet *patch=NULL;
    bool debug_middle_node=false;

    QVector<int> weak_nodes;
    QVector <QVector<int>> weak_paths;

    int predicted_label=-1;

    int trainingSetIndex=-1; // from this index, four elements
    int trainingSetType=-1; // if 2, in the fixedTrainingData_dend and fixedTrainingLabel_dend
                            // if 3, in the fixedTrainingData_axon and fixedTrainingLabel_axon

    bool temp_ck=false;
    int corrected_type=0; //0: no correction, 2: user dendrite, 3: user axon

    int neurite_ind=-1;

};





struct forSort3 {
  QVector2D value;
  float yvalue;
  float deep;
  int ind1=-1;
  int ind2=-1;
  bool operator<(const forSort3 &other) const {
    return (yvalue < other.yvalue);
  }
};



struct forSort_base { //increasing
    float value;
    int index;
      bool operator<(const forSort_base &other) const {
        return (value < other.value);
      }
};



class Mito{
public:
    int index=-1;
    int xmin=0;
    int xmax=0;
    int ymin=0;
    int ymax=0;
    int structure=0;
    bool enabled=true;
};

class MitoDataset: public QObject
{
    Q_OBJECT

signals:
    void correctorInit();
    void correctorRun();
    void correctorReTrain();
    void correctorGetCandidatePoints();

    void correctorTracingPointAdded(int pos);

    void sendStructureChanged();
    void sendCorrectionChanged();
    void sendConnectionChanged();

    void connectorRun();
    void connectorObjectIndexing();

    void correctorUserCostBrushing();
    void correctorStructureBrushing();

    void userInteractFinished();



public slots:
    void structureChanged();
    void correctionChanged();
    void connectionChanged();
public:

    QTime time;
    int prev_elapsed_time=0;

    int initial_deploy_time=0;

    tracingNode* tracingMap;

    QVector<int>* tracingMap_simplified;
    QVector<int>* tracingMap_simplified_collections;
    unsigned char *structureData_simplified;

    // if type is -1, needed to generate node,
    // if type is 0, posx and posy indicate near node.
    // if type is >=1, on the tracing line


    float *user_define_cost;

    int *user_brushing_buffer;

    QThread workerThread_connector;
    structureConnector *connector;

    unsigned char *connection_graph; //0: none, 2: dendrite, 3: axon

    QQueue <int> connection_work_list;

    int *connection_object1;
    int *connection_object2;

    unsigned int *connection_path_forward;
    unsigned int *connection_path_backward;


    unsigned short *objectIndex;

    int max_object_index=-1;

    unsigned char *type_of_paths;
    //0: none, 1: negative, 2: positive,
    //3: selected_as_background, 4: selected_as_foreground,
    //5: selected_as_background-negative, 6: selected_as_background-positive
    //7: selected_as_foregorund-negative, 8: selected_as_foreground-positive
    float *distance1_of_paths; //euclidian distance
    float *distance2_of_paths; //signal based distance
    float *mean_intensity_of_paths;

    bool doClassification=false;

    bool doTracing=false;

    QVector <int> selected_neurite_on_minimap;

    float connection_background_threshold=0.05;
    float connection_distance_threshold=20;
    float connection_visibility=1.0;
    float connection_fill_threshold=0.5;
    int connection_fill_distance_threshold=3;


    QThread workerThread;
    structureRefinement *corrector;

    QString path;
    QString resultPath;
    int group;
    int ImageType;
    int ImageW;
    int ImageH;
    int ImageD;
    unsigned short *imageData1; //neuron channel
    unsigned short *imageData2; //mitochondria channel
    unsigned char *structureData;

    unsigned char *connectionStructureData;

    unsigned char *cc_buffer=NULL;


    void connectionStructureUpdate();

    unsigned char *predictData;

    unsigned char *correctionData; //1: 2 -> 3, 2: 3 -> 2
    int correctorInputType=-1; //-1: none, 0:background,1:cellbody, 2: dendrite, 3: axon,4: tracingP
    bool correctorEnable_dend=false;
    bool correctorEnable_axon=false;

    float correction_opacity=0.5;
    float correction_threshold=0.5;

    unsigned char *scribbleData;

    unsigned char *mito_buffer=NULL;

    unsigned char *mitoCorrectionData; //1: 2 -> 3, 2: 3 -> 2


    unsigned char *subsetData;
    int brushSize=5;
    QString subsetName="";
    int subsetInputType=0;



    QVector <float>structureArea;
    unsigned char *mitoLabelImage_original;
    unsigned char *mitoLabelImage_label;
    unsigned short *mitoLabelImage_probability;
    unsigned short *mitoLabelImage;
    unsigned char *mitoLineImage=NULL;
    bool *check;
    int enabledNum=0;

    QVector <Mito> originalBoundary;


    QVector <Mito> mitoData_signal;
    unsigned short *mitoLabelImage_signal;


    QVector <Mito> mitoData;
    QVector <QVector<float>> features;
    QVector <QColor> mitoColors;
    QVector <bool> enabled;
    QVector <bool> globalEnabled;

    QVector<QVector2D> selectionRange; //selection for each feature
    QVector<bool> selection; //if there is selection on the exis, selection[featureN]=true;
    QVector<QVector2D> axisRange;

    QVector2D plotXaxisRange=QVector2D(0,1);
    QVector2D plotYaxisRange=QVector2D(0,1);

    QVector<QVector2D> plotCoord;
    int axis1=0;
    int axis2=0;
    //0: PCA, 1: UMAP, 2:

    QString dir="";
    int focusItem=-1;

    int maxStructureLabel=0;

    int dataIndex=0;

    float mito_thresh=0.5;
    float structure_thresh=0.6;

    GLuint tex_neuron;
    GLuint tex_structure;
    GLuint tex_mito;
    GLuint tex_mitoLabel;
    GLuint tex_mitoLabel_original;

    GLuint tex_correction;
    GLuint tex_connection;

    GLuint tex_subset;

    float structure_opacity=1;
//    float structure_opacity=0;
    float mitoLabel_opacity=1;

    QVector4D background_color=QVector4D(0,0,0,0);
    QVector2D shift=QVector2D(0,0);
    float scale=1;

    bool IsGLInit=false;

    float color_table_neuron[65536];
    float color_table_mito[65536];

    GLuint tex_color_table_neuron;
    GLuint tex_color_table_mito;

    float bright_neuron=0.5;
//    float bright_neuron=0;
    float contrast_neuron=0.2;
    float midpos_neuron=0.5;

    float bright_mito=0.5;
//    float bright_mito=600/255.0;
    float contrast_mito=0.2;
    float midpos_mito=0.5;


    float error_thresh=0.5;


    // error correction
    QVector <GlWidgetForObject*> error_candidates;


    tracingNode_neurite_based *tracingMap_neurite_based;


    QMap <int,QVector<float>> neurite_feature;
    QMap <int,int> neurite_res;
    QMap <int,float> neurite_probability_dend; //dend_prob + axon_prob =1
    QMap <int,bool> neurite_changed;


    QMap <int,int> patch_neurite_map;

    QMap <int,int> patch_res;
    QMap <int,float> patch_probability_dend; //dend_prob + axon_prob =1



    int *global_neurite_ind;
    int *global_patch_ind;

    QVector <FSRadar*> radars;
    QMap <int,int> radar_indexs;
    float *similarity_matrix=NULL;
    int similarity_matrix_width=0;
    QVector <int> similarity_matrix_neu_ind;
    QMap <int,int> neurite_index_map;

    QMap <int,int> neurite_sample_point;


    int focus_fixed_point=-1;

    bool tracingMap_loaded=false;

    int finetune_iter=0;

    float resolution=0.2058;

    MitoDataset(QString baseDir, int ind, int groupInd, bool tracing_map_load);
    void addMitoByConnectedComponent(Mito boundary);
    void ConnectedComponent(Mito L, int startx, int starty, int ind);
    bool getFeatures(Mito m);
    float getLength(Mito m);
    void checkDataEnable(Window *c_window);
    void setColorTable();
    void featureSave();
    void saveStructureLabel(QString path);
    void saveStructureInput(QString path);
    void saveMitoLabel(QString path,unsigned char *data);
    void saveMitoInput(QString path);
    void featureSave_with_path(QString _path);
    void changeMito(){
        delete []mitoLabelImage;
        mitoLabelImage=new unsigned short[ImageW*ImageH]();

        delete []check;
        check=new bool[ImageW*ImageH]();
        for(int i=0;i<ImageW*ImageH;i++){
            if(structureData[i]==CELL_BODY){
                check[i]=true;
            }
        }
        mitoData.clear();
        mitoColors.clear();
        enabled.clear();
        for(int i=0;i<features.length();i++)features[i].clear();
        features.clear();

        Mito newL;
        newL.index=0;
        newL.xmin=1;
        newL.ymin=1;
        newL.xmax=ImageW-2;
        newL.ymax=ImageH-2;
        addMitoByConnectedComponent(newL);


//        for(int i=0;i<originalBoundary.length();i++){
//            addMitoByConnectedComponent(originalBoundary[i]);
//        }
    }
    void save_tracingMap(QString path){
        FILE *f=fopen(path.toStdString().c_str(),"wb");
        for(int i=0;i<ImageW*ImageH;i++){
            fwrite(&tracingMap[i].posx,sizeof(int),1,f);
            fwrite(&tracingMap[i].posy,sizeof(int),1,f);
            fwrite(&tracingMap[i].type,sizeof(int),1,f);
            fwrite(&tracingMap[i].distanceFromNode,sizeof(float),1,f);
            int size=tracingMap[i].connected_nodes.size();
            fwrite(&size,sizeof(int),1,f);
            for(int j=0;j<tracingMap[i].connected_nodes.size();j++){
                fwrite(&tracingMap[i].connected_nodes[j],sizeof(int),1,f);
            }
            bool t=tracingMap[i].patch!=NULL;
            fwrite(&t,sizeof(bool),1,f);

            if(t){
                fwrite(tracingMap[i].patch->patch,sizeof(float),patchSize*patchSize,f);
                fwrite(&tracingMap[i].patch->midx,sizeof(int),1,f);
                fwrite(&tracingMap[i].patch->midy,sizeof(int),1,f);
                fwrite(&tracingMap[i].patch->angle,sizeof(float),1,f);
                fwrite(&tracingMap[i].patch->is_valid,sizeof(bool),1,f);
                for(int j=0;j<4;j++){
                    float v1=tracingMap[i].patch->patchCorner[j].x();
                    float v2=tracingMap[i].patch->patchCorner[j].y();
                    fwrite(&v1,sizeof(float),1,f);
                    fwrite(&v2,sizeof(float),1,f);
                }
                {
                    float v1=tracingMap[i].patch->boundMin.x();
                    float v2=tracingMap[i].patch->boundMin.y();
                    fwrite(&v1,sizeof(float),1,f);
                    fwrite(&v2,sizeof(float),1,f);
                }
                {
                    float v1=tracingMap[i].patch->boundMax.x();
                    float v2=tracingMap[i].patch->boundMax.y();
                    fwrite(&v1,sizeof(float),1,f);
                    fwrite(&v2,sizeof(float),1,f);
                }
                fwrite(&tracingMap[i].patch->n_included,sizeof(float),1,f);
                fwrite(&tracingMap[i].patch->label_type,sizeof(int),1,f);
                fwrite(&tracingMap[i].patch->index,sizeof(int),1,f);

            }
            fwrite(&tracingMap[i].debug_middle_node,sizeof(tracingMap[i].debug_middle_node),1,f);
            fwrite(&tracingMap[i].predicted_label,sizeof(tracingMap[i].predicted_label),1,f);
            fwrite(&tracingMap[i].trainingSetIndex,sizeof(tracingMap[i].trainingSetIndex),1,f);
            fwrite(&tracingMap[i].trainingSetType,sizeof(tracingMap[i].trainingSetType),1,f);
            fwrite(&tracingMap[i].temp_ck,sizeof(tracingMap[i].temp_ck),1,f);
            fwrite(&tracingMap[i].corrected_type,sizeof(tracingMap[i].corrected_type),1,f);
            fwrite(&tracingMap[i].neurite_ind,sizeof(tracingMap[i].neurite_ind),1,f);
        }
        fclose(f);
    }
    QVector2D load_tracingMap(QString path){
        QVector2D max_ind=QVector2D(-1,-1);
        FILE *f=fopen(path.toStdString().c_str(),"rb");
        for(int i=0;i<ImageW*ImageH;i++){
            fread(&tracingMap[i].posx,sizeof(int),1,f);
            fread(&tracingMap[i].posy,sizeof(int),1,f);
            fread(&tracingMap[i].type,sizeof(int),1,f);
            fread(&tracingMap[i].distanceFromNode,sizeof(float),1,f);
            int size;
            fread(&size,sizeof(int),1,f);
            for(int j=0;j<size;j++){
                int v;
                fread(&v,sizeof(int),1,f);
                tracingMap[i].connected_nodes.push_back(v);
            }
            bool t;
            fread(&t,sizeof(bool),1,f);

            if(t){
                if(tracingMap[i].patch==NULL){
                    tracingMap[i].patch=new patchSet;
                }
                fread(tracingMap[i].patch->patch,sizeof(float),patchSize*patchSize,f);
                fread(&tracingMap[i].patch->midx,sizeof(int),1,f);
                fread(&tracingMap[i].patch->midy,sizeof(int),1,f);
                fread(&tracingMap[i].patch->angle,sizeof(float),1,f);
                fread(&tracingMap[i].patch->is_valid,sizeof(bool),1,f);
                for(int j=0;j<4;j++){
                    float v1,v2;
                    fread(&v1,sizeof(float),1,f);
                    fread(&v2,sizeof(float),1,f);
                    tracingMap[i].patch->patchCorner[j]=QVector2D(v1,v2);
                }
                {
                    float v1,v2;
                    fread(&v1,sizeof(float),1,f);
                    fread(&v2,sizeof(float),1,f);
                    tracingMap[i].patch->boundMin=QVector2D(v1,v2);
                }
                {
                    float v1,v2;
                    fread(&v1,sizeof(float),1,f);
                    fread(&v2,sizeof(float),1,f);
                    tracingMap[i].patch->boundMax=QVector2D(v1,v2);
                }
                fread(&tracingMap[i].patch->n_included,sizeof(float),1,f);
                fread(&tracingMap[i].patch->label_type,sizeof(int),1,f);
                fread(&tracingMap[i].patch->index,sizeof(int),1,f);
                if(tracingMap[i].patch->index>max_ind.x()){
                    max_ind.setX(tracingMap[i].patch->index);
                }

            }
            fread(&tracingMap[i].debug_middle_node,sizeof(tracingMap[i].debug_middle_node),1,f);
            fread(&tracingMap[i].predicted_label,sizeof(tracingMap[i].predicted_label),1,f);
            fread(&tracingMap[i].trainingSetIndex,sizeof(tracingMap[i].trainingSetIndex),1,f);
            fread(&tracingMap[i].trainingSetType,sizeof(tracingMap[i].trainingSetType),1,f);
            fread(&tracingMap[i].temp_ck,sizeof(tracingMap[i].temp_ck),1,f);
            fread(&tracingMap[i].corrected_type,sizeof(tracingMap[i].corrected_type),1,f);
            fread(&tracingMap[i].neurite_ind,sizeof(tracingMap[i].neurite_ind),1,f);
            if(tracingMap[i].neurite_ind>max_ind.y()){
                max_ind.setY(tracingMap[i].neurite_ind);
            }

            if(tracingMap[i].type>0 && tracingMap[i].neurite_ind!=-1){
                neurite_sample_point[tracingMap[i].neurite_ind]=i;

            }

        }
        fclose(f);
        return max_ind;
    }

    void changeMito_signal(){

        int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

        memset(mitoLabelImage_signal,0,2*ImageW*ImageH);
        mitoData_signal.clear();
        int ind=1;
        unsigned char *ck=new unsigned char[ImageW*ImageH]();
        for(int iy=0;iy<ImageH;iy++){
            for(int ix=0;ix<ImageW;ix++){
                int tP=iy*ImageW+ix;
                if(ck[tP]==false){
                    ck[tP]=true;
                    float v=color_table_mito[imageData2[iy*ImageW+ix]];
                    if(v>=structure_thresh){
                        Mito newL;
                        newL.index=ind;
                        newL.xmax=ix;
                        newL.ymax=iy;
                        newL.xmin=ix;
                        newL.ymin=iy;


                        QQueue<int> quex,quey;
                        QQueue<int> quex2,quey2;
                        quex2.enqueue(ix);
                        quey2.enqueue(iy);
                        quex.enqueue(ix);
                        quey.enqueue(iy);
                        int cnt=0;
                        while(!quex.empty()){
                            int curx=quex.dequeue();
                            int cury=quey.dequeue();

                            for(int d=0;d<4;d++){
                                if(curx+dxylist[d][0]<0 || curx+dxylist[d][0]>=ImageW || cury+dxylist[d][1]<0 || cury+dxylist[d][1]>=ImageH){
                                    continue;
                                }
                                int tP=(cury+dxylist[d][1])*ImageW + curx+dxylist[d][0];
                                if(ck[tP]==false){
                                    ck[tP]=true;
                                    float v=color_table_mito[imageData2[tP]];
                                    if(v>=structure_thresh){
                                        quex.enqueue(curx+dxylist[d][0]);
                                        quey.enqueue(cury+dxylist[d][1]);
                                        quex2.enqueue(curx+dxylist[d][0]);
                                        quey2.enqueue(cury+dxylist[d][1]);
                                        cnt++;
                                        if(curx+dxylist[d][0]<newL.xmin)newL.xmin=curx+dxylist[d][0];
                                        if(curx+dxylist[d][0]>newL.xmax)newL.xmax=curx+dxylist[d][0];
                                        if(cury+dxylist[d][1]<newL.ymin)newL.ymin=cury+dxylist[d][1];
                                        if(cury+dxylist[d][1]>newL.ymax)newL.ymax=cury+dxylist[d][1];
                                    }
                                }
                            }
                        }
                        if(cnt>=4){
                            while(!quex2.empty()){
                                int curx=quex2.dequeue();
                                int cury=quey2.dequeue();
                                mitoLabelImage_signal[cury*ImageW+curx]=ind;

                            }
                            mitoData_signal.push_back(newL);
                            ind++;
                        }
                    }
                }
            }
        }
    }
};


//#define WINDOW_SCALE qApp->desktop()->logicalDpiX()/96.0
class spineDetectionWorker : public QObject
{
    Q_OBJECT

public:
    spineDetectionWorker(Window *p){
        c_window=p;
    }
private:
    Window *c_window;

public slots:
    void run(QString query){
        system(query.toStdString().c_str());
    }
};


class DataGroup{
public:
    QLineEdit *name=NULL;
    QPushButton *add=NULL;
    QPushButton *remove=NULL;
    QPushButton *colorChange=NULL;
    QColor color;
    QListWidget *dataList=NULL;
    QWidget *widget=NULL;
    QHBoxLayout *layout1=NULL;
    QVBoxLayout *layout2=NULL;

    int groupIndex=0;

    DataGroup(int a){
        QFont title_font("Arial", 9, QFont::Bold);
        name = new QLineEdit();
        name->setObjectName(QString::number(a));
        name->setText(QString("Group_")+QString::number(a));
        name->setFont(title_font);

        color = QColor((rand()*255)/RAND_MAX,(rand()*255)/RAND_MAX,(rand()*255)/RAND_MAX);

        colorChange = new QPushButton;
        colorChange->setObjectName(QString::number(a));
        colorChange->setFixedSize(QSize(15, 15));
        colorChange->setFlat(true);
        colorChange->setAutoFillBackground(true);
        colorChange->setStyleSheet(QString("background-color: %1;foreground-color: %1; border-style: none;").arg(color.name()));


        add=new QPushButton;
        add->setObjectName(QString::number(a));
        add->setIcon(QIcon("Resource/icon_add.png"));
        add->setIconSize(QSize(20, 20));
        add->setFixedSize(QSize(25, 25));
        QPalette pal = add->palette();
        pal.setColor(QPalette::Button, QColor(0,0,0,50));
        add->setPalette(pal);
        add->setFlat(true);
        //connect(&remove, SIGNAL(clicked()), this, SLOT(handleDeleteBtn()));

        remove=new QPushButton;
        remove->setObjectName(QString::number(a));
        remove->setIcon(QIcon("Resource/icon_trash.png"));
        remove->setIconSize(QSize(20, 20));
        remove->setFixedSize(QSize(25, 25));
        pal = remove->palette();
        pal.setColor(QPalette::Button, QColor(0,0,0,50));
        remove->setPalette(pal);
        remove->setFlat(true);

        layout1 = new QHBoxLayout;
        layout1->addWidget(name);
        layout1->addWidget(add);
        layout1->addWidget(colorChange);
        layout1->addWidget(remove);

        dataList=new QListWidget;
        dataList->setMouseTracking(true);
        dataList->setFocusPolicy(Qt::ClickFocus);
//        dataList->setAutoFillBackground(true);
        pal=dataList->palette();
        pal.setColor(QPalette::Base,QColor(50,50,50));
        dataList->setPalette(pal);
        //dataList->setStyle(new DarkStyle);



        layout2 = new QVBoxLayout;
        layout2->addLayout(layout1);
        layout2->addWidget(dataList);


        widget = new QWidget;
        widget->setLayout(layout2);
        //widget->setStyle(new DarkStyle);
        widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    }

    void setIndex(int index) {
        name->setObjectName(QString::number(index));
        colorChange->setObjectName(QString::number(index));
        add->setObjectName(QString::number(index));
        remove->setObjectName(QString::number(index));
    }
};




struct forSort1 {
  int index;
  float value;
  bool operator<(const forSort1 &other) const {
    return (value < other.value);
  }
};
struct forSort2 {
  int index;
  float value;
  bool operator<(const forSort2 &other) const {
    return (value > other.value);
  }
};


class Window : public QWidget
{
    Q_OBJECT

public:
    Window(MainWindow *parent);
    ~Window();


    QVector <MitoDataset*> dataset;


    void loadFile(const QUrl &fileName);

    QThread spineDetectionThread;

    QComboBox *projection1;
    QComboBox *projection2;

    float WINDOW_SCALE;

    QGroupBox *clusterGroup;
    QGroupBox *typeGroup;
    QGroupBox *coloringGroup;


    float maxX,maxY,maxZ,minX,minY,minZ;

    bool setType;
    bool setPrePost;
    bool setKD;
    bool setCTL;
    QString dataLocation;

    QVector<QString> spineNames;
    QVector<QColor> spineColors;
    int focusSpine;
    bool clicked;
    QVector<bool> spineEnable;
    int enabledSpineNum;
    int featureNum;

    bool typeEnable;
    bool testTrainEnable;
    QVector<QString> types;
    QVector<int> typesInt;
    QColor typeColors[100];

    QVector<int> TestTrain;
    QColor testTrainColors[3];


//    QVector<QVector2D> selectionRange; //selection for each feature
//    QVector<bool> selection; //if there is selection on the exis, selection[featureN]=true;
    QVector<QString> featureNames;
    QVector<QVector2D> featureRanges; // feature0 <min,max>: <featureRanges[0].x,featureRanges[0].y>

    QVector<int> spineGroups;
    QVector<bool> spineCorrect;

    QVector<QVector<float>> data; // spine1: data[1][0] ~ data[1][feature_number-1]

    QVector2D selectionAreaStart;
    QVector2D selectionAreaEnd;
    bool AreaSelection;

    QVector<QVector2D> tsneResult; //normalized result

    float centers[30][10];
    QColor clusterColors[30];

    QVector<int> clusterLabel;

    bool isFocus;

    int coloringType=0; //0: Data  1: Cluster  2: Type

    int numCluster;

    QCheckBox *clusterOnOff[30];

    QCheckBox *typeOnOff[10];

    QCheckBox *testTrainOnOff[3];

    void checkSpineEnable();

    QVector<bool> featureEnables;
    QVector<float> featureWeights;
    bool IsAutoControl;


    QString constrains[20];

    float minValues[30];
    float maxValues[30];
    float avgValues[30];

    int enabledNum=0;


    QSpinBox *GroupNumSet;
    int currentSelectGroup;
    QList<int> selectedSpines[30];
    QVector<int> indexOfSelectedSpines;

    void addSpineToGroup(int a,int group);
    void deleteSpineFromGroup(int a,int group);


    bool seedChanged;


    bool IsShift;

    void changeTitle();

    FILE *newFeature;
    QHBoxLayout *clusterOnOffLayout;

    QCheckBox *selectionMode;
    QCheckBox *groupChangeMode;

    QCheckBox *spineEnableMode;
    QCheckBox *lookEnabled;

    QCheckBox *drawLine;

    QCheckBox *lookSelectedSpine;




    bool *displaySpines;



    QSlider *SetMin;
    QSlider *SetMax;


    colorBar *colorbar;

    QPushButton *SelectiveTsne;
    QPushButton *AlltSNE;
    QPushButton *AlltSNEForTrend;
    QPushButton *AllPCAForTrend;



    bool enableRotation=true;


    void generateFlow();

    float minFlow[4][30];
    float maxFlow[4][30];
    float avgFlow[4][30];
    int nFlow[4];


    void arrangeRanderPart();

    QListWidget *groupList;
    QPushButton *groupAdd;
    QPushButton *groupDelete;
    QLabel *groupLabel;

    QHBoxLayout *groupLayout1;
    QVBoxLayout *groupLayout2;

    QSplitter *dataLayout1;
    QSplitter *dataLayout2;


    GlWidgetForData* dataRendering;

    QGridLayout *dataMeshRenderPart;
    QVBoxLayout *dataMeshLayout;

    QSplitter *groupInfoLayout;
    QList<int> groupInfoSizes;

    QVBoxLayout *groupInfoLayout2;




    int curDataIndex=-1;
    int prevDataIndex=-1;
    QString curDataName="";
    int globalDataIndex=0;

    QVector<DataGroup*>groups;

    QSpinBox *preGroup;
    QSpinBox *postGroup;
    QVBoxLayout *groupCompareLayout;

    variationGraph *comparisonPlot;

    QGroupBox *groupCompareBox;



    QSlider *imageStructureOpacityControl;
    QSlider *imageMitoLabelOpacityControl;
    QSlider *imageMitoThresOpacityControl;


    QVBoxLayout *imageLabelOpacityLayout;
    QVBoxLayout *imageControlLayout1;
    QHBoxLayout *imageControlLayout1_1;

    QHBoxLayout *imageControlLayout2;
    QVBoxLayout *imageControlLayout2_1;

    QGroupBox *imageControlLayoutGroup;
    QGroupBox *imageLabelOpacityLayoutGroup;



    QPushButton *correctorUpdate;

    QPushButton *StructureLabelLoad;

    QPushButton *StructurePatchLoad;

    QPushButton *StructureScribbleLoad;

    void patchCorrection(int curP, int target, MitoDataset *curData);

    QPushButton *correctorInput_tracingP;
    QPushButton *correctorInput_positivePath;
    QPushButton *correctorInput_negativePath;
    QPushButton *correctorInput_nodeEraser;


    QPushButton *correctorInput_background;
    QPushButton *correctorInput_cellbody;
    QPushButton *correctorInput_axon;
    QPushButton *correctorInput_dend;

    QPushButton *correctorInput_mito_foreground;
    QPushButton *correctorInput_mito_background;
    QPushButton *correctorInput_mito_splitting;
    QPushButton *correctorInput_mito_merging;


    QCheckBox *correctorEnable_axon;
    QCheckBox *correctorEnable_dend;
    QSlider *correctorSensibilityControl;
    QSlider *correctorOpacityControl;
    QVBoxLayout *correctorLayout;
    QGroupBox *correctorLayoutGroup;

    QPushButton *subsetAdd;
    QPushButton *subsetErase;
    QPushButton *subsetGenerate;
    QLineEdit *subsetName;
    QSlider *subsetBrushSize;
    QVBoxLayout *subsetLayout;
    QGroupBox *subsetLayoutGroup;


    QSlider *connectionBackgroundThreshold;
    QSlider *connectionDistanceThreshold;
    QSlider *connectionVisibility;
    QSlider *connectionFillThreshold;

    QVBoxLayout *connectionLayout;
    QGroupBox *connectionLayoutGroup;



    // error correction
    QPushButton *mitoCorrectorBrush;
    QPushButton *mitoLabelUpdate;
    QSlider *mitoSimilarity;
    QHBoxLayout *mitoCorrectorLayout;
    QGroupBox * mitoCorrectorLayoutGroup;


    QVBoxLayout *imageControlWholeLayout;

    QHBoxLayout *Rt2;

    QVBoxLayout *onoffLayout;

    QGroupBox *datagroupGroup;
    QCheckBox *datagroupOnOff[100];
    QGridLayout *datagroupLayout;

    QCheckBox *pcp_expand;

    QCheckBox *plot_expand;

    QPushButton *GenerateMorpData;
    QLineEdit *OutputPath;
    QLabel *OutputPathLabel;
    QGroupBox *DatasetWidget;
    QProgressBar *DatasetGenerateProgress;
    QLabel *ProgressLabel;



    QSlider *imageBrightControl_neuron;
    QSlider *imageContrastControl_neuron;
    QSlider *imageContrastPosControl_neuron;
    imageControlGraph *tfRendering_neuron;

    QSlider *imageBrightControl_mito;
    QSlider *imageContrastControl_mito;
    QSlider *imageContrastPosControl_mito;
    imageControlGraph *tfRendering_mito;

    QTabWidget *imageControlPartLayout;


    QVector<QVector2D> globalAxisRange;
    QVector<bool> globalSelection;
    QVector<QVector2D> globalSelectionRange;
    QVector2D globalPlotXaxisRange=QVector2D(0,1);
    QVector2D globalPlotYaxisRange=QVector2D(0,1);


    QGroupBox *StructureCorrection_GroupLayout;
    QGroupBox *MitoCorrection_GroupLayout;
    QPushButton *StructureModelSave;
    QPushButton *StructureModelLoad;
    QPushButton *MitoModelSave;
    QPushButton *MitoModelLoad;

    QString StructureModel_path="DL_model/origin_structure.pt";
    QString MitoModel_path="DL_model/origin_mitochondria.pth";

    QLineEdit *uncertainty_thres;
    QLineEdit *model_path;


    QGroupBox *analysisLayoutGroup;
    QLineEdit *resolution_edit;

    QCheckBox *grayscale_colormap;

protected:
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

signals:
    void runTSNE();
    void runSelectiveTSNE();
    void runSpineDetection(QString query);
    void projectionRun();

    void mitoUpdate();

    void selectDone(int res);


    void recommenderInit();
    void recommenderRun(int type);


public slots:
    void addNewPatch(QVector2D startPos,QVector2D endPos);


    void synchronization();
    void viewChange(int num);

    void selectDisplayMode(bool a);
    void displayShow(bool a);
    void selectReset();
    bool checkConstraint(int numConstrain,int spineNum);
    void featureUsageSetting();

    void handleGroupAdd();
    void handleGroupDelete();
    void addData();
    bool addDataSub(QStringList dir_list, int index);


    void focusGroup();
    void focusGroup(QString a);
    void currentDataChange(QListWidgetItem* a);
    void setUIbasedOnCurDataIndex();
    void UIChanged(int a);
    void renderVarGraph(int a);
    void colorChangeGroup();
    void groupNameChanged(QString a);

    void imageControlSliderChanged(int a);
    void StructureControlSliderChanged(int a);
    void MitoControlSliderChanged(int a);
    void ThresControlSliderChanged(int a);
    void ThresControlSliderReleased();
    void mitoControlReleased();

    void generateDataset();
    void checkDatasetGeneration();
    void orderingFormatrixView1();
    void orderingFormatrixView2();
    void orderingFormatrixView3();

    void preOrdering();

    void handleSnapshot();

    void plotAxis1Changed(int a);

    void plotAxis2Changed(int a);

    void ImageControlSliderChanged(int a);

    void changeAnalysisType(int a);

    void handleSnapshot_save();
    void AddSnapshotTitle();
    void boundaryConditionChanged(bool a);

    void structureChanged();
    void correctionChanged();
    void correctionChanged(int a);

    void connectionChanged();

    void correctorSensibilitySliderChanged(int a);

    void correctorInput_tracingP_pressed();
    void correctorInput_positivePath_pressed();
    void correctorInput_negativePath_pressed();
    void correctorInput_nodeEraser_pressed();

    void correctorInput_background_pressed();
    void correctorInput_cellbody_pressed();
    void correctorInput_axon_pressed();
    void correctorInput_dend_pressed();
    void correctorEnable_axon_clicked(bool a);
    void correctorEnable_dend_clicked(bool a);
    void correctorUpdate_pressed();
    void StructureLabelLoad_pressed();
    void StructurePatchLoad_pressed();
    void StructureScribbleLoad_pressed();

    void correctorInput_mito_background_pressed();
    void correctorInput_mito_foreground_pressed();
    void correctorInput_mito_splitting_pressed();
    void correctorInput_mito_merging_pressed();



    void subsetBrushSizeChanged(int a);
    void subsetAdd_pressed();
    void subsetErase_pressed();
    void subsetGenerate_pressed();
    void subsetName_changed(QString a);

    void controlTabChanged(int a);

    void connectionBackgroundThresholdChanged(int a);
    void connectionDistanceThresholdChanged(int a);
    void connectionVisibilityChanged(int a);
    void connectionFillThresholdChanged(int a);

    void startWaiting();
    void endWaiting();

    void foregroundThreshChanged(int a);

    void ReSegmentation_structure();
    void ReSegmentation_structure_finetune_ver();
    void ReSegmentation_mitochondria();
    void ReSegmentation_mitochondria_finetune_ver();

    void foregroundThreshStarted();
    void foregroundThreshEnded();

    void labelCleaning();

    void saveHistory();

    void setVariationPlot(int a);

    bool runChannelSelectDialog(QString dir);
    void handleCancelButton();
    void handleAcceptButton();

    void handle_StructureModelSave();
    void handle_StructureModelLoad();
    void handle_MitoModelSave();
    void handle_MitoModelLoad();


    void save_dataset_for_training(int data_ind);
    void save_dataset_for_training_all();
    int getPatchType(patchSet *patch, MitoDataset *curData);

    void load_pred_datatset(QString label_path, int data_ind, int iter_n);



    void hideAllRadars(int ind);
    void finetune_structure_segmentation_vis();

    void resolution_changed(QString v);

private slots:
    void runCheckSpineEnable(bool a);
    void runClassification();


    void changeFocus(bool a);
    void kmeansClustering();
    void optimizedKmeansClustering();

    void changeColoringType(int a);
    void selectionForClustering();

    void saveResult();


public:

    bool arranging=false;
    float matrixD[1000][1000];
    float matrixN[1000][1000];
    int indexPos[1000][1000];
    int posX[1000];
    int posY[1000];
    void recursiveDivideX(int startX,int endX,int startY,int endY,QVector<int> indexList);
    void recursiveDivideY(int startX,int endX,int startY,int endY,QVector<int> indexList);

    int processedIndex=-1;
    QString processedPath="";
    QTimer *processedTimer;
    QListWidgetItem *processedItem=NULL;


//    QTabWidget *RenderPart;
    QGridLayout *RenderPart;
    QSplitter *Frame1;
    QHBoxLayout *Frame2;
    QVBoxLayout *ControlPart;
    QVBoxLayout *ControlPart2;
    QTabWidget *totalFrame;

    QSlider *SetMean;

    QSplitter *plotLayout;


    QSlider *createSlider(int a);

//    QVector<GLWidget*> spines;

    ParallelCoordinate *parallelPlot;
    tsneSelection *tsneGraph;

    int dataNum;

    int curItem;

    bool dummy;
    QSpacerItem *dummyItem;

    QCheckBox *enableFocus;

    QLabel *GroupNumLabel;
    QPushButton *RunClustering1;
    QPushButton *RunClustering2;

    QHBoxLayout *clusterLayout;

    QComboBox *coloringTypeSet;

    QHBoxLayout *typeOnOffLayout;


    QHBoxLayout *TestTrainOnOffLayout;


    QPushButton *saveClusteringResult;

    QPushButton *displaySelectResult;


    MainWindow *mainWindow;

    int groupIndexOfDataList[1000];

    QListWidget *snapshotList;
    QPushButton *snapshot;
    QPushButton *snapshot_save;

    QComboBox *analysis_type;

    int focus_group=-1;


    QVector <QLineEdit*> snapshot_annotation;
    QVector <QString> snapshot_group;
    QVector <QString> snapshot_image;
    QVector <int> snapshot_count;
    QVector <float> snapshot_density;
    QVector <float> snapshot_dendriteArea;
    QVector <float> snapshot_axonArea;
    QVector <float> snapshot_mitoAreaOnDendrite;
    QVector <float> snapshot_mitoAreaOnAxon;


    QVector <QVector<float>> snapshot_features;


    QCheckBox *boundaryOnOff;

    QLabel *waiting;
    QMovie *movie;

    QPushButton *ReSegmentation_structure_button;
    QPushButton *ReSegmentation_mitochondria_button;

    QLabel *title_no;
    QLabel *title_anno;
    QLabel *title_group;
    QLabel *title_image;

    QWidget *filer_variationLayout;

    bool selectDialogResult=false;



    GlWidgetForTracingMap *miniMap;



    QGridLayout *recom_layout;

    patchRecommender *recommender;
    QThread workerThread_recommender;
    void setRecommendation_thread();

    QVector <GlWidgetForInset*>recoms;

    QVector <QVector<patchSet*>> neurites;

//    QCheckBox *correction_type;
    QComboBox *correction_type;
    QComboBox *correction_type_mito;

    QCheckBox *show_tracing_map;

    int global_neurite_ind=0;
    int global_patch_ind=0;

    int mitoSelectIndex=0;
    int venusSelectIndex=0;


    QVBoxLayout *structureCorrection_part_layout1;
    QVBoxLayout *structureCorrection_part_layout2;


    QLabel *imageStructureOpacityControl_label;
    QHBoxLayout *imageStructureOpacityControl_layout1;
    QHBoxLayout *imageStructureOpacityControl_layout2;

    QLabel *imageMitoLabelOpacityControl_label;
    QHBoxLayout *imageMitoLabelOpacityControl_layout1;
    QHBoxLayout *imageMitoLabelOpacityControl_layout2;
    QVBoxLayout *mitoCorrection_part_layout;


    QLabel *mito_bright_control_label;
    QHBoxLayout *mito_bright_control_layout1;
    QHBoxLayout *mito_bright_control_layout2;


    QVBoxLayout *tf_mito_layout;


    QString getRelativePath(QString path){
        QString absolute_path=QDir(".").absolutePath();
        QString relative_path=QDir(absolute_path).relativeFilePath(path);
        return relative_path;
    }

    void VIS22_userstudy_save(int user_iter);
    QTime VIS22_time;
    int VIS22_prev_elapsed_time=0;

    int VIS22_initial_deploy_time=0;


};

#endif
