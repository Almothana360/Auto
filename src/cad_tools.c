#include "cad_tools.h"
#include "cad_math.h"
#include "cad_pid.h"
#include "project_io.h"
#include "commands.h"
#include "render_utils.h"
#include "raymath.h"
#include <stdio.h>

void UpdateTools(AppContext *app, Vector2 activeToolPoint, bool overUI) {
    if (overUI) return;

    if (g_CADState.activeTool == TOOL_PID_PALETTE) {
        CAD_PID_OpenPalette(&app->cadPid, g_CADState.mouseScreen);
        g_CADState.activeTool = TOOL_SELECT;
        return;
    }

    if (g_CADState.activeTool == TOOL_ADD_LINE && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (app->dimStep == 0) {
            app->dimP1 = activeToolPoint;
            app->dimStep = 1;
            snprintf(app->statusMessage, 64, "Line: Click 2nd Point");
            app->statusMessageTimer = 3.0f;
        } else if (app->dimStep == 1) {
            if (app->elementCount < MAX_ELEMENTS) {
                GridElement newEl = { 0 };
                newEl.id = GenerateEntityID();
                newEl.type = ELEMENT_LINE;
                newEl.pos = (Vector2){ (app->dimP1.x + activeToolPoint.x) * 0.5f, (app->dimP1.y + activeToolPoint.y) * 0.5f };
                newEl.scale = (Vector2){ 1.0f, 1.0f };
                newEl.p1 = app->dimP1;
                newEl.p2 = activeToolPoint;
                newEl.lineThickness = 2.0f;
                newEl.useCustomColor = false;
                newEl.color = WHITE;
                newEl.layerIndex = app->activeLayerIndex;
                newEl.rotation = 0.0f;
                GetElementAABB(&newEl);
                DeselectAllElements(app->elements, app->elementCount);
                newEl.selected = true;
                Command cmd = { 0 };
                cmd.type = CMD_CREATE;
                cmd.data.create.index = app->elementCount;
                cmd.data.create.element = newEl;
                ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                snprintf(app->statusMessage, 64, "Line Added");
                app->statusMessageTimer = 2.0f;
            }
            app->dimStep = 0;
            g_CADState.activeTool = TOOL_SELECT;
        }
    } else if (g_CADState.activeTool == TOOL_ADD_POLYLINE && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (app->dimStep == 0) {
            app->tempPolyline.id = GenerateEntityID();
            app->tempPolyline.type = ELEMENT_POLYLINE;
            app->tempPolyline.scale = (Vector2){ 1.0f, 1.0f };
            app->tempPolyline.pos = activeToolPoint;
            app->tempPolyline.pointCount = 0;
            app->tempPolyline.lineThickness = 2.0f;
            app->tempPolyline.useCustomColor = false;
            app->tempPolyline.layerIndex = app->activeLayerIndex;
            app->tempPolyline.rotation = 0.0f;
            app->tempPolyline.points[app->tempPolyline.pointCount++] = activeToolPoint;
            app->dimStep = 1;
            snprintf(app->statusMessage, 64, "Polyline: Click next point (Right click to finish)");
            app->statusMessageTimer = 3.0f;
        } else if (app->tempPolyline.pointCount < MAX_POLYLINE_POINTS) {
            app->tempPolyline.points[app->tempPolyline.pointCount++] = activeToolPoint;
        }
    } else if (g_CADState.activeTool == TOOL_ADD_FREEHAND) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            app->tempPolyline.id = GenerateEntityID();
            app->tempPolyline.type = ELEMENT_FREEHAND;
            app->tempPolyline.scale = (Vector2){ 1.0f, 1.0f };
            app->tempPolyline.pos = activeToolPoint;
            app->tempPolyline.pointCount = 0;
            app->tempPolyline.lineThickness = 2.0f;
            app->tempPolyline.useCustomColor = false;
            app->tempPolyline.layerIndex = app->activeLayerIndex;
            app->tempPolyline.rotation = 0.0f;
            app->tempPolyline.points[app->tempPolyline.pointCount++] = activeToolPoint;
        } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && app->tempPolyline.pointCount < MAX_POLYLINE_POINTS) {
            Vector2 lastPt = app->tempPolyline.points[app->tempPolyline.pointCount - 1];
            if (Vector2Distance(lastPt, g_CADState.mouseWorld) > 4.0f / app->camera.zoom) {
                app->tempPolyline.points[app->tempPolyline.pointCount++] = g_CADState.mouseWorld;
            }
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && app->tempPolyline.pointCount > 1) {
            if (app->elementCount < MAX_ELEMENTS) {
                DeselectAllElements(app->elements, app->elementCount);
                app->tempPolyline.selected = true;
                GetElementAABB(&app->tempPolyline);
                Command cmd = { 0 };
                cmd.type = CMD_CREATE;
                cmd.data.create.index = app->elementCount;
                cmd.data.create.element = app->tempPolyline;
                ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                snprintf(app->statusMessage, 64, "Freehand Path Added");
                app->statusMessageTimer = 2.0f;
            }
            app->tempPolyline.pointCount = 0;
            g_CADState.activeTool = TOOL_SELECT;
        }
    } else if (g_CADState.activeTool == TOOL_ADD_ARC && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (app->dimStep == 0) {
            app->dimP1 = activeToolPoint;
            app->dimStep = 1;
            snprintf(app->statusMessage, 64, "Arc: Click 2nd Point");
            app->statusMessageTimer = 3.0f;
        } else if (app->dimStep == 1) {
            app->dimP2 = activeToolPoint;
            app->dimStep = 2;
            snprintf(app->statusMessage, 64, "Arc: Click End Point");
            app->statusMessageTimer = 3.0f;
        } else if (app->dimStep == 2) {
            app->dimP3 = activeToolPoint;
            Vector2 arcCenter; float radius, startAngle, endAngle;
            if (Calculate3PointArc(app->dimP1, app->dimP2, app->dimP3, &arcCenter, &radius, &startAngle, &endAngle) && app->elementCount < MAX_ELEMENTS) {
                GridElement newEl = { 0 };
                newEl.id = GenerateEntityID();
                newEl.type = ELEMENT_ARC;
                newEl.pos = arcCenter;
                newEl.scale = (Vector2){ 1.0f, 1.0f };
                newEl.radius = radius;
                newEl.startAngle = startAngle;
                newEl.endAngle = endAngle;
                newEl.p1 = app->dimP1; newEl.p2 = app->dimP2; newEl.p3 = app->dimP3;
                newEl.lineThickness = 2.0f;
                newEl.useCustomColor = false;
                newEl.layerIndex = app->activeLayerIndex;
                GetElementAABB(&newEl);
                DeselectAllElements(app->elements, app->elementCount);
                newEl.selected = true;
                Command cmd = { 0 };
                cmd.type = CMD_CREATE;
                cmd.data.create.index = app->elementCount;
                cmd.data.create.element = newEl;
                ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                snprintf(app->statusMessage, 64, "3-Point Arc Added");
                app->statusMessageTimer = 2.0f;
            }
            app->dimStep = 0;
            g_CADState.activeTool = TOOL_SELECT;
        }
    } else if (g_CADState.activeTool == TOOL_ADD_TEXT_NOTE && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (app->dimStep == 0) {
            app->dimP1 = activeToolPoint;
            app->dimStep = 1;
            snprintf(app->statusMessage, 64, "Text Note: Click Pointer Target");
            app->statusMessageTimer = 3.0f;
        } else if (app->dimStep == 1) {
            if (app->elementCount < MAX_ELEMENTS) {
                GridElement newEl = { 0 };
                newEl.id = GenerateEntityID();
                newEl.type = ELEMENT_TEXT_NOTE;
                newEl.pos = app->dimP1;
                newEl.scale = (Vector2){ 1.0f, 1.0f };
                newEl.width = 160.0f; newEl.height = 60.0f;
                newEl.arrowTarget = activeToolPoint;
                newEl.showArrow = true;
                snprintf(newEl.text, TEXT_NOTE_LEN, "Note Annotation");
                newEl.textSize = 16;
                newEl.useCustomColor = false;
                newEl.layerIndex = app->activeLayerIndex;
                GetElementAABB(&newEl);
                DeselectAllElements(app->elements, app->elementCount);
                newEl.selected = true;
                Command cmd = { 0 };
                cmd.type = CMD_CREATE;
                cmd.data.create.index = app->elementCount;
                cmd.data.create.element = newEl;
                ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                snprintf(app->statusMessage, 64, "Text Note Added");
                app->statusMessageTimer = 2.0f;
            }
            app->dimStep = 0;
            g_CADState.activeTool = TOOL_SELECT;
        }
    } else if (g_CADState.activeTool == TOOL_DIMENSION && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (app->dimStep == 0) {
            app->dimP1 = activeToolPoint;
            app->dimStep = 1;
            snprintf(app->statusMessage, 64, "Dimension: Click 2nd Point");
            app->statusMessageTimer = 3.0f;
        } else if (app->dimStep == 1) {
            app->dimP2 = activeToolPoint;
            app->dimStep = 2;
            snprintf(app->statusMessage, 64, "Dimension: Click Placement Position");
            app->statusMessageTimer = 3.0f;
        } else if (app->dimStep == 2) {
            if (app->elementCount < MAX_ELEMENTS) {
                GridElement newEl = { 0 };
                newEl.id = GenerateEntityID();
                newEl.type = ELEMENT_DIMENSION;
                newEl.pos = (Vector2){ (app->dimP1.x + app->dimP2.x) * 0.5f, (app->dimP1.y + app->dimP2.y) * 0.5f };
                newEl.scale = (Vector2){ 1.0f, 1.0f };
                newEl.p1 = app->dimP1; newEl.p2 = app->dimP2; newEl.dimPos = activeToolPoint;
                newEl.lineThickness = 2.0f; newEl.tickThickness = 2.0f; newEl.textSize = 14;
                newEl.color = PURPLE; newEl.useCustomColor = true; newEl.layerIndex = app->activeLayerIndex;
                newEl.rotation = 0.0f;
                GetElementAABB(&newEl);
                DeselectAllElements(app->elements, app->elementCount);
                newEl.selected = true;
                Command cmd = { 0 };
                cmd.type = CMD_CREATE;
                cmd.data.create.index = app->elementCount;
                cmd.data.create.element = newEl;
                ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                snprintf(app->statusMessage, 64, "Dimension Added");
                app->statusMessageTimer = 2.0f;
            }
            app->dimStep = 0;
            g_CADState.activeTool = TOOL_SELECT;
        }
    } else if (g_CADState.activeTool != TOOL_SELECT && g_CADState.activeTool != TOOL_DIMENSION &&
               g_CADState.activeTool != TOOL_ADD_LINE && g_CADState.activeTool != TOOL_ADD_POLYLINE &&
               g_CADState.activeTool != TOOL_ADD_FREEHAND && g_CADState.activeTool != TOOL_ADD_ARC &&
               g_CADState.activeTool != TOOL_ADD_TEXT_NOTE && g_CADState.activeTool != TOOL_PAN &&
               IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (app->layers[app->activeLayerIndex].locked || !app->layers[app->activeLayerIndex].visible) {
            snprintf(app->statusMessage, 64, "Cannot draw: Active Layer Locked/Hidden!");
            app->statusMessageTimer = 2.0f;
        } else if (app->elementCount < MAX_ELEMENTS) {
            GridElement newEl = { 0 };
            newEl.id = GenerateEntityID();
            newEl.scale = (Vector2){ 1.0f, 1.0f };
            if (g_CADState.activeTool == TOOL_ADD_ELLIPSE) {
                newEl.type = ELEMENT_ELLIPSE;
                newEl.pos = activeToolPoint;
                newEl.radiusX = 60.0f; newEl.radiusY = 40.0f;
            } else {
                newEl.type = (g_CADState.activeTool == TOOL_ADD_RECT) ? ELEMENT_RECT : ELEMENT_CIRCLE;
                newEl.pos = activeToolPoint; newEl.width = 80.0f; newEl.height = 60.0f; newEl.radius = 40.0f;
            }
            newEl.rotation = 0.0f;
            newEl.useCustomColor = false;
            newEl.color = WHITE;
            newEl.layerIndex = app->activeLayerIndex;
            GetElementAABB(&newEl);
            DeselectAllElements(app->elements, app->elementCount);
            newEl.selected = true;
            Command cmd = { 0 };
            cmd.type = CMD_CREATE;
            cmd.data.create.index = app->elementCount;
            cmd.data.create.element = newEl;
            ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
        }
        g_CADState.activeTool = TOOL_SELECT;
    }
}

