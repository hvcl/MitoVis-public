##JunYoung Choi
##juny0603@gmail.com
##Ulsan National Institute of Science and Technology(UNIST)

HEADERS       = window.h \
                mainwindow.h \
                parallelCoordinate.h \
                tsneSelection.h \
                colorBar.h \
                framelesswindow/framelesswindow.h \
                framelesswindow/windowdragger.h \
                DarkStyle.h \
                glwidget_for_data.h \
                glwidget_for_object.h \
                tinytiffreader.h \
                tinytiffwriter.h \
                imageControlGraph.h \
                structureRefinement.h \
                mojo/activation.h \
                mojo/core_math.h \
                mojo/cost.h \
                mojo/layer.h \
                mojo/mojo.h \
                mojo/network.h \
                mojo/solver.h \
                structureConnector.h \
                rf/decision-trees.hxx \
                rf/marray.hxx \
                variationGraph.h \
                glwidget_for_inset.h \
                glwidget_for_tracingMap.h \
                patchrecommender.h \
                feature_space_radar.h

SOURCES       = main.cpp \
                window.cpp \
                mainwindow.cpp \
                parallelCoordinate.cpp \
                tsneSelection.cpp \
                colorBar.cpp \
                framelesswindow/framelesswindow.cpp \
                framelesswindow/windowdragger.cpp \
                DarkStyle.cpp \
                glwidget_for_data.cpp \
                glwidget_for_object.cpp \
                tinytiffreader.cpp  \
                tinytiffwriter.cpp \
                imageControlGraph.cpp \
                structureRefinement.cpp \
                structureConnector.cpp \
                variationGraph.cpp \
                glwidget_for_inset.cpp \
                glwidget_for_tracingMap.cpp \
                patchrecommender.cpp \
                feature_space_radar.cpp



FORMS       +=  framelesswindow/framelesswindow.ui

INCLUDEPATH +="framelesswindow"

RESOURCES   += darkstyle.qrc \
               framelesswindow.qrc

QT           +=widgets
QT           +=gui


TARGET = MitoVis
TEMPLATE = app
