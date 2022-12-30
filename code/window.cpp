///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

//#define SERVER_ver
//#define HJ_SERVER_ver
//#define JY_SERVER_ver

//#define VIS22

#include "window.h"
#include "mainwindow.h"
#include "structureRefinement.h"
#include "structureConnector.h"
#include "patchrecommender.h"
#include "feature_space_radar.h"

unsigned short ReverseShort( const unsigned short inShort )
{
   unsigned short retVal;
   char *shortToConvert = ( char* ) & inShort;
   char *returnShort = ( char* ) & retVal;

   // swap the bytes into a temporary buffer
   returnShort[0] = shortToConvert[1];
   returnShort[1] = shortToConvert[0];
   return retVal;
}


Window::Window(MainWindow *parent)
{

#ifdef VIS22
    VIS22_time.start();
    startTimer(10000);
#endif

    featureNum=5;
    featureNames.push_back("Structure");
    featureNames.push_back("Area");
    featureNames.push_back("Length");
    featureNames.push_back("Eccentricity");
//    featureNames.push_back("Perimeter");
    featureNames.push_back("Circularity");


    for(int i=0;i<featureNum;i++){
//        selectionRange.push_back(QVector2D(0,0));
//        selection.push_back(false);
        featureRanges.push_back(QVector2D(1000,-1000));
    }



    waiting = new QLabel;
    movie = new QMovie("Resource/waiting.gif");
    movie->setScaledSize(QSize(100,100));
    waiting->setMovie(movie);
    waiting->setFixedSize(100,100);
    waiting->setAutoFillBackground(true);
    waiting->setWindowFlags(Qt::SplashScreen);
    QPalette tttp = waiting->palette();
    tttp.setColor(QPalette::Background, QColor(255,255,255));
    waiting->setPalette(tttp);

    qDebug()<<"1";

    qsrand(1);
    mainWindow=parent;

    WINDOW_SCALE=qApp->desktop()->logicalDpiX()/96.0;

    AlltSNE=new QPushButton("Projection");
    AlltSNE->setStyle(new DarkStyle);
    qDebug()<<"2";

    IsShift=false;
    for(int i=0;i<100;i++){
        typeColors[i]=QColor(qrand()%206+50,qrand()%206+50,qrand()%206+50);
    }
    typeColors[3]=QColor(qrand()%206+50,qrand()%206+50,qrand()%206+50);
    typeColors[3]=QColor(qrand()%206+50,qrand()%206+50,qrand()%206+50);
    typeColors[3]=QColor(qrand()%206+50,qrand()%206+50,qrand()%206+50);

    typeColors[2]=QColor(255,142,138);
    typeColors[3]=QColor(30,126,220);


    isFocus=true;
    AreaSelection=false;
    qDebug()<<"3";

    curItem=-1;

    parallelPlot=new ParallelCoordinate(this);
    connect(parallelPlot,SIGNAL(viewChange(int)),this,SLOT(viewChange(int)));
    qDebug()<<"4";

    tsneGraph=new tsneSelection(this);
    connect(tsneGraph,SIGNAL(viewChange(int)),this,SLOT(viewChange(int)));

    qDebug()<<"5";

    connect(parallelPlot,SIGNAL(synchronization()),tsneGraph,SLOT(update()));
    connect(tsneGraph,SIGNAL(synchronization()),parallelPlot,SLOT(update()));


    pcp_expand=new QCheckBox("Fit");
    pcp_expand->setStyle(new DarkStyle);
    pcp_expand->setStyleSheet("QCheckBox {font: bold;color: rgb(0,0,0)}");
    connect(pcp_expand,SIGNAL(released()),parallelPlot,SLOT(update()));


    QVBoxLayout *pcp_layout=new QVBoxLayout;
    pcp_layout->setAlignment(Qt::AlignRight);
    pcp_layout->addWidget(parallelPlot);

    QWidget* filler=new QWidget;
    filler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *pcp_temp=new QHBoxLayout;
    pcp_temp->addWidget(filler);
    pcp_temp->addWidget(pcp_expand);
    pcp_temp->addSpacing(30);

    pcp_layout->addLayout(pcp_temp);

    QWidget *pcp_widget=new QWidget;
    pcp_widget->setLayout(pcp_layout);

    currentSelectGroup=0;


    coloringTypeSet=new QComboBox;
    coloringTypeSet->addItem("Assign based on Structure");
    coloringTypeSet->addItem("Assign based on Group");
    coloringTypeSet->setStyle(new DarkStyle);
    coloringTypeSet->setStyleSheet(QString("QComboBox {font: bold;}"));
    connect(coloringTypeSet,SIGNAL(currentIndexChanged(int)),this,SLOT(changeColoringType(int)));
    coloringGroup=new QGroupBox("Representative Color Setting");
    coloringGroup->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);}"));
    coloringGroup->setFixedHeight(60*WINDOW_SCALE);
    QHBoxLayout *tColoringLayout=new QHBoxLayout;
    tColoringLayout->addWidget(coloringTypeSet);
    coloringGroup->setLayout(tColoringLayout);


    analysis_type=new QComboBox;
    analysis_type->addItem("One image");
    analysis_type->addItem("Whole data");
    analysis_type->setStyle(new DarkStyle);
    analysis_type->setStyleSheet(QString("QComboBox {font: bold;}"));
    connect(analysis_type,SIGNAL(currentIndexChanged(int)),this,SLOT(changeAnalysisType(int)));
    QGroupBox *analysisGroup=new QGroupBox("Analysis Type");
    analysisGroup->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);}"));
    analysisGroup->setFixedHeight(60*WINDOW_SCALE);
    QHBoxLayout *tAnalysisLayout=new QHBoxLayout;
    tAnalysisLayout->addWidget(analysis_type);
    analysisGroup->setLayout(tAnalysisLayout);


    datagroupLayout=new QGridLayout;
    datagroupGroup=new QGroupBox("Data Group");
    datagroupGroup->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);} "));
    datagroupGroup->setLayout(datagroupLayout);
    qDebug()<<"6_4";

    onoffLayout=new QVBoxLayout;
    onoffLayout->addWidget(datagroupGroup);

    QHBoxLayout *tcombolayout=new QHBoxLayout;
//    tcombolayout->addWidget(analysisGroup);
//    tcombolayout->addWidget(coloringGroup);

    qDebug()<<"6_1";

    projection1=new QComboBox;
    projection1->addItem("PCA");
    projection1->addItem("UMAP");
    for(int i=0;i<featureNum;i++){
        projection1->addItem(featureNames[i]);
    }
    projection1->setCurrentIndex(2);
    projection1->setStyle(new DarkStyle);
    projection1->setStyleSheet(QString("QComboBox {font: bold;}"));
    connect(projection1,SIGNAL(currentIndexChanged(int)),this,SLOT(plotAxis1Changed(int)));

    projection2=new QComboBox;
    projection2->addItem("PCA");
    projection2->addItem("UMAP");
    for(int i=0;i<featureNum;i++){
        projection2->addItem(featureNames[i]);
    }
    projection2->setCurrentIndex(3);
    projection2->setStyle(new DarkStyle);
    projection2->setStyleSheet(QString("QComboBox {font: bold;}"));
    connect(projection2,SIGNAL(currentIndexChanged(int)),this,SLOT(plotAxis2Changed(int)));

    tsneGraph->makeCoord();

    AlltSNE->setStyleSheet(QString("QPushButton {font: bold;}"));
    qDebug()<<"6_2";

    QLabel *axisLabel1=new QLabel("X-Axis: ");
    axisLabel1->setStyleSheet(QString("QLabel {font: Bold;}"));

    QLabel *axisLabel2=new QLabel("Y-Axis: ");
    axisLabel2->setStyleSheet(QString("QLabel {font: Bold;}"));



    plot_expand=new QCheckBox("Fit");
    plot_expand->setStyle(new DarkStyle);
    plot_expand->setStyleSheet("QCheckBox {font: bold;color: rgb(0,0,0)}");
    connect(plot_expand,SIGNAL(released()),tsneGraph,SLOT(update()));


    QWidget *filler2=new QWidget;
    filler2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    QHBoxLayout *ttSNE=new QHBoxLayout;
    ttSNE->addWidget(filler2);
    ttSNE->addWidget(axisLabel1);
    ttSNE->addWidget(projection1);
    ttSNE->addSpacing(40);
    ttSNE->addWidget(axisLabel2);
    ttSNE->addWidget(projection2);
    ttSNE->addSpacing(40);
    ttSNE->addWidget(plot_expand);

//    ttSNE->addWidget(AlltSNE);
    qDebug()<<"6_3";

    QVBoxLayout *sub1ControlPart2=new QVBoxLayout;

//    sub1ControlPart2->addWidget(coloringGroup);
    sub1ControlPart2->addWidget(tsneGraph);
    sub1ControlPart2->addLayout(ttSNE);



    qDebug()<<"6_5";


    groupLabel=new QLabel("Group List");
    QFont tff;
    tff.setBold(true);
    groupLabel->setFont(tff);

    groupAdd=new QPushButton;
    groupAdd->setIcon(QIcon("Resource/icon_add_black.png"));
    groupAdd->setIconSize(QSize(20, 20));
    groupAdd->setFixedSize(QSize(25, 25));
    QPalette pal = groupAdd->palette();
    pal.setColor(QPalette::Button, QColor(0,0,0,50));
    groupAdd->setPalette(pal);
    groupAdd->setFlat(true);
    connect(groupAdd, SIGNAL(clicked()), this, SLOT(handleGroupAdd()));

    qDebug()<<"6_5";

    groupList=new QListWidget;
    QPalette pal4 = groupList->palette();
    pal4.setColor(QPalette::Base, QColor(0,0,0,150));
    groupList->setPalette(pal4);
    groupList->setAutoFillBackground(true);

    groupLayout1=new QHBoxLayout;
    groupLayout1->addWidget(groupLabel);
    groupLayout1->addWidget(groupAdd);

    groupLayout2=new QVBoxLayout;
    groupLayout2->addLayout(groupLayout1);
    groupLayout2->addWidget(groupList);
    groupLayout2->addWidget(datagroupGroup);

    qDebug()<<"6_6";


    QWidget *groupLayout2Widget=new QWidget;
    groupLayout2Widget->setLayout(groupLayout2);
    qDebug()<<"7";




//    connect(parallelPlot,SIGNAL(itemClicked()),dataRendering,SLOT(shiftToItem()));

    qDebug()<<"8";

    imageStructureOpacityControl=new QSlider(Qt::Horizontal);
    imageStructureOpacityControl->setRange(0, 255);
    imageStructureOpacityControl->setSingleStep(1);
    imageStructureOpacityControl->setTickInterval(25);
    imageStructureOpacityControl->setTickPosition(QSlider::TicksAbove);
    imageStructureOpacityControl->setValue(255);
    imageStructureOpacityControl->setStyle(new DarkStyle);

    imageMitoLabelOpacityControl=new QSlider(Qt::Horizontal);
    imageMitoLabelOpacityControl->setRange(0, 255);
    imageMitoLabelOpacityControl->setSingleStep(1);
    imageMitoLabelOpacityControl->setTickInterval(25);
    imageMitoLabelOpacityControl->setTickPosition(QSlider::TicksAbove);
    imageMitoLabelOpacityControl->setValue(255);
    imageMitoLabelOpacityControl->setStyle(new DarkStyle);

    connect(imageStructureOpacityControl,SIGNAL(valueChanged(int)),this,SLOT(StructureControlSliderChanged(int)));
    connect(imageMitoLabelOpacityControl,SIGNAL(valueChanged(int)),this,SLOT(MitoControlSliderChanged(int)));



    imageMitoThresOpacityControl=new QSlider(Qt::Horizontal);
    imageMitoThresOpacityControl->setRange(1, 255);
    imageMitoThresOpacityControl->setSingleStep(1);
    imageMitoThresOpacityControl->setTickInterval(25);
    imageMitoThresOpacityControl->setTickPosition(QSlider::TicksAbove);
    imageMitoThresOpacityControl->setValue(128);
    imageMitoThresOpacityControl->setStyle(new DarkStyle);

    connect(imageMitoThresOpacityControl,SIGNAL(valueChanged(int)),this,SLOT(ThresControlSliderChanged(int)));
//    connect(imageMitoThresOpacityControl,SIGNAL(sliderReleased()),this,SLOT(ThresControlSliderReleased()));


    QLabel *tl3=new QLabel("Mito threshold");
    tl3->setFont(tff);


    imageStructureOpacityControl_label=new QLabel("Structure");
    imageStructureOpacityControl_label->setFont(tff);

    imageStructureOpacityControl_layout1=new QHBoxLayout;
    imageStructureOpacityControl_layout1->addWidget(imageStructureOpacityControl_label);
    imageStructureOpacityControl_layout1->addWidget(imageStructureOpacityControl);
//    tlayout1->addWidget(ReSegmentation_structure_button);




    imageMitoLabelOpacityControl_label=new QLabel("Detected Mitochondria");
    imageMitoLabelOpacityControl_label->setFont(tff);


    imageMitoLabelOpacityControl_layout1=new QHBoxLayout;
    imageMitoLabelOpacityControl_layout1->addWidget(imageMitoLabelOpacityControl_label);
    imageMitoLabelOpacityControl_layout1->addWidget(imageMitoLabelOpacityControl);
//    tlayout2->addWidget(ReSegmentation_mitochondria_button);

    boundaryOnOff=new QCheckBox("Mitochondria Boundary On/Off");
    boundaryOnOff->setStyle(new DarkStyle);
    boundaryOnOff->setStyleSheet("QCheckBox {font: bold;color: rgb(0,0,0)}");
    boundaryOnOff->setLayoutDirection(Qt::LeftToRight);
    connect(boundaryOnOff,SIGNAL(clicked(bool)),this,SLOT(boundaryConditionChanged(bool)));


    imageLabelOpacityLayout=new QVBoxLayout;
    imageLabelOpacityLayout->addLayout(imageStructureOpacityControl_layout1);
    imageLabelOpacityLayout->addLayout(imageMitoLabelOpacityControl_layout1);
    imageLabelOpacityLayout->addWidget(boundaryOnOff);



    //imageLabelOpacityLayout->addLayout(tlayout3);
    boundaryOnOff->setChecked(true);



    imageLabelOpacityLayoutGroup=new QGroupBox;
    imageLabelOpacityLayoutGroup->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);} "));
    imageLabelOpacityLayoutGroup->setLayout(imageLabelOpacityLayout);







    imageBrightControl_neuron=new QSlider(Qt::Horizontal);
    imageBrightControl_neuron->setRange(0, 600);
    imageBrightControl_neuron->setSingleStep(1);
    imageBrightControl_neuron->setTickInterval(25);
    imageBrightControl_neuron->setTickPosition(QSlider::TicksAbove);
    imageBrightControl_neuron->setValue(128);
    imageBrightControl_neuron->setStyle(new DarkStyle);

    connect(imageBrightControl_neuron,SIGNAL(valueChanged(int)),this,SLOT(ImageControlSliderChanged(int)));


    imageContrastControl_neuron=new QSlider(Qt::Horizontal);
    imageContrastControl_neuron->setRange(0, 255);
    imageContrastControl_neuron->setSingleStep(1);
    imageContrastControl_neuron->setTickInterval(25);
    imageContrastControl_neuron->setTickPosition(QSlider::TicksAbove);
    imageContrastControl_neuron->setValue(50);
    imageContrastControl_neuron->setStyle(new DarkStyle);

    connect(imageContrastControl_neuron,SIGNAL(valueChanged(int)),this,SLOT(ImageControlSliderChanged(int)));


    imageContrastPosControl_neuron=new QSlider(Qt::Horizontal);
    imageContrastPosControl_neuron->setRange(0, 255);
    imageContrastPosControl_neuron->setSingleStep(1);
    imageContrastPosControl_neuron->setTickInterval(25);
    imageContrastPosControl_neuron->setTickPosition(QSlider::TicksAbove);
    imageContrastPosControl_neuron->setValue(128);
    imageContrastPosControl_neuron->setStyle(new DarkStyle);

    connect(imageContrastPosControl_neuron,SIGNAL(valueChanged(int)),this,SLOT(ImageControlSliderChanged(int)));

    tfRendering_neuron=new imageControlGraph(0,this);

    QLabel *tlabel1_neuron=new QLabel("Bright");
    tlabel1_neuron->setFont(tff);
    QLabel *tlabel2_neuron=new QLabel("Contrast");
    tlabel2_neuron->setFont(tff);
    QLabel *tlabel3_neuron=new QLabel("MidPos");
    tlabel3_neuron->setFont(tff);

    QHBoxLayout *tlayout1_neuron=new QHBoxLayout;
    tlayout1_neuron->addWidget(tlabel1_neuron);
    tlayout1_neuron->addWidget(imageBrightControl_neuron);

    QHBoxLayout *tlayout2_neuron=new QHBoxLayout;
    tlayout2_neuron->addWidget(tlabel2_neuron);
    tlayout2_neuron->addWidget(imageContrastControl_neuron);

    QHBoxLayout *tlayout3_neuron=new QHBoxLayout;
    tlayout3_neuron->addWidget(tlabel3_neuron);
    tlayout3_neuron->addWidget(imageContrastPosControl_neuron);

    QVBoxLayout *tlayout4_neuron=new QVBoxLayout;
    tlayout4_neuron->addLayout(tlayout1_neuron);
    tlayout4_neuron->addLayout(tlayout2_neuron);
    tlayout4_neuron->addLayout(tlayout3_neuron);

    QHBoxLayout *tlayout5_neuron=new QHBoxLayout;
    tlayout5_neuron->addLayout(tlayout4_neuron,2);
    tlayout5_neuron->addWidget(tfRendering_neuron,1);


    QGroupBox *tgroupbox_neuron=new QGroupBox;
    tgroupbox_neuron->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);} "));
    tgroupbox_neuron->setLayout(tlayout5_neuron);





    imageBrightControl_mito=new QSlider(Qt::Horizontal);
    imageBrightControl_mito->setRange(0, 600);
    imageBrightControl_mito->setSingleStep(1);
    imageBrightControl_mito->setTickInterval(25);
    imageBrightControl_mito->setTickPosition(QSlider::TicksAbove);
    imageBrightControl_mito->setValue(128);
    imageBrightControl_mito->setStyle(new DarkStyle);

    connect(imageBrightControl_mito,SIGNAL(valueChanged(int)),this,SLOT(ImageControlSliderChanged(int)));
    connect(imageBrightControl_mito,SIGNAL(sliderReleased()),this,SLOT(mitoControlReleased()));


    imageContrastControl_mito=new QSlider(Qt::Horizontal);
    imageContrastControl_mito->setRange(0, 255);
    imageContrastControl_mito->setSingleStep(1);
    imageContrastControl_mito->setTickInterval(25);
    imageContrastControl_mito->setTickPosition(QSlider::TicksAbove);
    imageContrastControl_mito->setValue(50);
    imageContrastControl_mito->setStyle(new DarkStyle);

    connect(imageContrastControl_mito,SIGNAL(valueChanged(int)),this,SLOT(ImageControlSliderChanged(int)));
    connect(imageContrastControl_mito,SIGNAL(sliderReleased()),this,SLOT(mitoControlReleased()));


    imageContrastPosControl_mito=new QSlider(Qt::Horizontal);
    imageContrastPosControl_mito->setRange(0, 255);
    imageContrastPosControl_mito->setSingleStep(1);
    imageContrastPosControl_mito->setTickInterval(25);
    imageContrastPosControl_mito->setTickPosition(QSlider::TicksAbove);
    imageContrastPosControl_mito->setValue(128);
    imageContrastPosControl_mito->setStyle(new DarkStyle);

    connect(imageContrastPosControl_mito,SIGNAL(valueChanged(int)),this,SLOT(ImageControlSliderChanged(int)));
    connect(imageContrastPosControl_mito,SIGNAL(sliderReleased()),this,SLOT(mitoControlReleased()));

    tfRendering_mito=new imageControlGraph(1,this);


    QHBoxLayout *tlayout3=new QHBoxLayout;
    tlayout3->addWidget(tl3);
    tlayout3->addWidget(imageMitoThresOpacityControl);



    mito_bright_control_label=new QLabel("Bright");
    mito_bright_control_label->setFont(tff);
    QLabel *tlabel2_mito=new QLabel("Contrast");
    tlabel2_mito->setFont(tff);
    QLabel *tlabel3_mito=new QLabel("MidPos");
    tlabel3_mito->setFont(tff);

    mito_bright_control_layout1=new QHBoxLayout;
    mito_bright_control_layout1->addWidget(mito_bright_control_label);
    mito_bright_control_layout1->addWidget(imageBrightControl_mito);

    QHBoxLayout *tlayout2_mito=new QHBoxLayout;
    tlayout2_mito->addWidget(tlabel2_mito);
    tlayout2_mito->addWidget(imageContrastControl_mito);

    QHBoxLayout *tlayout3_mito=new QHBoxLayout;
    tlayout3_mito->addWidget(tlabel3_mito);
    tlayout3_mito->addWidget(imageContrastPosControl_mito);

    QVBoxLayout *tlayout4_mito=new QVBoxLayout;
    tlayout4_mito->addLayout(mito_bright_control_layout1);
    tlayout4_mito->addLayout(tlayout2_mito);
    tlayout4_mito->addLayout(tlayout3_mito);
//    tlayout4_mito->addLayout(tlayout3);

    grayscale_colormap=new QCheckBox("Grayscale");
    grayscale_colormap->setStyle(new DarkStyle);
    grayscale_colormap->setStyleSheet("QCheckBox {font: bold;color: rgb(0,0,0)}");


    tf_mito_layout=new QVBoxLayout;
    tf_mito_layout->addWidget(tfRendering_mito);
    tf_mito_layout->addWidget(grayscale_colormap);

    QHBoxLayout *tlayout5_mito=new QHBoxLayout;
    tlayout5_mito->addLayout(tlayout4_mito,2);
    tlayout5_mito->addLayout(tf_mito_layout,1);


    QGroupBox *tgroupbox_mito=new QGroupBox;
    tgroupbox_mito->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);} "));
    tgroupbox_mito->setLayout(tlayout5_mito);


    // corrector layout setting
    {
        correctorSensibilityControl=new QSlider(Qt::Horizontal);
        correctorSensibilityControl->setRange(0, 255);
        correctorSensibilityControl->setSingleStep(1);
        correctorSensibilityControl->setTickInterval(25);
        correctorSensibilityControl->setTickPosition(QSlider::TicksAbove);
        correctorSensibilityControl->setValue(0);
        correctorSensibilityControl->setStyle(new DarkStyle);

        connect(correctorSensibilityControl,SIGNAL(valueChanged(int)),this,SLOT(correctorSensibilitySliderChanged(int)));




        correctorOpacityControl=new QSlider(Qt::Horizontal);
        correctorOpacityControl->setRange(0, 255);
        correctorOpacityControl->setSingleStep(1);
        correctorOpacityControl->setTickInterval(25);
        correctorOpacityControl->setTickPosition(QSlider::TicksAbove);
        correctorOpacityControl->setValue(255*0.3);
        correctorOpacityControl->setStyle(new DarkStyle);

        connect(correctorOpacityControl,SIGNAL(valueChanged(int)),this,SLOT(foregroundThreshChanged(int)));
        connect(correctorOpacityControl,SIGNAL(sliderPressed()),this,SLOT(foregroundThreshStarted()));
        connect(correctorOpacityControl,SIGNAL(sliderReleased()),this,SLOT(foregroundThreshEnded()));


        QLabel *tl4=new QLabel("Predict threshold");
        tl4->setFont(tff);

        QHBoxLayout *tlayout4=new QHBoxLayout;
        tlayout4->addWidget(tl4);
        tlayout4->addWidget(correctorSensibilityControl);



//        correction_type=new QCheckBox("Tracing based");
//        correction_type->setStyle(new DarkStyle);
//        correction_type->setStyleSheet(QString("QCheckBox {font: bold;}"));

        QLabel *tl_correction_type=new QLabel("Correction method: ");
        tl_correction_type->setFont(tff);

        correction_type=new QComboBox;
        correction_type->addItem("Manual");
        correction_type->addItem("Tracing based");
        correction_type->setStyle(new DarkStyle);
        correction_type->setStyleSheet(QString("QComboBox {font: bold;}"));
        correction_type->setCurrentIndex(1);

        show_tracing_map=new QCheckBox("Show tracing map");
        show_tracing_map->setStyle(new DarkStyle);
        show_tracing_map->setStyleSheet(QString("QCheckBox {font: bold;}"));
//        show_tracing_map->setLayoutDirection(Qt::RightToLeft);

        QWidget *filler_tlayout5=new QWidget;
        filler_tlayout5->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);


        QLabel *uncertainty_thres_label=new QLabel("Uncertainty Threshold: ");
        uncertainty_thres_label->setFont(tff);
        uncertainty_thres=new QLineEdit;
        uncertainty_thres->setText("0.7");


        QHBoxLayout *uncertainty_thres_layout=new QHBoxLayout;
        uncertainty_thres_layout->addWidget(uncertainty_thres_label);
        uncertainty_thres_layout->addWidget(uncertainty_thres);


        QHBoxLayout *ttl1=new QHBoxLayout;
        ttl1->addWidget(tl_correction_type);
        ttl1->addWidget(correction_type);
        ttl1->addSpacing(10);
        ttl1->addWidget(show_tracing_map);


        QWidget *filler_tlayout5_2=new QWidget;
        filler_tlayout5_2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

        imageStructureOpacityControl_layout2=new QHBoxLayout;

        structureCorrection_part_layout1=new QVBoxLayout;
        structureCorrection_part_layout1->addLayout(ttl1);
        structureCorrection_part_layout1->addLayout(imageStructureOpacityControl_layout2);
        //tlayout5->addLayout(ttl2);
//        tlayout5->addLayout(uncertainty_thres_layout);
        structureCorrection_part_layout1->addWidget(filler_tlayout5);
//        tlayout5->addSpacing(10);
//        tlayout5->addWidget(tl5);
//        tlayout5->addWidget(correctorOpacityControl);




        correctorInput_tracingP=new QPushButton("Tracing\nPoint");
        correctorInput_tracingP->setFixedSize(70,50);
        correctorInput_tracingP->setStyle(new DarkStyle);
        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(correctorInput_tracingP,SIGNAL(pressed()),this,SLOT(correctorInput_tracingP_pressed()));

        correctorInput_positivePath=new QPushButton("Positive\nPath");
        correctorInput_positivePath->setFixedSize(70,50);
        correctorInput_positivePath->setStyle(new DarkStyle);
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(correctorInput_positivePath,SIGNAL(pressed()),this,SLOT(correctorInput_positivePath_pressed()));

        correctorInput_negativePath=new QPushButton("Negative\nPath");
        correctorInput_negativePath->setFixedSize(70,50);
        correctorInput_negativePath->setStyle(new DarkStyle);
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(correctorInput_negativePath,SIGNAL(pressed()),this,SLOT(correctorInput_negativePath_pressed()));

        correctorInput_nodeEraser=new QPushButton("Node\nEraser");
        correctorInput_nodeEraser->setFixedSize(70,50);
        correctorInput_nodeEraser->setStyle(new DarkStyle);
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(correctorInput_nodeEraser,SIGNAL(pressed()),this,SLOT(correctorInput_nodeEraser_pressed()));


        correctorInput_background=new QPushButton("Back\nground");
        correctorInput_background->setFixedSize(50,50);
        correctorInput_background->setStyle(new DarkStyle);
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(correctorInput_background,SIGNAL(pressed()),this,SLOT(correctorInput_background_pressed()));

        correctorInput_cellbody=new QPushButton("Cell\nbody");
        correctorInput_cellbody->setFixedSize(50,50);
        correctorInput_cellbody->setStyle(new DarkStyle);
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(correctorInput_cellbody,SIGNAL(pressed()),this,SLOT(correctorInput_cellbody_pressed()));


        correctorInput_axon=new QPushButton("Axon");
        correctorInput_axon->setFixedSize(50,50);
        correctorInput_axon->setStyle(new DarkStyle);
        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(correctorInput_axon,SIGNAL(pressed()),this,SLOT(correctorInput_axon_pressed()));
        //        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,146,146);}"));


        correctorInput_dend=new QPushButton("Dend\nrite");
        correctorInput_dend->setFixedSize(50,50);
        correctorInput_dend->setStyle(new DarkStyle);
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
//        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(254,24,254);}"));
        connect(correctorInput_dend,SIGNAL(pressed()),this,SLOT(correctorInput_dend_pressed()));


        correctorInput_mito_background=new QPushButton("Exclude");
        correctorInput_mito_background->setFixedSize(55,50);
        correctorInput_mito_background->setStyle(new DarkStyle);
        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(correctorInput_mito_background,SIGNAL(pressed()),this,SLOT(correctorInput_mito_background_pressed()));


        correctorInput_mito_foreground=new QPushButton("Include");
        correctorInput_mito_foreground->setFixedSize(50,50);
        correctorInput_mito_foreground->setStyle(new DarkStyle);
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(correctorInput_mito_foreground,SIGNAL(pressed()),this,SLOT(correctorInput_mito_foreground_pressed()));

        correctorInput_mito_splitting=new QPushButton("Split");
        correctorInput_mito_splitting->setFixedSize(50,50);
        correctorInput_mito_splitting->setStyle(new DarkStyle);
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(correctorInput_mito_splitting,SIGNAL(pressed()),this,SLOT(correctorInput_mito_splitting_pressed()));


        correctorInput_mito_merging=new QPushButton("Merge");
        correctorInput_mito_merging->setFixedSize(50,50);
        correctorInput_mito_merging->setStyle(new DarkStyle);
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(correctorInput_mito_merging,SIGNAL(pressed()),this,SLOT(correctorInput_mito_merging_pressed()));


        correctorEnable_axon=new QCheckBox("Dend->Axon");
        correctorEnable_axon->setStyle(new DarkStyle);
        correctorEnable_axon->setStyleSheet(QString("QCheckBox {font: bold;}"));
        connect(correctorEnable_axon,SIGNAL(clicked(bool)),this,SLOT(correctorEnable_axon_clicked(bool)));


        correctorEnable_dend=new QCheckBox("Axon->Dend");
        correctorEnable_dend->setStyle(new DarkStyle);
        correctorEnable_dend->setStyleSheet(QString("QCheckBox {font: bold;}"));
        connect(correctorEnable_dend,SIGNAL(clicked(bool)),this,SLOT(correctorEnable_dend_clicked(bool)));


        correctorUpdate=new QPushButton("Save");
        correctorUpdate->setStyle(new DarkStyle);
        correctorUpdate->setStyleSheet(QString("QPushButton {font: bold;}"));
        connect(correctorUpdate,SIGNAL(pressed()),this,SLOT(correctorUpdate_pressed()));

        StructureLabelLoad=new QPushButton("Load");
        StructureLabelLoad->setStyle(new DarkStyle);
        StructureLabelLoad->setStyleSheet(QString("QPushButton {font: bold;}"));
        connect(StructureLabelLoad,SIGNAL(pressed()),this,SLOT(StructureLabelLoad_pressed()));


        StructurePatchLoad=new QPushButton("Load (patch_test)");
        StructurePatchLoad->setStyle(new DarkStyle);
        StructurePatchLoad->setStyleSheet(QString("QPushButton {font: bold;}"));
        connect(StructurePatchLoad,SIGNAL(pressed()),this,SLOT(StructurePatchLoad_pressed()));

        StructureScribbleLoad=new QPushButton("Load (Scribble)");
        StructureScribbleLoad->setStyle(new DarkStyle);
        StructureScribbleLoad->setStyleSheet(QString("QPushButton {font: bold;}"));
        connect(StructureScribbleLoad,SIGNAL(pressed()),this,SLOT(StructureScribbleLoad_pressed()));


        ReSegmentation_structure_button=new QPushButton("Fine-tuning");
        ReSegmentation_structure_button->setStyle(new DarkStyle);
        ReSegmentation_structure_button->setStyleSheet(QString("QPushButton {font: bold; color: rgb(0,0,0); background-color: rgb(255,255,255);}"));
        connect(ReSegmentation_structure_button,SIGNAL(pressed()),this,SLOT(finetune_structure_segmentation_vis()));



        ReSegmentation_mitochondria_button=new QPushButton("Re-segmentation");
        ReSegmentation_mitochondria_button->setStyle(new DarkStyle);
        ReSegmentation_mitochondria_button->setStyleSheet(QString("QPushButton {font: bold; color: rgb(0,0,0); background-color: rgb(255,255,255);}"));
        connect(ReSegmentation_mitochondria_button,SIGNAL(pressed()),this,SLOT(ReSegmentation_mitochondria_finetune_ver()));


        StructureModelSave=new QPushButton("Save model");
        StructureModelSave->setStyle(new DarkStyle);
        StructureModelSave->setStyleSheet(QString("QPushButton {font: bold; color: rgb(0,0,0); background-color: rgb(255,255,255);}"));
        connect(StructureModelSave,SIGNAL(pressed()),this,SLOT(handle_StructureModelSave()));

        StructureModelLoad=new QPushButton("Load model");
        StructureModelLoad->setStyle(new DarkStyle);
        StructureModelLoad->setStyleSheet(QString("QPushButton {font: bold; color: rgb(0,0,0); background-color: rgb(255,255,255);}"));
        connect(StructureModelLoad,SIGNAL(pressed()),this,SLOT(handle_StructureModelLoad()));

        MitoModelSave=new QPushButton("Save model");
        MitoModelSave->setStyle(new DarkStyle);
        MitoModelSave->setStyleSheet(QString("QPushButton {font: bold; color: rgb(0,0,0); background-color: rgb(255,255,255);}"));
        connect(MitoModelSave,SIGNAL(pressed()),this,SLOT(handle_MitoModelSave()));

        MitoModelLoad=new QPushButton("Load model");
        MitoModelLoad->setStyle(new DarkStyle);
        MitoModelLoad->setStyleSheet(QString("QPushButton {font: bold; color: rgb(0,0,0); background-color: rgb(255,255,255);}"));
        connect(MitoModelLoad,SIGNAL(pressed()),this,SLOT(handle_MitoModelLoad()));


