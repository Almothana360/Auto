#ifndef CAD_CONTEXT_H
#define CAD_CONTEXT_H

#include "cad_types.h"
#include "commands.h"
#include "resource_loader.h"
#include "microui.h"
#include "tween.h"
#include "cad_pid.h"
#include "cad_connection.h"

typedef struct UiAnimState {
    float leftDockProgress;
    float rightDockProgress;
    float hudProgress;
    float contextMenuProgress;
    float panelBgR, panelBgG, panelBgB, panelBgA;
    float subpanelBgR, subpanelBgG, subpanelBgB, subpanelBgA;
    float borderR, borderG, borderB, borderA;
    float textR, textG, textB, textA;
    int currentThemeTarget;
} UiAnimState;

typedef struct AppContext {
    ResourceManager resManager;
    UiConfig uiConfig;
    Camera2D camera;
    float gridSpacing;
    float uiScale;
    float prevUiScale;
    float tempUiScale;
    bool showHudPanel;
    bool showInspector;
    bool showLeftDock;
    bool showRightDock;
    bool showUnitWindow;
    bool showScaleWindow;
    MeasureUnit currentUnit;
    char statusMessage[64];
    float statusMessageTimer;
    Layer layers[MAX_LAYERS];
    int layerCount;
    int activeLayerIndex;
    char layerNameEditBuf[LAYER_NAME_LEN];
    bool layerRenameEditMode;
    bool noteTextEditMode;
    GridElement *elements;
    GridElement *elementStartStates;
    AABB *cachedAABBs;
    SpatialQuadTree *spatialTree;
    int elementCount;
    bool spatialIndexDirty;
    Vector2 dimP1, dimP2, dimP3;
    int dimStep;
    GridElement tempPolyline;
    bool isDraggingElement;
    Vector2 dragStartWorldPos;
    HandleType activeHandle;
    int activeHandleElementIdx;
    GridElement initialHandleElementState;
    bool isBoxSelecting;
    Vector2 boxStartWorldPos;
    Vector2 boxCurrentWorldPos;
    CommandHistory *cmdHistory;
    bool snapToGrid;
    bool snapEnabled;
    float snapThreshold;
    bool hasSnapX, hasSnapY;
    float snapXVal, snapYVal;
    bool showContextMenu;
    Vector2 contextMenuPos;
    bool contextOnElement;
    int contextElementIndex;
    char commandText[CMD_BUFFER_SIZE];
    bool commandEditMode;
    bool openFileMenu;
    bool openEditMenu;
    bool openWindowMenu;
    bool openElementMenu;
    bool openFunctionsMenu;
    PIDSystemState cadPid;
    ConnectionSystemState connState;
    TweenContext *tweenCtx;
    UiAnimState uiAnim;
} AppContext;

void AppContext_Init(AppContext *ctx);
void AppContext_Cleanup(AppContext *ctx);
void AppContext_InitFonts(AppContext *ctx);
void AppContext_Update(AppContext *ctx);

#endif // CAD_CONTEXT_H