#include "cad_math.h"
#include "raymath.h"
#include <math.h>
#include <ctype.h>
#include <float.h>
#include <string.h>

CADGlobalState g_CADState = {
    .activeLayerIndex = 0,
    .activeLayerId = 1,
    .activeTool = TOOL_SELECT,
    .mouseScreen = { 0.0f, 0.0f },
    .mouseWorld = { 0.0f, 0.0f }
};

static unsigned int g_NextEntityID = 1;
unsigned int GenerateEntityID(void) {
    return g_NextEntityID++;
}

void SetNextEntityID(unsigned int id) {
    g_NextEntityID = id;
}

static unsigned int g_NextLayerID = 1;
unsigned int GenerateLayerID(void) {
    return g_NextLayerID++;
}

void SetNextLayerID(unsigned int id) {
    g_NextLayerID = id;
}

int StringEqualsIgnoreCase(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) return 0;
        s1++; s2++;
    }
    return *s1 == *s2;
}

Vector2 LocalToWorldPoint(Vector2 localPos, Vector2 origin, float rotationDeg) {
    float rad = rotationDeg * DEG2RAD;
    Vector2 rotated = Vector2Rotate(localPos, rad);
    return Vector2Add(origin, rotated);
}

Vector2 WorldToLocalPoint(Vector2 worldPos, Vector2 origin, float rotationDeg) {
    Vector2 diff = Vector2Subtract(worldPos, origin);
    float rad = -rotationDeg * DEG2RAD;
    return Vector2Rotate(diff, rad);
}

bool AABBIntersectsAABB(AABB a, AABB b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x &&
            a.min.y <= b.max.y && a.max.y >= b.min.y);
}

bool AABBContainsAABB(AABB parent, AABB child) {
    return (child.min.x >= parent.min.x && child.max.x <= parent.max.x &&
            child.min.y >= parent.min.y && child.max.y <= parent.max.y);
}

bool AABBContainsPoint(AABB a, Vector2 p) {
    return (p.x >= a.min.x && p.x <= a.max.x && p.y >= a.min.y && p.y <= a.max.y);
}

AABB ExpandAABB(AABB box, float margin) {
    return (AABB){
        .min = { box.min.x - margin, box.min.y - margin },
        .max = { box.max.x + margin, box.max.y + margin }
    };
}

float SnapAngle(float angleDeg, bool isSnapActive) {
    while (angleDeg < 0.0f) angleDeg += 360.0f;
    while (angleDeg >= 360.0f) angleDeg -= 360.0f;
    if (!isSnapActive) return angleDeg;
    const float snapSteps[] = { 90.0f, 45.0f, 15.0f };
    const float threshold = 5.0f;
    for (int i = 0; i < 3; i++) {
        float step = snapSteps[i];
        float nearest = roundf(angleDeg / step) * step;
        if (fabsf(angleDeg - nearest) <= threshold) {
            if (nearest >= 360.0f) nearest -= 360.0f;
            return nearest;
        }
    }
    return angleDeg;
}

bool Calculate3PointArc(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 *center, float *radius, float *startAngle, float *endAngle) {
    float temp = p2.x * p2.x + p2.y * p2.y;
    float bc = (p1.x * p1.x + p1.y * p1.y - temp) * 0.5f;
    float cd = (temp - p3.x * p3.x - p3.y * p3.y) * 0.5f;
    float det = (p1.x - p2.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p2.y);
    if (fabsf(det) < 0.0001f) return false;
    float invDet = 1.0f / det;
    center->x = (bc * (p2.y - p3.y) - cd * (p1.y - p2.y)) * invDet;
    center->y = ((p1.x - p2.x) * cd - (p2.x - p3.x) * bc) * invDet;
    *radius = Vector2Distance(*center, p1);
    float a1 = atan2f(p1.y - center->y, p1.x - center->x) * RAD2DEG;
    float a3 = atan2f(p3.y - center->y, p3.x - center->x) * RAD2DEG;
    if (a1 < 0) a1 += 360.0f;
    if (a3 < 0) a3 += 360.0f;
    *startAngle = a1;
    *endAngle = a3;
    return true;
}

void GetLocalControlNodePositions(const GridElement *el, Vector2 nodes[8]) {
    float halfW = (el->type == ELEMENT_CIRCLE) ? el->radius : (el->type == ELEMENT_ELLIPSE ? el->radiusX : el->width * 0.5f);
    float halfH = (el->type == ELEMENT_CIRCLE) ? el->radius : (el->type == ELEMENT_ELLIPSE ? el->radiusY : el->height * 0.5f);
    nodes[HANDLE_TOP_LEFT]     = (Vector2){ -halfW, -halfH };
    nodes[HANDLE_TOP_CENTER]   = (Vector2){  0.0f,  -halfH };
    nodes[HANDLE_TOP_RIGHT]    = (Vector2){  halfW, -halfH };
    nodes[HANDLE_RIGHT_CENTER] = (Vector2){  halfW,  0.0f  };
    nodes[HANDLE_BOTTOM_RIGHT] = (Vector2){  halfW,  halfH };
    nodes[HANDLE_BOTTOM_CENTER]= (Vector2){  0.0f,   halfH };
    nodes[HANDLE_BOTTOM_LEFT]  = (Vector2){ -halfW,  halfH };
    nodes[HANDLE_LEFT_CENTER]  = (Vector2){ -halfW,  0.0f  };
}

