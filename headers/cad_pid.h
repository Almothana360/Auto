#ifndef CAD_PID_H
#define CAD_PID_H

#include "cad_types.h"
#include "flange.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CAD_PID_MAX_RADIAL_ITEMS 6

typedef enum {
    PID_ITEM_VALVE = 0,
    PID_ITEM_FLANGE,
    PID_ITEM_TEE,
    PID_ITEM_REDUCER,
    PID_ITEM_ELBOW,
    PID_ITEM_PIPE
} PIDInstrumentType;

typedef struct PIDRadialItem {
    PIDInstrumentType type;
    const char *label;
    Rectangle hitBox;
    Vector2 centerPos;
} PIDRadialItem;

typedef struct PIDSystemState {
    bool isPaletteOpen;
    Vector2 paletteCenterScreen;
    float paletteRadius;
    float animProgress;
    int hoveredItemIndex;
    int selectedItemIndex;
    PIDInstrumentType activeToolInstrument;
    bool isPlacingInstrument;
    int placementStep;
    Vector2 placementP1;
    FlangeSpec currentFlangeSpec;
} PIDSystemState;

struct AppContext;

void CAD_PID_Init(PIDSystemState *pid);
void CAD_PID_OpenPalette(PIDSystemState *pid, Vector2 screenPos);
void CAD_PID_ClosePalette(PIDSystemState *pid);
bool CAD_PID_Update(PIDSystemState *pid, struct AppContext *app, bool overUI);
void CAD_PID_RenderPalette(const PIDSystemState *pid, float uiScale, Font font);
void CAD_PID_RenderToolPreview(const PIDSystemState *pid, struct AppContext *app, Vector2 activeToolPoint);
bool CAD_PID_ValidatePlacement(PIDInstrumentType type, Vector2 worldPos, const struct AppContext *app);
void CAD_PID_PlaceInstrument(PIDInstrumentType type, Vector2 worldPos, struct AppContext *app);

#ifdef __cplusplus
}
#endif

#endif // CAD_PID_H