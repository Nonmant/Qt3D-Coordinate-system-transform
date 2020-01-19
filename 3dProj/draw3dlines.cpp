#include "draw3dlines.h"
using namespace std;

void drawLine(const QVector3D& start, const QVector3D& end, const QColor& color, Qt3DCore::QEntity *_rootEntity)
{
    auto *geometry = new Qt3DRender::QGeometry(_rootEntity);

    // position vertices (start and end)
    QByteArray bufferBytes;
    bufferBytes.resize(3 * 2 * sizeof(float)); // start.x, start.y, start.end + end.x, end.y, end.z
    float *positions = reinterpret_cast<float*>(bufferBytes.data());
    *positions++ = start.x();
    *positions++ = start.y();
    *positions++ = start.z();
    *positions++ = end.x();
    *positions++ = end.y();
    *positions++ = end.z();

    auto *buf = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
    buf->setData(bufferBytes);

    auto *positionAttribute = new Qt3DRender::QAttribute(geometry);
    positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
    positionAttribute->setVertexSize(3);
    positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(buf);
    positionAttribute->setByteStride(3 * sizeof(float));
    positionAttribute->setCount(2);
    geometry->addAttribute(positionAttribute); // We add the vertices in the geometry

    // connectivity between vertices
    QByteArray indexBytes;
    indexBytes.resize(2 * sizeof(unsigned int)); // start to end
    unsigned int *indices = reinterpret_cast<unsigned int*>(indexBytes.data());
    *indices++ = 0;
    *indices++ = 1;

    auto *indexBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, geometry);
    indexBuffer->setData(indexBytes);

    auto *indexAttribute = new Qt3DRender::QAttribute(geometry);
    indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedInt);
    indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    indexAttribute->setBuffer(indexBuffer);
    indexAttribute->setCount(2);
    geometry->addAttribute(indexAttribute); // We add the indices linking the points in the geometry

    // mesh
    auto *line = new Qt3DRender::QGeometryRenderer(_rootEntity);
    line->setGeometry(geometry);
    line->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    auto *material = new Qt3DExtras::QPhongMaterial(_rootEntity);
    material->setAmbient(color);

    // entity
    auto *lineEntity = new Qt3DCore::QEntity(_rootEntity);
    lineEntity->addComponent(line);
    lineEntity->addComponent(material);
}

Qt3DCore::QEntity* drawLine(const QVector<QVector3D>& points, const QColor& color,Qt3DCore::QEntity *_rootEntity){
    auto *geometry = new Qt3DRender::QGeometry(_rootEntity);

    int count = points.length();
    uint uCount = uint(count);

    // position vertices
    QByteArray bufferBytes;

    bufferBytes.resize(uCount * 3 * sizeof(float));
    float *positions = reinterpret_cast<float*>(bufferBytes.data());
    for(int i=0; i<count; ++i){
        *positions++ = points[i].x();
        *positions++ = points[i].y();
        *positions++ = points[i].z();
    }

    auto *buf = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
    buf->setData(bufferBytes);

    auto *positionAttribute = new Qt3DRender::QAttribute(geometry);
    positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
    positionAttribute->setVertexSize(3);
    positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(buf);
    positionAttribute->setByteStride(3 * sizeof(float));
    positionAttribute->setCount(uCount);
    geometry->addAttribute(positionAttribute); // We add the vertices in the geometry

    // connectivity between vertices
    QByteArray indexBytes;
    indexBytes.resize((uCount * 2 - 2) * sizeof(unsigned int)); // start to end
    unsigned int *indices = reinterpret_cast<unsigned int*>(indexBytes.data());

    //QString nums="";
    for(uint i=0, j=1; i+j<uCount;){
        *indices++ = i;
        //nums+=QString::number(i);
        i+=j;
        j=(j)?0:1;
    }

    auto *indexBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, geometry);
    indexBuffer->setData(indexBytes);

    auto *indexAttribute = new Qt3DRender::QAttribute(geometry);
    indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedInt);
    indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    indexAttribute->setBuffer(indexBuffer);
    indexAttribute->setCount(uCount * 2 - 2);
    geometry->addAttribute(indexAttribute); // We add the indices linking the points in the geometry

    // mesh
    auto *line = new Qt3DRender::QGeometryRenderer(_rootEntity);
    line->setGeometry(geometry);
    line->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    auto *material = new Qt3DExtras::QPhongMaterial(_rootEntity);
    material->setAmbient(color);

    // entity
    auto *lineEntity = new Qt3DCore::QEntity(_rootEntity);
    lineEntity->addComponent(line);
    lineEntity->addComponent(material);
    return lineEntity;
}

Qt3DCore::QEntity* drawSegment(const float radiusIn,
                               float angleIn,
                               float angleInStart,
                               const QColor& color,
                               Qt3DCore::QEntity *_rootEntity,
                               const Draw3DLines::PlaneName planeName,//xy,xz,yz
                               float tol,
                               const Draw3DLines::TolMod tolMod,
                               const Draw3DLines::AngleMod angleMod
                               ){
    if(angleMod==Draw3DLines::deg){//if it's degrees - convert to radians
        angleIn=qDegreesToRadians(angleIn);
        angleInStart=qDegreesToRadians(angleInStart);
    }

    if(angleInStart>angleIn){
        float temp=angleIn;
        angleIn=angleInStart;
        angleInStart=temp;
    }

    float stepAngle;
    switch (tolMod) {
    case Draw3DLines::linSize:{
        stepAngle=acos(1-0.5f*(tol/radiusIn)*(tol/radiusIn));
        break;
    }
    case Draw3DLines::angDev:{
        if(angleMod==Draw3DLines::deg){
            tol=qDegreesToRadians(tol);
        }
        stepAngle=2*tol;
        break;
    }
    default:{//linDev
        stepAngle=4*asin(sqrt(tol/radiusIn));
        break;
    }
    }

    int stepsMax=int(ceil((angleIn-angleInStart)/stepAngle));
    stepAngle=(angleIn-angleInStart)/stepsMax;

    QVector<QVector3D> points;
    points.resize(stepsMax+1);

    float angle=0, x, y, z;

    switch (planeName) {
    case Draw3DLines::xz:{
        for(int i=0; i<=stepsMax; ++i){
            angle=(i!=stepsMax)?(angleInStart + stepAngle*i):angleIn;
            x=radiusIn*sin(angle);
            y=0;
            z=radiusIn*cos(angle);
            points[i]=QVector3D(x,y,z);
        }
        break;
    }
    case Draw3DLines::yz:{
        for(int i=0; i<=stepsMax; ++i){
            angle=(i!=stepsMax)?(angleInStart + stepAngle*i):angleIn;
            x=0;
            y=radiusIn*cos(angle);
            z=radiusIn*sin(angle);
            points[i]=QVector3D(x,y,z);
        }
        break;
    }
    default:{//xy
        for(int i=0; i<=stepsMax; ++i){
            angle=(i!=stepsMax)?(angleInStart + stepAngle*i):angleIn;
            x=radiusIn*cos(angle);
            y=radiusIn*sin(angle);
            z=0;
            points[i]=QVector3D(x,y,z);
        }
        break;
    }
    }

    return drawLine(points, color, _rootEntity);
}