//        QLabel *tl6=new QLabel("Structure Correction");
//        tl6->setFont(tff);
//        tl6->setAlignment(Qt::AlignCenter);

        QWidget *filler0_1=new QWidget;
        filler0_1->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
        QWidget *filler0_2=new QWidget;
        filler0_2->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);


        QVBoxLayout *tl6_layout=new QVBoxLayout;
//        tl6_layout->addWidget(filler0_1);
//        tl6_layout->addWidget(tl6);
//        tl6_layout->addWidget(StructureModelSave);
//        tl6_layout->addWidget(StructureModelLoad);
        tl6_layout->addWidget(ReSegmentation_structure_button);
//        tl6_layout->addWidget(filler0_2);

//        QLabel *tl6_1=new QLabel("Input\n(Mito)");
//        tl6_1->setFont(tff);
//        tl6_1->setAlignment(Qt::AlignCenter);
        QVBoxLayout *tl6_l_layout=new QVBoxLayout;
//        tl6_l_layout->addWidget(tl6_1);
        tl6_l_layout->addWidget(MitoModelSave);
        tl6_l_layout->addWidget(MitoModelLoad);
        tl6_l_layout->addWidget(ReSegmentation_mitochondria_button);



        QLabel *tl7=new QLabel("Corrector on/off ");
        tl7->setFont(tff);

        QWidget *filler4=new QWidget;
        filler4->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        QWidget *filler5=new QWidget;
        filler5->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        QWidget *filler6=new QWidget;
        filler6->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        QWidget *filler7=new QWidget;
        filler7->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        QWidget *filler8=new QWidget;
        filler8->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        QWidget *filler9=new QWidget;
        filler9->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

        QLabel *vline0 = new QLabel();
        vline0->setFrameStyle(QFrame::VLine | QFrame::Plain);

        QLabel *vline1 = new QLabel();
        vline1->setFrameStyle(QFrame::VLine | QFrame::Plain);

        QLabel *vline2 = new QLabel();
        vline2->setFrameStyle(QFrame::VLine | QFrame::Plain);


        QLabel *vline3 = new QLabel();
        vline3->setFrameStyle(QFrame::VLine | QFrame::Plain);

        QHBoxLayout *tlayout6=new QHBoxLayout;
//        tlayout6->addWidget(tl7);
//        tlayout6->addWidget(correctorEnable_axon);
//        tlayout6->addWidget(correctorEnable_dend);
//        tlayout6->addWidget(filler4);
//        tlayout6->addWidget(vline1);
//        tlayout6->addWidget(filler5);

        QHBoxLayout *tlyaout6_structure=new QHBoxLayout;

        tlyaout6_structure->addWidget(correctorInput_background);
        tlyaout6_structure->addWidget(correctorInput_cellbody);
        tlyaout6_structure->addWidget(correctorInput_axon);
        tlyaout6_structure->addWidget(correctorInput_dend);
        tlyaout6_structure->addSpacing(10);
        tlyaout6_structure->addLayout(tl6_layout);
//        tlayout6->addWidget(correctorInput_tracingP);
//        tlayout6->addWidget(correctorInput_nodeEraser);
//        tlayout6->addWidget(correctorInput_positivePath);
//        tlayout6->addWidget(correctorInput_negativePath);

        StructureCorrection_GroupLayout=new QGroupBox("Structure Correction");
        StructureCorrection_GroupLayout->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);}"));
        StructureCorrection_GroupLayout->setLayout(tlyaout6_structure);
        StructureCorrection_GroupLayout->setContentsMargins(0,0,0,0);

//        tlayout6->addWidget(filler4);
        tlayout6->addWidget(StructureCorrection_GroupLayout);
        tlayout6->addSpacing(10);
        tlayout6->addWidget(vline0);
        tlayout6->addSpacing(10);

        QHBoxLayout *tlayout6_mito=new QHBoxLayout;
//        tlayout6_mito->addLayout(tl6_l_layout);
        tlayout6_mito->addWidget(correctorInput_mito_background);
        tlayout6_mito->addWidget(correctorInput_mito_splitting);
        tlayout6_mito->addWidget(correctorInput_mito_merging);
        tlayout6_mito->addWidget(correctorInput_mito_foreground);

        MitoCorrection_GroupLayout=new QGroupBox("Mito Correction");
        MitoCorrection_GroupLayout->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);}"));
        MitoCorrection_GroupLayout->setLayout(tlayout6_mito);
        MitoCorrection_GroupLayout->setContentsMargins(0,0,0,0);

//        tlayout6->addWidget(MitoCorrection_GroupLayout);
//        tlayout6->addWidget(filler8);
//        tlayout6->addWidget(vline3);
//        tlayout6->addWidget(filler9);





        QLabel *model_path_label=new QLabel("Model path: ");
        model_path_label->setFont(tff);
        model_path=new QLineEdit;
        model_path->setText("model_name");

        QHBoxLayout *model_path_layout=new QHBoxLayout;
        model_path_layout->addWidget(model_path_label);
        model_path_layout->addWidget(model_path);


        QVBoxLayout *label_manage_buttons=new QVBoxLayout;
//        label_manage_buttons->addLayout(uncertainty_thres_layout);
//        label_manage_buttons->addLayout(model_path_layout);
        label_manage_buttons->addWidget(StructureLabelLoad);
        label_manage_buttons->addWidget(correctorUpdate);

//        tlayout6->addLayout(label_manage_buttons);
//        tlayout6->addWidget(filler5);


        tlayout6->addLayout(structureCorrection_part_layout1);
        tlayout6->addWidget(filler5);


        correctorLayout=new QVBoxLayout;
        correctorLayout->addLayout(tlayout6);
//        correctorLayout->addLayout(tlayout4);
//        correctorLayout->addLayout(tlayout5);

        correctorLayoutGroup=new QGroupBox;
        correctorLayoutGroup->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);} "));
        correctorLayoutGroup->setLayout(correctorLayout);

    }


    //subset layout
    {
        subsetBrushSize=new QSlider(Qt::Horizontal);
        subsetBrushSize->setRange(1, 100);
        subsetBrushSize->setSingleStep(1);
        subsetBrushSize->setTickInterval(5);
        subsetBrushSize->setTickPosition(QSlider::TicksAbove);
        subsetBrushSize->setValue(5);
        subsetBrushSize->setStyle(new DarkStyle);

        connect(subsetBrushSize,SIGNAL(valueChanged(int)),this,SLOT(subsetBrushSizeChanged(int)));


        subsetAdd=new QPushButton("Add");
        subsetAdd->setFixedSize(50,30);
        subsetAdd->setStyle(new DarkStyle);
        subsetAdd->setStyleSheet(QString("QPushButton {font: bold;color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(subsetAdd,SIGNAL(pressed()),this,SLOT(subsetAdd_pressed()));

        subsetErase=new QPushButton("Erase");
        subsetErase->setFixedSize(50,30);
        subsetErase->setStyle(new DarkStyle);
        subsetErase->setStyleSheet(QString("QPushButton {font: bold;color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        connect(subsetErase,SIGNAL(pressed()),this,SLOT(subsetErase_pressed()));

        subsetGenerate=new QPushButton("Generate");
        subsetGenerate->setFixedSize(80,30);
        subsetGenerate->setStyle(new DarkStyle);
        subsetGenerate->setStyleSheet(QString("QPushButton {font: bold;}"));
        connect(subsetGenerate,SIGNAL(pressed()),this,SLOT(subsetGenerate_pressed()));

        subsetName=new QLineEdit;
        connect(subsetName,SIGNAL(textChanged(QString)),this,SLOT(subsetName_changed(QString)));

        QLabel *tl=new QLabel("Subset Name: ");
        tl->setFont(tff);

        QWidget *filler4=new QWidget;
        filler4->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        QWidget *filler5=new QWidget;
        filler5->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        QWidget *filler6=new QWidget;
        filler6->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        QWidget *filler7=new QWidget;
        filler7->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

        QLabel *vline1 = new QLabel();
        vline1->setFrameStyle(QFrame::VLine | QFrame::Plain);


        QHBoxLayout *tlayout1=new QHBoxLayout;
        tlayout1->addWidget(filler4);
        tlayout1->addWidget(subsetAdd);
        tlayout1->addWidget(subsetErase);
        tlayout1->addWidget(filler5);
        tlayout1->addWidget(vline1);
        tlayout1->addWidget(filler6);
        tlayout1->addWidget(tl);
        tlayout1->addWidget(subsetName);
        tlayout1->addWidget(subsetGenerate);
        tlayout1->addWidget(filler7);

        QLabel *tl2=new QLabel("Brush size");
        tl2->setFont(tff);
        QHBoxLayout *tlayout2=new QHBoxLayout;
        tlayout2->addWidget(tl2);
        tlayout2->addWidget(subsetBrushSize);

        subsetLayout=new QVBoxLayout;
        subsetLayout->addLayout(tlayout1);
        subsetLayout->addLayout(tlayout2);

        subsetLayoutGroup=new QGroupBox;
        subsetLayoutGroup->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);} "));
        subsetLayoutGroup->setLayout(subsetLayout);



    }

    //low signal control layout
    {
        connectionBackgroundThreshold=new QSlider(Qt::Horizontal);
        connectionBackgroundThreshold->setRange(1, 100);
        connectionBackgroundThreshold->setSingleStep(1);
        connectionBackgroundThreshold->setTickInterval(5);
        connectionBackgroundThreshold->setTickPosition(QSlider::TicksAbove);
        connectionBackgroundThreshold->setValue(5);
        connectionBackgroundThreshold->setStyle(new DarkStyle);
        connect(connectionBackgroundThreshold,SIGNAL(valueChanged(int)),this,SLOT(connectionBackgroundThresholdChanged(int)));


        connectionDistanceThreshold=new QSlider(Qt::Horizontal);
        connectionDistanceThreshold->setRange(1, 100);
        connectionDistanceThreshold->setSingleStep(1);
        connectionDistanceThreshold->setTickInterval(5);
        connectionDistanceThreshold->setTickPosition(QSlider::TicksAbove);
        connectionDistanceThreshold->setValue(20);
        connectionDistanceThreshold->setStyle(new DarkStyle);
        connect(connectionDistanceThreshold,SIGNAL(valueChanged(int)),this,SLOT(connectionDistanceThresholdChanged(int)));


        connectionVisibility=new QSlider(Qt::Horizontal);
        connectionVisibility->setRange(0, 100);
        connectionVisibility->setSingleStep(1);
        connectionVisibility->setTickInterval(5);
        connectionVisibility->setTickPosition(QSlider::TicksAbove);
        connectionVisibility->setValue(100);
        connectionVisibility->setStyle(new DarkStyle);
        connect(connectionVisibility,SIGNAL(valueChanged(int)),this,SLOT(connectionVisibilityChanged(int)));


        connectionFillThreshold=new QSlider(Qt::Horizontal);
        connectionFillThreshold->setRange(1, 100);
        connectionFillThreshold->setSingleStep(1);
        connectionFillThreshold->setTickInterval(5);
        connectionFillThreshold->setTickPosition(QSlider::TicksAbove);
        connectionFillThreshold->setValue(50);
        connectionFillThreshold->setStyle(new DarkStyle);
        connect(connectionFillThreshold,SIGNAL(valueChanged(int)),this,SLOT(connectionFillThresholdChanged(int)));




        QLabel *tl1=new QLabel("Background Threshold");
        tl1->setFont(tff);
        QHBoxLayout *tlayout1=new QHBoxLayout;
        tlayout1->addWidget(tl1);
        tlayout1->addWidget(connectionBackgroundThreshold);


        QLabel *tl2=new QLabel("Distance Threshold");
        tl2->setFont(tff);
        QHBoxLayout *tlayout2=new QHBoxLayout;
        tlayout2->addWidget(tl2);
        tlayout2->addWidget(connectionDistanceThreshold);

        QLabel *tl3=new QLabel("Visibility");
        tl3->setFont(tff);
        QHBoxLayout *tlayout3=new QHBoxLayout;
        tlayout3->addWidget(tl3);
        tlayout3->addWidget(connectionVisibility);


        QLabel *tl4=new QLabel("Restore Threshold");
        tl4->setFont(tff);
        QHBoxLayout *tlayout4=new QHBoxLayout;
        tlayout4->addWidget(tl4);
        tlayout4->addWidget(connectionFillThreshold);


        connectionLayout=new QVBoxLayout;
        connectionLayout->addLayout(tlayout1);
        connectionLayout->addLayout(tlayout2);
        connectionLayout->addLayout(tlayout3);
        connectionLayout->addLayout(tlayout4);

        connectionLayoutGroup=new QGroupBox;
        connectionLayoutGroup->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);} "));
        connectionLayoutGroup->setLayout(connectionLayout);


    }


    {



        QLabel *tl_correction_type=new QLabel("Correction method: ");
        tl_correction_type->setFont(tff);

        correction_type_mito=new QComboBox;
        correction_type_mito->addItem("Global");
        correction_type_mito->addItem("Local");
        correction_type_mito->setStyle(new DarkStyle);
        correction_type_mito->setStyleSheet(QString("QComboBox {font: bold;}"));
        correction_type_mito->setCurrentIndex(1);

        QWidget *filler=new QWidget;
        filler->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        QWidget *filler2=new QWidget;
        filler2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);


        QHBoxLayout *ttl1=new QHBoxLayout;
        ttl1->addWidget(tl_correction_type);
        ttl1->addWidget(correction_type_mito);
        ttl1->addWidget(filler);


        QLabel *vline = new QLabel();
        vline->setFrameStyle(QFrame::VLine | QFrame::Plain);

        QLabel *tl5=new QLabel("Foreground Threshold");
        tl5->setFont(tff);

        QHBoxLayout *ttl2=new QHBoxLayout;
        ttl2->addWidget(tl5);
        ttl2->addWidget(correctorOpacityControl);


        imageMitoLabelOpacityControl_layout2=new QHBoxLayout;
        mito_bright_control_layout2=new QHBoxLayout;

        mitoCorrection_part_layout=new QVBoxLayout;
//        mitoCorrection_part_layout->addLayout(ttl1);
        mitoCorrection_part_layout->addLayout(mito_bright_control_layout2);
        mitoCorrection_part_layout->addLayout(imageMitoLabelOpacityControl_layout2);

//        ttt->addLayout(ttl2);

        mitoCorrectorLayout=new QHBoxLayout;
//        mitoCorrectorLayout->addWidget(filler);
        mitoCorrectorLayout->addWidget(MitoCorrection_GroupLayout);
        mitoCorrectorLayout->addSpacing(10);
        mitoCorrectorLayout->addWidget(vline);
        mitoCorrectorLayout->addSpacing(10);
//        mitoCorrectorLayout->addWidget(tl5);
        mitoCorrectorLayout->addLayout(mitoCorrection_part_layout);
//        mitoCorrectorLayout->addWidget(filler2);


        mitoCorrectorLayoutGroup=new QGroupBox;
        mitoCorrectorLayoutGroup->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);} "));
        mitoCorrectorLayoutGroup->setLayout(mitoCorrectorLayout);

    }

    {

        QLabel *unit=new QLabel("um");

        resolution_edit=new QLineEdit;
        resolution_edit->setText("0.2058");
        connect(resolution_edit,SIGNAL(textChanged(QString)),this,SLOT(resolution_changed(QString)));

        QHBoxLayout *reso_lay=new QHBoxLayout;
        reso_lay->addWidget(resolution_edit);
        reso_lay->addWidget(unit);

        QGroupBox *resolutionGroup=new QGroupBox("Image Resolution");
        resolutionGroup->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);}"));
        resolutionGroup->setFixedHeight(60*WINDOW_SCALE);
        resolutionGroup->setLayout(reso_lay);


        QHBoxLayout *analysis_layout=new QHBoxLayout;
        analysis_layout->addWidget(analysisGroup,1);
        analysis_layout->addSpacing(10);
        analysis_layout->addWidget(coloringGroup,1);
        analysis_layout->addSpacing(10);
        analysis_layout->addWidget(resolutionGroup,1);



        analysisLayoutGroup=new QGroupBox;
        analysisLayoutGroup->setStyleSheet(QString("QGroupBox {font: bold; color : rgb(150,150,150);} "));
        analysisLayoutGroup->setLayout(analysis_layout);

    }


    imageControlPartLayout=new QTabWidget;
    imageControlPartLayout->setStyle(new DarkStyle);
    QFont ttt;
    ttt.setBold(true);
    imageControlPartLayout->setFont(ttt);
    imageControlPartLayout->addTab(tgroupbox_neuron,"Neuron Channel");
    imageControlPartLayout->addTab(tgroupbox_mito,"Mito Channel");
    imageControlPartLayout->addTab(imageLabelOpacityLayoutGroup,"Label Channel");
    imageControlPartLayout->addTab(correctorLayoutGroup,"Structure Correction");
    imageControlPartLayout->addTab(mitoCorrectorLayoutGroup,"Mito Correction");
    imageControlPartLayout->addTab(subsetLayoutGroup,"ROI Selection");
    imageControlPartLayout->addTab(analysisLayoutGroup,"Analysis");
//    imageControlPartLayout->addTab(connectionLayoutGroup,"Low Signal Restore");
//    imageControlPartLayout->addTab(mitoCorrectorLayoutGroup,"Mito Corrector");

    connect(imageControlPartLayout,SIGNAL(currentChanged(int)),this,SLOT(controlTabChanged(int)));

//    QHBoxLayout *tlayout6=new QHBoxLayout;
//    tlayout6->addWidget(imageControlPartLayout);
//    tlayout6->addLayout(tcombolayout);


    dataRendering=new GlWidgetForData(this);

    connect(grayscale_colormap,SIGNAL(released()),dataRendering,SLOT(update()));
    connect(grayscale_colormap,SIGNAL(released()),tfRendering_mito,SLOT(update()));

    recom_layout=new QGridLayout;
    QWidget *recom_layout_widget=new QWidget;
    recom_layout_widget->setLayout(recom_layout);
    recom_layout_widget->setFixedWidth(100);

    QHBoxLayout *dataRendering_layout=new QHBoxLayout;
    dataRendering_layout->addWidget(dataRendering);
    //dataRendering_layout->addWidget(recom_layout_widget);


    imageControlWholeLayout=new QVBoxLayout;
    imageControlWholeLayout->addLayout(dataRendering_layout);
    imageControlWholeLayout->addWidget(imageControlPartLayout);


    qDebug()<<"9";


    QWidget *imageControlWholeLayoutWidget=new QWidget;
    imageControlWholeLayoutWidget->setLayout(imageControlWholeLayout);


    QWidget *tsneWidget=new QWidget;
    tsneWidget->setLayout(sub1ControlPart2);

    qDebug()<<"10";

    dataLayout1=new QSplitter(Qt::Horizontal);
    dataLayout1->addWidget(groupLayout2Widget);
    dataLayout1->addWidget(imageControlWholeLayoutWidget);
    dataLayout1->setChildrenCollapsible(true);

    //dataLayout1->addWidget(pcp_widget);
    //dataLayout1->addWidget(tsneWidget);
    qDebug()<<"11";

//    dataLayout1->setChildrenCollapsible(false);
    QList<int> sizes;
    sizes<<100<<this->width()-300;
    dataLayout1->setSizes(sizes);
    dataLayout1->setHandleWidth(15);
    dataLayout1->setStyleSheet("QSplitter::handle {image: url(Resource/icon_handle.png);}");



//    groupInfoLayout=new QSplitter(Qt::Horizontal);
//    groupInfoLayout->setChildrenCollapsible(false);
//    sizes.clear();
////    sizes<<this->width()/3<<this->width()/3<<this->width()/3;
////    groupInfoLayout->setSizes(sizes);
//    groupInfoLayout->setHandleWidth(15);
//    groupInfoLayout->setStyleSheet("QSplitter::handle {image: url(Resource/icon_handle.png);}");


    QWidget *lineA = new QWidget;
    lineA->setFixedHeight(3);
    lineA->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    lineA->setStyleSheet(QString("background-color: #000000;"));

    QWidget *lineB = new QWidget;
    lineB->setFixedHeight(1);
    lineB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    lineB->setStyleSheet(QString("background-color: #c0c0c0;"));

    QLabel *groupInfoLabel=new QLabel("Morphology Comparison");
    groupInfoLabel->setStyleSheet(QString("QLabel {font: bold; font-size: 12px; color : rgb(0,0,0);}"));


    preGroup=new QSpinBox;
    preGroup->setMaximum(0);
    preGroup->setFixedWidth(50);
    preGroup->setStyle(new DarkStyle);
    preGroup->setStyleSheet(QString("QSpinBox {font: bold;color: rgb(0,0,0);}"));
    connect(preGroup,SIGNAL(valueChanged(int)),this,SLOT(setVariationPlot(int)));
    preGroup->setContentsMargins(0,0,0,0);
    postGroup=new QSpinBox;
    postGroup->setFixedWidth(50);
    postGroup->setMaximum(0);
    postGroup->setStyle(new DarkStyle);
    postGroup->setStyleSheet(QString("QSpinBox {font: bold;color: rgb(0,0,0);}"));
    connect(postGroup,SIGNAL(valueChanged(int)),this,SLOT(setVariationPlot(int)));
    postGroup->setContentsMargins(0,0,0,0);

    comparisonPlot=new variationGraph(this);
    comparisonPlot->setFixedHeight(100);

    QLabel *preGroupLabel=new QLabel("FV1: ");
    preGroupLabel->setFixedWidth(30);
    preGroupLabel->setStyle(new DarkStyle);
    preGroupLabel->setStyleSheet(QString("QLabel {font: bold;color: rgb(29,125,220);}"));
    preGroupLabel->setContentsMargins(0,0,0,0);

    QLabel *postGroupLabel=new QLabel("FV2: ");
    postGroupLabel->setFixedWidth(30);
    postGroupLabel->setStyle(new DarkStyle);
    postGroupLabel->setStyleSheet(QString("QLabel {font: bold;color: rgb(255,142,138);}"));
    postGroupLabel->setContentsMargins(0,0,0,0);

    filer_variationLayout=new QWidget;
    filer_variationLayout->setFixedWidth(200);
    QHBoxLayout *variationLayout=new QHBoxLayout;
    variationLayout->addWidget(filer_variationLayout);
    variationLayout->addWidget(preGroupLabel);
    variationLayout->addWidget(preGroup);
    variationLayout->addWidget(postGroupLabel);
    variationLayout->addWidget(postGroup);
    variationLayout->addWidget(comparisonPlot);
    variationLayout->addSpacing(10);

    snapshotList=new QListWidget;
    QPalette tpal = snapshotList->palette();
    tpal.setColor(QPalette::Base, QColor(0,0,0,30));
    snapshotList->setPalette(tpal);
    snapshotList->setAutoFillBackground(true);

    snapshot=new QPushButton;
    snapshot->setIcon(QIcon("Resource/icon_add_black.png"));
    snapshot->setIconSize(QSize(20, 20));
    snapshot->setFixedSize(QSize(25, 25));
    tpal = snapshot->palette();
    tpal.setColor(QPalette::Button, QColor(0,0,0,50));
    snapshot->setPalette(tpal);
    snapshot->setFlat(true);
    connect(snapshot, SIGNAL(clicked()), this, SLOT(handleSnapshot()));

    snapshot_save=new QPushButton;
    snapshot_save->setIcon(QIcon("Resource/icon_save_black.png"));
    snapshot_save->setIconSize(QSize(20, 20));
    snapshot_save->setFixedSize(QSize(25, 25));
    tpal = snapshot_save->palette();
    tpal.setColor(QPalette::Button, QColor(0,0,0,50));
    snapshot_save->setPalette(tpal);
    snapshot_save->setFlat(true);
    connect(snapshot_save, SIGNAL(clicked()), this, SLOT(handleSnapshot_save()));

    QWidget *filler1=new QWidget;
    filler1->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
    QWidget *filler3=new QWidget;
    filler3->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);

    QVBoxLayout *icon_layout=new QVBoxLayout;
    icon_layout->addWidget(filler1);
    icon_layout->addWidget(snapshot);
    icon_layout->addWidget(snapshot_save);
    icon_layout->addWidget(filler3);

    QHBoxLayout *snapshotLayout=new QHBoxLayout;
    snapshotLayout->addLayout(icon_layout);
    snapshotLayout->addWidget(snapshotList);

    AddSnapshotTitle();

    groupInfoLayout2=new QVBoxLayout;
    groupInfoLayout2->addWidget(lineA);
    groupInfoLayout2->addWidget(groupInfoLabel);
    groupInfoLayout2->addWidget(lineB);
//    groupInfoLayout2->addWidget(groupInfoLayout);
    groupInfoLayout2->addLayout(snapshotLayout);
    groupInfoLayout2->addLayout(variationLayout);

    QWidget *groupInfoLayoutWidget=new QWidget;
    groupInfoLayoutWidget->setLayout(groupInfoLayout2);





    Frame1=new QSplitter(Qt::Vertical);
    Frame1->addWidget(dataLayout1);
    Frame1->addWidget(groupInfoLayoutWidget);
    //Frame1->setChildrenCollapsible(false);
    sizes.clear();
    sizes<<500<<300;
    Frame1->setSizes(sizes);
    Frame1->setHandleWidth(15);
    Frame1->setChildrenCollapsible(true);
    Frame1->setStyleSheet("QSplitter::handle {image: url(Resource/icon_handle2.png);}");


//    miniMap=new GlWidgetForTracingMap(this);


    plotLayout=new QSplitter(Qt::Vertical);
//    plotLayout->addWidget(miniMap);
    plotLayout->addWidget(pcp_widget);
    plotLayout->addWidget(tsneWidget);
    sizes.clear();
    sizes<<this->height()/2<<this->height()/2;
    plotLayout->setSizes(sizes);
    plotLayout->setHandleWidth(15);
    plotLayout->setChildrenCollapsible(true);
    plotLayout->setStyleSheet("QSplitter::handle {image: url(Resource/icon_handle2.png);}");

    QVBoxLayout *plotLayout_top=new QVBoxLayout;
    plotLayout_top->addLayout(tcombolayout);
    plotLayout_top->addWidget(plotLayout);

    QWidget *plotLayout_top_widget=new QWidget;
    plotLayout_top_widget->setLayout(plotLayout_top);

    dataLayout2=new QSplitter(Qt::Horizontal);
    dataLayout2->addWidget(Frame1);
    dataLayout2->addWidget(plotLayout_top_widget);
    dataLayout2->setHandleWidth(15);
    dataLayout2->setStyleSheet("QSplitter::handle {image: url(Resource/icon_handle.png);}");
    dataLayout2->setChildrenCollapsible(true);
    QList<int> sizes2;
    qDebug()<<this->width();
    sizes2<<1300<<300;
    dataLayout2->setSizes(sizes2);




    totalFrame=new QTabWidget;
    totalFrame->addTab(dataLayout2,"Mitochondria Analysis");
//    totalFrame->addTab(tFrame2Widget,"Exploration and Phenotype Classification");
    QFont t;
    t.setBold(true);
    totalFrame->setFont(t);
//    totalFrame->setTabPosition(QTabWidget::West);

    connect(totalFrame,SIGNAL(currentChanged(int)),this,SLOT(UIChanged(int)));

    QHBoxLayout *totalFrameLayout=new QHBoxLayout;
    totalFrameLayout->addWidget(totalFrame);

    setLayout(totalFrameLayout);

    setWindowTitle(tr("Mitochondria Analysis"));

    //this->show();
    QWidget::setFocusPolicy(Qt::ClickFocus);



    connect(this,SIGNAL(mitoUpdate()),this,SLOT(mitoControlReleased()));


    //arrangeRanderPart();



//    setRecommendation_thread();

    qDebug()<<"12";

    qDebug()<<"end window init";

//    startTimer(500);


}
void Window::setRecommendation_thread(){
    recommender=new patchRecommender(this);
    recommender->moveToThread(&workerThread_recommender);

    connect(&workerThread_recommender, &QThread::finished, recommender, &QObject::deleteLater);

    connect(this, SIGNAL(recommenderInit()), recommender, SLOT(init()));
    connect(this, SIGNAL(recommenderRun(int)), recommender, SLOT(run(int)));

    connect(recommender, SIGNAL(addNewPatch(QVector2D,QVector2D)), this, SLOT(addNewPatch(QVector2D,QVector2D)));
    workerThread_recommender.start();

//    emit recommenderRun();


}
void Window::addNewPatch(QVector2D startPos, QVector2D endPos){
    GlWidgetForInset *newPatch=new GlWidgetForInset(this,dataset[curDataIndex],0,0,startPos,endPos,this);
    recoms.push_back(newPatch);

    recom_layout->addWidget(newPatch,recoms.size()-1,0);
    update();


}
void Window::handle_StructureModelSave(){

}

void Window::handle_MitoModelSave(){

}

void Window::handle_StructureModelLoad(){

}

void Window::handle_MitoModelLoad(){

}


void Window::setVariationPlot(int a){
    comparisonPlot->update();
}
void Window::foregroundThreshStarted(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->correctorInputType>=0 && dataset[curDataIndex]->correctorInputType<10)
        dataRendering->thresh_control=1;
    else if(dataset[curDataIndex]->correctorInputType>=10)
        dataRendering->thresh_control=2;

    dataRendering->structureChanged=true;
    dataRendering->update();

}
void Window::foregroundThreshEnded(){
    if(curDataIndex==-1)return;

    if(correction_type_mito->currentIndex()==0){

    }

    dataRendering->thresh_control=0;
    dataRendering->structureChanged=true;
    dataRendering->update();

}
void Window::foregroundThreshChanged(int a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->changeMito_signal();
    dataRendering->mitoLabelChanged=true;

    dataset[curDataIndex]->structure_thresh=float(a)/255;
    dataRendering->structureChanged=true;
    dataRendering->update();
}
void Window::startWaiting(){
    waiting->show();
    movie->start();
}

void Window::endWaiting(){
    waiting->hide();
    movie->stop();
}

void Window::connectionBackgroundThresholdChanged(int a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->connection_background_threshold=float(a)/100;
//    dataset[curDataIndex]->connector->cur_work_update_signal=true;
//    dataset[curDataIndex]->connector->updateWorkList();

}

void Window::connectionFillThresholdChanged(int a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->connection_fill_threshold=float(a)/100;
    dataset[curDataIndex]->connectionStructureUpdate();
    dataRendering->structureChanged=true;
    dataRendering->update();

}


void Window::connectionDistanceThresholdChanged(int a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->connection_distance_threshold=a;
    dataset[curDataIndex]->connector->cur_work_update_signal=true;
//    dataset[curDataIndex]->connector->updateWorkList();
}
void Window::connectionVisibilityChanged(int a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->connection_visibility=float(a)/100;
    dataRendering->connectionChanged=true;
    dataRendering->update();
}


