
#pragma once
#ifndef PATCHRECOMMENDER_H
#define PATCHRECOMMENDER_H


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

class patchRecommender :public QObject
{
    Q_OBJECT

public:
    patchRecommender(Window *p, QWidget *parent = 0);
    ~patchRecommender();

    Window *c_window;



public slots:
    void getCandidatePoints();
    void run(int type);
    void init();

signals:
    void addNewPatch(QVector2D startPos,QVector2D endPos);

protected:
//    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

public:
    QVector<QVector2D> candidateStartPos_dend;
    QVector<QVector2D> candidateEndPos_dend;
    QVector<QVector2D> candidateStartPos_axon;
    QVector<QVector2D> candidateEndPos_axon;

    MitoDataset *curData;

};


#endif // PATCHRECOMMENDER_H
