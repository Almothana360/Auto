#include "project_io.h"
#include "cJSON.h"
#include "layer.h"
#include "cad_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *ElementTypeToString(ElementType type) {
    switch (type) {
        case ELEMENT_RECT:      return "ELEMENT_RECT";
        case ELEMENT_CIRCLE:    return "ELEMENT_CIRCLE";
        case ELEMENT_DIMENSION: return "ELEMENT_DIMENSION";
        case ELEMENT_LINE:      return "ELEMENT_LINE";
        case ELEMENT_POLYLINE:  return "ELEMENT_POLYLINE";
        case ELEMENT_FREEHAND:  return "ELEMENT_FREEHAND";
        case ELEMENT_ARC:       return "ELEMENT_ARC";
        case ELEMENT_ELLIPSE:   return "ELEMENT_ELLIPSE";
        case ELEMENT_TEXT_NOTE: return "ELEMENT_TEXT_NOTE";
        case ELEMENT_SYMBOL:    return "ELEMENT_SYMBOL";
        default:                return "ELEMENT_RECT";
    }
}

static ElementType StringToElementType(const char *str) {
    if (!str) return ELEMENT_RECT;
    if (strcmp(str, "ELEMENT_RECT") == 0)      return ELEMENT_RECT;
    if (strcmp(str, "ELEMENT_CIRCLE") == 0)    return ELEMENT_CIRCLE;
    if (strcmp(str, "ELEMENT_DIMENSION") == 0) return ELEMENT_DIMENSION;
    if (strcmp(str, "ELEMENT_LINE") == 0)      return ELEMENT_LINE;
    if (strcmp(str, "ELEMENT_POLYLINE") == 0)  return ELEMENT_POLYLINE;
    if (strcmp(str, "ELEMENT_FREEHAND") == 0)  return ELEMENT_FREEHAND;
    if (strcmp(str, "ELEMENT_ARC") == 0)       return ELEMENT_ARC;
    if (strcmp(str, "ELEMENT_ELLIPSE") == 0)   return ELEMENT_ELLIPSE;
    if (strcmp(str, "ELEMENT_TEXT_NOTE") == 0) return ELEMENT_TEXT_NOTE;
    if (strcmp(str, "ELEMENT_SYMBOL") == 0)    return ELEMENT_SYMBOL;
    return ELEMENT_RECT;
}

static cJSON *CreateVector2JSON(Vector2 v) {
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.x));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(v.y));
    return arr;
}

static Vector2 ParseVector2JSON(const cJSON *arr, Vector2 defaultVal) {
    if (!arr || (arr->type & 0xFF) != cJSON_Array) return defaultVal;
    Vector2 res = defaultVal;
    cJSON *x = cJSON_GetArrayItem(arr, 0);
    cJSON *y = cJSON_GetArrayItem(arr, 1);
    if (x && (x->type & 0xFF) == cJSON_Number) res.x = (float)x->valuedouble;
    if (y && (y->type & 0xFF) == cJSON_Number) res.y = (float)y->valuedouble;
    return res;
}

static cJSON *CreateColorJSON(Color c) {
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(c.r));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(c.g));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(c.b));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(c.a));
    return arr;
}

static Color ParseColorJSON(const cJSON *arr, Color defaultVal) {
    if (!arr || (arr->type & 0xFF) != cJSON_Array) return defaultVal;
    Color res = defaultVal;
    cJSON *r = cJSON_GetArrayItem(arr, 0);
    cJSON *g = cJSON_GetArrayItem(arr, 1);
    cJSON *b = cJSON_GetArrayItem(arr, 2);
    cJSON *a = cJSON_GetArrayItem(arr, 3);
    if (r && (r->type & 0xFF) == cJSON_Number) res.r = (unsigned char)r->valueint;
    if (g && (g->type & 0xFF) == cJSON_Number) res.g = (unsigned char)g->valueint;
    if (b && (b->type & 0xFF) == cJSON_Number) res.b = (unsigned char)b->valueint;
    if (a && (a->type & 0xFF) == cJSON_Number) res.a = (unsigned char)a->valueint;
    return res;
}

