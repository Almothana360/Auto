#include "commands.h"
#include "cad_math.h"
#include "layer.h"
#include "raymath.h"
#include <stdlib.h>
#include <string.h>

void FreeCommand(Command *cmd) {
    if (!cmd) return;
    if (cmd->type == CMD_DELETE_BATCH) {
        if (cmd->data.deleteBatch.items) {
            free(cmd->data.deleteBatch.items);
            cmd->data.deleteBatch.items = NULL;
        }
        cmd->data.deleteBatch.count = 0;
    } else if (cmd->type == CMD_MOVE_BATCH) {
        if (cmd->data.moveBatch.indices) {
            free(cmd->data.moveBatch.indices);
            cmd->data.moveBatch.indices = NULL;
        }
        cmd->data.moveBatch.count = 0;
    } else if (cmd->type == CMD_SNAPSHOT) {
        if (cmd->data.snapshot.before.elements) {
            free(cmd->data.snapshot.before.elements);
            cmd->data.snapshot.before.elements = NULL;
        }
        for (int l = 0; l < cmd->data.snapshot.before.layerCount; l++) {
            Layer_Free(&cmd->data.snapshot.before.layers[l]);
        }
        if (cmd->data.snapshot.after.elements) {
            free(cmd->data.snapshot.after.elements);
            cmd->data.snapshot.after.elements = NULL;
        }
        for (int l = 0; l < cmd->data.snapshot.after.layerCount; l++) {
            Layer_Free(&cmd->data.snapshot.after.layers[l]);
        }
    }
}

void ClearCommandHistory(CommandHistory *history) {
    if (!history) return;
    for (int i = 0; i < history->count; i++) {
        FreeCommand(&history->commands[i]);
    }
    history->count = 0;
    history->currentIndex = 0;
}

void ExecuteCommand(CommandHistory *history, Command cmd, GridElement *elements, int *elementCount, Layer *layers, int *layerCount, bool *spatialIndexDirty) {
    if (history->currentIndex < history->count) {
        for (int i = history->currentIndex; i < history->count; i++) {
            FreeCommand(&history->commands[i]);
        }
        history->count = history->currentIndex;
    }

    if (history->count >= MAX_COMMAND_HISTORY) {
        FreeCommand(&history->commands[0]);
        for (int i = 0; i < MAX_COMMAND_HISTORY - 1; i++) {
            history->commands[i] = history->commands[i + 1];
        }
        history->count = MAX_COMMAND_HISTORY - 1;
        history->currentIndex = history->count;
    }

    switch (cmd.type) {
        case CMD_CREATE: {
            if (cmd.data.create.index >= 0 && cmd.data.create.index <= *elementCount && *elementCount < MAX_ELEMENTS) {
                for (int i = *elementCount; i > cmd.data.create.index; i--) {
                    elements[i] = elements[i - 1];
                }
                elements[cmd.data.create.index] = cmd.data.create.element;
                if (elements[cmd.data.create.index].id == 0) {
                    elements[cmd.data.create.index].id = GenerateEntityID();
                }
                GetElementAABB(&elements[cmd.data.create.index]);
                (*elementCount)++;
                int lIdx = elements[cmd.data.create.index].layerIndex;
                if (lIdx >= 0 && lIdx < *layerCount) {
                    Layer_AddEntityId(&layers[lIdx], elements[cmd.data.create.index].id);
                }
            }
            break;
        }
        case CMD_DELETE: {
            int idx = cmd.data.del.index;
            if (idx >= 0 && idx < *elementCount) {
                int lIdx = elements[idx].layerIndex;
                if (lIdx >= 0 && lIdx < *layerCount) {
                    Layer_RemoveEntityId(&layers[lIdx], elements[idx].id);
                }
                for (int i = idx; i < *elementCount - 1; i++) {
                    elements[i] = elements[i + 1];
                }
                (*elementCount)--;
            }
            break;
        }
        case CMD_DELETE_BATCH: {
            for (int k = cmd.data.deleteBatch.count - 1; k >= 0; k--) {
                int idx = cmd.data.deleteBatch.items[k].index;
                if (idx >= 0 && idx < *elementCount) {
                    int lIdx = elements[idx].layerIndex;
                    if (lIdx >= 0 && lIdx < *layerCount) {
                        Layer_RemoveEntityId(&layers[lIdx], elements[idx].id);
                    }
                    for (int i = idx; i < *elementCount - 1; i++) {
                        elements[i] = elements[i + 1];
                    }
                    (*elementCount)--;
                }
            }
            break;
        }
        case CMD_MOVE_BATCH: {
            Vector2 d = cmd.data.moveBatch.delta;
            for (int k = 0; k < cmd.data.moveBatch.count; k++) {
                int idx = cmd.data.moveBatch.indices[k];
                if (idx < 0 || idx >= *elementCount) continue;
                GridElement *el = &elements[idx];
                if (el->type == ELEMENT_DIMENSION) {
                    el->dimPos = Vector2Add(el->dimPos, d);
                } else if (el->type == ELEMENT_LINE) {
                    el->p1 = Vector2Add(el->p1, d);
                    el->p2 = Vector2Add(el->p2, d);
                } else if (el->type == ELEMENT_POLYLINE || el->type == ELEMENT_FREEHAND) {
                    for (int p = 0; p < el->pointCount; p++) {
                        el->points[p] = Vector2Add(el->points[p], d);
                    }
                } else if (el->type == ELEMENT_ARC) {
                    el->pos = Vector2Add(el->pos, d);
                    el->p1 = Vector2Add(el->p1, d);
                    el->p2 = Vector2Add(el->p2, d);
                    el->p3 = Vector2Add(el->p3, d);
                } else {
                    el->pos = Vector2Add(el->pos, d);
                }
                GetElementAABB(el);
            }
            break;
        }
        case CMD_TRANSFORM: {
            int idx = cmd.data.transform.index;
            if (idx >= 0 && idx < *elementCount) {
                elements[idx] = cmd.data.transform.after;
                GetElementAABB(&elements[idx]);
            }
            break;
        }
        case CMD_LAYER_CHANGE: {
            int idx = cmd.data.layerChange.index;
            if (idx >= 0 && idx < *elementCount) {
                int oldL = elements[idx].layerIndex;
                int newL = cmd.data.layerChange.newLayer;
                if (oldL >= 0 && oldL < *layerCount) Layer_RemoveEntityId(&layers[oldL], elements[idx].id);
                if (newL >= 0 && newL < *layerCount) Layer_AddEntityId(&layers[newL], elements[idx].id);
                elements[idx].layerIndex = newL;
            }
            break;
        }
        case CMD_ORDER_CHANGE: {
            int from = cmd.data.orderChange.oldIndex;
            int to = cmd.data.orderChange.newIndex;
            if (from >= 0 && from < *elementCount && to >= 0 && to < *elementCount && from != to) {
                GridElement tmp = elements[from];
                if (from < to) {
                    for (int i = from; i < to; i++) elements[i] = elements[i + 1];
                } else {
                    for (int i = from; i > to; i--) elements[i] = elements[i - 1];
                }
                elements[to] = tmp;
            }
            break;
        }
        case CMD_SNAPSHOT: {
            break;
        }
    }

    history->commands[history->currentIndex++] = cmd;
    history->count = history->currentIndex;
    *spatialIndexDirty = true;
}