void Window::controlTabChanged(int a){



    if(a==1){
        mito_bright_control_layout1->addWidget(mito_bright_control_label);
        mito_bright_control_layout1->addWidget(imageBrightControl_mito);

        tf_mito_layout->addWidget(grayscale_colormap);
    }
    else if(a==2){
        imageMitoLabelOpacityControl_layout1->addWidget(imageMitoLabelOpacityControl_label);
        imageMitoLabelOpacityControl_layout1->addWidget(imageMitoLabelOpacityControl);

        imageStructureOpacityControl_layout1->addWidget(imageStructureOpacityControl_label);
        imageStructureOpacityControl_layout1->addWidget(imageStructureOpacityControl);

        imageLabelOpacityLayout->addWidget(boundaryOnOff);
        boundaryOnOff->setText("Mitochondria Boundary On/Off");

    }
    else if(a==3){
        imageStructureOpacityControl_layout2->addWidget(imageStructureOpacityControl_label);
        imageStructureOpacityControl_layout2->addWidget(imageStructureOpacityControl);
    }
    else if(a==4){
        imageMitoLabelOpacityControl_layout2->addWidget(imageMitoLabelOpacityControl_label);
        imageMitoLabelOpacityControl_layout2->addWidget(imageMitoLabelOpacityControl);
        imageMitoLabelOpacityControl_layout2->addWidget(boundaryOnOff);
        boundaryOnOff->setText("Boundary");

        mito_bright_control_layout2->addWidget(mito_bright_control_label);
        mito_bright_control_layout2->addWidget(imageBrightControl_mito);
        mito_bright_control_layout2->addWidget(grayscale_colormap);
    }

    if(curDataIndex==-1)return;

    dataRendering->structureChanged=true;
    dataRendering->mitoLabelChanged=true;
    dataRendering->colorTableChanged=true;
    dataRendering->update();

    if(a==5)dataRendering->subsetChanged=true;



//    if(a==5){
//        dataset[curDataIndex]->connector->finish_signal=false;
//        emit dataset[curDataIndex]->connectorRun();
//    }
//    else{
//        dataset[curDataIndex]->connector->finish_signal=true;
//    }

}

void Window::subsetBrushSizeChanged(int a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->brushSize=a;


}
void Window::subsetAdd_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->subsetInputType==0){
        dataset[curDataIndex]->subsetInputType=1;
        subsetAdd->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(30,126,220);}"));
    }
    else if(dataset[curDataIndex]->subsetInputType==2){
        dataset[curDataIndex]->subsetInputType=1;
        subsetAdd->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(30,126,220);}"));
        subsetErase->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else if(dataset[curDataIndex]->subsetInputType==1){
        dataset[curDataIndex]->subsetInputType=0;
        subsetAdd->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    subsetAdd->repaint();
    subsetAdd->update();
    subsetErase->repaint();
    subsetErase->update();

}
void Window::subsetErase_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->subsetInputType==0){
        dataset[curDataIndex]->subsetInputType=2;
        subsetErase->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(30,126,220);}"));
    }
    else if(dataset[curDataIndex]->subsetInputType==1){
        dataset[curDataIndex]->subsetInputType=2;
        subsetErase->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(30,126,220);}"));
        subsetAdd->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else if(dataset[curDataIndex]->subsetInputType==2){
        dataset[curDataIndex]->subsetInputType=0;
        subsetErase->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    subsetAdd->repaint();
    subsetAdd->update();
    subsetErase->repaint();
    subsetErase->update();
}
void Window::subsetGenerate_pressed(){
    if(curDataIndex==-1)return;
    MitoDataset *curData=dataset[curDataIndex];
    if(curData->subsetName==""){
        QMessageBox msgBox;
        msgBox.setText("Please set a subset name first");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();
        return;
    }

    QString path=QString("dataset/")+curData->subsetName+"/";
    if(!QDir(path).exists()){
        QDir().mkdir(path);
    }
    int datasize=curData->ImageW * curData->ImageH;
    {
        unsigned short *temp_image1=new unsigned short[datasize]();

        for(int i=0;i<datasize;i++){
            if(curData->subsetData[i]!=0){
                temp_image1[i]=curData->imageData1[i];
            }
        }

        TinyTIFFFile* tif=TinyTIFFWriter_open((path+"neuron_image.tif").toStdString().c_str(), 16,curData->ImageW,curData->ImageH);
        if (tif) {
            TinyTIFFWriter_writeImage(tif, temp_image1);
            TinyTIFFWriter_close(tif);
        }
        else return;
        delete []temp_image1;
    }
    {
        unsigned short *temp_image2=new unsigned short[datasize]();

        for(int i=0;i<datasize;i++){
            if(curData->subsetData[i]!=0){
                temp_image2[i]=curData->imageData2[i];
            }
        }

        TinyTIFFFile* tif=TinyTIFFWriter_open((path+"mitochondria_image.tif").toStdString().c_str(), 16,curData->ImageW,curData->ImageH);
        if (tif) {
            TinyTIFFWriter_writeImage(tif, temp_image2);
            TinyTIFFWriter_close(tif);
        }
        else return;
        delete []temp_image2;
    }
    {
        unsigned char *temp_structure=new unsigned char[datasize]();
        for(int i=0;i<datasize;i++){
            if(curData->subsetData[i]!=0){
                temp_structure[i]=curData->structureData[i];
            }
        }
        TinyTIFFFile* tif=TinyTIFFWriter_open((path+"structure_label.tif").toStdString().c_str(), 8,curData->ImageW,curData->ImageH);
        if (tif) {
            TinyTIFFWriter_writeImage(tif, temp_structure);
            TinyTIFFWriter_close(tif);
        }
        else return;
        delete []temp_structure;
    }
    {
        unsigned char *tData=new unsigned char[curData->ImageW * curData->ImageH]();
        for(int iy=0;iy<curData->ImageH;iy++){
            for(int ix=0;ix<curData->ImageW;ix++){
                int tP=iy*curData->ImageW + ix;
                if(curData->mitoCorrectionData[tP]!=10 &&curData->mitoCorrectionData[tP]!=11){
                    if(curData->mitoLabelImage_label[tP]==1 || curData->mitoCorrectionData[tP]==12 || curData->mitoCorrectionData[tP]==13){
                        tData[tP]=1;
                    }
                }

            }
        }
        TinyTIFFFile* tif=TinyTIFFWriter_open((path+"mitochondria_label.tif").toStdString().c_str(), 8,curData->ImageW,curData->ImageH);
        if (tif) {
            TinyTIFFWriter_writeImage(tif, tData);
            TinyTIFFWriter_close(tif);
        }
        else return;
        delete []tData;
    }

    int index = curData->group;
    QString dir = path+curData->subsetName+".MitoVis";
    FILE *f=fopen(dir.toStdString().c_str(),"w");
    fclose(f);

    MitoDataset *prev_dataset=dataset[curDataIndex];

    QStringList dir_list;
    dir_list.append(dir);
    bool ok=addDataSub(dir_list,index);
    if(ok==false)return;

    dataset[curDataIndex]->bright_mito=prev_dataset->bright_mito;
    dataset[curDataIndex]->bright_neuron=prev_dataset->bright_neuron;
    dataset[curDataIndex]->contrast_mito=prev_dataset->contrast_mito;
    dataset[curDataIndex]->contrast_neuron=prev_dataset->contrast_neuron;
    dataset[curDataIndex]->midpos_mito=prev_dataset->midpos_mito;
    dataset[curDataIndex]->midpos_neuron=prev_dataset->midpos_neuron;
    dataset[curDataIndex]->mitoLabel_opacity=prev_dataset->mitoLabel_opacity;
    dataset[curDataIndex]->mito_thresh=prev_dataset->mito_thresh;
    dataset[curDataIndex]->structure_opacity=prev_dataset->structure_opacity;

    dataset[curDataIndex]->setColorTable();

    dataRendering->colorTableChanged=true;

    tfRendering_neuron->update();
    tfRendering_mito->update();

    dataRendering->structureChanged=true;
    dataRendering->mitoLabelChanged=true;
    dataRendering->update();


//    if(projection1->currentIndex()==0||projection1->currentIndex()==1){
//        emit projectionRun();
//    }
//    else tsneGraph->makeCoord();

    setUIbasedOnCurDataIndex();
    synchronization();

    mitoControlReleased();


}
void Window::subsetName_changed(QString a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->subsetName=a;
}
void Window::StructureLabelLoad_pressed(){
    if(curDataIndex==-1)return;

    MitoDataset * curData=dataset[curDataIndex];

    QString label_path = QFileDialog::getOpenFileName(this, "Select structure label...", QDir::currentPath()+"/dataset","*.tif");


    //load structure label
    TinyTIFFReaderFile* tiffr=NULL;
    tiffr=TinyTIFFReader_open(label_path.toStdString().c_str());
    if (!tiffr) {
        QMessageBox msgBox;
        msgBox.setText("Fail to load label");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();

        return;
    } else {
        uint32_t width=TinyTIFFReader_getWidth(tiffr);
        uint32_t height=TinyTIFFReader_getHeight(tiffr);
        uint32_t frame=0;
        do {
            bool ok=true;
            if (ok) {
                TinyTIFFReader_getSampleData(tiffr, &curData->structureData[frame*width*height], 0);
                frame++;

            }
        } while (TinyTIFFReader_readNext(tiffr)); // iterate over all frames
    }
    TinyTIFFReader_close(tiffr);
//    labelCleaning();
    mitoControlReleased();

    dataRendering->structureChanged=true;
    dataRendering->update();

    QMessageBox msgBox;
    msgBox.setText("Success to load label");
    msgBox.setStyle(new DarkStyle);
    msgBox.exec();




}


void Window::StructurePatchLoad_pressed(){
    if(curDataIndex==-1)return;

    MitoDataset * curData=dataset[curDataIndex];

    QString label_path = QFileDialog::getExistingDirectory(this, "Select structure patch dir...", QDir::currentPath()+"/dataset");

    if(label_path=="")return;


//    QString iter="01/";
    QString iter="";



    int t;
    float prob_dend,prob_axon;
    int neu;

    FILE *sim_mat=fopen((label_path+"/feature/"+iter+"feature_matrix.txt").toStdString().c_str(),"r");
    FILE *mat_neurite_ind=fopen((label_path+"/feature/"+iter+"neurite_idx.txt").toStdString().c_str(),"r");
    if(mat_neurite_ind!=NULL && sim_mat!=NULL){
        curData->similarity_matrix_neu_ind.clear();
        while(EOF!=fscanf(mat_neurite_ind,"%d",&neu)){
            curData->similarity_matrix_neu_ind.push_back(neu);
            curData->neurite_index_map[neu]=curData->similarity_matrix_neu_ind.size()-1;
        }
        curData->similarity_matrix_width=curData->similarity_matrix_neu_ind.size();

        if(curData->similarity_matrix!=NULL){
            delete []curData->similarity_matrix;
        }
        curData->similarity_matrix=new float[curData->similarity_matrix_width*curData->similarity_matrix_width];
        for(int i=0;i<curData->similarity_matrix_width;i++){
            for(int j=0;j<curData->similarity_matrix_width;j++){
                float v;
                fscanf(sim_mat,"%f",&v);
                curData->similarity_matrix[i*curData->similarity_matrix_width+j]=v;
            }
        }
        fclose(sim_mat);
        fclose(mat_neurite_ind);

    }


    bool is_ok=false;
    float thres_v=uncertainty_thres->text().toFloat(&is_ok);
    if(is_ok==false)thres_v=0.7;

//    FILE *pred_res2=fopen((label_path+"/pred/pred2.txt").toStdString().c_str(),"w"); // neurite_ind patch_ind label prob1 prob2


    FILE *pred_res=fopen((label_path+"/pred/"+iter+"pred.txt").toStdString().c_str(),"r");

    int patch_ind;
    int neu_ind;
    if(pred_res!=NULL){
        while(EOF!=fscanf(pred_res,"%d",&neu_ind)){
            fscanf(pred_res,"%d %d %f %f",&patch_ind,&t,&prob_dend,&prob_axon);
            if(t==0)curData->patch_res[patch_ind]=3; //2: dend, 3: axon
            if(t==1)curData->patch_res[patch_ind]=2; //2: dend, 3: axon
            if(prob_dend<thres_v && prob_dend>1.0-thres_v)curData->patch_res[patch_ind]=4; // 4: uncertain
            curData->patch_probability_dend[patch_ind]=prob_dend;

    //        fprintf(pred_res2,"%d %d %f %f\n",curData->patch_neurite_map[patch_ind],patch_ind,t,prob_dend,prob_axon);

        }
        fclose(pred_res);
        for(int iy=0;iy<curData->ImageH;iy++){
            for(int ix=0;ix<curData->ImageW;ix++){
                int curP=iy*curData->ImageW+ix;
                if(curData->tracingMap[curP].neurite_ind!=-1){
                    if(curData->patch_res.contains(curData->tracingMap[curP].patch->index)){
                        patchCorrection(curP,curData->patch_res[curData->tracingMap[curP].patch->index],curData);
                    }
                }
            }
        }

    }

//    fclose(pred_res2);



    FILE *feature_axon=fopen((label_path+"/feature/"+iter+"feature_axon.txt").toStdString().c_str(),"r");
    if(feature_axon!=NULL){
        while(EOF!=fscanf(feature_axon,"%d",&neu)){
            QVector <float> feature;
            for(int i=0;i<128;i++){
                float f;
                fscanf(feature_axon,"%f",&f);
                feature.push_back(f);
            }
            curData->neurite_feature[neu]=feature;

            if(curData->neurite_res[neu]!=3){
                curData->neurite_changed[neu]=true;
            }
        }
        fclose(feature_axon);

    }

    FILE *feature_axon_user=fopen((label_path+"/feature/"+iter+"feature_axon_user.txt").toStdString().c_str(),"r");
    if(feature_axon_user!=NULL){

        while(EOF!=fscanf(feature_axon_user,"%d",&neu)){

            curData->neurite_res[neu]=3;

            QVector <float> feature;
            for(int i=0;i<128;i++){
                float f;
                fscanf(feature_axon_user,"%f",&f);
                feature.push_back(f);
            }
            curData->neurite_feature[neu]=feature;
            curData->neurite_changed[neu]=false;
        }
        fclose(feature_axon_user);

    }

    FILE *feature_dend=fopen((label_path+"/feature/"+iter+"feature_dend.txt").toStdString().c_str(),"r");
    if(feature_dend!=NULL){

        while(EOF!=fscanf(feature_dend,"%d",&neu)){
            QVector <float> feature;
            for(int i=0;i<128;i++){
                float f;
                fscanf(feature_dend,"%f",&f);
                feature.push_back(f);
            }
            curData->neurite_feature[neu]=feature;
            if(curData->neurite_res[neu]!=2){
                curData->neurite_changed[neu]=true;
            }

        }
        fclose(feature_dend);

    }


    FILE *feature_dend_user=fopen((label_path+"/feature/"+iter+"feature_dend_user.txt").toStdString().c_str(),"r");
    if(feature_dend_user!=NULL){
        while(EOF!=fscanf(feature_dend_user,"%d",&neu)){

            curData->neurite_res[neu]=2;

            QVector <float> feature;
            for(int i=0;i<128;i++){
                float f;
                fscanf(feature_dend_user,"%f",&f);
                feature.push_back(f);
            }
            curData->neurite_feature[neu]=feature;
            curData->neurite_changed[neu]=false;
        }
        fclose(feature_dend_user);

    }

    FILE *feature_mixed=fopen((label_path+"/feature/"+iter+"feature_mixed.txt").toStdString().c_str(),"r");
    if(feature_mixed!=NULL){
        while(EOF!=fscanf(feature_mixed,"%d",&neu)){

            QVector <float> feature;
            for(int i=0;i<128;i++){
                float f;
                fscanf(feature_mixed,"%f",&f);
                feature.push_back(f);
            }
            curData->neurite_feature[neu]=feature;
            curData->neurite_changed[neu]=true;
        }
        fclose(feature_mixed);

    }




    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int curP=iy*curData->ImageW+ix;

            if(curData->tracingMap[curP].temp_ck)continue;

            QQueue<int> res;
            QQueue<int> res2;

            if(curData->tracingMap[curP].type==2 || curData->tracingMap[curP].type==3){
                curData->tracingMap[curP].temp_ck=true;
                res.enqueue(curP);
                res2.enqueue(curP);
            }
            else if(curData->tracingMap[curP].type>0){
                QQueue<int> nodes;
                nodes.push_back(curP);
                curData->tracingMap[curP].temp_ck=true;

                while(!nodes.isEmpty()){
                    int curP=nodes.dequeue();

                    if(curData->tracingMap[curP].type>0){
                        res.enqueue(curP);
                        res2.enqueue(curP);
                        for(int i=0;i<curData->tracingMap[curP].connected_nodes.size();i++){
                            if(curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].temp_ck==false
                                    && curData->tracingMap[curP].type!=2 && curData->tracingMap[curP].type!=3){
                                curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].temp_ck=true;
                                nodes.enqueue(curData->tracingMap[curP].connected_nodes[i]);
                            }
                        }
                    }
                }
            }
            int cnt_dend=0;
            int cnt_axon=0;
            while(!res.isEmpty()){
                int curP=res.dequeue();
                int type=getPatchType(curData->tracingMap[curP].patch,curData);
                if(type==2){
                    cnt_dend++;
                }
                if(type==3){
                    cnt_axon++;
                }
            }

            int type=0;
            if(cnt_dend>cnt_axon){
                type=2;
            }
            if(cnt_dend<cnt_axon){
                type=3;
            }
            curData->neurite_res[curData->tracingMap[curP].neurite_ind]=0;
            curData->neurite_probability_dend[curData->tracingMap[curP].neurite_ind]=0.5;
            if(type!=0){
                float probability_mean=0;
                int cnt=0;
//                curData->neurite_probability_dend[curData->tracingMap[curP].neurite_ind]=float(cnt_dend)/(cnt_dend+cnt_axon);

                curData->neurite_res[curData->tracingMap[curP].neurite_ind]=type;
                while(!res2.isEmpty()){
                    int curP=res2.dequeue();
                    probability_mean+=curData->patch_probability_dend[curData->tracingMap[curP].patch->index];
                    cnt++;
                    patchCorrection(curP,type,curData);
                }
                curData->neurite_probability_dend[curData->tracingMap[curP].neurite_ind]=probability_mean/cnt;

            }
        }
    }

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int curP=iy*curData->ImageW+ix;
            curData->tracingMap[curP].temp_ck=false;
        }
    }





    mitoControlReleased();

    dataRendering->structureChanged=true;
    dataRendering->update();


    QMessageBox msgBox;
    msgBox.setText("Success to load patch");
    msgBox.setStyle(new DarkStyle);
    msgBox.exec();
}



void Window::StructureScribbleLoad_pressed(){

}


void Window::patchCorrection(int curP,int target,MitoDataset *curData){
    if(curData==NULL)return;

    if(target==0){
        return;
    }

    if(target==4)return;

//    curData->tracingMap[curP].corrected_type=curData->correctorInputType;
    patchSet *patch=curData->tracingMap[curP].patch;
    if(patch==NULL)return;
    int tW=patch->boundMax.x()-patch->boundMin.x()+1;
    int tH=patch->boundMax.y()-patch->boundMin.y()+1;

    bool *ck=new bool[tW*tH]();
    int dxylist[4][2]={{-1,0},{1,0},{0,-1},{0,1}};

    QQueue<int> quex,quey;
    quex.enqueue(patch->midx);
    quey.enqueue(patch->midy);
    ck[int((patch->midy-patch->boundMin.y())*tW+patch->midx-patch->boundMin.x())]=true;
    if(curData->correctionData[curP]==0)curData->structureData[curP]=target;
//    curData->correctionData[curP]=target;


    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();
        for(int d=0;d<4;d++){
            int nextx=curx+dxylist[d][0];
            int nexty=cury+dxylist[d][1];
            if(nextx<patch->boundMin.x() ||nextx>patch->boundMax.x()
                    || nexty<patch->boundMin.y() || nexty>patch->boundMax.y()){
                continue;
            }
            if(curData->corrector->isInside(QVector2D(nextx,nexty),patch->patchCorner)==false)continue;

            int tP=nexty*curData->ImageW + nextx;
            int tP2=(nexty-patch->boundMin.y())*tW+nextx-patch->boundMin.x();
            if(ck[tP2]==false){
                ck[tP2]=true;
                if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                    quex.enqueue(nextx);
                    quey.enqueue(nexty);
                    if(curData->correctionData[tP]==0)curData->structureData[tP]=target;
//                    curData->correctionData[tP]=target;
                }
            }
        }
    }
    delete []ck;

}

void Window::correctorUpdate_pressed(){
    if(curDataIndex==-1)return;

    MitoDataset * curData=dataset[curDataIndex];

    curData->changeMito();
    curData->featureSave();

//    qDebug()<<curData->features.length();
    for(int j=0;j<featureNum;j++){
        featureRanges[j].setX(1000);
        featureRanges[j].setY(-1000);
    }

    if(analysis_type->currentIndex()==0){
//        qDebug()<<curData->features.length();
        for(int j=0;j<featureNum;j++){
            featureRanges[j].setX(1000);
            featureRanges[j].setY(-1000);
        }
        for(int i=0;i<curData->features.length();i++){
            for(int j=0;j<featureNum;j++){
//                qDebug()<<curData->features[i][j];

                if(featureRanges[j].x()>curData->features[i][j])featureRanges[j].setX(curData->features[i][j]);
                if(featureRanges[j].y()<curData->features[i][j])featureRanges[j].setY(curData->features[i][j]);
            }
        }
        for(int j=0;j<featureNum;j++){
            if(featureRanges[j].x()==featureRanges[j].y()){
                featureRanges[j].setX(featureRanges[j].x()-1);
                featureRanges[j].setY(featureRanges[j].y()+1);
            }
        }
        if(projection1->currentIndex()==0||projection1->currentIndex()==1){
            emit projectionRun();
        }
        else tsneGraph->makeCoord();
    }
    else{
        for(int j=0;j<featureNum;j++){
            featureRanges[j].setX(1000);
            featureRanges[j].setY(-1000);
        }
        for(int g=0;g<dataset.length();g++){
            for(int i=0;i<dataset[g]->features.length();i++){
                for(int j=0;j<featureNum;j++){
                    if(featureRanges[j].x()>dataset[g]->features[i][j])featureRanges[j].setX(dataset[g]->features[i][j]);
                    if(featureRanges[j].y()<dataset[g]->features[i][j])featureRanges[j].setY(dataset[g]->features[i][j]);
                }
            }
        }
        for(int j=0;j<featureNum;j++){
            if(featureRanges[j].x()==featureRanges[j].y()){
                featureRanges[j].setX(featureRanges[j].x()-1);
                featureRanges[j].setY(featureRanges[j].y()+1);
            }
        }
    }

    curData->checkDataEnable(this);

//    groupIndexOfDataList[curDataIndex]=curDataIndex;

    dataRendering->mitoLabelChanged=true;
    dataRendering->update();

    if(projection1->currentIndex()==0||projection1->currentIndex()==1){
        //emit projectionRun();
    }
    else tsneGraph->makeCoord();

    tsneGraph->update();

    curData->saveStructureLabel(curData->path+"structure_label.tif");

    unsigned char *tData=new unsigned char[curData->ImageW * curData->ImageH]();
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW + ix;
            if(curData->mitoCorrectionData[tP]!=10 &&curData->mitoCorrectionData[tP]!=11){
                if(curData->mitoLabelImage_label[tP]==1 || curData->mitoCorrectionData[tP]==12 || curData->mitoCorrectionData[tP]==13){
                    tData[tP]=1;
                }
            }

        }
    }

    curData->saveMitoLabel(curData->path+"mitochondria_label.tif",tData);
    delete []tData;

    QMessageBox msgBox;
    msgBox.setText("Label updated!");
    msgBox.setStyle(new DarkStyle);
    msgBox.exec();


    //user study
    /*
    {
        QString t_path=curData->path+"history/"+QString::number((curData->time.elapsed()+curData->prev_elapsed_time)/1000);

        FILE *f=fopen((t_path+"_feature.csv").toStdString().c_str(),"w");
        if(f==NULL)return;
        fprintf(f,"No,Structure,Area,Length,Eccentricity,Circularity,Enabled\n");
        for(int i=0;i<curData->features.length();i++){
            fprintf(f,"%d",i);
            for(int j=0;j<curData->features[i].length();j++){
                fprintf(f,",%f",curData->features[i][j]);
            }
            fprintf(f,",%d",curData->enabled[i]);
            fprintf(f,"\n");
        }
        fclose(f);


        curData->saveStructureLabel(t_path+"_structure_label.tif");

        TinyTIFFFile* tif=TinyTIFFWriter_open((t_path+"_mitochondria_object.tif").toStdString().c_str(), 16,curData->ImageW,curData->ImageH);
        if (tif) {
            TinyTIFFWriter_writeImage(tif,curData->mitoLabelImage);
            TinyTIFFWriter_close(tif);
        }
        else {
            QMessageBox msgBox;
            msgBox.setText("Unknown error...");
            msgBox.setStyle(new DarkStyle);
            msgBox.exec();
         }
    }
    */






}
void MitoDataset::connectionStructureUpdate(){
//    qDebug()<<"connecionStructureUpdate()";
    int s=ImageW*ImageH;
    for(int i=0;i<s;i++){
        connectionStructureData[i]=0;
    }

    int dxylist[4][2]={{-1,0},{1,0},{0,-1},{0,1}};

    for(int iy=0;iy<ImageH;iy++){
        for(int ix=0;ix<ImageW;ix++){
            int tP=iy*ImageW+ix;

            for(int j=0;j<tracingMap[tP].weak_paths.size();j++){
                for(int k=0;k<tracingMap[tP].weak_paths[j].size()-1;k++){
                    int curP=tracingMap[tP].weak_paths[j][k];
                    int nextP=tracingMap[tP].weak_paths[j][k+1];
                    if(curP == nextP)continue;
//                    if(structureData[curP]!=0)continue;
                    int x1=curP%ImageW;
                    int y1=curP/ImageW;
                    int x2=nextP%ImageW;
                    int y2=nextP/ImageW;

                    QVector2D dir1(x2-x1,y2-y1);
                    QVector2D dir2(x1-x2,y1-y2);

                    float a=y1-y2;
                    float b=x2-x1;
                    float c=y2*x1-y1*x2;
                    float ab2=sqrt(a*a+b*b);

                    QQueue<int> quex,quey;
                    quex.enqueue(x1);
                    quey.enqueue(y1);
                    connectionStructureData[curP]=10;
                    while(!quex.empty()){
                        int curx=quex.dequeue();
                        int cury=quey.dequeue();

                        for(int d=0;d<4;d++){
                            int newx=curx+dxylist[d][0];
                            int newy=cury+dxylist[d][1];
                            if(newx<0 || newx>=ImageW || newy<0 || newy>=ImageH){
                                continue;
                            }

                            int newP=newy*ImageW + newx;


                            QVector2D newDir1(newx-x1,newy-y1);
                            float angle1=acos((dir1.x()*newDir1.x()+dir1.y()*newDir1.y())/(dir1.length()*newDir1.length()));
                            QVector2D newDir2(newx-x2,newy-y2);
                            float angle2=acos((dir2.x()*newDir2.x()+dir2.y()*newDir2.y())/(dir2.length()*newDir2.length()));

                            int dis=abs(a*newx+b*newy+c)/ab2;

                            if((connectionStructureData[newP]==0|| connectionStructureData[newP]==9)&& structureData[newP]==0 && dis<=1.5 && angle1<1.57 && angle2<1.57){
                                connectionStructureData[newP]=10;
                                quex.enqueue(newx);
                                quey.enqueue(newy);
                            }
                        }
                    }
                }
            }
        }
    }
}
void MitoDataset::saveStructureLabel(QString path){
    TinyTIFFFile* tif=TinyTIFFWriter_open(path.toStdString().c_str(), 8,ImageW,ImageH);
    if (tif) {
        TinyTIFFWriter_writeImage(tif, structureData);
        TinyTIFFWriter_close(tif);
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("Unknown error...");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();
     }
}
void MitoDataset::saveStructureInput(QString path){
    TinyTIFFFile* tif=TinyTIFFWriter_open(path.toStdString().c_str(), 8,ImageW,ImageH);
    if (tif) {
        TinyTIFFWriter_writeImage(tif, correctionData);
        TinyTIFFWriter_close(tif);
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("Unknown error...");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();
     }
}
void MitoDataset::saveMitoLabel(QString path,unsigned char *data){
    TinyTIFFFile* tif=TinyTIFFWriter_open(path.toStdString().c_str(), 8,ImageW,ImageH);
    if (tif) {
        TinyTIFFWriter_writeImage(tif, data);
        TinyTIFFWriter_close(tif);
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("Unknown error...");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();
     }
}
void MitoDataset::saveMitoInput(QString path){
    TinyTIFFFile* tif=TinyTIFFWriter_open(path.toStdString().c_str(), 8,ImageW,ImageH);
    if (tif) {
        TinyTIFFWriter_writeImage(tif, mitoCorrectionData);
        TinyTIFFWriter_close(tif);
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("Unknown error...");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();
     }
}



void Window::correctorInput_background_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->correctorInputType==-1){
        dataset[curDataIndex]->correctorInputType=0;
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,0,0);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==0){
        dataset[curDataIndex]->correctorInputType=-1;
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else{
        dataset[curDataIndex]->correctorInputType=0;
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,0,0);}"));

        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();

    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();

    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();


}
void Window::correctorInput_cellbody_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->correctorInputType==-1){
        dataset[curDataIndex]->correctorInputType=1;
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(120,50,220);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==1){
        dataset[curDataIndex]->correctorInputType=-1;
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else{
        dataset[curDataIndex]->correctorInputType=1;
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(120,50,220);}"));

        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    }
    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();

    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();
    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();


}
void Window::correctorInput_tracingP_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->correctorInputType==-1){
        dataset[curDataIndex]->correctorInputType=4;
        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,220,220);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==4){
        dataset[curDataIndex]->correctorInputType=-1;
        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else{
        dataset[curDataIndex]->correctorInputType=4;
        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,220,220);}"));

        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    }
    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();

    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();
    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();


}
void Window::correctorInput_nodeEraser_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->correctorInputType==-1){
        dataset[curDataIndex]->correctorInputType=7;
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,220,220);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==7){
        dataset[curDataIndex]->correctorInputType=-1;
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else{
        dataset[curDataIndex]->correctorInputType=7;
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,220,220);}"));

        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    }
    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();

    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();
    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();


}

void Window::correctorInput_positivePath_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->correctorInputType==-1){
        dataset[curDataIndex]->correctorInputType=5;
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,220,220);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==5){
        dataset[curDataIndex]->correctorInputType=-1;
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else{
        dataset[curDataIndex]->correctorInputType=5;
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,220,220);}"));

        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    }
    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();

    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();
    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();


}

void Window::correctorInput_negativePath_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->correctorInputType==-1){
        dataset[curDataIndex]->correctorInputType=6;
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,50,100);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==6){
        dataset[curDataIndex]->correctorInputType=-1;
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else{
        dataset[curDataIndex]->correctorInputType=6;
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,50,100);}"));

        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    }
    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();

    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();
    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();


}



void Window::correctorInput_axon_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->correctorInputType==-1){
        dataset[curDataIndex]->correctorInputType=3;
        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(30,126,220);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==3){
        dataset[curDataIndex]->correctorInputType=-1;
        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else{
        dataset[curDataIndex]->correctorInputType=3;
        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(30,126,220);}"));

        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    }

    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();
    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();
    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();


}
void Window::correctorInput_dend_pressed(){
    if(curDataIndex==-1)return;

    if(dataset[curDataIndex]->correctorInputType==-1){
        dataset[curDataIndex]->correctorInputType=2;
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(255,142,138);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==2){
        dataset[curDataIndex]->correctorInputType=-1;
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else{
        dataset[curDataIndex]->correctorInputType=2;
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(255,142,138);}"));

        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    }
    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();
    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();
    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();

}


void Window::correctorInput_mito_background_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->correctorInputType==-1){
        dataset[curDataIndex]->correctorInputType=10;
        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,0,0);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==10){
        dataset[curDataIndex]->correctorInputType=-1;
        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else{
        dataset[curDataIndex]->correctorInputType=10;
        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,0,0);}"));

        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    }
    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();

    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();
    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();


}


void Window::correctorInput_mito_foreground_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->correctorInputType==-1){
        dataset[curDataIndex]->correctorInputType=13;
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(255,0,0);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==13){
        dataset[curDataIndex]->correctorInputType=-1;
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else{
        dataset[curDataIndex]->correctorInputType=13;
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(255,0,0);}"));

        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    }
    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();

    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();
    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();


}



void Window::correctorInput_mito_splitting_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->correctorInputType==-1){
        dataset[curDataIndex]->correctorInputType=11;
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,0,0);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==11){
        dataset[curDataIndex]->correctorInputType=-1;
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else{
        dataset[curDataIndex]->correctorInputType=11;
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,0,0);}"));

        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    }
    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();

    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();
    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();
}



void Window::correctorInput_mito_merging_pressed(){
    if(curDataIndex==-1)return;
    if(dataset[curDataIndex]->correctorInputType==-1){
        dataset[curDataIndex]->correctorInputType=12;
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(255,0,0);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==12){
        dataset[curDataIndex]->correctorInputType=-1;
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else{
        dataset[curDataIndex]->correctorInputType=12;
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(255,0,0);}"));

        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    }
    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();

    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();
    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();
}


