#include "cad_pid.h"
#include "cad_context.h"
#include "cad_math.h"
#include "commands.h"
#include "project_io.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static const char *kPidLabels[CAD_PID_MAX_RADIAL_ITEMS] = {
    "Valve",
    "Flange",
    "Tee",
    "Reducer",
    "Elbow",
    "Pipe"
};

void CAD_PID_Init(PIDSystemState *pid) {
    if (!pid) return;
    memset(pid, 0, sizeof(PIDSystemState));
    pid->paletteRadius = 90.0f;
    pid->hoveredItemIndex = -1;
    pid->selectedItemIndex = -1;
    Flange_LoadDatabase("P_ID_res/Flanges_WN.json");
    Flange_InitDefaultSpec(&pid->currentFlangeSpec, FLANGE_WELD_NECK);
}

void CAD_PID_OpenPalette(PIDSystemState *pid, Vector2 screenPos) {
    if (!pid) return;
    pid->isPaletteOpen = true;
    pid->paletteCenterScreen = screenPos;
    pid->animProgress = 0.0f;
    pid->hoveredItemIndex = -1;
}

void CAD_PID_ClosePalette(PIDSystemState *pid) {
    if (!pid) return;
    pid->isPaletteOpen = false;
    pid->hoveredItemIndex = -1;
}

static void CalculateItemRadialPosition(Vector2 center, float radius, int index, int total, Vector2 *outPos) {
    float angle = ((float)index / (float)total) * 2.0f * PI - (PI / 2.0f);
    outPos->x = center.x + cosf(angle) * radius;
    outPos->y = center.y + sinf(angle) * radius;
}

bool CAD_PID_Update(PIDSystemState *pid, AppContext *app, bool overUI) {
    if (!pid || !app) return false;
    float dt = GetFrameTime();
    if (pid->isPaletteOpen) {
        if (pid->animProgress < 1.0f) {
            pid->animProgress += dt * 6.0f;
            if (pid->animProgress > 1.0f) pid->animProgress = 1.0f;
        }

        Vector2 mouseScreen = g_CADState.mouseScreen;
        float scaledRadius = pid->paletteRadius * app->uiScale * pid->animProgress;
        float itemBtnSize = 42.0f * app->uiScale;
        pid->hoveredItemIndex = -1;

        for (int i = 0; i < CAD_PID_MAX_RADIAL_ITEMS; i++) {
            Vector2 itemCenter;
            CalculateItemRadialPosition(pid->paletteCenterScreen, scaledRadius, i, CAD_PID_MAX_RADIAL_ITEMS, &itemCenter);
            Rectangle btnRec = { itemCenter.x - itemBtnSize * 0.5f, itemCenter.y - itemBtnSize * 0.5f, itemBtnSize, itemBtnSize };
            if (CheckCollisionPointRec(mouseScreen, btnRec)) {
                pid->hoveredItemIndex = i;
                break;
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (pid->hoveredItemIndex >= 0) {
                pid->activeToolInstrument = (PIDInstrumentType)pid->hoveredItemIndex;
                pid->isPlacingInstrument = true;
                pid->placementStep = 0;
                snprintf(app->statusMessage, sizeof(app->statusMessage), "P&ID: Selected %s", kPidLabels[pid->hoveredItemIndex]);
                app->statusMessageTimer = 2.0f;
                CAD_PID_ClosePalette(pid);
                return true;
            } else {
                CAD_PID_ClosePalette(pid);
            }
        }
        if (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            CAD_PID_ClosePalette(pid);
            return true;
        }
        return true;
    }

    if (pid->isPlacingInstrument) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            pid->isPlacingInstrument = false;
            pid->placementStep = 0;
            snprintf(app->statusMessage, sizeof(app->statusMessage), "P&ID placement cancelled");
            app->statusMessageTimer = 1.5f;
            return false;
        }

        if (!overUI && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 worldTarget = g_CADState.mouseWorld;
            if (app->snapEnabled || app->snapToGrid) {
                worldTarget = (app->snapToGrid) ? (Vector2){ roundf(worldTarget.x / app->gridSpacing) * app->gridSpacing, roundf(worldTarget.y / app->gridSpacing) * app->gridSpacing } : worldTarget;
            }

            if (pid->activeToolInstrument == PID_ITEM_PIPE) {
                if (pid->placementStep == 0) {
                    pid->placementP1 = worldTarget;
                    pid->placementStep = 1;
                    snprintf(app->statusMessage, sizeof(app->statusMessage), "P&ID Pipe: Click End Position");
                    app->statusMessageTimer = 2.5f;
                } else if (pid->placementStep == 1) {
                    if (app->elementCount < MAX_ELEMENTS) {
                        GridElement pipeEl;
                        memset(&pipeEl, 0, sizeof(GridElement));
                        pipeEl.id = GenerateEntityID();
                        pipeEl.type = ELEMENT_LINE;
                        pipeEl.pos = (Vector2){ (pid->placementP1.x + worldTarget.x) * 0.5f, (pid->placementP1.y + worldTarget.y) * 0.5f };
                        pipeEl.scale = (Vector2){ 1.0f, 1.0f };
                        pipeEl.p1 = pid->placementP1;
                        pipeEl.p2 = worldTarget;
                        pipeEl.lineThickness = 4.0f;
                        pipeEl.useCustomColor = true;
                        pipeEl.color = SKYBLUE;
                        pipeEl.layerIndex = app->activeLayerIndex;
                        GetElementAABB(&pipeEl);

                        DeselectAllElements(app->elements, app->elementCount);
                        pipeEl.selected = true;

                        Command cmd = { 0 };
                        cmd.type = CMD_CREATE;
                        cmd.data.create.index = app->elementCount;
                        cmd.data.create.element = pipeEl;
                        ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);

                        snprintf(app->statusMessage, sizeof(app->statusMessage), "P&ID Pipe Inserted");
                        app->statusMessageTimer = 2.0f;
                    }
                    pid->placementStep = 0;
                    pid->isPlacingInstrument = false;
                }
            } else {
                CAD_PID_PlaceInstrument(pid->activeToolInstrument, worldTarget, app);
                pid->isPlacingInstrument = false;
                pid->placementStep = 0;
            }
            return true;
        }
    }
    return false;
}

