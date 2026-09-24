#include "flange.h"
#include "cad_math.h"
#include "cJSON.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FlangeDatabase g_FlangeDB = { 0 };

bool Flange_LoadDatabase(const char *jsonPath) {
    if (g_FlangeDB.isLoaded) return true;
    const char *pathsToTry[] = {
        jsonPath,
        "P_ID_res/Flanges_WN.json",
        "../P_ID_res/Flanges_WN.json",
        "P_ID_res/Flange_WN.json",
        "../P_ID_res/Flange_WN.json",
        "Flanges_WN.json"
    };

    FILE *f = NULL;
    for (size_t i = 0; i < sizeof(pathsToTry) / sizeof(pathsToTry[0]); i++) {
        if (pathsToTry[i] != NULL && FileExists(pathsToTry[i])) {
            f = fopen(pathsToTry[i], "rb");
            if (f) break;
        }
    }

    if (!f) {
        TraceLog(LOG_WARNING, "Flange_LoadDatabase: Could not locate Flanges_WN.json in P_ID_res folder");
        return false;
    }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) {
        fclose(f);
        return false;
    }

    char *buf = (char *)malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return false;
    }

    size_t readBytes = fread(buf, 1, (size_t)sz, f);
    buf[readBytes] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root) {
        TraceLog(LOG_ERROR, "Flange_LoadDatabase: cJSON failed to parse Flanges_WN.json");
        return false;
    }

    cJSON *classesObj = cJSON_GetObjectItem(root, "classes");
    if (!classesObj || (classesObj->type & 0xFF) != cJSON_Object) {
        cJSON_Delete(root);
        return false;
    }

    memset(&g_FlangeDB, 0, sizeof(FlangeDatabase));
    cJSON *clsItem = classesObj->child;
    while (clsItem && g_FlangeDB.classCount < MAX_FLANGE_CLASSES) {
        FlangeClassTable *ct = &g_FlangeDB.classes[g_FlangeDB.classCount];
        strncpy(ct->className, clsItem->string ? clsItem->string : "150#", sizeof(ct->className) - 1);
        ct->className[sizeof(ct->className) - 1] = '\0';
        ct->recordCount = 0;

        cJSON *npsItem = clsItem->child;
        while (npsItem && ct->recordCount < MAX_FLANGE_SIZES) {
            FlangeRecord *rec = &ct->records[ct->recordCount];
            strncpy(rec->nps, npsItem->string ? npsItem->string : "1/2\"", sizeof(rec->nps) - 1);
            rec->nps[sizeof(rec->nps) - 1] = '\0';

            cJSON *cPod = cJSON_GetObjectItem(npsItem, "pipe_od_mm");
            cJSON *cFh  = cJSON_GetObjectItem(npsItem, "fh");
            cJSON *cFw  = cJSON_GetObjectItem(npsItem, "fw");
            cJSON *cFt  = cJSON_GetObjectItem(npsItem, "ft");
            cJSON *cBh  = cJSON_GetObjectItem(npsItem, "bolt_holes");
            cJSON *cBc  = cJSON_GetObjectItem(npsItem, "bolt_circle_mm");

            rec->pipe_od_mm     = cPod ? (float)cPod->valuedouble : 21.3f;
            rec->fh             = cFh  ? (float)cFh->valuedouble  : 88.9f;
            rec->fw             = cFw  ? (float)cFw->valuedouble  : 11.2f;
            rec->ft             = cFt  ? (float)cFt->valuedouble  : 47.6f;
            rec->bolt_holes     = cBh  ? cBh->valueint : 4;
            rec->bolt_circle_mm = cBc  ? (float)cBc->valuedouble  : 60.3f;

            ct->recordCount++;
            npsItem = npsItem->next;
        }
        g_FlangeDB.classCount++;
        clsItem = clsItem->next;
    }

    cJSON_Delete(root);
    g_FlangeDB.isLoaded = true;
    return true;
}