void Window::correctorEnable_axon_clicked(bool a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->correctorEnable_axon=a;
    if(a){
        dataset[curDataIndex]->correctorEnable_dend=false;
        correctorEnable_dend->setChecked(false);
        correctorEnable_dend->repaint();
        correctorEnable_dend->update();
    }
    dataset[curDataIndex]->corrector->stopSignal=true;
    emit dataset[curDataIndex]->correctorRun();
}
void Window::correctorEnable_dend_clicked(bool a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->correctorEnable_dend=a;
    if(a){
        dataset[curDataIndex]->correctorEnable_axon=false;
        correctorEnable_axon->setChecked(false);
        correctorEnable_axon->repaint();
        correctorEnable_axon->update();
    }
    dataset[curDataIndex]->corrector->stopSignal=true;
    emit dataset[curDataIndex]->correctorRun();

}
void Window::correctorSensibilitySliderChanged(int a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->correction_threshold=0.5+float(a)*0.5/255;
    dataset[curDataIndex]->corrector->stopSignal=true;
    emit dataset[curDataIndex]->correctorRun();
}
void Window::boundaryConditionChanged(bool a){

    dataRendering->mitoLabelChanged=true;
    dataRendering->update();
}
void Window::handleSnapshot_save(){

#ifdef VIS22
    VIS22_userstudy_save((dataset[curDataIndex]->finetune_iter-1)*2+1);
#endif

    QString dir = QFileDialog::getSaveFileName(this, "Save analysis results...", QDir::currentPath(),"*.csv");
    if (dir == "")return;

    FILE *f=fopen(dir.toStdString().c_str(),"w");
    fprintf(f,"Annotation,Group,Image,Count,Density(%%)");
    for(int i=1;i<featureNum;i++){
        fprintf(f,",Avg. %s",featureNames[i].toStdString().c_str());
    }
    fprintf(f,",Dendrite Area,Axon Area,Mitocondria Area on Dendrite,Mitocondria Area on Axon");

    fprintf(f,"\n");

    for(int i=0;i<snapshot_annotation.length();i++){
        fprintf(f,"%s",snapshot_annotation[i]->text().toStdString().c_str());
        fprintf(f,",%s",snapshot_group[i].toStdString().c_str());
        fprintf(f,",%s",snapshot_image[i].toStdString().c_str());
        fprintf(f,",%d",snapshot_count[i]);
        fprintf(f,",%f",snapshot_density[i]);
//        fprintf(f,",%f,%f",(3-snapshot_features[i][0])*100,(snapshot_features[i][0]-2)*100);
        for(int j=1;j<featureNum;j++){
            fprintf(f,",%f",snapshot_features[i][j-1]);
        }
        fprintf(f,",%f",snapshot_dendriteArea[i]);
        fprintf(f,",%f",snapshot_axonArea[i]);
        fprintf(f,",%f",snapshot_mitoAreaOnDendrite[i]);
        fprintf(f,",%f",snapshot_mitoAreaOnAxon[i]);

        fprintf(f,"\n");
    }
    fclose(f);


}
void Window::ImageControlSliderChanged(int a){
    if(curDataIndex==-1)return;
    MitoDataset *curData=dataset[curDataIndex];
    curData->bright_neuron=imageBrightControl_neuron->value()/255.0;
    curData->contrast_neuron=imageContrastControl_neuron->value()/255.0;
    curData->midpos_neuron=imageContrastPosControl_neuron->value()/255.0;

    curData->bright_mito=imageBrightControl_mito->value()/255.0;
    curData->contrast_mito=imageContrastControl_mito->value()/255.0;
    curData->midpos_mito=imageContrastPosControl_mito->value()/255.0;

    curData->setColorTable();

    dataRendering->colorTableChanged=true;
    dataRendering->update();

    tfRendering_neuron->update();
    tfRendering_mito->update();



}


void Window::plotAxis1Changed(int a){
    if(projection1->currentIndex()==0
            || projection1->currentIndex()==1){
        projection2->setCurrentIndex(projection1->currentIndex());
        emit projectionRun();
        projection2->setEnabled(false);
    }
    else{
        if(projection2->isEnabled()==false){
            projection2->setEnabled(true);
            projection2->setCurrentIndex(2);
        }
        tsneGraph->makeCoord();
    }
}
void Window::plotAxis2Changed(int a){
    if(projection2->currentIndex()==0
            || projection2->currentIndex()==1){
        projection1->setCurrentIndex(projection2->currentIndex());
        //emit projectionRun();
        projection2->setEnabled(false);
    }
    else{
        tsneGraph->makeCoord();
    }
}

void Window::imageControlSliderChanged(int a){
}
void Window::StructureControlSliderChanged(int a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->structure_opacity=float(a)/255.0;
    dataRendering->structureChanged=true;
    dataRendering->update();
}
void Window::MitoControlSliderChanged(int a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->mitoLabel_opacity=float(a)/255.0;
    dataRendering->mitoLabelChanged=true;
    dataRendering->update();
}
void Window::ThresControlSliderChanged(int a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->mito_thresh=float(a)/512.0+0.25;

    dataset[curDataIndex]->changeMito();
    dataset[curDataIndex]->featureSave();

//    qDebug()<<dataset[curDataIndex]->features.length();
    for(int j=0;j<featureNum;j++){
        featureRanges[j].setX(1000);
        featureRanges[j].setY(-1000);
    }

    if(analysis_type->currentIndex()==0){
//        qDebug()<<dataset[curDataIndex]->features.length();
        for(int j=0;j<featureNum;j++){
            featureRanges[j].setX(1000);
            featureRanges[j].setY(-1000);
        }
        for(int i=0;i<dataset[curDataIndex]->features.length();i++){
            for(int j=0;j<featureNum;j++){
//                qDebug()<<dataset[curDataIndex]->features[i][j];

                if(featureRanges[j].x()>dataset[curDataIndex]->features[i][j])featureRanges[j].setX(dataset[curDataIndex]->features[i][j]);
                if(featureRanges[j].y()<dataset[curDataIndex]->features[i][j])featureRanges[j].setY(dataset[curDataIndex]->features[i][j]);
            }
        }
        for(int j=0;j<featureNum;j++){
            if(featureRanges[j].x()==featureRanges[j].y()){
                featureRanges[j].setX(featureRanges[j].x()-1);
                featureRanges[j].setY(featureRanges[j].y()+1);
            }
        }
        if(projection1->currentIndex()==0||projection1->currentIndex()==1){
            emit projectionRun();
        }
        else tsneGraph->makeCoord();
    }
    else{
        for(int j=0;j<featureNum;j++){
            featureRanges[j].setX(1000);
            featureRanges[j].setY(-1000);
        }
        for(int g=0;g<dataset.length();g++){
            for(int i=0;i<dataset[g]->features.length();i++){
                for(int j=0;j<featureNum;j++){
                    if(featureRanges[j].x()>dataset[g]->features[i][j])featureRanges[j].setX(dataset[g]->features[i][j]);
                    if(featureRanges[j].y()<dataset[g]->features[i][j])featureRanges[j].setY(dataset[g]->features[i][j]);
                }
            }
        }
        for(int j=0;j<featureNum;j++){
            if(featureRanges[j].x()==featureRanges[j].y()){
                featureRanges[j].setX(featureRanges[j].x()-1);
                featureRanges[j].setY(featureRanges[j].y()+1);
            }
        }
    }

    dataset[curDataIndex]->checkDataEnable(this);

//    groupIndexOfDataList[curDataIndex]=curDataIndex;


    dataRendering->mitoLabelChanged=true;
    dataRendering->update();


    if(projection1->currentIndex()==0||projection1->currentIndex()==1){
        //emit projectionRun();
    }
    else tsneGraph->makeCoord();

    tsneGraph->update();


}
void Window::mitoControlReleased(){
    if(curDataIndex==-1)return;

    qDebug()<<"1";

    MitoDataset *curData=dataset[curDataIndex];
    curData->maxStructureLabel=0;
    for(int i=0;i<curData->ImageW*curData->ImageH;i++){
        if(curData->structureData[i]!=0){

            if(curData->maxStructureLabel<curData->structureData[i]){
                curData->maxStructureLabel=curData->structureData[i];
            }
        }
    }
    curData->structureArea.clear();
    float pixelArea=curData->resolution*curData->resolution;
    for(int i=0;i<=curData->maxStructureLabel;i++){
        float areaa=0;
        for(int j=0;j<curData->ImageW*curData->ImageH;j++){
            if(curData->structureData[j]==i)areaa+=pixelArea;
        }
        curData->structureArea.push_back(areaa);
    }


    dataset[curDataIndex]->changeMito();
    dataset[curDataIndex]->featureSave();

    qDebug()<<"2";


    dataRendering->insetClearTrigger=true;

//    qDebug()<<dataset[curDataIndex]->features.length();
    for(int j=0;j<featureNum;j++){
        featureRanges[j].setX(1000);
        featureRanges[j].setY(-1000);
    }
    if(analysis_type->currentIndex()==0){
//        qDebug()<<dataset[curDataIndex]->features.length();
        for(int j=0;j<featureNum;j++){
            featureRanges[j].setX(1000);
            featureRanges[j].setY(-1000);
        }
        for(int i=0;i<dataset[curDataIndex]->features.length();i++){
            for(int j=0;j<featureNum;j++){
//                qDebug()<<dataset[curDataIndex]->features[i][j];

                if(featureRanges[j].x()>dataset[curDataIndex]->features[i][j])featureRanges[j].setX(dataset[curDataIndex]->features[i][j]);
                if(featureRanges[j].y()<dataset[curDataIndex]->features[i][j])featureRanges[j].setY(dataset[curDataIndex]->features[i][j]);
            }
        }
        for(int j=0;j<featureNum;j++){
            if(featureRanges[j].x()==featureRanges[j].y()){
                featureRanges[j].setX(featureRanges[j].x()-1);
                featureRanges[j].setY(featureRanges[j].y()+1);
            }
        }
        qDebug()<<"3";


        if(projection1->currentIndex()==0||projection1->currentIndex()==1){
            emit projectionRun();
        }
        else tsneGraph->makeCoord();

        qDebug()<<"4";

    }
    else{
        for(int j=0;j<featureNum;j++){
            featureRanges[j].setX(1000);
            featureRanges[j].setY(-1000);
        }
        for(int g=0;g<dataset.length();g++){
            for(int i=0;i<dataset[g]->features.length();i++){
                for(int j=0;j<featureNum;j++){
                    if(featureRanges[j].x()>dataset[g]->features[i][j])featureRanges[j].setX(dataset[g]->features[i][j]);
                    if(featureRanges[j].y()<dataset[g]->features[i][j])featureRanges[j].setY(dataset[g]->features[i][j]);
                }
            }
        }
        for(int j=0;j<featureNum;j++){
            if(featureRanges[j].x()==featureRanges[j].y()){
                featureRanges[j].setX(featureRanges[j].x()-1);
                featureRanges[j].setY(featureRanges[j].y()+1);
            }
        }
    }


    dataset[curDataIndex]->checkDataEnable(this);

    dataRendering->mitoLabelChanged=true;
    dataRendering->update();


    if(projection1->currentIndex()==0||projection1->currentIndex()==1){
        //emit projectionRun();
    }
    else tsneGraph->makeCoord();

    qDebug()<<"5";


    tsneGraph->update();

    curData->changeMito_signal();
    qDebug()<<"6";


}
void Window::ThresControlSliderReleased(){
    if(curDataIndex==-1)return;

    if(projection1->currentIndex()==0||projection1->currentIndex()==1){
        emit projectionRun();
        tsneGraph->update();
    }

}




void Window::AddSnapshotTitle(){
    QWidget *snapshotWidget=new QWidget;

    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(50,40));//groups[groups.size()-1]->widget->sizeHint());
    snapshotList->addItem(item);
    snapshotList->setItemWidget(item, snapshotWidget);

    QListWidgetItem *item2 = new QListWidgetItem();
    item2->setSizeHint(QSize(50,2));//groups[groups.size()-1]->widget->sizeHint());
    QLabel *hline = new QLabel(snapshotList);
    hline->setFrameStyle(QFrame::HLine | QFrame::Plain);
    hline->setFixedHeight(2);
    snapshotList->addItem(item2);
    snapshotList->setItemWidget(item2, hline);


    QHBoxLayout *l1=new QHBoxLayout;
    l1->setContentsMargins(0,0,0,0);

    QString titleF=QString("QLabel {font: bold; font-size: 12px; color : rgb(0,200,200);}");

    title_no=new QLabel("No");
    title_no->setFixedWidth(30);
    title_no->setStyleSheet(titleF);
    title_no->setAlignment(Qt::AlignCenter);
    l1->addWidget(title_no);


    title_anno=new QLabel("Annotation");
    title_anno->setFixedWidth(200);
    title_anno->setStyleSheet(titleF);
    title_anno->setAlignment(Qt::AlignCenter);

    l1->addWidget(title_anno);
//    l1->addSpacing(30);

    int col=0;
    title_group=new QLabel("Group");
    title_group->setStyleSheet(titleF);
    l1->addWidget(title_group,4);

    title_image=new QLabel("Image");
    title_image->setStyleSheet(titleF);
    l1->addWidget(title_image,6);

    QLabel *w5=new QLabel("Count");
    w5->setAlignment(Qt::AlignCenter);
    w5->setStyleSheet(titleF);
    l1->addWidget(w5,3);

    QLabel *w9=new QLabel("Density\n(%)");
    w9->setAlignment(Qt::AlignCenter);
    w9->setStyleSheet(titleF);
    l1->addWidget(w9,3);

    for(int i=1;i<featureNum;i++){
        QLabel *w7=new QLabel(featureNames[i]);
        if(i==0){
            w7->setText("Structure\n(Dend:Axon)");
        }
        if(i==1){
            w7->setText(QString("Avg. Area<br>(")+QChar(181)+QString("m<sup>2</sup>)"));
        }
        if(i==2){
            w7->setText(QString("Avg. Length\n(")+QChar(181)+"m)");
        }
        if(i==3){
            w7->setText(QString("Avg.\nEccentricity"));
        }
        if(i==4){
            w7->setText(QString("Avg.\nCircularity"));
        }
        w7->setStyleSheet(titleF);
        w7->setAlignment(Qt::AlignCenter);
        l1->addWidget(w7,4);
    }
    snapshotWidget->setLayout(l1);
    snapshotList->update();


}
void Window::handleSnapshot(){
    if(curDataIndex==-1)return;
    qDebug()<<"snapshot start";
    QWidget *snapshotWidget=new QWidget;

    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(50,30));//groups[groups.size()-1]->widget->sizeHint());
    snapshotList->addItem(item);
    snapshotList->setItemWidget(item, snapshotWidget);

    QListWidgetItem *item2 = new QListWidgetItem();
    item2->setSizeHint(QSize(50,2));//groups[groups.size()-1]->widget->sizeHint());
    QLabel *hline = new QLabel(snapshotList);
    hline->setFrameStyle(QFrame::HLine | QFrame::Plain);
    snapshotList->addItem(item2);
    snapshotList->setItemWidget(item2, hline);

    QHBoxLayout *l1=new QHBoxLayout;
    l1->setContentsMargins(0,0,0,0);

    int fh=15;
    QString valueF=QString("QLabel {font: bold; font-size: 12px; color : rgb(50,50,50);}");


    QLabel *no=new QLabel(QString::number(snapshot_features.size()));
    no->setStyleSheet(valueF);
    no->setFixedHeight(fh);
    no->setFixedWidth(30);
    l1->addWidget(no);


    QLineEdit *anno=new QLineEdit;
    anno->setFixedWidth(200);
    snapshot_annotation.push_back(anno);

    l1->addWidget(anno);
//    l1->addSpacing(30);



    int col=0;
    QLabel *w2;
    if(analysis_type->currentIndex()==0){
        w2=new QLabel(groups[groupIndexOfDataList[dataset[curDataIndex]->dataIndex]]->name->text());
        snapshot_group.push_back(groups[groupIndexOfDataList[dataset[curDataIndex]->dataIndex]]->name->text());
    }
    else{
        w2=new QLabel("All");
        snapshot_group.push_back("All");

    }
    w2->setStyleSheet(valueF);
    w2->setFixedHeight(fh);
    l1->addWidget(w2,4);
    //qDebug()<<"snapshot 2";

    QLabel *w4;
    if(analysis_type->currentIndex()==0){
        w4=new QLabel(dataset[curDataIndex]->dir);
        snapshot_image.push_back(dataset[curDataIndex]->dir);
    }
    else{
        w4=new QLabel("All");
        snapshot_image.push_back("All");
    }

    w4->setStyleSheet(valueF);
    w4->setFixedHeight(fh);
    l1->addWidget(w4,6);
    //qDebug()<<"snapshot 3";

    QLabel *w6;
    if(analysis_type->currentIndex()==0){
        w6=new QLabel(QString::number(enabledNum));
        snapshot_count.push_back(enabledNum);
    }
    else{
        int total=0;
        for(int i=0;i<dataset.length();i++){
            if(datagroupOnOff[dataset[i]->group]->isChecked())
                total+=dataset[i]->enabledNum;
        }
        w6=new QLabel(QString::number(total));
        snapshot_count.push_back(total);

    }
    w6->setStyleSheet(valueF);
    w6->setFixedHeight(fh);
    w6->setAlignment(Qt::AlignCenter);
    l1->addWidget(w6,3);
    //qDebug()<<"snapshot 4";

    bool checkStructure[5];
    for(int i=0;i<5;i++)checkStructure[i]=false;
    float structureArea=0;
    float mitoArea=0;

    float DendriteArea=0;
    float mitoAreaOnDendrite=0;
    float AxonArea=0;
    float mitoAreaOnAxon=0;

    if(analysis_type->currentIndex()==0){

        MitoDataset *curData=dataset[curDataIndex];
        for(int i=0;i<curData->features.length();i++){
            if(curData->enabled[i]){
                mitoArea+=curData->features[i][1];
                checkStructure[int(curData->features[i][0])]=true;
                if(int(curData->features[i][0])==2){
                    mitoAreaOnDendrite+=curData->features[i][1];
                }
                if(int(curData->features[i][0])==3){
                    mitoAreaOnAxon+=curData->features[i][1];
                }
            }
        }
        for(int i=0;i<5;i++){
            if(checkStructure[i])structureArea+=curData->structureArea[i];
        }
        DendriteArea+=curData->structureArea[2];
        AxonArea+=curData->structureArea[3];
    }
    else{
        for(int i=0;i<dataset.length();i++){
            if(datagroupOnOff[dataset[i]->group]->isChecked()==false)
                continue;

            MitoDataset *curData=dataset[i];
            for(int i=0;i<curData->features.length();i++){
                if(curData->globalEnabled[i]){
                    mitoArea+=curData->features[i][1];
                    checkStructure[int(curData->features[i][0])]=true;
                    if(int(curData->features[i][0])==2){
                        mitoAreaOnDendrite+=curData->features[i][1];
                    }
                    if(int(curData->features[i][0])==3){
                        mitoAreaOnAxon+=curData->features[i][1];
                    }
                }
            }
            for(int i=0;i<5;i++){
                if(checkStructure[i])structureArea+=curData->structureArea[i];
            }
            DendriteArea+=curData->structureArea[2];
            AxonArea+=curData->structureArea[3];
        }
    }
   // qDebug()<<"snapshot 5";


    QLabel *w10=new QLabel(QString::number(mitoArea/structureArea * 100,'g',4));
    w10->setStyleSheet(valueF);
    w10->setFixedHeight(fh);
    w10->setAlignment(Qt::AlignCenter);
    l1->addWidget(w10,3);
    snapshot_density.push_back(mitoArea/structureArea * 100);

    snapshot_dendriteArea.push_back(DendriteArea);
    snapshot_axonArea.push_back(AxonArea);
    snapshot_mitoAreaOnDendrite.push_back(mitoAreaOnDendrite);
    snapshot_mitoAreaOnAxon.push_back(mitoAreaOnAxon);


    if(analysis_type->currentIndex()==0){
        QVector <float> values;
        for(int i=1;i<featureNum;i++){
            QLabel *w8=new QLabel(QString::number(avgValues[i],'g',4));
            if(i==0){
                w8->setText(QString::number((3-avgValues[i])*100,'g',3)+":"+QString::number((avgValues[i]-2)*100,'g',3));
            }
            w8->setStyleSheet(valueF);
            w8->setFixedHeight(fh);
            w8->setAlignment(Qt::AlignCenter);
            l1->addWidget(w8,4);
            values.push_back(avgValues[i]);
        }
        snapshot_features.push_back(values);
    }
    else{
        QVector <float> values;

        for(int i=1;i<featureNum;i++){
            float avgV=0;
            int cnt=0;
            for(int g=0;g<dataset.length();g++){
                if(datagroupOnOff[dataset[g]->group]->isChecked()==false)
                    continue;

                MitoDataset *curData=dataset[g];
                for(int j=0;j<curData->features.length();j++){
                    if(curData->globalEnabled[j]){
                        cnt++;
                        avgV+=curData->features[j][i];
                    }
                }
            }
            if(cnt!=0){
                avgV/=cnt;
            }

            QLabel *w8=new QLabel(QString::number(avgV,'g',4));
            if(i==0){
                w8->setText(QString::number((3-avgV)*100,'g',3)+":"+QString::number((avgV-2)*100,'g',3));
            }
            w8->setStyleSheet(valueF);
            w8->setFixedHeight(fh);
            w8->setAlignment(Qt::AlignCenter);
            l1->addWidget(w8,4);
            values.push_back(avgV);
        }
        snapshot_features.push_back(values);

    }
    //qDebug()<<"snapshot 6";

    snapshotWidget->setLayout(l1);
    snapshotList->update();

    preGroup->setMaximum(snapshot_features.size()-1);
    postGroup->setMaximum(snapshot_features.size()-1);


    //user study
    /*
    {
    MitoDataset *curData=dataset[curDataIndex];
    QString t_path=curData->path+"history/"+QString::number((curData->time.elapsed()+curData->prev_elapsed_time)/1000);



    FILE *ff=fopen((t_path+"_snapshot.csv").toStdString().c_str(),"w");
    fprintf(ff,"Annotation,Group,Image,Count,Density(%%)");
    for(int i=1;i<featureNum;i++){
        fprintf(ff,",Avg. %s",featureNames[i].toStdString().c_str());
    }
    fprintf(ff,",Dendrite Area,Axon Area,Mitocondria Area on Dendrite,Mitocondria Area on Axon");

    fprintf(ff,"\n");

    for(int i=0;i<snapshot_annotation.length();i++){
        fprintf(ff,"%s",snapshot_annotation[i]->text().toStdString().c_str());
        fprintf(ff,",%s",snapshot_group[i].toStdString().c_str());
        fprintf(ff,",%s",snapshot_image[i].toStdString().c_str());
        fprintf(ff,",%d",snapshot_count[i]);
        fprintf(ff,",%f",snapshot_density[i]);
//        fprintf(f,",%f,%f",(3-snapshot_features[i][0])*100,(snapshot_features[i][0]-2)*100);
        for(int j=1;j<featureNum;j++){
            fprintf(ff,",%f",snapshot_features[i][j-1]);
        }
        fprintf(ff,",%f",snapshot_dendriteArea[i]);
        fprintf(ff,",%f",snapshot_axonArea[i]);
        fprintf(ff,",%f",snapshot_mitoAreaOnDendrite[i]);
        fprintf(ff,",%f",snapshot_mitoAreaOnAxon[i]);

        fprintf(ff,"\n");
    }
    fclose(ff);
    }
    */




}
void Window::renderVarGraph(int a){
}
void Window::UIChanged(int a){

}
void Window::generateDataset(){

}
void Window::checkDatasetGeneration(){

}
void Window::handleGroupAdd(){
    int index=groups.size();
    DataGroup *newGroup=new DataGroup(index);
    newGroup->name->installEventFilter(this);
    newGroup->name->setMouseTracking(true);
    connect(newGroup->name,SIGNAL(textChanged(QString)),this,SLOT(groupNameChanged(QString)));
    connect(newGroup->add, SIGNAL(pressed()), this,SLOT( addData()));
    connect(newGroup->remove, SIGNAL(pressed()), this,SLOT( removeGroup()));
    connect(newGroup->colorChange, SIGNAL(pressed()), this, SLOT(colorChangeGroup()));

    connect(newGroup->name, SIGNAL(textEdited(QString)), this, SLOT(focusGroup(QString)));
    connect(newGroup->add, SIGNAL(pressed()), this,SLOT( focusGroup()));
    connect(newGroup->colorChange, SIGNAL(pressed()), this, SLOT(focusGroup()));


    connect(newGroup->dataList,SIGNAL(itemPressed(QListWidgetItem*)),this,SLOT(currentDataChange(QListWidgetItem*)));

    QListWidgetItem *item = new QListWidgetItem();
    groups.push_back(newGroup);
    item->setSizeHint(QSize(50,200));//groups[groups.size()-1]->widget->sizeHint());
    groupList->addItem(item);
    groupList->setItemWidget(item, groups[groups.size() - 1]->widget);


    datagroupOnOff[index]=new QCheckBox(newGroup->name->text());
    char ct[100];
    sprintf(ct,"QCheckBox {font: bold; color : rgb(%d,%d,%d);}",newGroup->color.red(),newGroup->color.green(),newGroup->color.blue());
    datagroupOnOff[index]->setStyleSheet(QString(ct));
    datagroupOnOff[index]->setChecked(true);
    connect(datagroupOnOff[index],SIGNAL(clicked(bool)),this,SLOT(runCheckSpineEnable(bool)));

    datagroupLayout->addWidget(datagroupOnOff[index],index/3,index%3);

    //focusItem(item);

}
void Window::groupNameChanged(QString a){
    QObject *senderObj = sender(); // This will give Sender object
    int index = senderObj->objectName().toInt();
    datagroupOnOff[index]->setText(a);
}
void Window::colorChangeGroup(){
    QObject *senderObj = sender(); // This will give Sender object
    int index = senderObj->objectName().toInt();
    QColorDialog color_test;
    QColor t=color_test.getColor(groups[index]->color);
    if(t.isValid()){
        groups[index]->color = t;
        groups[index]->colorChange->setStyleSheet(QString("background-color: %1;foreground-color: %1; border-style: none;").arg(t.name()));
        datagroupOnOff[index]->setStyleSheet(QString("QCheckBox {font: bold; color: %1;}").arg(t.name()));

        synchronization();
    }


}
void Window::currentDataChange(QListWidgetItem* a){

    if(prevDataIndex==a->whatsThis().toInt())return;

//    dataset[prevDataIndex]->connector->finish_signal=true;


    //user study
    /*{
    if(prevDataIndex!=-1){
        dataset[prevDataIndex]->prev_elapsed_time+=dataset[prevDataIndex]->time.elapsed();
    }
    }*/

    if(processedIndex==-1)processedItem=a;
    curDataIndex=a->whatsThis().toInt();
    if(prevDataIndex!=-1){
        hideAllRadars(prevDataIndex);
    }
    prevDataIndex=curDataIndex;
    curDataName=a->text();

    //user study
//    dataset[curDataIndex]->time.restart();


//    if(imageControlPartLayout->currentIndex()==5){
//        dataset[curDataIndex]->connector->finish_signal=false;
//        emit dataset[curDataIndex]->connectorRun();
//    }

    if(analysis_type->currentIndex()==0){
//        qDebug()<<dataset[curDataIndex]->features.length();
        for(int j=0;j<featureNum;j++){
            featureRanges[j].setX(1000);
            featureRanges[j].setY(-1000);
        }
        for(int i=0;i<dataset[curDataIndex]->features.length();i++){
            for(int j=0;j<featureNum;j++){
//                qDebug()<<dataset[curDataIndex]->features[i][j];

                if(featureRanges[j].x()>dataset[curDataIndex]->features[i][j])featureRanges[j].setX(dataset[curDataIndex]->features[i][j]);
                if(featureRanges[j].y()<dataset[curDataIndex]->features[i][j])featureRanges[j].setY(dataset[curDataIndex]->features[i][j]);
            }
        }
        for(int j=0;j<featureNum;j++){
            if(featureRanges[j].x()==featureRanges[j].y()){
                featureRanges[j].setX(featureRanges[j].x()-1);
                featureRanges[j].setY(featureRanges[j].y()+1);
            }
        }
        if(projection1->currentIndex()==0||projection1->currentIndex()==1){
            emit projectionRun();
        }
        else tsneGraph->makeCoord();

        dataset[curDataIndex]->checkDataEnable(this);
    }
    else{
        for(int j=0;j<featureNum;j++){
            featureRanges[j].setX(1000);
            featureRanges[j].setY(-1000);
        }
        for(int g=0;g<dataset.length();g++){
            if(datagroupOnOff[dataset[g]->group]->isChecked()==false)continue;

            for(int i=0;i<dataset[g]->features.length();i++){
                for(int j=0;j<featureNum;j++){
                    if(featureRanges[j].x()>dataset[g]->features[i][j])featureRanges[j].setX(dataset[g]->features[i][j]);
                    if(featureRanges[j].y()<dataset[g]->features[i][j])featureRanges[j].setY(dataset[g]->features[i][j]);
                }
            }
        }
        for(int j=0;j<featureNum;j++){
            if(featureRanges[j].x()==featureRanges[j].y()){
                featureRanges[j].setX(featureRanges[j].x()-1);
                featureRanges[j].setY(featureRanges[j].y()+1);
            }
        }
        if(projection1->currentIndex()==0||projection1->currentIndex()==1){
            emit projectionRun();
        }
        else tsneGraph->makeCoord();
        for(int g=0;g<dataset.length();g++){
            if(datagroupOnOff[dataset[g]->group]->isChecked()==false)continue;
            dataset[g]->checkDataEnable(this);
        }

    }


    setUIbasedOnCurDataIndex();
    synchronization();

}


bool Window::runChannelSelectDialog(QString dir){

    QDialog *widget=new QDialog;
    QVBoxLayout *totalLayout=new QVBoxLayout;
    QHBoxLayout *imageLayout=new QHBoxLayout;
    QHBoxLayout *labelLayout=new QHBoxLayout;
    QHBoxLayout *selectLayout=new QHBoxLayout;
    QHBoxLayout *buttonLayout=new QHBoxLayout;

    QComboBox *mitoSelect=new QComboBox;
    mitoSelect->setStyle(new DarkStyle);
    mitoSelect->setStyleSheet(QString("QComboBox {font: bold;}"));

    QLabel *mitoLabel=new QLabel("Mitochondria Channel:");
    mitoLabel->setStyleSheet(QString("QLabel {font: bold;}"));

    QComboBox *venusSelect=new QComboBox;
    venusSelect->setStyle(new DarkStyle);
    venusSelect->setStyleSheet(QString("QComboBox {font: bold;}"));

    QLabel *venusLabel=new QLabel("Neuron Structure Channel:");
    venusLabel->setStyleSheet(QString("QLabel {font: bold;}"));


    QWidget *filler1=new QWidget;
    filler1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    QWidget *filler2=new QWidget;
    filler2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    selectLayout->addWidget(filler1);
    selectLayout->addWidget(venusLabel);
    selectLayout->addWidget(venusSelect);
    selectLayout->addSpacing(100);
    selectLayout->addWidget(mitoLabel);
    selectLayout->addWidget(mitoSelect);
    selectLayout->addWidget(filler2);

    QPushButton *acceptButton=new QPushButton("Ok");
    acceptButton->setStyle(new DarkStyle);
    acceptButton->setStyleSheet(QString("QPushButton {font: bold;}"));
    connect(acceptButton,SIGNAL(pressed()),this,SLOT(handleAcceptButton()));


    QPushButton *cancelButton=new QPushButton("Cancel");
    cancelButton->setStyle(new DarkStyle);
    cancelButton->setStyleSheet(QString("QPushButton {font: bold;}"));
    connect(cancelButton,SIGNAL(pressed()),this,SLOT(handleCancelButton()));

    buttonLayout->setAlignment(Qt::AlignRight);
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(cancelButton);

    int maxChannel=0;
    for(int i=0;i<4;i++){
        QString fileName=dir+"/ch"+QString::number(i)+".tif";
        if(QFile::exists(fileName)){
            maxChannel=i;
        }

    }


    for(int i=0;i<4;i++){
        QString fileName=dir+"/ch"+QString::number(i)+".tif";
        if(QFile::exists(fileName)){
            QImageReader reader(fileName);
            reader.setAutoTransform(true);
            const QImage newImage = reader.read();
            QLabel *view=new QLabel;
            view->setBackgroundRole(QPalette::Base);
            view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            view->setScaledContents(true);
            view->setPixmap(QPixmap::fromImage(newImage));
            view->setFixedSize(200,200);

            QString l="Channel "+QString::number(i);
            if(maxChannel-i==0)l+=" (red)";
            else if(maxChannel-i==1)l+=" (green)";
            else if(maxChannel-i==2)l+=" (blue)";
            QLabel *ch=new QLabel(l);
            ch->setStyleSheet(QString("QLabel {font: bold;}"));

            mitoSelect->addItem(l);
            venusSelect->addItem(l);

            imageLayout->addWidget(view);
            imageLayout->addSpacing(30);
            labelLayout->addWidget(ch,Qt::AlignCenter);
            labelLayout->addSpacing(30);

        }

    }

    venusSelect->setCurrentIndex(maxChannel-1);
    mitoSelect->setCurrentIndex(maxChannel);

    totalLayout->addLayout(imageLayout);
    totalLayout->addLayout(labelLayout);
    totalLayout->addSpacing(20);
    totalLayout->addLayout(selectLayout);
    totalLayout->addSpacing(20);
    totalLayout->addLayout(buttonLayout);

    widget->setLayout(totalLayout);
    widget->setAutoFillBackground(true);
    QPalette tp = widget->palette();
    tp.setColor(QPalette::Background, QColor(255,241,204));
    widget->setPalette(tp);
    widget->setWindowFlags(Qt::SplashScreen);
    connect(this,SIGNAL(selectDone(int)),widget,SLOT(done(int)));
    widget->exec();

    mitoSelectIndex=maxChannel-mitoSelect->currentIndex();
    venusSelectIndex=maxChannel-venusSelect->currentIndex();

    return selectDialogResult;
}