static double GetJSONNumber(const cJSON *obj, const char *key, double defaultVal) {
    cJSON *item = cJSON_GetObjectItem(obj, key);
    return (item && (item->type & 0xFF) == cJSON_Number) ? item->valuedouble : defaultVal;
}

static bool GetJSONBool(const cJSON *obj, const char *key, bool defaultVal) {
    cJSON *item = cJSON_GetObjectItem(obj, key);
    if (!item) return defaultVal;
    if ((item->type & 0xFF) == cJSON_True) return true;
    if ((item->type & 0xFF) == cJSON_False) return false;
    return defaultVal;
}

static const char *GetJSONString(const cJSON *obj, const char *key, const char *defaultVal) {
    cJSON *item = cJSON_GetObjectItem(obj, key);
    return (item && (item->type & 0xFF) == cJSON_String && item->valuestring) ? item->valuestring : defaultVal;
}

static bool WriteStringToFile(const char *filename, const char *str) {
    FILE *f = fopen(filename, "wb");
    if (!f) return false;
    size_t len = strlen(str);
    size_t written = fwrite(str, 1, len, f);
    fclose(f);
    return written == len;
}

static char *ReadStringFromFile(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return NULL; }
    char *buf = (char*)malloc(sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t read_bytes = fread(buf, 1, sz, f);
    buf[read_bytes] = '\0';
    fclose(f);
    return buf;
}

