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

#include <QtCore/QDebug>

#include <QTime>

using namespace std;

SceneModifier::SceneModifier(Qt3DCore::QEntity *rootEntity)
    : rootEntity(rootEntity)
{

    // Plane shape data
    Qt3DRender::QMesh *plane = new Qt3DRender::QMesh;
    plane->setSource(QUrl(QStringLiteral("qrc:/models/axes.obj")));

    Qt3DRender::QTextureImage *image= new Qt3DRender::QTextureImage();
    image->setSource(QUrl(QStringLiteral("qrc:/models/axes.webp")));
    Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D();
    texture->addTextureImage(image);
    Qt3DExtras::QTextureMaterial *textureMaterial = new Qt3DExtras::QTextureMaterial();
    textureMaterial->setTexture(texture);

    Qt3DRender::QTextureImage *imageBase= new Qt3DRender::QTextureImage();
    imageBase->setSource(QUrl(QStringLiteral("qrc:/models/axesBase.webp")));
    Qt3DRender::QTexture2D *textureBase = new Qt3DRender::QTexture2D();
    textureBase->addTextureImage(imageBase);
    Qt3DExtras::QTextureMaterial *textureMaterialBase = new Qt3DExtras::QTextureMaterial();
    textureMaterialBase->setTexture(textureBase);


    for(int i=0; i<3; ++i){
        //plane Mesh Transform
        planeTransform[i] = new Qt3DCore::QTransform();
        planeTransform[i]->setTranslation(planeTranslation[i]);
        //planeTransform[i]->setScale(0.6f);

        // plane entity
        planeEntity[i] = new Qt3DCore::QEntity(rootEntity);
        planeEntity[i]->addComponent(plane);
        planeEntity[i]->addComponent(textureMaterial);
        planeEntity[i]->addComponent(planeTransform[i]);

        //base Mesh Transform
        baseTransform[i] = new Qt3DCore::QTransform();
        baseTransform[i]->setTranslation(planeTranslation[i]);
        //baseTransform[i]->setScale(0.6f);

        // base entity
        baseEntity[i] = new Qt3DCore::QEntity(rootEntity);
        baseEntity[i]->addComponent(plane);
        baseEntity[i]->addComponent(textureMaterialBase);
        baseEntity[i]->addComponent(baseTransform[i]);

        //plane animations
        planeAnimation[i]=new QPropertyAnimation();

        planeAnimation[i]->setLoopCount(1);
        planeAnimation[i]->setTargetObject(planeTransform[i]);
        planeAnimation[i]->setDuration(1000);
    }

    //rules for last plane
    quat.fromEulerAngles(0,0,0);
    planeTransform[2]->setRotation(quat);//QQuaternion &rotation
    planeAnimation[2]->setDuration(3000+10);
    groupAnimation = new QParallelAnimationGroup;
    groupAnimation->addAnimation(planeAnimation[0]);
    groupAnimation->addAnimation(planeAnimation[1]);

    //QObject::connect(planeAnimation[0], &QPropertyAnimation::finished, this, &SceneModifier::planeAnimationFunc);
    QObject::connect(groupAnimation, &QParallelAnimationGroup::finished, this, &SceneModifier::oneRotStepFinished);
    QObject::connect(planeAnimation[2], &QPropertyAnimation::finished, this, &SceneModifier::enableButton);

    //QObject::connect(planeAnimation[0], &QPropertyAnimation::valueChanged, this, &SceneModifier::matrixUpdate);

    cinemAnimation = new QVariantAnimation();
    cinemAnimation->setDuration(1000);
    cinemAnimation->setLoopCount(-1);
    cinemAnimation->setStartValue(0);
    cinemAnimation->setEndValue(1000);
    QObject::connect(cinemAnimation, &QVariantAnimation::valueChanged, this, &SceneModifier::cinemAnimationTick);
    //cinemAnimation->start();

    nullLines3DEnt();
    lineQuatTransform = new Qt3DCore::QTransform;
    initText2d();
}

SceneModifier::~SceneModifier()
{
}

float cosDeg(float val){
    return cos(qDegreesToRadians(val));
}