Vector2 GetLocalRotationHandlePosition(const GridElement *el, float zoom) {
    float halfH = (el->type == ELEMENT_CIRCLE) ? el->radius : (el->type == ELEMENT_ELLIPSE ? el->radiusY : el->height * 0.5f);
    float offset = ROTATION_HANDLE_OFFSET / zoom;
    return (Vector2){ 0.0f, -halfH - offset };
}

void GetElementSnapLines(const GridElement *el, float xOut[8], int *xCount, float yOut[8], int *yCount) {
    *xCount = 0;
    *yCount = 0;
    if (el->type == ELEMENT_RECT || el->type == ELEMENT_CIRCLE || el->type == ELEMENT_ELLIPSE || el->type == ELEMENT_TEXT_NOTE) {
        float halfW = (el->type == ELEMENT_CIRCLE) ? el->radius : (el->type == ELEMENT_ELLIPSE ? el->radiusX : el->width * 0.5f);
        float halfH = (el->type == ELEMENT_CIRCLE) ? el->radius : (el->type == ELEMENT_ELLIPSE ? el->radiusY : el->height * 0.5f);
        Vector2 localPts[5] = {
            { 0, 0 }, { -halfW, -halfH }, { halfW, -halfH },
            { -halfW, halfH }, { halfW, halfH }
        };
        for (int i = 0; i < 5; i++) {
            Vector2 worldPt = LocalToWorldPoint(localPts[i], el->pos, el->rotation);
            if (*xCount < 8) xOut[(*xCount)++] = worldPt.x;
            if (*yCount < 8) yOut[(*yCount)++] = worldPt.y;
        }
    } else if (el->type == ELEMENT_LINE || el->type == ELEMENT_DIMENSION) {
        xOut[0] = el->p1.x; xOut[1] = (el->p1.x + el->p2.x) * 0.5f; xOut[2] = el->p2.x;
        *xCount = 3;
        yOut[0] = el->p1.y; yOut[1] = (el->p1.y + el->p2.y) * 0.5f; yOut[2] = el->p2.y;
        *yCount = 3;
    } else if (el->type == ELEMENT_SYMBOL) {
        if (strchr(el->text, '|') != NULL) {
            float fw = el->width;
            float fh = el->height;
            float ft = el->radius;
            if (fw <= 0.0f) fw = 19.1f;
            if (fh <= 0.0f) fh = 152.4f;
            if (ft <= 0.0f) ft = 63.5f;
            float leftX = fw - ft;
            float rightX = fw;
            float midX = fw - (ft * 0.5f);
            float topY = -fh * 0.5f;
            float botY = fh * 0.5f;
            Vector2 pts[8] = {
                { leftX, topY },
                { midX,  topY },
                { rightX, topY },
                { rightX, 0.0f },
                { rightX, botY },
                { midX,  botY },
                { leftX, botY },
                { leftX, 0.0f }
            };
            for (int i = 0; i < 8; i++) {
                Vector2 wpt = LocalToWorldPoint(pts[i], el->pos, el->rotation);
                if (*xCount < 8) xOut[(*xCount)++] = wpt.x;
                if (*yCount < 8) yOut[(*yCount)++] = wpt.y;
            }
        } else {
            xOut[0] = el->pos.x;
            *xCount = 1;
            yOut[0] = el->pos.y;
            *yCount = 1;
        }
    }
}

