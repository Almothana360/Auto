#include "cad_connection.h"
#include "cad_context.h"
#include "cad_math.h"
#include "flange.h"
#include "commands.h"
#include "render_utils.h"
#include "raygui.h"
#include "raymath.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "project_io.h"

void ConnectionSystem_Init(ConnectionSystemState *conn) {
    if (!conn) return;
    memset(conn, 0, sizeof(ConnectionSystemState));
    conn->targetElementIndex = -1;
    conn->activePortType = CONN_PORT_NONE;
    conn->hoveredOption = -1;
    conn->justOpened = false;
    conn->showMessageBox = false;
    snprintf(conn->messageBoxTitle, sizeof(conn->messageBoxTitle), "Information");
    snprintf(conn->messageBoxText, sizeof(conn->messageBoxText), "Has Not been Implemented!");
}

void ConnectionSystem_UpdatePorts(ConnectionSystemState *conn, AppContext *app) {
    if (!conn || !app) return;
    conn->portCount = 0;
    conn->hasActivePorts = false;
    conn->targetElementIndex = -1;

    int selectedCount = CountSelectedElements(app->elements, app->elementCount);
    if (selectedCount != 1 || app->isDraggingElement || app->isBoxSelecting) {
        conn->showContextMenu = false;
        return;
    }

    int selIdx = GetFirstSelectedIndex(app->elements, app->elementCount);
    if (selIdx < 0 || selIdx >= app->elementCount) return;

    GridElement *el = &app->elements[selIdx];
    bool isFlange = ((el->type == ELEMENT_SYMBOL || el->type == ELEMENT_PID) && strchr(el->text, '|') != NULL);
    if (!isFlange) {
        conn->showContextMenu = false;
        return;
    }

    conn->targetElementIndex = selIdx;
    conn->hasActivePorts = true;

    float fw = (el->width > 0.0f) ? el->width : 19.1f;
    float ft = (el->radius > 0.0f) ? el->radius : 63.5f;

    float rad = el->rotation * DEG2RAD;
    Vector2 dirX = { cosf(rad), sinf(rad) };   // Local (+1, 0) in world space
    Vector2 negDirX = { -dirX.x, -dirX.y };    // Local (-1, 0) in world space

    // 1. Flange Head (Mating Face): local (fw, 0), normal (+1, 0) pointing to the right
    Vector2 headLocal = { fw, 0.0f };
    Vector2 headWorld = LocalToWorldPoint(headLocal, el->pos, el->rotation);
    conn->ports[0].type = CONN_PORT_FLANGE_HEAD;
    conn->ports[0].worldPos = headWorld;
    conn->ports[0].normal = dirX;

    // 2. Flange Tail (Weld Neck Hub): local (fw - ft, 0), normal (-1, 0) pointing to the left
    Vector2 tailLocal = { fw - ft, 0.0f };
    Vector2 tailWorld = LocalToWorldPoint(tailLocal, el->pos, el->rotation);
    conn->ports[1].type = CONN_PORT_FLANGE_TAIL;
    conn->ports[1].worldPos = tailWorld;
    conn->ports[1].normal = negDirX;

    conn->portCount = 2;

    // Compact, balanced triangle proportions
    float baseScreenH  = 22.0f * app->uiScale;
    float heightScreen = 18.0f * app->uiScale;
    float gapScreen    = 8.0f  * app->uiScale;

    float triBase   = baseScreenH / app->camera.zoom;
    float halfBase  = triBase * 0.5f;
    float triHeight = heightScreen / app->camera.zoom;
    float gap       = gapScreen / app->camera.zoom;
    float pad       = 4.0f / app->camera.zoom; // Clearance matching selection bounding box

    for (int i = 0; i < 2; i++) {
        Vector2 n = conn->ports[i].normal;
        Vector2 perp = { -n.y, n.x };
        Vector2 pCenter = conn->ports[i].worldPos;

        // Base center positioned just outside the bounding box
        Vector2 baseCenter = Vector2Add(pCenter, Vector2Scale(n, pad + gap));
        Vector2 tip   = Vector2Add(baseCenter, Vector2Scale(n, triHeight));
        Vector2 base1 = Vector2Add(baseCenter, Vector2Scale(perp, halfBase));
        Vector2 base2 = Vector2Subtract(baseCenter, Vector2Scale(perp, halfBase));

        conn->ports[i].triPts[0] = tip;
        conn->ports[i].triPts[1] = base1;
        conn->ports[i].triPts[2] = base2;

        Vector2 sTip = GetWorldToScreen2D(tip, app->camera);
        Vector2 sB1  = GetWorldToScreen2D(base1, app->camera);
        Vector2 sB2  = GetWorldToScreen2D(base2, app->camera);

        float minX = fminf(fminf(sTip.x, sB1.x), sB2.x) - 6.0f;
        float maxX = fmaxf(fmaxf(sTip.x, sB1.x), sB2.x) + 6.0f;
        float minY = fminf(fminf(sTip.y, sB1.y), sB2.y) - 6.0f;
        float maxY = fmaxf(fmaxf(sTip.y, sB1.y), sB2.y) + 6.0f;

        conn->ports[i].screenHitBox = (Rectangle){ minX, minY, maxX - minX, maxY - minY };
    }
}

