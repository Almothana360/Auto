#include "render_utils.h"
#include "cad_math.h"
#include "raymath.h"
#include <math.h>
#include <string.h>

static Font g_CurrentUIFont = { 0 };

void SetActiveUIFont(Font font) {
    g_CurrentUIFont = font;
}

Color GetElementColor(const GridElement *el, const Layer *layers, int layerCount) {
    if (!el) return WHITE;
    if (el->useCustomColor) {
        Color c = el->color;
        if (c.a == 0) c.a = 255;
        return c;
    }
    if (el->layerIndex >= 0 && el->layerIndex < layerCount) {
        Color c = layers[el->layerIndex].defaultColor;
        if (c.a == 0) c.a = 255;
        return c;
    }
    return RAYWHITE;
}

void DrawDimensionElement(GridElement *el, MeasureUnit currentUnit, float zoom, bool isSelected, Font font) {
    Vector2 dir = { el->p2.x - el->p1.x, el->p2.y - el->p1.y };
    float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (length < 0.001f) return;
    Vector2 uDir = { dir.x / length, dir.y / length };
    Vector2 normal = { -uDir.y, uDir.x };
    Vector2 vP1ToOffset = { el->dimPos.x - el->p1.x, el->dimPos.y - el->p1.y };
    float offsetDist = vP1ToOffset.x * normal.x + vP1ToOffset.y * normal.y;
    Vector2 dimP1 = { el->p1.x + normal.x * offsetDist, el->p1.y + normal.y * offsetDist };
    Vector2 dimP2 = { el->p2.x + normal.x * offsetDist, el->p2.y + normal.y * offsetDist };
    Color renderColor = isSelected ? GOLD : el->color;
    float lThick = (el->lineThickness > 0.0f ? el->lineThickness : 3.0f) / zoom;
    float tThick = (el->tickThickness > 0.0f ? el->tickThickness : 2.0f) / zoom;

    DrawLineEx(el->p1, dimP1, 1.0f / zoom, Fade(renderColor, 0.6f));
    DrawLineEx(el->p2, dimP2, 1.0f / zoom, Fade(renderColor, 0.6f));
    DrawLineEx(dimP1, dimP2, lThick, renderColor);

    float tickSize = 12.0f / zoom;
    Vector2 tickVec = { (uDir.x + normal.x) * tickSize * 0.5f, (uDir.y + normal.y) * tickSize * 0.5f };
    DrawLineEx((Vector2){ dimP1.x - tickVec.x, dimP1.y - tickVec.y }, (Vector2){ dimP1.x + tickVec.x, dimP1.y + tickVec.y }, tThick, renderColor);
    DrawLineEx((Vector2){ dimP2.x - tickVec.x, dimP2.y - tickVec.y }, (Vector2){ dimP2.x + tickVec.x, dimP2.y + tickVec.y }, tThick, renderColor);

    float convLength;
    const char *unitStr;
    GetUnitConvertedLength(length, currentUnit, &convLength, &unitStr);
    Vector2 midPoint = { (dimP1.x + dimP2.x) * 0.5f, (dimP1.y + dimP2.y) * 0.5f };
    const char *distText = TextFormat("%.2f %s", convLength, unitStr);
    float fSize = (float)el->textSize / zoom;
    if (fSize < 10.0f) fSize = 10.0f;
    Vector2 txtDim = MeasureTextEx(font, distText, fSize, 1.0f);
    DrawTextEx(font, distText, (Vector2){ midPoint.x - txtDim.x * 0.5f, midPoint.y - txtDim.y * 0.5f }, fSize, 1.0f, renderColor);
}