float cosDeg(float *val){
    return cos(qDegreesToRadians(*val));
}

float sinDeg(float val){
    return sin(qDegreesToRadians(val));
}

float sinDeg(float *val){
    return sin(qDegreesToRadians(*val));
}

QQuaternion operator/(const QQuaternion &a, const QQuaternion &b){
    return a*(b.conjugated()/b.length());
}

QVector3D rotateQVector3D(const QVector3D &vect, const QQuaternion &rot){
    auto matrix = rot.toRotationMatrix();
    QVector3D outVect;
    for(int i=0; i<3; ++i){
        for(int j=0; j<3; ++j){
            outVect[i]+=matrix(i,j)*vect[j];
        }
    }
    return outVect;
}

void SceneModifier::initText2d(){
    for(int i=0; i<2; ++i){
        for(int j=0; j<3; ++j){
            for(int k=0;k<2;++k){
                text2dTransform[i][j][k] = new Qt3DCore::QTransform;
                text2dTransform[i][j][k]->setScale(fText2dS);

                text2dEntity[i][j][k] = new Qt3DExtras::QText2DEntity(rootEntity);
                text2dEntity[i][j][k]->setFont(QFont("monospace",12));
                text2dEntity[i][j][k]->setHeight(fText2dH);
                text2dEntity[i][j][k]->setWidth(fText2dW);
                //text2dEntity[i][j][k]->setText("A");
                text2dEntity[i][j][k]->setColor(colors[i][j]);
                text2dEntity[i][j][k]->addComponent(text2dTransform[i][j][k]);
            }
        }
    }
}

void SceneModifier::clearText2d(){
    for(int i=0; i<2; ++i){
        for(int j=0; j<3; ++j){
            for(int k=0;k<2;++k){
                text2dEntity[i][j][k]->setText(" ");
            }
        }
    }
}

void SceneModifier::checkBoxText2dToggled(bool state){
    showText2d=state;
    enableText2d(state);
}

void SceneModifier::enableText2d(bool state){
    for(int i=0; i<2; ++i){
        for(int j=0; j<3; ++j){
            for(int k=0;k<2;++k){
                text2dEntity[i][j][k]->setEnabled(state);
            }
        }
    }
}

void SceneModifier::setStepsRot(bool state){
    stepsRot=state;
}

void SceneModifier::oneRotStepFinished(){
    if(!stepsRot){
        planeAnimationFunc();
    }
    else{
        startButton->setEnabled(true);

        if(curRotation==-1){
            startButton->setText(startButtonTexts[0]);
        }

        if(curRotation==3){
            planeAnimationFunc();
        }

        if(planeAnimation[2]->state()==QVariantAnimation::State::Running){
            planeAnimation[2]->pause();
        }

        return;
    }
}

void SceneModifier::setLines3DEnt(Qt3DCore::QEntity *ptr){
    for(int k=0; k<2; ++k){
        for(int i=0; i<3; ++i){
            for(int j=0; j<2; ++j){
                if(lines3DEntity[k][i][j]!=nullptr){
                    lines3DEntity[k][i][j]->deleteLater();
                    lines3DEntity[k][i][j]=ptr;
                }
            }
        }
    }
    if(lineQuat!=nullptr){
        lineQuat->deleteLater();
        lineQuat=ptr;
    }
}
void SceneModifier::nullLines3DEnt(){
    for(int k=0; k<2; ++k){
        for(int i=0; i<3; ++i){
            for(int j=0; j<2; ++j){
                lines3DEntity[k][i][j]=nullptr;
            }
        }
    }
    lineQuat=nullptr;
}