bool ConnectionSystem_HandleInput(ConnectionSystemState *conn, AppContext *app, bool overUI) {
    if (!conn || !app) return false;
    if (conn->showMessageBox) return true;

    Vector2 mouseScreen = g_CADState.mouseScreen;

    if (conn->showContextMenu) {
        float menuW = 160.0f * app->uiScale;
        float itemH = 24.0f * app->uiScale;
        float totalH = (itemH * 5.0f) + (8.0f * app->uiScale);
        Rectangle menuRect = { conn->menuScreenPos.x, conn->menuScreenPos.y, menuW, totalH };

        if (IsKeyPressed(KEY_ESCAPE)) {
            conn->showContextMenu = false;
            return true;
        }

        if (CheckCollisionPointRec(mouseScreen, menuRect)) {
            return true;
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            conn->showContextMenu = false;
        }
    }

    if (conn->hasActivePorts && !overUI) {
        for (int i = 0; i < conn->portCount; i++) {
            if (CheckCollisionPointRec(mouseScreen, conn->ports[i].screenHitBox)) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    conn->showContextMenu = true;
                    conn->justOpened = true;
                    conn->activePortType = conn->ports[i].type;

                    Vector2 sTip = GetWorldToScreen2D(conn->ports[i].triPts[0], app->camera);
                    float menuW = 160.0f * app->uiScale;
                    float itemH = 24.0f * app->uiScale;
                    float totalH = (itemH * 5.0f) + (8.0f * app->uiScale);

                    // Position context menu adjacent to the triangle tip
                    if (conn->ports[i].normal.x >= 0.0f) {
                        conn->menuScreenPos.x = sTip.x + 8.0f * app->uiScale;
                        conn->menuScreenPos.y = sTip.y - 20.0f * app->uiScale;
                    } else {
                        conn->menuScreenPos.x = sTip.x - menuW - 8.0f * app->uiScale;
                        conn->menuScreenPos.y = sTip.y - 20.0f * app->uiScale;
                    }

                    // Clamp within screen boundaries
                    if (conn->menuScreenPos.x + menuW > (float)GetScreenWidth() - 10.0f) {
                        conn->menuScreenPos.x = (float)GetScreenWidth() - menuW - 10.0f;
                    }
                    if (conn->menuScreenPos.x < 10.0f) {
                        conn->menuScreenPos.x = 10.0f;
                    }
                    if (conn->menuScreenPos.y + totalH > (float)GetScreenHeight() - 10.0f) {
                        conn->menuScreenPos.y = (float)GetScreenHeight() - totalH - 10.0f;
                    }
                    if (conn->menuScreenPos.y < 35.0f * app->uiScale) {
                        conn->menuScreenPos.y = 35.0f * app->uiScale;
                    }

                    return true;
                }
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    return true;
                }
            }
        }
    }

    return false;
}