void Window::handleCancelButton(){
    selectDialogResult=false;
    emit selectDone(0);
}

void Window::handleAcceptButton(){
    selectDialogResult=true;
    emit selectDone(1);
}


void Window::addData(){
    QObject *senderObj = sender(); // This will give Sender object
    int index = senderObj->objectName().toInt();
//    QString dir = QFileDialog::getOpenFileName(this, "Select dataset or .nd2 file...", QDir::currentPath()+"/dataset","*.MitoVis *.nd2 *.tif");
    QStringList dir = QFileDialog::getOpenFileNames(this, "Select datasets (.MitoVis),dataset list (.txt), nd2 images, or neuron image files (.nd2/.tif)...", QDir::currentPath()+"/dataset","*.MitoVis *.nd2 *.tif *.txt");
    if (dir.size() == 0)return;
    bool ok=addDataSub(dir,index);

    qDebug()<<"end of dataset generation";
    if(ok==false)return;

    if(projection1->currentIndex()==0||projection1->currentIndex()==1){
        emit projectionRun();
    }
    else tsneGraph->makeCoord();

    dataRendering->colorTableChanged=true;
    setUIbasedOnCurDataIndex();
    synchronization();


    emit recommenderInit();

#ifdef VIS22
    VIS22_userstudy_save(0);
#endif
//    ImageControlSliderChanged(0);
}

bool Window::addDataSub(QStringList dir_list,int index){

    QString init_dirO=dir_list[0];
    QString init_tword="";
    QString init_tdir="";

    QString init_dir="";
    for(int i=0;i<init_dirO.length();i++){
        init_tword+=init_dirO[i];
        if(init_dirO[i]=='/'){
            init_dir+=init_tword;
            init_tdir=init_tword;
            init_tword="";
        }
    }
    qDebug()<<init_dirO;
    qDebug()<<init_tword;

    if(dir_list.size()==1
            && init_tword[init_tword.length()-3]=='t'
            && init_tword[init_tword.length()-2]=='x'
            && init_tword[init_tword.length()-1]=='t'){

        bool is_okay=false;
        FILE *file_list=fopen(QString(dir_list[0]).toStdString().c_str(),"r");
        char file_name[255];
        while(EOF!=fscanf(file_list,"%s",&file_name)){

            bool load_tracing_map=false;
            QString dirO=QString(file_name);
            QString tword="";
            QString tdir="";

            QString dir="";
            for(int i=0;i<dirO.length();i++){
                tword+=dirO[i];
                if(dirO[i]=='/'){
                    dir+=tword;
                    tdir=tword;
                    tword="";
                }
            }
            qDebug()<<dirO;
            qDebug()<<tword;

            if(!(tword[tword.length()-3]=='V' && tword[tword.length()-2]=='i' && tword[tword.length()-1]=='s')){
                continue;
            }

            QListWidgetItem *item = new QListWidgetItem();
            item->setSizeHint(QSize(50,30));//groups[groups.size()-1]->widget->sizeHint());
            item->setBackgroundColor(QColor(50,50,50));
            item->setTextColor(QColor(200,200,200));
            item->setText(tword);
            groups[index]->dataList->addItem(item);
            item->setWhatsThis(QString::number(globalDataIndex));
            item->setSelected(true);
            if(processedIndex==-1)processedItem=item;

            curDataIndex=globalDataIndex;
            if(prevDataIndex!=-1){
                hideAllRadars(prevDataIndex);
            }
            prevDataIndex=curDataIndex;
            globalDataIndex++;

            qDebug()<<dir;
            MitoDataset *newData=new MitoDataset(dir,curDataIndex,index,load_tracing_map);
            newData->global_neurite_ind=&global_neurite_ind;
            newData->global_patch_ind=&global_patch_ind;

            QVector2D max_ind=newData->load_tracingMap(newData->path+"tracing_map.data");
            if(global_patch_ind<max_ind.x()){
                global_patch_ind=max_ind.x();
            }
            if(global_neurite_ind<max_ind.y()){
                global_neurite_ind=max_ind.y();
            }


            connect(newData,SIGNAL(sendStructureChanged()),this,SLOT(structureChanged()));
            connect(newData,SIGNAL(sendCorrectionChanged()),this,SLOT(correctionChanged()));
            connect(newData,SIGNAL(sendConnectionChanged()),this,SLOT(connectionChanged()));

            newData->dir=tword;

            FILE *f=fopen((dir+"features.csv").toStdString().c_str(),"w");
            fprintf(f,"index");
            for(int i=0;i<featureNum;i++){
                fprintf(f,",%s",featureNames[i].toStdString().c_str());
            }
            fprintf(f,"\n");

            for(int i=0;i<newData->features.length();i++){
                fprintf(f,"%d",newData->mitoData[i].index);
                for(int j=0;j<featureNum;j++){
                    fprintf(f,",%f",newData->features[i][j]);
                }
                fprintf(f,"\n");
            }
            fclose(f);

            for(int j=0;j<featureNum;j++){
                featureRanges[j].setX(1000);
                featureRanges[j].setY(-1000);
            }
        //    qDebug()<<newData->features.length();
            for(int i=0;i<newData->features.length();i++){
                for(int j=0;j<featureNum;j++){
        //            qDebug()<<newData->features[i][j];

                    if(featureRanges[j].x()>newData->features[i][j])featureRanges[j].setX(newData->features[i][j]);
                    if(featureRanges[j].y()<newData->features[i][j])featureRanges[j].setY(newData->features[i][j]);
                }
            }
            for(int j=0;j<featureNum;j++){
                if(featureRanges[j].x()==featureRanges[j].y()){
                    featureRanges[j].setX(featureRanges[j].x()-1);
                    featureRanges[j].setY(featureRanges[j].y()+1);
                }
            }


            for(int i=0;i<featureNum;i++){
                minValues[i]=100000;
                maxValues[i]=-100000;
                avgValues[i]=0;
                enabledNum=0;
                newData->enabledNum=0;
                for(int j=0;j<newData->features.length();j++){
                    if(newData->enabled[j]){
                        if(minValues[i]>newData->features[j][i])minValues[i]=newData->features[j][i];
                        if(maxValues[i]<newData->features[j][i])maxValues[i]=newData->features[j][i];
                        avgValues[i]+=newData->features[j][i];
                        enabledNum++;
                    }
                    if(newData->globalEnabled[j]){
                        newData->enabledNum++;
                    }
                }
                if(enabledNum!=0)
                    avgValues[i]/=enabledNum;
            }

            newData->finetune_iter=1;

            dataset.push_back(newData);

            groupIndexOfDataList[curDataIndex]=index;
            curDataName=tword;

            QString label_path = newData->path+"output_dataset/";
            load_pred_datatset(label_path,curDataIndex,0);
             newData->finetune_iter=1;





            is_okay=true;
        }
        fclose(file_list);
        return is_okay;
    }
    else if(dir_list.size()==1){
        bool load_tracing_map=false;
        QString dirO=dir_list[0];
        QString tword="";
        QString tdir="";

        QString dir="";
        for(int i=0;i<dirO.length();i++){
            tword+=dirO[i];
            if(dirO[i]=='/'){
                dir+=tword;
                tdir=tword;
                tword="";
            }
        }
        qDebug()<<dirO;
        qDebug()<<tword;
        if(tword[tword.length()-3]=='n' && tword[tword.length()-2]=='d' && tword[tword.length()-1]=='2'){
            QString dir2 = QFileDialog::getExistingDirectory(this, "Select dataset folder...", QDir::currentPath()+"/dataset");
            if(dir2=="")return false;
            if(dir2[dir2.length()-1]=='/')dir2.remove(dir2.length()-1,1);
            QString imagePath=dirO;
            QString resultPath=dir2;

            std::string query;
            query="python processing/preprocessing_nd2.py ";
            query+='"'+imagePath.toStdString()+'"';
            query+=" ";
            query+='"'+resultPath.toStdString()+'"';
            qDebug()<<QString(query.c_str());
            system(query.c_str());


            bool accept=runChannelSelectDialog(resultPath);

            if(accept==false){
                return false;
            }

            int maxChannel=0;
            for(int i=0;i<4;i++){
                QString fileName=resultPath+"/ch"+QString::number(i)+".tif";
                if(QFile::exists(fileName)){
                    maxChannel=i;
                }
            }



            for(int i=0;i<=maxChannel;i++){
                if(maxChannel-i==mitoSelectIndex){
                    QString fileName0=resultPath+"/ch"+QString::number(i)+".tif";
                    QString fileName1=resultPath+"/mitochondria_image.tif";
                    if (QFile::exists(fileName1))
                    {
                        QFile::remove(fileName1);
                    }
                    QFile::copy(fileName0,fileName1);
                }
                if(maxChannel-i==venusSelectIndex){
                    QString fileName0=resultPath+"/ch"+QString::number(i)+".tif";
                    QString fileName1=resultPath+"/neuron_image.tif";
                    if (QFile::exists(fileName1))
                    {
                        QFile::remove(fileName1);
                    }
                    QFile::copy(fileName0,fileName1);
                }
            }


#ifdef HJ_SERVER_ver
            query="wsl \"/mnt/c/Users/hjoh/excutable_220329/initial_seg.sh\" ";
            query+='"'+getRelativePath(imagePath).toStdString()+'"';
            query+=" ";
            query+='"'+getRelativePath(resultPath).toStdString()+'"';
            query+=" ";
            query+='"'+tword.toStdString()+'"';
            query+=" ";
            query+='"'+getRelativePath(StructureModel_path).toStdString()+'"';
            query+=" ";
            query+='"'+getRelativePath(MitoModel_path).toStdString()+'"';

            qDebug()<<QString(query.c_str());
#else

//            query="wsl \"/mnt/d/MitoVis/build/initial_seg.sh\" ";
//            query+='"'+getRelativePath(imagePath).toStdString()+'"';
//            query+=" ";
//            query+='"'+getRelativePath(resultPath).toStdString()+'"';
//            query+=" ";
//            query+='"'+tword.toStdString()+'"';
//            query+=" ";
//            query+='"'+getRelativePath(StructureModel_path).toStdString()+'"';
//            query+=" ";
//            query+='"'+getRelativePath(MitoModel_path).toStdString()+'"';

            qDebug()<<QString(query.c_str());


            query="python processing/neuron_segmentation_new_model.py ";
            query+='"'+imagePath.toStdString()+'"';
            query+=" ";
            query+='"'+resultPath.toStdString()+'"';
            query+=" ";
            query+='"'+tword.toStdString()+'"';
            query+=" ";
            query+='"'+StructureModel_path.toStdString()+'"';
            query+=" ";
            query+='"'+MitoModel_path.toStdString()+'"';

            qDebug()<<QString(query.c_str());

#endif

            system(query.c_str());
            //Sleep(5000);

            dirO=resultPath+"/"+tword+".MitoVis";
            qDebug()<<dirO;
            dir="";
            tword="";
            tdir="";

            for(int i=0;i<dirO.length();i++){
                tword+=dirO[i];
                if(dirO[i]=='/'){
                    dir+=tword;
                    tdir=tword;
                    tword="";
                }
            }
            qDebug()<<tword;


        }
        else if(tword[tword.length()-3]=='t' && tword[tword.length()-2]=='i' && tword[tword.length()-1]=='f'){
            QString mitoPath = QFileDialog::getOpenFileName(this, "Select mitochondria channel image...", QDir::currentPath()+"/dataset","*.tif");

            if(mitoPath=="")return false;


            QString dir2 = QFileDialog::getExistingDirectory(this, "Select dataset folder...", QDir::currentPath()+"/dataset");
            if(dir2=="")return false;
            if(dir2[dir2.length()-1]=='/')dir2.remove(dir2.length()-1,1);
            QString imagePath=dirO;
            QString resultPath=dir2;


            char tt[30];
            std::string query;
            query="python processing/structure_segmentation.py ";
            query+='"'+imagePath.toStdString()+'"';
            query+=" ";
            query+='"'+resultPath.toStdString()+'/'+'"';
            query+=" 1";
            query+=" ";
            query+='"'+StructureModel_path.toStdString()+'"';

            qDebug()<<QString(query.c_str());

            system(query.c_str());

            query="python processing/mitochondria_segmentation.py ";
            query+='"'+mitoPath.toStdString()+'"';
            query+=" ";
            query+='"'+resultPath.toStdString()+'/'+'"';
            query+=" 1";
            query+=" ";
            query+='"'+MitoModel_path.toStdString()+'"';

            qDebug()<<QString(query.c_str());

            system(query.c_str());


            FILE *res_check=fopen((resultPath+"\\"+tword+".MitoVis").toStdString().c_str(),"w");
            fclose(res_check);

            //Sleep(5000);

            dirO=resultPath+"/"+tword+".MitoVis";
            qDebug()<<dirO;
            dir="";
            tword="";
            tdir="";

            for(int i=0;i<dirO.length();i++){
                tword+=dirO[i];
                if(dirO[i]=='/'){
                    dir+=tword;
                    tdir=tword;
                    tword="";
                }
            }
            qDebug()<<tword;


        }
        else if(tword[tword.length()-3]=='V' && tword[tword.length()-2]=='i' && tword[tword.length()-1]=='s'){
            load_tracing_map=true;
        }

        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(50,30));//groups[groups.size()-1]->widget->sizeHint());
        item->setBackgroundColor(QColor(50,50,50));
        item->setTextColor(QColor(200,200,200));
        item->setText(tword);
        groups[index]->dataList->addItem(item);
        item->setWhatsThis(QString::number(globalDataIndex));
        item->setSelected(true);
        if(processedIndex==-1)processedItem=item;
    //    groups[index]->dataList->addItem(dir);

    //    if(curDataIndex!=-1){
    //        dataset[curDataIndex]->connector->finish_signal=true;
    //    }

        //user study
        //if(curDataIndex!=-1)dataset[curDataIndex]->prev_elapsed_time+=dataset[curDataIndex]->time.elapsed();

        curDataIndex=globalDataIndex;
        if(prevDataIndex!=-1){
            hideAllRadars(prevDataIndex);
        }
        prevDataIndex=curDataIndex;
        globalDataIndex++;

        qDebug()<<"make mito dataset";



        qDebug()<<dir;
        MitoDataset *newData=new MitoDataset(dir,curDataIndex,index,load_tracing_map);
        newData->global_neurite_ind=&global_neurite_ind;
        newData->global_patch_ind=&global_patch_ind;
        if(load_tracing_map){
            QVector2D max_ind=newData->load_tracingMap(newData->path+"tracing_map.data");
            if(global_patch_ind<max_ind.x()){
                global_patch_ind=max_ind.x();
            }
            if(global_neurite_ind<max_ind.y()){
                global_neurite_ind=max_ind.y();
            }
        }
        else{
            connect(newData->corrector, SIGNAL(tracingFinished(int)), this, SLOT(save_dataset_for_training(int)));
            newData->workerThread.start();

            emit newData->correctorInit();
        }


    //    if(imageControlPartLayout->currentIndex()!=5){
    //        newData->connector->finish_signal=true;
    //    }
        connect(newData,SIGNAL(sendStructureChanged()),this,SLOT(structureChanged()));
        connect(newData,SIGNAL(sendCorrectionChanged()),this,SLOT(correctionChanged()));
        connect(newData,SIGNAL(sendConnectionChanged()),this,SLOT(connectionChanged()));

    //    connect(newData->corrector,SIGNAL(startWaiting()),this,SLOT(startWaiting()));
    //    connect(newData->corrector,SIGNAL(endWaiting()),this,SLOT(endWaiting()));


    //    if(curDataIndex!=0){
    //        newData->corrector->cnn=dataset[0]->corrector->cnn;
    //        emit newData->correctorGetCandidatePoints();
    //        emit newData->correctorRun();
    //    }

        newData->dir=tword;
    //    newData->resultPath=resultPath;

        FILE *f=fopen((dir+"features.csv").toStdString().c_str(),"w");
        fprintf(f,"index");
        for(int i=0;i<featureNum;i++){
            fprintf(f,",%s",featureNames[i].toStdString().c_str());
        }
        fprintf(f,"\n");

        for(int i=0;i<newData->features.length();i++){
            fprintf(f,"%d",newData->mitoData[i].index);
            for(int j=0;j<featureNum;j++){
                fprintf(f,",%f",newData->features[i][j]);
            }
            fprintf(f,"\n");
        }
        fclose(f);

        for(int j=0;j<featureNum;j++){
            featureRanges[j].setX(1000);
            featureRanges[j].setY(-1000);
        }
    //    qDebug()<<newData->features.length();
        for(int i=0;i<newData->features.length();i++){
            for(int j=0;j<featureNum;j++){
    //            qDebug()<<newData->features[i][j];

                if(featureRanges[j].x()>newData->features[i][j])featureRanges[j].setX(newData->features[i][j]);
                if(featureRanges[j].y()<newData->features[i][j])featureRanges[j].setY(newData->features[i][j]);
            }
        }
        for(int j=0;j<featureNum;j++){
            if(featureRanges[j].x()==featureRanges[j].y()){
                featureRanges[j].setX(featureRanges[j].x()-1);
                featureRanges[j].setY(featureRanges[j].y()+1);
            }
        }


        for(int i=0;i<featureNum;i++){
            minValues[i]=100000;
            maxValues[i]=-100000;
            avgValues[i]=0;
            enabledNum=0;
            newData->enabledNum=0;
            for(int j=0;j<newData->features.length();j++){
                if(newData->enabled[j]){
                    if(minValues[i]>newData->features[j][i])minValues[i]=newData->features[j][i];
                    if(maxValues[i]<newData->features[j][i])maxValues[i]=newData->features[j][i];
                    avgValues[i]+=newData->features[j][i];
                    enabledNum++;
                }
                if(newData->globalEnabled[j]){
                    newData->enabledNum++;
                }
            }
            if(enabledNum!=0)
                avgValues[i]/=enabledNum;
        }

        newData->finetune_iter=1;

        dataset.push_back(newData);
    //    labelCleaning();

        groupIndexOfDataList[curDataIndex]=index;
        curDataName=tword;



        if(load_tracing_map){
            QString label_path = newData->path+"output_dataset/";
            qDebug()<<label_path;
            load_pred_datatset(label_path,curDataIndex,0);
             newData->finetune_iter=1;
        }

        return true;
    //    typeCompareLayoutUpdate();
    }
    else{
        QString dir_root = QFileDialog::getExistingDirectory(this, "Select dataset folder...", QDir::currentPath()+"/dataset");

        FILE *file_list=fopen((dir_root+"/dataset_list.txt").toStdString().c_str(),"w");
        bool is_ok=false;
        int init_curDataIndex=curDataIndex;
        qDebug()<<"file number: "<<dir_list.size();
        for(int dir_n=0;dir_n<dir_list.size();dir_n++){
            curDataIndex=init_curDataIndex;
            QString dirO=dir_list[dir_n];
            QString tword="";
            QString tdir="";

            QString dir="";
            for(int i=0;i<dirO.length();i++){
                tword+=dirO[i];
                if(dirO[i]=='/'){
                    dir+=tword;
                    tdir=tword;
                    tword="";
                }
            }
            qDebug()<<dirO;
            qDebug()<<tword;

            QString dir2 = dir_root;
            if(dir2[dir2.length()-1]=='/')dir2.remove(dir2.length()-1,1);

            dir2+="/"+tword;
            dir2.remove(dir2.length()-4,4);
            QString imagePath=dirO;
            QString resultPath=dir2;

            std::string query;
            query="python processing/preprocessing_nd2.py ";
            query+='"'+imagePath.toStdString()+'"';
            query+=" ";
            query+='"'+resultPath.toStdString()+'"';
            qDebug()<<QString(query.c_str());
            system(query.c_str());


            if(dir_n==0){
                bool accept=runChannelSelectDialog(resultPath);

                if(accept==false){
                    return false;
                }
            }



            int maxChannel=0;
            for(int i=0;i<4;i++){
                QString fileName=resultPath+"/ch"+QString::number(i)+".tif";
                if(QFile::exists(fileName)){
                    maxChannel=i;
                }
            }



            for(int i=0;i<=maxChannel;i++){
                if(maxChannel-i==mitoSelectIndex){
                    QString fileName0=resultPath+"/ch"+QString::number(i)+".tif";
                    QString fileName1=resultPath+"/mitochondria_image.tif";
                    if (QFile::exists(fileName1))
                    {
                        QFile::remove(fileName1);
                    }
                    QFile::copy(fileName0,fileName1);
                }
                if(maxChannel-i==venusSelectIndex){
                    QString fileName0=resultPath+"/ch"+QString::number(i)+".tif";
                    QString fileName1=resultPath+"/neuron_image.tif";
                    if (QFile::exists(fileName1))
                    {
                        QFile::remove(fileName1);
                    }
                    QFile::copy(fileName0,fileName1);
                }
            }



#ifdef HJ_SERVER_ver
            query="wsl \"/mnt/c/Users/hjoh/excutable_220329/initial_seg.sh\" ";
            query+='"'+getRelativePath(imagePath).toStdString()+'"';
            query+=" ";
            query+='"'+getRelativePath(resultPath).toStdString()+'"';
            query+=" ";
            query+='"'+tword.toStdString()+'"';
            query+=" ";
            query+='"'+getRelativePath(StructureModel_path).toStdString()+'"';
            query+=" ";
            query+='"'+getRelativePath(MitoModel_path).toStdString()+'"';

            qDebug()<<QString(query.c_str());

#else

//            query="wsl \"/mnt/d/MitoVis/build/initial_seg.sh\" ";
//            query+='"'+getRelativePath(imagePath).toStdString()+'"';
//            query+=" ";
//            query+='"'+getRelativePath(resultPath).toStdString()+'"';
//            query+=" ";
//            query+='"'+tword.toStdString()+'"';
//            query+=" ";
//            query+='"'+getRelativePath(StructureModel_path).toStdString()+'"';
//            query+=" ";
//            query+='"'+getRelativePath(MitoModel_path).toStdString()+'"';

//            qDebug()<<QString(query.c_str());

            query="python processing/neuron_segmentation_new_model.py ";
            query+='"'+imagePath.toStdString()+'"';
            query+=" ";
            query+='"'+resultPath.toStdString()+'"';
            query+=" ";
            query+='"'+tword.toStdString()+'"';
            query+=" ";
            query+='"'+StructureModel_path.toStdString()+'"';
            query+=" ";
            query+='"'+MitoModel_path.toStdString()+'"';

            qDebug()<<QString(query.c_str());
#endif

            system(query.c_str());
            //Sleep(5000);

            dirO=resultPath+"/"+tword+".MitoVis";
            qDebug()<<dirO;
            fprintf(file_list,"%s\n",dirO.toStdString().c_str());
            dir="";
            tword="";
            tdir="";

            for(int i=0;i<dirO.length();i++){
                tword+=dirO[i];
                if(dirO[i]=='/'){
                    dir+=tword;
                    tdir=tword;
                    tword="";
                }
            }
            qDebug()<<tword;


            QListWidgetItem *item = new QListWidgetItem();
            item->setSizeHint(QSize(50,30));//groups[groups.size()-1]->widget->sizeHint());
            item->setBackgroundColor(QColor(50,50,50));
            item->setTextColor(QColor(200,200,200));
            item->setText(tword);
            groups[index]->dataList->addItem(item);
            item->setWhatsThis(QString::number(globalDataIndex));
            item->setSelected(true);
            if(processedIndex==-1)processedItem=item;
        //    groups[index]->dataList->addItem(dir);

        //    if(curDataIndex!=-1){
        //        dataset[curDataIndex]->connector->finish_signal=true;
        //    }

            //user study
            //if(curDataIndex!=-1)dataset[curDataIndex]->prev_elapsed_time+=dataset[curDataIndex]->time.elapsed();

            curDataIndex=globalDataIndex;
            if(prevDataIndex!=-1){
                hideAllRadars(prevDataIndex);
            }
            prevDataIndex=curDataIndex;
            globalDataIndex++;

            qDebug()<<"make mito dataset";



            qDebug()<<dir;
            MitoDataset *newData=new MitoDataset(dir,curDataIndex,index,false);
            newData->global_neurite_ind=&global_neurite_ind;
            newData->global_patch_ind=&global_patch_ind;

            connect(newData->corrector, SIGNAL(tracingFinished(int)), this, SLOT(save_dataset_for_training(int)));
            newData->workerThread.start();


            emit newData->correctorInit();



        //    if(imageControlPartLayout->currentIndex()!=5){
        //        newData->connector->finish_signal=true;
        //    }
            connect(newData,SIGNAL(sendStructureChanged()),this,SLOT(structureChanged()));
            connect(newData,SIGNAL(sendCorrectionChanged()),this,SLOT(correctionChanged()));
            connect(newData,SIGNAL(sendConnectionChanged()),this,SLOT(connectionChanged()));

        //    connect(newData->corrector,SIGNAL(startWaiting()),this,SLOT(startWaiting()));
        //    connect(newData->corrector,SIGNAL(endWaiting()),this,SLOT(endWaiting()));


        //    if(curDataIndex!=0){
        //        newData->corrector->cnn=dataset[0]->corrector->cnn;
        //        emit newData->correctorGetCandidatePoints();
        //        emit newData->correctorRun();
        //    }

            newData->dir=tword;
        //    newData->resultPath=resultPath;

            FILE *f=fopen((dir+"features.csv").toStdString().c_str(),"w");
            fprintf(f,"index");
            for(int i=0;i<featureNum;i++){
                fprintf(f,",%s",featureNames[i].toStdString().c_str());
            }
            fprintf(f,"\n");

            for(int i=0;i<newData->features.length();i++){
                fprintf(f,"%d",newData->mitoData[i].index);
                for(int j=0;j<featureNum;j++){
                    fprintf(f,",%f",newData->features[i][j]);
                }
                fprintf(f,"\n");
            }
            fclose(f);

            for(int j=0;j<featureNum;j++){
                featureRanges[j].setX(1000);
                featureRanges[j].setY(-1000);
            }
        //    qDebug()<<newData->features.length();
            for(int i=0;i<newData->features.length();i++){
                for(int j=0;j<featureNum;j++){
        //            qDebug()<<newData->features[i][j];

                    if(featureRanges[j].x()>newData->features[i][j])featureRanges[j].setX(newData->features[i][j]);
                    if(featureRanges[j].y()<newData->features[i][j])featureRanges[j].setY(newData->features[i][j]);
                }
            }
            for(int j=0;j<featureNum;j++){
                if(featureRanges[j].x()==featureRanges[j].y()){
                    featureRanges[j].setX(featureRanges[j].x()-1);
                    featureRanges[j].setY(featureRanges[j].y()+1);
                }
            }


            for(int i=0;i<featureNum;i++){
                minValues[i]=100000;
                maxValues[i]=-100000;
                avgValues[i]=0;
                enabledNum=0;
                newData->enabledNum=0;
                for(int j=0;j<newData->features.length();j++){
                    if(newData->enabled[j]){
                        if(minValues[i]>newData->features[j][i])minValues[i]=newData->features[j][i];
                        if(maxValues[i]<newData->features[j][i])maxValues[i]=newData->features[j][i];
                        avgValues[i]+=newData->features[j][i];
                        enabledNum++;
                    }
                    if(newData->globalEnabled[j]){
                        newData->enabledNum++;
                    }
                }
                if(enabledNum!=0)
                    avgValues[i]/=enabledNum;
            }

            newData->finetune_iter=1;

            dataset.push_back(newData);
        //    labelCleaning();

            groupIndexOfDataList[curDataIndex]=index;
            curDataName=tword;
        //    typeCompareLayoutUpdate();
            is_ok=true;
            if(projection1->currentIndex()==0||projection1->currentIndex()==1){
                emit projectionRun();
            }
            else tsneGraph->makeCoord();

        }
        fclose(file_list);
        return is_ok;

    }

}
void Window::setUIbasedOnCurDataIndex(){

    resolution_edit->setText(QString::number(dataset[curDataIndex]->resolution));

    imageStructureOpacityControl->setValue(dataset[curDataIndex]->structure_opacity*255);
    imageStructureOpacityControl->update();
    imageMitoLabelOpacityControl->setValue(dataset[curDataIndex]->mitoLabel_opacity*255);
    imageMitoLabelOpacityControl->update();
    imageMitoThresOpacityControl->setValue((dataset[curDataIndex]->mito_thresh-0.25)*512);
    imageMitoThresOpacityControl->update();


    int bn=dataset[curDataIndex]->bright_neuron*255;
    int cn=dataset[curDataIndex]->contrast_neuron*255;
    int pn=dataset[curDataIndex]->midpos_neuron*255;
    int bm=dataset[curDataIndex]->bright_mito*255;
    int cm=dataset[curDataIndex]->contrast_mito*255;
    int pm=dataset[curDataIndex]->midpos_mito*255;
    imageBrightControl_neuron->setValue(bn);
    imageBrightControl_neuron->update();
    imageContrastControl_neuron->setValue(cn);
    imageContrastControl_neuron->update();
    imageContrastPosControl_neuron->setValue(pn);
    imageContrastPosControl_neuron->update();
    tfRendering_neuron->update();

    imageBrightControl_mito->setValue(bm);
    imageBrightControl_mito->update();
    imageContrastControl_mito->setValue(cm);
    imageContrastControl_mito->update();
    imageContrastPosControl_mito->setValue(pm);
    imageContrastPosControl_mito->update();
    tfRendering_mito->update();



    correctorSensibilityControl->setValue((dataset[curDataIndex]->correction_threshold-0.5)*255);
    correctorSensibilityControl->update();
    correctorOpacityControl->setValue(dataset[curDataIndex]->structure_thresh*255);
    correctorOpacityControl->update();


    correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));

    if(dataset[curDataIndex]->correctorInputType==0){
        correctorInput_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,0,0);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==1){
        correctorInput_cellbody->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(120,50,220);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==2){
        correctorInput_dend->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(255,142,138);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==3){
        correctorInput_axon->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(30,126,220);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==4){
        correctorInput_tracingP->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,220,220);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==5){
        correctorInput_positivePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,220,220);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==6){
        correctorInput_negativePath->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,50,100);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==7){
        correctorInput_nodeEraser->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,220,220);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==10){
        correctorInput_mito_background->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,0,0);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==11){
        correctorInput_mito_splitting->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(0,0,0);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==12){
        correctorInput_mito_merging->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(255,0,0);}"));
    }
    else if(dataset[curDataIndex]->correctorInputType==13){
        correctorInput_mito_foreground->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(255,0,0);}"));
    }
    correctorInput_nodeEraser->repaint();
    correctorInput_nodeEraser->update();
    correctorInput_tracingP->repaint();
    correctorInput_tracingP->update();
    correctorInput_positivePath->repaint();
    correctorInput_positivePath->update();
    correctorInput_negativePath->repaint();
    correctorInput_negativePath->update();
    correctorInput_background->repaint();
    correctorInput_background->update();
    correctorInput_cellbody->repaint();
    correctorInput_cellbody->update();
    correctorInput_axon->repaint();
    correctorInput_axon->update();
    correctorInput_dend->repaint();
    correctorInput_dend->update();

    correctorInput_mito_background->repaint();
    correctorInput_mito_background->update();
    correctorInput_mito_foreground->repaint();
    correctorInput_mito_foreground->update();
    correctorInput_mito_splitting->repaint();
    correctorInput_mito_splitting->update();
    correctorInput_mito_merging->repaint();
    correctorInput_mito_merging->update();

    correctorEnable_axon->setChecked(dataset[curDataIndex]->correctorEnable_axon);
    correctorEnable_dend->setChecked(dataset[curDataIndex]->correctorEnable_dend);
