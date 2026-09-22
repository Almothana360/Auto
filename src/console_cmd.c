#include "console_cmd.h"
#include "cad_math.h"
#include "project_io.h"
#include "layer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static void TrimString(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
}

void ProcessCommand(const char *cmdStr, GridElement *elements, int *elementCount, 
                    Layer *layers, int *layerCount, int *activeLayerIndex, 
                    CADTool *currentTool, Camera2D *camera, bool *showHudPanel, 
                    bool *showInspector, bool *showLayersPanel, bool *showElementsPanel, 
                    float *uiScale, char *statusMessage, float *statusMessageTimer, 
                    int *dimStep, CommandHistory *cmdHistory, bool *spatialIndexDirty, 
                    MeasureUnit currentUnit) {
    (void)uiScale;
    (void)currentUnit;
    char buf[CMD_BUFFER_SIZE];
    strncpy(buf, cmdStr, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    TrimString(buf);
    char *cmdPtr = buf;
    if (cmdPtr[0] == '/') { cmdPtr++; TrimString(cmdPtr); }
    if (strlen(cmdPtr) == 0) return;

    if (StringEqualsIgnoreCase(cmdPtr, "new")) {
        Command snapCmd = { 0 };
        snapCmd.type = CMD_SNAPSHOT;
        snapCmd.data.snapshot.before.elementCount = *elementCount;
        snapCmd.data.snapshot.before.elements = (GridElement*)calloc(*elementCount > 0 ? *elementCount : 1, sizeof(GridElement));
        if (*elementCount > 0) memcpy(snapCmd.data.snapshot.before.elements, elements, sizeof(GridElement) * (*elementCount));
        snapCmd.data.snapshot.before.layerCount = *layerCount;
        for (int l = 0; l < *layerCount; l++) Layer_Copy(&snapCmd.data.snapshot.before.layers[l], &layers[l]);

        *elementCount = 0;
        DeselectAllElements(elements, *elementCount);
        *currentTool = TOOL_SELECT;
        g_CADState.activeTool = TOOL_SELECT;
        InitDefaultLayers(layers, layerCount);
        *activeLayerIndex = 0;
        g_CADState.activeLayerIndex = 0;
        g_CADState.activeLayerId = layers[0].id;

        snapCmd.data.snapshot.after.elementCount = *elementCount;
        snapCmd.data.snapshot.after.elements = (GridElement*)calloc(1, sizeof(GridElement));
        snapCmd.data.snapshot.after.layerCount = *layerCount;
        for (int l = 0; l < *layerCount; l++) Layer_Copy(&snapCmd.data.snapshot.after.layers[l], &layers[l]);

        ExecuteCommand(cmdHistory, snapCmd, elements, elementCount, layers, layerCount, spatialIndexDirty);
        snprintf(statusMessage, 64, "Created New Project"); *statusMessageTimer = 2.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "open")) {
        Command snapCmd = { 0 };
        snapCmd.type = CMD_SNAPSHOT;
        snapCmd.data.snapshot.before.elementCount = *elementCount;
        snapCmd.data.snapshot.before.elements = (GridElement*)calloc(*elementCount > 0 ? *elementCount : 1, sizeof(GridElement));
        if (*elementCount > 0) memcpy(snapCmd.data.snapshot.before.elements, elements, sizeof(GridElement) * (*elementCount));
        snapCmd.data.snapshot.before.layerCount = *layerCount;
        for (int l = 0; l < *layerCount; l++) Layer_Copy(&snapCmd.data.snapshot.before.layers[l], &layers[l]);

        if (LoadProject(PROJECT_FILENAME, elements, elementCount, layers, layerCount)) {
            DeselectAllElements(elements, *elementCount);
            *activeLayerIndex = 0;
            g_CADState.activeLayerIndex = 0;
            g_CADState.activeLayerId = layers[0].id;

            snapCmd.data.snapshot.after.elementCount = *elementCount;
            snapCmd.data.snapshot.after.elements = (GridElement*)calloc(*elementCount > 0 ? *elementCount : 1, sizeof(GridElement));
            if (*elementCount > 0) memcpy(snapCmd.data.snapshot.after.elements, elements, sizeof(GridElement) * (*elementCount));
            snapCmd.data.snapshot.after.layerCount = *layerCount;
            for (int l = 0; l < *layerCount; l++) Layer_Copy(&snapCmd.data.snapshot.after.layers[l], &layers[l]);

            ExecuteCommand(cmdHistory, snapCmd, elements, elementCount, layers, layerCount, spatialIndexDirty);
            snprintf(statusMessage, 64, "Project Loaded Successfully!");
        } else {
            FreeCommand(&snapCmd);
            snprintf(statusMessage, 64, "Error: Failed to Load Project!");
        }
        *statusMessageTimer = 2.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "save")) {
        SaveProject(PROJECT_FILENAME, elements, *elementCount, layers, *layerCount);
        snprintf(statusMessage, 64, "Project Saved (JSON) Successfully!"); *statusMessageTimer = 2.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "undo")) {
        if (History_Undo(cmdHistory, elements, elementCount, layers, layerCount, spatialIndexDirty)) {
            snprintf(statusMessage, 64, "Undo Performed");
        } else snprintf(statusMessage, 64, "Nothing to Undo");
        *statusMessageTimer = 1.5f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "redo")) {
        if (History_Redo(cmdHistory, elements, elementCount, layers, layerCount, spatialIndexDirty)) {
            snprintf(statusMessage, 64, "Redo Performed");
        } else snprintf(statusMessage, 64, "Nothing to Redo");
        *statusMessageTimer = 1.5f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "exit") || StringEqualsIgnoreCase(cmdPtr, "quit")) {
        CloseWindow();
    } else if (StringEqualsIgnoreCase(cmdPtr, "reset view") || StringEqualsIgnoreCase(cmdPtr, "reset")) {
        camera->target = (Vector2){ 0.0f, 0.0f };
        camera->offset = (Vector2){ (float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f };
        camera->zoom = 1.0f;
        snprintf(statusMessage, 64, "View Reset"); *statusMessageTimer = 1.5f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "fullscreen")) {
        ToggleFullscreen();
    } else if (StringEqualsIgnoreCase(cmdPtr, "toggle hud") || StringEqualsIgnoreCase(cmdPtr, "hud")) {
        *showHudPanel = !(*showHudPanel);
    } else if (StringEqualsIgnoreCase(cmdPtr, "toggle inspector") || StringEqualsIgnoreCase(cmdPtr, "inspector")) {
        *showInspector = !(*showInspector);
    } else if (StringEqualsIgnoreCase(cmdPtr, "toggle layers") || StringEqualsIgnoreCase(cmdPtr, "layers")) {
        *showLayersPanel = !(*showLayersPanel);
    } else if (StringEqualsIgnoreCase(cmdPtr, "toggle elements") || StringEqualsIgnoreCase(cmdPtr, "elements")) {
        *showElementsPanel = !(*showElementsPanel);
    } else if (StringEqualsIgnoreCase(cmdPtr, "select") || StringEqualsIgnoreCase(cmdPtr, "sel")) {
        *currentTool = TOOL_SELECT; g_CADState.activeTool = TOOL_SELECT; snprintf(statusMessage, 64, "Tool: Select & Transform"); *statusMessageTimer = 2.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "pipe") || StringEqualsIgnoreCase(cmdPtr, "draw pipe")) {
        *currentTool = TOOL_DRAW_PIPE; g_CADState.activeTool = TOOL_DRAW_PIPE; *dimStep = 0; snprintf(statusMessage, 64, "Tool: Draw Pipe (Click Start Point)"); *statusMessageTimer = 2.5f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "flange") || StringEqualsIgnoreCase(cmdPtr, "place flange")) {
        *currentTool = TOOL_PLACE_FLANGE; g_CADState.activeTool = TOOL_PLACE_FLANGE; snprintf(statusMessage, 64, "Tool: Place Flange Symbol"); *statusMessageTimer = 2.5f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "add rect") || StringEqualsIgnoreCase(cmdPtr, "rect") || StringEqualsIgnoreCase(cmdPtr, "r")) {
        *currentTool = TOOL_ADD_RECT; g_CADState.activeTool = TOOL_ADD_RECT; snprintf(statusMessage, 64, "Tool: Click to place Rectangle"); *statusMessageTimer = 2.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "add circle") || StringEqualsIgnoreCase(cmdPtr, "circle") || StringEqualsIgnoreCase(cmdPtr, "c")) {
        *currentTool = TOOL_ADD_CIRCLE; g_CADState.activeTool = TOOL_ADD_CIRCLE; snprintf(statusMessage, 64, "Tool: Click to place Circle"); *statusMessageTimer = 2.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "polyline") || StringEqualsIgnoreCase(cmdPtr, "pline")) {
        *currentTool = TOOL_ADD_POLYLINE; g_CADState.activeTool = TOOL_ADD_POLYLINE; *dimStep = 0; snprintf(statusMessage, 64, "Polyline: Click Points (Right-Click End)"); *statusMessageTimer = 3.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "freehand") || StringEqualsIgnoreCase(cmdPtr, "draw")) {
        *currentTool = TOOL_ADD_FREEHAND; g_CADState.activeTool = TOOL_ADD_FREEHAND; snprintf(statusMessage, 64, "Freehand: Drag to Draw"); *statusMessageTimer = 3.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "arc")) {
        *currentTool = TOOL_ADD_ARC; g_CADState.activeTool = TOOL_ADD_ARC; *dimStep = 0; snprintf(statusMessage, 64, "3-Pt Arc: Click 1st Point"); *statusMessageTimer = 3.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "ellipse")) {
        *currentTool = TOOL_ADD_ELLIPSE; g_CADState.activeTool = TOOL_ADD_ELLIPSE; snprintf(statusMessage, 64, "Ellipse: Click Center Position"); *statusMessageTimer = 3.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "text") || StringEqualsIgnoreCase(cmdPtr, "note")) {
        *currentTool = TOOL_ADD_TEXT_NOTE; g_CADState.activeTool = TOOL_ADD_TEXT_NOTE; *dimStep = 0; snprintf(statusMessage, 64, "Text Note: Click Box Position"); *statusMessageTimer = 3.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "line") || StringEqualsIgnoreCase(cmdPtr, "l")) {
        *currentTool = TOOL_ADD_LINE; g_CADState.activeTool = TOOL_ADD_LINE; *dimStep = 0; snprintf(statusMessage, 64, "Line: Click 1st Point"); *statusMessageTimer = 3.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "dimension") || StringEqualsIgnoreCase(cmdPtr, "dim")) {
        *currentTool = TOOL_DIMENSION; g_CADState.activeTool = TOOL_DIMENSION; *dimStep = 0; snprintf(statusMessage, 64, "Dimension: Click 1st Point"); *statusMessageTimer = 3.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "pan")) {
        *currentTool = TOOL_PAN; g_CADState.activeTool = TOOL_PAN; snprintf(statusMessage, 64, "Pan Mode: ON"); *statusMessageTimer = 2.0f;
    } else if (StringEqualsIgnoreCase(cmdPtr, "clear") || StringEqualsIgnoreCase(cmdPtr, "clear elements")) {
        if (*elementCount > 0) {
            Command batchDel = { 0 };
            batchDel.type = CMD_DELETE_BATCH;
            batchDel.data.deleteBatch.count = *elementCount;
            batchDel.data.deleteBatch.items = (IndexedElement*)malloc(sizeof(IndexedElement) * (*elementCount));
            for (int i = 0; i < *elementCount; i++) {
                batchDel.data.deleteBatch.items[i].index = i;
                batchDel.data.deleteBatch.items[i].element = elements[i];
            }
            ExecuteCommand(cmdHistory, batchDel, elements, elementCount, layers, layerCount, spatialIndexDirty);
            DeselectAllElements(elements, *elementCount);
            *currentTool = TOOL_SELECT;
            g_CADState.activeTool = TOOL_SELECT;
            snprintf(statusMessage, 64, "All Elements Cleared"); *statusMessageTimer = 2.0f;
        }
    } else {
        snprintf(statusMessage, 64, "Unknown Command: %s", cmdPtr); *statusMessageTimer = 2.5f;
    }
}