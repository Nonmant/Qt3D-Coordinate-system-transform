#ifndef DRAW3DLINES_H
#define DRAW3DLINES_H

#include <QVector3D>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DExtras/QPhongMaterial>
#include <QtMath>

namespace Draw3DLines {
enum PlaneName{xy = 'x'+'y', xz = 'x'+'z', yz = 'y'+'z'};
enum TolMod{linSize=0, linDev=1, angDev=2};
enum AngleMod{deg='d', rad='r'};
}

//draws single line between two points
void drawLine(const QVector3D& start,
              const QVector3D& end,
              const QColor& color,
              Qt3DCore::QEntity *_rootEntity);

//draws polyline by vector of points
Qt3DCore::QEntity* drawLine(const QVector<QVector3D>& points,
                           const QColor& color,
                           Qt3DCore::QEntity *_rootEntity=nullptr);

//draws segment of plate circle as polyline.
//angleMod=0 for degree value of angleIn, =1 for radian
Qt3DCore::QEntity* drawSegment(const float radiusIn,
                               float angleIn,
                               float angleInStart,
                               const QColor& color,
                               Qt3DCore::QEntity *_rootEntity=nullptr,
                               const Draw3DLines::PlaneName planeName=Draw3DLines::xy,//xy,xz,yz
                               float tol=0.05f,
                               const Draw3DLines::TolMod tolMod = Draw3DLines::linDev,
                               const Draw3DLines::AngleMod angleMod=Draw3DLines::deg
                               );
/*
template <typename T> inline constexpr
int sign(T val) {
    return (T(0) < val) - (val < T(0));
}
*/
template <typename T> inline constexpr
int sign(T x, std::false_type is_signed) {
    return T(0) < x;
}

template <typename T> inline constexpr
int sign(T x, std::true_type is_signed) {
    return (T(0) < x) - (x < T(0));
}

template <typename T> inline constexpr
int sign(T x) {
    return sign(x, std::is_signed<T>());
}



#endif // DRAW3DLINES_H
