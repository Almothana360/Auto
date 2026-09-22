#ifndef FLANGE_H
#define FLANGE_H

#include "cad_types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_FLANGE_CLASSES 8
#define MAX_FLANGE_SIZES   32

typedef enum {
    FLANGE_WELD_NECK = 0,
    FLANGE_SLIP_ON,
    FLANGE_BLIND,
    FLANGE_THREADED,
    FLANGE_SOCKET_WELD,
    FLANGE_LAP_JOINT
} FlangeType;

typedef struct FlangeRecord {
    char nps[16];
    float pipe_od_mm;
    float fh;              // Flange Height (Outer Diameter)
    float fw;              // Flange Width (Thickness)
    float ft;              // Flange Tail (Length Through Hub)
    int bolt_holes;
    float bolt_circle_mm;
} FlangeRecord;

typedef struct FlangeClassTable {
    char className[16];    // e.g. "150#", "300#"
    FlangeRecord records[MAX_FLANGE_SIZES];
    int recordCount;
} FlangeClassTable;

typedef struct FlangeDatabase {
    FlangeClassTable classes[MAX_FLANGE_CLASSES];
    int classCount;
    bool isLoaded;
} FlangeDatabase;

typedef struct FlangeSpec {
    FlangeType type;
    int classIndex;
    int sizeIndex;
    char className[16];
    char nps[16];
    float pipeOdMm;
    float fh;
    float fw;
    float ft;
    int boltHoles;
    float boltCircleMm;
    char standardCode[32]; // "ASME B16.5"
} FlangeSpec;

typedef struct FlangeComponent {
    unsigned int id;
    FlangeSpec spec;
    Vector2 position;
    float rotationDeg;
    bool isConnected;
    unsigned int connectedPipeId;
} FlangeComponent;

extern FlangeDatabase g_FlangeDB;

/* Lifecycle and Database Functions */
bool Flange_LoadDatabase(const char *jsonPath);
const FlangeDatabase *Flange_GetDatabase(void);
void Flange_InitDefaultSpec(FlangeSpec *outSpec, FlangeType type);
bool Flange_SetSpecBySize(FlangeSpec *spec, const char *className, const char *nps);
bool Flange_ValidateConnection(const FlangeSpec *spec, float pipeDiameterMm, int pipeSchedule);

/* Drawing & GridElement Conversion */
GridElement Flange_CreateGridElement(Vector2 worldPos, float rotationDeg, int layerIndex, const FlangeSpec *spec);
void Flange_DrawElement(const GridElement *el, Color color, float zoom, bool isSelected);

#ifdef __cplusplus
}
#endif

#endif // FLANGE_H