void ConnectionSystem_RenderPortButtons(const ConnectionSystemState *conn, AppContext *app) {
    if (!conn || !conn->hasActivePorts || !app) return;

    Vector2 mouseScreen = g_CADState.mouseScreen;

    Color bgFill = (app->uiConfig.uiTheme == UI_THEME_LIGHT) ? (Color){ 240, 242, 245, 240 } : (Color){ 24, 25, 28, 240 };
    Color bgHoverFill = (app->uiConfig.uiTheme == UI_THEME_LIGHT) ? (Color){ 220, 230, 245, 240 } : (Color){ 45, 50, 60, 240 };
    Color normStroke = (app->uiConfig.uiTheme == UI_THEME_LIGHT) ? (Color){ 70, 75, 85, 255 } : (Color){ 215, 220, 225, 255 };

    float strokeThick = 1.5f / app->camera.zoom;
    if (strokeThick < 1.0f) strokeThick = 1.0f;
    if (strokeThick > 2.5f) strokeThick = 2.5f;

    for (int i = 0; i < conn->portCount; i++) {
        bool hovered = CheckCollisionPointRec(mouseScreen, conn->ports[i].screenHitBox);
        Color fillCol = hovered ? bgHoverFill : bgFill;
        Color strokeCol = hovered ? GOLD : normStroke;

        // Double-sided triangle fill to ensure clean rasterization regardless of face winding
        DrawTriangle(conn->ports[i].triPts[0], conn->ports[i].triPts[1], conn->ports[i].triPts[2], fillCol);
        DrawTriangle(conn->ports[i].triPts[0], conn->ports[i].triPts[2], conn->ports[i].triPts[1], fillCol);

        // Crisp outlined borders
        DrawLineEx(conn->ports[i].triPts[0], conn->ports[i].triPts[1], strokeThick, strokeCol);
        DrawLineEx(conn->ports[i].triPts[1], conn->ports[i].triPts[2], strokeThick, strokeCol);
        DrawLineEx(conn->ports[i].triPts[2], conn->ports[i].triPts[0], strokeThick, strokeCol);
    }
}

static void ConnectNewFlange(AppContext *app, int sourceIndex) {
    if (!app || sourceIndex < 0 || sourceIndex >= app->elementCount || app->elementCount >= MAX_ELEMENTS) return;

    GridElement *src = &app->elements[sourceIndex];
    char curClass[16] = "150#";
    char curNps[16] = "2\"";
    char *sep = strchr(src->text, '|');
    if (sep) {
        size_t cLen = (size_t)(sep - src->text);
        if (cLen < sizeof(curClass)) {
            strncpy(curClass, src->text, cLen);
            curClass[cLen] = '\0';
        }
        strncpy(curNps, sep + 1, sizeof(curNps) - 1);
        curNps[sizeof(curNps) - 1] = '\0';
    }

    FlangeSpec spec;
    Flange_InitDefaultSpec(&spec, FLANGE_WELD_NECK);
    Flange_SetSpecBySize(&spec, curClass, curNps);

    // Mated flange placed facing the source flange head-to-head
    float newRot = src->rotation + 180.0f;
    while (newRot >= 360.0f) newRot -= 360.0f;

    // The mating face of the source flange is at headWorld
    Vector2 headWorld = LocalToWorldPoint((Vector2){ src->width, 0.0f }, src->pos, src->rotation);

    // Calculate position such that the new flange's face meets headWorld flush
    float rad = src->rotation * DEG2RAD;
    Vector2 offset = { spec.fw * cosf(rad), spec.fw * sinf(rad) };
    Vector2 newPos = Vector2Add(headWorld, offset);

    GridElement newEl = Flange_CreateGridElement(newPos, newRot, src->layerIndex, &spec);
    newEl.lineThickness = src->lineThickness;
    newEl.useCustomColor = src->useCustomColor;
    newEl.color = src->color;
    GetElementAABB(&newEl);

    DeselectAllElements(app->elements, app->elementCount);
    newEl.selected = true;

    Command cmd = { 0 };
    cmd.type = CMD_CREATE;
    cmd.data.create.index = app->elementCount;
    cmd.data.create.element = newEl;
    ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);

    snprintf(app->statusMessage, sizeof(app->statusMessage), "Connected Mated Flange [%s %s]", curClass, curNps);
    app->statusMessageTimer = 2.5f;
}

