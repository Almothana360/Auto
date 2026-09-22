#include "cad_context.h"
#include "project_io.h"
#include "layer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cad_math.h"
#include "raygui.h"
#include "spatial_tree.h"
#include "render_utils.h"

static const char* ResolvePath(const char *path) {
    if (FileExists(path)) return path;
    // Fallback: check without leading "../" if launched directly from parent dir
    if (strncmp(path, "../", 3) == 0 && FileExists(path + 3)) {
        return path + 3;
    }
    return NULL;
}

void AppContext_InitFonts(AppContext *ctx) {
    if (!ctx) return;
    int menuSize  = (int)(15 * ctx->uiScale);
    int titleSize = (int)(16 * ctx->uiScale);
    int bodySize  = (int)(14 * ctx->uiScale);
    int noteSize  = (int)(24 * ctx->uiScale);
    if (menuSize < 12) menuSize = 12;
    if (titleSize < 12) titleSize = 12;
    if (bodySize < 12) bodySize = 12;

    const char *menuPath  = ResolvePath(FONT_PATH_MENU);
    const char *titlePath = ResolvePath(FONT_PATH_TITLE);
    const char *bodyPath  = ResolvePath(FONT_PATH_BODY);
    const char *notePath  = ResolvePath(FONT_PATH_NOTE);

    if (!menuPath)  TraceLog(LOG_WARNING, "Font not found: %s", FONT_PATH_MENU);
    if (!titlePath) TraceLog(LOG_WARNING, "Font not found: %s", FONT_PATH_TITLE);
    if (!bodyPath)  TraceLog(LOG_WARNING, "Font not found: %s", FONT_PATH_BODY);
    if (!notePath)  TraceLog(LOG_WARNING, "Font not found: %s", FONT_PATH_NOTE);

    ResourceManager_LoadFont(&ctx->resManager, FONT_SLOT_MENU,  menuPath, menuSize);
    ResourceManager_LoadFont(&ctx->resManager, FONT_SLOT_TITLE, titlePath, titleSize);
    ResourceManager_LoadFont(&ctx->resManager, FONT_SLOT_BODY,  bodyPath, bodySize);
    ResourceManager_LoadFont(&ctx->resManager, FONT_SLOT_NOTE,  notePath, noteSize);

    Font bodyFont = ResourceManager_GetFont(&ctx->resManager, FONT_SLOT_BODY);
    SetActiveUIFont(bodyFont);
}

void AppContext_Init(AppContext *ctx) {
    memset(ctx, 0, sizeof(AppContext));
    ResourceManager_Init(&ctx->resManager);
    ctx->uiConfig = LoadUiConfig(CONFIG_FILENAME);
    ctx->camera.target = (Vector2){ 0.0f, 0.0f };
    ctx->camera.offset = (Vector2){ (float)SCREEN_WIDTH / 2.0f, (float)SCREEN_HEIGHT / 2.0f };
    ctx->camera.zoom = 1.0f;
    ctx->gridSpacing = 50.0f;
    ctx->uiScale = ctx->uiConfig.uiScale;
    ctx->prevUiScale = ctx->uiConfig.uiScale;
    ctx->tempUiScale = ctx->uiConfig.uiScale * 100.0f;
    ctx->showHudPanel = true;
    ctx->showLeftDock = true;
    ctx->showRightDock = true;
    ctx->showInspector = true;
    ctx->currentUnit = (MeasureUnit)ctx->uiConfig.currentUnit;

    InitDefaultLayers(ctx->layers, &ctx->layerCount);
    ctx->activeLayerIndex = 0;
    ctx->layerRenameEditMode = false;
    ctx->noteTextEditMode = false;

    ctx->elements = (GridElement*)calloc(MAX_ELEMENTS, sizeof(GridElement));
    ctx->elementStartStates = (GridElement*)calloc(MAX_ELEMENTS, sizeof(GridElement));
    ctx->cachedAABBs = (AABB*)calloc(MAX_ELEMENTS, sizeof(AABB));
    ctx->spatialTree = (SpatialQuadTree*)calloc(1, sizeof(SpatialQuadTree));
    ctx->spatialIndexDirty = true;
    ctx->activeHandle = HANDLE_NONE;
    ctx->activeHandleElementIdx = -1;
    ctx->cmdHistory = (CommandHistory*)calloc(1, sizeof(CommandHistory));

    ctx->snapToGrid = ctx->uiConfig.snapToGrid;
    ctx->snapEnabled = ctx->uiConfig.snapEnabled;
    ctx->snapThreshold = 14.0f;
}

void AppContext_Cleanup(AppContext *ctx) {
    SaveUiConfig(CONFIG_FILENAME, &ctx->uiConfig);
    ClearCommandHistory(ctx->cmdHistory);
    free(ctx->cmdHistory);
    free(ctx->elements);
    free(ctx->elementStartStates);
    free(ctx->cachedAABBs);
    free(ctx->spatialTree);
    for (int l = 0; l < ctx->layerCount; l++) {
        Layer_Free(&ctx->layers[l]);
    }
    ResourceManager_UnloadAll(&ctx->resManager);
}

void AppContext_Update(AppContext *ctx) {
    if (ctx->statusMessageTimer > 0.0f) {
        ctx->statusMessageTimer -= GetFrameTime();
    }

    ctx->camera.offset = (Vector2){ (float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f };
    Vector2 mousePos = GetMousePosition();
    g_CADState.mouseScreen = mousePos;
    g_CADState.mouseWorld = GetScreenToWorld2D(mousePos, ctx->camera);

    if (ctx->activeLayerIndex >= 0 && ctx->activeLayerIndex < ctx->layerCount) {
        g_CADState.activeLayerIndex = ctx->activeLayerIndex;
        g_CADState.activeLayerId = ctx->layers[ctx->activeLayerIndex].id;
    }

    if (ctx->spatialIndexDirty) {
        for (int i = 0; i < ctx->elementCount; i++) {
            ctx->cachedAABBs[i] = GetElementAABB(&ctx->elements[i]);
        }
        SpatialIndex_Build(ctx->spatialTree, ctx->elements, ctx->cachedAABBs, ctx->elementCount);
        ctx->spatialIndexDirty = false;
    }

    if (ctx->uiScale != ctx->prevUiScale) {
        AppContext_InitFonts(ctx);
        Font bodyFont = ResourceManager_GetFont(&ctx->resManager, FONT_SLOT_BODY);
        GuiSetFont(bodyFont);
        GuiSetStyle(DEFAULT, TEXT_SIZE, (int)(11 * ctx->uiScale));
        ctx->prevUiScale = ctx->uiScale;
    }
}