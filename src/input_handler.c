#include "input_handler.h"
#include "console_cmd.h"
#include "project_io.h"
#include "commands.h"
#include "cad_pid.h"
#include <stdio.h>
#include <stdlib.h>

void HandleGlobalInput(AppContext *app, mu_Context *mu_ctx) {
    bool isTextInputActive = app->commandEditMode;
    if (app->uiConfig.uiBackend == UI_BACKEND_MICROUI && mu_ctx) {
        isTextInputActive = isTextInputActive || (mu_ctx->focus != 0) || (mu_ctx->number_edit != 0);
    } else {
        isTextInputActive = isTextInputActive || app->layerRenameEditMode || app->noteTextEditMode;
    }

    int selectedCount = CountSelectedElements(app->elements, app->elementCount);

    if (!isTextInputActive && !app->showUnitWindow && !app->showScaleWindow) {
        if (IsKeyPressed(KEY_DELETE) && selectedCount > 0) {
            int selTotal = 0;
            for (int i = 0; i < app->elementCount; i++) if (app->elements[i].selected) selTotal++;
            Command batchDel = { 0 };
            batchDel.type = CMD_DELETE_BATCH;
            batchDel.data.deleteBatch.count = selTotal;
            batchDel.data.deleteBatch.items = (IndexedElement*)malloc(sizeof(IndexedElement) * selTotal);
            int ins = 0;
            for (int i = 0; i < app->elementCount; i++) {
                if (app->elements[i].selected) {
                    batchDel.data.deleteBatch.items[ins].index = i;
                    batchDel.data.deleteBatch.items[ins].element = app->elements[i];
                    ins++;
                }
            }
            ExecuteCommand(app->cmdHistory, batchDel, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Selected Elements Deleted");
            app->statusMessageTimer = 1.5f;
        }
        else if (IsKeyPressed(KEY_SLASH) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            app->commandEditMode = true;
        }
        else if (IsKeyPressed(KEY_S) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) {
            g_CADState.activeTool = TOOL_SELECT;
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Tool: Select & Transform");
            app->statusMessageTimer = 2.0f;
        }
        else if (IsKeyPressed(KEY_R) && g_CADState.activeTool != TOOL_PAN) {
            g_CADState.activeTool = TOOL_ADD_RECT;
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Tool: Click to place Rectangle");
            app->statusMessageTimer = 2.0f;
        }
        else if (IsKeyPressed(KEY_C) && g_CADState.activeTool != TOOL_PAN) {
            g_CADState.activeTool = TOOL_ADD_CIRCLE;
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Tool: Click to place Circle");
            app->statusMessageTimer = 2.0f;
        }
        else if (IsKeyPressed(KEY_P) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL) && g_CADState.activeTool != TOOL_PAN) {
            CAD_PID_OpenPalette(&app->cadPid, g_CADState.mouseScreen);
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Tool: P&ID Circular Palate");
            app->statusMessageTimer = 2.0f;
        }
        else if (IsKeyPressed(KEY_L) && g_CADState.activeTool != TOOL_PAN) {
            g_CADState.activeTool = TOOL_ADD_LINE;
            app->dimStep = 0;
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Tool: Line");
            app->statusMessageTimer = 2.0f;
        }
        else if (IsKeyPressed(KEY_A) && g_CADState.activeTool != TOOL_PAN) {
            g_CADState.activeTool = TOOL_ADD_ARC;
            app->dimStep = 0;
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Tool: 3-Pt Arc (Click 1st Point)");
            app->statusMessageTimer = 2.0f;
        }
        else if (IsKeyPressed(KEY_T) && g_CADState.activeTool != TOOL_PAN) {
            g_CADState.activeTool = TOOL_ADD_TEXT_NOTE;
            app->dimStep = 0;
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Tool: Text Note");
            app->statusMessageTimer = 2.0f;
        }
        else if (IsKeyPressed(KEY_D) && g_CADState.activeTool != TOOL_PAN) {
            g_CADState.activeTool = TOOL_DIMENSION;
            app->dimStep = 0;
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Dimension: Click 1st Point");
            app->statusMessageTimer = 3.0f;
        }
        else if (IsKeyPressed(KEY_ESCAPE)) {
            g_CADState.activeTool = TOOL_SELECT;
            DeselectAllElements(app->elements, app->elementCount);
            app->showContextMenu = false;
            CAD_PID_ClosePalette(&app->cadPid);
            app->cadPid.isPlacingInstrument = false;
            app->dimStep = 0;
            app->isBoxSelecting = false;
            app->showScaleWindow = false;
            app->showUnitWindow = false;
            app->activeHandle = HANDLE_NONE;
            app->layerRenameEditMode = false;
            app->noteTextEditMode = false;
            if (mu_ctx) {
                mu_ctx->focus = 0;
                mu_ctx->number_edit = 0;
            }
        }
        else if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_Z)) {
            if (History_Undo(app->cmdHistory, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty)) {
                snprintf(app->statusMessage, sizeof(app->statusMessage), "Undo Performed");
                app->statusMessageTimer = 1.5f;
            }
        }
        else if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_Y)) {
            if (History_Redo(app->cmdHistory, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty)) {
                snprintf(app->statusMessage, sizeof(app->statusMessage), "Redo Performed");
                app->statusMessageTimer = 1.5f;
            }
        }
        else if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_S)) {
            SaveProject(PROJECT_FILENAME, app->elements, app->elementCount, app->layers, app->layerCount);
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Project Saved (JSON) Successfully!");
            app->statusMessageTimer = 2.0f;
        }
        else if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_O)) {
            ProcessCommand("open", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit);
        }
    } else if (app->commandEditMode) {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            ProcessCommand(app->commandText, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit);
            app->commandText[0] = '\0';
            app->commandEditMode = false;
        } else if (IsKeyPressed(KEY_ESCAPE)) {
            app->commandEditMode = false;
        }
    } else {
        if (IsKeyPressed(KEY_ESCAPE)) {
            app->layerRenameEditMode = false;
            app->noteTextEditMode = false;
            if (mu_ctx) {
                mu_ctx->focus = 0;
                mu_ctx->number_edit = 0;
            }
        }
    }
}