void CAD_PID_RenderPalette(const PIDSystemState *pid, float uiScale, Font font) {
    if (!pid || !pid->isPaletteOpen) return;
    float currentRadius = pid->paletteRadius * uiScale * pid->animProgress;
    Vector2 center = pid->paletteCenterScreen;
    float itemBtnSize = 44.0f * uiScale;

    // Palette central indicator
    DrawCircleV(center, 18.0f * uiScale, Fade(DARKGRAY, 0.75f * pid->animProgress));
    DrawCircleLinesV(center, 18.0f * uiScale, Fade(GOLD, 0.9f * pid->animProgress));
    DrawTextEx(font, "PID", (Vector2){ center.x - 12.0f * uiScale, center.y - 7.0f * uiScale }, 12.0f * uiScale, 1.0f, RAYWHITE);

    // Orbit Guide Ring
    DrawCircleLinesV(center, currentRadius, Fade(LIGHTGRAY, 0.45f * pid->animProgress));

    for (int i = 0; i < CAD_PID_MAX_RADIAL_ITEMS; i++) {
        Vector2 itemCenter;
        CalculateItemRadialPosition(center, currentRadius, i, CAD_PID_MAX_RADIAL_ITEMS, &itemCenter);
        Rectangle btnRec = { itemCenter.x - itemBtnSize * 0.5f, itemCenter.y - itemBtnSize * 0.5f, itemBtnSize, itemBtnSize };

        bool isHovered = (pid->hoveredItemIndex == i);
        Color baseBg = isHovered ? Fade(MAROON, 0.9f) : Fade(DARKGRAY, 0.85f);
        Color borderCol = isHovered ? GOLD : Fade(WHITE, 0.7f);

        DrawRectangleRounded(btnRec, 0.35f, 8, baseBg);
        DrawRectangleRoundedLines(btnRec, 0.35f, 8, borderCol);

        Vector2 txtSize = MeasureTextEx(font, kPidLabels[i], 10.0f * uiScale, 1.0f);
        Vector2 txtPos = { itemCenter.x - txtSize.x * 0.5f, itemCenter.y - txtSize.y * 0.5f };
        DrawTextEx(font, kPidLabels[i], txtPos, 10.0f * uiScale, 1.0f, borderCol);
    }
}