const FlangeDatabase *Flange_GetDatabase(void) {
    if (!g_FlangeDB.isLoaded) {
        Flange_LoadDatabase("P_ID_res/Flanges_WN.json");
    }
    return &g_FlangeDB;
}

void Flange_InitDefaultSpec(FlangeSpec *outSpec, FlangeType type) {
    if (!outSpec) return;
    memset(outSpec, 0, sizeof(FlangeSpec));
    outSpec->type = type;
    snprintf(outSpec->standardCode, sizeof(outSpec->standardCode), "ASME B16.5");

    if (!g_FlangeDB.isLoaded) {
        Flange_LoadDatabase("P_ID_res/Flanges_WN.json");
    }

    if (g_FlangeDB.isLoaded && g_FlangeDB.classCount > 0) {
        outSpec->classIndex = 0;
        FlangeClassTable *ct = &g_FlangeDB.classes[0];
        strncpy(outSpec->className, ct->className, sizeof(outSpec->className) - 1);

        int defSizeIdx = 0;
        for (int i = 0; i < ct->recordCount; i++) {
            if (strcmp(ct->records[i].nps, "2\"") == 0) {
                defSizeIdx = i;
                break;
            }
        }
        outSpec->sizeIndex = defSizeIdx;
        FlangeRecord *rec = &ct->records[defSizeIdx];
        strncpy(outSpec->nps, rec->nps, sizeof(outSpec->nps) - 1);
        outSpec->pipeOdMm     = rec->pipe_od_mm;
        outSpec->fh           = rec->fh;
        outSpec->fw           = rec->fw;
        outSpec->ft           = rec->ft;
        outSpec->boltHoles     = rec->bolt_holes;
        outSpec->boltCircleMm = rec->bolt_circle_mm;
    } else {
        outSpec->classIndex = 0;
        outSpec->sizeIndex = 0;
        strncpy(outSpec->className, "150#", sizeof(outSpec->className) - 1);
        strncpy(outSpec->nps, "2\"", sizeof(outSpec->nps) - 1);
        outSpec->pipeOdMm = 60.3f;
        outSpec->fh = 152.4f;
        outSpec->fw = 19.1f;
        outSpec->ft = 63.5f;
        outSpec->boltHoles = 4;
        outSpec->boltCircleMm = 120.7f;
    }
}

bool Flange_SetSpecBySize(FlangeSpec *spec, const char *className, const char *nps) {
    if (!spec || !className || !nps) return false;
    const FlangeDatabase *db = Flange_GetDatabase();
    if (!db->isLoaded) return false;

    for (int c = 0; c < db->classCount; c++) {
        if (strcmp(db->classes[c].className, className) == 0) {
            for (int s = 0; s < db->classes[c].recordCount; s++) {
                if (strcmp(db->classes[c].records[s].nps, nps) == 0) {
                    spec->classIndex = c;
                    spec->sizeIndex = s;
                    strncpy(spec->className, className, sizeof(spec->className) - 1);
                    strncpy(spec->nps, nps, sizeof(spec->nps) - 1);
                    spec->pipeOdMm     = db->classes[c].records[s].pipe_od_mm;
                    spec->fh           = db->classes[c].records[s].fh;
                    spec->fw           = db->classes[c].records[s].fw;
                    spec->ft           = db->classes[c].records[s].ft;
                    spec->boltHoles     = db->classes[c].records[s].bolt_holes;
                    spec->boltCircleMm = db->classes[c].records[s].bolt_circle_mm;
                    return true;
                }
            }
        }
    }
    return false;
}

bool Flange_ValidateConnection(const FlangeSpec *spec, float pipeDiameterMm, int pipeSchedule) {
    if (!spec) return false;
    (void)pipeSchedule;
    float delta = spec->pipeOdMm - pipeDiameterMm;
    if (delta < 0.0f) delta = -delta;
    return (delta <= 5.0f);
}