bool History_Undo(CommandHistory *history, GridElement *elements, int *elementCount, Layer *layers, int *layerCount, bool *spatialIndexDirty) {
    if (history->currentIndex <= 0) return false;
    history->currentIndex--;
    Command *cmd = &history->commands[history->currentIndex];

    switch (cmd->type) {
        case CMD_CREATE: {
            int idx = cmd->data.create.index;
            if (idx >= 0 && idx < *elementCount) {
                int lIdx = elements[idx].layerIndex;
                if (lIdx >= 0 && lIdx < *layerCount) Layer_RemoveEntityId(&layers[lIdx], elements[idx].id);
                for (int i = idx; i < *elementCount - 1; i++) {
                    elements[i] = elements[i + 1];
                }
                (*elementCount)--;
            }
            break;
        }
        case CMD_DELETE: {
            int idx = cmd->data.del.index;
            if (idx >= 0 && idx <= *elementCount && *elementCount < MAX_ELEMENTS) {
                for (int i = *elementCount; i > idx; i--) {
                    elements[i] = elements[i - 1];
                }
                elements[idx] = cmd->data.del.element;
                GetElementAABB(&elements[idx]);
                (*elementCount)++;
                int lIdx = elements[idx].layerIndex;
                if (lIdx >= 0 && lIdx < *layerCount) Layer_AddEntityId(&layers[lIdx], elements[idx].id);
            }
            break;
        }
        case CMD_DELETE_BATCH: {
            for (int k = 0; k < cmd->data.deleteBatch.count; k++) {
                int idx = cmd->data.deleteBatch.items[k].index;
                if (idx >= 0 && idx <= *elementCount && *elementCount < MAX_ELEMENTS) {
                    for (int i = *elementCount; i > idx; i--) {
                        elements[i] = elements[i - 1];
                    }
                    elements[idx] = cmd->data.deleteBatch.items[k].element;
                    GetElementAABB(&elements[idx]);
                    (*elementCount)++;
                    int lIdx = elements[idx].layerIndex;
                    if (lIdx >= 0 && lIdx < *layerCount) Layer_AddEntityId(&layers[lIdx], elements[idx].id);
                }
            }
            break;
        }
        case CMD_MOVE_BATCH: {
            Vector2 invD = Vector2Negate(cmd->data.moveBatch.delta);
            for (int k = 0; k < cmd->data.moveBatch.count; k++) {
                int idx = cmd->data.moveBatch.indices[k];
                if (idx < 0 || idx >= *elementCount) continue;
                GridElement *el = &elements[idx];
                if (el->type == ELEMENT_DIMENSION) {
                    el->dimPos = Vector2Add(el->dimPos, invD);
                } else if (el->type == ELEMENT_LINE) {
                    el->p1 = Vector2Add(el->p1, invD);
                    el->p2 = Vector2Add(el->p2, invD);
                } else if (el->type == ELEMENT_POLYLINE || el->type == ELEMENT_FREEHAND) {
                    for (int p = 0; p < el->pointCount; p++) {
                        el->points[p] = Vector2Add(el->points[p], invD);
                    }
                } else if (el->type == ELEMENT_ARC) {
                    el->pos = Vector2Add(el->pos, invD);
                    el->p1 = Vector2Add(el->p1, invD);
                    el->p2 = Vector2Add(el->p2, invD);
                    el->p3 = Vector2Add(el->p3, invD);
                } else {
                    el->pos = Vector2Add(el->pos, invD);
                }
                GetElementAABB(el);
            }
            break;
        }
        case CMD_TRANSFORM: {
            int idx = cmd->data.transform.index;
            if (idx >= 0 && idx < *elementCount) {
                elements[idx] = cmd->data.transform.before;
                GetElementAABB(&elements[idx]);
            }
            break;
        }
        case CMD_LAYER_CHANGE: {
            int idx = cmd->data.layerChange.index;
            if (idx >= 0 && idx < *elementCount) {
                int curL = elements[idx].layerIndex;
                int origL = cmd->data.layerChange.oldLayer;
                if (curL >= 0 && curL < *layerCount) Layer_RemoveEntityId(&layers[curL], elements[idx].id);
                if (origL >= 0 && origL < *layerCount) Layer_AddEntityId(&layers[origL], elements[idx].id);
                elements[idx].layerIndex = origL;
            }
            break;
        }
        case CMD_ORDER_CHANGE: {
            int from = cmd->data.orderChange.newIndex;
            int to = cmd->data.orderChange.oldIndex;
            if (from >= 0 && from < *elementCount && to >= 0 && to < *elementCount && from != to) {
                GridElement tmp = elements[from];
                if (from < to) {
                    for (int i = from; i < to; i++) elements[i] = elements[i + 1];
                } else {
                    for (int i = from; i > to; i--) elements[i] = elements[i - 1];
                }
                elements[to] = tmp;
            }
            break;
        }
        case CMD_SNAPSHOT: {
            *elementCount = cmd->data.snapshot.before.elementCount;
            memcpy(elements, cmd->data.snapshot.before.elements, sizeof(GridElement) * (*elementCount));
            for (int l = 0; l < *layerCount; l++) Layer_Free(&layers[l]);
            *layerCount = cmd->data.snapshot.before.layerCount;
            for (int l = 0; l < *layerCount; l++) {
                Layer_Copy(&layers[l], &cmd->data.snapshot.before.layers[l]);
            }
            for (int i = 0; i < *elementCount; i++) GetElementAABB(&elements[i]);
            break;
        }
    }

    *spatialIndexDirty = true;
    return true;
}