//    dataset[curDataIndex]->corrector->stopSignal=true;
//    emit dataset[curDataIndex]->correctorRun();


    subsetName->setText(dataset[curDataIndex]->subsetName);
    subsetBrushSize->setValue(dataset[curDataIndex]->brushSize);
    subsetBrushSize->update();
    if(dataset[curDataIndex]->subsetInputType==0){
        subsetAdd->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
        subsetErase->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else if(dataset[curDataIndex]->subsetInputType==1){
        subsetAdd->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(30,126,220);}"));
        subsetErase->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    else if(dataset[curDataIndex]->subsetInputType==2){
        subsetErase->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(30,126,220);}"));
        subsetAdd->setStyleSheet(QString("QPushButton {font: bold; color: rgb(255,255,255); background-color: rgb(200,200,200);}"));
    }
    subsetAdd->repaint();
    subsetAdd->update();
    subsetErase->repaint();
    subsetErase->update();


    connectionBackgroundThreshold->setValue(dataset[curDataIndex]->connection_background_threshold*100);
    connectionBackgroundThreshold->update();

    connectionDistanceThreshold->setValue(dataset[curDataIndex]->connection_distance_threshold);
    connectionDistanceThreshold->update();

    connectionVisibility->setValue(dataset[curDataIndex]->connection_visibility*100);
    connectionVisibility->update();


}
void Window::focusGroup() {
    QObject *senderObj = sender(); // This will give Sender object
    int index = senderObj->objectName().toInt();
    QListWidgetItem* item = groupList->item(index);
    item->setSelected(true);
    groupList->setCurrentItem(item, QItemSelectionModel::Select);
    groupList->repaint();
}
void Window::focusGroup(QString a) {
    QObject *senderObj = sender(); // This will give Sender object
    int index = senderObj->objectName().toInt();
    QListWidgetItem* item = groupList->item(index);
    item->setSelected(true);
    groupList->setCurrentItem(item, QItemSelectionModel::Select);
    groupList->repaint();
}
void Window::handleGroupDelete(){

}
void Window::paintEvent(QPaintEvent *event){
    filer_variationLayout->setFixedWidth(title_no->width()+title_anno->width()+title_group->width()+title_image->width()+25-160);
}
void Window::timerEvent(QTimerEvent *event){

#ifdef VIS22
    if(curDataIndex!=-1){
        VIS22_userstudy_save(dataset[curDataIndex]->finetune_iter*2+1);
    }
#endif

    //saveHistory();

//    filer_variationLayout->setFixedWidth(title_no->width()+title_anno->width()+title_group->width()+title_image->width()+25-160);
}


void Window::selectReset(){
    for(int i=0;i<data.length();i++){
        displaySpines[i]=false;
    }
}
void Window::selectDisplayMode(bool a){

}

void Window::displayShow(bool a){
    checkSpineEnable();
    synchronization();
}

Window::~Window(){

//    for(int i=0;i<dataset.length();i++){
//        if(dataset[i]->workerThread.isRunning()){
//            dataset[i]->corrector->stopSignal=true;
//            dataset[i]->corrector->exitSignal=true;
//            dataset[i]->workerThread.quit();
//            dataset[i]->workerThread.wait();

//        }

//    }

    workerThread_recommender.quit();
    workerThread_recommender.wait();

}


void Window::addSpineToGroup(int a, int group){

}
void Window::deleteSpineFromGroup(int a, int group){

}

void Window::selectionForClustering(){

}

void Window::featureUsageSetting(){

}

void Window::arrangeRanderPart(){


}
void Window::orderingFormatrixView1(){

}
void Window::preOrdering(){

}
void Window::orderingFormatrixView2(){

}
void Window::orderingFormatrixView3(){

}
void Window::recursiveDivideX(int startX,int endX,int startY,int endY,QVector<int> indexList){

}
void Window::recursiveDivideY(int startX,int endX,int startY,int endY,QVector<int> indexList){

}


QSlider *Window::createSlider(int a)
{
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, a);
    slider->setSingleStep(1);
    slider->setPageStep(10);
    slider->setTickInterval(10);
    slider->setTickPosition(QSlider::TicksAbove);
    return slider;
}

void Window::loadFile(const QUrl &fileName)
{
    setWindowFilePath(fileName.toString());
}
void Window::viewChange(int num){

}
void Window::synchronization(){
    //changeTitle();
    parallelPlot->update();
    tsneGraph->update();
    dataRendering->update();
}
void Window::changeFocus(bool a){
    isFocus=a;
}

void Window::changeTitle(){

}

void Window::saveResult(){


}
void Window::runClassification(){


}

bool Window::checkConstraint(int numConstrain,int spineNum){
   return false;
}
void Window::optimizedKmeansClustering(){

}


void Window::kmeansClustering(){


}
void Window::changeColoringType(int a){
    coloringType=a;
    synchronization();

}
void Window::changeAnalysisType(int a){
    if(curDataIndex==-1)return;
    if(analysis_type->currentIndex()==0){
//        qDebug()<<dataset[curDataIndex]->features.length();
        for(int j=0;j<featureNum;j++){
            featureRanges[j].setX(1000);
            featureRanges[j].setY(-1000);
        }
        for(int i=0;i<dataset[curDataIndex]->features.length();i++){
            for(int j=0;j<featureNum;j++){
//                qDebug()<<dataset[curDataIndex]->features[i][j];

                if(featureRanges[j].x()>dataset[curDataIndex]->features[i][j])featureRanges[j].setX(dataset[curDataIndex]->features[i][j]);
                if(featureRanges[j].y()<dataset[curDataIndex]->features[i][j])featureRanges[j].setY(dataset[curDataIndex]->features[i][j]);
            }
        }
        for(int j=0;j<featureNum;j++){
            if(featureRanges[j].x()==featureRanges[j].y()){
                featureRanges[j].setX(featureRanges[j].x()-1);
                featureRanges[j].setY(featureRanges[j].y()+1);
            }
        }
        if(projection1->currentIndex()==0||projection1->currentIndex()==1){
            emit projectionRun();
        }
        else tsneGraph->makeCoord();
    }
    else{
        for(int j=0;j<featureNum;j++){
            featureRanges[j].setX(1000);
            featureRanges[j].setY(-1000);
        }
        for(int g=0;g<dataset.length();g++){
            for(int i=0;i<dataset[g]->features.length();i++){
                for(int j=0;j<featureNum;j++){
                    if(featureRanges[j].x()>dataset[g]->features[i][j])featureRanges[j].setX(dataset[g]->features[i][j]);
                    if(featureRanges[j].y()<dataset[g]->features[i][j])featureRanges[j].setY(dataset[g]->features[i][j]);
                }
            }
        }
        for(int j=0;j<featureNum;j++){
            if(featureRanges[j].x()==featureRanges[j].y()){
                featureRanges[j].setX(featureRanges[j].x()-1);
                featureRanges[j].setY(featureRanges[j].y()+1);
            }
        }

        if(projection1->currentIndex()==0||projection1->currentIndex()==1){
            emit projectionRun();
        }
        else tsneGraph->makeCoord();


    }

    synchronization();

}
void Window::checkSpineEnable(){

}
void Window::runCheckSpineEnable(bool a){
    checkSpineEnable();
    synchronization();

}

void Window::generateFlow(){

}


MitoDataset::MitoDataset(QString baseDir,int ind,int groupInd,bool tracing_map_load=false){
    // user study
//    time.start();
    for(int i=0;i<10;i++){
        selection.push_back(false);
        selectionRange.push_back(QVector2D(1000,-1000));
        axisRange.push_back(QVector2D(0,1));
    }
    group=groupInd;
    dataIndex=ind;
    path=baseDir;
    setColorTable();
    //load neuron image
    TinyTIFFReaderFile* tiffr=NULL;
    tiffr=TinyTIFFReader_open((baseDir+"neuron_image.tif").toStdString().c_str());
    while(!tiffr){
        tiffr=TinyTIFFReader_open((baseDir+"neuron_image.tif").toStdString().c_str());
    }

    if (!tiffr) {
        //qDebug()<<"    ERROR reading (not existent, not accessible or no TIFF file)";
    } else {
        //if (TinyTIFFReader_wasError(tiffr)) qDebug()<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr);
        //qDebug()<<"    ImageDescription:"<< TinyTIFFReader_getImageDescription(tiffr).c_str();

        uint32_t width=TinyTIFFReader_getWidth(tiffr);
        uint32_t height=TinyTIFFReader_getHeight(tiffr);

        ImageType=TinyTIFFReader_getBitsPerSample(tiffr);
        //qDebug()<<"    type: "<<newData->type;

        uint32_t frames=TinyTIFFReader_countFrames(tiffr);
        //qDebug()<<"    frames: "<<frames;

        ImageW=width,ImageH=height,ImageD=frames;
        imageData1=new unsigned short[width*height*frames]();

        //if (TinyTIFFReader_wasError(tiffr)) qDebug()<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr);
        uint32_t frame=0;
        do {
            bool ok=true;
            //if (width>0 && height>0) qDebug()<<"    size of frame "<<frame<<": "<<width<<"x"<<height;
            //else { qDebug()<<"    ERROR IN FRAME "<<frame<<": size too small "<<width<<"x"<<height; ok=false; }
            if (ok) {
                TinyTIFFReader_getSampleData(tiffr, &imageData1[frame*width*height], 0);
                //if (TinyTIFFReader_wasError(tiffr)) { ok=false; qDebug()<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr); }
                frame++;

            }
        } while (TinyTIFFReader_readNext(tiffr)); // iterate over all frames
        //qDebug()<<"    read "<<frame<<" frames";
    }
    TinyTIFFReader_close(tiffr);

    float maxD=0;
    for(int i=0;i<ImageW*ImageH;i++){
//        imageData1[i]=ReverseShort(imageData1[i]);
        if(imageData1[i]>maxD)maxD=imageData1[i];
    }
    for(int i=0;i<ImageW*ImageH;i++){
        imageData1[i]=float(imageData1[i])/maxD*65535;
    }

    qDebug()<<"end load image";

    //load mitochondria image
    tiffr=NULL;
    tiffr=TinyTIFFReader_open((baseDir+"mitochondria_image.tif").toStdString().c_str());
    while(!tiffr){
        tiffr=TinyTIFFReader_open((baseDir+"mitochondria_image.tif").toStdString().c_str());
    }
    if (!tiffr) {
        //qDebug()<<"    ERROR reading (not existent, not accessible or no TIFF file)";
    } else {
        //if (TinyTIFFReader_wasError(tiffr)) qDebug()<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr);
        //qDebug()<<"    ImageDescription:"<< TinyTIFFReader_getImageDescription(tiffr).c_str();

        uint32_t width=TinyTIFFReader_getWidth(tiffr);
        uint32_t height=TinyTIFFReader_getHeight(tiffr);

        ImageType=TinyTIFFReader_getBitsPerSample(tiffr);
        //qDebug()<<"    type: "<<newData->type;

        uint32_t frames=TinyTIFFReader_countFrames(tiffr);
        //qDebug()<<"    frames: "<<frames;

        ImageW=width,ImageH=height,ImageD=frames;
        imageData2=new unsigned short[width*height*frames]();

        //if (TinyTIFFReader_wasError(tiffr)) qDebug()<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr);
        uint32_t frame=0;
        do {
            bool ok=true;
            //if (width>0 && height>0) qDebug()<<"    size of frame "<<frame<<": "<<width<<"x"<<height;
            //else { qDebug()<<"    ERROR IN FRAME "<<frame<<": size too small "<<width<<"x"<<height; ok=false; }
            if (ok) {
                TinyTIFFReader_getSampleData(tiffr, &imageData2[frame*width*height], 0);
                //if (TinyTIFFReader_wasError(tiffr)) { ok=false; qDebug()<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr); }
                frame++;

            }
        } while (TinyTIFFReader_readNext(tiffr)); // iterate over all frames
        //qDebug()<<"    read "<<frame<<" frames";
    }
    TinyTIFFReader_close(tiffr);



    maxD=0;
    for(int i=0;i<ImageW*ImageH;i++){
//        imageData2[i]=ReverseShort(imageData2[i]);
        if(imageData2[i]>maxD)maxD=imageData2[i];
    }
    for(int i=0;i<ImageW*ImageH;i++){
        imageData2[i]=float(imageData2[i])/maxD*65535;
    }

    qDebug()<<"end load mito";

    //load structure label
    tiffr=NULL;
    tiffr=TinyTIFFReader_open((baseDir+"structure_label.tif").toStdString().c_str());
    while(!tiffr){
        tiffr=TinyTIFFReader_open((baseDir+"structure_label.tif").toStdString().c_str());
    }
    if (!tiffr) {
        //qDebug()<<"    ERROR reading (not existent, not accessible or no TIFF file)";
    } else {
        //if (TinyTIFFReader_wasError(tiffr)) qDebug()<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr);
        //qDebug()<<"    ImageDescription:"<< TinyTIFFReader_getImageDescription(tiffr).c_str();

        uint32_t width=TinyTIFFReader_getWidth(tiffr);
        uint32_t height=TinyTIFFReader_getHeight(tiffr);
        uint32_t frames=TinyTIFFReader_countFrames(tiffr);
        //qDebug()<<"    frames: "<<frames;
        structureData=new unsigned char[width*height*frames]();
        connectionStructureData=new unsigned char[width*height*frames]();
        scribbleData=new unsigned char[width*height*frames]();
        correctionData=new unsigned char[width*height*frames]();
        predictData=new unsigned char[width*height*frames]();

        subsetData=new unsigned char[width*height*frames]();
        mitoCorrectionData=new unsigned char[width*height*frames]();
        cc_buffer=new unsigned char[width*height*frames]();
        mito_buffer=new unsigned char[width*height*frames]();

        //if (TinyTIFFReader_wasError(tiffr)) qDebug()<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr);
        uint32_t frame=0;
        do {
            bool ok=true;
            //if (width>0 && height>0) qDebug()<<"    size of frame "<<frame<<": "<<width<<"x"<<height;
            //else { qDebug()<<"    ERROR IN FRAME "<<frame<<": size too small "<<width<<"x"<<height; ok=false; }
            if (ok) {
                TinyTIFFReader_getSampleData(tiffr, &structureData[frame*width*height], 0);
                //if (TinyTIFFReader_wasError(tiffr)) { ok=false; qDebug()<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr); }
                frame++;

            }
        } while (TinyTIFFReader_readNext(tiffr)); // iterate over all frames
        //qDebug()<<"    read "<<frame<<" frames";
    }
    TinyTIFFReader_close(tiffr);

//    QMap <int,bool> labelMap;
//    QVector<int> labelList;
//    for(int i=0;i<ImageW*ImageH*ImageD;i++){
//        if(structureData[i]!=0){
//            if(labelMap.find(structureData[i])==labelMap.end()){
//                qDebug()<<"structure: "<<structureData[i];
//                labelMap[structureData[i]]=true;
//                labelList.push_back(structureData[i]);
//            }
//        }
//    }
//    qSort(labelList);
//    qDebug()<<labelList.length();
//    QMap <int,int> labelInverse;
//    for(int i=0;i<labelList.length();i++){
//        labelInverse[labelList[i]]=i+1;
//        qDebug()<<"structure order: "<<labelList[i];

//    }
    qDebug()<<ImageW;
    qDebug()<<ImageH;
    qDebug()<<ImageD;
//    for(int i=0;i<ImageW*ImageH*ImageD;i++){

//        if(structureData[i]!=0){

//            structureData[i]=labelInverse[structureData[i]];
//        }
//    }
//    maxStructureLabel=labelList.length();
    for(int i=0;i<ImageW*ImageH*ImageD;i++){
        if(structureData[i]!=0){

            if(maxStructureLabel<structureData[i]){
                maxStructureLabel=structureData[i];
            }
        }
    }
    float pixelArea=resolution*resolution;
    for(int i=0;i<=maxStructureLabel;i++){
        float areaa=0;
        for(int j=0;j<ImageW*ImageH*ImageD;j++){
            if(structureData[j]==i)areaa+=pixelArea;
        }
        structureArea.push_back(areaa);
    }

    qDebug()<<"max strucutre label: "<<maxStructureLabel;


    mitoLabelImage_probability=new unsigned short[ImageW*ImageH]();
    mitoLabelImage=new unsigned short[ImageW*ImageH]();
    mitoLabelImage_label=new unsigned char[ImageW*ImageH]();


    //load mito label
    tiffr=NULL;
    tiffr=TinyTIFFReader_open((baseDir+"mitochondria_label.tif").toStdString().c_str());
    while(!tiffr){
        tiffr=TinyTIFFReader_open((baseDir+"mitochondria_label.tif").toStdString().c_str());
    }
    if (!tiffr) {
        //qDebug()<<"    ERROR reading (not existent, not accessible or no TIFF file)";
    } else {
        //if (TinyTIFFReader_wasError(tiffr)) qDebug()<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr);
        //qDebug()<<"    ImageDescription:"<< TinyTIFFReader_getImageDescription(tiffr).c_str();

        uint32_t width=TinyTIFFReader_getWidth(tiffr);
        uint32_t height=TinyTIFFReader_getHeight(tiffr);
        uint32_t frames=TinyTIFFReader_countFrames(tiffr);
        //qDebug()<<"    frames: "<<frames;

        //if (TinyTIFFReader_wasError(tiffr)) qDebug()<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr);
        uint32_t frame=0;
        do {
            bool ok=true;
            //if (width>0 && height>0) qDebug()<<"    size of frame "<<frame<<": "<<width<<"x"<<height;
            //else { qDebug()<<"    ERROR IN FRAME "<<frame<<": size too small "<<width<<"x"<<height; ok=false; }
            if (ok) {
                TinyTIFFReader_getSampleData(tiffr, &mitoLabelImage_label[frame*width*height], 0);
                //if (TinyTIFFReader_wasError(tiffr)) { ok=false; qDebug()<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr); }
                frame++;

            }
        } while (TinyTIFFReader_readNext(tiffr)); // iterate over all frames
        //qDebug()<<"    read "<<frame<<" frames";
    }
    TinyTIFFReader_close(tiffr);






//    mitoLineImage=new unsigned char[ImageW*ImageH]();
    check=new bool[ImageW*ImageH]();
    for(int i=0;i<ImageW*ImageH;i++){
        if(structureData[i]==CELL_BODY){
            check[i]=true;
        }
    }



    //load mitochondria label

    Mito newL;
    newL.index=0;
    newL.xmin=1;
    newL.ymin=1;
    newL.xmax=ImageW-2;
    newL.ymax=ImageH-2;

    addMitoByConnectedComponent(newL);
    featureSave();



    mitoLabelImage_signal=new unsigned short[ImageW*ImageH]();
    changeMito_signal();



    corrector=new structureRefinement(this);
    tracingMap=corrector->tracingMap;
    tracingMap_simplified=new QVector<int>[ImageW*ImageH];
    tracingMap_simplified_collections=new QVector<int>[ImageW*ImageH];
    structureData_simplified=new unsigned char[ImageW*ImageH];
    corrector->moveToThread(&workerThread);

    connect(&workerThread, &QThread::finished, corrector, &QObject::deleteLater);

    connect(this, SIGNAL(correctorInit()), corrector, SLOT(init()));
    connect(this, SIGNAL(correctorRun()), corrector, SLOT(doPredictByTracing()));
    connect(this, SIGNAL(correctorReTrain()), corrector, SLOT(reTraining()));
    connect(this, SIGNAL(correctorGetCandidatePoints()), corrector, SLOT(getCandidatePoints()));

    connect(corrector, SIGNAL(structureChanged()), this, SLOT(structureChanged()));
    connect(corrector, SIGNAL(correctionChanged()), this, SLOT(correctionChanged()));
    connect(this, SIGNAL(correctorTracingPointAdded(int)), corrector, SLOT(tracingFromOneNode(int)));

    connect(this, SIGNAL(correctorUserCostBrushing()), corrector, SLOT(handleUserCostBrushing()));
    connect(this, SIGNAL(correctorStructureBrushing()), corrector, SLOT(handleStructureBrushing()));

    connect(corrector, SIGNAL(connection_graph_updated()), this, SLOT(connectionChanged()));
    connect(this, SIGNAL(userInteractFinished()), corrector, SLOT(userInteractFinished()));





    //user study
    /*
    {
        initial_deploy_time=time.elapsed();
        QString t_path=path+"history/";
        QDir().mkpath(t_path);

        FILE *f=fopen((t_path+"initial_deployed.txt").toStdString().c_str(),"w");
        if(f==NULL)return;
        fprintf(f,"Time (ms): %d\n",initial_deploy_time);
        fclose(f);


        f=fopen((t_path+"initial_feature.csv").toStdString().c_str(),"w");
        if(f==NULL)return;
        fprintf(f,"No,Structure,Area,Length,Eccentricity,Circularity,Enabled\n");
        for(int i=0;i<features.length();i++){
            fprintf(f,"%d",i);
            for(int j=0;j<features[i].length();j++){
                fprintf(f,",%f",features[i][j]);
            }
            fprintf(f,",%d",enabled[i]);
            fprintf(f,"\n");
        }
        fclose(f);


        saveStructureLabel(t_path+"initial_structure_label.tif");

        TinyTIFFFile* tif=TinyTIFFWriter_open((t_path+"initial_mitochondria_object.tif").toStdString().c_str(), 16,ImageW,ImageH);
        if (tif) {
            TinyTIFFWriter_writeImage(tif, mitoLabelImage);
            TinyTIFFWriter_close(tif);
        }
        else {
            QMessageBox msgBox;
            msgBox.setText("Unknown error...");
            msgBox.setStyle(new DarkStyle);
            msgBox.exec();
         }
    }
    */

}


void MitoDataset::structureChanged(){
//    connector->update_path_map();
    emit sendStructureChanged();
}
void MitoDataset::correctionChanged(){
    emit sendCorrectionChanged();
}

void MitoDataset::connectionChanged(){
    //connectionStructureUpdate();
    emit sendConnectionChanged();
}

void Window::structureChanged(){
    dataRendering->structureChanged=true;
    dataRendering->update();

}
void Window::correctionChanged(){
    dataRendering->correctionChanged=true;
    dataRendering->update();

}

void Window::connectionChanged(){
    if(curDataIndex==-1)return;
//    dataset[curDataIndex]->connectionStructureUpdate();
    dataRendering->structureChanged=true;
//    dataRendering->connectionChanged=true;
    dataRendering->update();

}
void Window::correctionChanged(int a){
    if(curDataIndex==-1)return;
    dataset[curDataIndex]->correction_opacity=float(a)/255;
    dataRendering->correctionChanged=true;
    dataRendering->update();

}
void MitoDataset::setColorTable(){
    for(int i=0;i<65536;i++){
        float t=1.0/65535 * i;
        float c=contrast_neuron; //0~1
        c=pow(c,3)*10; //0~10
        float minv=1.0/(1+exp(-c*(-6)));
        float maxv=1.0/(1+exp(-c*6));
        t=t*12-6;// -6 ~ 6
        t=t - (midpos_neuron-0.5) *12;
        t=1.0/(1+exp(-c*t)); //0~1
        t=(t-minv)/(maxv-minv);

        t=t*bright_neuron*2;
        if(t<0)t=0;
        if(t>1)t=1;
        color_table_neuron[i]=t;
    }
    for(int i=0;i<65536;i++){
        float t=1.0/65535 * i;
        float c=contrast_mito; //0~1
        c=pow(c,3)*10; //0~10
        float minv=1.0/(1+exp(-c*(-6)));
        float maxv=1.0/(1+exp(-c*6));
        t=t*12-6;// -6 ~ 6
        t=t - (midpos_mito-0.5) *12;
        t=1.0/(1+exp(-c*t)); //0~1
        t=(t-minv)/(maxv-minv);

        t=t*bright_mito*2;
        if(t<0)t=0;
        if(t>1)t=1;
        color_table_mito[i]=t;
    }
}

void MitoDataset::addMitoByConnectedComponent(Mito boundary){

//    for(int i=0;i<ImageW*ImageH;i++)mitoLineImage[i]=0;

    for(int iy=boundary.ymin;iy<=boundary.ymax;iy++){
        for(int ix=boundary.xmin;ix<=boundary.xmax;ix++){
            int tP=iy*ImageW+ix;
            if(check[tP]==false){
                check[tP]=true;

//                float t=color_table_mito[imageData2[iy*ImageW+ix]];
//                float t=float(mitoLabelImage_probability[iy*ImageW+ix])/65535;
//                if(t>mito_thresh){
                if(mitoCorrectionData[tP]!=10 && mitoCorrectionData[tP]!=11){
                    if(mitoLabelImage_label[tP]==1 || mitoCorrectionData[tP]==12 || mitoCorrectionData[tP]==13){
                        ConnectedComponent(boundary,ix,iy,mitoData.length()+1);
                    }
                }
            }
        }
    }
    qDebug()<<"end";

}
void MitoDataset::ConnectedComponent(Mito L,int startx, int starty, int ind){
    //qDebug()<<"start";

    Mito newL;
    newL.index=ind;
    newL.xmax=startx;
    newL.ymax=starty;
    newL.xmin=startx;
    newL.ymin=starty;

    int dxylist[8][2]={{-1,0},{1,0},{0,-1},{0,1},{1,1},{1,-1},{-1,1},{-1,-1}};

    int cnt=1;
    QQueue<int> quex,quey;
    QQueue<int> quex2,quey2;
    quex2.enqueue(startx);
    quey2.enqueue(starty);
//    mitoLabelImage[starty*ImageW+startx]=ind;
    quex.enqueue(startx);
    quey.enqueue(starty);

    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();

        for(int d=0;d<4;d++){
            if(curx+dxylist[d][0]<L.xmin || curx+dxylist[d][0]>L.xmax || cury+dxylist[d][1]<L.ymin || cury+dxylist[d][1]>L.ymax){
                continue;
            }
            int tP=(cury+dxylist[d][1])*ImageW + curx+dxylist[d][0];
            if(check[tP]==false){
                check[tP]=true;
//                float t=color_table_mito[imageData2[tP]];
//                float t=float(mitoLabelImage_probability[tP])/65535;
                //                if(t>mito_thresh){
//                if(t>mito_thresh){
                if(mitoCorrectionData[tP]!=10 && mitoCorrectionData[tP]!=11){
                    if(mitoLabelImage_label[tP]==1 || mitoCorrectionData[tP]==12 || mitoCorrectionData[tP]==13){
                        quex.enqueue(curx+dxylist[d][0]);
                        quey.enqueue(cury+dxylist[d][1]);
                        quex2.enqueue(curx+dxylist[d][0]);
                        quey2.enqueue(cury+dxylist[d][1]);
                        cnt++;
    //                        mitoLabelImage[tP]=ind;
                        if(curx+dxylist[d][0]<newL.xmin)newL.xmin=curx+dxylist[d][0];
                        if(curx+dxylist[d][0]>newL.xmax)newL.xmax=curx+dxylist[d][0];
                        if(cury+dxylist[d][1]<newL.ymin)newL.ymin=cury+dxylist[d][1];
                        if(cury+dxylist[d][1]>newL.ymax)newL.ymax=cury+dxylist[d][1];
                    }
                }
            }
        }
    }
    //qDebug()<<"qq";
    if(cnt>=4){
        while(!quex2.empty()){
            int curx=quex2.dequeue();
            int cury=quey2.dequeue();
            mitoLabelImage[cury*ImageW+curx]=ind;

        }
        //qDebug()<<newL.xmin<<newL.xmax<<newL.ymin<<newL.ymax;
        if(getFeatures(newL)==false)return;
        //qDebug()<<"qqqddd";

        mitoData.push_back(newL);
        mitoColors.push_back(QColor(qrand()%206+50,qrand()%206+50,qrand()%206+50));
        enabled.push_back(true);
        globalEnabled.push_back(true);

    }
    //qDebug()<<"qqq";

}
bool MitoDataset::getFeatures(Mito m){
    QVector <float> curFeature; //area, eccentricity, length, Circularity, mean intensity, Perimeter



    float pixelArea=resolution*resolution;
    float area=0; //0.2094 * 0.2094 micron per one pixel
    float eccentricity=0; //circle:0~1:line
    float length=0; //deepest points line
    float circularity=0; //line:0~1:circle
    float intensity1=0; //0~1
    float intensity2=0; //0~1
    float perimeter=0; //micron
    int count=0; //pixel number
    int structure=0; //0: none 1~3: dendrite, axon, cell body (order can be changed)
    float posx=0;
    float posy=0;


    length=getLength(m);


    int structCnt[20];
    for(int i=0;i<=maxStructureLabel;i++){
        structCnt[i]=0;
    }

    for(int iy=m.ymin;iy<=m.ymax;iy++){
        for(int ix=m.xmin;ix<=m.xmax;ix++){
            int tP=iy*ImageW+ix;
            if(mitoLabelImage[tP]==m.index){
                structCnt[structureData[tP]]++;
                count++;
                area+=pixelArea;
                intensity1+=float(imageData1[tP])/65535;
                intensity2+=float(imageData2[tP])/65535;
                posx+=ix;
                posy+=iy;
                if(iy==0 || iy==ImageH-1 || ix==0 || ix==ImageW-1)perimeter+=resolution;
                else{
                    if(mitoLabelImage[tP-1]!=m.index ||
                        mitoLabelImage[tP+1]!=m.index ||
                        mitoLabelImage[tP-ImageW]!=m.index ||
                        mitoLabelImage[tP+ImageW]!=m.index){
                            perimeter+=resolution;
                    }
                }
            }
        }
    }
    if(count==0)return false;
    intensity1/=count;
    intensity2/=count;
    posx/=count;
    posy/=count;


    float maxLen=0;
    for(int iy=m.ymin;iy<=m.ymax;iy++){
        for(int ix=m.xmin;ix<=m.xmax;ix++){
            int tP=iy*ImageW+ix;
            if(mitoLabelImage[tP]==m.index){
                if(iy==0 || iy==ImageH-1 || ix==0 || ix==ImageW-1){
                    int l=sqrt((posx-ix)*(posx-ix)+(posy-iy)*(posy-iy));
                    if(l>maxLen)maxLen=l;
                }
                else{
                    if(mitoLabelImage[tP-1]!=m.index ||
                        mitoLabelImage[tP+1]!=m.index ||
                        mitoLabelImage[tP-ImageW]!=m.index ||
                        mitoLabelImage[tP+ImageW]!=m.index){
                        int l=sqrt((posx-ix)*(posx-ix)+(posy-iy)*(posy-iy));
                        if(l>maxLen){
                            maxLen=l;
                        }
                    }
                }
            }
        }
    }


    //length=maxLen*2*0.2094;

//    if(length>60)return false;




    circularity=(4*area*3.141593)/(perimeter*perimeter);

    int maxCnt=0;
    for(int i=1;i<=maxStructureLabel;i++){
        if(structCnt[i]>maxCnt){
            maxCnt=structCnt[i];
            structure=i;
        }
    }
    if(structure==0){
        return false;
    }


    float covarianceMatrix[2][2];
    double means[2] = {posx, posy };
    for (int i = 0; i < 2; i++){
        for (int j = 0; j < 2; j++){
            covarianceMatrix[i][j] = 0.0;
            for(int iy=m.ymin;iy<=m.ymax;iy++){
                for(int ix=m.xmin;ix<=m.xmax;ix++){
                    int tP=iy*ImageW+ix;
                    if(mitoLabelImage[tP]==m.index){
                        if (i == 0 && j == 0)
                            covarianceMatrix[i][j] += (means[i] - ix) * (means[j] - ix);
                        else if (i == 0 && j == 1)
                            covarianceMatrix[i][j] += (means[i] - ix) * (means[j] - iy);
                        else if (i == 1 && j == 0)
                            covarianceMatrix[i][j] += (means[i] - iy) * (means[j] - ix);
                        else if (i == 1 && j == 1)
                            covarianceMatrix[i][j] += (means[i] - iy) * (means[j] - iy);
                    }
                }
            }
            covarianceMatrix[i][j] /= count;
        }
    }
    Eigen::MatrixXd Mat(2, 2);
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            Mat(i, j) = covarianceMatrix[i][j];
        }
    }
    Eigen::EigenSolver<Eigen::MatrixXd> es(Mat);
    std::vector<float> eigenvals;
    float sum = 0;
    for (int i = 0; i < 2; i++)
    {
        eigenvals.push_back(es.eigenvalues()[i].real());
        sum += eigenvals.at(i);

    }
    std::sort(eigenvals.begin(), eigenvals.end(), std::greater<int>());