void SaveProject(const char *filename, GridElement *elements, int elementCount, Layer *layers, int layerCount) {
    const char *targetFilename = (filename && strstr(filename, ".dat")) ? PROJECT_FILENAME : (filename ? filename : PROJECT_FILENAME);
    SyncLayersWithElements(layers, layerCount, elements, elementCount);
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "schema_version", cJSON_CreateNumber(CURRENT_SCHEMA_VERSION));
    cJSON_AddItemToObject(root, "layer_count", cJSON_CreateNumber(layerCount));
    cJSON *layersArr = cJSON_CreateArray();
    for (int i = 0; i < layerCount; i++) {
        cJSON *layerObj = cJSON_CreateObject();
        cJSON_AddItemToObject(layerObj, "id", cJSON_CreateNumber(layers[i].id));
        cJSON_AddItemToObject(layerObj, "name", cJSON_CreateString(layers[i].name));
        cJSON_AddItemToObject(layerObj, "visible", cJSON_CreateBool(layers[i].visible));
        cJSON_AddItemToObject(layerObj, "locked", cJSON_CreateBool(layers[i].locked));
        cJSON_AddItemToObject(layerObj, "defaultColor", CreateColorJSON(layers[i].defaultColor));
        cJSON_AddItemToObject(layerObj, "renderOrder", cJSON_CreateNumber(layers[i].renderOrder));
        cJSON *entArr = cJSON_CreateArray();
        for (int k = 0; k < layers[i].entityCount; k++) {
            cJSON_AddItemToArray(entArr, cJSON_CreateNumber(layers[i].entityIds[k]));
        }
        cJSON_AddItemToObject(layerObj, "entityIds", entArr);
        cJSON_AddItemToArray(layersArr, layerObj);
    }
    cJSON_AddItemToObject(root, "layers", layersArr);
    cJSON_AddItemToObject(root, "element_count", cJSON_CreateNumber(elementCount));
    cJSON *elemsArr = cJSON_CreateArray();
    for (int i = 0; i < elementCount; i++) {
        GridElement *el = &elements[i];
        cJSON *elObj = cJSON_CreateObject();
        cJSON_AddItemToObject(elObj, "id", cJSON_CreateNumber(el->id));
        cJSON_AddItemToObject(elObj, "type", cJSON_CreateString(ElementTypeToString(el->type)));
        cJSON_AddItemToObject(elObj, "pos", CreateVector2JSON(el->pos));
        cJSON_AddItemToObject(elObj, "rotation", cJSON_CreateNumber(el->rotation));
        cJSON_AddItemToObject(elObj, "scale", CreateVector2JSON(el->scale));
        cJSON *bboxObj = cJSON_CreateObject();
        cJSON_AddItemToObject(bboxObj, "min", CreateVector2JSON(el->bbox.min));
        cJSON_AddItemToObject(bboxObj, "max", CreateVector2JSON(el->bbox.max));
        cJSON_AddItemToObject(elObj, "bbox", bboxObj);
        cJSON_AddItemToObject(elObj, "width", cJSON_CreateNumber(el->width));
        cJSON_AddItemToObject(elObj, "height", cJSON_CreateNumber(el->height));
        cJSON_AddItemToObject(elObj, "radius", cJSON_CreateNumber(el->radius));
        cJSON_AddItemToObject(elObj, "layerIndex", cJSON_CreateNumber(el->layerIndex));
        cJSON_AddItemToObject(elObj, "selected", cJSON_CreateBool(el->selected));
        cJSON_AddItemToObject(elObj, "useCustomColor", cJSON_CreateBool(el->useCustomColor));
        cJSON_AddItemToObject(elObj, "color", CreateColorJSON(el->color));
        switch (el->type) {
            case ELEMENT_LINE:
                cJSON_AddItemToObject(elObj, "p1", CreateVector2JSON(el->p1));
                cJSON_AddItemToObject(elObj, "p2", CreateVector2JSON(el->p2));
                cJSON_AddItemToObject(elObj, "lineThickness", cJSON_CreateNumber(el->lineThickness));
                break;
            case ELEMENT_DIMENSION:
                cJSON_AddItemToObject(elObj, "p1", CreateVector2JSON(el->p1));
                cJSON_AddItemToObject(elObj, "p2", CreateVector2JSON(el->p2));
                cJSON_AddItemToObject(elObj, "dimPos", CreateVector2JSON(el->dimPos));
                cJSON_AddItemToObject(elObj, "lineThickness", cJSON_CreateNumber(el->lineThickness));
                cJSON_AddItemToObject(elObj, "tickThickness", cJSON_CreateNumber(el->tickThickness));
                cJSON_AddItemToObject(elObj, "textSize", cJSON_CreateNumber(el->textSize));
                break;
            case ELEMENT_ARC:
                cJSON_AddItemToObject(elObj, "p1", CreateVector2JSON(el->p1));
                cJSON_AddItemToObject(elObj, "p2", CreateVector2JSON(el->p2));
                cJSON_AddItemToObject(elObj, "p3", CreateVector2JSON(el->p3));
                cJSON_AddItemToObject(elObj, "startAngle", cJSON_CreateNumber(el->startAngle));
                cJSON_AddItemToObject(elObj, "endAngle", cJSON_CreateNumber(el->endAngle));
                cJSON_AddItemToObject(elObj, "lineThickness", cJSON_CreateNumber(el->lineThickness));
                break;
            case ELEMENT_ELLIPSE:
                cJSON_AddItemToObject(elObj, "radiusX", cJSON_CreateNumber(el->radiusX));
                cJSON_AddItemToObject(elObj, "radiusY", cJSON_CreateNumber(el->radiusY));
                break;
            case ELEMENT_POLYLINE:
            case ELEMENT_FREEHAND: {
                cJSON_AddItemToObject(elObj, "pointCount", cJSON_CreateNumber(el->pointCount));
                cJSON_AddItemToObject(elObj, "lineThickness", cJSON_CreateNumber(el->lineThickness));
                cJSON *pts = cJSON_CreateArray();
                int cnt = (el->pointCount <= MAX_POLYLINE_POINTS) ? el->pointCount : MAX_POLYLINE_POINTS;
                for (int p = 0; p < cnt; p++) {
                    cJSON_AddItemToArray(pts, CreateVector2JSON(el->points[p]));
                }
                cJSON_AddItemToObject(elObj, "points", pts);
                break;
            }
            case ELEMENT_TEXT_NOTE:
                cJSON_AddItemToObject(elObj, "text", cJSON_CreateString(el->text));
                cJSON_AddItemToObject(elObj, "arrowTarget", CreateVector2JSON(el->arrowTarget));
                cJSON_AddItemToObject(elObj, "showArrow", cJSON_CreateBool(el->showArrow));
                cJSON_AddItemToObject(elObj, "textSize", cJSON_CreateNumber(el->textSize));
                break;
            default:
                break;
        }
        cJSON_AddItemToArray(elemsArr, elObj);
    }
    cJSON_AddItemToObject(root, "elements", elemsArr);
    char *jsonStr = cJSON_Print(root);
    if (jsonStr) {
        WriteStringToFile(targetFilename, jsonStr);
        free(jsonStr);
    }
    cJSON_Delete(root);
}