void CAD_PID_RenderToolPreview(const PIDSystemState *pid, AppContext *app, Vector2 activeToolPoint) {
    if (!pid || !pid->isPlacingInstrument || !app) return;

    if (pid->activeToolInstrument == PID_ITEM_PIPE) {
        if (pid->placementStep == 1) {
            DrawLineEx(pid->placementP1, activeToolPoint, 4.0f / app->camera.zoom, Fade(SKYBLUE, 0.75f));
            DrawCircleV(pid->placementP1, 4.0f / app->camera.zoom, SKYBLUE);
            DrawCircleV(activeToolPoint, 4.0f / app->camera.zoom, SKYBLUE);
        }
    } else if (pid->activeToolInstrument == PID_ITEM_FLANGE) {
        GridElement previewEl = Flange_CreateGridElement(activeToolPoint, 0.0f, app->activeLayerIndex, &pid->currentFlangeSpec);
        Flange_DrawElement(&previewEl, Fade(previewEl.color, 0.7f), app->camera.zoom, false);
    } else {
        float size = 18.0f / app->camera.zoom;
        Rectangle previewRec = { activeToolPoint.x - size * 0.5f, activeToolPoint.y - size * 0.5f, size, size };
        DrawRectangleLinesEx(previewRec, 1.5f / app->camera.zoom, Fade(GOLD, 0.8f));
        DrawCircleV(activeToolPoint, 3.0f / app->camera.zoom, Fade(RED, 0.7f));
    }
}

bool CAD_PID_ValidatePlacement(PIDInstrumentType type, Vector2 worldPos, const AppContext *app) {
    if (!app) return false;
    (void)type;
    (void)worldPos;
    if (app->activeLayerIndex < 0 || app->activeLayerIndex >= app->layerCount) return false;
    if (app->layers[app->activeLayerIndex].locked || !app->layers[app->activeLayerIndex].visible) return false;
    return true;
}

void CAD_PID_PlaceInstrument(PIDInstrumentType type, Vector2 worldPos, AppContext *app) {
    if (!app || app->elementCount >= MAX_ELEMENTS) return;
    if (!CAD_PID_ValidatePlacement(type, worldPos, app)) {
        snprintf(app->statusMessage, sizeof(app->statusMessage), "Cannot place: Layer is locked or hidden!");
        app->statusMessageTimer = 2.0f;
        return;
    }

    GridElement newEl;
    memset(&newEl, 0, sizeof(GridElement));
    newEl.id = GenerateEntityID();
    newEl.scale = (Vector2){ 1.0f, 1.0f };
    newEl.layerIndex = app->activeLayerIndex;
    newEl.pos = worldPos;
    newEl.rotation = 0.0f;
    newEl.useCustomColor = true;

    switch (type) {
        case PID_ITEM_FLANGE: {
            newEl = Flange_CreateGridElement(worldPos, 0.0f, app->activeLayerIndex, &app->cadPid.currentFlangeSpec);
            break;
        }
        case PID_ITEM_VALVE: {
            newEl.type = ELEMENT_SYMBOL;
            newEl.color = GOLD;
            break;
        }
        case PID_ITEM_TEE: {
            newEl.type = ELEMENT_SYMBOL;
            newEl.color = LIME;
            break;
        }
        case PID_ITEM_REDUCER: {
            newEl.type = ELEMENT_SYMBOL;
            newEl.color = PURPLE;
            break;
        }
        case PID_ITEM_ELBOW: {
            newEl.type = ELEMENT_SYMBOL;
            newEl.color = ORANGE;
            break;
        }
        default:
            newEl.type = ELEMENT_SYMBOL;
            newEl.color = RAYWHITE;
            break;
    }

    GetElementAABB(&newEl);
    DeselectAllElements(app->elements, app->elementCount);
    newEl.selected = true;

    Command cmd = { 0 };
    cmd.type = CMD_CREATE;
    cmd.data.create.index = app->elementCount;
    cmd.data.create.element = newEl;
    ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);

    snprintf(app->statusMessage, sizeof(app->statusMessage), "P&ID: %s Added", kPidLabels[type]);
    app->statusMessageTimer = 2.0f;
}