void SceneModifier::cinemAnimationTick(const QVariant &value){

    cinemTimePrev=cinemTime;
    cinemTime=cinemAnimation->currentTime()/1000.0f;
    cinemTimeDt=cinemTime-cinemTimePrev;

    *cinemAngleDX[0]=(*cinemSpeedX[0] * cos(*cinemAngleZ[0]) - *cinemSpeedY[0] * sin(*cinemAngleZ[0]))/cos(*cinemAngleY[0]);//(wx*cosFiZ - wy*sinFiZ)/cosFiY
    *cinemAngleDY[0]=*cinemSpeedX[0] * sin(*cinemAngleZ[0]) + *cinemSpeedY[0] * cos(*cinemAngleZ[0]);//wx*sinFiZ + wy*cosFiZ
    *cinemAngleDZ[0]=sin(*cinemAngleY[0]) * (*cinemSpeedY[0] * sin(*cinemAngleZ[0]) - *cinemSpeedX[0] * cos(*cinemAngleZ[0]))/cos(*cinemAngleY[0]) + *cinemSpeedZ[0];//sinFiz*(wy*sinFiZ - wx*cosFiZ)/cosFiY + wz

    /*
    *cinemAngleX[0]+= *cinemAngleDX[0] * cinemTimeDt;
    *cinemAngleY[0]+= *cinemAngleDY[0] * cinemTimeDt;
    *cinemAngleZ[0]+= *cinemAngleDZ[0] * cinemTimeDt;

    planeTransform[0]->setRotationX(qRadiansToDegrees(*cinemAngleX[0]));
    planeTransform[0]->setRotationY(qRadiansToDegrees(*cinemAngleY[0]));
    planeTransform[0]->setRotationZ(qRadiansToDegrees(*cinemAngleZ[0]));
    */

    cinemQuat+=(0.5f*cinemTimeDt)*(cinemQuat*QQuaternion(0,*cinemSpeedX[0],*cinemSpeedY[0], *cinemSpeedZ[0]));
    if(fabs(cinemQuat.lengthSquared()-1)>0.01f)
        cinemQuat.normalize();
    planeTransform[1]->setRotation(cinemQuat);
    //lineQuatTransform->setRotation(cinemQuat);
    //lineSpeed[0]->setText(QString::number(cinemTime));

    matrixCinemShow();
}

void SceneModifier::checkBoxBaseToggled(bool state){
    if(cinemAnimation->state()==QVariantAnimation::State::Running){
        baseEntity[1]->setEnabled(state);
        return;
    }
    for(int i=0;i<3;++i){
        baseEntity[i]->setEnabled(state);
    }
}
void SceneModifier::checkBoxPerspToggled(bool state){
    float field = cameraEntity->fieldOfView();
    float distance = cameraEntity->position().distanceToPoint(QVector3D(0,0,0));
    if(state){
        //cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
        cameraEntity->lens()->setPerspectiveProjection(45.0f,
                                                       16.0f/7.0f,
                                                       cameraEntity->lens()->nearPlane(),
                                                       cameraEntity->lens()->farPlane());
    }
    else{
        //float base=42.4f*(field/45.0f)*distance/(QVector3D(0,10,20).distanceToPoint(QVector3D(0,0,0)));
        float base=field*distance*0.0421375f;
        cameraEntity->lens()->setOrthographicProjection(-base/2.0f,
                                                        base/2.0f,
                                                        -(base/2.0f)/(16.0f/7.0f),
                                                        base/2.0f/(16.0f/7.0f),
                                                        cameraEntity->lens()->nearPlane(),
                                                        cameraEntity->lens()->farPlane());
    }
}
void SceneModifier::enableButton(){
    startButton->setEnabled(true);
}
void SceneModifier::line1Edited(){
    checkLineInput(lineAngle[0], startButton, angleFloatPtr[0]);
}
void SceneModifier::line2Edited(){
    checkLineInput(lineAngle[1], startButton, angleFloatPtr[1]);
}
void SceneModifier::line3Edited(){
    checkLineInput(lineAngle[2], startButton, angleFloatPtr[2]);
}
void SceneModifier::lineSpeed1Edited(){
    checkLineInput(lineSpeed[0], startButton, cinemSpeedPtr[0]);
}
void SceneModifier::lineSpeed2Edited(){
    checkLineInput(lineSpeed[1], startButton, cinemSpeedPtr[1]);
}
void SceneModifier::lineSpeed3Edited(){
    checkLineInput(lineSpeed[2], startButton, cinemSpeedPtr[2]);
}
void SceneModifier::tabChanged(int index){

    if(planeAnimation[2]->state()==QPropertyAnimation::State::Running)
        return;
    if(cinemAnimation->state()==QVariantAnimation::State::Running)
        return;
    if(index==0){
        checkLines(lineAngle[0],startButton);
    }
    else{
        checkLines(lineSpeed[0],startButton);
    }
}
void SceneModifier::checkLineInput(QLineEdit* line, QWidget *goal, float *value){
    QVector<float *>values={value};
    checkLineInput(line, goal, values);

}
void SceneModifier::checkLineInput(QLineEdit* line, QWidget *goal, QVector<float *>values){
    QString bufString=line->text();
    bool bufBool=false;
    float bufFloat=bufString.toFloat(&bufBool);

    if(!bufBool){
        bufString.replace(",",".");
        bufFloat=bufString.toFloat(&bufBool);
    }

    if(bufBool){
        for(int i=0; i<values.length(); ++i){
            *values[i]=bufFloat;
        }
        //*value=bufFloat;
        line->setClearButtonEnabled(false);
    }else{
        line->setClearButtonEnabled(true);
    }

    checkLines(line, goal);
}