void ConnectionSystem_RenderContextMenu(ConnectionSystemState *conn, AppContext *app) {
    if (!conn || !conn->showContextMenu || !app) return;

    float menuW = 160.0f * app->uiScale;
    float itemH = 24.0f * app->uiScale;
    const char *headOptions[] = { "Flange", "Valve", "Blind Flange", "Gasket", "Instrument" };
    const char *tailOptions[] = { "Pipe", "Elbow", "Tee", "Reducer", "Cap" };
    int optionCount = 5;

    float totalH = (itemH * (float)optionCount) + (8.0f * app->uiScale);
    Rectangle menuRec = { conn->menuScreenPos.x, conn->menuScreenPos.y, menuW, totalH };

    Color bgCol = (app->uiConfig.uiTheme == UI_THEME_LIGHT) ? (Color){ 245, 246, 248, 255 } : (Color){ 36, 36, 36, 255 };
    Color borderCol = (app->uiConfig.uiTheme == UI_THEME_LIGHT) ? (Color){ 180, 185, 195, 255 } : (Color){ 20, 20, 20, 255 };

    DrawRectangleRec(menuRec, bgCol);
    DrawRectangleLinesEx(menuRec, 1.0f, borderCol);

    float curY = menuRec.y + (4.0f * app->uiScale);
    float btnX = menuRec.x + (4.0f * app->uiScale);
    float btnW = menuRec.width - (8.0f * app->uiScale);

    bool canClick = !conn->justOpened;
    if (conn->justOpened && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        conn->justOpened = false;
    }

    for (int i = 0; i < optionCount; i++) {
        const char *optName = (conn->activePortType == CONN_PORT_FLANGE_HEAD) ? headOptions[i] : tailOptions[i];
        Rectangle btnRec = { btnX, curY, btnW, itemH - 2.0f };

        if (canClick && GuiButton(btnRec, optName)) {
            conn->showContextMenu = false;
            if (conn->activePortType == CONN_PORT_FLANGE_HEAD && i == 0) {
                ConnectNewFlange(app, conn->targetElementIndex);
            } else {
                conn->showMessageBox = true;
                snprintf(conn->messageBoxTitle, sizeof(conn->messageBoxTitle), "Information");
                snprintf(conn->messageBoxText, sizeof(conn->messageBoxText), "Has Not been Implemented!");
            }
            break;
        } else if (!canClick) {
            GuiButton(btnRec, optName);
        }
        curY += itemH;
    }
}

void ConnectionSystem_RenderMessageBox(ConnectionSystemState *conn, AppContext *app) {
    if (!conn || !conn->showMessageBox || !app) return;

    int winW = GetScreenWidth();
    int winH = GetScreenHeight();

    DrawRectangle(0, 0, winW, winH, Fade(BLACK, 0.40f));

    float boxW = 280.0f * app->uiScale;
    float boxH = 140.0f * app->uiScale;
    Rectangle boxRec = { ((float)winW - boxW) * 0.5f, ((float)winH - boxH) * 0.5f, boxW, boxH };

    if (conn->showMessageBox) {
        int result = -1;
        GuiMessageBox(boxRec, conn->messageBoxTitle, conn->messageBoxText, "OK", &result);
        if (result >= 0) {
            conn->showMessageBox = false;
        }
    }
}

bool ConnectionSystem_IsHovered(const ConnectionSystemState *conn, const AppContext *app) {
    if (!conn) return false;
    if (conn->showMessageBox) return true;
    Vector2 mouseScreen = g_CADState.mouseScreen;
    if (conn->showContextMenu && app) {
        float menuW = 160.0f * app->uiScale;
        float itemH = 24.0f * app->uiScale;
        float totalH = (itemH * 5.0f) + (8.0f * app->uiScale);
        Rectangle menuRec = { conn->menuScreenPos.x, conn->menuScreenPos.y, menuW, totalH };
        if (CheckCollisionPointRec(mouseScreen, menuRec)) return true;
    }
    return false;
}