void DrawTextNoteElement(GridElement *el, float zoom, bool isSelected, Font font) {
    Color borderCol = isSelected ? GOLD : el->color;
    Rectangle textRec = { el->pos.x - el->width * 0.5f, el->pos.y - el->height * 0.5f, el->width, el->height };
    DrawRectangleRec(textRec, Fade(WHITE, 0.85f));
    DrawRectangleLinesEx(textRec, (isSelected ? 2.0f : 1.0f) / zoom, borderCol);

    if (el->showArrow) {
        Vector2 textCenter = el->pos;
        DrawLineEx(textCenter, el->arrowTarget, (el->lineThickness > 0.0f ? el->lineThickness : 3.0f) / zoom, borderCol);
        Vector2 dir = Vector2Normalize(Vector2Subtract(el->arrowTarget, textCenter));
        Vector2 side1 = Vector2Rotate(dir, 145.0f * DEG2RAD);
        Vector2 side2 = Vector2Rotate(dir, -145.0f * DEG2RAD);
        float arrowSize = 12.0f / zoom;
        Vector2 p1 = Vector2Add(el->arrowTarget, Vector2Scale(side1, arrowSize));
        Vector2 p2 = Vector2Add(el->arrowTarget, Vector2Scale(side2, arrowSize));
        DrawTriangle(el->arrowTarget, p1, p2, borderCol);
    }

    float fSize = (float)el->textSize / zoom;
    if (fSize < 8.0f) fSize = 8.0f;
    Vector2 txtPos = { textRec.x + 8.0f / zoom, textRec.y + 8.0f / zoom };
    DrawTextEx(font, el->text, txtPos, fSize, 1.0f, borderCol);
}

void DrawElementSelectionGizmo(const GridElement *el, float zoom) {
    if (!el || !el->selected) return;

    // Outer Bounding Box visualization for every element
    AABB box = el->bbox;
    float pad = 4.0f / zoom;
    Rectangle selBox = {
        box.min.x - pad,
        box.min.y - pad,
        (box.max.x - box.min.x) + pad * 2.0f,
        (box.max.y - box.min.y) + pad * 2.0f
    };

    // Soft selection tint and distinct border
    DrawRectangleRec(selBox, Fade(GOLD, 0.08f));
    DrawRectangleLinesEx(selBox, 1.5f / zoom, Fade(GOLD, 0.85f));

    // Corner brackets for clear visual emphasis
    float cornerLen = fminf(selBox.width * 0.25f, 10.0f / zoom);
    float cThick = 2.0f / zoom;
    // Top-Left
    DrawLineEx((Vector2){ selBox.x, selBox.y }, (Vector2){ selBox.x + cornerLen, selBox.y }, cThick, GOLD);
    DrawLineEx((Vector2){ selBox.x, selBox.y }, (Vector2){ selBox.x, selBox.y + cornerLen }, cThick, GOLD);
    // Top-Right
    DrawLineEx((Vector2){ selBox.x + selBox.width, selBox.y }, (Vector2){ selBox.x + selBox.width - cornerLen, selBox.y }, cThick, GOLD);
    DrawLineEx((Vector2){ selBox.x + selBox.width, selBox.y }, (Vector2){ selBox.x + selBox.width, selBox.y + cornerLen }, cThick, GOLD);
    // Bottom-Left
    DrawLineEx((Vector2){ selBox.x, selBox.y + selBox.height }, (Vector2){ selBox.x + cornerLen, selBox.y + selBox.height }, cThick, GOLD);
    DrawLineEx((Vector2){ selBox.x, selBox.y + selBox.height }, (Vector2){ selBox.x, selBox.y + selBox.height - cornerLen }, cThick, GOLD);
    // Bottom-Right
    DrawLineEx((Vector2){ selBox.x + selBox.width, selBox.y + selBox.height }, (Vector2){ selBox.x + selBox.width - cornerLen, selBox.y + selBox.height }, cThick, GOLD);
    DrawLineEx((Vector2){ selBox.x + selBox.width, selBox.y + selBox.height }, (Vector2){ selBox.x + selBox.width, selBox.y + selBox.height - cornerLen }, cThick, GOLD);

    // Interactive resizing handles for supported geometries
    if (el->type == ELEMENT_RECT || el->type == ELEMENT_CIRCLE || el->type == ELEMENT_ELLIPSE || el->type == ELEMENT_TEXT_NOTE) {
        Vector2 localNodes[8];
        GetLocalControlNodePositions(el, localNodes);
        Vector2 worldNodes[8];
        for (int i = 0; i < 8; i++) {
            worldNodes[i] = LocalToWorldPoint(localNodes[i], el->pos, el->rotation);
        }

        Vector2 rotLocal = GetLocalRotationHandlePosition(el, zoom);
        Vector2 rotWorld = LocalToWorldPoint(rotLocal, el->pos, el->rotation);
        Vector2 topCenterWorld = worldNodes[HANDLE_TOP_CENTER];
        DrawLineEx(topCenterWorld, rotWorld, 1.5f / zoom, DARKGRAY);
        DrawCircleV(rotWorld, (HANDLE_SIZE_PX * 0.8f) / zoom, GOLD);
        DrawCircleLines((int)rotWorld.x, (int)rotWorld.y, (HANDLE_SIZE_PX * 0.8f) / zoom, DARKGRAY);

        float side = HANDLE_SIZE_PX / zoom;
        for (int i = 0; i < 8; i++) {
            Rectangle hRect = { worldNodes[i].x - side * 0.5f, worldNodes[i].y - side * 0.5f, side, side };
            DrawRectangleRec(hRect, WHITE);
            DrawRectangleLinesEx(hRect, 1.0f / zoom, BLUE);
        }
    }
}