void SceneModifier::checkLines(QLineEdit* line, QWidget *goal){
    bool enable=true;
    for(int i=0; i<3; ++i){
        if(lineAngle[i]==line){
            for(int i=0; i<3; ++i){
                enable&=!lineAngle[i]->isClearButtonEnabled();
            }
            break;
        }
        if(lineSpeed[i]==line){
            for(int i=0; i<3; ++i){
                enable&=!lineSpeed[i]->isClearButtonEnabled();
            }
            break;
        }
    }

    goal->setEnabled(enable);
}

void SceneModifier::startButtonClicked(){
    if(cinemAnimation->state()==QVariantAnimation::State::Running){
        cinemAnimation->stop();
        tab->setEnabled(true);
        startButton->setText(startButtonTexts[0]);
        //lineQuat->removeComponent(lineQuatTransform);
        setLines3DEnt(new Qt3DCore::QEntity());

        //tabChanged(-1);//check values at current tab
        return;
    }
    tab->setEnabled(false);
    if(tab->currentIndex()==0){//angles section
        startButton->setEnabled(false);
        enableText2d(showText2d);
        if(stepsRot&(curRotation!=-1)){
            if(planeAnimation[2]->state()==QVariantAnimation::State::Paused){
                planeAnimation[2]->resume();
            }
            planeAnimationFunc();
            return;
        }

        clearText2d();
        startButton->setText(startButtonTexts[2]);
        for(int i=0; i<3; ++i){ 
            planeEntity[i]->setEnabled(true);
            baseEntity[i]->setEnabled(true);
            planeTransform[i]->setRotation(QQuaternion());
        }

        //matrix_InI();

        for (int k=0; k<2;++k){
            angleQuats[k][0]=QQuaternion::fromAxisAndAngle(k==0,k==1,0, angle[k][0]);
            //angleQuats[0]=planeTransform[0]->rotation()+angleQuats[0];!!!как-то учесть начальное положение
            angleQuats[k][0].normalize();

            for(int i=1; i<=2; ++i){
                angleQuats[k][i]=angleQuats[k][i-1]*QQuaternion::fromAxisAndAngle(i==k,i==(!k),i==2, angle[k][i]);
                angleQuats[k][i].normalize();
            }
        }
        for(int i=0; i<2; ++i){
            for(int j=0; j<3; ++j){//поворот
                for(int k=0; k<2; ++k){//2 угла
                    text2dTr[i][j][k]=rotateQVector3D(text2dTrInit[i][j][k],
                                                      QQuaternion::nlerp(j>0?angleQuats[i][j-1]:QQuaternion(),angleQuats[i][j],0.5f));
                }
            }
        }

        matrixInit(true);
        matrixShowLine();
        curRotation=0;
        setLines3DEnt(new Qt3DCore::QEntity());
        planeAnimationQuatFunc();
        matrixInit();
        planeAnimationFunc();
    }
    else{//1 Cinematic section
        clearText2d();
        enableText2d(false);
        setLines3DEnt(new Qt3DCore::QEntity());
        matrixClearShow();

        for(int i=0; i<3; ++i){
            planeTransform[i]->setRotation(QQuaternion());
        }

        startButton->setText(startButtonTexts[1]);
        cinemTime=0;
        cinemQuat=planeTransform[2]->rotation();
        lineSpeed1Edited();
        lineSpeed2Edited();
        lineSpeed3Edited();
        for(int i=0;i<2;++i){
            *cinemAngleX[i]=qDegreesToRadians(planeTransform[i]->rotationX());
            *cinemAngleY[i]=qDegreesToRadians(planeTransform[i]->rotationY());
            *cinemAngleZ[i]=qDegreesToRadians(planeTransform[i]->rotationZ());
            for(int j=0;j<3;++j){
                cinemSpeed[i][j]=qDegreesToRadians(cinemSpeed[i][j]);
            }
        }
        //отключение двух других ск (0 и 2)
        planeEntity[0]->setEnabled(false);
        planeEntity[2]->setEnabled(false);
        baseEntity[0]->setEnabled(false);
        baseEntity[2]->setEnabled(false);

        //приведение ск к начальному положению

        planeTransform[1]->setRotation(QQuaternion());

        //рисовка вектора
        QVector<QVector3D> points={QVector3D(0, 0, 0), QVector3D(*cinemSpeedX[0], *cinemSpeedY[0], *cinemSpeedZ[0]).normalized()};
        QColor col(abs(int(255*points.at(1).x())), abs(int(255*points.at(1).y())), abs(int(255*points.at(1).z())));
        points[1]*=(planeTranslation[2].x()/2.0f + 0.5f);
        lineQuat=drawLine(points,col, rootEntity);

        auto *transf = new Qt3DCore::QTransform;
        transf->setTranslation(planeTransform[1]->translation());
        lineQuat->addComponent(transf);
        //lineQuatTransform->setTranslation(planeTransform[1]->translation());
        //lineQuat->addComponent(lineQuatTransform);

        cinemAnimation->start();
    }
}
void SceneModifier::planeAnimationQuatFunc(){
    quat=QQuaternion::fromEulerAngles(angle[0][0],angle[0][1],angle[0][2]);
    planeAnimation[2]->stop();

    if(planeTransform[2]->property("rotation")==quat){
        enableButton();
        return;
    }

    QVector<QVector3D> points={QVector3D(0, 0, 0), quat.vector().normalized()};
    QColor col(abs(int(255*points.at(1).x())), abs(int(255*points.at(1).y())), abs(int(255*points.at(1).z())));
    points[1]*=(planeTranslation[2].x()/2.0f + 0.5f);
    lineQuat=drawLine(points,col, rootEntity);

    auto *transf = new Qt3DCore::QTransform;
    transf->setTranslation(planeTransform[2]->translation());
    lineQuat->addComponent(transf);

    planeAnimation[2]->setPropertyName("rotation");
    planeAnimation[2]->setStartValue(planeTransform[2]->property("rotation"));
    //planeAnimation2->setEndValue(*quat);
    planeAnimation[2]->setEndValue(quat);
    planeAnimation[2]->start();
}

