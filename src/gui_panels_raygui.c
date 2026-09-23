#include "gui_panels_raygui.h"
#include "console_cmd.h"
#include "project_io.h"
#include "layer.h"
#include "cad_math.h"
#include "cad_pid.h"
#include "flange.h"
#include "raygui.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

void ApplyRayguiTheme(int theme) {
    if (theme == UI_THEME_LIGHT) {
        GuiSetStyle(DEFAULT,       BACKGROUND_COLOR,    ColorToInt((Color){ 240, 242, 245, 255 }));
        GuiSetStyle(DEFAULT,       BASE_COLOR_NORMAL,   ColorToInt((Color){ 225, 228, 233, 255 }));
        GuiSetStyle(DEFAULT,       BASE_COLOR_FOCUSED,  ColorToInt((Color){ 210, 215, 224, 255 }));
        GuiSetStyle(DEFAULT,       BASE_COLOR_PRESSED,  ColorToInt((Color){ 195, 202, 215, 255 }));
        GuiSetStyle(DEFAULT,       BORDER_COLOR_NORMAL, ColorToInt((Color){ 185, 190, 198, 255 }));
        GuiSetStyle(DEFAULT,       BORDER_COLOR_FOCUSED,ColorToInt((Color){ 120, 140, 170, 255 }));
        GuiSetStyle(DEFAULT,       BORDER_COLOR_PRESSED,ColorToInt((Color){ 90, 110, 140, 255 }));
        GuiSetStyle(DEFAULT,       TEXT_COLOR_NORMAL,   ColorToInt((Color){ 30, 30, 30, 255 }));
        GuiSetStyle(DEFAULT,       TEXT_COLOR_FOCUSED,  ColorToInt((Color){ 10, 10, 10, 255 }));
        GuiSetStyle(DEFAULT,       TEXT_COLOR_PRESSED,  ColorToInt((Color){ 0, 0, 0, 255 }));
        GuiSetStyle(LABEL,         TEXT_COLOR_NORMAL,   ColorToInt((Color){ 30, 30, 30, 255 }));
        GuiSetStyle(BUTTON,        BASE_COLOR_NORMAL,   ColorToInt((Color){ 220, 224, 230, 255 }));
        GuiSetStyle(BUTTON,        TEXT_COLOR_NORMAL,   ColorToInt((Color){ 25, 25, 25, 255 }));
        GuiSetStyle(BUTTON,        BORDER_COLOR_NORMAL, ColorToInt((Color){ 180, 185, 195, 255 }));
        GuiSetStyle(TEXTBOX,       BASE_COLOR_NORMAL,   ColorToInt((Color){ 252, 252, 252, 255 }));
        GuiSetStyle(TEXTBOX,       TEXT_COLOR_NORMAL,   ColorToInt((Color){ 20, 20, 20, 255 }));
        GuiSetStyle(TEXTBOX,       BORDER_COLOR_NORMAL, ColorToInt((Color){ 180, 185, 195, 255 }));
        GuiSetStyle(SLIDER,        BASE_COLOR_NORMAL,   ColorToInt((Color){ 205, 210, 218, 255 }));
        GuiSetStyle(SLIDER,        BORDER_COLOR_NORMAL, ColorToInt((Color){ 175, 180, 190, 255 }));
        GuiSetStyle(CHECKBOX,      BASE_COLOR_NORMAL,   ColorToInt((Color){ 252, 252, 252, 255 }));
        GuiSetStyle(CHECKBOX,      TEXT_COLOR_NORMAL,   ColorToInt((Color){ 30, 30, 30, 255 }));
        GuiSetStyle(CHECKBOX,      BORDER_COLOR_NORMAL, ColorToInt((Color){ 180, 185, 195, 255 }));
    } else {
        GuiSetStyle(DEFAULT,       BACKGROUND_COLOR,    ColorToInt((Color){ 38, 38, 38, 255 }));
        GuiSetStyle(DEFAULT,       BASE_COLOR_NORMAL,   ColorToInt((Color){ 48, 48, 48, 255 }));
        GuiSetStyle(DEFAULT,       BASE_COLOR_FOCUSED,  ColorToInt((Color){ 62, 62, 62, 255 }));
        GuiSetStyle(DEFAULT,       BASE_COLOR_PRESSED,  ColorToInt((Color){ 75, 75, 75, 255 }));
        GuiSetStyle(DEFAULT,       BORDER_COLOR_NORMAL, ColorToInt((Color){ 24, 24, 24, 255 }));
        GuiSetStyle(DEFAULT,       BORDER_COLOR_FOCUSED,ColorToInt((Color){ 80, 80, 80, 255 }));
        GuiSetStyle(DEFAULT,       BORDER_COLOR_PRESSED,ColorToInt((Color){ 100, 100, 100, 255 }));
        GuiSetStyle(DEFAULT,       TEXT_COLOR_NORMAL,   ColorToInt((Color){ 230, 230, 230, 255 }));
        GuiSetStyle(DEFAULT,       TEXT_COLOR_FOCUSED,  ColorToInt((Color){ 255, 255, 255, 255 }));
        GuiSetStyle(DEFAULT,       TEXT_COLOR_PRESSED,  ColorToInt((Color){ 255, 255, 255, 255 }));
        GuiSetStyle(LABEL,         TEXT_COLOR_NORMAL,   ColorToInt((Color){ 220, 220, 220, 255 }));
        GuiSetStyle(BUTTON,        BASE_COLOR_NORMAL,   ColorToInt((Color){ 52, 52, 52, 255 }));
        GuiSetStyle(BUTTON,        TEXT_COLOR_NORMAL,   ColorToInt((Color){ 230, 230, 230, 255 }));
        GuiSetStyle(BUTTON,        BORDER_COLOR_NORMAL, ColorToInt((Color){ 26, 26, 26, 255 }));
        GuiSetStyle(TEXTBOX,       BASE_COLOR_NORMAL,   ColorToInt((Color){ 28, 28, 28, 255 }));
        GuiSetStyle(TEXTBOX,       TEXT_COLOR_NORMAL,   ColorToInt((Color){ 230, 230, 230, 255 }));
        GuiSetStyle(TEXTBOX,       BORDER_COLOR_NORMAL, ColorToInt((Color){ 20, 20, 20, 255 }));
        GuiSetStyle(SLIDER,        BASE_COLOR_NORMAL,   ColorToInt((Color){ 44, 44, 44, 255 }));
        GuiSetStyle(SLIDER,        BORDER_COLOR_NORMAL, ColorToInt((Color){ 24, 24, 24, 255 }));
        GuiSetStyle(CHECKBOX,      BASE_COLOR_NORMAL,   ColorToInt((Color){ 28, 28, 28, 255 }));
        GuiSetStyle(CHECKBOX,      TEXT_COLOR_NORMAL,   ColorToInt((Color){ 230, 230, 230, 255 }));
        GuiSetStyle(CHECKBOX,      BORDER_COLOR_NORMAL, ColorToInt((Color){ 24, 24, 24, 255 }));
    }
}

static Color GetThemePanelBg(const AppContext *app) {
    return (Color){
        (unsigned char)app->uiAnim.panelBgR,
        (unsigned char)app->uiAnim.panelBgG,
        (unsigned char)app->uiAnim.panelBgB,
        255
    };
}

static Color GetThemeSubpanelBg(const AppContext *app) {
    return (Color){
        (unsigned char)app->uiAnim.subpanelBgR,
        (unsigned char)app->uiAnim.subpanelBgG,
        (unsigned char)app->uiAnim.subpanelBgB,
        255
    };
}

static Color GetThemeBorder(const AppContext *app) {
    return (Color){
        (unsigned char)app->uiAnim.borderR,
        (unsigned char)app->uiAnim.borderG,
        (unsigned char)app->uiAnim.borderB,
        255
    };
}

static Color GetThemeTextColor(const AppContext *app) {
    return (Color){
        (unsigned char)app->uiAnim.textR,
        (unsigned char)app->uiAnim.textG,
        (unsigned char)app->uiAnim.textB,
        255
    };
}

