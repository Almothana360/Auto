#include "cad_context.h"
#include <math.h>
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

static void InitThemeColors(AppContext *ctx) {
    if (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) {
        ctx->uiAnim.panelBgR = 242.0f; ctx->uiAnim.panelBgG = 243.0f; ctx->uiAnim.panelBgB = 245.0f; ctx->uiAnim.panelBgA = 255.0f;
        ctx->uiAnim.subpanelBgR = 230.0f; ctx->uiAnim.subpanelBgG = 233.0f; ctx->uiAnim.subpanelBgB = 238.0f; ctx->uiAnim.subpanelBgA = 255.0f;
        ctx->uiAnim.borderR = 195.0f; ctx->uiAnim.borderG = 198.0f; ctx->uiAnim.borderB = 204.0f; ctx->uiAnim.borderA = 255.0f;
        ctx->uiAnim.textR = 30.0f; ctx->uiAnim.textG = 30.0f; ctx->uiAnim.textB = 30.0f; ctx->uiAnim.textA = 255.0f;
    } else {
        ctx->uiAnim.panelBgR = 38.0f; ctx->uiAnim.panelBgG = 38.0f; ctx->uiAnim.panelBgB = 38.0f; ctx->uiAnim.panelBgA = 255.0f;
        ctx->uiAnim.subpanelBgR = 30.0f; ctx->uiAnim.subpanelBgG = 30.0f; ctx->uiAnim.subpanelBgB = 30.0f; ctx->uiAnim.subpanelBgA = 255.0f;
        ctx->uiAnim.borderR = 24.0f; ctx->uiAnim.borderG = 24.0f; ctx->uiAnim.borderB = 24.0f; ctx->uiAnim.borderA = 255.0f;
        ctx->uiAnim.textR = 230.0f; ctx->uiAnim.textG = 230.0f; ctx->uiAnim.textB = 230.0f; ctx->uiAnim.textA = 255.0f;
    }
    ctx->uiAnim.currentThemeTarget = ctx->uiConfig.uiTheme;
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

    CAD_PID_Init(&ctx->cadPid);

    ctx->tweenCtx = TweenContext_Create(128);
    ctx->uiAnim.leftDockProgress = ctx->showLeftDock ? 1.0f : 0.0f;
    ctx->uiAnim.rightDockProgress = ctx->showRightDock ? 1.0f : 0.0f;
    ctx->uiAnim.hudProgress = ctx->showHudPanel ? 1.0f : 0.0f;
    ctx->uiAnim.contextMenuProgress = 0.0f;

    InitThemeColors(ctx);
}

void AppContext_Cleanup(AppContext *ctx) {
    if (ctx->tweenCtx) {
        TweenContext_Destroy(ctx->tweenCtx);
        ctx->tweenCtx = NULL;
    }
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
    float dt = GetFrameTime();
    if (ctx->statusMessageTimer > 0.0f) {
        ctx->statusMessageTimer -= dt;
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

        // Dynamically update the RayGUI icon scale proportionally with uiScale
        int iconScale = (int)roundf(ctx->uiScale);
        if (iconScale < 1) iconScale = 1;
        GuiSetIconScale(iconScale);

        ctx->prevUiScale = ctx->uiScale;
    }

    float targetLeft = ctx->showLeftDock ? 1.0f : 0.0f;
    if (fabsf(ctx->uiAnim.leftDockProgress - targetLeft) > 0.001f) {
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.leftDockProgress, targetLeft, 0.20f, TweenEase_CubicOut);
    }
    float targetRight = ctx->showRightDock ? 1.0f : 0.0f;
    if (fabsf(ctx->uiAnim.rightDockProgress - targetRight) > 0.001f) {
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.rightDockProgress, targetRight, 0.20f, TweenEase_CubicOut);
    }
    float targetHud = ctx->showHudPanel ? 1.0f : 0.0f;
    if (fabsf(ctx->uiAnim.hudProgress - targetHud) > 0.001f) {
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.hudProgress, targetHud, 0.20f, TweenEase_CubicOut);
    }
    float targetCtx = ctx->showContextMenu ? 1.0f : 0.0f;
    if (fabsf(ctx->uiAnim.contextMenuProgress - targetCtx) > 0.001f) {
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.contextMenuProgress, targetCtx, 0.15f, TweenEase_QuadOut);
    }

    if (ctx->uiAnim.currentThemeTarget != ctx->uiConfig.uiTheme) {
        ctx->uiAnim.currentThemeTarget = ctx->uiConfig.uiTheme;
        float targetBgR = (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) ? 242.0f : 38.0f;
        float targetBgG = (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) ? 243.0f : 38.0f;
        float targetBgB = (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) ? 245.0f : 38.0f;

        float targetSubR = (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) ? 230.0f : 30.0f;
        float targetSubG = (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) ? 233.0f : 30.0f;
        float targetSubB = (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) ? 238.0f : 30.0f;

        float targetBrdR = (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) ? 195.0f : 24.0f;
        float targetBrdG = (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) ? 198.0f : 24.0f;
        float targetBrdB = (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) ? 204.0f : 24.0f;

        float targetTxtR = (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) ? 30.0f : 230.0f;
        float targetTxtG = (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) ? 30.0f : 230.0f;
        float targetTxtB = (ctx->uiConfig.uiTheme == UI_THEME_LIGHT) ? 30.0f : 230.0f;

        const float themeDuration = 0.25f;
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.panelBgR, targetBgR, themeDuration, TweenEase_QuadInOut);
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.panelBgG, targetBgG, themeDuration, TweenEase_QuadInOut);
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.panelBgB, targetBgB, themeDuration, TweenEase_QuadInOut);

        Tween_To(ctx->tweenCtx, &ctx->uiAnim.subpanelBgR, targetSubR, themeDuration, TweenEase_QuadInOut);
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.subpanelBgG, targetSubG, themeDuration, TweenEase_QuadInOut);
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.subpanelBgB, targetSubB, themeDuration, TweenEase_QuadInOut);

        Tween_To(ctx->tweenCtx, &ctx->uiAnim.borderR, targetBrdR, themeDuration, TweenEase_QuadInOut);
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.borderG, targetBrdG, themeDuration, TweenEase_QuadInOut);
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.borderB, targetBrdB, themeDuration, TweenEase_QuadInOut);

        Tween_To(ctx->tweenCtx, &ctx->uiAnim.textR, targetTxtR, themeDuration, TweenEase_QuadInOut);
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.textG, targetTxtG, themeDuration, TweenEase_QuadInOut);
        Tween_To(ctx->tweenCtx, &ctx->uiAnim.textB, targetTxtB, themeDuration, TweenEase_QuadInOut);
    }

    Tween_Update(ctx->tweenCtx, dt);
}