#include "cad_editor.h"
#include "cad_camera.h"
#include "cad_tools.h"
#include "cad_selection.h"
#include "cad_math.h"
#include "spatial_tree.h"
#include "project_io.h"
#include "render_utils.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void UpdateCadEditor(AppContext *app, mu_Context *mu_ctx, bool overUI) {
    (void)mu_ctx;
    Vector2 activeToolPoint = g_CADState.mouseWorld;
    bool isElementSnapped = false;

    // Viewport Context snapping calculations
    if (!overUI && g_CADState.activeTool != TOOL_PAN) {
        if (app->snapEnabled) {
            activeToolPoint = GetClosestSnapPoint_Spatial(app->spatialTree, g_CADState.mouseWorld, app->elements, app->elementCount, app->layers, 20.0f / app->camera.zoom);
            isElementSnapped = (activeToolPoint.x != g_CADState.mouseWorld.x || activeToolPoint.y != g_CADState.mouseWorld.y);
        }
        if (app->snapToGrid && !isElementSnapped) {
            activeToolPoint.x = roundf(g_CADState.mouseWorld.x / app->gridSpacing) * app->gridSpacing;
            activeToolPoint.y = roundf(g_CADState.mouseWorld.y / app->gridSpacing) * app->gridSpacing;
        }
    }

    int selectedCount = CountSelectedElements(app->elements, app->elementCount);

    // Right-Click Context and Tool Resolution
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !overUI && g_CADState.activeTool != TOOL_PAN) {
        if (g_CADState.activeTool == TOOL_ADD_POLYLINE && app->tempPolyline.pointCount > 1) {
            if (app->elementCount < MAX_ELEMENTS) {
                DeselectAllElements(app->elements, app->elementCount);
                app->tempPolyline.id = GenerateEntityID();
                app->tempPolyline.scale = (Vector2){ 1.0f, 1.0f };
                app->tempPolyline.selected = true;
                GetElementAABB(&app->tempPolyline);
                Command cmd = { 0 };
                cmd.type = CMD_CREATE;
                cmd.data.create.index = app->elementCount;
                cmd.data.create.element = app->tempPolyline;
                ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                snprintf(app->statusMessage, 64, "Polyline Added"); app->statusMessageTimer = 2.0f;
            }
            app->tempPolyline.pointCount = 0;
            g_CADState.activeTool = TOOL_SELECT;
        } else {
            int hitIndex = HitTestElement_Spatial(app->spatialTree, app->elements, app->elementCount, app->layers, app->layerCount, g_CADState.mouseWorld);
            app->showContextMenu = true;
            app->contextMenuPos = g_CADState.mouseScreen;
            float ctxWidth = 180.0f * app->uiScale;
            float ctxHeight = app->contextOnElement ? (210.0f * app->uiScale) : (210.0f * app->uiScale);
            if (app->contextMenuPos.x + ctxWidth > GetScreenWidth()) app->contextMenuPos.x = GetScreenWidth() - ctxWidth;
            if (app->contextMenuPos.y + ctxHeight > GetScreenHeight()) app->contextMenuPos.y = GetScreenHeight() - ctxHeight;
            if (hitIndex != -1) {
                app->contextOnElement = true;
                app->contextElementIndex = hitIndex;
                if (!IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL) && !app->elements[hitIndex].selected) {
                    DeselectAllElements(app->elements, app->elementCount);
                }
                app->elements[hitIndex].selected = true;
            } else {
                app->contextOnElement = false;
                app->contextElementIndex = -1;
            }
        }
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && app->showContextMenu && !overUI) {
        app->showContextMenu = false;
    }

    // Hand-off execution to modular components
    UpdateTools(app, activeToolPoint, overUI);
    UpdateSelectionAndHandles(app, activeToolPoint, overUI, selectedCount);
    UpdateCameraInput(app, overUI);
}

