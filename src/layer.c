#include "layer.h"
#include <stdlib.h>
#include <string.h>

void Layer_Init(Layer *layer, unsigned int id, const char *name, Color defaultColor, int renderOrder) {
    if (!layer) return;
    layer->id = id;
    strncpy(layer->name, name ? name : "Layer", LAYER_NAME_LEN - 1);
    layer->name[LAYER_NAME_LEN - 1] = '\0';
    layer->visible = true;
    layer->locked = false;
    layer->defaultColor = defaultColor;
    layer->renderOrder = renderOrder;
    layer->entityCount = 0;
    layer->entityCapacity = 16;
    layer->entityIds = (unsigned int*)malloc(sizeof(unsigned int) * layer->entityCapacity);
}

void Layer_Free(Layer *layer) {
    if (!layer) return;
    if (layer->entityIds) {
        free(layer->entityIds);
        layer->entityIds = NULL;
    }
    layer->entityCount = 0;
    layer->entityCapacity = 0;
}

void Layer_Copy(Layer *dest, const Layer *src) {
    if (!dest || !src) return;
    dest->id = src->id;
    strncpy(dest->name, src->name, LAYER_NAME_LEN - 1);
    dest->name[LAYER_NAME_LEN - 1] = '\0';
    dest->visible = src->visible;
    dest->locked = src->locked;
    dest->defaultColor = src->defaultColor;
    dest->renderOrder = src->renderOrder;
    dest->entityCount = src->entityCount;
    dest->entityCapacity = (src->entityCapacity > 0) ? src->entityCapacity : 16;
    dest->entityIds = (unsigned int*)malloc(sizeof(unsigned int) * dest->entityCapacity);
    if (src->entityIds && src->entityCount > 0) {
        memcpy(dest->entityIds, src->entityIds, sizeof(unsigned int) * src->entityCount);
    }
}

void Layer_AddEntityId(Layer *layer, unsigned int entityId) {
    if (!layer || entityId == 0) return;
    for (int i = 0; i < layer->entityCount; i++) {
        if (layer->entityIds[i] == entityId) return;
    }
    if (layer->entityCount >= layer->entityCapacity) {
        layer->entityCapacity = (layer->entityCapacity == 0) ? 16 : layer->entityCapacity * 2;
        layer->entityIds = (unsigned int*)realloc(layer->entityIds, sizeof(unsigned int) * layer->entityCapacity);
    }
    layer->entityIds[layer->entityCount++] = entityId;
}

void Layer_RemoveEntityId(Layer *layer, unsigned int entityId) {
    if (!layer || entityId == 0) return;
    for (int i = 0; i < layer->entityCount; i++) {
        if (layer->entityIds[i] == entityId) {
            for (int j = i; j < layer->entityCount - 1; j++) {
                layer->entityIds[j] = layer->entityIds[j + 1];
            }
            layer->entityCount--;
            break;
        }
    }
}

void SyncLayersWithElements(Layer *layers, int layerCount, const GridElement *elements, int elementCount) {
    for (int l = 0; l < layerCount; l++) {
        layers[l].entityCount = 0;
    }
    for (int i = 0; i < elementCount; i++) {
        int lIdx = elements[i].layerIndex;
        if (lIdx >= 0 && lIdx < layerCount && elements[i].id != 0) {
            Layer_AddEntityId(&layers[lIdx], elements[i].id);
        }
    }
}

void InitDefaultLayers(Layer *layers, int *layerCount) {
    for (int i = 0; i < *layerCount; i++) {
        Layer_Free(&layers[i]);
    }
    *layerCount = 1;
    Layer_Init(&layers[0], GenerateLayerID(), "0", SKYBLUE, 0);
    g_CADState.activeLayerIndex = 0;
    g_CADState.activeLayerId = layers[0].id;
}

void DeleteLayer(GridElement *elements, int *elementCount, Layer *layers, int *layerCount, int layerIndexToDelete, int *activeLayerIndex) {
    if (*layerCount <= 1 || layerIndexToDelete < 0 || layerIndexToDelete >= *layerCount) return;

    Layer_Free(&layers[layerIndexToDelete]);

    int i = 0;
    while (i < *elementCount) {
        if (elements[i].layerIndex == layerIndexToDelete) {
            for (int j = i; j < *elementCount - 1; j++) elements[j] = elements[j + 1];
            (*elementCount)--;
        } else {
            if (elements[i].layerIndex > layerIndexToDelete) elements[i].layerIndex--;
            i++;
        }
    }
    for (int l = layerIndexToDelete; l < *layerCount - 1; l++) layers[l] = layers[l + 1];
    (*layerCount)--;
    if (*activeLayerIndex >= *layerCount) *activeLayerIndex = *layerCount - 1;

    g_CADState.activeLayerIndex = *activeLayerIndex;
    g_CADState.activeLayerId = layers[*activeLayerIndex].id;

    SyncLayersWithElements(layers, *layerCount, elements, *elementCount);
}