int TextWidthCallback(mu_Font font, const char *str, int len) {
    float fontSize = (font != NULL) ? (float)(intptr_t)font : 14.0f;
    if (len < 0) len = (int)strlen(str);
    char temp[256];
    if (len >= (int)sizeof(temp)) len = sizeof(temp) - 1;
    memcpy(temp, str, len);
    temp[len] = '\0';
    Font f = (g_CurrentUIFont.texture.id > 0) ? g_CurrentUIFont : GetFontDefault();
    Vector2 size = MeasureTextEx(f, temp, fontSize, 1.0f);
    return (int)ceilf(size.x);
}

int TextHeightCallback(mu_Font font) {
    return (font != NULL) ? (int)(intptr_t)font : 14;
}

static inline Color MuToRaylibColor(mu_Color c) {
    return (Color){ c.r, c.g, c.b, c.a };
}

void RenderMicroui(mu_Context *ctx, Font uiFont) {
    Font f = (uiFont.texture.id > 0) ? uiFont : GetFontDefault();
    mu_Command *cmd = NULL;
    while (mu_next_command(ctx, &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_TEXT: {
                float fSize = (cmd->text.font != NULL) ? (float)(intptr_t)cmd->text.font : 14.0f;
                DrawTextEx(f, cmd->text.str, (Vector2){ (float)cmd->text.pos.x, (float)cmd->text.pos.y }, fSize, 1.0f, MuToRaylibColor(cmd->text.color));
                break;
            }
            case MU_COMMAND_RECT: {
                DrawRectangle(cmd->rect.rect.x, cmd->rect.rect.y, cmd->rect.rect.w, cmd->rect.rect.h, MuToRaylibColor(cmd->rect.color));
                break;
            }
            case MU_COMMAND_ICON: {
                Rectangle r = { (float)cmd->icon.rect.x, (float)cmd->icon.rect.y, (float)cmd->icon.rect.w, (float)cmd->icon.rect.h };
                Color col = MuToRaylibColor(cmd->icon.color);
                int midX = r.x + r.width / 2;
                int midY = r.y + r.height / 2;
                switch (cmd->icon.id) {
                    case MU_ICON_CLOSE:
                        DrawLine(r.x + 4, r.y + 4, r.x + r.width - 4, r.y + r.height - 4, col);
                        DrawLine(r.x + r.width - 4, r.y + 4, r.x + 4, r.y + r.height - 4, col);
                        break;
                    case MU_ICON_CHECK:
                        DrawLine(r.x + 3, midY, midX, r.y + r.height - 4, col);
                        DrawLine(midX, r.y + r.height - 4, r.x + r.width - 3, r.y + 4, col);
                        break;
                    case MU_ICON_COLLAPSED:
                        DrawTriangle((Vector2){ (float)r.x + 4.0f, (float)r.y + 4.0f },
                                     (Vector2){ (float)r.x + 4.0f, (float)r.y + r.height - 4.0f },
                                     (Vector2){ (float)r.x + r.width - 4.0f, (float)midY }, col);
                        break;
                    case MU_ICON_EXPANDED:
                        DrawTriangle((Vector2){ (float)r.x + 4.0f, (float)r.y + 4.0f },
                                     (Vector2){ (float)midX, (float)r.y + r.height - 4.0f },
                                     (Vector2){ (float)r.x + r.width - 4.0f, (float)r.y + 4.0f }, col);
                        break;
                }
                break;
            }
            case MU_COMMAND_CLIP: {
                EndScissorMode();
                if (cmd->clip.rect.w > 0 && cmd->clip.rect.h > 0) {
                    BeginScissorMode(cmd->clip.rect.x, cmd->clip.rect.y, cmd->clip.rect.w, cmd->clip.rect.h);
                }
                break;
            }
        }
    }
    EndScissorMode();
}