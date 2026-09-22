#ifndef SPATIAL_TREE_H
#define SPATIAL_TREE_H

#include "cad_types.h"

void SpatialIndex_Build(SpatialQuadTree *tree, const GridElement *elements, const AABB *cachedAABBs, int elementCount);
int HitTestElement_Spatial(const SpatialQuadTree *tree, const GridElement *elements, int count, const Layer *layers, int layerCount, Vector2 worldPos);
Vector2 GetClosestSnapPoint_Spatial(const SpatialQuadTree *tree, Vector2 mousePos, const GridElement *elements, int count, const Layer *layers, float snapRadius);

#endif // SPATIAL_TREE_H