static void DispatchCommand(AppContext *app, const char *cmd) {
    ProcessCommand(cmd, app->elements, &app->elementCount, app->layers, &app->layerCount,
                   &app->activeLayerIndex, &g_CADState.activeTool, &app->camera,
                   &app->showHudPanel, &app->showInspector, &app->showLeftDock,
                   &app->showLeftDock, &app->uiScale, app->statusMessage,
                   &app->statusMessageTimer, &app->dimStep, app->cmdHistory,
                   &app->spatialIndexDirty, app->currentUnit);
}

static void CloseAllPopups(AppContext *app) {
    app->openFileMenu = false;
    app->openEditMenu = false;
    app->openWindowMenu = false;
    app->openElementMenu = false;
    app->openFunctionsMenu = false;
}

// Retained UI states for RayGUI flange dropdown pickers
static bool s_flangeClassOpen = false;
static bool s_flangeNpsOpen   = false;
static float s_flangeNpsScrollOffset = 0.0f;

/* Custom 100% opaque, scrollable combobox picker that completely prevents background transparency */
static bool DrawOpaqueScrollablePicker(Rectangle bounds, const char *label, const char *items[], int itemCount, int *selectedIndex, bool *isOpen, float *scrollOffset, Color bgCol, Color borderCol, Font font, float uiScale) {
    (void)font;
    bool changed = false;
    const char *currentLabel = (*selectedIndex >= 0 && *selectedIndex < itemCount) ? items[*selectedIndex] : label;

    // Header toggle button - Draw opaque backing first
    Color solidBg = { bgCol.r, bgCol.g, bgCol.b, 255 };
    DrawRectangleRec(bounds, solidBg);
    DrawRectangleLinesEx(bounds, 1.0f, borderCol);

    if (GuiButton(bounds, TextFormat("%s: %s %s", label, currentLabel, *isOpen ? "[^]" : "[v]"))) {
        *isOpen = !(*isOpen);
    }

    // Dropdown list container rendered with full 255 opacity
    if (*isOpen) {
        float itemH = 22.0f * uiScale;
        int maxVisible = 6;
        float viewH = fminf((float)itemCount, (float)maxVisible) * itemH;
        Rectangle popRect = { bounds.x, bounds.y + bounds.height + 2.0f, bounds.width, viewH };

        // 100% Solid Opaque Backing Rectangles
        DrawRectangleRec(popRect, solidBg);
        DrawRectangleLinesEx(popRect, 1.5f, borderCol);

        // Handle mouse wheel scrolling
        if (CheckCollisionPointRec(GetMousePosition(), popRect)) {
            float wheel = GetMouseWheelMove();
            if (wheel != 0.0f) {
                *scrollOffset -= wheel * itemH;
            }
        }

        float maxScroll = fmaxf(0.0f, (itemCount * itemH) - viewH);
        if (*scrollOffset < 0.0f) *scrollOffset = 0.0f;
        if (*scrollOffset > maxScroll) *scrollOffset = maxScroll;

        BeginScissorMode((int)popRect.x, (int)popRect.y, (int)popRect.width, (int)popRect.height);
        for (int i = 0; i < itemCount; i++) {
            float itemY = popRect.y + (i * itemH) - *scrollOffset;
            if (itemY + itemH < popRect.y || itemY > popRect.y + popRect.height) continue;
            Rectangle itemBtnRect = { popRect.x + 2.0f, itemY, popRect.width - (maxScroll > 0 ? 14.0f : 4.0f), itemH - 1.0f };

            // Draw solid opaque base for every item button to guarantee zero bleed-through
            DrawRectangleRec(itemBtnRect, solidBg);

            bool isCur = (i == *selectedIndex);
            if (isCur) {
                DrawRectangleRec(itemBtnRect, Fade(GOLD, 0.35f));
            }
            if (GuiButton(itemBtnRect, TextFormat("%s %s", isCur ? ">" : " ", items[i]))) {
                *selectedIndex = i;
                *isOpen = false;
                changed = true;
            }
        }

        // Draw vertical scrollbar indicator if scrollable
        if (maxScroll > 0.0f) {
            float scrollThumbH = (viewH / (itemCount * itemH)) * viewH;
            float scrollThumbY = popRect.y + (*scrollOffset / maxScroll) * (viewH - scrollThumbH);
            Rectangle scrollTrack = { popRect.x + popRect.width - 10.0f, popRect.y, 8.0f, viewH };
            Rectangle scrollThumb = { popRect.x + popRect.width - 10.0f, scrollThumbY, 8.0f, scrollThumbH };
            DrawRectangleRec(scrollTrack, Fade(BLACK, 0.4f));
            DrawRectangleRec(scrollThumb, borderCol);
        }
        EndScissorMode();
    }
    return changed;
}

bool CheckGuiHover_Raygui(AppContext *app) {
    int winW = GetScreenWidth();
    int winH = GetScreenHeight();
    Vector2 mousePos = GetMousePosition();

    if (app->cadPid.isPaletteOpen) return true;
    if (s_flangeClassOpen || s_flangeNpsOpen) return true;

    float menuBarHeight = 32.0f * app->uiScale;
    float bottomStripH = 34.0f * app->uiScale;
    float leftDockW = 320.0f * app->uiScale;
    float rightDockW = 290.0f * app->uiScale;
    float dockH = (float)winH - menuBarHeight - bottomStripH;

    float cmdW = 460.0f * app->uiScale;
    float cmdH = 26.0f * app->uiScale;
    Rectangle commandBoxRect = { ((float)winW - cmdW) / 2.0f, (float)winH - bottomStripH - cmdH - (4.0f * app->uiScale), cmdW, cmdH };
    if (CheckCollisionPointRec(mousePos, commandBoxRect)) return true;

    if (CheckCollisionPointRec(mousePos, (Rectangle){ 0, 0, (float)winW, menuBarHeight })) return true;

    if (app->uiAnim.hudProgress > 0.01f) {
        float hudY = (float)winH - (bottomStripH * app->uiAnim.hudProgress);
        if (CheckCollisionPointRec(mousePos, (Rectangle){ 0, hudY, (float)winW, bottomStripH })) return true;
    }

    if (app->uiAnim.leftDockProgress > 0.01f) {
        float curLeftX = -leftDockW * (1.0f - app->uiAnim.leftDockProgress);
        if (CheckCollisionPointRec(mousePos, (Rectangle){ curLeftX, menuBarHeight, leftDockW, dockH })) return true;
    } else {
        if (CheckCollisionPointRec(mousePos, (Rectangle){ 4, menuBarHeight + 4, 28.0f * app->uiScale, 24.0f * app->uiScale })) return true;
    }

    if (app->uiAnim.rightDockProgress > 0.01f) {
        float curRightX = (float)winW - (rightDockW * app->uiAnim.rightDockProgress);
        if (CheckCollisionPointRec(mousePos, (Rectangle){ curRightX, menuBarHeight, rightDockW, dockH })) return true;
    } else {
        if (CheckCollisionPointRec(mousePos, (Rectangle){ winW - 32.0f * app->uiScale, menuBarHeight + 4, 28.0f * app->uiScale, 24.0f * app->uiScale })) return true;
    }

    if (app->showUnitWindow) {
        Rectangle modalUnitRect = { (winW - 300.0f * app->uiScale) / 2.0f, (winH - 240.0f * app->uiScale) / 2.0f, 300.0f * app->uiScale, 240.0f * app->uiScale };
        if (CheckCollisionPointRec(mousePos, modalUnitRect)) return true;
    }

    if (app->showScaleWindow) {
        Rectangle modalScaleRect = { (winW - 300.0f * app->uiScale) / 2.0f, (winH - 200.0f * app->uiScale) / 2.0f, 300.0f * app->uiScale, 200.0f * app->uiScale };
        if (CheckCollisionPointRec(mousePos, modalScaleRect)) return true;
    }

    if (app->uiAnim.contextMenuProgress > 0.01f) {
        float ctxWidth = 180.0f * app->uiScale;
        float ctxHeight = (210.0f * app->uiScale) * app->uiAnim.contextMenuProgress;
        Rectangle ctxMenuRect = { app->contextMenuPos.x, app->contextMenuPos.y, ctxWidth, ctxHeight };
        if (CheckCollisionPointRec(mousePos, ctxMenuRect)) return true;
    }

    float btnH = 22.0f * app->uiScale;
    if (app->openFileMenu && CheckCollisionPointRec(mousePos, (Rectangle){ 4.0f * app->uiScale, menuBarHeight, 150.0f * app->uiScale, 4 * (btnH + 2) + 6 })) return true;
    if (app->openEditMenu && CheckCollisionPointRec(mousePos, (Rectangle){ (4.0f + 68.0f + 4.0f) * app->uiScale, menuBarHeight, 130.0f * app->uiScale, 2 * (btnH + 2) + 6 })) return true;
    if (app->openWindowMenu && CheckCollisionPointRec(mousePos, (Rectangle){ (4.0f + (68.0f + 4.0f) * 2) * app->uiScale, menuBarHeight, 225.0f * app->uiScale, 11 * (btnH + 2) + 8 })) return true;
    if (app->openElementMenu && CheckCollisionPointRec(mousePos, (Rectangle){ (4.0f + (68.0f + 4.0f) * 2 + 83.0f * app->uiScale), menuBarHeight, 170.0f * app->uiScale, 12 * (btnH + 2) + 6 })) return true;
    if (app->openFunctionsMenu && CheckCollisionPointRec(mousePos, (Rectangle){ (4.0f + (68.0f + 4.0f) * 2 + (83.0f + 83.0f) * app->uiScale), menuBarHeight, 140.0f * app->uiScale, 2 * (btnH + 2) + 6 })) return true;

    return false;
}