void RenderCadEditorViewport(AppContext *app, bool overUI) {
    (void)overUI;
    Font noteFont = ResourceManager_GetFont(&app->resManager, FONT_SLOT_NOTE);

    BeginMode2D(app->camera);

    // Frustum and Grid Mapping
    Vector2 topLeft = GetScreenToWorld2D((Vector2){ 0, 0 }, app->camera);
    Vector2 bottomRight = GetScreenToWorld2D((Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() }, app->camera);
    int startX = (int)(floorf(topLeft.x / app->gridSpacing) * app->gridSpacing); int endX   = (int)(ceilf(bottomRight.x / app->gridSpacing) * app->gridSpacing);
    int startY = (int)(floorf(topLeft.y / app->gridSpacing) * app->gridSpacing); int endY   = (int)(ceilf(bottomRight.y / app->gridSpacing) * app->gridSpacing);

    for (int x = startX; x <= endX; x += (int)app->gridSpacing) DrawLine(x, startY, x, endY, LIGHTGRAY);
    for (int y = startY; y <= endY; y += (int)app->gridSpacing) DrawLine(startX, y, endX, y, LIGHTGRAY);
    DrawLine(0, startY, 0, endY, RED); DrawLine(startX, 0, endX, 0, GREEN); DrawCircle(0, 0, 4, DARKBLUE);

    AABB viewFrustumAABB = {
        .min = { fminf(topLeft.x, bottomRight.x), fminf(topLeft.y, bottomRight.y) },
        .max = { fmaxf(topLeft.x, bottomRight.x), fmaxf(topLeft.y, bottomRight.y) }
    };

    // Render Layer Elements
    for (int r = -100; r <= 100; r++) {
        for (int l = 0; l < app->layerCount; l++) {
            if (!app->layers[l].visible || app->layers[l].renderOrder != r) continue;
            for (int i = 0; i < app->elementCount; i++) {
                if (app->elements[i].layerIndex != l) continue;
                if (!AABBIntersectsAABB(app->cachedAABBs[i], viewFrustumAABB)) continue;

                bool isSelected = app->elements[i].selected;
                Color renderColor = GetElementColor(&app->elements[i], app->layers, app->layerCount);
                if (app->elements[i].type == ELEMENT_RECT) {
                    Rectangle rect = { app->elements[i].pos.x, app->elements[i].pos.y, app->elements[i].width * app->elements[i].scale.x, app->elements[i].height * app->elements[i].scale.y };
                    Vector2 origin = { rect.width * 0.5f, rect.height * 0.5f };
                    DrawRectanglePro(rect, origin, app->elements[i].rotation, renderColor);
                } else if (app->elements[i].type == ELEMENT_CIRCLE) {
                    DrawCircleV(app->elements[i].pos, app->elements[i].radius * app->elements[i].scale.x, renderColor);
                    DrawCircleLines((int)app->elements[i].pos.x, (int)app->elements[i].pos.y, app->elements[i].radius * app->elements[i].scale.x, renderColor);
                } else if (app->elements[i].type == ELEMENT_ELLIPSE) {
                    DrawEllipse((int)app->elements[i].pos.x, (int)app->elements[i].pos.y, app->elements[i].radiusX * app->elements[i].scale.x, app->elements[i].radiusY * app->elements[i].scale.y, renderColor);
                    DrawEllipseLines((int)app->elements[i].pos.x, (int)app->elements[i].pos.y, app->elements[i].radiusX * app->elements[i].scale.x, app->elements[i].radiusY * app->elements[i].scale.y, renderColor);
                } else if (app->elements[i].type == ELEMENT_LINE) {
                    DrawLineEx(app->elements[i].p1, app->elements[i].p2, isSelected ? (app->elements[i].lineThickness + 2.0f)/app->camera.zoom : app->elements[i].lineThickness/app->camera.zoom, isSelected ? GOLD : renderColor);
                    DrawCircleV(app->elements[i].p1, 4.0f / app->camera.zoom, isSelected ? GOLD : renderColor); DrawCircleV(app->elements[i].p2, 4.0f / app->camera.zoom, isSelected ? GOLD : renderColor);
                } else if (app->elements[i].type == ELEMENT_POLYLINE || app->elements[i].type == ELEMENT_FREEHAND) {
                    for (int p = 0; p < app->elements[i].pointCount - 1; p++) {
                        DrawLineEx(app->elements[i].points[p], app->elements[i].points[p+1], isSelected ? (app->elements[i].lineThickness + 2.0f)/app->camera.zoom : app->elements[i].lineThickness/app->camera.zoom, isSelected ? GOLD : renderColor);
                    }
                } else if (app->elements[i].type == ELEMENT_ARC) {
                    DrawRing(app->elements[i].pos, app->elements[i].radius - 1.5f/app->camera.zoom, app->elements[i].radius + 1.5f/app->camera.zoom, app->elements[i].startAngle, app->elements[i].endAngle, 32, isSelected ? GOLD : renderColor);
                } else if (app->elements[i].type == ELEMENT_TEXT_NOTE) {
                    DrawTextNoteElement(&app->elements[i], app->camera.zoom, isSelected, noteFont);
                } else if (app->elements[i].type == ELEMENT_DIMENSION) {
                    DrawDimensionElement(&app->elements[i], app->currentUnit, app->camera.zoom, isSelected, noteFont);
                } else if (app->elements[i].type == ELEMENT_SYMBOL) {
                    float sSize = 16.0f * app->elements[i].scale.x;
                    DrawRectanglePro((Rectangle){ app->elements[i].pos.x, app->elements[i].pos.y, sSize, sSize }, (Vector2){ sSize * 0.5f, sSize * 0.5f }, app->elements[i].rotation + 45.0f, renderColor);
                    DrawCircleLines((int)app->elements[i].pos.x, (int)app->elements[i].pos.y, sSize * 0.7f, isSelected ? GOLD : renderColor);
                }
            }
        }
    }

    // Resolve Tool/Snap Points for Rendering Output
    Vector2 activeToolPoint = g_CADState.mouseWorld;
    bool isElementSnapped = false;
    if (app->snapEnabled) {
        activeToolPoint = GetClosestSnapPoint_Spatial(app->spatialTree, g_CADState.mouseWorld, app->elements, app->elementCount, app->layers, 20.0f / app->camera.zoom);
        isElementSnapped = (activeToolPoint.x != g_CADState.mouseWorld.x || activeToolPoint.y != g_CADState.mouseWorld.y);
    }
    if (app->snapToGrid && !isElementSnapped) {
        activeToolPoint.x = roundf(g_CADState.mouseWorld.x / app->gridSpacing) * app->gridSpacing;
        activeToolPoint.y = roundf(g_CADState.mouseWorld.y / app->gridSpacing) * app->gridSpacing;
    }

    // Draw Snapping Indicators
    if (!overUI && g_CADState.activeTool != TOOL_SELECT && g_CADState.activeTool != TOOL_PAN) {
        Color snapIndicatorColor = isElementSnapped ? LIME : SKYBLUE;
        float markerRadius = (isElementSnapped ? 6.0f : 4.0f) / app->camera.zoom;
        DrawCircleV(activeToolPoint, markerRadius, Fade(snapIndicatorColor, 0.8f));
        DrawCircleLinesV(activeToolPoint, (markerRadius + 2.0f / app->camera.zoom), snapIndicatorColor);
    }

    // Hand-off Render Execution to modular components
    if (!overUI) {
        RenderToolPreviews(app, activeToolPoint, noteFont);
    }
    RenderSelectionGizmos(app);

    EndMode2D();
}