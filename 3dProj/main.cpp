/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "scenemodifier.h"

#include <QGuiApplication>

#include <Qt3DRender/qcamera.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DRender/qcameralens.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtGui/QScreen>

#include <Qt3DInput/QInputAspect>

#include <Qt3DRender/qmesh.h>
#include <Qt3DRender/qtechnique.h>
#include <Qt3DRender/qmaterial.h>
#include <Qt3DRender/qeffect.h>
#include <Qt3DRender/qtexture.h>
#include <Qt3DRender/qrenderpass.h>
#include <Qt3DRender/qsceneloader.h>
#include <Qt3DRender/qpointlight.h>

#include <Qt3DCore/qtransform.h>
#include <Qt3DCore/qaspectengine.h>

#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DExtras/qforwardrenderer.h>

#include <Qt3DExtras/qt3dwindow.h>
#include <Qt3DExtras/qfirstpersoncameracontroller.h>

#include <QtWidgets>
#include <Qt3DExtras>

class FilterObject:public QObject{
    //Q_OBJECT
public:
    QWidget *target = nullptr;
    int goalHeight=0;
    FilterObject(QObject *parent=nullptr):QObject(parent){}
    //~FilterObject(){}
    bool eventFilter(QObject *watched, QEvent *event) override;
};
bool FilterObject::eventFilter(QObject *watched, QEvent *event) {
    if(watched!=target){
        return false;
    }
    if(event->type()!=QEvent::Resize){
        return false;
    }

    QResizeEvent *resEvent = static_cast<QResizeEvent*>(event);

    goalHeight = 7*resEvent->size().width()/16;
    if(target->height()!=goalHeight){
        target->setFixedHeight(goalHeight);
    }

    return true;
};

class FilterCamPos:public QObject{
        QPropertyAnimation *animPos, *animUpVect, *animViewCenter;
        QParallelAnimationGroup *animGroup;
        Qt3DRender::QCamera *camera = nullptr;
    public:

    QVector <QVector3D> positions, upVectors;
    QGridLayout *layout = nullptr;

    FilterCamPos(QObject *parent=nullptr):QObject(parent){
        animPos = new QPropertyAnimation();
        animPos->setPropertyName("position");
        animPos->setLoopCount(1);
        animPos->setDuration(1000);

        animUpVect = new QPropertyAnimation();
        animUpVect->setPropertyName("upVector");
        animUpVect->setLoopCount(1);
        animUpVect->setDuration(1000);

        animViewCenter = new QPropertyAnimation();
        animViewCenter->setPropertyName("viewCenter");
        animViewCenter->setLoopCount(1);
        animViewCenter->setDuration(1000);
        animViewCenter->setEndValue(QVector3D(0, 0, 0));

        animGroup = new QParallelAnimationGroup();
        animGroup->addAnimation(animPos);
        animGroup->addAnimation(animUpVect);
        animGroup->addAnimation(animViewCenter);

        QObject::connect(animGroup, &QParallelAnimationGroup::finished, this, &FilterCamPos::enableLayout);
    }
    bool eventFilter(QObject *watched, QEvent *event) override;
    void addCamera(Qt3DRender::QCamera *cameraIn);
    void enableLayout();
};

void FilterCamPos::addCamera(Qt3DRender::QCamera *cameraIn){
    camera=cameraIn;
    animPos->setTargetObject(camera);
    animUpVect->setTargetObject(camera);
    animViewCenter->setTargetObject(camera);
}