void RenderAllGuiPanels_Raygui(AppContext *app) {
    int winW = GetScreenWidth();
    int winH = GetScreenHeight();
    float menuBarHeight = 32.0f * app->uiScale;
    float bottomStripH = 34.0f * app->uiScale;
    float dockY = menuBarHeight;
    float dockH = (float)winH - menuBarHeight - bottomStripH;
    if (dockH < 100.0f) dockH = 100.0f;
    float leftDockW = 320.0f * app->uiScale;
    float rightDockW = 290.0f * app->uiScale;
    float btnH = 22.0f * app->uiScale;
    float spacing = 4.0f * app->uiScale;

    Font bodyFont = ResourceManager_GetFont(&app->resManager, FONT_SLOT_BODY);
    Color pBg = GetThemePanelBg(app);
    Color subBg = GetThemeSubpanelBg(app);
    Color pBorder = GetThemeBorder(app);
    Color pText = GetThemeTextColor(app);

    bool clickedTopMenuButton = false;

    if (g_CADState.activeTool == TOOL_PAN) {
        Rectangle pRect = { winW - 160.0f * app->uiScale, winH - 50.0f * app->uiScale, 140.0f * app->uiScale, 35.0f * app->uiScale };
        DrawRectangleRec(pRect, pBg);
        DrawRectangleLinesEx(pRect, 1.0f, pBorder);
        if (GuiButton((Rectangle){ pRect.x + 4, pRect.y + 4, pRect.width - 8, pRect.height - 8 }, "Exit Pan Mode")) {
            g_CADState.activeTool = TOOL_SELECT;
        }
        return;
    }

    // 1. Top Menu Bar
    Rectangle menuBarRect = { 0, 0, (float)winW, menuBarHeight };
    DrawRectangleRec(menuBarRect, pBg);
    DrawLine(0, (int)menuBarHeight, winW, (int)menuBarHeight, pBorder);

    float mBtnX = 4.0f * app->uiScale;
    float mBtnY = 3.0f * app->uiScale;
    float mBtnW = 68.0f * app->uiScale;
    float mBtnH = menuBarHeight - 6.0f * app->uiScale;

    if (GuiButton((Rectangle){ mBtnX, mBtnY, mBtnW, mBtnH }, "File")) {
        clickedTopMenuButton = true;
        bool prev = app->openFileMenu;
        CloseAllPopups(app);
        app->openFileMenu = !prev;
    }
    mBtnX += mBtnW + spacing;
    if (GuiButton((Rectangle){ mBtnX, mBtnY, mBtnW, mBtnH }, "Edit")) {
        clickedTopMenuButton = true;
        bool prev = app->openEditMenu;
        CloseAllPopups(app);
        app->openEditMenu = !prev;
    }
    mBtnX += mBtnW + spacing;
    if (GuiButton((Rectangle){ mBtnX, mBtnY, mBtnW + 15.0f * app->uiScale, mBtnH }, "Window")) {
        clickedTopMenuButton = true;
        bool prev = app->openWindowMenu;
        CloseAllPopups(app);
        app->openWindowMenu = !prev;
    }
    mBtnX += mBtnW + 15.0f * app->uiScale + spacing;
    if (GuiButton((Rectangle){ mBtnX, mBtnY, mBtnW + 15.0f * app->uiScale, mBtnH }, "Element")) {
        clickedTopMenuButton = true;
        bool prev = app->openElementMenu;
        CloseAllPopups(app);
        app->openElementMenu = !prev;
    }
    mBtnX += mBtnW + 15.0f * app->uiScale + spacing;
    if (GuiButton((Rectangle){ mBtnX, mBtnY, mBtnW + 25.0f * app->uiScale, mBtnH }, "Functions")) {
        clickedTopMenuButton = true;
        bool prev = app->openFunctionsMenu;
        CloseAllPopups(app);
        app->openFunctionsMenu = !prev;
    }

    // 2. Animated Left Dock
    if (!app->showLeftDock && app->uiAnim.leftDockProgress <= 0.05f) {
        Rectangle lToggleRect = { 4, dockY + 4, 28.0f * app->uiScale, 24.0f * app->uiScale };
        DrawRectangleRec(lToggleRect, pBg);
        DrawRectangleLinesEx(lToggleRect, 1.0f, pBorder);
        if (GuiButton(lToggleRect, ">")) app->showLeftDock = true;
    } else if (app->uiAnim.leftDockProgress > 0.01f) {
        float curLeftX = -leftDockW * (1.0f - app->uiAnim.leftDockProgress);
        Rectangle lDockRect = { curLeftX, dockY, leftDockW, dockH };
        DrawRectangleRec(lDockRect, pBg);
        DrawRectangleLinesEx(lDockRect, 1.0f, pBorder);

        Rectangle lHeader = { curLeftX, dockY, leftDockW, 26.0f * app->uiScale };
        DrawRectangleRec(lHeader, subBg);
        DrawLine((int)curLeftX, (int)(dockY + 26.0f * app->uiScale), (int)(curLeftX + leftDockW), (int)(dockY + 26.0f * app->uiScale), pBorder);
        DrawTextEx(bodyFont, "Project Workspace", (Vector2){ curLeftX + 10.0f * app->uiScale, dockY + 6.0f * app->uiScale }, 12.0f * app->uiScale, 1.0f, pText);
        if (GuiButton((Rectangle){ curLeftX + leftDockW - 30.0f * app->uiScale, dockY + 3.0f * app->uiScale, 24.0f * app->uiScale, 20.0f * app->uiScale }, "<")) {
            app->showLeftDock = false;
        }

        float availableH = dockH - (46.0f * app->uiScale);
        float halfH = (availableH * 0.5f) - (4.0f * app->uiScale);
        if (halfH < 80.0f) halfH = 80.0f;

        // Layers Section
        Rectangle layersBox = { curLeftX + 6.0f * app->uiScale, dockY + 30.0f * app->uiScale, leftDockW - 12.0f * app->uiScale, halfH };
        DrawRectangleRec(layersBox, subBg);
        DrawRectangleLinesEx(layersBox, 1.0f, pBorder);
        GuiGroupBox(layersBox, "Layers");

        Rectangle addLayerBtn = { layersBox.x + layersBox.width - 64.0f * app->uiScale, layersBox.y + 4.0f * app->uiScale, 58.0f * app->uiScale, 20.0f * app->uiScale };
        if (GuiButton(addLayerBtn, "+ Add")) {
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

        float rowY = layersBox.y + 26.0f * app->uiScale;
        float itemH = 20.0f * app->uiScale;
        int maxVisibleLayers = (int)((layersBox.height - 65.0f * app->uiScale) / (itemH + 2.0f));
        if (maxVisibleLayers < 1) maxVisibleLayers = 1;

        for (int i = 0; i < app->layerCount && i < maxVisibleLayers; i++) {
            float lx = layersBox.x + 4.0f * app->uiScale;
            float bWidth = 18.0f * app->uiScale;
            bool isTargetActive = (i == app->activeLayerIndex);
            if (GuiButton((Rectangle){ lx, rowY, bWidth, itemH }, isTargetActive ? ">" : " ")) {
                app->activeLayerIndex = i;
                g_CADState.activeLayerIndex = i;
                g_CADState.activeLayerId = app->layers[i].id;
                strncpy(app->layerNameEditBuf, app->layers[i].name, sizeof(app->layerNameEditBuf) - 1);
                app->layerNameEditBuf[sizeof(app->layerNameEditBuf) - 1] = '\0';
            }
            lx += bWidth + 2;
            if (GuiButton((Rectangle){ lx, rowY, bWidth, itemH }, app->layers[i].visible ? "V" : "H")) {
                app->layers[i].visible = !app->layers[i].visible;
            }
            lx += bWidth + 2;
            if (GuiButton((Rectangle){ lx, rowY, bWidth, itemH }, app->layers[i].locked ? "L" : "U")) {
                app->layers[i].locked = !app->layers[i].locked;
            }
            lx += bWidth + 2;
            if (GuiButton((Rectangle){ lx, rowY, bWidth, itemH }, "D")) {
                DeleteLayer(app->elements, &app->elementCount, app->layers, &app->layerCount, i, &app->activeLayerIndex);
                ClearCommandHistory(app->cmdHistory);
                app->spatialIndexDirty = true;
                break;
            }
            lx += bWidth + 2;
            if (GuiButton((Rectangle){ lx, rowY, bWidth, itemH }, "^")) app->layers[i].renderOrder++;
            lx += bWidth + 2;
            if (GuiButton((Rectangle){ lx, rowY, bWidth, itemH }, "v")) app->layers[i].renderOrder--;
            lx += bWidth + 2;
            if (GuiButton((Rectangle){ lx, rowY, bWidth, itemH }, "C")) {
                for (int c = 0; c < PALETTE_SIZE; c++) {
                    if (ColorToInt(app->layers[i].defaultColor) == ColorToInt(PALETTE[c])) {
                        app->layers[i].defaultColor = PALETTE[(c + 1) % PALETTE_SIZE];
                        break;
                    }
                }
            }
            lx += bWidth + 4;
            float labelW = layersBox.x + layersBox.width - lx - 4.0f;
            DrawRectangle((int)lx, (int)rowY, (int)labelW, (int)itemH, Fade(app->layers[i].defaultColor, 0.35f));
            DrawTextEx(bodyFont, TextFormat("[%u] %s (%d)", app->layers[i].id, app->layers[i].name, app->layers[i].entityCount),
                       (Vector2){ lx + 2.0f, rowY + 3.0f }, 11.0f * app->uiScale, 1.0f, pText);
            rowY += itemH + 2.0f;
        }

        // Rename row
        float renameY = layersBox.y + layersBox.height - 26.0f * app->uiScale;
        DrawTextEx(bodyFont, "Rename:", (Vector2){ layersBox.x + 6.0f, renameY + 4.0f }, 11.0f * app->uiScale, 1.0f, pText);
        Rectangle renameBox = { layersBox.x + 60.0f * app->uiScale, renameY, layersBox.width - 66.0f * app->uiScale, 20.0f * app->uiScale };
        if (GuiTextBox(renameBox, app->layerNameEditBuf, sizeof(app->layerNameEditBuf), app->layerRenameEditMode)) {
            app->layerRenameEditMode = !app->layerRenameEditMode;
            if (!app->layerRenameEditMode && strlen(app->layerNameEditBuf) > 0 && app->activeLayerIndex < app->layerCount) {
                strncpy(app->layers[app->activeLayerIndex].name, app->layerNameEditBuf, LAYER_NAME_LEN - 1);
                app->layers[app->activeLayerIndex].name[LAYER_NAME_LEN - 1] = '\0';
            }
        }

        // Elements Section
        Rectangle elemsBox = { curLeftX + 6.0f * app->uiScale, layersBox.y + layersBox.height + 6.0f * app->uiScale, leftDockW - 12.0f * app->uiScale, halfH };
        DrawRectangleRec(elemsBox, subBg);
        DrawRectangleLinesEx(elemsBox, 1.0f, pBorder);
        GuiGroupBox(elemsBox, TextFormat("Elements (%d)", app->elementCount));

        Rectangle deselectBtn = { elemsBox.x + elemsBox.width - 74.0f * app->uiScale, elemsBox.y + 4.0f * app->uiScale, 68.0f * app->uiScale, 20.0f * app->uiScale };
        if (GuiButton(deselectBtn, "Deselect")) {
            DeselectAllElements(app->elements, app->elementCount);
        }

        float elRowY = elemsBox.y + 26.0f * app->uiScale;
        int maxVisibleElems = (int)((elemsBox.height - 30.0f * app->uiScale) / (itemH + 2.0f));
        if (maxVisibleElems < 1) maxVisibleElems = 1;
        bool isCtrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

        for (int i = 0; i < app->elementCount && i < maxVisibleElems; i++) {
            const char *typeStr = (app->elements[i].type == ELEMENT_RECT) ? "Rect" :
                                  (app->elements[i].type == ELEMENT_CIRCLE ? "Circle" :
                                  (app->elements[i].type == ELEMENT_ELLIPSE ? "Ellipse" :
                                  (app->elements[i].type == ELEMENT_ARC ? "Arc" :
                                  (app->elements[i].type == ELEMENT_TEXT_NOTE ? "Text Note" :
                                  (app->elements[i].type == ELEMENT_POLYLINE ? "Polyline" :
                                  (app->elements[i].type == ELEMENT_FREEHAND ? "Freehand" :
                                  (app->elements[i].type == ELEMENT_LINE ? "Line" :
                                  (app->elements[i].type == ELEMENT_SYMBOL ? (strchr(app->elements[i].text, '|') ? "WN Flange" : "Symbol") : "Dim"))))))));
            const char *layerName = (app->elements[i].layerIndex >= 0 && app->elements[i].layerIndex < app->layerCount) ? app->layers[app->elements[i].layerIndex].name : "Unknown";
            char itemLabel[64];
            snprintf(itemLabel, sizeof(itemLabel), "%s#%d [ID:%u] %s [%s]", app->elements[i].selected ? "* " : "", i + 1, app->elements[i].id, typeStr, layerName);

            Rectangle itemRect = { elemsBox.x + 4.0f * app->uiScale, elRowY, elemsBox.width - 8.0f * app->uiScale, itemH };
            if (GuiButton(itemRect, itemLabel)) {
                if (!isCtrl) DeselectAllElements(app->elements, app->elementCount);
                app->elements[i].selected = !app->elements[i].selected;
            }
            elRowY += itemH + 2.0f;
        }
    }

    // 3. Animated Right Dock (Inspector)
    int selectedCount = CountSelectedElements(app->elements, app->elementCount);
    int selectedElementIndex = GetFirstSelectedIndex(app->elements, app->elementCount);

    if (!app->showRightDock && app->uiAnim.rightDockProgress <= 0.05f) {
        Rectangle rToggleRect = { winW - 32.0f * app->uiScale, dockY + 4, 28.0f * app->uiScale, 24.0f * app->uiScale };
        DrawRectangleRec(rToggleRect, pBg);
        DrawRectangleLinesEx(rToggleRect, 1.0f, pBorder);
        if (GuiButton(rToggleRect, "<")) app->showRightDock = true;
    } else if (app->uiAnim.rightDockProgress > 0.01f) {
        float curRightX = (float)winW - (rightDockW * app->uiAnim.rightDockProgress);
        Rectangle rDockRect = { curRightX, dockY, rightDockW, dockH };
        DrawRectangleRec(rDockRect, pBg);
        DrawRectangleLinesEx(rDockRect, 1.0f, pBorder);

        Rectangle rHeader = { curRightX, dockY, rightDockW, 26.0f * app->uiScale };
        DrawRectangleRec(rHeader, subBg);
        DrawLine((int)curRightX, (int)(dockY + 26.0f * app->uiScale), (int)(curRightX + rightDockW), (int)(dockY + 26.0f * app->uiScale), pBorder);
        DrawTextEx(bodyFont, "Inspector Properties", (Vector2){ curRightX + 10.0f * app->uiScale, dockY + 6.0f * app->uiScale }, 12.0f * app->uiScale, 1.0f, pText);

        Rectangle closeInspectorBtn = { curRightX + rightDockW - 30.0f * app->uiScale, dockY + 3.0f * app->uiScale, 24.0f * app->uiScale, 20.0f * app->uiScale };
        if (GuiButton(closeInspectorBtn, ">")) {
            app->showRightDock = false;
        }

        float inspY = dockY + 32.0f * app->uiScale;
        float inspX = curRightX + 10.0f * app->uiScale;
        float inspW = rightDockW - 20.0f * app->uiScale;

        if (selectedCount > 0 && selectedElementIndex >= 0) {
            GridElement *el = &app->elements[selectedElementIndex];
            bool isFlange = (el->type == ELEMENT_SYMBOL && strchr(el->text, '|') != NULL);

            const char *title = isFlange ? "Type: Weld Neck Flange (ASME B16.5)" :
                                ((el->type == ELEMENT_RECT) ? "Type: Rectangle" :
                                ((el->type == ELEMENT_CIRCLE) ? "Type: Circle" :
                                ((el->type == ELEMENT_ELLIPSE) ? "Type: Ellipse" :
                                ((el->type == ELEMENT_ARC) ? "Type: Arc" :
                                ((el->type == ELEMENT_TEXT_NOTE) ? "Type: Text Note" :
                                ((el->type == ELEMENT_POLYLINE) ? "Type: Polyline" :
                                ((el->type == ELEMENT_FREEHAND) ? "Type: Freehand" :
                                ((el->type == ELEMENT_LINE) ? "Type: Line" :
                                ((el->type == ELEMENT_SYMBOL) ? "Type: Symbol / Inst" : "Type: Dimension")))))))));

            GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, title); inspY += btnH;
            GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Entity ID: %u", el->id)); inspY += btnH;
            GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Pos: (%.1f, %.1f)", el->pos.x, el->pos.y)); inspY += btnH;
            GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("BBox: [%.0f,%.0f] to [%.0f,%.0f]", el->bbox.min.x, el->bbox.min.y, el->bbox.max.x, el->bbox.max.y)); inspY += btnH;
            GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Assigned: [%u] %s", app->layers[el->layerIndex].id, app->layers[el->layerIndex].name)); inspY += btnH;

            if (GuiButton((Rectangle){ inspX, inspY, inspW, btnH }, "Move to Active Layer")) {
                Command cmd = { 0 };
                cmd.type = CMD_LAYER_CHANGE;
                cmd.data.layerChange.index = selectedElementIndex;
                cmd.data.layerChange.oldLayer = el->layerIndex;
                cmd.data.layerChange.newLayer = app->activeLayerIndex;
                ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
            }
            inspY += btnH + spacing;

            if (GuiButton((Rectangle){ inspX, inspY, (inspW - spacing) / 2.0f, btnH }, "Send Back (1 Step)")) {
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
            if (GuiButton((Rectangle){ inspX + (inspW - spacing) / 2.0f + spacing, inspY, (inspW - spacing) / 2.0f, btnH }, "Send Backmost")) {
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
            inspY += btnH + spacing;

            // Flange-Specific Class & NPS Dropdowns (Non-transparent, solid background)
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

                int activeClassIdx = 0;
                const char *classList[MAX_FLANGE_CLASSES];
                for (int c = 0; c < db->classCount; c++) {
                    classList[c] = db->classes[c].className;
                    if (strcmp(db->classes[c].className, curClass) == 0) activeClassIdx = c;
                }

                // 1. Rating Class Dropdown Picker
                float pickerH = 24.0f * app->uiScale;
                Rectangle classPickerRect = { inspX, inspY, inspW, pickerH };
                int selectedClassIdx = activeClassIdx;
                float dummyScroll = 0.0f;
                if (DrawOpaqueScrollablePicker(classPickerRect, "Class", classList, db->classCount, &selectedClassIdx, &s_flangeClassOpen, &dummyScroll, pBg, pBorder, bodyFont, app->uiScale)) {
                    FlangeSpec spec;
                    Flange_InitDefaultSpec(&spec, FLANGE_WELD_NECK);
                    Flange_SetSpecBySize(&spec, db->classes[selectedClassIdx].className, curNps);
                    el->width = spec.fw;
                    el->height = spec.fh;
                    el->radius = spec.ft;
                    snprintf(el->text, TEXT_NOTE_LEN, "%s|%s", spec.className, spec.nps);
                    GetElementAABB(el);
                    app->spatialIndexDirty = true;
                }
                inspY += pickerH + spacing;

                // 2. NPS (OD) Scrollable Dropdown Picker
                const char *npsList[MAX_FLANGE_SIZES];
                int activeNpsIdx = 0;
                for (int s = 0; s < db->classes[activeClassIdx].recordCount; s++) {
                    npsList[s] = db->classes[activeClassIdx].records[s].nps;
                    if (strcmp(db->classes[activeClassIdx].records[s].nps, curNps) == 0) activeNpsIdx = s;
                }
                Rectangle npsPickerRect = { inspX, inspY, inspW, pickerH };
                int selectedNpsIdx = activeNpsIdx;
                if (DrawOpaqueScrollablePicker(npsPickerRect, "NPS", npsList, db->classes[activeClassIdx].recordCount, &selectedNpsIdx, &s_flangeNpsOpen, &s_flangeNpsScrollOffset, pBg, pBorder, bodyFont, app->uiScale)) {
                    FlangeSpec spec;
                    Flange_InitDefaultSpec(&spec, FLANGE_WELD_NECK);
                    Flange_SetSpecBySize(&spec, curClass, db->classes[activeClassIdx].records[selectedNpsIdx].nps);
                    el->width = spec.fw;
                    el->height = spec.fh;
                    el->radius = spec.ft;
                    snprintf(el->text, TEXT_NOTE_LEN, "%s|%s", spec.className, spec.nps);
                    GetElementAABB(el);
                    app->spatialIndexDirty = true;
                }
                inspY += pickerH + spacing;

                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Thickness (fw): %.1f mm", el->width)); inspY += btnH;
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Height (fh): %.1f mm", el->height)); inspY += btnH;
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Hub Tail (ft): %.1f mm", el->radius)); inspY += btnH + spacing;
            }

            GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Rotation: %.1f deg", el->rotation)); inspY += btnH;
            if (GuiSliderBar((Rectangle){ inspX, inspY, inspW, btnH }, "", "", &el->rotation, 0.0f, 360.0f)) {
                app->spatialIndexDirty = true;
            }
            inspY += btnH + spacing;

            float prW = (inspW - 4 * spacing) / 5.0f;
            float presets[] = { 0.0f, 45.0f, 90.0f, 180.0f, 270.0f };
            const char *presetLabels[] = { "0", "45", "90", "180", "270" };
            for (int p = 0; p < 5; p++) {
                if (GuiButton((Rectangle){ inspX + p * (prW + spacing), inspY, prW, btnH }, presetLabels[p])) {
                    Command cmd = { 0 };
                    cmd.type = CMD_TRANSFORM;
                    cmd.data.transform.index = selectedElementIndex;
                    cmd.data.transform.before = *el;
                    el->rotation = presets[p];
                    cmd.data.transform.after = *el;
                    ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                }
            }
            inspY += btnH + spacing;

            // SCALE SLIDER: Locked and uneditable for standard Flanges
            if (isFlange) {
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, "Scale: 1.00 (Locked by ASME Standard)"); inspY += btnH + spacing;
            } else {
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Scale X: %.2f | Y: %.2f", el->scale.x, el->scale.y)); inspY += btnH;
                if (GuiSliderBar((Rectangle){ inspX, inspY, (inspW - spacing) / 2.0f, btnH }, "X", "", &el->scale.x, 0.1f, 5.0f)) app->spatialIndexDirty = true;
                if (GuiSliderBar((Rectangle){ inspX + (inspW - spacing) / 2.0f + spacing, inspY, (inspW - spacing) / 2.0f, btnH }, "Y", "", &el->scale.y, 0.1f, 5.0f)) app->spatialIndexDirty = true;
                inspY += btnH + spacing;
            }

            // LINE THICKNESS SLIDER
            if (el->lineThickness <= 0.0f) el->lineThickness = 3.0f;
            GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Line Thickness: %.1f", el->lineThickness)); inspY += btnH;
            if (GuiSliderBar((Rectangle){ inspX, inspY, inspW, btnH }, "", "", &el->lineThickness, 1.0f, 12.0f)) {
                GetElementAABB(el);
                app->spatialIndexDirty = true;
            }
            inspY += btnH + spacing;

            if (el->type == ELEMENT_RECT) {
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Width: %.1f", el->width)); inspY += btnH;
                if (GuiSliderBar((Rectangle){ inspX, inspY, inspW, btnH }, "", "", &el->width, MIN_ELEMENT_SIZE, 300.0f)) app->spatialIndexDirty = true;
                inspY += btnH + spacing;
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Height: %.1f", el->height)); inspY += btnH;
                if (GuiSliderBar((Rectangle){ inspX, inspY, inspW, btnH }, "", "", &el->height, MIN_ELEMENT_SIZE, 300.0f)) app->spatialIndexDirty = true;
                inspY += btnH + spacing;
            } else if (el->type == ELEMENT_CIRCLE) {
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Radius: %.1f", el->radius)); inspY += btnH;
                if (GuiSliderBar((Rectangle){ inspX, inspY, inspW, btnH }, "", "", &el->radius, MIN_ELEMENT_SIZE, 150.0f)) app->spatialIndexDirty = true;
                inspY += btnH + spacing;
            } else if (el->type == ELEMENT_ELLIPSE) {
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Radius X: %.1f", el->radiusX)); inspY += btnH;
                if (GuiSliderBar((Rectangle){ inspX, inspY, inspW, btnH }, "", "", &el->radiusX, MIN_ELEMENT_SIZE, 200.0f)) app->spatialIndexDirty = true;
                inspY += btnH + spacing;
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Radius Y: %.1f", el->radiusY)); inspY += btnH;
                if (GuiSliderBar((Rectangle){ inspX, inspY, inspW, btnH }, "", "", &el->radiusY, MIN_ELEMENT_SIZE, 200.0f)) app->spatialIndexDirty = true;
                inspY += btnH + spacing;
            } else if (el->type == ELEMENT_TEXT_NOTE) {
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, "Note Content:"); inspY += btnH;
                Rectangle noteBox = { inspX, inspY, inspW, btnH };
                if (GuiTextBox(noteBox, el->text, TEXT_NOTE_LEN, app->noteTextEditMode)) {
                    app->noteTextEditMode = !app->noteTextEditMode;
                    app->spatialIndexDirty = true;
                }
                inspY += btnH + spacing;
                if (GuiCheckBox((Rectangle){ inspX, inspY, 18.0f * app->uiScale, 18.0f * app->uiScale }, "Pointer Arrow", &el->showArrow)) {
                    app->spatialIndexDirty = true;
                }
                inspY += btnH + spacing;
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Font Size: %d", el->textSize)); inspY += btnH;
                float ts = (float)el->textSize;
                if (GuiSliderBar((Rectangle){ inspX, inspY, inspW, btnH }, "", "", &ts, 8.0f, 48.0f)) el->textSize = (int)ts;
                inspY += btnH + spacing;
            } else if (el->type == ELEMENT_DIMENSION) {
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Tick Thick: %.1f", el->tickThickness)); inspY += btnH;
                GuiSliderBar((Rectangle){ inspX, inspY, inspW, btnH }, "", "", &el->tickThickness, 1.0f, 10.0f);
                inspY += btnH + spacing;
                GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, TextFormat("Text Size: %d", el->textSize)); inspY += btnH;
                float ts = (float)el->textSize;
                if (GuiSliderBar((Rectangle){ inspX, inspY, inspW, btnH }, "", "", &ts, 8.0f, 48.0f)) el->textSize = (int)ts;
                inspY += btnH + spacing;
            }

            // COLOR PALETTE
            GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, el->useCustomColor ? "Color: Custom" : "Color: Layer"); inspY += btnH;
            float cW = (inspW - 3 * spacing) / 4.0f;
            const char *colorNames[] = { "Sky", "Lime", "Orange", "Purp", "Red", "Gold", "Gray", "Black" };
            for (int p = 0; p < PALETTE_SIZE; p++) {
                float cx = inspX + (p % 4) * (cW + spacing);
                float cy = inspY + (p / 4) * (btnH + spacing);
                if (GuiButton((Rectangle){ cx, cy, cW, btnH }, colorNames[p])) {
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
            inspY += (btnH + spacing) * 2;
            if (el->useCustomColor && GuiButton((Rectangle){ inspX, inspY, inspW, btnH }, "Reset to Layer Color")) {
                Command cmd = { 0 };
                cmd.type = CMD_TRANSFORM;
                cmd.data.transform.index = selectedElementIndex;
                cmd.data.transform.before = *el;
                el->useCustomColor = false;
                cmd.data.transform.after = *el;
                ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
            }
        } else {
            GuiLabel((Rectangle){ inspX, inspY, inspW, btnH }, "No Element Selected");
        }
    }

    // 4. Animated Bottom Status Strip
    if (app->uiAnim.hudProgress > 0.01f) {
        float hudY = (float)winH - (bottomStripH * app->uiAnim.hudProgress);
        Rectangle stripRect = { 0, hudY, (float)winW, bottomStripH };
        DrawRectangleRec(stripRect, pBg);
        DrawLine(0, (int)hudY, winW, (int)hudY, pBorder);

        float sX = 10.0f * app->uiScale;
        float sY = hudY + 6.0f * app->uiScale;
        float sH = bottomStripH - 12.0f * app->uiScale;

        const char *curLName = (g_CADState.activeLayerIndex >= 0 && g_CADState.activeLayerIndex < app->layerCount) ? app->layers[g_CADState.activeLayerIndex].name : "0";
        GuiLabel((Rectangle){ sX, sY, 170.0f * app->uiScale, sH }, TextFormat("Layer: [%u] %s", g_CADState.activeLayerId, curLName));
        sX += 175.0f * app->uiScale;

        GuiLabel((Rectangle){ sX, sY, 150.0f * app->uiScale, sH }, TextFormat("W: (%.1f, %.1f)", g_CADState.mouseWorld.x, g_CADState.mouseWorld.y));
        sX += 155.0f * app->uiScale;

        GuiLabel((Rectangle){ sX, sY, 130.0f * app->uiScale, sH }, TextFormat("S: (%.0f, %.0f)", g_CADState.mouseScreen.x, g_CADState.mouseScreen.y));
        sX += 135.0f * app->uiScale;

        Rectangle snapGridRec = { sX, sY + 2, 16.0f * app->uiScale, 16.0f * app->uiScale };
        bool snapGridVal = app->snapToGrid;
        if (GuiCheckBox(snapGridRec, "Snap Grid", &snapGridVal)) {
            app->snapToGrid = snapGridVal;
            app->uiConfig.snapToGrid = app->snapToGrid;
            SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
        }
        sX += 105.0f * app->uiScale;

        Rectangle snapElemRec = { sX, sY + 2, 16.0f * app->uiScale, 16.0f * app->uiScale };
        bool snapElemVal = app->snapEnabled;
        if (GuiCheckBox(snapElemRec, "Snap Elem", &snapElemVal)) {
            app->snapEnabled = snapElemVal;
            app->uiConfig.snapEnabled = app->snapEnabled;
            SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
        }
        sX += 105.0f * app->uiScale;

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

        GuiLabel((Rectangle){ sX, sY, 150.0f * app->uiScale, sH }, TextFormat("Tool: %s", toolName));
        sX += 155.0f * app->uiScale;

        GuiLabel((Rectangle){ sX, sY, 160.0f * app->uiScale, sH }, TextFormat("Elements: %d (Sel: %d)", app->elementCount, CountSelectedElements(app->elements, app->elementCount)));
    }

    // 5. Unit Modal Window
    if (app->showUnitWindow) {
        Rectangle modalUnitRect = { (winW - 300.0f * app->uiScale) / 2.0f, (winH - 240.0f * app->uiScale) / 2.0f, 300.0f * app->uiScale, 240.0f * app->uiScale };
        DrawRectangle(0, 0, winW, winH, Fade(BLACK, 0.45f));
        DrawRectangleRec(modalUnitRect, pBg);
        DrawRectangleLinesEx(modalUnitRect, 1.0f, pBorder);
        DrawTextEx(bodyFont, "Measurement Unit", (Vector2){ modalUnitRect.x + 20.0f * app->uiScale, modalUnitRect.y + 12.0f * app->uiScale }, 14.0f * app->uiScale, 1.0f, pText);

        float uX = modalUnitRect.x + 20.0f * app->uiScale;
        float uY = modalUnitRect.y + 35.0f * app->uiScale;
        float uW = modalUnitRect.width - 40.0f * app->uiScale;

        if (GuiButton((Rectangle){ uX, uY, uW, btnH }, app->currentUnit == UNIT_MM ? "[X] mm" : "[ ] mm")) { app->currentUnit = UNIT_MM; app->uiConfig.currentUnit = app->currentUnit; SaveUiConfig(CONFIG_FILENAME, &app->uiConfig); } uY += btnH + spacing;
        if (GuiButton((Rectangle){ uX, uY, uW, btnH }, app->currentUnit == UNIT_CM ? "[X] cm" : "[ ] cm")) { app->currentUnit = UNIT_CM; app->uiConfig.currentUnit = app->currentUnit; SaveUiConfig(CONFIG_FILENAME, &app->uiConfig); } uY += btnH + spacing;
        if (GuiButton((Rectangle){ uX, uY, uW, btnH }, app->currentUnit == UNIT_M  ? "[X] m"  : "[ ] m"))  { app->currentUnit = UNIT_M;  app->uiConfig.currentUnit = app->currentUnit; SaveUiConfig(CONFIG_FILENAME, &app->uiConfig); } uY += btnH + spacing;
        if (GuiButton((Rectangle){ uX, uY, uW, btnH }, app->currentUnit == UNIT_IN ? "[X] in" : "[ ] in")) { app->currentUnit = UNIT_IN; app->uiConfig.currentUnit = app->currentUnit; SaveUiConfig(CONFIG_FILENAME, &app->uiConfig); } uY += btnH + spacing;
        if (GuiButton((Rectangle){ uX, uY, uW, btnH }, app->currentUnit == UNIT_FT ? "[X] ft" : "[ ] ft")) { app->currentUnit = UNIT_FT; app->uiConfig.currentUnit = app->currentUnit; SaveUiConfig(CONFIG_FILENAME, &app->uiConfig); } uY += btnH + spacing;

        if (GuiButton((Rectangle){ uX, uY + 5.0f, uW, btnH }, "Close")) {
            app->showUnitWindow = false;
        }
    }

    // 6. UI Scale Modal Window
    if (app->showScaleWindow) {
        Rectangle modalScaleRect = { (winW - 300.0f * app->uiScale) / 2.0f, (winH - 200.0f * app->uiScale) / 2.0f, 300.0f * app->uiScale, 200.0f * app->uiScale };
        DrawRectangle(0, 0, winW, winH, Fade(BLACK, 0.45f));
        DrawRectangleRec(modalScaleRect, pBg);
        DrawRectangleLinesEx(modalScaleRect, 1.0f, pBorder);
        DrawTextEx(bodyFont, "Adjust UI Scale (%)", (Vector2){ modalScaleRect.x + 20.0f * app->uiScale, modalScaleRect.y + 12.0f * app->uiScale }, 14.0f * app->uiScale, 1.0f, pText);

        float sX = modalScaleRect.x + 20.0f * app->uiScale;
        float sY = modalScaleRect.y + 40.0f * app->uiScale;
        float sW = modalScaleRect.width - 40.0f * app->uiScale;

        GuiLabel((Rectangle){ sX, sY, sW, btnH }, TextFormat("Scale: %.0f %%", app->tempUiScale)); sY += btnH + spacing;
        GuiSliderBar((Rectangle){ sX, sY, sW, btnH }, "", "", &app->tempUiScale, 50.0f, 400.0f); sY += btnH + spacing * 2;

        if (GuiButton((Rectangle){ sX, sY, (sW - spacing) / 2.0f, btnH }, "Apply")) {
            app->uiScale = app->tempUiScale / 100.0f;
            app->uiConfig.uiScale = app->uiScale;
            SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
        }
        if (GuiButton((Rectangle){ sX + (sW - spacing) / 2.0f + spacing, sY, (sW - spacing) / 2.0f, btnH }, "Close")) {
            app->showScaleWindow = false;
        }
    }

    // 7. Animated Context Menu
    if (app->uiAnim.contextMenuProgress > 0.01f) {
        float ctxWidth = 180.0f * app->uiScale;
        float ctxFullHeight = 210.0f * app->uiScale;
        float ctxHeight = ctxFullHeight * app->uiAnim.contextMenuProgress;
        Rectangle ctxMenuRect = { app->contextMenuPos.x, app->contextMenuPos.y, ctxWidth, ctxHeight };

        DrawRectangleRec(ctxMenuRect, Fade(pBg, app->uiAnim.contextMenuProgress));
        DrawRectangleLinesEx(ctxMenuRect, 1.0f, Fade(pBorder, app->uiAnim.contextMenuProgress));

        BeginScissorMode((int)ctxMenuRect.x, (int)ctxMenuRect.y, (int)ctxMenuRect.width, (int)ctxMenuRect.height);
        float cX = ctxMenuRect.x + 4.0f * app->uiScale;
        float cY = ctxMenuRect.y + 6.0f * app->uiScale;
        float cW = ctxMenuRect.width - 8.0f * app->uiScale;
        float cH = 20.0f * app->uiScale;

        if (app->contextOnElement && app->contextElementIndex >= 0 && app->contextElementIndex < app->elementCount) {
            if (GuiButton((Rectangle){ cX, cY, cW, cH }, "Delete Element")) {
                Command cmd = { 0 };
                cmd.type = CMD_DELETE;
                cmd.data.del.index = app->contextElementIndex;
                cmd.data.del.element = app->elements[app->contextElementIndex];
                ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                app->showContextMenu = false;
                snprintf(app->statusMessage, 64, "Element Deleted");
                app->statusMessageTimer = 1.5f;
            } cY += cH + 2;
            if (GuiButton((Rectangle){ cX, cY, cW, cH }, "Send Back (1 Step)")) {
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
            } cY += cH + 2;
            if (GuiButton((Rectangle){ cX, cY, cW, cH }, "Send to Backmost")) {
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
            } cY += cH + 2;
            if (GuiButton((Rectangle){ cX, cY, cW, cH }, "Bring to Active Layer")) {
                Command cmd = { 0 };
                cmd.type = CMD_LAYER_CHANGE;
                cmd.data.layerChange.index = app->contextElementIndex;
                cmd.data.layerChange.oldLayer = app->elements[app->contextElementIndex].layerIndex;
                cmd.data.layerChange.newLayer = app->activeLayerIndex;
                ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
                app->showContextMenu = false;
            } cY += cH + 2;
            if (GuiButton((Rectangle){ cX, cY, cW, cH }, "Duplicate Element")) {
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
            } cY += cH + 2;
            if (GuiButton((Rectangle){ cX, cY, cW, cH }, "Deselect Elements")) {
                DeselectAllElements(app->elements, app->elementCount);
                app->showContextMenu = false;
            }
        } else {
            if (GuiButton((Rectangle){ cX, cY, cW, cH }, "Select Mode")) { g_CADState.activeTool = TOOL_SELECT; app->showContextMenu = false; } cY += cH + 2;
            if (GuiButton((Rectangle){ cX, cY, cW, cH }, "P&ID Palette")) {
                CAD_PID_OpenPalette(&app->cadPid, app->contextMenuPos);
                app->showContextMenu = false;
            } cY += cH + 2;
            if (GuiButton((Rectangle){ cX, cY, cW, cH }, "+ Add Rectangle")) { g_CADState.activeTool = TOOL_ADD_RECT; app->showContextMenu = false; } cY += cH + 2;
            if (GuiButton((Rectangle){ cX, cY, cW, cH }, "+ Add Circle")) { g_CADState.activeTool = TOOL_ADD_CIRCLE; app->showContextMenu = false; } cY += cH + 2;
            if (GuiButton((Rectangle){ cX, cY, cW, cH }, "Deselect Elements")) { DeselectAllElements(app->elements, app->elementCount); app->showContextMenu = false; } cY += cH + 2;
            if (GuiButton((Rectangle){ cX, cY, cW, cH }, "Reset View")) { DispatchCommand(app, "reset"); app->showContextMenu = false; }
        }
        EndScissorMode();
    }

    // 8. Top Menu Popups
    if (app->openFileMenu) {
        Rectangle pop = { 4.0f * app->uiScale, menuBarHeight, 150.0f * app->uiScale, 4 * (btnH + 2) + 6 };
        DrawRectangleRec(pop, pBg);
        DrawRectangleLinesEx(pop, 1.0f, pBorder);
        float py = pop.y + 3;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "New Project")) { DispatchCommand(app, "new"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Open Project")) { DispatchCommand(app, "open"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Save Project")) { DispatchCommand(app, "save"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Exit")) { DispatchCommand(app, "exit"); CloseAllPopups(app); }
    }
    if (app->openEditMenu) {
        Rectangle pop = { (4.0f + 68.0f + 4.0f) * app->uiScale, menuBarHeight, 130.0f * app->uiScale, 2 * (btnH + 2) + 6 };
        DrawRectangleRec(pop, pBg);
        DrawRectangleLinesEx(pop, 1.0f, pBorder);
        float py = pop.y + 3;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Undo")) { DispatchCommand(app, "undo"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Redo")) { DispatchCommand(app, "redo"); CloseAllPopups(app); }
    }
    if (app->openWindowMenu) {
        Rectangle pop = { (4.0f + (68.0f + 4.0f) * 2) * app->uiScale, menuBarHeight, 225.0f * app->uiScale, 11 * (btnH + 2) + 8 };
        DrawRectangleRec(pop, pBg);
        DrawRectangleLinesEx(pop, 1.0f, pBorder);
        float py = pop.y + 3;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Reset View")) { DispatchCommand(app, "reset"); CloseAllPopups(app); } py += btnH + 2;
        bool isFs = IsWindowFullscreen();
        if (GuiCheckBox((Rectangle){ pop.x + 5, py + 2, 16.0f * app->uiScale, 16.0f * app->uiScale }, "Fullscreen", &isFs)) {
            ToggleFullscreen();
            app->uiConfig.isFullscreen = isFs;
            SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
        } py += btnH + 2;
        if (GuiCheckBox((Rectangle){ pop.x + 5, py + 2, 16.0f * app->uiScale, 16.0f * app->uiScale }, "Left Panel (Layers/Elems)", &app->showLeftDock)) {} py += btnH + 2;
        if (GuiCheckBox((Rectangle){ pop.x + 5, py + 2, 16.0f * app->uiScale, 16.0f * app->uiScale }, "Right Panel (Inspector)", &app->showRightDock)) {} py += btnH + 2;
        if (GuiCheckBox((Rectangle){ pop.x + 5, py + 2, 16.0f * app->uiScale, 16.0f * app->uiScale }, "Bottom Status Strip", &app->showHudPanel)) {} py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Unit Settings")) { app->showUnitWindow = !app->showUnitWindow; CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "UI Scale")) {
            app->showScaleWindow = !app->showScaleWindow;
            app->tempUiScale = app->uiScale * 100.0f;
            CloseAllPopups(app);
        } py += btnH + 2;
        const char *thmDark = (app->uiConfig.uiTheme == UI_THEME_DARK) ? "[*] Theme: Dark" : "[ ] Theme: Dark";
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, thmDark)) {
            app->uiConfig.uiTheme = UI_THEME_DARK;
            ApplyRayguiTheme(UI_THEME_DARK);
            SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Theme: Dark");
            app->statusMessageTimer = 2.0f;
            CloseAllPopups(app);
        } py += btnH + 2;
        const char *thmLight = (app->uiConfig.uiTheme == UI_THEME_LIGHT) ? "[*] Theme: Light" : "[ ] Theme: Light";
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, thmLight)) {
            app->uiConfig.uiTheme = UI_THEME_LIGHT;
            ApplyRayguiTheme(UI_THEME_LIGHT);
            SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Theme: Light");
            app->statusMessageTimer = 2.0f;
            CloseAllPopups(app);
        } py += btnH + 2;
        const char *uiOpt1 = (app->uiConfig.uiBackend == UI_BACKEND_MICROUI) ? "[*] Startup UI: microui" : "[ ] Startup UI: microui";
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, uiOpt1)) {
            app->uiConfig.uiBackend = UI_BACKEND_MICROUI;
            SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Next Startup: microui");
            app->statusMessageTimer = 2.5f;
            CloseAllPopups(app);
        } py += btnH + 2;
        const char *uiOpt2 = (app->uiConfig.uiBackend == UI_BACKEND_RAYGUI) ? "[*] Startup UI: raygui" : "[ ] Startup UI: raygui";
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, uiOpt2)) {
            app->uiConfig.uiBackend = UI_BACKEND_RAYGUI;
            SaveUiConfig(CONFIG_FILENAME, &app->uiConfig);
            snprintf(app->statusMessage, sizeof(app->statusMessage), "Next Startup: raygui");
            app->statusMessageTimer = 2.5f;
            CloseAllPopups(app);
        }
    }
    if (app->openElementMenu) {
        Rectangle pop = { (4.0f + (68.0f + 4.0f) * 2 + 83.0f * app->uiScale), menuBarHeight, 170.0f * app->uiScale, 11 * (btnH + 2) + 6 };
        DrawRectangleRec(pop, pBg);
        DrawRectangleLinesEx(pop, 1.0f, pBorder);
        float py = pop.y + 3;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Select Tool"))    { DispatchCommand(app, "select"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "P&ID Circular Palate")) {
            CAD_PID_OpenPalette(&app->cadPid, (Vector2){ (float)winW * 0.5f, (float)winH * 0.5f });
            CloseAllPopups(app);
        } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Add Rectangle"))  { DispatchCommand(app, "rect"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Add Circle"))     { DispatchCommand(app, "circle"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Add Line"))       { DispatchCommand(app, "line"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Add  Polyline"))  { DispatchCommand(app, "polyline"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Add Freehand"))   { DispatchCommand(app, "freehand"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Add 3-Pt Arc"))   { DispatchCommand(app, "arc"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Add Ellipse"))    { DispatchCommand(app, "ellipse"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Add Text Note"))  { DispatchCommand(app, "text"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Clear Elements")) { DispatchCommand(app, "clear"); CloseAllPopups(app); }
    }
    if (app->openFunctionsMenu) {
        Rectangle pop = { (4.0f + (68.0f + 4.0f) * 2 + (83.0f + 83.0f) * app->uiScale), menuBarHeight, 140.0f * app->uiScale, 2 * (btnH + 2) + 6 };
        DrawRectangleRec(pop, pBg);
        DrawRectangleLinesEx(pop, 1.0f, pBorder);
        float py = pop.y + 3;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Dimension")) { DispatchCommand(app, "dimension"); CloseAllPopups(app); } py += btnH + 2;
        if (GuiButton((Rectangle){ pop.x + 3, py, pop.width - 6, btnH }, "Pan Mode"))  { DispatchCommand(app, "pan"); CloseAllPopups(app); }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckGuiHover_Raygui(app) && !clickedTopMenuButton) {
        CloseAllPopups(app);
        app->layerRenameEditMode = false;
        app->noteTextEditMode = false;
        s_flangeClassOpen = false;
        s_flangeNpsOpen = false;
    }
}