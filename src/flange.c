#include "flange.h"
#include "cad_math.h"
#include <string.h>
#include <stdio.h>

void Flange_InitDefaultSpec(FlangeSpec *outSpec, FlangeType type) {
    if (!outSpec) return;
    outSpec->type = type;
    outSpec->ratingClass = FLANGE_CLASS_150;
    outSpec->nominalSizeMm = 50.0f;     // Default 2 inch
    outSpec->outerDiameterMm = 150.0f;
    outSpec->thicknessMm = 19.0f;
    outSpec->boltHoleCount = 4;
    snprintf(outSpec->standardCode, sizeof(outSpec->standardCode), "ASME B16.5");
}

bool Flange_ValidateConnection(const FlangeSpec *spec, float pipeDiameterMm, int pipeSchedule) {
    if (!spec) return false;
    (void)pipeSchedule;
    // Rule checking: pipe diameter compatibility within tolerance
    float delta = spec->nominalSizeMm - pipeDiameterMm;
    if (delta < 0.0f) delta = -delta;
    return (delta <= 5.0f);
}

GridElement Flange_CreateGridElement(Vector2 worldPos, float rotationDeg, int layerIndex, const FlangeSpec *spec) {
    GridElement el;
    memset(&el, 0, sizeof(GridElement));
    el.id = GenerateEntityID();
    el.type = ELEMENT_SYMBOL;
    el.pos = worldPos;
    el.scale = (Vector2){ 1.0f, 1.0f };
    el.rotation = rotationDeg;
    el.layerIndex = layerIndex;
    el.useCustomColor = true;
    el.color = (spec && spec->type == FLANGE_BLIND) ? MAROON : DARKBLUE;
    GetElementAABB(&el);
    return el;
}