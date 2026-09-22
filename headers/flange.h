#ifndef FLANGE_H
#define FLANGE_H

#include "cad_types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FLANGE_WELD_NECK = 0,
    FLANGE_SLIP_ON,
    FLANGE_BLIND,
    FLANGE_THREADED,
    FLANGE_SOCKET_WELD,
    FLANGE_LAP_JOINT
} FlangeType;

typedef enum {
    FLANGE_CLASS_150 = 150,
    FLANGE_CLASS_300 = 300,
    FLANGE_CLASS_600 = 600,
    FLANGE_CLASS_900 = 900
} FlangeRatingClass;

typedef struct FlangeSpec {
    FlangeType type;
    FlangeRatingClass ratingClass;
    float nominalSizeMm;
    float outerDiameterMm;
    float thicknessMm;
    int boltHoleCount;
    char standardCode[32]; // e.g. "ASME B16.5"
} FlangeSpec;

typedef struct FlangeComponent {
    unsigned int id;
    FlangeSpec spec;
    Vector2 position;
    float rotationDeg;
    bool isConnected;
    unsigned int connectedPipeId;
} FlangeComponent;

/* Placeholder API for Flange Lifecycle and Rule Evaluation */
void Flange_InitDefaultSpec(FlangeSpec *outSpec, FlangeType type);
bool Flange_ValidateConnection(const FlangeSpec *spec, float pipeDiameterMm, int pipeSchedule);
GridElement Flange_CreateGridElement(Vector2 worldPos, float rotationDeg, int layerIndex, const FlangeSpec *spec);

#ifdef __cplusplus
}
#endif

#endif // FLANGE_H