bool History_Redo(CommandHistory *history, GridElement *elements, int *elementCount, Layer *layers, int *layerCount, bool *spatialIndexDirty) {
    if (history->currentIndex >= history->count) return false;
    Command *cmd = &history->commands[history->currentIndex];
    history->currentIndex++;

    switch (cmd->type) {
        case CMD_CREATE: {
            int idx = cmd->data.create.index;
            if (idx >= 0 && idx <= *elementCount && *elementCount < MAX_ELEMENTS) {
                for (int i = *elementCount; i > idx; i--) {
                    elements[i] = elements[i - 1];
                }
                elements[idx] = cmd->data.create.element;
                GetElementAABB(&elements[idx]);
                (*elementCount)++;
                int lIdx = elements[idx].layerIndex;
                if (lIdx >= 0 && lIdx < *layerCount) Layer_AddEntityId(&layers[lIdx], elements[idx].id);
            }
            break;
        }
        case CMD_DELETE: {
            int idx = cmd->data.del.index;
            if (idx >= 0 && idx < *elementCount) {
                int lIdx = elements[idx].layerIndex;
                if (lIdx >= 0 && lIdx < *layerCount) Layer_RemoveEntityId(&layers[lIdx], elements[idx].id);
                for (int i = idx; i < *elementCount - 1; i++) {
                    elements[i] = elements[i + 1];
                }
                (*elementCount)--;
            }
            break;
        }
        case CMD_DELETE_BATCH: {
            for (int k = cmd->data.deleteBatch.count - 1; k >= 0; k--) {
                int idx = cmd->data.deleteBatch.items[k].index;
                if (idx >= 0 && idx < *elementCount) {
                    int lIdx = elements[idx].layerIndex;
                    if (lIdx >= 0 && lIdx < *layerCount) Layer_RemoveEntityId(&layers[lIdx], elements[idx].id);
                    for (int i = idx; i < *elementCount - 1; i++) {
                        elements[i] = elements[i + 1];
                    }
                    (*elementCount)--;
                }
            }
            break;
        }
        case CMD_MOVE_BATCH: {
            Vector2 d = cmd->data.moveBatch.delta;
            for (int k = 0; k < cmd->data.moveBatch.count; k++) {
                int idx = cmd->data.moveBatch.indices[k];
                if (idx < 0 || idx >= *elementCount) continue;
                GridElement *el = &elements[idx];
                if (el->type == ELEMENT_DIMENSION) {
                    el->dimPos = Vector2Add(el->dimPos, d);
                } else if (el->type == ELEMENT_LINE) {
                    el->p1 = Vector2Add(el->p1, d);
                    el->p2 = Vector2Add(el->p2, d);
                } else if (el->type == ELEMENT_POLYLINE || el->type == ELEMENT_FREEHAND) {
                    for (int p = 0; p < el->pointCount; p++) {
                        el->points[p] = Vector2Add(el->points[p], d);
                    }
                } else if (el->type == ELEMENT_ARC) {
                    el->pos = Vector2Add(el->pos, d);
                    el->p1 = Vector2Add(el->p1, d);
                    el->p2 = Vector2Add(el->p2, d);
                    el->p3 = Vector2Add(el->p3, d);
                } else {
                    el->pos = Vector2Add(el->pos, d);
                }
                GetElementAABB(el);
            }
            break;
        }
        case CMD_TRANSFORM: {
            int idx = cmd->data.transform.index;
            if (idx >= 0 && idx < *elementCount) {
                elements[idx] = cmd->data.transform.after;
                GetElementAABB(&elements[idx]);
            }
            break;
        }
        case CMD_LAYER_CHANGE: {
            int idx = cmd->data.layerChange.index;
            if (idx >= 0 && idx < *elementCount) {
                int oldL = elements[idx].layerIndex;
                int newL = cmd->data.layerChange.newLayer;
                if (oldL >= 0 && oldL < *layerCount) Layer_RemoveEntityId(&layers[oldL], elements[idx].id);
                if (newL >= 0 && newL < *layerCount) Layer_AddEntityId(&layers[newL], elements[idx].id);
                elements[idx].layerIndex = newL;
            }
            break;
        }
        case CMD_ORDER_CHANGE: {
            int from = cmd->data.orderChange.oldIndex;
            int to = cmd->data.orderChange.newIndex;
            if (from >= 0 && from < *elementCount && to >= 0 && to < *elementCount && from != to) {
                GridElement tmp = elements[from];
                if (from < to) {
                    for (int i = from; i < to; i++) elements[i] = elements[i + 1];
                } else {
                    for (int i = from; i > to; i--) elements[i] = elements[i - 1];
                }
                elements[to] = tmp;
            }
            break;
        }
        case CMD_SNAPSHOT: {
            *elementCount = cmd->data.snapshot.after.elementCount;
            memcpy(elements, cmd->data.snapshot.after.elements, sizeof(GridElement) * (*elementCount));
            for (int l = 0; l < *layerCount; l++) Layer_Free(&layers[l]);
            *layerCount = cmd->data.snapshot.after.layerCount;
            for (int l = 0; l < *layerCount; l++) {
                Layer_Copy(&layers[l], &cmd->data.snapshot.after.layers[l]);
            }
            for (int i = 0; i < *elementCount; i++) GetElementAABB(&elements[i]);
            break;
        }
    }

    *spatialIndexDirty = true;
    return true;
}