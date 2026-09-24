#include "ui_manager.h"
#include <stddef.h>
#include <math.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "gui_panels.h"
#include "gui_panels_raygui.h"
#include "render_utils.h"
#include "cad_pid.h"
#include "cad_connection.h"

void UIManager_Init(AppContext *app, mu_Context *mu_ctx) {
    mu_init(mu_ctx);
    mu_ctx->text_width = TextWidthCallback;
    mu_ctx->text_height = TextHeightCallback;

    Font bodyFont = ResourceManager_GetFont(&app->resManager, FONT_SLOT_BODY);
    SetActiveUIFont(bodyFont);
    GuiSetFont(bodyFont);
    GuiSetStyle(DEFAULT, TEXT_SIZE, (int)(11 * app->uiScale));

    int iconScale = (int)roundf(app->uiScale);
    if (iconScale < 1) iconScale = 1;
    GuiSetIconScale(iconScale);

    ApplyRayguiTheme(app->uiConfig.uiTheme);
}

void UIManager_ProcessInput(AppContext *app, mu_Context *mu_ctx) {
    if (app->uiConfig.uiBackend != UI_BACKEND_MICROUI) return;
    if (app->connState.showMessageBox) return;

    Vector2 mousePos = GetMousePosition();
    mu_input_mousemove(mu_ctx, (int)mousePos.x, (int)mousePos.y);

    Vector2 mouseWheel = GetMouseWheelMoveV();
    if (mouseWheel.y != 0.0f || mouseWheel.x != 0.0f) {
        mu_input_scroll(mu_ctx, (int)(mouseWheel.x * -30.0f), (int)(mouseWheel.y * -30.0f));
    }

    int btnMap[3] = { MOUSE_BUTTON_LEFT, MOUSE_BUTTON_RIGHT, MOUSE_BUTTON_MIDDLE };
    int muBtnMap[3] = { MU_MOUSE_LEFT, MU_MOUSE_RIGHT, MU_MOUSE_MIDDLE };

    for (int b = 0; b < 3; b++) {
        if (IsMouseButtonPressed(btnMap[b])) mu_input_mousedown(mu_ctx, (int)mousePos.x, (int)mousePos.y, muBtnMap[b]);
        if (IsMouseButtonReleased(btnMap[b])) mu_input_mouseup(mu_ctx, (int)mousePos.x, (int)mousePos.y, muBtnMap[b]);
    }

    if (!app->commandEditMode) {
        int charCode = GetCharPressed();
        while (charCode > 0) {
            if (charCode >= 32 && charCode < 127) {
                char s[2] = { (char)charCode, 0 };
                mu_input_text(mu_ctx, s);
            }
            charCode = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) mu_input_keydown(mu_ctx, MU_KEY_BACKSPACE);
        if (IsKeyReleased(KEY_BACKSPACE)) mu_input_keyup(mu_ctx, MU_KEY_BACKSPACE);
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) mu_input_keydown(mu_ctx, MU_KEY_RETURN);
        if (IsKeyReleased(KEY_ENTER) || IsKeyReleased(KEY_KP_ENTER)) mu_input_keyup(mu_ctx, MU_KEY_RETURN);
    }
}

bool UIManager_UpdateAndRenderPanels(AppContext *app, mu_Context *mu_ctx) {
    bool overUI = false;
    Vector2 mousePos = GetMousePosition();
    int winW = GetScreenWidth();
    int winH = GetScreenHeight();

    if (app->cadPid.isPaletteOpen || ConnectionSystem_IsHovered(&app->connState, app)) {
        return true;
    }

    float cmdW = 460.0f * app->uiScale;
    float cmdH = 26.0f * app->uiScale;
    float bottomStripH = 34.0f * app->uiScale;
    Rectangle commandBoxRect = { ((float)winW - cmdW) / 2.0f, (float)winH - bottomStripH - cmdH - (4.0f * app->uiScale), cmdW, cmdH };

    if (app->uiConfig.uiBackend == UI_BACKEND_MICROUI) {
        RenderAllGuiPanels(mu_ctx, app);
        overUI = (mu_ctx->hover_root != NULL) || CheckCollisionPointRec(mousePos, commandBoxRect);
    } else {
        overUI = CheckGuiHover_Raygui(app);
        if (CheckCollisionPointRec(mousePos, commandBoxRect)) overUI = true;
    }

    return overUI;
}