GridElement Flange_CreateGridElement(Vector2 worldPos, float rotationDeg, int layerIndex, const FlangeSpec *spec) {
    GridElement el;
    memset(&el, 0, sizeof(GridElement));
    el.id = GenerateEntityID();
    el.type = ELEMENT_PID;
    el.pos = worldPos;
    el.scale = (Vector2){ 1.0f, 1.0f };
    el.rotation = rotationDeg;
    el.layerIndex = layerIndex;
    el.useCustomColor = true;
    el.color = (Color){ 41, 128, 185, 255 }; // Distinct engineering steel blue
    el.lineThickness = 3.0f; // Default line thickness

    FlangeSpec s;
    if (spec) {
        s = *spec;
    } else {
        Flange_InitDefaultSpec(&s, FLANGE_WELD_NECK);
    }
    el.width = s.fw;
    el.height = s.fh;
    el.radius = s.ft;
    snprintf(el.text, TEXT_NOTE_LEN, "%s|%s", s.className, s.nps);

    GetElementAABB(&el);
    return el;
}

void Flange_DrawElement(const GridElement *el, Color color, float zoom, bool isSelected) {
    float fw = el->width;
    float fh = el->height;
    float ft = el->radius;
    if (fw <= 0.0f) fw = 19.1f;
    if (fh <= 0.0f) fh = 152.4f;
    if (ft <= 0.0f) ft = 63.5f;

    // Fixed unscaled physical dimensions per ASME standards
    // Local coordinates:
    // Flange rectangle: X: [0 to fw], Y: [-fh/2 to fh/2]
    // Weld neck centerline: from (fw, 0) backwards to (fw - ft, 0)
    // Tail vertical tick: from (fw - ft, -fw/2) to (fw - ft, fw/2)
    Vector2 rectLocalCorners[4] = {
        { 0.0f, -fh * 0.5f },
        { fw,   -fh * 0.5f },
        { fw,    fh * 0.5f },
        { 0.0f,  fh * 0.5f }
    };
    Vector2 wCorners[4];
    for (int i = 0; i < 4; i++) {
        wCorners[i] = LocalToWorldPoint(rectLocalCorners[i], el->pos, el->rotation);
    }

    // Ensure fillColor is completely opaque and uses user-chosen color
    Color fillColor = color;
    if (fillColor.a == 0) fillColor.a = 255;
    Color strokeColor = isSelected ? GOLD : DARKGRAY;
    float userThick = (el->lineThickness > 0.0f) ? el->lineThickness : 3.0f;
    float strokeThick = (isSelected ? (userThick + 1.5f) : userThick) / zoom;

    // Weld Neck Centerline: from (fw, 0) to (fw - ft, 0)
    Vector2 tailStartLocal = { fw, 0.0f };
    Vector2 tailEndLocal   = { fw - ft, 0.0f };
    Vector2 tailStartWorld = LocalToWorldPoint(tailStartLocal, el->pos, el->rotation);
    Vector2 tailEndWorld   = LocalToWorldPoint(tailEndLocal, el->pos, el->rotation);
    DrawLineEx(tailStartWorld, tailEndWorld, strokeThick, strokeColor);

    // Fill flange rectangle with correct triangle orientation
    DrawTriangle(wCorners[0], wCorners[2], wCorners[1], fillColor);
    DrawTriangle(wCorners[0], wCorners[3], wCorners[2], fillColor);

    // Flange rectangle border outline
    for (int i = 0; i < 4; i++) {
        DrawLineEx(wCorners[i], wCorners[(i + 1) % 4], strokeThick, strokeColor);
    }

    // Joint dot at weld prep tip
    DrawCircleV(tailEndWorld, (strokeThick * 0.75f) > 2.0f / zoom ? (strokeThick * 0.75f) : 2.0f / zoom, strokeColor);
}