//    length=eigenvals[0]*0.2094;
    eccentricity=sqrt(1 - ((eigenvals[1] * eigenvals[1]) / (eigenvals[0] * eigenvals[0])));
    if(isnan(eccentricity))eccentricity=1;
    curFeature.push_back(structure);
    curFeature.push_back(area);
    curFeature.push_back(length);
    curFeature.push_back(eccentricity);
//    curFeature.push_back(perimeter);
    curFeature.push_back(circularity);

    features.push_back(curFeature);
    return true;
}

float MitoDataset::getLength(Mito m){



    QVector <QVector2D> vertexs;
    QVector <QVector2D> boundaries;
    QVector <float> deep;
    QVector <int> deepIndex;

    QVector2D midPoint(0,0);
    int startPointIndex=0;
    int endPointIndex=0;

    for(int iy=m.ymin;iy<=m.ymax;iy++){
        for(int ix=m.xmin;ix<=m.xmax;ix++){
            int tP=iy*ImageW+ix;
            if(mitoLabelImage[tP]==m.index){
                if(iy==0 || iy==ImageH-1 || ix==0 || ix==ImageW-1){
                    boundaries.push_back(QVector2D(ix-m.xmin,iy-m.ymin));
                }
                else{
                    if(mitoLabelImage[tP-1]!=m.index ||
                        mitoLabelImage[tP+1]!=m.index ||
                        mitoLabelImage[tP-ImageW]!=m.index ||
                        mitoLabelImage[tP+ImageW]!=m.index){
                        boundaries.push_back(QVector2D(ix-m.xmin,iy-m.ymin));
                    }
                    else{
                        vertexs.push_back(QVector2D(ix-m.xmin,iy-m.ymin));
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

//    for(int i=0;i<vertexs.length();i++){
//        mitoLineImage[int(vertexs[i].y()+m.ymin)*ImageW+int(vertexs[i].x()+m.xmin)]=deep[i]*20;
//    }


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
    if(startPointIndex==endPointIndex)return resolution;

    QVector3D axis=QVector3D::crossProduct(QVector3D(boundaries[startPointIndex]-boundaries[endPointIndex],0),QVector3D(0,-1,0));
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

//    mitoLineImage[int((boundaries[startPointIndex].y()+m.ymin)*ImageW+boundaries[startPointIndex].x()+m.xmin)]=255;
    QVector2D prevP=QVector2D(boundaries[startPointIndex].x()*cos(angle)-boundaries[startPointIndex].y()*sin(angle),
                              boundaries[startPointIndex].y()*cos(angle)+boundaries[startPointIndex].x()*sin(angle));
    float maxDeep=-1;
    int ind=0;
    float resLength=0;
    float curH=5;
    for(int i=0;i<rotated_vertexs.length();i++){
        //mitoLineImage[int((rotated_vertexs[i].value.y()+m.ymin))*ImageW+int(rotated_vertexs[i].value.x()+m.xmin)]=255;

        if(rotated_vertexs[i].yvalue>curH+minY){
            resLength+=(prevP-rotated_vertexs[ind].value).length();

//            if(rotated_vertexs[ind].ind1!=-1)
//                mitoLineImage[int((boundaries[rotated_vertexs[ind].ind1].y()+m.ymin))*ImageW+int(boundaries[rotated_vertexs[ind].ind1].x()+m.xmin)]=255;
//            else
//                mitoLineImage[int((vertexs[rotated_vertexs[ind].ind2].y()+m.ymin))*ImageW+int(vertexs[rotated_vertexs[ind].ind2].x()+m.xmin)]=255;

//            mitoLineImage[int((rotated_vertexs[ind].value.y()+m.ymin)*ImageW+rotated_vertexs[ind].value.x()+m.xmin)]=255;


            prevP=rotated_vertexs[ind].value;
            maxDeep=-1;
            curH+=5;
        }
        if(rotated_vertexs[i].deep>maxDeep){
            maxDeep=rotated_vertexs[i].deep;
            ind=i;
        }
    }
    QVector2D endP=QVector2D(boundaries[endPointIndex].x()*cos(angle)-boundaries[endPointIndex].y()*sin(angle),
                             boundaries[endPointIndex].y()*cos(angle)+boundaries[endPointIndex].x()*sin(angle));
    resLength+=(prevP-endP).length();
//    mitoLineImage[int((boundaries[endPointIndex].y()+m.ymin)*ImageW+boundaries[endPointIndex].x()+m.xmin)]=255;

    return resLength*resolution;

}


//float MitoDataset::getLength(Mito m){
//    int xwidth=(m.xmax-m.xmin+1);
//    int ywidth=(m.ymax-m.ymin+1);
//    float *distanceMap=new float[ywidth*xwidth]();

//    for(int iy=m.ymin;iy<=m.ymax;iy++){
//        for(int ix=m.xmin;ix<=m.xmax;ix++){
//            int tP=iy*ImageW+ix;
//            int tP2=(iy-m.ymin)*xwidth+ix-m.xmin;
//            distanceMap[tP2]=-1;
//            if(mitoLabelImage[tP]==m.index){
//                distanceMap[tP2]=10000;
//                if(iy==0 || iy==ImageH-1 || ix==0 || ix==ImageW-1){
//                    distanceMap[tP2]=0;
//                }
//                else{
//                    if(mitoLabelImage[tP-1]!=m.index ||
//                        mitoLabelImage[tP+1]!=m.index ||
//                        mitoLabelImage[tP-ImageW]!=m.index ||
//                        mitoLabelImage[tP+ImageW]!=m.index){
//                        distanceMap[tP2]=0;
//                    }
//                }
//            }
//        }
//    }

//    for(int iy=0;iy<ywidth;iy++){
//        for(int ix=0;ix<xwidth;ix++){
//            int tP=iy*xwidth+ix;
//            if(abs(distanceMap[tP])<0.0001){
//                for(int iy2=0;iy2<ywidth;iy2++){
//                    for(int ix2=0;ix2<xwidth;ix2++){
//                        float dis=sqrt((ix-ix2)*(ix-ix2)+(iy-iy2)*(iy-iy2));
//                        if(dis<distanceMap[tP])distanceMap[tP]=dis;
//                    }
//                }
//            }
//        }
//    }
//    float *costMap=new float[ywidth*xwidth]();
//    for(int i=0;i<xwidth*ywidth;i++){
//        costMap[i]=1000-distanceMap[i]*100;
//    }

//    qDebug()<<"tt1";
//    float **dis=new float*[xwidth*ywidth];
//    for(int i=0;i<xwidth*ywidth;i++){
//        dis[i]=new float[xwidth*ywidth];
//    }
//    qDebug()<<"tt2";

//    for (int iy = 0; iy < ywidth; iy++) {
//        for(int ix=0;ix<xwidth;ix++){
//            for(int iy2=0;iy2<ywidth;iy2++){
//                for(int ix2=0;ix2<xwidth;ix2++){
//                    if(ix==ix2 && iy==iy2)dis[iy*xwidth+ix][iy2*xwidth+ix2]=0;
//                    else if(abs(ix-ix2)<=1 || abs(iy-iy2)<=1)dis[iy*xwidth+ix][iy2*xwidth+ix2]=costMap[iy2*xwidth+ix2];
//                    else dis[iy*xwidth+ix][iy2*xwidth+ix2]=1000000;
//                }
//            }
//        }
//    }
//    qDebug()<<"tt3";

//    for (int iy = 0; iy < ywidth; iy++) {
//        for(int ix=0;ix<xwidth;ix++){
//            for(int iy2=0;iy2<ywidth;iy2++){
//                for(int ix2=0;ix2<xwidth;ix2++){
//                    for(int iy3=0;iy3<ywidth;iy3++){
//                        for(int ix3=0;ix3<xwidth;ix3++){
//                            int p1=iy*xwidth+ix;
//                            int p2=iy2*xwidth+ix2;
//                            int p3=iy3*xwidth+ix3;
//                            if(dis[p2][p1]+dis[p1][p3]<dis[p2][p3]){
//                                dis[p2][p3]=dis[p2][p1]+dis[p1][p3];
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//    qDebug()<<"tt4";

//    float maxLength=0;
//    for (int iy = 0; iy < ywidth; iy++) {
//        for(int ix=0;ix<xwidth;ix++){
//            for(int iy2=0;iy2<ywidth;iy2++){
//                for(int ix2=0;ix2<xwidth;ix2++){
//                    int p1=iy*xwidth+ix;
//                    int p2=iy2*xwidth+ix2;
//                    if(abs(distanceMap[p1])<0.0001 && abs(distanceMap[p2])<0.0001){
//                        if(maxLength<dis[p1][p2]){
//                            maxLength=dis[p1][p2];
//                        }
//                    }
//                }
//            }
//        }
//    }
//    qDebug()<<"tt5";

//    delete []distanceMap;
//    delete []costMap;
//    for(int i=0;i<xwidth*ywidth;i++){
//        delete []dis[i];
//    }
//    delete[]dis;
//    qDebug()<<"tt6";


//    return maxLength;
//}
void MitoDataset::checkDataEnable(Window *c_window){
    for(int i=0;i<features.length();i++){
        globalEnabled[i]=true;
        enabled[i]=true;
        if(mitoData[i].enabled==false){
            enabled[i]=false;
            globalEnabled[i]=false;
            continue;
        }
        for(int j=0;j<c_window->featureNum;j++){
            float value1=(features[i][j]-c_window->featureRanges[j].x())/(c_window->featureRanges[j].y()-c_window->featureRanges[j].x());
            if(selection[j]){
                if(value1<selectionRange[j].x()-0.03 || value1>selectionRange[j].y()+0.03){
                    enabled[i]=false;
                    globalEnabled[i]=false;
                    break;
                }
            }
            if(c_window->globalSelection[j]){
                if(value1<c_window->globalSelectionRange[j].x()-0.03 || value1>c_window->globalSelectionRange[j].y()+0.03){
                    globalEnabled[i]=false;
                }
            }

            if(value1<axisRange[j].x() || value1>axisRange[j].y()){
                enabled[i]=false;
                globalEnabled[i]=false;
                break;
            }
            if(value1<c_window->globalAxisRange[j].x() || value1>c_window->globalAxisRange[j].y()){
                globalEnabled[i]=false;
            }

        }
        if(plotCoord.length()==features.length()){

            if(c_window->analysis_type->currentIndex()==0){
                if(plotCoord[i].x()<plotXaxisRange.x() || plotCoord[i].x()>plotXaxisRange.y() ||
                        plotCoord[i].y()<plotYaxisRange.x() || plotCoord[i].y()>plotYaxisRange.y() ){
                    enabled[i]=false;
                }
            }
            else{
                if(plotCoord[i].x()<c_window->globalPlotXaxisRange.x() || plotCoord[i].x()>c_window->globalPlotXaxisRange.y() ||
                        plotCoord[i].y()<c_window->globalPlotYaxisRange.x() || plotCoord[i].y()>c_window->globalPlotYaxisRange.y() ){
                    globalEnabled[i]=false;
                }
            }
        }



    }
    for(int i=0;i<c_window->featureNum;i++){
        c_window->minValues[i]=100000;
        c_window->maxValues[i]=-100000;
        c_window->avgValues[i]=0;
        c_window->enabledNum=0;
        enabledNum=0;
        for(int j=0;j<features.length();j++){
            if(enabled[j]){
                if(c_window->minValues[i]>features[j][i])c_window->minValues[i]=features[j][i];
                if(c_window->maxValues[i]<features[j][i])c_window->maxValues[i]=features[j][i];
                c_window->avgValues[i]+=features[j][i];
                c_window->enabledNum++;
            }
            if(globalEnabled[j]){
                enabledNum++;
            }
        }
        if(c_window->enabledNum!=0)
            c_window->avgValues[i]/=c_window->enabledNum;
    }
    c_window->dataRendering->mitoLabelChanged=true;
    c_window->synchronization();
}

void MitoDataset::featureSave(){
    FILE *f=fopen((path+"features.csv").toStdString().c_str(),"w");
    if(f==NULL)return;
    fprintf(f,"No,Structure,Area,Length,Eccentricity,Circularity\n");

    for(int i=0;i<features.length();i++){
        fprintf(f,"%d",i);
        for(int j=0;j<features[i].length();j++){
            fprintf(f,",%f",features[i][j]);
        }
        fprintf(f,"\n");
    }
    fclose(f);
}

void MitoDataset::featureSave_with_path(QString _path){
    FILE *f=fopen(_path.toStdString().c_str(),"w");
    if(f==NULL)return;
    fprintf(f,"No,Structure,Area,Length,Eccentricity,Circularity\n");

    for(int i=0;i<features.length();i++){
        fprintf(f,"%d",i);
        for(int j=0;j<features[i].length();j++){
            fprintf(f,",%f",features[i][j]);
        }
        fprintf(f,"\n");
    }
    fclose(f);
}


void Window::ReSegmentation_structure(){
    if(curDataIndex==-1)return;
    MitoDataset *curData=dataset[curDataIndex];

    unsigned short *tData=new unsigned short[curData->ImageW*curData->ImageH]();
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            tData[tP]=curData->color_table_neuron[curData->imageData1[tP]] * 65535;
        }
    }
    TinyTIFFFile* tif=TinyTIFFWriter_open((curData->path+"temp.tif").toStdString().c_str(), 16,curData->ImageW,curData->ImageH);
    if (tif) {
        TinyTIFFWriter_writeImage(tif, tData);
        TinyTIFFWriter_close(tif);
    }
    else{
        delete [] tData;
        QMessageBox msgBox;
        msgBox.setText("Unknown error...");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();
        return;
    }


    std::string query;
    query="python processing/structure_segmentation.py ";
    query+='"'+(curData->path+"temp.tif").toStdString()+'"';
    query+=" ";
    query+='"'+curData->path.toStdString()+'"';
    query+=" 0";

    qDebug()<<QString(query.c_str());

    system(query.c_str());



    //load structure label
    TinyTIFFReaderFile* tiffr=NULL;
    tiffr=TinyTIFFReader_open((curData->path+"structure_label.tif").toStdString().c_str());
    while(!tiffr){
        tiffr=TinyTIFFReader_open((curData->path+"structure_label.tif").toStdString().c_str());
    }
    if (!tiffr) {
        //qDebug()<<"    ERROR reading (not existent, not accessible or no TIFF file)";
    } else {
        uint32_t width=TinyTIFFReader_getWidth(tiffr);
        uint32_t height=TinyTIFFReader_getHeight(tiffr);
        uint32_t frame=0;
        do {
            bool ok=true;
            if (ok) {
                TinyTIFFReader_getSampleData(tiffr, &curData->structureData[frame*width*height], 0);
                frame++;

            }
        } while (TinyTIFFReader_readNext(tiffr)); // iterate over all frames
    }
    TinyTIFFReader_close(tiffr);
//    labelCleaning();
    mitoControlReleased();

    dataRendering->structureChanged=true;
    dataRendering->update();

}

void Window::ReSegmentation_structure_finetune_ver(){
    if(curDataIndex==-1)return;
    MitoDataset *curData=dataset[curDataIndex];

    TinyTIFFFile* tif=TinyTIFFWriter_open((curData->path+"input.tif").toStdString().c_str(), 8,curData->ImageW,curData->ImageH);
    if (tif) {
        TinyTIFFWriter_writeImage(tif, curData->correctionData);
        TinyTIFFWriter_close(tif);
    }
    else{
        QMessageBox msgBox;
        msgBox.setText("Unknown error...");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();
        return;
    }
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            curData->correctionData[tP]=0;
        }
    }


    unsigned short *tData=new unsigned short[curData->ImageW*curData->ImageH]();
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            tData[tP]=curData->color_table_neuron[curData->imageData1[tP]] * 65535;
        }
    }

    tif=TinyTIFFWriter_open((curData->path+"temp.tif").toStdString().c_str(), 16,curData->ImageW,curData->ImageH);
    if (tif) {
        TinyTIFFWriter_writeImage(tif, tData);
        TinyTIFFWriter_close(tif);
    }
    else{
        delete [] tData;
        QMessageBox msgBox;
        msgBox.setText("Unknown error...");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();
        return;
    }



    std::string query;
    query="python processing/structure_segmentation_fine-tuning_new.py ";
    query+='"'+curData->path.toStdString()+'"';
    query+=" ";
    query+='"'+curData->path.toStdString()+'"';
    query+=" 0";
    query+=" ";
    query+='"'+StructureModel_path.toStdString()+'"';

    qDebug()<<QString(query.c_str());


//    QTime t_time;
//    t_time.start();
    system(query.c_str());


    StructureModel_path="DL_model/temp/latest_structure.pt";


    //load structure label
    TinyTIFFReaderFile* tiffr=NULL;
    tiffr=TinyTIFFReader_open((curData->path+"structure_label_fineTune.tif").toStdString().c_str());
    if (!tiffr) {
        return;
        //qDebug()<<"    ERROR reading (not existent, not accessible or no TIFF file)";
    } else {
        uint32_t width=TinyTIFFReader_getWidth(tiffr);
        uint32_t height=TinyTIFFReader_getHeight(tiffr);
        uint32_t frame=0;
        do {
            bool ok=true;
            if (ok) {
                TinyTIFFReader_getSampleData(tiffr, &curData->structureData[frame*width*height], 0);
                frame++;

            }
        } while (TinyTIFFReader_readNext(tiffr)); // iterate over all frames
    }
    TinyTIFFReader_close(tiffr);

//    labelCleaning();
    mitoControlReleased();

    dataRendering->structureChanged=true;
    dataRendering->update();



    //user study
    /*
    {
    QString t_path=curData->path+"history/"+QString::number((curData->time.elapsed()+curData->prev_elapsed_time)/1000);

    FILE *f=fopen((t_path+"_structure_deployed.txt").toStdString().c_str(),"w");
    if(f==NULL)return;
    fprintf(f,"Time (ms): %d\n",t_time.elapsed());
    fclose(f);

    f=fopen((t_path+"_feature.csv").toStdString().c_str(),"w");
    if(f==NULL)return;
    fprintf(f,"No,Structure,Area,Length,Eccentricity,Circularity,Enabled\n");
    for(int i=0;i<curData->features.length();i++){
        fprintf(f,"%d",i);
        for(int j=0;j<curData->features[i].length();j++){
            fprintf(f,",%f",curData->features[i][j]);
        }
        fprintf(f,",%d",curData->enabled[i]);
        fprintf(f,"\n");
    }
    fclose(f);


    curData->saveStructureLabel(t_path+"_structure_label.tif");

    curData->saveStructureInput(t_path+"_structure_input.tif");
    }
    */



}



void Window::ReSegmentation_mitochondria(){
    if(curDataIndex==-1)return;
    MitoDataset *curData=dataset[curDataIndex];

    unsigned short *tData=new unsigned short[curData->ImageW*curData->ImageH]();
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            tData[tP]=curData->color_table_mito[curData->imageData2[tP]] * 65535;
        }
    }
    TinyTIFFFile* tif=TinyTIFFWriter_open((curData->path+"temp.tif").toStdString().c_str(), 16,curData->ImageW,curData->ImageH);
    if (tif) {
        TinyTIFFWriter_writeImage(tif, tData);
        TinyTIFFWriter_close(tif);
    }
    else{
        delete [] tData;
        QMessageBox msgBox;
        msgBox.setText("Unknown error...");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();
        return;
    }


    std::string query;
    query="python processing/mitochondria_segmentation.py ";
    query+='"'+(curData->path+"temp.tif").toStdString()+'"';
    query+=" ";
    query+='"'+curData->path.toStdString()+'"';
    query+=" 0";

    qDebug()<<QString(query.c_str());

    system(query.c_str());




    //load structure label
    TinyTIFFReaderFile* tiffr=NULL;
    tiffr=TinyTIFFReader_open((curData->path+"mitochondria_label.tif").toStdString().c_str());
    while(!tiffr){
        tiffr=TinyTIFFReader_open((curData->path+"mitochondria_label.tif").toStdString().c_str());
    }
    if (!tiffr) {
        //qDebug()<<"    ERROR reading (not existent, not accessible or no TIFF file)";
    } else {
        uint32_t width=TinyTIFFReader_getWidth(tiffr);
        uint32_t height=TinyTIFFReader_getHeight(tiffr);
        uint32_t frame=0;
        do {
            bool ok=true;
            if (ok) {
                TinyTIFFReader_getSampleData(tiffr, &curData->mitoLabelImage_original[frame*width*height], 0);
                frame++;

            }
        } while (TinyTIFFReader_readNext(tiffr)); // iterate over all frames
    }
    TinyTIFFReader_close(tiffr);

    mitoControlReleased();

    dataRendering->mitoLabelChanged=true;
    dataRendering->update();
}




void Window::ReSegmentation_mitochondria_finetune_ver(){
    if(curDataIndex==-1)return;
    MitoDataset *curData=dataset[curDataIndex];


    TinyTIFFFile* tif=TinyTIFFWriter_open((curData->path+"input.tif").toStdString().c_str(), 8,curData->ImageW,curData->ImageH);
    if (tif) {
        TinyTIFFWriter_writeImage(tif, curData->mitoCorrectionData);
        TinyTIFFWriter_close(tif);
    }
    else{
        QMessageBox msgBox;
        msgBox.setText("Unknown error...");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();
        return;
    }
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            curData->mitoCorrectionData[tP]=0;
        }
    }

    unsigned short *tData=new unsigned short[curData->ImageW*curData->ImageH]();
    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int tP=iy*curData->ImageW+ix;
            tData[tP]=curData->color_table_mito[curData->imageData2[tP]] * 65535;
        }
    }
    tif=TinyTIFFWriter_open((curData->path+"temp.tif").toStdString().c_str(), 16,curData->ImageW,curData->ImageH);
    if (tif) {
        TinyTIFFWriter_writeImage(tif, tData);
        TinyTIFFWriter_close(tif);
    }
    else{
        delete [] tData;
        QMessageBox msgBox;
        msgBox.setText("Unknown error...");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();
        return;
    }


    std::string query;
    query="python processing/mitochondria_segmentation_fine-tuning.py ";
    query+='"'+curData->path.toStdString()+'"';
    query+=" ";
    query+='"'+curData->path.toStdString()+'"';
    query+=" 0";

    qDebug()<<QString(query.c_str());