int LoadProject(const char *filename, GridElement *elements, int *elementCount, Layer *layers, int *layerCount) {
    const char *targetFilename = filename;
    char *jsonRaw = NULL;
    if (targetFilename) {
        jsonRaw = ReadStringFromFile(targetFilename);
    }
    if (!jsonRaw && targetFilename && strcmp(targetFilename, LEGACY_PROJECT_FILENAME) == 0) {
        jsonRaw = ReadStringFromFile(PROJECT_FILENAME);
    }
    if (!jsonRaw) return 0;
    cJSON *root = cJSON_Parse(jsonRaw);
    free(jsonRaw);
    if (!root) return 0;
    for (int i = 0; i < *layerCount; i++) {
        Layer_Free(&layers[i]);
    }
    cJSON *layersArr = cJSON_GetObjectItem(root, "layers");
    if (layersArr && (layersArr->type & 0xFF) == cJSON_Array) {
        int lSz = cJSON_GetArraySize(layersArr);
        if (lSz > MAX_LAYERS) lSz = MAX_LAYERS;
        *layerCount = lSz;
        for (int i = 0; i < lSz; i++) {
            cJSON *lObj = cJSON_GetArrayItem(layersArr, i);
            if (!lObj) continue;
            unsigned int lid = (unsigned int)GetJSONNumber(lObj, "id", 0);
            if (lid == 0) lid = GenerateLayerID();
            if (lid >= GenerateLayerID()) SetNextLayerID(lid + 1);
            const char *name = GetJSONString(lObj, "name", "Layer");
            Color col = ParseColorJSON(cJSON_GetObjectItem(lObj, "defaultColor"), ParseColorJSON(cJSON_GetObjectItem(lObj, "colorTag"), SKYBLUE));
            int order = (int)GetJSONNumber(lObj, "renderOrder", i);
            Layer_Init(&layers[i], lid, name, col, order);
            layers[i].visible = GetJSONBool(lObj, "visible", true);
            layers[i].locked = GetJSONBool(lObj, "locked", false);
            cJSON *entArr = cJSON_GetObjectItem(lObj, "entityIds");
            if (entArr && (entArr->type & 0xFF) == cJSON_Array) {
                int idCount = cJSON_GetArraySize(entArr);
                for (int e = 0; e < idCount; e++) {
                    cJSON *idItem = cJSON_GetArrayItem(entArr, e);
                    if (idItem && (idItem->type & 0xFF) == cJSON_Number) {
                        Layer_AddEntityId(&layers[i], (unsigned int)idItem->valueint);
                    }
                }
            }
        }
    } else {
        InitDefaultLayers(layers, layerCount);
    }
    cJSON *elemsArr = cJSON_GetObjectItem(root, "elements");
    int count = 0;
    if (elemsArr && (elemsArr->type & 0xFF) == cJSON_Array) {
        int eSz = cJSON_GetArraySize(elemsArr);
        if (eSz > MAX_ELEMENTS) eSz = MAX_ELEMENTS;
        for (int i = 0; i < eSz; i++) {
            cJSON *elObj = cJSON_GetArrayItem(elemsArr, i);
            if (!elObj) continue;
            GridElement *el = &elements[count++];
            memset(el, 0, sizeof(GridElement));
            unsigned int parsedId = (unsigned int)GetJSONNumber(elObj, "id", 0);
            el->id = (parsedId > 0) ? parsedId : GenerateEntityID();
            if (el->id >= GenerateEntityID()) SetNextEntityID(el->id + 1);
            const char *typeStr = GetJSONString(elObj, "type", "ELEMENT_RECT");
            el->type = StringToElementType(typeStr);
            el->pos = ParseVector2JSON(cJSON_GetObjectItem(elObj, "pos"), (Vector2){0, 0});
            el->scale = ParseVector2JSON(cJSON_GetObjectItem(elObj, "scale"), (Vector2){1.0f, 1.0f});
            if (el->scale.x == 0.0f && el->scale.y == 0.0f) el->scale = (Vector2){1.0f, 1.0f};
            el->width = (float)GetJSONNumber(elObj, "width", 80.0);
            el->height = (float)GetJSONNumber(elObj, "height", 60.0);
            el->radius = (float)GetJSONNumber(elObj, "radius", 40.0);
            el->rotation = (float)GetJSONNumber(elObj, "rotation", 0.0);
            el->layerIndex = (int)GetJSONNumber(elObj, "layerIndex", 0);
            el->selected = GetJSONBool(elObj, "selected", false);
            el->useCustomColor = GetJSONBool(elObj, "useCustomColor", false);
            el->color = ParseColorJSON(cJSON_GetObjectItem(elObj, "color"), WHITE);
            el->lineThickness = (float)GetJSONNumber(elObj, "lineThickness", 2.0);
            el->tickThickness = (float)GetJSONNumber(elObj, "tickThickness", 2.0);
            el->textSize = (int)GetJSONNumber(elObj, "textSize", 14);
            el->startAngle = (float)GetJSONNumber(elObj, "startAngle", 0.0);
            el->endAngle = (float)GetJSONNumber(elObj, "endAngle", 360.0);
            el->radiusX = (float)GetJSONNumber(elObj, "radiusX", 60.0);
            el->radiusY = (float)GetJSONNumber(elObj, "radiusY", 40.0);
            el->p1 = ParseVector2JSON(cJSON_GetObjectItem(elObj, "p1"), (Vector2){0, 0});
            el->p2 = ParseVector2JSON(cJSON_GetObjectItem(elObj, "p2"), (Vector2){0, 0});
            el->p3 = ParseVector2JSON(cJSON_GetObjectItem(elObj, "p3"), (Vector2){0, 0});
            el->dimPos = ParseVector2JSON(cJSON_GetObjectItem(elObj, "dimPos"), (Vector2){0, 0});
            el->arrowTarget = ParseVector2JSON(cJSON_GetObjectItem(elObj, "arrowTarget"), (Vector2){0, 0});
            el->showArrow = GetJSONBool(elObj, "showArrow", false);
            const char *txt = GetJSONString(elObj, "text", "");
            strncpy(el->text, txt, TEXT_NOTE_LEN - 1);
            el->text[TEXT_NOTE_LEN - 1] = '\0';
            cJSON *ptsArr = cJSON_GetObjectItem(elObj, "points");
            if (ptsArr && (ptsArr->type & 0xFF) == cJSON_Array) {
                int pSz = cJSON_GetArraySize(ptsArr);
                if (pSz > MAX_POLYLINE_POINTS) pSz = MAX_POLYLINE_POINTS;
                el->pointCount = pSz;
                for (int p = 0; p < pSz; p++) {
                    el->points[p] = ParseVector2JSON(cJSON_GetArrayItem(ptsArr, p), (Vector2){0, 0});
                }
            } else {
                el->pointCount = (int)GetJSONNumber(elObj, "pointCount", 0);
            }
            GetElementAABB(el);
        }
    }
    *elementCount = count;
    SyncLayersWithElements(layers, *layerCount, elements, *elementCount);
    cJSON_Delete(root);
    return 1;
}