void SceneModifier::planeAnimationFunc(){

    //block checking current step

    //for first system (x-y-z) animation

    //for second system (y-x-z) animation

    if(curRotation<3){
        curRotationInd=curRotation;

        for(int i=0; i<2; ++i){
        planeAnimation[i]->setStartValue(planeTransform[i]->rotation());
        planeAnimation[i]->setEndValue(angleQuats[i][curRotation]);
        planeAnimation[i]->setPropertyName("rotation");
        }


        //arcs plotting
        Draw3DLines::PlaneName names[2][3]={{Draw3DLines::yz, Draw3DLines::xz, Draw3DLines::xy},
                                            {Draw3DLines::xz, Draw3DLines::yz, Draw3DLines::xy}};

        for(int k=0;k<2;++k){//k=0 - левая СК, =1 - центральная
            if(planeAnimation[k]->startValue()==planeAnimation[k]->endValue()){
                continue;
            }
            for(int j=0; j<2; ++j){//для каждого поворота 2 дуги для 2-х оставшихся осей

                //lines3DEntity[k][curRotation][j]->deleteLater();


                if(curRotation!=2){//первые 2 повотора отличаются
                lines3DEntity[k][curRotation][j] = drawSegment(arcRadius, 90*j+angle[k][curRotation], 90*j,
                                                               colors[k][curRotation], rootEntity,names[k][curRotation],0.005f);

                }
                else{//костыль, поворот z для первой с.к. не работает
                    lines3DEntity[k][curRotation][j] = drawSegment(arcRadius, 90*j+angle[1][curRotation], 90*j,
                                                                   colors[1][curRotation], rootEntity,names[1][curRotation],0.005f);
                }
                auto *transf = new Qt3DCore::QTransform;
                transf->setRotation(planeTransform[k]->rotation());
                transf->setTranslation(planeTransform[k]->translation());
                //transf->setTranslation(planeTranslation[0]);
                lines3DEntity[k][curRotation][j]->addComponent(transf);

                //text 2d plotting
                if(angle[k][curRotation]!=0){//don't show zero angles sizes
                text2dEntity[k][curRotation][j]->setText(QString::number(angle[k][curRotation],'g',3) % "°");

                text2dTransform[k][curRotation][j]->setTranslation(planeTranslation[k]+text2dTr[k][curRotation][j]);
                }
            }
        }
        matrixShowLine(curRotationInd);

        groupAnimation->start();

        curRotation++;
        return;
    }
    matrixShowLine();
    curRotation=-1;
    startButton->setEnabled(true);
    startButton->setText(startButtonTexts[0]);
    tab->setEnabled(true);
}

