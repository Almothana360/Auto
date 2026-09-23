#include "cad_selection.h"
#include "cad_math.h"
#include "spatial_tree.h"
#include "project_io.h"
#include "commands.h"
#include "render_utils.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>

void UpdateSelectionAndHandles(AppContext *app, Vector2 activeToolPoint, bool overUI, int selectedCount) {
    bool isCtrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    app->hasSnapX = false;
    app->hasSnapY = false;

    if (!overUI && g_CADState.activeTool == TOOL_SELECT) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            HandleType hTested = HANDLE_NONE;
            int hIdx = -1;
            if (selectedCount == 1) {
                int sIdx = GetFirstSelectedIndex(app->elements, app->elementCount);
                hTested = HitTestHandles(&app->elements[sIdx], g_CADState.mouseWorld, app->camera.zoom);
                if (hTested != HANDLE_NONE) hIdx = sIdx;
            }
            if (hTested != HANDLE_NONE && hIdx != -1) {
                app->activeHandle = hTested;
                app->activeHandleElementIdx = hIdx;
                app->initialHandleElementState = app->elements[hIdx];
            } else {
                int hitIndex = HitTestElement_Spatial(app->spatialTree, app->elements, app->elementCount, app->layers, app->layerCount, g_CADState.mouseWorld);
                if (hitIndex != -1) {
                    if (isCtrlDown) app->elements[hitIndex].selected = !app->elements[hitIndex].selected;
                    else if (!app->elements[hitIndex].selected) {
                        DeselectAllElements(app->elements, app->elementCount);
                        app->elements[hitIndex].selected = true;
                    }
                    app->isDraggingElement = true;
                    app->dragStartWorldPos = g_CADState.mouseWorld;
                    for (int i = 0; i < app->elementCount; i++) {
                        if (app->elements[i].selected) {
                            app->elementStartStates[i] = app->elements[i];
                        }
                    }
                } else {
                    if (!isCtrlDown) DeselectAllElements(app->elements, app->elementCount);
                    app->isBoxSelecting = true;
                    app->boxStartWorldPos = g_CADState.mouseWorld;
                    app->boxCurrentWorldPos = g_CADState.mouseWorld;
                }
            }
        } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (app->activeHandle != HANDLE_NONE && app->activeHandleElementIdx >= 0) {
                GridElement *el = &app->elements[app->activeHandleElementIdx];
                Vector2 localMouse = WorldToLocalPoint(activeToolPoint, app->initialHandleElementState.pos, app->initialHandleElementState.rotation);

                if (app->activeHandle == HANDLE_ROTATION) {
                    Vector2 diff = Vector2Subtract(activeToolPoint, el->pos);
                    float angleRad = atan2f(diff.y, diff.x);
                    float angleDeg = angleRad * RAD2DEG + 90.0f;
                    bool snapShift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) || app->snapToGrid || app->snapEnabled;
                    el->rotation = SnapAngle(angleDeg, snapShift);
                } else if (el->type == ELEMENT_RECT || el->type == ELEMENT_TEXT_NOTE) {
                    float origW = app->initialHandleElementState.width;
                    float origH = app->initialHandleElementState.height;
                    float halfW = origW * 0.5f;
                    float halfH = origH * 0.5f;
                    float left = -halfW, right = halfW, top = -halfH, bottom = halfH;
                    switch (app->activeHandle) {
                        case HANDLE_TOP_LEFT: left = localMouse.x; top = localMouse.y; break;
                        case HANDLE_TOP_CENTER: top = localMouse.y; break;
                        case HANDLE_TOP_RIGHT: right = localMouse.x; top = localMouse.y; break;
                        case HANDLE_RIGHT_CENTER: right = localMouse.x; break;
                        case HANDLE_BOTTOM_RIGHT: right = localMouse.x; bottom = localMouse.y; break;
                        case HANDLE_BOTTOM_CENTER: bottom = localMouse.y; break;
                        case HANDLE_BOTTOM_LEFT: left = localMouse.x; bottom = localMouse.y; break;
                        case HANDLE_LEFT_CENTER: left = localMouse.x; break;
                        default: break;
                    }
                    if (right - left < MIN_ELEMENT_SIZE) {
                        if (app->activeHandle == HANDLE_LEFT_CENTER || app->activeHandle == HANDLE_TOP_LEFT || app->activeHandle == HANDLE_BOTTOM_LEFT) left = right - MIN_ELEMENT_SIZE;
                        else right = left + MIN_ELEMENT_SIZE;
                    }
                    if (bottom - top < MIN_ELEMENT_SIZE) {
                        if (app->activeHandle == HANDLE_TOP_CENTER || app->activeHandle == HANDLE_TOP_LEFT || app->activeHandle == HANDLE_TOP_RIGHT) top = bottom - MIN_ELEMENT_SIZE;
                        else bottom = top + MIN_ELEMENT_SIZE;
                    }
                    float newW = right - left;
                    float newH = bottom - top;
                    Vector2 localCenter = { (left + right) * 0.5f, (top + bottom) * 0.5f };
                    el->width = newW;
                    el->height = newH;
                    el->pos = LocalToWorldPoint(localCenter, app->initialHandleElementState.pos, app->initialHandleElementState.rotation);
                } else if (el->type == ELEMENT_CIRCLE) {
                    float dist = Vector2Length(localMouse);
                    if (dist < MIN_ELEMENT_SIZE) dist = MIN_ELEMENT_SIZE;
                    el->radius = dist;
                } else if (el->type == ELEMENT_ELLIPSE) {
                    float distPointX = fabsf(localMouse.x);
                    float distPointY = fabsf(localMouse.y);
                    if (distPointX > MIN_ELEMENT_SIZE) el->radiusX = distPointX;
                    if (distPointY > MIN_ELEMENT_SIZE) el->radiusY = distPointY;
                }
                GetElementAABB(el);
                app->spatialIndexDirty = true;
            } else if (app->isBoxSelecting) {
                app->boxCurrentWorldPos = g_CADState.mouseWorld;
                float minX = fminf(app->boxStartWorldPos.x, app->boxCurrentWorldPos.x);
                float maxX = fmaxf(app->boxStartWorldPos.x, app->boxCurrentWorldPos.x);
                float minY = fminf(app->boxStartWorldPos.y, app->boxCurrentWorldPos.y);
                float maxY = fmaxf(app->boxStartWorldPos.y, app->boxCurrentWorldPos.y);
                AABB selBox = { { minX, minY }, { maxX, maxY } };
                for (int i = 0; i < app->elementCount; i++) {
                    if (!app->layers[app->elements[i].layerIndex].visible || app->layers[app->elements[i].layerIndex].locked) continue;
                    if (AABBIntersectsAABB(app->cachedAABBs[i], selBox)) {
                        Vector2 p = app->elements[i].pos;
                        if (app->elements[i].type == ELEMENT_DIMENSION) p = app->elements[i].dimPos;
                        else if (app->elements[i].type == ELEMENT_LINE) p = (Vector2){ (app->elements[i].p1.x + app->elements[i].p2.x)*0.5f, (app->elements[i].p1.y + app->elements[i].p2.y)*0.5f };
                        if (AABBContainsPoint(selBox, p)) app->elements[i].selected = true;
                    }
                }
            } else if (app->isDraggingElement && selectedCount > 0) {
                Vector2 rawMouseDelta = { g_CADState.mouseWorld.x - app->dragStartWorldPos.x, g_CADState.mouseWorld.y - app->dragStartWorldPos.y };
                Vector2 mouseDelta = rawMouseDelta;

                // Snapping calculations during drag
                if ((app->snapEnabled || app->snapToGrid) && selectedCount == 1) {
                    int sIdx = GetFirstSelectedIndex(app->elements, app->elementCount);
                    GridElement tempEl = app->elements[sIdx];
                    GridElement *start = &app->elementStartStates[sIdx];
                    if (tempEl.type == ELEMENT_DIMENSION) {
                        tempEl.dimPos.x = start->dimPos.x + rawMouseDelta.x;
                        tempEl.dimPos.y = start->dimPos.y + rawMouseDelta.y;
                    } else if (tempEl.type == ELEMENT_LINE) {
                        tempEl.p1.x = start->p1.x + rawMouseDelta.x;
                        tempEl.p1.y = start->p1.y + rawMouseDelta.y;
                        tempEl.p2.x = start->p2.x + rawMouseDelta.x;
                        tempEl.p2.y = start->p2.y + rawMouseDelta.y;
                    } else if (tempEl.type == ELEMENT_POLYLINE || tempEl.type == ELEMENT_FREEHAND) {
                        for (int p = 0; p < tempEl.pointCount; p++) {
                            tempEl.points[p].x = start->points[p].x + rawMouseDelta.x;
                            tempEl.points[p].y = start->points[p].y + rawMouseDelta.y;
                        }
                    } else if (tempEl.type == ELEMENT_ARC) {
                        tempEl.pos.x = start->pos.x + rawMouseDelta.x;
                        tempEl.pos.y = start->pos.y + rawMouseDelta.y;
                        tempEl.p1.x = start->p1.x + rawMouseDelta.x;
                        tempEl.p1.y = start->p1.y + rawMouseDelta.y;
                        tempEl.p2.x = start->p2.x + rawMouseDelta.x;
                        tempEl.p2.y = start->p2.y + rawMouseDelta.y;
                        tempEl.p3.x = start->p3.x + rawMouseDelta.x;
                        tempEl.p3.y = start->p3.y + rawMouseDelta.y;
                    } else {
                        tempEl.pos.x = start->pos.x + rawMouseDelta.x;
                        tempEl.pos.y = start->pos.y + rawMouseDelta.y;
                    }

                    float myLinesX[8], myLinesY[8];
                    int myCntX = 0, myCntY = 0;
                    GetElementSnapLines(&tempEl, myLinesX, &myCntX, myLinesY, &myCntY);

                    float bestDx = app->snapThreshold / app->camera.zoom;
                    float bestDy = app->snapThreshold / app->camera.zoom;
                    float applyDx = 0.0f, applyDy = 0.0f;

                    // 1. Element-to-element snapping
                    if (app->snapEnabled) {
                        AABB dragAABB = ExpandAABB(GetElementAABB(&tempEl), app->snapThreshold / app->camera.zoom);
                        for (int i = 0; i < app->elementCount; i++) {
                            if (i == sIdx || !app->layers[app->elements[i].layerIndex].visible) continue;
                            if (!AABBIntersectsAABB(app->cachedAABBs[i], dragAABB)) continue;

                            float tLinesX[8], tLinesY[8];
                            int tCntX = 0, tCntY = 0;
                            GetElementSnapLines(&app->elements[i], tLinesX, &tCntX, tLinesY, &tCntY);

                            for (int m = 0; m < myCntX; m++) {
                                for (int t = 0; t < tCntX; t++) {
                                    float dx = tLinesX[t] - myLinesX[m];
                                    if (fabsf(dx) < fabsf(bestDx)) {
                                        bestDx = dx;
                                        applyDx = dx;
                                        app->hasSnapX = true;
                                        app->snapXVal = tLinesX[t];
                                    }
                                }
                            }
                            for (int m = 0; m < myCntY; m++) {
                                for (int t = 0; t < tCntY; t++) {
                                    float dy = tLinesY[t] - myLinesY[m];
                                    if (fabsf(dy) < fabsf(bestDy)) {
                                        bestDy = dy;
                                        applyDy = dy;
                                        app->hasSnapY = true;
                                        app->snapYVal = tLinesY[t];
                                    }
                                }
                            }
                        }
                    }

                    // 2. Grid snapping
                    if (app->snapToGrid) {
                        for (int m = 0; m < myCntX; m++) {
                            float gridX = roundf(myLinesX[m] / app->gridSpacing) * app->gridSpacing;
                            float dx = gridX - myLinesX[m];
                            if (fabsf(dx) < fabsf(bestDx)) {
                                bestDx = dx;
                                applyDx = dx;
                                app->hasSnapX = true;
                                app->snapXVal = gridX;
                            }
                        }
                        for (int m = 0; m < myCntY; m++) {
                            float gridY = roundf(myLinesY[m] / app->gridSpacing) * app->gridSpacing;
                            float dy = gridY - myLinesY[m];
                            if (fabsf(dy) < fabsf(bestDy)) {
                                bestDy = dy;
                                applyDy = dy;
                                app->hasSnapY = true;
                                app->snapYVal = gridY;
                            }
                        }
                    }

                    mouseDelta.x += applyDx;
                    mouseDelta.y += applyDy;
                }

                for (int i = 0; i < app->elementCount; i++) {
                    if (!app->elements[i].selected) continue;
                    GridElement *curr = &app->elements[i];
                    GridElement *start = &app->elementStartStates[i];
                    if (curr->type == ELEMENT_DIMENSION) {
                        curr->dimPos.x = start->dimPos.x + mouseDelta.x;
                        curr->dimPos.y = start->dimPos.y + mouseDelta.y;
                    } else if (curr->type == ELEMENT_LINE) {
                        curr->p1.x = start->p1.x + mouseDelta.x;
                        curr->p1.y = start->p1.y + mouseDelta.y;
                        curr->p2.x = start->p2.x + mouseDelta.x;
                        curr->p2.y = start->p2.y + mouseDelta.y;
                    } else if (curr->type == ELEMENT_POLYLINE || curr->type == ELEMENT_FREEHAND) {
                        for (int p = 0; p < curr->pointCount; p++) {
                            curr->points[p].x = start->points[p].x + mouseDelta.x;
                            curr->points[p].y = start->points[p].y + mouseDelta.y;
                        }
                    } else if (curr->type == ELEMENT_ARC) {
                        curr->pos.x = start->pos.x + mouseDelta.x;
                        curr->pos.y = start->pos.y + mouseDelta.y;
                        curr->p1.x = start->p1.x + mouseDelta.x;
                        curr->p1.y = start->p1.y + mouseDelta.y;
                        curr->p2.x = start->p2.x + mouseDelta.x;
                        curr->p2.y = start->p2.y + mouseDelta.y;
                        curr->p3.x = start->p3.x + mouseDelta.x;
                        curr->p3.y = start->p3.y + mouseDelta.y;
                    } else {
                        curr->pos.x = start->pos.x + mouseDelta.x;
                        curr->pos.y = start->pos.y + mouseDelta.y;
                    }
                    GetElementAABB(curr);
                }
                app->spatialIndexDirty = true;
            }
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (app->activeHandle != HANDLE_NONE && app->activeHandleElementIdx >= 0) {
            Command cmd = { 0 };
            cmd.type = CMD_TRANSFORM;
            cmd.data.transform.index = app->activeHandleElementIdx;
            cmd.data.transform.before = app->initialHandleElementState;
            cmd.data.transform.after = app->elements[app->activeHandleElementIdx];
            ExecuteCommand(app->cmdHistory, cmd, app->elements, &app->elementCount, app->layers, &app->layerCount, &app->spatialIndexDirty);
        } else if (app->isDraggingElement && selectedCount > 0) {
            int selTotal = 0;
            for (int i = 0; i < app->elementCount; i++) if (app->elements[i].selected) selTotal++;
            int firstIdx = GetFirstSelectedIndex(app->elements, app->elementCount);
            Vector2 diff = Vector2Subtract(app->elements[firstIdx].pos, app->elementStartStates[firstIdx].pos);
            if (app->elements[firstIdx].type == ELEMENT_LINE) {
                diff = Vector2Subtract(app->elements[firstIdx].p1, app->elementStartStates[firstIdx].p1);
            } else if (app->elements[firstIdx].type == ELEMENT_DIMENSION) {
                diff = Vector2Subtract(app->elements[firstIdx].dimPos, app->elementStartStates[firstIdx].dimPos);
            }
            if (Vector2LengthSqr(diff) > 0.0001f) {
                Command moveCmd = { 0 };
                moveCmd.type = CMD_MOVE_BATCH;
                moveCmd.data.moveBatch.count = selTotal;
                moveCmd.data.moveBatch.delta = diff;
                moveCmd.data.moveBatch.indices = (int*)malloc(sizeof(int) * selTotal);
                int ins = 0;
                for (int i = 0; i < app->elementCount; i++) {
                    if (app->elements[i].selected) {
                        moveCmd.data.moveBatch.indices[ins++] = i;
                    }
                }
                if (app->cmdHistory->currentIndex < app->cmdHistory->count) {
                    for (int i = app->cmdHistory->currentIndex; i < app->cmdHistory->count; i++) FreeCommand(&app->cmdHistory->commands[i]);
                    app->cmdHistory->count = app->cmdHistory->currentIndex;
                }
                if (app->cmdHistory->count >= MAX_COMMAND_HISTORY) {
                    FreeCommand(&app->cmdHistory->commands[0]);
                    for (int i = 0; i < MAX_COMMAND_HISTORY - 1; i++) app->cmdHistory->commands[i] = app->cmdHistory->commands[i + 1];
                    app->cmdHistory->count = MAX_COMMAND_HISTORY - 1;
                    app->cmdHistory->currentIndex = app->cmdHistory->count;
                }
                app->cmdHistory->commands[app->cmdHistory->currentIndex++] = moveCmd;
                app->cmdHistory->count = app->cmdHistory->currentIndex;
            }
        }
        app->isDraggingElement = false;
        app->isBoxSelecting = false;
        app->activeHandle = HANDLE_NONE;
        app->activeHandleElementIdx = -1;
    }
}