//    QTime t_time;
//    t_time.start();
    system(query.c_str());




    //load structure label
    TinyTIFFReaderFile* tiffr=NULL;
    tiffr=TinyTIFFReader_open((curData->path+"mitochondria_label_fineTune.tif").toStdString().c_str());
    if (!tiffr) {
        return;
    } else {
        uint32_t width=TinyTIFFReader_getWidth(tiffr);
        uint32_t height=TinyTIFFReader_getHeight(tiffr);
        uint32_t frame=0;
        do {
            bool ok=true;
            if (ok) {
                TinyTIFFReader_getSampleData(tiffr, &curData->mitoLabelImage_label[frame*width*height], 0);
                frame++;

            }
        } while (TinyTIFFReader_readNext(tiffr)); // iterate over all frames
    }
    TinyTIFFReader_close(tiffr);

    mitoControlReleased();

    dataRendering->mitoLabelChanged=true;
    dataRendering->update();



    //user study
    /*
    {
    QString t_path=curData->path+"history/"+QString::number((curData->time.elapsed()+curData->prev_elapsed_time)/1000);

    FILE *f=fopen((t_path+"_mito_deployed.txt").toStdString().c_str(),"w");
    if(f==NULL)return;
    fprintf(f,"Time (ms): %d\n",t_time.elapsed());
    fclose(f);

    f=fopen((t_path+"_feature.csv").toStdString().c_str(),"w");
    if(f==NULL)return;
    fprintf(f,"No,Structure,Area,Length,Eccentricity,Circularity,Enabled\n");
    for(int i=0;i<curData->features.length();i++){
        fprintf(f,"%d",i);
        for(int j=0;j<curData->features[i].length();j++){
            fprintf(f,",%f",curData->features[i][j]);
        }
        fprintf(f,",%d",curData->enabled[i]);
        fprintf(f,"\n");
    }
    fclose(f);


    tif=TinyTIFFWriter_open((t_path+"_mitochondria_object.tif").toStdString().c_str(), 16,curData->ImageW,curData->ImageH);
    if (tif) {
        TinyTIFFWriter_writeImage(tif,curData->mitoLabelImage);
        TinyTIFFWriter_close(tif);
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("Unknown error...");
        msgBox.setStyle(new DarkStyle);
        msgBox.exec();
     }

    curData->saveMitoLabel(t_path+"_mitochondria_input.tif",curData->mitoCorrectionData);
    }
    */



}
void Window::labelCleaning(){
    if(curDataIndex==-1)return;
    MitoDataset *curData=dataset[curDataIndex];
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

            for(int d=0;d<4;d++){
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
        if(cnt<20 && touched){
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

void Window::saveHistory(){
    if(curDataIndex==-1)return;
    MitoDataset *curData=dataset[curDataIndex];

    QString path=curData->path+"history/";

    int et=curData->time.elapsed() + curData->prev_elapsed_time;
    QString elapsedTime=QString::number(et/1000);


    QPixmap t=this->grab(QRect(QPoint(0, 0), QSize(-1, -1)));
    t.save(path+elapsedTime+".jpg");


    FILE *f=fopen((path+elapsedTime+"_param.txt").toStdString().c_str(),"w");
    if(f==NULL)return;
    fprintf(f,"Neuron Channel Bright: %f\n",curData->bright_neuron);
    fprintf(f,"Neuron Channel Contrast: %f\n",curData->contrast_neuron);
    fprintf(f,"Neuron Channel MidPos: %f\n",curData->midpos_neuron);
    fprintf(f,"Mito Channel Bright: %f\n",curData->bright_mito);
    fprintf(f,"Mito Channel Contrast: %f\n",curData->contrast_mito);
    fprintf(f,"Mito Channel MidPos: %f\n",curData->midpos_mito);
    fprintf(f,"Structure Label Opacity: %f\n",curData->structure_opacity);
    fprintf(f,"Mito Label Opacity: %f\n",curData->mitoLabel_opacity);
    fprintf(f,"Mito Object Boundary: %d\n",boundaryOnOff->isChecked());
    fprintf(f,"Analysis Type: %d\n",analysis_type->currentIndex());
    fprintf(f,"Representative Color Setting: %d\n",coloringTypeSet->currentIndex());
    fprintf(f,"X-axis: %d\n",projection1->currentIndex());
    fprintf(f,"Y-axis: %d\n",projection2->currentIndex());
    fclose(f);

    f=fopen((path+elapsedTime+"_feature.csv").toStdString().c_str(),"w");
    if(f==NULL)return;
    fprintf(f,"No,Structure,Area,Length,Eccentricity,Circularity,Enabled\n");
    for(int i=0;i<curData->features.length();i++){
        fprintf(f,"%d",i);
        for(int j=0;j<curData->features[i].length();j++){
            fprintf(f,",%f",curData->features[i][j]);
        }
        fprintf(f,",%d",curData->enabled[i]);
        fprintf(f,"\n");
    }
    fclose(f);



}

void Window::save_dataset_for_training(int data_ind){

    if(data_ind==-1)return;

    MitoDataset *curData=dataset[data_ind];

    QString path=curData->path+"input_dataset/";
    if(!QDir(path).exists()){
        QDir().mkdir(path);
    }


    TinyTIFFFile* dend_user=TinyTIFFWriter_open((path+"dend_user.tif").toStdString().c_str(),32,patchSize,patchSize);
    TinyTIFFFile* axon_user=TinyTIFFWriter_open((path+"axon_user.tif").toStdString().c_str(),32,patchSize,patchSize);
    TinyTIFFFile* dend=TinyTIFFWriter_open((path+"dend.tif").toStdString().c_str(),32,patchSize,patchSize);
    TinyTIFFFile* axon=TinyTIFFWriter_open((path+"axon.tif").toStdString().c_str(),32,patchSize,patchSize);
    TinyTIFFFile* mixed=TinyTIFFWriter_open((path+"mixed.tif").toStdString().c_str(),32,patchSize,patchSize);

    FILE *dend_user_patch=fopen((path+"dend_user_patch.txt").toStdString().c_str(),"w");
    FILE *axon_user_patch=fopen((path+"axon_user_patch.txt").toStdString().c_str(),"w");
    FILE *dend_patch=fopen((path+"dend_patch.txt").toStdString().c_str(),"w");
    FILE *axon_patch=fopen((path+"axon_patch.txt").toStdString().c_str(),"w");
    FILE *mixed_patch=fopen((path+"mixed_patch.txt").toStdString().c_str(),"w");

    FILE *dend_user_ind=fopen((path+"dend_user_neurite.txt").toStdString().c_str(),"w");
    FILE *axon_user_ind=fopen((path+"axon_user_neurite.txt").toStdString().c_str(),"w");
    FILE *dend_ind=fopen((path+"dend_neurite.txt").toStdString().c_str(),"w");
    FILE *axon_ind=fopen((path+"axon_neurite.txt").toStdString().c_str(),"w");
    FILE *mixed_ind=fopen((path+"mixed_neurite.txt").toStdString().c_str(),"w");

    FILE *initial_label=fopen((path+"initial_label.txt").toStdString().c_str(),"w");




    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int curP=iy*curData->ImageW+ix;

            if(curData->tracingMap[curP].temp_ck)continue;

            QQueue<int> res;
            QQueue<int> res2;


            if(curData->tracingMap[curP].type==2 || curData->tracingMap[curP].type==3){
                curData->tracingMap[curP].temp_ck=true;
                res.enqueue(curP);
                res2.enqueue(curP);
            }
            else if(curData->tracingMap[curP].type>0 && curData->tracingMap[curP].connected_nodes.size()>0){
                QQueue<int> nodes;
                nodes.push_back(curP);
                curData->tracingMap[curP].temp_ck=true;

                while(!nodes.isEmpty()){
                    int curP=nodes.dequeue();

                    if(curData->tracingMap[curP].type>0){
                        res.enqueue(curP);
                        res2.enqueue(curP);
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
            int cnt_dend=0;
            int cnt_axon=0;
            int corrected_type=-1;
            int neurite_ind=-1;
            while(!res.isEmpty()){
                int curP=res.dequeue();
                int type=getPatchType(curData->tracingMap[curP].patch,curData);
                if(corrected_type!=-1 && curData->tracingMap[curP].corrected_type!=corrected_type){
                    qDebug()<<"Error on corrected type";
                }
                corrected_type=curData->tracingMap[curP].corrected_type;
                if(neurite_ind!=-1 && curData->tracingMap[curP].neurite_ind!=neurite_ind){
                    qDebug()<<"Error on neurite ind";
                }
                neurite_ind=curData->tracingMap[curP].neurite_ind;

                if(type==2){
                    cnt_dend++;
                }
                if(type==3){
                    cnt_axon++;
                }
            }

            int type=0;
            float thresh=0.5;
            if(cnt_dend+cnt_axon>0){
                if(cnt_dend/(cnt_dend+cnt_axon)>thresh){
                    type=2;
                }
                if(cnt_axon/(cnt_dend+cnt_axon)>thresh){
                    type=3;
                }
            }

            while(!res2.isEmpty()){
                int curP=res2.dequeue();
                if(corrected_type==2 && type==2){
                    TinyTIFFWriter_writeImage(dend_user, curData->tracingMap[curP].patch->patch);
                    fprintf(dend_user_ind,"%d\n",neurite_ind);
                    fprintf(dend_user_patch,"%d\n",curData->tracingMap[curP].patch->index);

                }
                if(corrected_type==3 && type==3){
                    TinyTIFFWriter_writeImage(axon_user, curData->tracingMap[curP].patch->patch);
                    fprintf(axon_user_ind,"%d\n",neurite_ind);
                    fprintf(axon_user_patch,"%d\n",curData->tracingMap[curP].patch->index);

                }
                if(corrected_type==0 && type==2){
                    TinyTIFFWriter_writeImage(dend, curData->tracingMap[curP].patch->patch);
                    fprintf(dend_ind,"%d\n",neurite_ind);
                    fprintf(dend_patch,"%d\n",curData->tracingMap[curP].patch->index);
                }
                if(corrected_type==0 && type==3){
                    TinyTIFFWriter_writeImage(axon, curData->tracingMap[curP].patch->patch);
                    fprintf(axon_ind,"%d\n",neurite_ind);
                    fprintf(axon_patch,"%d\n",curData->tracingMap[curP].patch->index);

                }
                if(corrected_type==0 && type==0){
                    TinyTIFFWriter_writeImage(mixed, curData->tracingMap[curP].patch->patch);
                    fprintf(mixed_ind,"%d\n",neurite_ind);
                    fprintf(mixed_patch,"%d\n",curData->tracingMap[curP].patch->index);
                }
                //fprintf(gt,"%d %d\n",curData->tracingMap[curP].patch->index,type);

            }
            if(neurite_ind!=-1){
                fprintf(initial_label,"%d %d\n",neurite_ind,type);
            }
        }
    }

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int curP=iy*curData->ImageW+ix;
            curData->tracingMap[curP].temp_ck=false;
        }
    }


    fclose(dend_user_patch);
    fclose(axon_user_patch);
    fclose(dend_patch);
    fclose(axon_patch);
    fclose(mixed_patch);
    fclose(initial_label);

    fclose(dend_user_ind);
    fclose(axon_user_ind);
    fclose(dend_ind);
    fclose(axon_ind);
    fclose(mixed_ind);


    TinyTIFFWriter_close(dend_user);
    TinyTIFFWriter_close(axon_user);
    TinyTIFFWriter_close(dend);
    TinyTIFFWriter_close(axon);
    TinyTIFFWriter_close(mixed);


    QString absolute_path=QDir(".").absolutePath();
    QString relative_path=QDir(absolute_path).relativeFilePath(curData->path);


#ifdef HJ_SERVER_ver
    std::string query;
    query="wsl \"/mnt/c/Users/hjoh/excutable_220329/pretrain.sh\" ";
    query+='"'+relative_path.toStdString()+'/'+'"';
    qDebug()<<QString(query.c_str());
#else

//    std::string query;
//    query="wsl \"/mnt/d/MitoVis/build/pretrain.sh\" ";
//    query+='"'+relative_path.toStdString()+'/'+'"';
//    qDebug()<<QString(query.c_str());


    std::string query;
    query="python VIS_processing/pretrain_pred.py ";
    query+='"'+curData->path.toStdString()+'"';

    qDebug()<<QString(query.c_str());
#endif

    system(query.c_str());


    QString label_path = curData->path+"output_dataset/";


    load_pred_datatset(label_path,data_ind,0);
//    dataset[data_ind]->finetune_iter++;

//    QMessageBox msgBox;
//    msgBox.setText("Preprocessing finished!");
//    msgBox.setStyle(new DarkStyle);
//    msgBox.exec();

}



void Window::save_dataset_for_training_all(){

    if(curDataIndex==-1)return;

    int target_group=dataset[curDataIndex]->group;


    QString path="models/";
    if(!QDir(path).exists()){
        QDir().mkdir(path);
    }
    QString base_path=path+groups[dataset[curDataIndex]->group]->name->text();
    if(!QDir(base_path).exists()){
        QDir().mkdir(base_path);
    }
    path=base_path+"/input_dataset/";
    if(!QDir(path).exists()){
        QDir().mkdir(path);
    }


    TinyTIFFFile* dend_user=TinyTIFFWriter_open((path+"dend_user.tif").toStdString().c_str(),32,patchSize,patchSize);
    TinyTIFFFile* axon_user=TinyTIFFWriter_open((path+"axon_user.tif").toStdString().c_str(),32,patchSize,patchSize);
    TinyTIFFFile* dend=TinyTIFFWriter_open((path+"dend.tif").toStdString().c_str(),32,patchSize,patchSize);
    TinyTIFFFile* axon=TinyTIFFWriter_open((path+"axon.tif").toStdString().c_str(),32,patchSize,patchSize);
    TinyTIFFFile* mixed=TinyTIFFWriter_open((path+"mixed.tif").toStdString().c_str(),32,patchSize,patchSize);

    FILE *dend_user_patch=fopen((path+"dend_user_patch.txt").toStdString().c_str(),"w");
    FILE *axon_user_patch=fopen((path+"axon_user_patch.txt").toStdString().c_str(),"w");
    FILE *dend_patch=fopen((path+"dend_patch.txt").toStdString().c_str(),"w");
    FILE *axon_patch=fopen((path+"axon_patch.txt").toStdString().c_str(),"w");
    FILE *mixed_patch=fopen((path+"mixed_patch.txt").toStdString().c_str(),"w");

    FILE *dend_user_ind=fopen((path+"dend_user_neurite.txt").toStdString().c_str(),"w");
    FILE *axon_user_ind=fopen((path+"axon_user_neurite.txt").toStdString().c_str(),"w");
    FILE *dend_ind=fopen((path+"dend_neurite.txt").toStdString().c_str(),"w");
    FILE *axon_ind=fopen((path+"axon_neurite.txt").toStdString().c_str(),"w");
    FILE *mixed_ind=fopen((path+"mixed_neurite.txt").toStdString().c_str(),"w");

    FILE *initial_label=fopen((path+"initial_label.txt").toStdString().c_str(),"w");



    for(int data_ind=0;data_ind<dataset.size();data_ind++){
        if(dataset[data_ind]->group!=target_group){
            continue;
        }
        MitoDataset *curData=dataset[data_ind];
        for(int iy=0;iy<curData->ImageH;iy++){
            for(int ix=0;ix<curData->ImageW;ix++){
                int curP=iy*curData->ImageW+ix;

                if(curData->tracingMap[curP].temp_ck)continue;

                QQueue<int> res;
                QQueue<int> res2;


                if(curData->tracingMap[curP].type==2 || curData->tracingMap[curP].type==3){
                    curData->tracingMap[curP].temp_ck=true;
                    res.enqueue(curP);
                    res2.enqueue(curP);
                }
                else if(curData->tracingMap[curP].type>0 && curData->tracingMap[curP].connected_nodes.size()>0){
                    QQueue<int> nodes;
                    nodes.push_back(curP);
                    curData->tracingMap[curP].temp_ck=true;

                    while(!nodes.isEmpty()){
                        int curP=nodes.dequeue();

                        if(curData->tracingMap[curP].type>0){
                            res.enqueue(curP);
                            res2.enqueue(curP);
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
                int cnt_dend=0;
                int cnt_axon=0;
                int corrected_type=-1;
                int neurite_ind=-1;
                while(!res.isEmpty()){
                    int curP=res.dequeue();
                    int type=getPatchType(curData->tracingMap[curP].patch,curData);
                    if(corrected_type!=-1 && curData->tracingMap[curP].corrected_type!=corrected_type){
                        qDebug()<<"Error on corrected type";
                    }
                    corrected_type=curData->tracingMap[curP].corrected_type;
                    if(neurite_ind!=-1 && curData->tracingMap[curP].neurite_ind!=neurite_ind){
                        qDebug()<<"Error on neurite ind";
                    }
                    neurite_ind=curData->tracingMap[curP].neurite_ind;

                    if(type==2){
                        cnt_dend++;
                    }
                    if(type==3){
                        cnt_axon++;
                    }
                }

                int type=0;
                float thresh=0.5;
                if(cnt_dend+cnt_axon>0){
                    if(cnt_dend/(cnt_dend+cnt_axon)>thresh){
                        type=2;
                    }
                    if(cnt_axon/(cnt_dend+cnt_axon)>thresh){
                        type=3;
                    }
                }

                while(!res2.isEmpty()){
                    int curP=res2.dequeue();
                    if(corrected_type==2 && type==2){
                        TinyTIFFWriter_writeImage(dend_user, curData->tracingMap[curP].patch->patch);
                        fprintf(dend_user_ind,"%d\n",neurite_ind);
                        fprintf(dend_user_patch,"%d\n",curData->tracingMap[curP].patch->index);

                    }
                    if(corrected_type==3 && type==3){
                        TinyTIFFWriter_writeImage(axon_user, curData->tracingMap[curP].patch->patch);
                        fprintf(axon_user_ind,"%d\n",neurite_ind);
                        fprintf(axon_user_patch,"%d\n",curData->tracingMap[curP].patch->index);

                    }
                    if(corrected_type==0 && type==2){
                        TinyTIFFWriter_writeImage(dend, curData->tracingMap[curP].patch->patch);
                        fprintf(dend_ind,"%d\n",neurite_ind);
                        fprintf(dend_patch,"%d\n",curData->tracingMap[curP].patch->index);
                    }
                    if(corrected_type==0 && type==3){
                        TinyTIFFWriter_writeImage(axon, curData->tracingMap[curP].patch->patch);
                        fprintf(axon_ind,"%d\n",neurite_ind);
                        fprintf(axon_patch,"%d\n",curData->tracingMap[curP].patch->index);

                    }
                    if(corrected_type==0 && type==0){
                        TinyTIFFWriter_writeImage(mixed, curData->tracingMap[curP].patch->patch);
                        fprintf(mixed_ind,"%d\n",neurite_ind);
                        fprintf(mixed_patch,"%d\n",curData->tracingMap[curP].patch->index);
                    }
                    //fprintf(gt,"%d %d\n",curData->tracingMap[curP].patch->index,type);

                }
                if(neurite_ind!=-1){
                    fprintf(initial_label,"%d %d\n",neurite_ind,type);
                }
            }
        }

        for(int iy=0;iy<curData->ImageH;iy++){
            for(int ix=0;ix<curData->ImageW;ix++){
                int curP=iy*curData->ImageW+ix;
                curData->tracingMap[curP].temp_ck=false;
            }
        }
    }



    fclose(dend_user_patch);
    fclose(axon_user_patch);
    fclose(dend_patch);
    fclose(axon_patch);
    fclose(mixed_patch);
    fclose(initial_label);

    fclose(dend_user_ind);
    fclose(axon_user_ind);
    fclose(dend_ind);
    fclose(axon_ind);
    fclose(mixed_ind);


    TinyTIFFWriter_close(dend_user);
    TinyTIFFWriter_close(axon_user);
    TinyTIFFWriter_close(dend);
    TinyTIFFWriter_close(axon);
    TinyTIFFWriter_close(mixed);




}
int Window::getPatchType(patchSet *patch,MitoDataset *curData){
    if(patch==NULL)return 0;

    int tW=patch->boundMax.x()-patch->boundMin.x()+1;
    int tH=patch->boundMax.y()-patch->boundMin.y()+1;

    bool *ck=new bool[tW*tH]();
    int dxylist[4][2]={{-1,0},{1,0},{0,-1},{0,1}};

    QQueue<int> quex,quey;
    quex.enqueue(patch->midx);
    quey.enqueue(patch->midy);
    ck[int((patch->midy-patch->boundMin.y())*tW+patch->midx-patch->boundMin.x())]=true;

    int axon_count=0;
    int dend_count=0;
    while(!quex.empty()){
        int curx=quex.dequeue();
        int cury=quey.dequeue();
        if(curData->structureData[cury*curData->ImageW + curx]==2){
            dend_count++;
        }
        else{
            axon_count++;
        }

        for(int d=0;d<4;d++){
            int nextx=curx+dxylist[d][0];
            int nexty=cury+dxylist[d][1];
            if(nextx<patch->boundMin.x() ||nextx>patch->boundMax.x()
                    || nexty<patch->boundMin.y() || nexty>patch->boundMax.y()){
                continue;
            }
            if(curData->corrector->isInside(QVector2D(nextx,nexty),patch->patchCorner)==false)continue;

            int tP=nexty*curData->ImageW + nextx;
            int tP2=(nexty-patch->boundMin.y())*tW+nextx-patch->boundMin.x();
            if(ck[tP2]==false){
                ck[tP2]=true;
                if(curData->structureData[tP]==2 || curData->structureData[tP]==3){
                    quex.enqueue(nextx);
                    quey.enqueue(nexty);
                }
            }
        }
    }
    delete []ck;

    if(dend_count>=axon_count)return 2;
    return 3;
}
void Window::hideAllRadars(int ind){
    if(ind!=-1){
        dataset[ind]->focus_fixed_point=-1;
        for(int i=0;i<dataset[ind]->radars.size();i++){
            dataset[ind]->radars[i]->is_focused=false;
            dataset[ind]->radars[i]->is_focus_fixed=false;
            dataset[ind]->radars[i]->hide();
            dataset[ind]->radars[i]->update();
        }
    }
    ind=curDataIndex;

}

void Window::load_pred_datatset(QString label_path,int data_ind,int iter_n){
    if(data_ind==-1)return;

    MitoDataset * curData=dataset[data_ind];



//    QString iter="01/";
    QString iter;
    if(iter_n==0)iter="";
    else if(iter_n<10)iter="0"+QString::number(iter_n)+"/";
    else iter=QString::number(iter_n)+"/";



    int t;
    float prob_dend,prob_axon;
    int neu;

    FILE *sim_mat=fopen((label_path+"/feature/"+iter+"feature_matrix.txt").toStdString().c_str(),"r");
    FILE *mat_neurite_ind=fopen((label_path+"/feature/"+iter+"neurite_idx.txt").toStdString().c_str(),"r");
    qDebug()<<(label_path+"/feature/"+iter+"neurite_idx.txt");
    qDebug()<<(label_path+"/feature/"+iter+"feature_matrix.txt");
    if(mat_neurite_ind!=NULL && sim_mat!=NULL){
        qDebug()<<"success";

        curData->similarity_matrix_neu_ind.clear();
        while(EOF!=fscanf(mat_neurite_ind,"%d",&neu)){
            curData->similarity_matrix_neu_ind.push_back(neu);
            curData->neurite_index_map[neu]=curData->similarity_matrix_neu_ind.size()-1;
        }
        curData->similarity_matrix_width=curData->similarity_matrix_neu_ind.size();

        if(curData->similarity_matrix!=NULL){
            delete []curData->similarity_matrix;
        }
        curData->similarity_matrix=new float[curData->similarity_matrix_width*curData->similarity_matrix_width];
        for(int i=0;i<curData->similarity_matrix_width;i++){
            for(int j=0;j<curData->similarity_matrix_width;j++){
                float v;
                fscanf(sim_mat,"%f",&v);
                curData->similarity_matrix[i*curData->similarity_matrix_width+j]=v;
            }
        }
        fclose(sim_mat);
        fclose(mat_neurite_ind);

    }



//    FILE *pred_res2=fopen((label_path+"/pred/pred2.txt").toStdString().c_str(),"w"); // neurite_ind patch_ind label prob1 prob2


    bool is_ok=false;
    float thres_v=uncertainty_thres->text().toFloat(&is_ok);
    if(is_ok==false)thres_v=0.7;

    FILE *pred_res=fopen((label_path+"/pred/"+iter+"pred.txt").toStdString().c_str(),"r");
    qDebug()<<(label_path+"/pred/"+iter+"pred.txt");

    int patch_ind;
    int neu_ind;
    if(pred_res!=NULL){
        qDebug()<<"success";
        while(EOF!=fscanf(pred_res,"%d",&neu_ind)){
            fscanf(pred_res,"%d %d %f %f",&patch_ind,&t,&prob_dend,&prob_axon);
            if(t==0)curData->patch_res[patch_ind]=3; //2: dend, 3: axon
            if(t==1)curData->patch_res[patch_ind]=2; //2: dend, 3: axon
            if(prob_dend<thres_v && prob_dend>1.0-thres_v)curData->patch_res[patch_ind]=4; // 4: uncertain
            curData->patch_probability_dend[patch_ind]=prob_dend;

    //        fprintf(pred_res2,"%d %d %f %f\n",curData->patch_neurite_map[patch_ind],patch_ind,t,prob_dend,prob_axon);

        }
        fclose(pred_res);
        for(int iy=0;iy<curData->ImageH;iy++){
            for(int ix=0;ix<curData->ImageW;ix++){
                int curP=iy*curData->ImageW+ix;
                if(curData->tracingMap[curP].neurite_ind!=-1){
                    if(curData->patch_res.contains(curData->tracingMap[curP].patch->index)){
                        patchCorrection(curP,curData->patch_res[curData->tracingMap[curP].patch->index],curData);
                    }
                }
            }
        }

    }

//    fclose(pred_res2);



    FILE *feature_axon=fopen((label_path+"/feature/"+iter+"feature_axon.txt").toStdString().c_str(),"r");
    if(feature_axon!=NULL){
        while(EOF!=fscanf(feature_axon,"%d",&neu)){
            QVector <float> feature;
            for(int i=0;i<128;i++){
                float f;
                fscanf(feature_axon,"%f",&f);
                feature.push_back(f);
            }
            curData->neurite_feature[neu]=feature;

            if(curData->neurite_res[neu]!=3){
                curData->neurite_changed[neu]=true;
            }
        }
        fclose(feature_axon);

    }

    FILE *feature_axon_user=fopen((label_path+"/feature/"+iter+"feature_axon_user.txt").toStdString().c_str(),"r");
    if(feature_axon_user!=NULL){

        while(EOF!=fscanf(feature_axon_user,"%d",&neu)){

            curData->neurite_res[neu]=3;

            QVector <float> feature;
            for(int i=0;i<128;i++){
                float f;
                fscanf(feature_axon_user,"%f",&f);
                feature.push_back(f);
            }
            curData->neurite_feature[neu]=feature;
            curData->neurite_changed[neu]=false;
        }
        fclose(feature_axon_user);

    }

    FILE *feature_dend=fopen((label_path+"/feature/"+iter+"feature_dend.txt").toStdString().c_str(),"r");
    if(feature_dend!=NULL){

        while(EOF!=fscanf(feature_dend,"%d",&neu)){
            QVector <float> feature;
            for(int i=0;i<128;i++){
                float f;
                fscanf(feature_dend,"%f",&f);
                feature.push_back(f);
            }
            curData->neurite_feature[neu]=feature;
            if(curData->neurite_res[neu]!=2){
                curData->neurite_changed[neu]=true;
            }

        }
        fclose(feature_dend);

    }


    FILE *feature_dend_user=fopen((label_path+"/feature/"+iter+"feature_dend_user.txt").toStdString().c_str(),"r");
    if(feature_dend_user!=NULL){
        while(EOF!=fscanf(feature_dend_user,"%d",&neu)){

            curData->neurite_res[neu]=2;

            QVector <float> feature;
            for(int i=0;i<128;i++){
                float f;
                fscanf(feature_dend_user,"%f",&f);
                feature.push_back(f);
            }
            curData->neurite_feature[neu]=feature;
            curData->neurite_changed[neu]=false;
        }
        fclose(feature_dend_user);

    }

    FILE *feature_mixed=fopen((label_path+"/feature/"+iter+"feature_mixed.txt").toStdString().c_str(),"r");
    if(feature_mixed!=NULL){
        while(EOF!=fscanf(feature_mixed,"%d",&neu)){

            QVector <float> feature;
            for(int i=0;i<128;i++){
                float f;
                fscanf(feature_mixed,"%f",&f);
                feature.push_back(f);
            }
            curData->neurite_feature[neu]=feature;
            curData->neurite_changed[neu]=true;
        }
        fclose(feature_mixed);

    }




    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int curP=iy*curData->ImageW+ix;

            if(curData->tracingMap[curP].temp_ck)continue;

            QQueue<int> res;
            QQueue<int> res2;

            if(curData->tracingMap[curP].type==2 || curData->tracingMap[curP].type==3){
                curData->tracingMap[curP].temp_ck=true;
                res.enqueue(curP);
                res2.enqueue(curP);
            }
            else if(curData->tracingMap[curP].type>0){
                QQueue<int> nodes;
                nodes.push_back(curP);
                curData->tracingMap[curP].temp_ck=true;

                while(!nodes.isEmpty()){
                    int curP=nodes.dequeue();

                    if(curData->tracingMap[curP].type>0){
                        res.enqueue(curP);
                        res2.enqueue(curP);
                        for(int i=0;i<curData->tracingMap[curP].connected_nodes.size();i++){
                            if(curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].temp_ck==false
                                    && curData->tracingMap[curP].type!=2 && curData->tracingMap[curP].type!=3){
                                curData->tracingMap[curData->tracingMap[curP].connected_nodes[i]].temp_ck=true;
                                nodes.enqueue(curData->tracingMap[curP].connected_nodes[i]);
                            }
                        }
                    }
                }
            }
            int cnt_dend=0;
            int cnt_axon=0;
            while(!res.isEmpty()){
                int curP=res.dequeue();
                if(curData->patch_res[curData->tracingMap[curP].patch->index]==4)continue;
                int type=getPatchType(curData->tracingMap[curP].patch,curData);
                if(type==2){
                    cnt_dend++;
                }
                if(type==3){
                    cnt_axon++;
                }
            }

            int type=0;
            if(cnt_dend>cnt_axon){
                type=2;
            }
            if(cnt_dend<cnt_axon){
                type=3;
            }
            curData->neurite_res[curData->tracingMap[curP].neurite_ind]=0;
            curData->neurite_probability_dend[curData->tracingMap[curP].neurite_ind]=0.5;
            if(type!=0){
                float probability_mean=0;
                int cnt=0;
//                curData->neurite_probability_dend[curData->tracingMap[curP].neurite_ind]=float(cnt_dend)/(cnt_dend+cnt_axon);

                curData->neurite_res[curData->tracingMap[curP].neurite_ind]=type;
                while(!res2.isEmpty()){
                    int curP=res2.dequeue();
                    if(curData->patch_res[curData->tracingMap[curP].patch->index]!=4){
                        probability_mean+=curData->patch_probability_dend[curData->tracingMap[curP].patch->index];
                        cnt++;
                    }
                    patchCorrection(curP,type,curData);
                }
                curData->neurite_probability_dend[curData->tracingMap[curP].neurite_ind]=probability_mean/cnt;

            }
        }
    }

    for(int iy=0;iy<curData->ImageH;iy++){
        for(int ix=0;ix<curData->ImageW;ix++){
            int curP=iy*curData->ImageW+ix;
            curData->tracingMap[curP].temp_ck=false;
        }
    }





    mitoControlReleased();

    dataRendering->structureChanged=true;
    dataRendering->update();


}


void Window::finetune_structure_segmentation_vis(){

    #ifdef VIS22
        VIS22_userstudy_save(dataset[curDataIndex]->finetune_iter*2+0);
    #endif


    save_dataset_for_training_all();


    QString path="models/";
    QString base_path=path+groups[dataset[curDataIndex]->group]->name->text();
    path=base_path+"/input_dataset/";


    QString absolute_path=QDir(".").absolutePath();
    QString relative_path=QDir(absolute_path).relativeFilePath(base_path);


#ifdef HJ_SERVER_ver
    std::string query;
    query="wsl \"/mnt/c/Users/hjoh/excutable_220329/finetune.sh\" ";
    query+='"'+relative_path.toStdString()+'/'+'"'+' ';
    query+='"'+QString::number(dataset[curDataIndex]->finetune_iter).toStdString()+'"';
    qDebug()<<QString(query.c_str());
#else

//    std::string query;
//    query="wsl \"/mnt/d/MitoVis/build/finetune.sh\" ";
//    query+='"'+relative_path.toStdString()+'/'+'"'+' ';
//    query+='"'+QString::number(dataset[curDataIndex]->finetune_iter).toStdString()+'"';
//    qDebug()<<QString(query.c_str());


    std::string query;
    query="python VIS_processing/finetune.py ";
    query+='"'+base_path.toStdString()+'"';
    query+=" "+QString::number(dataset[curDataIndex]->finetune_iter).toStdString();

    qDebug()<<QString(query.c_str());
#endif

    system(query.c_str());


    int target_group=dataset[curDataIndex]->group;

    for(int data_ind=0;data_ind<dataset.size();data_ind++){
        if(dataset[data_ind]->group!=target_group){
            continue;
        }
        load_pred_datatset(base_path+"/output_dataset/",data_ind,dataset[data_ind]->finetune_iter);
        dataset[data_ind]->finetune_iter++;
    }

#ifdef VIS22
    VIS22_userstudy_save((dataset[curDataIndex]->finetune_iter-1)*2+1);
#endif


//    QMessageBox msgBox;
//    msgBox.setText("Finetuning finished!");
//    msgBox.setStyle(new DarkStyle);
//    msgBox.exec();
}


void Window::resolution_changed(QString v){
    if(curDataIndex==-1)return;
    bool is_ok=false;
    float vv=v.toFloat(&is_ok);
    if(is_ok){
        dataset[curDataIndex]->resolution=vv;
        mitoControlReleased();
    }
}


void Window::VIS22_userstudy_save(int user_iter){

    int target_group=dataset[curDataIndex]->group;

    int timestamp=VIS22_time.elapsed()/1000;


    QString path="models/";
    if(!QDir(path).exists()){
        QDir().mkdir(path);
    }
    QString base_path=path+groups[dataset[curDataIndex]->group]->name->text();
    if(!QDir(base_path).exists()){
        QDir().mkdir(base_path);
    }

    QString iter;
    if(user_iter<10)iter="0"+QString::number(user_iter)+"_"+QString::number(timestamp)+"/";
    else iter=QString::number(user_iter)+"/";

    path=base_path+"/userstudy_data/";
    if(!QDir(path).exists()){
        QDir().mkdir(path);
    }

    path=path+iter;

    if(!QDir(path).exists()){
        QDir().mkdir(path);
    }


    TinyTIFFFile* dend_user=TinyTIFFWriter_open((path+"dend_user.tif").toStdString().c_str(),32,patchSize,patchSize);
    TinyTIFFFile* axon_user=TinyTIFFWriter_open((path+"axon_user.tif").toStdString().c_str(),32,patchSize,patchSize);
    TinyTIFFFile* dend=TinyTIFFWriter_open((path+"dend.tif").toStdString().c_str(),32,patchSize,patchSize);
    TinyTIFFFile* axon=TinyTIFFWriter_open((path+"axon.tif").toStdString().c_str(),32,patchSize,patchSize);
    TinyTIFFFile* mixed=TinyTIFFWriter_open((path+"mixed.tif").toStdString().c_str(),32,patchSize,patchSize);

    FILE *dend_user_patch=fopen((path+"dend_user_patch.txt").toStdString().c_str(),"w");
    FILE *axon_user_patch=fopen((path+"axon_user_patch.txt").toStdString().c_str(),"w");
    FILE *dend_patch=fopen((path+"dend_patch.txt").toStdString().c_str(),"w");
    FILE *axon_patch=fopen((path+"axon_patch.txt").toStdString().c_str(),"w");
    FILE *mixed_patch=fopen((path+"mixed_patch.txt").toStdString().c_str(),"w");

    FILE *dend_user_ind=fopen((path+"dend_user_neurite.txt").toStdString().c_str(),"w");
    FILE *axon_user_ind=fopen((path+"axon_user_neurite.txt").toStdString().c_str(),"w");
    FILE *dend_ind=fopen((path+"dend_neurite.txt").toStdString().c_str(),"w");
    FILE *axon_ind=fopen((path+"axon_neurite.txt").toStdString().c_str(),"w");
    FILE *mixed_ind=fopen((path+"mixed_neurite.txt").toStdString().c_str(),"w");

    FILE *initial_label=fopen((path+"initial_label.txt").toStdString().c_str(),"w");



    for(int data_ind=0;data_ind<dataset.size();data_ind++){
        if(dataset[data_ind]->group!=target_group){
            continue;
        }
        MitoDataset *curData=dataset[data_ind];
        for(int iy=0;iy<curData->ImageH;iy++){
            for(int ix=0;ix<curData->ImageW;ix++){
                int curP=iy*curData->ImageW+ix;

                if(curData->tracingMap[curP].temp_ck)continue;

                QQueue<int> res;
                QQueue<int> res2;


                if(curData->tracingMap[curP].type==2 || curData->tracingMap[curP].type==3){
                    curData->tracingMap[curP].temp_ck=true;
                    res.enqueue(curP);
                    res2.enqueue(curP);
                }
                else if(curData->tracingMap[curP].type>0 && curData->tracingMap[curP].connected_nodes.size()>0){
                    QQueue<int> nodes;
                    nodes.push_back(curP);
                    curData->tracingMap[curP].temp_ck=true;

                    while(!nodes.isEmpty()){
                        int curP=nodes.dequeue();

                        if(curData->tracingMap[curP].type>0){
                            res.enqueue(curP);
                            res2.enqueue(curP);
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
                int cnt_dend=0;
                int cnt_axon=0;
                int corrected_type=-1;
                int neurite_ind=-1;
                while(!res.isEmpty()){
                    int curP=res.dequeue();
                    int type=getPatchType(curData->tracingMap[curP].patch,curData);
                    if(corrected_type!=-1 && curData->tracingMap[curP].corrected_type!=corrected_type){
                        qDebug()<<"Error on corrected type";
                    }
                    corrected_type=curData->tracingMap[curP].corrected_type;
                    if(neurite_ind!=-1 && curData->tracingMap[curP].neurite_ind!=neurite_ind){
                        qDebug()<<"Error on neurite ind";
                    }
                    neurite_ind=curData->tracingMap[curP].neurite_ind;

                    if(type==2){
                        cnt_dend++;
                    }
                    if(type==3){
                        cnt_axon++;
                    }
                }

                int type=0;
                float thresh=0.5;
                if(cnt_dend+cnt_axon>0){
                    if(cnt_dend/(cnt_dend+cnt_axon)>thresh){
                        type=2;
                    }
                    if(cnt_axon/(cnt_dend+cnt_axon)>thresh){
                        type=3;
                    }
                }

                while(!res2.isEmpty()){
                    int curP=res2.dequeue();
                    if(corrected_type==2 && type==2){
                        TinyTIFFWriter_writeImage(dend_user, curData->tracingMap[curP].patch->patch);
                        fprintf(dend_user_ind,"%d\n",neurite_ind);
                        fprintf(dend_user_patch,"%d\n",curData->tracingMap[curP].patch->index);

                    }
                    if(corrected_type==3 && type==3){
                        TinyTIFFWriter_writeImage(axon_user, curData->tracingMap[curP].patch->patch);
                        fprintf(axon_user_ind,"%d\n",neurite_ind);
                        fprintf(axon_user_patch,"%d\n",curData->tracingMap[curP].patch->index);

                    }
                    if(corrected_type==0 && type==2){
                        TinyTIFFWriter_writeImage(dend, curData->tracingMap[curP].patch->patch);
                        fprintf(dend_ind,"%d\n",neurite_ind);
                        fprintf(dend_patch,"%d\n",curData->tracingMap[curP].patch->index);
                    }
                    if(corrected_type==0 && type==3){
                        TinyTIFFWriter_writeImage(axon, curData->tracingMap[curP].patch->patch);
                        fprintf(axon_ind,"%d\n",neurite_ind);
                        fprintf(axon_patch,"%d\n",curData->tracingMap[curP].patch->index);

                    }
                    if(corrected_type==0 && type==0){
                        TinyTIFFWriter_writeImage(mixed, curData->tracingMap[curP].patch->patch);
                        fprintf(mixed_ind,"%d\n",neurite_ind);
                        fprintf(mixed_patch,"%d\n",curData->tracingMap[curP].patch->index);
                    }
                    //fprintf(gt,"%d %d\n",curData->tracingMap[curP].patch->index,type);

                }
                if(neurite_ind!=-1){
                    fprintf(initial_label,"%d %d\n",neurite_ind,type);
                }
            }
        }

        for(int iy=0;iy<curData->ImageH;iy++){
            for(int ix=0;ix<curData->ImageW;ix++){
                int curP=iy*curData->ImageW+ix;
                curData->tracingMap[curP].temp_ck=false;
            }
        }
    }



    fclose(dend_user_patch);
    fclose(axon_user_patch);
    fclose(dend_patch);
    fclose(axon_patch);
    fclose(mixed_patch);
    fclose(initial_label);

    fclose(dend_user_ind);
    fclose(axon_user_ind);
    fclose(dend_ind);
    fclose(axon_ind);
    fclose(mixed_ind);


    TinyTIFFWriter_close(dend_user);
    TinyTIFFWriter_close(axon_user);
    TinyTIFFWriter_close(dend);
    TinyTIFFWriter_close(axon);
    TinyTIFFWriter_close(mixed);

    for(int data_ind=0;data_ind<dataset.size();data_ind++){
        if(dataset[data_ind]->group!=target_group){
            continue;
        }
        MitoDataset *curData=dataset[data_ind];
        curData->saveStructureLabel(path+curData->dir+"_StructureLabel.tif");
        curData->saveStructureInput(path+curData->dir+"_StructureInput.tif");

        unsigned char *tData=new unsigned char[curData->ImageW * curData->ImageH]();
        for(int iy=0;iy<curData->ImageH;iy++){
            for(int ix=0;ix<curData->ImageW;ix++){
                int tP=iy*curData->ImageW + ix;
                if(curData->mitoCorrectionData[tP]!=10 &&curData->mitoCorrectionData[tP]!=11){
                    if(curData->mitoLabelImage_label[tP]==1 || curData->mitoCorrectionData[tP]==12 || curData->mitoCorrectionData[tP]==13){
                        tData[tP]=1;
                    }
                }

            }
        }
        curData->saveMitoLabel(path+curData->dir+"_MitoLabel.tif",tData);
        delete []tData;
        curData->saveMitoInput(path+curData->dir+"_MitoInput.tif");
        curData->featureSave_with_path(path+curData->dir+"_feature.csv");


    }




}
