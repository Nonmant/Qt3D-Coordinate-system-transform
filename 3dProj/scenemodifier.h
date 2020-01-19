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

#ifndef SCENEMODIFIER_H
#define SCENEMODIFIER_H

#include <QtCore/QObject>

#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>

#include <Qt3DExtras/QPhongMaterial>

#include <Qt3DRender/QMesh>
#include <QCoreApplication>

#include <QPropertyAnimation>
#include <QtWidgets>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DRender>
#include <Qt3DRender/QBuffer>

#include "draw3dlines.h"

#include <Qt3DExtras/QText2DEntity>

class SceneModifier : public QObject
{
    Q_OBJECT
    void setLines3DEnt(Qt3DCore::QEntity *ptr=nullptr);
    void nullLines3DEnt();
    void initText2d();
    void clearText2d();
    void enableText2d(bool state);
    void matrixInit(bool zeros=false);
    void matrixClearShow();
    void matrixCinemShow();
    float arcRadius=5.0f;
    float fText2dW=100, fText2dH=20, fText2dS=0.06f;
    bool stepsRot=false;
    bool showText2d=true;
    Qt3DExtras::QText2DEntity *text2dEntity[2][3][2];
    Qt3DCore::QTransform *text2dTransform[2][3][2];
    const QVector3D text2dTrInit[2][3][2]=
    {
        {{{0,0,arcRadius},{0,arcRadius,0}}, {{0,0,arcRadius},{arcRadius,0,0}}, {{arcRadius,0,0},{0,arcRadius,0}}},
        {{{0,0,arcRadius},{arcRadius,0,0}}, {{0,0,arcRadius},{0,arcRadius,0}}, {{arcRadius,0,0},{0,arcRadius,0}}}
    };
    QVector3D text2dTr[2][3][2];
    const QColor colors[2][3]={{Qt::red, Qt::green, Qt::blue},
                               {Qt::green, Qt::red, Qt::blue}};

public:
    explicit SceneModifier(Qt3DCore::QEntity *rootEntity);
    ~SceneModifier();

    Qt3DCore::QEntity *rootEntity;
    Qt3DRender::QCamera *cameraEntity;

    Qt3DCore::QEntity *planeEntity[3], *baseEntity[3], *lines3DEntity[2][3][2], *lineQuat=nullptr;
    Qt3DCore::QTransform * planeTransform[3], *baseTransform[3], *lineQuatTransform;
    QVector3D planeTranslation[3]={{-13,0,0}, {0,0,0}, {13,0,0}};
    QPropertyAnimation *planeAnimation[3];
    QParallelAnimationGroup *groupAnimation;
    QVariantAnimation *cinemAnimation;
    QQuaternion quat, angleQuats[2][3], cinemQuat;

    int curRotation=-1, curRotationInd=-1;

    QMatrix3x3 angleMatrix[3][3];

    float angle[2][3]={{0,0,0},{0,0,0}};
    float *angleX[2]={&angle[0][0], &angle[1][0]};
    float *angleY[2]={&angle[0][1], &angle[1][1]};
    float *angleZ[2]={&angle[0][2], &angle[1][2]};

    float cinemAngle[2][3]={{0,0,0},{0,0,0}};//radians
    float *cinemAngleX[2]={&cinemAngle[0][0], &cinemAngle[1][0]};
    float *cinemAngleY[2]={&cinemAngle[0][1], &cinemAngle[1][1]};
    float *cinemAngleZ[2]={&cinemAngle[0][2], &cinemAngle[1][2]};

    float cinemSpeed[2][3]={{0,0,0},{0,0,0}};//converts to radians/s
    float *cinemSpeedX[2]={&cinemSpeed[0][0], &cinemSpeed[1][0]};
    float *cinemSpeedY[2]={&cinemSpeed[0][1], &cinemSpeed[1][1]};
    float *cinemSpeedZ[2]={&cinemSpeed[0][2], &cinemSpeed[1][2]};

    float cinemAngleD[2][3]={{0,0,0},{0,0,0}};//radians
    float *cinemAngleDX[2]={&cinemAngleD[0][0], &cinemAngleD[1][0]};
    float *cinemAngleDY[2]={&cinemAngleD[0][1], &cinemAngleD[1][1]};
    float *cinemAngleDZ[2]={&cinemAngleD[0][2], &cinemAngleD[1][2]};
    float cinemTime=0, cinemTimePrev=0, cinemTimeDt=0;
    QString startButtonTexts[3]={"Показать","Остановить", "Продолжить"};
    QString angleName[2][3]={{"rotationX", "rotationY", "rotationZ"},
                            {"rotationY", "rotationX", "rotationZ"}};
    QWidget *parentWidget;
    QTabWidget *tab;
    QPushButton *startButton;
    QLineEdit *lineAngle[3], *lineSpeed[3];
    QVector<float *>angleFloatPtr[3]={{&angle[0][0], &angle[1][1]},
                                    {&angle[0][1], &angle[1][0]},
                                    {&angle[0][2], &angle[1][2]}};
    QVector<float *>cinemSpeedPtr[3]={{&cinemSpeed[0][0], &cinemSpeed[1][1]},//radians
                                    {&cinemSpeed[0][1], &cinemSpeed[1][0]},
                                    {&cinemSpeed[0][2], &cinemSpeed[1][2]}};
    QLabel * matrixLabel[3][3];

    void checkLineInput(QLineEdit* line, QWidget *goal, QVector <float *>values);
    void checkLineInput(QLineEdit* line, QWidget *goal, float *value);
    void checkLines(QLineEdit* line, QWidget *goal);
    void matrixShowLine(int matrixLine=-1);

public slots:
    //void getAnimation(QString *name, double endValue);

    void line1Edited();
    void line2Edited();
    void line3Edited();
    void lineSpeed1Edited();
    void lineSpeed2Edited();
    void lineSpeed3Edited();
    void startButtonClicked();
    void planeAnimationFunc();
    void planeAnimationQuatFunc();
    void setStepsRot(bool state);
    void checkBoxText2dToggled(bool state);
    void checkBoxBaseToggled(bool state);
    void checkBoxPerspToggled(bool state);
    void enableButton();
    void tabChanged(int index);
    void cinemAnimationTick(const QVariant &value);
    void oneRotStepFinished();
};

#endif // SCENEMODIFIER_H