void RenderToolPreviews(AppContext *app, Vector2 activeToolPoint, Font noteFont) {
    if (g_CADState.activeTool == TOOL_ADD_POLYLINE && app->tempPolyline.pointCount > 0) {
        for (int p = 0; p < app->tempPolyline.pointCount - 1; p++) {
            DrawLineEx(app->tempPolyline.points[p], app->tempPolyline.points[p+1], 2.0f / app->camera.zoom, SKYBLUE);
        }
        DrawLineEx(app->tempPolyline.points[app->tempPolyline.pointCount - 1], activeToolPoint, 2.0f / app->camera.zoom, SKYBLUE);
    }
    if (g_CADState.activeTool == TOOL_ADD_LINE && app->dimStep == 1) {
        DrawLineEx(app->dimP1, activeToolPoint, 2.0f / app->camera.zoom, SKYBLUE);
    }
    if (g_CADState.activeTool == TOOL_DIMENSION) {
        if (app->dimStep == 1) {
            DrawLineEx(app->dimP1, activeToolPoint, 2.0f / app->camera.zoom, SKYBLUE);
        } else if (app->dimStep == 2) {
            GridElement previewDim = {0};
            previewDim.type = ELEMENT_DIMENSION;
            previewDim.p1 = app->dimP1;
            previewDim.p2 = app->dimP2;
            previewDim.dimPos = activeToolPoint;
            previewDim.lineThickness = 2.0f;
            previewDim.tickThickness = 2.0f;
            previewDim.textSize = 14;
            previewDim.color = SKYBLUE;
            previewDim.useCustomColor = true;
            DrawDimensionElement(&previewDim, app->currentUnit, app->camera.zoom, false, noteFont);
        }
    }
    if (g_CADState.activeTool == TOOL_ADD_ARC) {
        if (app->dimStep == 1) {
            DrawLineEx(app->dimP1, activeToolPoint, 2.0f / app->camera.zoom, SKYBLUE);
        } else if (app->dimStep == 2) {
            Vector2 arcCenter; float radius, startAngle, endAngle;
            if (Calculate3PointArc(app->dimP1, app->dimP2, activeToolPoint, &arcCenter, &radius, &startAngle, &endAngle)) {
                DrawRing(arcCenter, radius - 1.5f/app->camera.zoom, radius + 1.5f/app->camera.zoom, startAngle, endAngle, 32, SKYBLUE);
            }
        }
    }
}