bool FilterCamPos::eventFilter(QObject *watched, QEvent *event) {
    if(layout==nullptr || camera==nullptr){
        return false;
    }

    if(animGroup->state()!=QParallelAnimationGroup::State::Stopped){
        return false;
    }

    if(event->type()!=QEvent::MouseButtonPress){
        return false;
    }
    QWidget *widget = static_cast<QWidget*>(watched);

    //QMouseEvent *resEvent = static_cast<QMouseEvent*>(event);
    int index = layout->indexOf(widget);
    if(index>=positions.length()||index>=upVectors.length()){
        return false;
    }
    /*
    camera->setUpVector(upVectors.at(index));
    camera->setPosition(positions.at(index));
    camera->setViewCenter(QVector3D(0, 0, 0));*/

    layout->setEnabled(false);

    animPos->setStartValue(camera->position());
    animPos->setEndValue(positions.at(index));

    animUpVect->setStartValue(camera->upVector());
    animUpVect->setEndValue(upVectors.at(index));

    animViewCenter->setStartValue(camera->viewCenter());

    animGroup->start();

    return true;
};
void FilterCamPos::enableLayout(){
    if(layout==nullptr){
        return;
    }
    layout->setEnabled(true);
}

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(axes);

    QApplication app(argc, argv);
    Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    QWidget *container = QWidget::createWindowContainer(view);
    QSize screenSize = view->screen()->size();
    container->setMinimumSize(QSize(200, 100));
    container->setMaximumSize(screenSize);

    FilterObject *filter = new FilterObject();
    filter->target=container;
    container->installEventFilter(filter);

    QSizePolicy *containerSizePolicy= new QSizePolicy();
    containerSizePolicy->setHorizontalPolicy(QSizePolicy::Expanding);
    containerSizePolicy->setHeightForWidth(true);
    container->setSizePolicy(*(containerSizePolicy));

    QWidget *widget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setAlignment(Qt::AlignTop);

    QSizePolicy menuSizePolicy;
    menuSizePolicy.setHorizontalPolicy(QSizePolicy::Minimum);

    QVBoxLayout *vLayoutOut = new QVBoxLayout();
    vLayoutOut->addWidget(container, 1);
    hLayout->addLayout(vLayoutOut);
    hLayout->addLayout(vLayout);

    widget->setWindowTitle(QStringLiteral("Визуализация переходов между СК"));

    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
    view->registerAspect(input);

    // Root entity
    Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();

    // Camera
    Qt3DRender::QCamera *cameraEntity = view->camera();

    //cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/7.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0, 10.0f, 20.0f));
    //cameraEntity->setPosition(QVector3D(0, 0, 1.0f));
    cameraEntity->setUpVector(QVector3D(0, 1, 0));
    cameraEntity->setViewCenter(QVector3D(0, 0, 0));

    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1);
    lightEntity->addComponent(light);
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(cameraEntity->position());
    lightEntity->addComponent(lightTransform);

    // For camera controls
    //Qt3DExtras::QFirstPersonCameraController *camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
    Qt3DExtras::QOrbitCameraController *camController = new Qt3DExtras::QOrbitCameraController (rootEntity);

    camController->setCamera(cameraEntity);

    // Scenemodifier
    SceneModifier *modifier = new SceneModifier(rootEntity);
    modifier->parentWidget=widget;
    modifier->cameraEntity=cameraEntity;
    // Set root object of the scene
    view->setRootEntity(rootEntity);

    // Create control widgets

    QTabWidget *menu=new QTabWidget();
    modifier->tab=menu;
    QWidget *tabAngle=new QWidget();
    QWidget *tabSpeed = new QWidget();
    menu->addTab(tabAngle, "Углы");
    menu->addTab(tabSpeed, "Скорости");
    QVBoxLayout *tabAngleLayout = new QVBoxLayout();
    QVBoxLayout *tabSpeedLayout = new QVBoxLayout();
    tabAngle->setLayout(tabAngleLayout);
    tabSpeed->setLayout(tabSpeedLayout);


    menu->setSizePolicy(menuSizePolicy);
    vLayout->addWidget(menu);
    QColor axesColors[3]={"#FF0100", "#00FF1F", "#0095FF"};

    QLabel *angleLabel[3];
    QLineEdit *angleLine[3];
    QString labelText[3]={"Тангаж, θ° <font color=%1><b>O<sub>X</sub></b></font>:",
                          "Курс, ψ° <font color=%1><b>O<sub>Y</sub></b></font>:",
                          "Крен, γ° <font color=%1><b>O<sub>Z</sub></b></font>:"};
    for(int i=0; i<3; ++i){
        //labelText[i]=labelText[i].arg("green");
        angleLabel[i] = new QLabel();
        angleLabel[i]->setText(labelText[i].arg(axesColors[i].name()));
        angleLabel[i]->setSizePolicy(menuSizePolicy);

        angleLine[i] = new QLineEdit(widget);
        modifier->lineAngle[i]=angleLine[i];
        angleLine[i]->setText("0");
        angleLine[i]->setSizePolicy(menuSizePolicy);

        tabAngleLayout->addWidget(angleLabel[i]);
        tabAngleLayout->addWidget(angleLine[i]);
    }

    QCheckBox *checkBoxText2d=new QCheckBox;
    checkBoxText2d->setCheckState(Qt::CheckState::Checked);
    checkBoxText2d->setText("Значения углов");
    tabAngleLayout->addWidget(checkBoxText2d);
    QObject::connect(checkBoxText2d, &QCheckBox::toggled, modifier, &SceneModifier::checkBoxText2dToggled);

    QCheckBox *checkBoxStepsRot=new QCheckBox;
    checkBoxStepsRot->setCheckState(Qt::CheckState::Unchecked);
    checkBoxStepsRot->setText("Остановки при поворотах");
    tabAngleLayout->addWidget(checkBoxStepsRot);
    QObject::connect(checkBoxStepsRot, &QCheckBox::toggled, modifier, &SceneModifier::setStepsRot);

    QLabel *speedLabel[3];
    QLineEdit *speedLine[3];
    QString speedLabelText[3]={"ω<font color=%1><b><sub>X</sub></b></font> °/с:",//"ω<font size=small; color=%1><b>X</b></font> °/с:",
                               "ω<font color=%1><b><sub>Y</sub></b></font> °/с:",
                               "ω<font color=%1><b><sub>Z</sub></b></font> °/с:"};
    for(int i=0; i<3; ++i){
        speedLabel[i] = new QLabel();
        speedLabel[i]->setText(speedLabelText[i].arg(axesColors[i].name()));
        speedLabel[i]->setSizePolicy(menuSizePolicy);

        speedLine[i] = new QLineEdit(widget);
        modifier->lineSpeed[i]=speedLine[i];
        speedLine[i]->setText("0");
        speedLine[i]->setSizePolicy(menuSizePolicy);

        tabSpeedLayout->addWidget(speedLabel[i]);
        tabSpeedLayout->addWidget(speedLine[i]);
    }

    QPushButton *startButton = new QPushButton();
    modifier->startButton=startButton;
    startButton->setText("Показать");
    startButton->setSizePolicy(menuSizePolicy);
    vLayout->addWidget(startButton);

    QCheckBox *checkBoxBase=new QCheckBox ();
    checkBoxBase->setCheckState(Qt::CheckState::Checked);
    checkBoxBase->setText("Показать опорные оси");
    vLayout->addWidget(checkBoxBase);
    QObject::connect(checkBoxBase, &QCheckBox::toggled, modifier, &SceneModifier::checkBoxBaseToggled);

    QCheckBox *checkBoxPersp=new QCheckBox ();
    checkBoxPersp->setCheckState(Qt::CheckState::Checked);
    checkBoxPersp->setText("Перспектива");
    vLayout->addWidget(checkBoxPersp);
    QObject::connect(checkBoxPersp, &QCheckBox::toggled, modifier, &SceneModifier::checkBoxPerspToggled);


    QString camPosItems[6]={"Верх", "Лево", "Центр", "Право", "Низ", "🏠"};
    int camPosItemsIndex[6][2]={{0,1},{1,0}, {1,1}, {1,2}, {2,1}, {2,0}};
    QVector <QVector3D> camPositions{QVector3D(0, 22.36f, 0),
                                        QVector3D(-22.36f, 0, 0),
                                        QVector3D(0, 0, 22.36f),
                                        QVector3D(22.36f, 0, 0),
                                        QVector3D(0, -22.36f, 0),
                                        QVector3D(0, 10.0f, 20.0f)};


    QVector <QVector3D> camUpVectors{QVector3D(0, 0, -1),
                                        QVector3D(0, 1, 0),
                                        QVector3D(0, 1, 0),
                                        QVector3D(0, 1, 0),
                                        QVector3D(0, 0, 1),
                                        QVector3D(0, 1, 0)};

    int rowsNum=0;
    int columnsNum=0;

    for(int i=0; i<5; ++i){
        if(camPosItemsIndex[i][0]>rowsNum){
            rowsNum=camPosItemsIndex[i][0];
        }
        if(camPosItemsIndex[i][1]>columnsNum){
            columnsNum=camPosItemsIndex[i][1];
        }
    }
    QGridLayout *layoutCamPos=new QGridLayout();
    FilterCamPos *filterCamPos = new FilterCamPos();
    filterCamPos->layout=layoutCamPos;
    filterCamPos->addCamera(cameraEntity);
    filterCamPos->positions = camPositions;
    filterCamPos->upVectors = camUpVectors;
    for(int i=0;i<6;++i){
        QPushButton *button = new QPushButton();
        button->setText(camPosItems[i]);
        button->setFlat(true);
        button->installEventFilter(filterCamPos);
        layoutCamPos->addWidget(button, camPosItemsIndex[i][0], camPosItemsIndex[i][1]);
    }
    layoutCamPos->setSpacing(0);

    vLayout->addLayout(layoutCamPos);

    QGridLayout *matrixLayout=new QGridLayout();

    QLabel * matrixLabel[3][3];
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    QString matrixInitText;

   for(int i=0; i<3;++i){
        for(int j=0; j<3;++j){
            matrixLabel[i][j]=new QLabel();
            matrixInitText="Матрица:\nСК №"+QString::number(j+1)+",\nповорот "+ QString::number(i+1);
            matrixLabel[i][j]->setText(matrixInitText);
            matrixLabel[i][j]->setAlignment(Qt::AlignmentFlag::AlignCenter);
            matrixLabel[i][j]->setFont(fixedFont);
            matrixLabel[i][j]->setToolTip(matrixInitText);
            matrixLayout->addWidget(matrixLabel[i][j], i, j);
            modifier->matrixLabel[i][j]=matrixLabel[i][j];
        }
    }
    vLayoutOut->addLayout(matrixLayout);

    QObject::connect(angleLine[0], &QLineEdit::textEdited, modifier, &SceneModifier::line1Edited);
    QObject::connect(angleLine[1], &QLineEdit::textEdited, modifier, &SceneModifier::line2Edited);
    QObject::connect(angleLine[2], &QLineEdit::textEdited, modifier, &SceneModifier::line3Edited);

    QObject::connect(speedLine[0], &QLineEdit::textEdited, modifier, &SceneModifier::lineSpeed1Edited);
    QObject::connect(speedLine[1], &QLineEdit::textEdited, modifier, &SceneModifier::lineSpeed2Edited);
    QObject::connect(speedLine[2], &QLineEdit::textEdited, modifier, &SceneModifier::lineSpeed3Edited);

    QObject::connect(menu, &QTabWidget::currentChanged, modifier, &SceneModifier::tabChanged);

    QObject::connect(startButton, &QPushButton::clicked, modifier, &SceneModifier::startButtonClicked);

    // Show window
    widget->show();
    widget->resize(1200, 600);

    return app.exec();
}
