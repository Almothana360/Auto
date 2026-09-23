#include "gui_panels.h"
#include "console_cmd.h"
#include "project_io.h"
#include "layer.h"
#include "cad_pid.h"
#include "flange.h"
#include <stdio.h>
#include <string.h>
#include "cad_math.h"

void ApplyMicroUiTheme(mu_Context *ctx, int theme) {
    if (theme == UI_THEME_LIGHT) {
        ctx->style->colors[MU_COLOR_TEXT]        = mu_color(30, 30, 30, 255);
        ctx->style->colors[MU_COLOR_BORDER]      = mu_color(190, 195, 200, 255);
        ctx->style->colors[MU_COLOR_WINDOWBG]    = mu_color(240, 242, 245, 255);
        ctx->style->colors[MU_COLOR_TITLEBG]     = mu_color(225, 228, 232, 255);
        ctx->style->colors[MU_COLOR_TITLETEXT]   = mu_color(20, 20, 20, 255);
        ctx->style->colors[MU_COLOR_PANELBG]     = mu_color(232, 234, 238, 255);
        ctx->style->colors[MU_COLOR_BUTTON]      = mu_color(218, 222, 228, 255);
        ctx->style->colors[MU_COLOR_BUTTONHOVER] = mu_color(205, 210, 218, 255);
        ctx->style->colors[MU_COLOR_BUTTONFOCUS] = mu_color(190, 198, 210, 255);
        ctx->style->colors[MU_COLOR_BASE]        = mu_color(248, 249, 250, 255);
        ctx->style->colors[MU_COLOR_BASEHOVER]   = mu_color(230, 233, 238, 255);
        ctx->style->colors[MU_COLOR_BASEFOCUS]   = mu_color(215, 220, 228, 255);
        ctx->style->colors[MU_COLOR_SCROLLBASE]  = mu_color(220, 224, 230, 255);
        ctx->style->colors[MU_COLOR_SCROLLTHUMB] = mu_color(180, 185, 192, 255);
    } else {
        ctx->style->colors[MU_COLOR_TEXT]        = mu_color(230, 230, 230, 255);
        ctx->style->colors[MU_COLOR_BORDER]      = mu_color(25, 25, 25, 255);
        ctx->style->colors[MU_COLOR_WINDOWBG]    = mu_color(40, 40, 40, 255);
        ctx->style->colors[MU_COLOR_TITLEBG]     = mu_color(30, 30, 30, 255);
        ctx->style->colors[MU_COLOR_TITLETEXT]   = mu_color(240, 240, 240, 255);
        ctx->style->colors[MU_COLOR_PANELBG]     = mu_color(34, 34, 34, 255);
        ctx->style->colors[MU_COLOR_BUTTON]      = mu_color(65, 65, 65, 255);
        ctx->style->colors[MU_COLOR_BUTTONHOVER] = mu_color(85, 85, 85, 255);
        ctx->style->colors[MU_COLOR_BUTTONFOCUS] = mu_color(105, 105, 105, 255);
        ctx->style->colors[MU_COLOR_BASE]        = mu_color(28, 28, 28, 255);
        ctx->style->colors[MU_COLOR_BASEHOVER]   = mu_color(38, 38, 38, 255);
        ctx->style->colors[MU_COLOR_BASEFOCUS]   = mu_color(48, 48, 48, 255);
        ctx->style->colors[MU_COLOR_SCROLLBASE]  = mu_color(40, 40, 40, 255);
        ctx->style->colors[MU_COLOR_SCROLLTHUMB] = mu_color(30, 30, 30, 255);
    }
    ctx->style->colors[MU_COLOR_PANELBG]  = mu_color((int)ctx->style->colors[MU_COLOR_PANELBG].r, (int)ctx->style->colors[MU_COLOR_PANELBG].g, (int)ctx->style->colors[MU_COLOR_PANELBG].b, 255);
    ctx->style->colors[MU_COLOR_WINDOWBG] = mu_color((int)ctx->style->colors[MU_COLOR_WINDOWBG].r, (int)ctx->style->colors[MU_COLOR_WINDOWBG].g, (int)ctx->style->colors[MU_COLOR_WINDOWBG].b, 255);
}

