#ifndef CAD_MATH_H
#define CAD_MATH_H

#include "cad_types.h"

int StringEqualsIgnoreCase(const char *s1, const char *s2);
Vector2 LocalToWorldPoint(Vector2 localPos, Vector2 origin, float rotationDeg);
Vector2 WorldToLocalPoint(Vector2 worldPos, Vector2 origin, float rotationDeg);
bool AABBIntersectsAABB(AABB a, AABB b);
bool AABBContainsAABB(AABB parent, AABB child);
bool AABBContainsPoint(AABB a, Vector2 p);
AABB ExpandAABB(AABB box, float margin);
float SnapAngle(float angleDeg, bool isSnapActive);
bool Calculate3PointArc(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 *center, float *radius, float *startAngle, float *endAngle);
void GetElementLocalExtents(const GridElement *el, float *minX, float *maxX, float *minY, float *maxY);
void GetLocalControlNodePositions(const GridElement *el, Vector2 nodes[8]);
Vector2 GetLocalRotationHandlePosition(const GridElement *el, float zoom);
void GetElementSnapLines(const GridElement *el, float xOut[8], int *xCount, float yOut[8], int *yCount);
void GetElementSnapPoints(const GridElement *el, Vector2 points[MAX_POLYLINE_POINTS], int *pointCount);
AABB GetElementAABB(GridElement *el);
HandleType HitTestHandles(const GridElement *el, Vector2 worldPos, float zoom);
void GetUnitConvertedLength(float baseLength, MeasureUnit unit, float *outLength, const char **outUnitStr);

#endif // CAD_MATH_H