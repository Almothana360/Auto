#ifndef LAYER_H
#define LAYER_H

#include "cad_types.h"

void Layer_Init(Layer *layer, unsigned int id, const char *name, Color defaultColor, int renderOrder);
void Layer_Free(Layer *layer);
void Layer_Copy(Layer *dest, const Layer *src);
void Layer_AddEntityId(Layer *layer, unsigned int entityId);
void Layer_RemoveEntityId(Layer *layer, unsigned int entityId);
void SyncLayersWithElements(Layer *layers, int layerCount, const GridElement *elements, int elementCount);
void InitDefaultLayers(Layer *layers, int *layerCount);
void DeleteLayer(GridElement *elements, int *elementCount, Layer *layers, int *layerCount, int layerIndexToDelete, int *activeLayerIndex);

#endif // LAYER_H