void GetElementSnapPoints(const GridElement *el, Vector2 points[MAX_POLYLINE_POINTS], int *pointCount) {
    *pointCount = 0;
    if (el->type == ELEMENT_RECT || el->type == ELEMENT_TEXT_NOTE) {
        Vector2 localNodes[8];
        GetLocalControlNodePositions(el, localNodes);
        points[0] = el->pos;
        for (int i = 0; i < 8; i++) {
            points[i + 1] = LocalToWorldPoint(localNodes[i], el->pos, el->rotation);
        }
        *pointCount = 9;
    } else if (el->type == ELEMENT_CIRCLE || el->type == ELEMENT_ELLIPSE) {
        points[0] = el->pos;
        Vector2 localNodes[8];
        GetLocalControlNodePositions(el, localNodes);
        points[1] = LocalToWorldPoint(localNodes[HANDLE_LEFT_CENTER], el->pos, el->rotation);
        points[2] = LocalToWorldPoint(localNodes[HANDLE_RIGHT_CENTER], el->pos, el->rotation);
        points[3] = LocalToWorldPoint(localNodes[HANDLE_TOP_CENTER], el->pos, el->rotation);
        points[4] = LocalToWorldPoint(localNodes[HANDLE_BOTTOM_CENTER], el->pos, el->rotation);
        *pointCount = 5;
    } else if (el->type == ELEMENT_DIMENSION || el->type == ELEMENT_LINE) {
        points[0] = el->p1;
        points[1] = el->p2;
        points[2] = (Vector2){ (el->p1.x + el->p2.x) * 0.5f, (el->p1.y + el->p2.y) * 0.5f };
        *pointCount = 3;
    } else if (el->type == ELEMENT_POLYLINE || el->type == ELEMENT_FREEHAND) {
        for (int i = 0; i < el->pointCount && i < MAX_POLYLINE_POINTS; i++) {
            points[i] = el->points[i];
        }
        *pointCount = el->pointCount;
    } else if (el->type == ELEMENT_ARC) {
        points[0] = el->p1; points[1] = el->p2; points[2] = el->p3; points[3] = el->pos;
        *pointCount = 4;
    } else if (el->type == ELEMENT_SYMBOL) {
        if (strchr(el->text, '|') != NULL) {
            float fw = el->width;
            float fh = el->height;
            float ft = el->radius;
            if (fw <= 0.0f) fw = 19.1f;
            if (fh <= 0.0f) fh = 152.4f;
            if (ft <= 0.0f) ft = 63.5f;
            float leftX = fw - ft;
            float rightX = fw;
            float midX = fw - (ft * 0.5f);
            float topY = -fh * 0.5f;
            float botY = fh * 0.5f;

            // 8 Perimeter Snap Points matching the bounding envelope
            Vector2 local8[8] = {
                { leftX,  topY },
                { midX,   topY },
                { rightX, topY },
                { rightX, 0.0f },
                { rightX, botY },
                { midX,   botY },
                { leftX,  botY },
                { leftX,  0.0f }
            };
            for (int i = 0; i < 8; i++) {
                points[i] = LocalToWorldPoint(local8[i], el->pos, el->rotation);
            }
            *pointCount = 8;
        } else {
            points[0] = el->pos;
            *pointCount = 1;
        }
    }
}

