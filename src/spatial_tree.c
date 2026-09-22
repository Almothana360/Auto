#include "spatial_tree.h"
#include "cad_math.h"
#include "raymath.h"
#include <string.h>
#include <math.h>

static void QuadTree_InitNode(SpatialQuadTree *tree, int nodeIdx, AABB bounds) {
    tree->nodes[nodeIdx].bounds = bounds;
    tree->nodes[nodeIdx].count = 0;
    tree->nodes[nodeIdx].isLeaf = true;
    for (int i = 0; i < 4; i++) tree->nodes[nodeIdx].children[i] = -1;
}

static void QuadTree_Subdivide(SpatialQuadTree *tree, int nodeIdx) {
    if (tree->nodeCount + 4 > QUADTREE_MAX_NODES) return;

    Vector2 min = tree->nodes[nodeIdx].bounds.min;
    Vector2 max = tree->nodes[nodeIdx].bounds.max;
    Vector2 mid = { (min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f };

    AABB childBounds[4] = {
        { { min.x, min.y }, { mid.x, mid.y } },
        { { mid.x, min.y }, { max.x, mid.y } },
        { { min.x, mid.y }, { mid.x, max.y } },
        { { mid.x, mid.y }, { max.x, max.y } }
    };

    for (int i = 0; i < 4; i++) {
        int cIdx = tree->nodeCount++;
        QuadTree_InitNode(tree, cIdx, childBounds[i]);
        tree->nodes[nodeIdx].children[i] = cIdx;
    }
    tree->nodes[nodeIdx].isLeaf = false;
}

static void QuadTree_Insert(SpatialQuadTree *tree, int nodeIdx, int elementIdx, const AABB *elementAABB, int depth) {
    QuadTreeNode *node = &tree->nodes[nodeIdx];

    if (!node->isLeaf) {
        for (int i = 0; i < 4; i++) {
            int cIdx = node->children[i];
            if (cIdx != -1 && AABBContainsAABB(tree->nodes[cIdx].bounds, *elementAABB)) {
                QuadTree_Insert(tree, cIdx, elementIdx, elementAABB, depth + 1);
                return;
            }
        }
    }

    if (node->count < QUADTREE_NODE_CAPACITY || depth >= QUADTREE_MAX_DEPTH) {
        if (node->count < QUADTREE_NODE_CAPACITY) {
            node->elementIndices[node->count++] = elementIdx;
            return;
        }
    }

    if (node->isLeaf && depth < QUADTREE_MAX_DEPTH) {
        QuadTree_Subdivide(tree, nodeIdx);
        if (!tree->nodes[nodeIdx].isLeaf) {
            int remaining = 0;
            int tempIndices[QUADTREE_NODE_CAPACITY];
            memcpy(tempIndices, tree->nodes[nodeIdx].elementIndices, sizeof(int) * tree->nodes[nodeIdx].count);
            int prevCount = tree->nodes[nodeIdx].count;
            tree->nodes[nodeIdx].count = 0;

            for (int e = 0; e < prevCount; e++) {
                int existingIdx = tempIndices[e];
                bool pushed = false;
                for (int i = 0; i < 4; i++) {
                    int cIdx = tree->nodes[nodeIdx].children[i];
                    if (cIdx != -1 && AABBContainsAABB(tree->nodes[cIdx].bounds, *elementAABB)) {
                        QuadTree_Insert(tree, cIdx, existingIdx, elementAABB, depth + 1);
                        pushed = true;
                        break;
                    }
                }
                if (!pushed) tempIndices[remaining++] = existingIdx;
            }
            memcpy(tree->nodes[nodeIdx].elementIndices, tempIndices, sizeof(int) * remaining);
            tree->nodes[nodeIdx].count = remaining;

            QuadTree_Insert(tree, nodeIdx, elementIdx, elementAABB, depth);
            return;
        }
    }

    if (node->count < QUADTREE_NODE_CAPACITY) {
        node->elementIndices[node->count++] = elementIdx;
    }
}

void SpatialIndex_Build(SpatialQuadTree *tree, const GridElement *elements, const AABB *cachedAABBs, int elementCount) {
    (void)elements;
    tree->nodeCount = 0;
    AABB worldBounds = {
        .min = { -SPATIAL_WORLD_EXTENT, -SPATIAL_WORLD_EXTENT },
        .max = {  SPATIAL_WORLD_EXTENT,  SPATIAL_WORLD_EXTENT }
    };
    QuadTree_InitNode(tree, tree->nodeCount++, worldBounds);

    for (int i = 0; i < elementCount; i++) {
        QuadTree_Insert(tree, 0, i, &cachedAABBs[i], 0);
    }
}

static bool CheckSingleElementHit(const GridElement *el, Vector2 worldPos) {
    Vector2 localPos = WorldToLocalPoint(worldPos, el->pos, el->rotation);

    if (el->type == ELEMENT_RECT || el->type == ELEMENT_TEXT_NOTE) {
        float halfW = el->width * 0.5f;
        float halfH = el->height * 0.5f;
        return (localPos.x >= -halfW && localPos.x <= halfW && localPos.y >= -halfH && localPos.y <= halfH);
    } else if (el->type == ELEMENT_CIRCLE) {
        return (Vector2Length(localPos) <= el->radius);
    } else if (el->type == ELEMENT_ELLIPSE) {
        float normX = localPos.x / el->radiusX;
        float normY = localPos.y / el->radiusY;
        return (normX * normX + normY * normY <= 1.0f);
    } else if (el->type == ELEMENT_DIMENSION) {
        return (Vector2Distance(worldPos, el->dimPos) <= 30.0f);
    } else if (el->type == ELEMENT_LINE) {
        Vector2 p1 = el->p1, p2 = el->p2;
        float l2 = Vector2DistanceSqr(p1, p2);
        if (l2 == 0.0f) return (Vector2Distance(worldPos, p1) < 10.0f);
        float t = Vector2DotProduct(Vector2Subtract(worldPos, p1), Vector2Subtract(p2, p1)) / l2;
        t = fmaxf(0.0f, fminf(1.0f, t));
        Vector2 proj = Vector2Add(p1, Vector2Scale(Vector2Subtract(p2, p1), t));
        return (Vector2Distance(worldPos, proj) < 10.0f);
    } else if (el->type == ELEMENT_POLYLINE || el->type == ELEMENT_FREEHAND) {
        for (int p = 0; p < el->pointCount - 1; p++) {
            Vector2 p1 = el->points[p], p2 = el->points[p + 1];
            float l2 = Vector2DistanceSqr(p1, p2);
            if (l2 == 0.0f) continue;
            float t = Vector2DotProduct(Vector2Subtract(worldPos, p1), Vector2Subtract(p2, p1)) / l2;
            t = fmaxf(0.0f, fminf(1.0f, t));
            Vector2 proj = Vector2Add(p1, Vector2Scale(Vector2Subtract(p2, p1), t));
            if (Vector2Distance(worldPos, proj) < 10.0f) return true;
        }
    } else if (el->type == ELEMENT_ARC) {
        float dist = Vector2Distance(worldPos, el->pos);
        return (fabsf(dist - el->radius) < 12.0f);
    } else if (el->type == ELEMENT_SYMBOL) {
        return (Vector2Distance(worldPos, el->pos) < 20.0f * fmaxf(el->scale.x, el->scale.y));
    }
    return false;
}

static void HitTestElement_Spatial_Rec(const SpatialQuadTree *tree, int nodeIdx, const GridElement *elements, const Layer *layers, int layerCount, Vector2 worldPos, AABB queryBox, int *bestHit) {
    const QuadTreeNode *node = &tree->nodes[nodeIdx];
    if (!AABBIntersectsAABB(node->bounds, queryBox)) return;

    for (int i = 0; i < node->count; i++) {
        int idx = node->elementIndices[i];
        if (idx <= *bestHit) continue;

        int lIdx = elements[idx].layerIndex;
        if (lIdx >= 0 && lIdx < layerCount) {
            if (!layers[lIdx].visible || layers[lIdx].locked) continue;
        }

        if (CheckSingleElementHit(&elements[idx], worldPos)) {
            if (idx > *bestHit) *bestHit = idx;
        }
    }

    if (!node->isLeaf) {
        for (int i = 0; i < 4; i++) {
            if (node->children[i] != -1) {
                HitTestElement_Spatial_Rec(tree, node->children[i], elements, layers, layerCount, worldPos, queryBox, bestHit);
            }
        }
    }
}

int HitTestElement_Spatial(const SpatialQuadTree *tree, const GridElement *elements, int count, const Layer *layers, int layerCount, Vector2 worldPos) {
    if (count <= 0 || tree->nodeCount == 0) return -1;
    AABB queryBox = {
        .min = { worldPos.x - 12.0f, worldPos.y - 12.0f },
        .max = { worldPos.x + 12.0f, worldPos.y + 12.0f }
    };
    int bestHit = -1;
    HitTestElement_Spatial_Rec(tree, 0, elements, layers, layerCount, worldPos, queryBox, &bestHit);
    return bestHit;
}

static void GetClosestSnapPoint_Spatial_Rec(const SpatialQuadTree *tree, int nodeIdx, const GridElement *elements, const Layer *layers, Vector2 mousePos, AABB queryBox, float *minSqDist, Vector2 *bestPoint) {
    const QuadTreeNode *node = &tree->nodes[nodeIdx];
    if (!AABBIntersectsAABB(node->bounds, queryBox)) return;

    for (int i = 0; i < node->count; i++) {
        int idx = node->elementIndices[i];
        if (!layers[elements[idx].layerIndex].visible) continue;

        Vector2 pts[MAX_POLYLINE_POINTS];
        int count = 0;
        GetElementSnapPoints(&elements[idx], pts, &count);

        for (int p = 0; p < count; p++) {
            float dx = mousePos.x - pts[p].x;
            float dy = mousePos.y - pts[p].y;
            float sqDist = dx * dx + dy * dy;
            if (sqDist < *minSqDist) {
                *minSqDist = sqDist;
                *bestPoint = pts[p];
                float currentDist = sqrtf(*minSqDist);
                queryBox.min = (Vector2){ mousePos.x - currentDist, mousePos.y - currentDist };
                queryBox.max = (Vector2){ mousePos.x + currentDist, mousePos.y + currentDist };
            }
        }
    }

    if (!node->isLeaf) {
        for (int i = 0; i < 4; i++) {
            if (node->children[i] != -1) {
                GetClosestSnapPoint_Spatial_Rec(tree, node->children[i], elements, layers, mousePos, queryBox, minSqDist, bestPoint);
            }
        }
    }
}

Vector2 GetClosestSnapPoint_Spatial(const SpatialQuadTree *tree, Vector2 mousePos, const GridElement *elements, int count, const Layer *layers, float snapRadius) {
    Vector2 bestPoint = mousePos;
    if (count <= 0 || tree->nodeCount == 0) return bestPoint;

    float minSqDist = snapRadius * snapRadius;
    AABB queryBox = {
        .min = { mousePos.x - snapRadius, mousePos.y - snapRadius },
        .max = { mousePos.x + snapRadius, mousePos.y + snapRadius }
    };

    GetClosestSnapPoint_Spatial_Rec(tree, 0, elements, layers, mousePos, queryBox, &minSqDist, &bestPoint);
    return bestPoint;
}