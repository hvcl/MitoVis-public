///////////////////////////////////////////////////////////
//JunYoung Choi
//juny0603@gmail.com
//Ulsan National Institute of Science and Technology(UNIST)
///////////////////////////////////////////////////////////

#include <QtWidgets>

#include "mainwindow.h"
#include "window.h"
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QDockWidget>
#include <seedSelectionWindow.h>
#include <fstream>
MainWindow::MainWindow()
{
    setAttribute(Qt::WA_DeleteOnClose);
    createActions();
    createMenus();
    (void)statusBar();
//    setWindowFilePath(QString());

    c_window=new Window(this);
    QPalette pal = palette();

    // set black background
    pal.setColor(QPalette::Background, QColor(255,255,255));
    c_window->setAutoFillBackground(true);
    c_window->setPalette(pal);
    setCentralWidget(c_window);
    this->resize(1600,800);
}


void MainWindow::open()
{

}

void MainWindow::save(){



}

void MainWindow::saveas(){

    projectPath=QFileDialog::getSaveFileName(this,tr("Save project file"),"untitled.DXplorer",tr("*.DXplorer"));
    if(projectPath!=""){
        save();
    }

}

void MainWindow::close()
{
//    setCentralWidget(NULL);
//    delete [] c_window;
//    setWindowFilePath(QString());
}
void MainWindow::featureManager(){
    c_window->featureUsageSetting();
}
void MainWindow::predefinedTypes(){
    if(c_window->typeGroup->isHidden()){
        c_window->typeGroup->show();
    }
    else{
        c_window->typeGroup->hide();
    }
}
void MainWindow::dataGroups(){
    if(c_window->datagroupGroup->isHidden()){
        c_window->datagroupGroup->show();
    }
    else{
        c_window->datagroupGroup->hide();
    }
}
void MainWindow::createActions()
{

    openAct = new QAction(tr("Load..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(tr("Save..."), this);
    saveAct->setShortcuts(QKeySequence::Save);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveas()));

    featureManagerAct = new QAction(tr("Open Manager..."), this);
    connect(featureManagerAct, SIGNAL(triggered()), this, SLOT(featureManager()));

    predefinedTypesAct = new QAction(tr("Pre-defined Types On/Off..."), this);
    connect(predefinedTypesAct, SIGNAL(triggered()), this, SLOT(predefinedTypes()));

    dataGroupsAct = new QAction(tr("Data Groups On/Off..."), this);
    connect(dataGroupsAct, SIGNAL(triggered()), this, SLOT(dataGroups()));


//    exitAct = new QAction(tr("Exit"), this);
//    exitAct->setShortcuts(QKeySequence::Quit);
//    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("Project"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
//    fileMenu->addAction(saveAsAct);

    featureMenu= menuBar()->addMenu(tr("Feature"));
//    featureMenu->addAction(featureManagerAct);

    windowMenu=menuBar()->addMenu(tr("Window"));
//    windowMenu->addAction(predefinedTypesAct);
//    windowMenu->addAction(dataGroupsAct);

}