void RenderSelectionGizmos(AppContext *app) {
    for (int i = 0; i < app->elementCount; i++) {
        if (app->elements[i].selected) {
            DrawElementSelectionGizmo(&app->elements[i], app->camera.zoom);
        }
    }
    if (app->isBoxSelecting) {
        float minX = fminf(app->boxStartWorldPos.x, app->boxCurrentWorldPos.x);
        float minY = fminf(app->boxStartWorldPos.y, app->boxCurrentWorldPos.y);
        float width = fabsf(app->boxCurrentWorldPos.x - app->boxStartWorldPos.x);
        float height = fabsf(app->boxCurrentWorldPos.y - app->boxStartWorldPos.y);
        Rectangle box = { minX, minY, width, height };
        DrawRectangleRec(box, Fade(SKYBLUE, 0.2f));
        DrawRectangleLinesEx(box, 1.0f / app->camera.zoom, BLUE);
    }
    if (app->isDraggingElement && (app->snapEnabled || app->snapToGrid)) {
        if (app->hasSnapX) DrawLineEx((Vector2){ app->snapXVal, -100000.0f }, (Vector2){ app->snapXVal, 100000.0f }, 1.0f / app->camera.zoom, MAGENTA);
        if (app->hasSnapY) DrawLineEx((Vector2){ -100000.0f, app->snapYVal }, (Vector2){ 100000.0f, app->snapYVal }, 1.0f / app->camera.zoom, MAGENTA);
    }
}