void UIManager_RenderOverlays(AppContext *app, mu_Context *mu_ctx) {
    int winW = GetScreenWidth();
    int winH = GetScreenHeight();
    float bottomStripH = 34.0f * app->uiScale;
    float menuBarHeight = 32.0f * app->uiScale;

    Color barBg = {
        (unsigned char)app->uiAnim.panelBgR,
        (unsigned char)app->uiAnim.panelBgG,
        (unsigned char)app->uiAnim.panelBgB,
        (unsigned char)app->uiAnim.panelBgA
    };
    Color barBorder = {
        (unsigned char)app->uiAnim.borderR,
        (unsigned char)app->uiAnim.borderG,
        (unsigned char)app->uiAnim.borderB,
        (unsigned char)app->uiAnim.borderA
    };

    if (app->uiConfig.uiBackend == UI_BACKEND_MICROUI) {
        if (g_CADState.activeTool != TOOL_PAN) {
            DrawRectangle(0, 0, winW, (int)menuBarHeight, barBg);
            DrawLine(0, (int)menuBarHeight, winW, (int)menuBarHeight, barBorder);

            if (app->uiAnim.hudProgress > 0.01f) {
                float hudY = (float)winH - (bottomStripH * app->uiAnim.hudProgress);
                DrawRectangle(0, (int)hudY, winW, (int)bottomStripH, barBg);
                DrawLine(0, (int)hudY, winW, (int)hudY, barBorder);
            }
        }
        Font bodyFont = ResourceManager_GetFont(&app->resManager, FONT_SLOT_BODY);
        RenderMicroui(mu_ctx, bodyFont);
    } else {
        RenderAllGuiPanels_Raygui(app);
    }

    // Render P&ID Circular Palette overlay
    Font menuFont = ResourceManager_GetFont(&app->resManager, FONT_SLOT_MENU);
    CAD_PID_RenderPalette(&app->cadPid, app->uiScale, menuFont);

    // Render Connection System context menu
    ConnectionSystem_RenderContextMenu(&app->connState, app);

    // Bottom command prompt
    float cmdW = 460.0f * app->uiScale;
    float cmdH = 26.0f * app->uiScale;
    Rectangle commandBoxRect = { ((float)winW - cmdW) / 2.0f, (float)winH - bottomStripH - cmdH - (4.0f * app->uiScale), cmdW, cmdH };

    if (g_CADState.activeTool != TOOL_PAN) {
        DrawRectangleRec(commandBoxRect, (app->uiConfig.uiTheme == UI_THEME_LIGHT) ? (Color){ 252, 252, 252, 255 } : (Color){ 32, 32, 32, 255 });
        DrawRectangleLinesEx(commandBoxRect, 1.0f, barBorder);
        if (GuiTextBox(commandBoxRect, app->commandText, CMD_BUFFER_SIZE, app->commandEditMode)) {
            app->commandEditMode = !app->commandEditMode;
        }
    }

    // Modal RayGUI Message Box (supported across both backends)
    ConnectionSystem_RenderMessageBox(&app->connState, app);

    if (app->statusMessageTimer > 0.0f) {
        Font titleFont = ResourceManager_GetFont(&app->resManager, FONT_SLOT_TITLE);
        float fontSize = 14.0f * app->uiScale;
        Vector2 smSize = MeasureTextEx(titleFont, app->statusMessage, fontSize, 1.0f);
        float smW = smSize.x + (20.0f * app->uiScale);
        DrawRectangleRec((Rectangle){ ((float)winW - smW)/2.0f, (float)winH - bottomStripH - cmdH - (36.0f * app->uiScale), smW, 25.0f * app->uiScale }, Fade(DARKGRAY, 0.85f));
        DrawTextEx(titleFont, app->statusMessage, (Vector2){ ((float)winW - smW)/2.0f + (10.0f * app->uiScale), (float)winH - bottomStripH - cmdH - (31.0f * app->uiScale) }, fontSize, 1.0f, WHITE);
    }
}