void SceneModifier::matrixInit(bool zeros){
    QQuaternion current;
    for(int i=0; i<2; ++i){
        for(int j=0; j<3; ++j){
            if(!zeros){
                //i==k,i==(!k),i==2
                current=QQuaternion::fromAxisAndAngle(i==j,i!=j,j==2, angle[i][j]);
                current.normalize();
            }
            angleMatrix[i][j]=current.toRotationMatrix();
        }
    }
    for(int j=0; j<3; ++j){
        if(!zeros){
            //i==k,i==(!k),i==2
            current=QQuaternion::nlerp(QQuaternion(),quat,(j+1.0f)/3.0f)/
                    QQuaternion::nlerp(QQuaternion(),quat,j/3.0f);
            current.normalize();
        }
        angleMatrix[2][j]=current.toRotationMatrix();
    }
}

void SceneModifier::matrixClearShow(){
    QString labelText;
    for(int i=0;i<3;++i){
        for(int j=0; j<3;++j){
            matrixLabel[j][i]->setText(labelText);
        }
    }
}

void SceneModifier::matrixShowLine(int matrixLine){
    //angleMatrix[3][3];
    QString labelText;
    for(int i=0; i<3; ++i){//all axes
        int j=0, jmax=3;//default case, matrixLine = -1, three matrix for each axe

        if(matrixLine!=-1){
            j=matrixLine;
            jmax=j+1;
        }

        for(; j<jmax; ++j){
            labelText="";
            for(int k=0; k<3; ++k){//3 lines in matrix
                for(int m=0; m<3; ++m){
                    //previous + space if value is positive
                    labelText=labelText % (angleMatrix[i][j](k,m)>=0?" ":"") % QString::number(angleMatrix[i][j](k,m),'f',3) % " ";
                    // + value.000 + space if it's not last val in line or \n if it is
                }
                labelText=labelText % '\n';
            }
            labelText.remove(labelText.length()-2, 2);
            matrixLabel[j][i]->setText(labelText);
        }
    }
}

void SceneModifier::matrixCinemShow(){
    angleMatrix[1][1]=cinemQuat.toRotationMatrix();
    QString labelText="";
    for(int k=0; k<3; ++k){//3 lines in matrix
        for(int m=0; m<3; ++m){
            //previous + space if value is positive
            labelText=labelText % (angleMatrix[1][1](k,m)>=0?" ":"") % QString::number(angleMatrix[1][1](k,m),'f',3) % " ";
            // + value.000 + space if it's not last val in line or \n if it is
        }
        labelText=labelText % '\n';
    }
    labelText.remove(labelText.length()-2, 2);
    matrixLabel[1][1]->setText(labelText);
}