AABB GetElementAABB(GridElement *el) {
    AABB box;
    box.min = (Vector2){ FLT_MAX, FLT_MAX };
    box.max = (Vector2){ -FLT_MAX, -FLT_MAX };
    switch (el->type) {
        case ELEMENT_RECT:
        case ELEMENT_TEXT_NOTE: {
            float hw = el->width * 0.5f;
            float hh = el->height * 0.5f;
            Vector2 corners[4] = {
                LocalToWorldPoint((Vector2){ -hw, -hh }, el->pos, el->rotation),
                LocalToWorldPoint((Vector2){  hw, -hh }, el->pos, el->rotation),
                LocalToWorldPoint((Vector2){  hw,  hh }, el->pos, el->rotation),
                LocalToWorldPoint((Vector2){ -hw,  hh }, el->pos, el->rotation)
            };
            for (int i = 0; i < 4; i++) {
                box.min.x = fminf(box.min.x, corners[i].x);
                box.min.y = fminf(box.min.y, corners[i].y);
                box.max.x = fmaxf(box.max.x, corners[i].x);
                box.max.y = fmaxf(box.max.y, corners[i].y);
            }
            if (el->type == ELEMENT_TEXT_NOTE && el->showArrow) {
                box.min.x = fminf(box.min.x, el->arrowTarget.x - 10.0f);
                box.min.y = fminf(box.min.y, el->arrowTarget.y - 10.0f);
                box.max.x = fmaxf(box.max.x, el->arrowTarget.x + 10.0f);
                box.max.y = fmaxf(box.max.y, el->arrowTarget.x + 10.0f);
            }
            break;
        }
        case ELEMENT_CIRCLE: {
            box.min = (Vector2){ el->pos.x - el->radius, el->pos.y - el->radius };
            box.max = (Vector2){ el->pos.x + el->radius, el->pos.y + el->radius };
            break;
        }
        case ELEMENT_ELLIPSE: {
            float maxR = fmaxf(el->radiusX, el->radiusY);
            box.min = (Vector2){ el->pos.x - maxR, el->pos.y - maxR };
            box.max = (Vector2){ el->pos.x + maxR, el->pos.y + maxR };
            break;
        }
        case ELEMENT_LINE: {
            box.min.x = fminf(el->p1.x, el->p2.x);
            box.min.y = fminf(el->p1.y, el->p2.y);
            box.max.x = fmaxf(el->p1.x, el->p2.x);
            box.max.y = fmaxf(el->p1.y, el->p2.y);
            box = ExpandAABB(box, fmaxf(el->lineThickness * 0.5f, 5.0f));
            break;
        }
        case ELEMENT_DIMENSION: {
            box.min.x = fminf(fminf(el->p1.x, el->p2.x), el->dimPos.x);
            box.min.y = fminf(fminf(el->p1.y, el->p2.y), el->dimPos.y);
            box.max.x = fmaxf(fmaxf(el->p1.x, el->p2.x), el->dimPos.x);
            box.max.y = fmaxf(fmaxf(el->p1.y, el->p2.y), el->dimPos.y);
            box = ExpandAABB(box, 30.0f);
            break;
        }
        case ELEMENT_POLYLINE:
        case ELEMENT_FREEHAND: {
            for (int i = 0; i < el->pointCount; i++) {
                box.min.x = fminf(box.min.x, el->points[i].x);
                box.min.y = fminf(box.min.y, el->points[i].y);
                box.max.x = fmaxf(box.max.x, el->points[i].x);
                box.max.y = fmaxf(box.max.y, el->points[i].y);
            }
            box = ExpandAABB(box, fmaxf(el->lineThickness * 0.5f, 5.0f));
            break;
        }
        case ELEMENT_ARC: {
            box.min = (Vector2){ el->pos.x - el->radius, el->pos.y - el->radius };
            box.max = (Vector2){ el->pos.x + el->radius, el->pos.y + el->radius };
            box = ExpandAABB(box, 15.0f);
            break;
        }
        case ELEMENT_SYMBOL: {
            if (strchr(el->text, '|') != NULL) {
                float fw = el->width * el->scale.x;
                float fh = el->height * el->scale.y;
                float ft = el->radius * el->scale.x;
                Vector2 pts[6] = {
                    { 0.0f, -fh * 0.5f },
                    { fw,   -fh * 0.5f },
                    { fw,    fh * 0.5f },
                    { 0.0f,  fh * 0.5f },
                    { fw - ft, -fw * 0.5f },
                    { fw - ft,  fw * 0.5f }
                };
                for (int p = 0; p < 6; p++) {
                    Vector2 wPt = LocalToWorldPoint(pts[p], el->pos, el->rotation);
                    box.min.x = fminf(box.min.x, wPt.x);
                    box.min.y = fminf(box.min.y, wPt.y);
                    box.max.x = fmaxf(box.max.x, wPt.x);
                    box.max.y = fmaxf(box.max.y, wPt.y);
                }
                box = ExpandAABB(box, 5.0f);
            } else {
                box.min = (Vector2){ el->pos.x - 20.0f * el->scale.x, el->pos.y - 20.0f * el->scale.y };
                box.max = (Vector2){ el->pos.x + 20.0f * el->scale.x, el->pos.y + 20.0f * el->scale.y };
            }
            break;
        }
    }
    if (el) el->bbox = box;
    return box;
}

HandleType HitTestHandles(const GridElement *el, Vector2 worldPos, float zoom) {
    if (!el->selected || (el->type != ELEMENT_RECT && el->type != ELEMENT_CIRCLE && el->type != ELEMENT_ELLIPSE && el->type != ELEMENT_TEXT_NOTE)) return HANDLE_NONE;
    float hitRadius = (HANDLE_SIZE_PX * 1.5f) / zoom;
    Vector2 rotLocal = GetLocalRotationHandlePosition(el, zoom);
    Vector2 rotWorld = LocalToWorldPoint(rotLocal, el->pos, el->rotation);
    if (Vector2Distance(worldPos, rotWorld) <= hitRadius) return HANDLE_ROTATION;
    Vector2 localNodes[8];
    GetLocalControlNodePositions(el, localNodes);
    for (int i = 0; i < 8; i++) {
        Vector2 nodeWorld = LocalToWorldPoint(localNodes[i], el->pos, el->rotation);
        if (Vector2Distance(worldPos, nodeWorld) <= hitRadius) return (HandleType)i;
    }
    return HANDLE_NONE;
}

void GetUnitConvertedLength(float baseLength, MeasureUnit unit, float *outLength, const char **outUnitStr) {
    switch (unit) {
        case UNIT_MM: *outLength = baseLength; *outUnitStr = "mm"; break;
        case UNIT_CM: *outLength = baseLength * 0.1f; *outUnitStr = "cm"; break;
        case UNIT_M:  *outLength = baseLength * 0.001f; *outUnitStr = "m"; break;
        case UNIT_IN: *outLength = baseLength * 0.0393701f; *outUnitStr = "in"; break;
        case UNIT_FT: *outLength = baseLength * 0.00328084f; *outUnitStr = "ft"; break;
    }
}