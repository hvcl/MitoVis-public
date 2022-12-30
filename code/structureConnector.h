#pragma once

#ifndef STRUCTURECONNECTOR_H
#define STRUCTURECONNECTOR_H


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
#include <QVector>
#include <queue>


class structureConnector :public QObject
{
    Q_OBJECT

public:
    structureConnector(MitoDataset *p, QWidget *parent = 0);
    ~structureConnector();
    void init_graph();
    void connected_component(int startx, int starty, int ind);
    void find_paths(int start_object,float distance_thresh,float background_thresh);
    float node_cost(float v);
    void update_path_map();
public slots:
    void run();
    void object_indexing();
    void classification();
    void updateWorkList();

signals:
    void connection_graph_updated();

protected:

public:
    MitoDataset *curData;
    nodeInfo *D;
    float *distances;
    QVector <int> **paths;

    int max_object_index=0;
    bool finish_signal=false;

    int cur_work_index=1;
    bool cur_work_update_signal=false;
    int ind=1;
};

#endif // STRUCTURECONNECTOR_H