void DeselectAllElements(GridElement *elements, int elementCount) {
    for (int i = 0; i < elementCount; i++) elements[i].selected = false;
}

int CountSelectedElements(GridElement *elements, int elementCount) {
    int cnt = 0;
    for (int i = 0; i < elementCount; i++) if (elements[i].selected) cnt++;
    return cnt;
}

int GetFirstSelectedIndex(GridElement *elements, int elementCount) {
    for (int i = 0; i < elementCount; i++) if (elements[i].selected) return i;
    return -1;
}

UiConfig LoadUiConfig(const char *filename) {
    UiConfig config = {
        .uiScale = 1.0f,
        .isFullscreen = false,
        .showHudPanel = true,
        .showInspector = true,
        .showLayersPanel = true,
        .showElementsPanel = true,
        .snapToGrid = false,
        .snapEnabled = true,
        .currentUnit = UNIT_MM,
        .uiBackend = UI_BACKEND_MICROUI,
        .uiTheme = UI_THEME_DARK
    };
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        char line[128];
        while (fgets(line, sizeof(line), file)) {
            float fval; int ival;
            if (sscanf(line, "uiScale=%f", &fval) == 1) { if (fval >= 0.5f && fval <= 4.0f) config.uiScale = fval; }
            else if (sscanf(line, "isFullscreen=%d", &ival) == 1) config.isFullscreen = (ival != 0);
            else if (sscanf(line, "showHudPanel=%d", &ival) == 1) config.showHudPanel = (ival != 0);
            else if (sscanf(line, "showInspector=%d", &ival) == 1) config.showInspector = (ival != 0);
            else if (sscanf(line, "showLayersPanel=%d", &ival) == 1) config.showLayersPanel = (ival != 0);
            else if (sscanf(line, "showElementsPanel=%d", &ival) == 1) config.showElementsPanel = (ival != 0);
            else if (sscanf(line, "snapToGrid=%d", &ival) == 1) config.snapToGrid = (ival != 0);
            else if (sscanf(line, "snapEnabled=%d", &ival) == 1) config.snapEnabled = (ival != 0);
            else if (sscanf(line, "currentUnit=%d", &ival) == 1) config.currentUnit = ival;
            else if (sscanf(line, "uiBackend=%d", &ival) == 1) config.uiBackend = ival;
            else if (sscanf(line, "uiTheme=%d", &ival) == 1) config.uiTheme = ival;
        }
        fclose(file);
    }
    return config;
}

void SaveUiConfig(const char *filename, const UiConfig *config) {
    if (!config) return;
    FILE *file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "uiScale=%.2f\n", config->uiScale);
        fprintf(file, "isFullscreen=%d\n", config->isFullscreen ? 1 : 0);
        fprintf(file, "showHudPanel=%d\n", config->showHudPanel ? 1 : 0);
        fprintf(file, "showInspector=%d\n", config->showInspector ? 1 : 0);
        fprintf(file, "showLayersPanel=%d\n", config->showLayersPanel ? 1 : 0);
        fprintf(file, "showElementsPanel=%d\n", config->showElementsPanel ? 1 : 0);
        fprintf(file, "snapToGrid=%d\n", config->snapToGrid ? 1 : 0);
        fprintf(file, "snapEnabled=%d\n", config->snapEnabled ? 1 : 0);
        fprintf(file, "currentUnit=%d\n", config->currentUnit);
        fprintf(file, "uiBackend=%d\n", config->uiBackend);
        fprintf(file, "uiTheme=%d\n", config->uiTheme);
        fclose(file);
    }
}