void RenderAllGuiPanels(mu_Context *mu_ctx, AppContext *app) {
    int winW = GetScreenWidth();
    int winH = GetScreenHeight();

    float menuBarHeight = 32.0f * app->uiScale;
    float bottomStripH = 34.0f * app->uiScale;
    float dockY = menuBarHeight;
    float dockH = (float)winH - menuBarHeight - bottomStripH;
    if (dockH < 100.0f) dockH = 100.0f;
    float leftDockW = 320.0f * app->uiScale;
    float rightDockW = 290.0f * app->uiScale;

    ApplyMicroUiTheme(mu_ctx, app->uiConfig.uiTheme);
    mu_ctx->style->font = (mu_Font)(intptr_t)(int)(12 * app->uiScale);
    mu_ctx->style->size = mu_vec2((int)(68 * app->uiScale), (int)(24 * app->uiScale));
    mu_ctx->style->padding = (int)(3 * app->uiScale);
    mu_ctx->style->spacing = (int)(4 * app->uiScale);
    mu_ctx->style->scrollbar_size = (int)(12 * app->uiScale);
    mu_ctx->style->title_height = (int)(22 * app->uiScale);

    mu_begin(mu_ctx);

    // 1. Top Menu Bar (Pinned)
    if (g_CADState.activeTool != TOOL_PAN) {
        mu_Rect barRect = mu_rect(0, 0, winW, (int)menuBarHeight);
        mu_Container *menuWin = mu_get_container(mu_ctx, "##MenuBar");
        if (menuWin) menuWin->rect = barRect;
        if (mu_begin_window_ex(mu_ctx, "##MenuBar", barRect, MU_OPT_NOTITLE | MU_OPT_NORESIZE | MU_OPT_NOSCROLL | MU_OPT_NOFRAME)) {
            int colWidths[] = { (int)(65 * app->uiScale), (int)(65 * app->uiScale), (int)(80 * app->uiScale), (int)(85 * app->uiScale), (int)(95 * app->uiScale), -1 };
            int menuBtnH = (int)(menuBarHeight - 6.0f * app->uiScale);
            if (menuBtnH < 18) menuBtnH = 18;
            mu_layout_row(mu_ctx, 5, colWidths, menuBtnH);
            if (mu_button(mu_ctx, "File")) { mu_open_popup(mu_ctx, "FileMenu"); app->openFileMenu = true; }
            if (mu_button(mu_ctx, "Edit")) { mu_open_popup(mu_ctx, "EditMenu"); app->openEditMenu = true; }
            if (mu_button(mu_ctx, "Window")) { mu_open_popup(mu_ctx, "WindowMenu"); app->openWindowMenu = true; }
            if (mu_button(mu_ctx, "Element")) { mu_open_popup(mu_ctx, "ElementMenu"); app->openElementMenu = true; }
            if (mu_button(mu_ctx, "Functions")) { mu_open_popup(mu_ctx, "FunctionsMenu"); app->openFunctionsMenu = true; }

            if (app->openFileMenu && mu_begin_popup(mu_ctx, "FileMenu")) {
                mu_layout_row(mu_ctx, 1, (int[]){ (int)(150 * app->uiScale) }, (int)(22 * app->uiScale));
                if (mu_button(mu_ctx, "New Project")) { ProcessCommand("new", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openFileMenu = false; }
                if (mu_button(mu_ctx, "Open Project")) { ProcessCommand("open", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openFileMenu = false; }
                if (mu_button(mu_ctx, "Save Project")) { ProcessCommand("save", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openFileMenu = false; }
                if (mu_button(mu_ctx, "Exit")) { ProcessCommand("exit", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openFileMenu = false; }
                mu_end_popup(mu_ctx);
            }

            if (app->openEditMenu && mu_begin_popup(mu_ctx, "EditMenu")) {
                mu_layout_row(mu_ctx, 1, (int[]){ (int)(120 * app->uiScale) }, (int)(22 * app->uiScale));
                if (mu_button(mu_ctx, "Undo")) { ProcessCommand("undo", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openEditMenu = false; }
                if (mu_button(mu_ctx, "Redo")) { ProcessCommand("redo", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openEditMenu = false; }
                mu_end_popup(mu_ctx);
            }

            if (app->openWindowMenu && mu_begin_popup(mu_ctx, "WindowMenu")) {
                mu_layout_row(mu_ctx, 1, (int[]){ (int)(210 * app->uiScale) }, (int)(22 * app->uiScale));
                if (mu_button(mu_ctx, "Reset View")) { ProcessCommand("reset", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openWindowMenu = false; }
                int fsState = IsWindowFullscreen() ? 1 : 0;
                if (mu_checkbox(mu_ctx, "Fullscreen", &fsState)) { ToggleFullscreen(); app->uiConfig.isFullscreen = (fsState != 0); SaveUiConfig(CONFIG_FILENAME, &app->uiConfig); }
                int leftDockState = app->showLeftDock ? 1 : 0;
                if (mu_checkbox(mu_ctx, "Left Panel (Layers/Elems)", &leftDockState)) { app->showLeftDock = (leftDockState != 0); }
                int rightDockState = app->showRightDock ? 1 : 0;
                if (mu_checkbox(mu_ctx, "Right Panel (Inspector)", &rightDockState)) { app->showRightDock = (rightDockState != 0); }
                int hudState = app->showHudPanel ? 1 : 0;
                if (mu_checkbox(mu_ctx, "Bottom Status Strip", &hudState)) { app->showHudPanel = (hudState != 0); }
                if (mu_button(mu_ctx, "Unit Settings")) { app->showUnitWindow = !app->showUnitWindow; app->openWindowMenu = false; }
                if (mu_button(mu_ctx, "UI Scale")) {
                    app->showScaleWindow = !app->showScaleWindow;
                    app->tempUiScale = app->uiScale * 100.0f;
                    app->openWindowMenu = false;
                }
                const char *thmDark = (app->uiConfig.uiTheme == UI_THEME_DARK) ? "[*] Theme: Dark" : "[ ] Theme: Dark";
                if (mu_button(mu_ctx, thmDark)) {
                    app->uiConfig.uiTheme = UI_THEME_DARK;
                    SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
                    snprintf(app->statusMessage, sizeof(app->statusMessage), "Theme: Dark");
                    app->statusMessageTimer = 2.0f;
                    app->openWindowMenu = false;
                }
                const char *thmLight = (app->uiConfig.uiTheme == UI_THEME_LIGHT) ? "[*] Theme: Light" : "[ ] Theme: Light";
                if (mu_button(mu_ctx, thmLight)) {
                    app->uiConfig.uiTheme = UI_THEME_LIGHT;
                    SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
                    snprintf(app->statusMessage, sizeof(app->statusMessage), "Theme: Light");
                    app->statusMessageTimer = 2.0f;
                    app->openWindowMenu = false;
                }
                const char *uiOpt1 = (app->uiConfig.uiBackend == UI_BACKEND_MICROUI) ? "[*] Startup UI: microui" : "[ ] Startup UI: microui";
                if (mu_button(mu_ctx, uiOpt1)) {
                    app->uiConfig.uiBackend = UI_BACKEND_MICROUI;
                    SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
                    snprintf(app->statusMessage, sizeof(app->statusMessage), "Next Startup: microui");
                    app->statusMessageTimer = 2.5f;
                    app->openWindowMenu = false;
                }
                const char *uiOpt2 = (app->uiConfig.uiBackend == UI_BACKEND_RAYGUI) ? "[*] Startup UI: raygui" : "[ ] Startup UI: raygui";
                if (mu_button(mu_ctx, uiOpt2)) {
                    app->uiConfig.uiBackend = UI_BACKEND_RAYGUI;
                    SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
                    snprintf(app->statusMessage, sizeof(app->statusMessage), "Next Startup: raygui");
                    app->statusMessageTimer = 2.5f;
                    app->openWindowMenu = false;
                }
                mu_end_popup(mu_ctx);
            }

            if (app->openElementMenu && mu_begin_popup(mu_ctx, "ElementMenu")) {
                mu_layout_row(mu_ctx, 1, (int[]){ (int)(160 * app->uiScale) }, (int)(22 * app->uiScale));
                if (mu_button(mu_ctx, "Select Tool")) { ProcessCommand("select", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openElementMenu = false; }
                if (mu_button(mu_ctx, "P&ID Circular Palate")) {
                    CAD_PID_OpenPalette(&app->cadPid, (Vector2){ (float)winW * 0.5f, (float)winH * 0.5f });
                    app->openElementMenu = false;
                }
                if (mu_button(mu_ctx, "Add Rectangle")) { ProcessCommand("rect", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openElementMenu = false; }
                if (mu_button(mu_ctx, "Add Circle")) { ProcessCommand("circle", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openElementMenu = false; }
                if (mu_button(mu_ctx, "Add Line")) { ProcessCommand("line", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openElementMenu = false; }
                if (mu_button(mu_ctx, "Add  Polyline")) { ProcessCommand("polyline", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openElementMenu = false; }
                if (mu_button(mu_ctx, "Add Freehand")) { ProcessCommand("freehand", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openElementMenu = false; }
                if (mu_button(mu_ctx, "Add 3-Pt Arc")) { ProcessCommand("arc", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openElementMenu = false; }
                if (mu_button(mu_ctx, "Add Ellipse")) { ProcessCommand("ellipse", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openElementMenu = false; }
                if (mu_button(mu_ctx, "Add Text Note")) { ProcessCommand("text", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openElementMenu = false; }
                if (mu_button(mu_ctx, "Clear Elements")) { ProcessCommand("clear", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openElementMenu = false; }
                mu_end_popup(mu_ctx);
            }

            if (app->openFunctionsMenu && mu_begin_popup(mu_ctx, "FunctionsMenu")) {
                mu_layout_row(mu_ctx, 1, (int[]){ (int)(130 * app->uiScale) }, (int)(22 * app->uiScale));
                if (mu_button(mu_ctx, "Dimension")) { ProcessCommand("dimension", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openFunctionsMenu = false; }
                if (mu_button(mu_ctx, "Pan Mode")) { ProcessCommand("pan", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->openFunctionsMenu = false; }
                mu_end_popup(mu_ctx);
            }

            mu_end_window(mu_ctx);
        }

        // Left Toggle Icon / Button when hidden
        if (!app->showLeftDock && app->uiAnim.leftDockProgress <= 0.05f) {
            mu_Rect lToggleRect = mu_rect(4, (int)dockY + 4, (int)(28 * app->uiScale), (int)(24 * app->uiScale));
            mu_Container *ltWin = mu_get_container(mu_ctx, "##LeftShow");
            if (ltWin) ltWin->rect = lToggleRect;
            if (mu_begin_window_ex(mu_ctx, "##LeftShow", lToggleRect, MU_OPT_NOTITLE | MU_OPT_NORESIZE | MU_OPT_NOSCROLL)) {
                mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(20 * app->uiScale));
                if (mu_button(mu_ctx, ">")) app->showLeftDock = true;
                mu_end_window(mu_ctx);
            }
        }

        // 2. Animated Unified Left Dock
        if (app->uiAnim.leftDockProgress > 0.01f) {
            float curLeftW = leftDockW * app->uiAnim.leftDockProgress;
            float curLeftX = -leftDockW * (1.0f - app->uiAnim.leftDockProgress);
            mu_Rect lDockRect = mu_rect((int)curLeftX, (int)dockY, (int)leftDockW, (int)dockH);
            mu_Container *ldWin = mu_get_container(mu_ctx, "Project Tree");
            if (ldWin) ldWin->rect = lDockRect;
            if (mu_begin_window_ex(mu_ctx, "Project Tree", lDockRect, MU_OPT_NORESIZE | MU_OPT_NOSCROLL)) {
                mu_layout_row(mu_ctx, 2, (int[]){ -36, -1 }, (int)(22 * app->uiScale));
                mu_text(mu_ctx, "Project Workspace");
                if (mu_button(mu_ctx, "<")) { app->showLeftDock = false; }

                float availableH = dockH - (44.0f * app->uiScale);
                int halfHeight = (int)(availableH * 0.5f) - (int)(4.0f * app->uiScale);
                if (halfHeight < 60) halfHeight = 60;

                // Layers Section
                mu_Rect layersBoxRect = mu_rect(0, 0, (int)leftDockW - 8, halfHeight);
                mu_layout_row(mu_ctx, 1, (int[]){ -1 }, halfHeight);
                if (mu_begin_window_ex(mu_ctx, "##LayersScroll", layersBoxRect, MU_OPT_NORESIZE)) {
                    mu_layout_row(mu_ctx, 2, (int[]){ (int)(100 * app->uiScale), -1 }, (int)(20 * app->uiScale));
                    mu_text(mu_ctx, "Layers");
                    if (mu_button(mu_ctx, "+ Add")) {
                        if (app->layerCount < MAX_LAYERS) {
                            char defaultName[LAYER_NAME_LEN];
                            snprintf(defaultName, LAYER_NAME_LEN, "Layer %d", app->layerCount + 1);
                            Layer_Init(&app->layers[app->layerCount], GenerateLayerID(), defaultName, PALETTE[app->layerCount % PALETTE_SIZE], app->layerCount);
                            app->activeLayerIndex = app->layerCount;
                            g_CADState.activeLayerIndex = app->layerCount;
                            g_CADState.activeLayerId = app->layers[app->layerCount].id;
                            app->layerCount++;
                        }
                    }

                    int lCols[] = { (int)(18 * app->uiScale), (int)(18 * app->uiScale), (int)(18 * app->uiScale), (int)(18 * app->uiScale), (int)(18 * app->uiScale), (int)(18 * app->uiScale), (int)(22 * app->uiScale), -1 };
                    for (int i = 0; i < app->layerCount; i++) {
                        mu_push_id(mu_ctx, &app->layers[i], sizeof(Layer*));
                        mu_layout_row(mu_ctx, 8, lCols, (int)(19 * app->uiScale));
                        bool isTargetActive = (i == app->activeLayerIndex);
                        if (mu_button(mu_ctx, isTargetActive ? ">" : " ")) {
                            app->activeLayerIndex = i;
                            g_CADState.activeLayerIndex = i;
                            g_CADState.activeLayerId = app->layers[i].id;
                        }
                        if (mu_button(mu_ctx, app->layers[i].visible ? "V" : "H")) app->layers[i].visible = !app->layers[i].visible;
                        if (mu_button(mu_ctx, app->layers[i].locked ? "L" : "U")) app->layers[i].locked = !app->layers[i].locked;
                        if (mu_button(mu_ctx, "D")) {
                            DeleteLayer(app->elements, &app->elementCount, app->layers, &app->layerCount, i, &app->activeLayerIndex);
                            ClearCommandHistory(app->cmdHistory);
                            app->spatialIndexDirty = true;
                            mu_pop_id(mu_ctx);
                            break;
                        }
                        if (mu_button(mu_ctx, "^")) app->layers[i].renderOrder++;
                        if (mu_button(mu_ctx, "v")) app->layers[i].renderOrder--;
                        if (mu_button(mu_ctx, "C")) {
                            for (int c = 0; c < PALETTE_SIZE; c++) {
                                if (ColorToInt(app->layers[i].defaultColor) == ColorToInt(PALETTE[c])) {
                                    app->layers[i].defaultColor = PALETTE[(c + 1) % PALETTE_SIZE];
                                    break;
                                }
                            }
                        }
                        mu_text(mu_ctx, TextFormat("[%u] %s (%d)", app->layers[i].id, app->layers[i].name, app->layers[i].entityCount));
                        mu_pop_id(mu_ctx);
                    }

                    mu_layout_row(mu_ctx, 2, (int[]){ (int)(70 * app->uiScale), -1 }, (int)(20 * app->uiScale));
                    mu_label(mu_ctx, "Rename:");
                    if (mu_textbox(mu_ctx, app->layerNameEditBuf, sizeof(app->layerNameEditBuf)) & MU_RES_SUBMIT) {
                        if (strlen(app->layerNameEditBuf) > 0) {
                            strncpy(app->layers[app->activeLayerIndex].name, app->layerNameEditBuf, LAYER_NAME_LEN - 1);
                            app->layers[app->activeLayerIndex].name[LAYER_NAME_LEN - 1] = '\0';
                        }
                    }
                    mu_end_window(mu_ctx);
                }

                // Elements Section
                mu_Rect elemsBoxRect = mu_rect(0, 0, (int)leftDockW - 8, halfHeight);
                mu_layout_row(mu_ctx, 1, (int[]){ -1 }, halfHeight);
                if (mu_begin_window_ex(mu_ctx, "##ElementsScroll", elemsBoxRect, MU_OPT_NORESIZE)) {
                    mu_layout_row(mu_ctx, 2, (int[]){ -95, -1 }, (int)(20 * app->uiScale));
                    mu_text(mu_ctx, TextFormat("Elements (%d)", app->elementCount));
                    if (mu_button(mu_ctx, "Deselect")) DeselectAllElements(app->elements, app->elementCount);
                    bool isCtrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
                    for (int i = 0; i < app->elementCount; i++) {
                        mu_push_id(mu_ctx, &app->elements[i], sizeof(GridElement*));
                        mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(19 * app->uiScale));
                        const char *typeStr = (app->elements[i].type == ELEMENT_RECT) ? "Rectangle" : (app->elements[i].type == ELEMENT_CIRCLE ? "Circle" : (app->elements[i].type == ELEMENT_ELLIPSE ? "Ellipse" : (app->elements[i].type == ELEMENT_ARC ? "Arc" : (app->elements[i].type == ELEMENT_TEXT_NOTE ? "Text Note" : (app->elements[i].type == ELEMENT_POLYLINE ? "Polyline" : (app->elements[i].type == ELEMENT_FREEHAND ? "Freehand" : (app->elements[i].type == ELEMENT_LINE ? "Line" : (app->elements[i].type == ELEMENT_SYMBOL ? (strchr(app->elements[i].text, '|') ? "Weld Neck Flange" : "Symbol/Instrument") : "Dimension"))))))));
                        const char *layerName = (app->elements[i].layerIndex >= 0 && app->elements[i].layerIndex < app->layerCount) ? app->layers[app->elements[i].layerIndex].name : "Unknown";
                        char itemLabel[64];
                        snprintf(itemLabel, sizeof(itemLabel), "%s#%d [ID:%u] %s [%s]", app->elements[i].selected ? "* " : "", i + 1, app->elements[i].id, typeStr, layerName);
                        if (mu_button(mu_ctx, itemLabel)) {
                            if (!isCtrl) DeselectAllElements(app->elements, app->elementCount);
                            app->elements[i].selected = !app->elements[i].selected;
                        }
                        mu_pop_id(mu_ctx);
                    }
                    mu_end_window(mu_ctx);
                }
                mu_end_window(mu_ctx);
            }
        }

        // Right Toggle Icon / Button when hidden
        if (!app->showRightDock && app->uiAnim.rightDockProgress <= 0.05f) {
            mu_Rect rToggleRect = mu_rect(winW - (int)(32 * app->uiScale), (int)dockY + 4, (int)(28 * app->uiScale), (int)(24 * app->uiScale));
            mu_Container *rtWin = mu_get_container(mu_ctx, "##RightShow");
            if (rtWin) rtWin->rect = rToggleRect;
            if (mu_begin_window_ex(mu_ctx, "##RightShow", rToggleRect, MU_OPT_NOTITLE | MU_OPT_NORESIZE | MU_OPT_NOSCROLL)) {
                mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(20 * app->uiScale));
                if (mu_button(mu_ctx, "<")) app->showRightDock = true;
                mu_end_window(mu_ctx);
            }
        }

        // 3. Animated Docked Right Panel (Inspector)
        int selectedCount = CountSelectedElements(app->elements, app->elementCount);
        int selectedElementIndex = GetFirstSelectedIndex(app->elements, app->elementCount);
        if (app->uiAnim.rightDockProgress > 0.01f) {
            float curRightX = (float)winW - (rightDockW * app->uiAnim.rightDockProgress);
            mu_Rect insRect = mu_rect((int)curRightX, (int)dockY, (int)rightDockW, (int)dockH);
            mu_Container *inWin = mu_get_container(mu_ctx, "Inspector");
            if (inWin) inWin->rect = insRect;
            if (mu_begin_window_ex(mu_ctx, "Inspector", insRect, MU_OPT_NORESIZE)) {
                mu_layout_row(mu_ctx, 2, (int[]){ -36, -1 }, (int)(22 * app->uiScale));
                mu_text(mu_ctx, "Properties");
                if (mu_button(mu_ctx, ">")) { app->showRightDock = false; }

                if (selectedCount > 0 && selectedElementIndex >= 0) {
                    GridElement *el = &app->elements[selectedElementIndex];
                    bool isFlange = (el->type == ELEMENT_SYMBOL && strchr(el->text, '|') != NULL);

                    mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(18 * app->uiScale));
                    const char *title = isFlange ? "Type: Weld Neck Flange (ASME B16.5)" :
                                        ((el->type == ELEMENT_RECT) ? "Type: Rectangle" :
                                        (el->type == ELEMENT_CIRCLE ? "Type: Circle" :
                                        (el->type == ELEMENT_ELLIPSE ? "Type: Ellipse" :
                                        (el->type == ELEMENT_ARC ? "Type: Arc" :
                                        (el->type == ELEMENT_TEXT_NOTE ? "Type: Text Note" :
                                        (el->type == ELEMENT_POLYLINE ? "Type: Polyline" :
                                        (el->type == ELEMENT_FREEHAND ? "Type: Freehand" :
                                        (el->type == ELEMENT_LINE ? "Type: Line" :
                                        (el->type == ELEMENT_SYMBOL ? "Type: Instrument / Symbol" : "Type: Dimension")))))))));
                    mu_text(mu_ctx, title);
                    mu_text(mu_ctx, TextFormat("Entity ID: %u", el->id));
                    mu_text(mu_ctx, TextFormat("Pos: (%.1f, %.1f)", el->pos.x, el->pos.y));
                    mu_text(mu_ctx, TextFormat("BBox: [%.0f,%.0f] to [%.0f,%.0f]", el->bbox.min.x, el->bbox.min.y, el->bbox.max.x, el->bbox.max.y));
                    mu_text(mu_ctx, TextFormat("Assigned: [%u] %s", app->layers[el->layerIndex].id, app->layers[el->layerIndex].name));

                    if (mu_button(mu_ctx, "Move to Active Layer")) {
                        Command cmd = { 0 };
                        cmd.type = CMD_LAYER_CHANGE;
                        cmd.data.layerChange.index = selectedElementIndex;
                        cmd.data.layerChange.oldLayer = el->layerIndex;
                        cmd.data.layerChange.newLayer = app->activeLayerIndex;
                        ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                    }

                    if (mu_button(mu_ctx, "Send Back (1 Step)")) {
                        int targetLayer = el->layerIndex;
                        int prevSameLayerIdx = -1;
                        for (int i = selectedElementIndex - 1; i >= 0; i--) {
                            if (app->elements[i].layerIndex == targetLayer) { prevSameLayerIdx = i; break; }
                        }
                        if (prevSameLayerIdx != -1) {
                            Command cmd = { 0 };
                            cmd.type = CMD_ORDER_CHANGE;
                            cmd.data.orderChange.oldIndex = selectedElementIndex;
                            cmd.data.orderChange.newIndex = prevSameLayerIdx;
                            ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                        }
                    }

                    if (mu_button(mu_ctx, "Send to Backmost")) {
                        int targetLayer = el->layerIndex;
                        int firstSameLayerIdx = -1;
                        for (int i = 0; i < selectedElementIndex; i++) {
                            if (app->elements[i].layerIndex == targetLayer) { firstSameLayerIdx = i; break; }
                        }
                        if (firstSameLayerIdx != -1) {
                            Command cmd = { 0 };
                            cmd.type = CMD_ORDER_CHANGE;
                            cmd.data.orderChange.oldIndex = selectedElementIndex;
                            cmd.data.orderChange.newIndex = firstSameLayerIdx;
                            ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                        }
                    }

                    // Flange Rating Dropdown Properties
                    if (isFlange) {
                        const FlangeDatabase *db = Flange_GetDatabase();
                        char curClass[16] = "150#";
                        char curNps[16] = "2\"";
                        char *sep = strchr(el->text, '|');
                        if (sep) {
                            size_t cLen = (size_t)(sep - el->text);
                            if (cLen < sizeof(curClass)) {
                                strncpy(curClass, el->text, cLen);
                                curClass[cLen] = '\0';
                            }
                            strncpy(curNps, sep + 1, sizeof(curNps) - 1);
                            curNps[sizeof(curNps) - 1] = '\0';
                        }

                        mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(20 * app->uiScale));
                        mu_text(mu_ctx, "--- ASME B16.5 Ratings ---");

                        // Class Tree/Dropdown
                        if (mu_begin_treenode(mu_ctx, TextFormat("Class: %s", curClass))) {
                            for (int c = 0; c < db->classCount; c++) {
                                mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(18 * app->uiScale));
                                bool isCur = (strcmp(db->classes[c].className, curClass) == 0);
                                if (mu_button(mu_ctx, TextFormat("%s %s", isCur ? ">" : " ", db->classes[c].className))) {
                                    FlangeSpec spec;
                                    Flange_InitDefaultSpec(&spec, FLANGE_WELD_NECK);
                                    Flange_SetSpecBySize(&spec, db->classes[c].className, curNps);
                                    el->width = spec.fw;
                                    el->height = spec.fh;
                                    el->radius = spec.ft;
                                    snprintf(el->text, TEXT_NOTE_LEN, "%s|%s", spec.className, spec.nps);
                                    GetElementAABB(el);
                                    app->spatialIndexDirty = true;
                                }
                            }
                            mu_end_treenode(mu_ctx);
                        }

                        // NPS Tree/Dropdown
                        if (mu_begin_treenode(mu_ctx, TextFormat("NPS: %s", curNps))) {
                            int activeClassIdx = 0;
                            for (int c = 0; c < db->classCount; c++) {
                                if (strcmp(db->classes[c].className, curClass) == 0) {
                                    activeClassIdx = c;
                                    break;
                                }
                            }
                            for (int s = 0; s < db->classes[activeClassIdx].recordCount; s++) {
                                mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(18 * app->uiScale));
                                bool isCur = (strcmp(db->classes[activeClassIdx].records[s].nps, curNps) == 0);
                                if (mu_button(mu_ctx, TextFormat("%s %s", isCur ? ">" : " ", db->classes[activeClassIdx].records[s].nps))) {
                                    FlangeSpec spec;
                                    Flange_InitDefaultSpec(&spec, FLANGE_WELD_NECK);
                                    Flange_SetSpecBySize(&spec, curClass, db->classes[activeClassIdx].records[s].nps);
                                    el->width = spec.fw;
                                    el->height = spec.fh;
                                    el->radius = spec.ft;
                                    snprintf(el->text, TEXT_NOTE_LEN, "%s|%s", spec.className, spec.nps);
                                    GetElementAABB(el);
                                    app->spatialIndexDirty = true;
                                }
                            }
                            mu_end_treenode(mu_ctx);
                        }

                        mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(18 * app->uiScale));
                        mu_text(mu_ctx, TextFormat("Thickness (fw): %.1f mm", el->width));
                        mu_text(mu_ctx, TextFormat("Height (fh): %.1f mm", el->height));
                        mu_text(mu_ctx, TextFormat("Tail Length (ft): %.1f mm", el->radius));
                    }

                    mu_text(mu_ctx, TextFormat("Rotation: %.1f deg", el->rotation));
                    if (mu_slider(mu_ctx, &el->rotation, 0.0f, 360.0f)) app->spatialIndexDirty = true;

                    // SCALE SLIDER: Locked and uneditable for standard Flanges
                    if (isFlange) {
                        mu_text(mu_ctx, "Scale: 1.00 (Locked by ASME Standard)");
                    } else {
                        mu_text(mu_ctx, TextFormat("Scale X: %.2f | Y: %.2f", el->scale.x, el->scale.y));
                        if (mu_slider(mu_ctx, &el->scale.x, 0.1f, 5.0f)) app->spatialIndexDirty = true;
                        if (mu_slider(mu_ctx, &el->scale.y, 0.1f, 5.0f)) app->spatialIndexDirty = true;
                    }

                    // ROTATION PRESETS
                    mu_layout_row(mu_ctx, 5, (int[]){ (int)(42 * app->uiScale), (int)(42 * app->uiScale), (int)(42 * app->uiScale), (int)(42 * app->uiScale), (int)(42 * app->uiScale) }, (int)(20 * app->uiScale));
                    float presets[] = { 0.0f, 45.0f, 90.0f, 180.0f, 270.0f };
                    const char *presetLabels[] = { "0", "45", "90", "180", "270" };
                    for (int p = 0; p < 5; p++) {
                        if (mu_button(mu_ctx, presetLabels[p])) {
                            Command cmd = { 0 };
                            cmd.type = CMD_TRANSFORM;
                            cmd.data.transform.index = selectedElementIndex;
                            cmd.data.transform.before = *el;
                            el->rotation = presets[p];
                            cmd.data.transform.after = *el;
                            ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                        }
                    }

                    mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(20 * app->uiScale));
                    // LINE THICKNESS SLIDER: Available for all element types
                    if (el->lineThickness <= 0.0f) el->lineThickness = 3.0f;
                    mu_text(mu_ctx, TextFormat("Line Thickness: %.1f", el->lineThickness));
                    if (mu_slider(mu_ctx, &el->lineThickness, 1.0f, 12.0f)) {
                        GetElementAABB(el);
                        app->spatialIndexDirty = true;
                    }

                    if (el->type == ELEMENT_RECT) {
                        mu_text(mu_ctx, TextFormat("Width: %.1f", el->width));
                        if (mu_slider(mu_ctx, &el->width, MIN_ELEMENT_SIZE, 300.0f)) app->spatialIndexDirty = true;
                        mu_text(mu_ctx, TextFormat("Height: %.1f", el->height));
                        if (mu_slider(mu_ctx, &el->height, MIN_ELEMENT_SIZE, 300.0f)) app->spatialIndexDirty = true;
                    } else if (el->type == ELEMENT_CIRCLE) {
                        mu_text(mu_ctx, TextFormat("Radius: %.1f", el->radius));
                        if (mu_slider(mu_ctx, &el->radius, MIN_ELEMENT_SIZE, 150.0f)) app->spatialIndexDirty = true;
                    } else if (el->type == ELEMENT_ELLIPSE) {
                        mu_text(mu_ctx, TextFormat("Radius X: %.1f", el->radiusX));
                        if (mu_slider(mu_ctx, &el->radiusX, MIN_ELEMENT_SIZE, 200.0f)) app->spatialIndexDirty = true;
                        mu_text(mu_ctx, TextFormat("Radius Y: %.1f", el->radiusY));
                        if (mu_slider(mu_ctx, &el->radiusY, MIN_ELEMENT_SIZE, 200.0f)) app->spatialIndexDirty = true;
                    } else if (el->type == ELEMENT_TEXT_NOTE) {
                        mu_text(mu_ctx, "Note Content:");
                        mu_textbox(mu_ctx, el->text, TEXT_NOTE_LEN);
                        int arrowInt = el->showArrow ? 1 : 0;
                        if (mu_checkbox(mu_ctx, "Pointer Arrow", &arrowInt)) { el->showArrow = (arrowInt != 0); app->spatialIndexDirty = true; }
                        mu_text(mu_ctx, TextFormat("Font Size: %d", el->textSize));
                        float ts = (float)el->textSize;
                        if (mu_slider(mu_ctx, &ts, 8.0f, 48.0f)) el->textSize = (int)ts;
                    } else if (el->type == ELEMENT_DIMENSION) {
                        mu_text(mu_ctx, TextFormat("Tick Thick: %.1f", el->tickThickness));
                        mu_slider(mu_ctx, &el->tickThickness, 1.0f, 10.0f);
                        mu_text(mu_ctx, TextFormat("Text Size: %d", el->textSize));
                        float ts = (float)el->textSize;
                        if (mu_slider(mu_ctx, &ts, 8.0f, 48.0f)) el->textSize = (int)ts;
                    }

                    // COLOR PALETTE
                    mu_text(mu_ctx, el->useCustomColor ? "Color: Custom" : "Color: Layer");
                    mu_layout_row(mu_ctx, 4, (int[]){ (int)(55 * app->uiScale), (int)(55 * app->uiScale), (int)(55 * app->uiScale), (int)(55 * app->uiScale) }, (int)(20 * app->uiScale));
                    const char *colorNames[] = { "Sky", "Lime", "Orange", "Purple", "Red", "Gold", "Gray", "Black" };
                    for (int p = 0; p < PALETTE_SIZE; p++) {
                        if (mu_button(mu_ctx, colorNames[p])) {
                            Command cmd = { 0 };
                            cmd.type = CMD_TRANSFORM;
                            cmd.data.transform.index = selectedElementIndex;
                            cmd.data.transform.before = *el;
                            el->color = PALETTE[p];
                            el->color.a = 255;
                            el->useCustomColor = true;
                            cmd.data.transform.after = *el;
                            ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                        }
                    }

                    mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(20 * app->uiScale));
                    if (el->useCustomColor && mu_button(mu_ctx, "Reset to Layer Color")) {
                        Command cmd = { 0 };
                        cmd.type = CMD_TRANSFORM;
                        cmd.data.transform.index = selectedElementIndex;
                        cmd.data.transform.before = *el;
                        el->useCustomColor = false;
                        cmd.data.transform.after = *el;
                        ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                    }
                } else {
                    mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(24 * app->uiScale));
                    mu_text(mu_ctx, "No Element Selected");
                }
                mu_end_window(mu_ctx);
            }
        }

        // 4. Animated Status Strip
        if (app->uiAnim.hudProgress > 0.01f) {
            float curStripY = (float)winH - (bottomStripH * app->uiAnim.hudProgress);
            mu_Rect stripRect = mu_rect(0, (int)curStripY, winW, (int)bottomStripH);
            mu_Container *bsWin = mu_get_container(mu_ctx, "##BottomStrip");
            if (bsWin) bsWin->rect = stripRect;
            if (mu_begin_window_ex(mu_ctx, "##BottomStrip", stripRect, MU_OPT_NOTITLE | MU_OPT_NORESIZE | MU_OPT_NOSCROLL | MU_OPT_NOFRAME)) {
                int cols[] = {
                    (int)(170 * app->uiScale),
                    (int)(160 * app->uiScale),
                    (int)(140 * app->uiScale),
                    (int)(105 * app->uiScale),
                    (int)(105 * app->uiScale),
                    (int)(160 * app->uiScale),
                    (int)(150 * app->uiScale),
                    -1
                };
                mu_layout_row(mu_ctx, 8, cols, (int)(bottomStripH - 6.0f * app->uiScale));
                mu_text(mu_ctx, TextFormat("Layer: [%u] %s", g_CADState.activeLayerId, app->layers[g_CADState.activeLayerIndex].name));
                mu_text(mu_ctx, TextFormat("W: (%.1f, %.1f)", g_CADState.mouseWorld.x, g_CADState.mouseWorld.y));
                mu_text(mu_ctx, TextFormat("S: (%.0f, %.0f)", g_CADState.mouseScreen.x, g_CADState.mouseScreen.y));

                int snapGridInt = app->snapToGrid ? 1 : 0;
                if (mu_checkbox(mu_ctx, "Snap Grid", &snapGridInt)) {
                    app->snapToGrid = (snapGridInt != 0);
                    app->uiConfig.snapToGrid = app->snapToGrid;
                    SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
                }

                int snapElemInt = app->snapEnabled ? 1 : 0;
                if (mu_checkbox(mu_ctx, "Snap Elem", &snapElemInt)) {
                    app->snapEnabled = (snapElemInt != 0);
                    app->uiConfig.snapEnabled = app->snapEnabled;
                    SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
                }

                const char *toolName = "Select";
                if (app->cadPid.isPlacingInstrument) toolName = "P&ID Insert";
                else if (g_CADState.activeTool == TOOL_ADD_RECT) toolName = "Add Rect";
                else if (g_CADState.activeTool == TOOL_ADD_CIRCLE) toolName = "Add Circle";
                else if (g_CADState.activeTool == TOOL_ADD_ELLIPSE) toolName = "Add Ellipse";
                else if (g_CADState.activeTool == TOOL_ADD_POLYLINE) toolName = "Polyline";
                else if (g_CADState.activeTool == TOOL_ADD_FREEHAND) toolName = "Freehand";
                else if (g_CADState.activeTool == TOOL_ADD_ARC) toolName = "Arc";
                else if (g_CADState.activeTool == TOOL_ADD_TEXT_NOTE) toolName = "Text Note";
                else if (g_CADState.activeTool == TOOL_ADD_LINE) toolName = (app->dimStep == 0) ? "Line (Start)" : "Line (End)";
                else if (g_CADState.activeTool == TOOL_DIMENSION) toolName = "Dimension";
                else if (g_CADState.activeTool == TOOL_PAN) toolName = "Pan View";

                mu_text(mu_ctx, TextFormat("Tool: %s", toolName));
                mu_text(mu_ctx, TextFormat("Elements: %d (Sel: %d)", app->elementCount, CountSelectedElements(app->elements, app->elementCount)));
                mu_end_window(mu_ctx);
            }
        }

        // 5. Unit Modal Window
        if (app->showUnitWindow) {
            mu_Rect uRect = mu_rect(winW / 2 - (int)(150 * app->uiScale), winH / 2 - (int)(100 * app->uiScale), (int)(300 * app->uiScale), (int)(200 * app->uiScale));
            mu_Container *uWin = mu_get_container(mu_ctx, "Measurement Unit");
            if (uWin) uWin->rect = uRect;
            if (mu_begin_window_ex(mu_ctx, "Measurement Unit", uRect, MU_OPT_NORESIZE)) {
                mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(24 * app->uiScale));
                if (mu_button(mu_ctx, app->currentUnit == UNIT_MM ? "[X] mm" : "[ ] mm")) { app->currentUnit = UNIT_MM; app->uiConfig.currentUnit = app->currentUnit; SaveUiConfig(CONFIG_FILENAME, &app->uiConfig); }
                if (mu_button(mu_ctx, app->currentUnit == UNIT_CM ? "[X] cm" : "[ ] cm")) { app->currentUnit = UNIT_CM; app->uiConfig.currentUnit = app->currentUnit; SaveUiConfig(CONFIG_FILENAME, &app->uiConfig); }
                if (mu_button(mu_ctx, app->currentUnit == UNIT_M  ? "[X] m"  : "[ ] m"))  { app->currentUnit = UNIT_M;  app->uiConfig.currentUnit = app->currentUnit; SaveUiConfig(CONFIG_FILENAME, &app->uiConfig); }
                if (mu_button(mu_ctx, app->currentUnit == UNIT_IN ? "[X] in" : "[ ] in")) { app->currentUnit = UNIT_IN; app->uiConfig.currentUnit = app->currentUnit; SaveUiConfig(CONFIG_FILENAME, &app->uiConfig); }
                if (mu_button(mu_ctx, app->currentUnit == UNIT_FT ? "[X] ft" : "[ ] ft")) { app->currentUnit = UNIT_FT; app->uiConfig.currentUnit = app->currentUnit; SaveUiConfig(CONFIG_FILENAME, &app->uiConfig); }
                if (mu_button(mu_ctx, "Close")) {
                    app->showUnitWindow = false;
                    mu_ctx->focus = 0;
                    mu_ctx->number_edit = 0;
                }
                mu_end_window(mu_ctx);
            }
        }

        // 6. UI Scale Modal Window
        if (app->showScaleWindow) {
            mu_Rect sRect = mu_rect(winW / 2 - (int)(150 * app->uiScale), winH / 2 - (int)(100 * app->uiScale), (int)(300 * app->uiScale), (int)(200 * app->uiScale));
            mu_Container *sWin = mu_get_container(mu_ctx, "Adjust UI Scale (%)");
            if (sWin) sWin->rect = sRect;
            if (mu_begin_window_ex(mu_ctx, "Adjust UI Scale (%)", sRect, MU_OPT_NORESIZE)) {
                mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(24 * app->uiScale));
                mu_text(mu_ctx, TextFormat("Scale: %.0f %%", app->tempUiScale));
                mu_slider(mu_ctx, &app->tempUiScale, 50.0f, 400.0f);
                if (mu_button(mu_ctx, "Apply")) {
                    app->uiScale = app->tempUiScale / 100.0f;
                    app->uiConfig.uiScale = app->uiScale;
                    SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
                    mu_ctx->focus = 0;
                    mu_ctx->number_edit = 0;
                }
                if (mu_button(mu_ctx, "Close")) {
                    app->showScaleWindow = false;
                    mu_ctx->focus = 0;
                    mu_ctx->number_edit = 0;
                }
                mu_end_window(mu_ctx);
            }
        }

        // 7. Animated Context Menu
        if (app->uiAnim.contextMenuProgress > 0.01f) {
            float ctxWidth = 180.0f * app->uiScale;
            float ctxHeight = 210.0f * app->uiScale;
            float animatedHeight = ctxHeight * app->uiAnim.contextMenuProgress;
            mu_Rect cmRect = mu_rect((int)app->contextMenuPos.x, (int)app->contextMenuPos.y, (int)ctxWidth, (int)animatedHeight);
            mu_Container *cmWin = mu_get_container(mu_ctx, "##ContextMenu");
            if (cmWin) cmWin->rect = cmRect;
            if (mu_begin_window_ex(mu_ctx, "##ContextMenu", cmRect, MU_OPT_NOTITLE | MU_OPT_NORESIZE)) {
                mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(22 * app->uiScale));
                if (app->contextOnElement && app->contextElementIndex >= 0 && app->contextElementIndex < app->elementCount) {
                    if (mu_button(mu_ctx, "Delete Element")) {
                        Command cmd = { 0 };
                        cmd.type = CMD_DELETE;
                        cmd.data.del.index = app->contextElementIndex;
                        cmd.data.del.element = app->elements[app->contextElementIndex];
                        ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                        app->showContextMenu = false;
                        snprintf(app->statusMessage, 64, "Element Deleted");
                        app->statusMessageTimer = 1.5f;
                    }
                    if (mu_button(mu_ctx, "Send Back (1 Step)")) {
                        int targetLayer = app->elements[app->contextElementIndex].layerIndex;
                        int prevSameLayerIdx = -1;
                        for (int i = app->contextElementIndex - 1; i >= 0; i--) {
                            if (app->elements[i].layerIndex == targetLayer) { prevSameLayerIdx = i; break; }
                        }
                        if (prevSameLayerIdx != -1) {
                            Command cmd = { 0 };
                            cmd.type = CMD_ORDER_CHANGE;
                            cmd.data.orderChange.oldIndex = app->contextElementIndex;
                            cmd.data.orderChange.newIndex = prevSameLayerIdx;
                            ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                        }
                        app->showContextMenu = false;
                    }
                    if (mu_button(mu_ctx, "Send to Backmost")) {
                        int targetLayer = app->elements[app->contextElementIndex].layerIndex;
                        int firstSameLayerIdx = -1;
                        for (int i = 0; i < app->contextElementIndex; i++) {
                            if (app->elements[i].layerIndex == targetLayer) { firstSameLayerIdx = i; break; }
                        }
                        if (firstSameLayerIdx != -1) {
                            Command cmd = { 0 };
                            cmd.type = CMD_ORDER_CHANGE;
                            cmd.data.orderChange.oldIndex = app->contextElementIndex;
                            cmd.data.orderChange.newIndex = firstSameLayerIdx;
                            ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                        }
                        app->showContextMenu = false;
                    }
                    if (mu_button(mu_ctx, "Bring to Active Layer")) {
                        Command cmd = { 0 };
                        cmd.type = CMD_LAYER_CHANGE;
                        cmd.data.layerChange.index = app->contextElementIndex;
                        cmd.data.layerChange.oldLayer = app->elements[app->contextElementIndex].layerIndex;
                        cmd.data.layerChange.newLayer = app->activeLayerIndex;
                        ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                        app->showContextMenu = false;
                    }
                    if (mu_button(mu_ctx, "Duplicate Element")) {
                        if (app->elementCount < MAX_ELEMENTS) {
                            GridElement dup = app->elements[app->contextElementIndex];
                            dup.id = GenerateEntityID();
                            dup.pos.x += 30.0f; dup.pos.y += 30.0f;
                            DeselectAllElements(app->elements, app->elementCount);
                            dup.selected = true;
                            GetElementAABB(&dup);
                            Command cmd = { 0 };
                            cmd.type = CMD_CREATE;
                            cmd.data.create.index = app->elementCount;
                            cmd.data.create.element = dup;
                            ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                            app->contextElementIndex = app->elementCount - 1;
                            app->showContextMenu = false;
                            snprintf(app->statusMessage, 64, "Element Duplicated");
                            app->statusMessageTimer = 1.5f;
                        }
                    }
                    if (mu_button(mu_ctx, "Deselect Elements")) {
                        DeselectAllElements(app->elements, app->elementCount);
                        app->showContextMenu = false;
                    }
                } else {
                    if (mu_button(mu_ctx, "Select Mode")) { g_CADState.activeTool = TOOL_SELECT; app->showContextMenu = false; }
                    if (mu_button(mu_ctx, "P&ID Circular Palate")) {
                        CAD_PID_OpenPalette(&app->cadPid, app->contextMenuPos);
                        app->showContextMenu = false;
                    }
                    if (mu_button(mu_ctx, "+ Add Rectangle")) { g_CADState.activeTool = TOOL_ADD_RECT; app->showContextMenu = false; }
                    if (mu_button(mu_ctx, "+ Add Circle")) { g_CADState.activeTool = TOOL_ADD_CIRCLE; app->showContextMenu = false; }
                    if (mu_button(mu_ctx, "Deselect Elements")) { DeselectAllElements(app->elements, app->elementCount); app->showContextMenu = false; }
                    if (mu_button(mu_ctx, "Reset View")) { ProcessCommand("reset", app->elements, &app->elementCount, app->layers, &app->layerCount, &app->activeLayerIndex, &g_CADState.activeTool, &app->camera, &app->showHudPanel, &app->showInspector, &app->showLeftDock, &app->showLeftDock, &app->uiScale, app->statusMessage, &app->statusMessageTimer, &app->dimStep, app->cmdHistory, &app->spatialIndexDirty, app->currentUnit); app->showContextMenu = false; }
                }
                mu_end_window(mu_ctx);
            }
        }
    } else {
        mu_Rect pRect = mu_rect(winW - (int)(160 * app->uiScale), winH - (int)(50 * app->uiScale), (int)(140 * app->uiScale), (int)(35 * app->uiScale));
        mu_Container *pWin = mu_get_container(mu_ctx, "##PanQuit");
        if (pWin) pWin->rect = pRect;
        if (mu_begin_window_ex(mu_ctx, "##PanQuit", pRect, MU_OPT_NOTITLE | MU_OPT_NORESIZE | MU_OPT_NOSCROLL)) {
            mu_layout_row(mu_ctx, 1, (int[]){ -1 }, (int)(25 * app->uiScale));
            if (mu_button(mu_ctx, "Exit Pan Mode")) g_CADState.activeTool = TOOL_SELECT;
            mu_end_window(mu_ctx);
        }
    }

    mu_end(mu_ctx);
}