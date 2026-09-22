#ifndef COMMANDS_H
#define COMMANDS_H

#include "cad_types.h"

typedef enum {
    CMD_CREATE,
    CMD_DELETE,
    CMD_DELETE_BATCH,
    CMD_MOVE_BATCH,
    CMD_TRANSFORM,
    CMD_LAYER_CHANGE,
    CMD_ORDER_CHANGE,
    CMD_SNAPSHOT
} CommandType;

typedef struct {
    int index;
    GridElement element;
} IndexedElement;

typedef struct {
    CommandType type;
    union {
        struct {
            int index;
            GridElement element;
        } create;
        struct {
            int index;
            GridElement element;
        } del;
        struct {
            IndexedElement *items;
            int count;
        } deleteBatch;
        struct {
            int *indices;
            int count;
            Vector2 delta;
        } moveBatch;
        struct {
            int index;
            GridElement before;
            GridElement after;
        } transform;
        struct {
            int index;
            int oldLayer;
            int newLayer;
        } layerChange;
        struct {
            int oldIndex;
            int newIndex;
        } orderChange;
        struct {
            ProjectState before;
            ProjectState after;
        } snapshot;
    } data;
} Command;

typedef struct {
    Command commands[MAX_COMMAND_HISTORY];
    int count;
    int currentIndex;
} CommandHistory;

void FreeCommand(Command *cmd);
void ClearCommandHistory(CommandHistory *history);
void ExecuteCommand(CommandHistory *history, Command cmd, GridElement *elements, int *elementCount, Layer *layers, int *layerCount, bool *spatialIndexDirty);
bool History_Undo(CommandHistory *history, GridElement *elements, int *elementCount, Layer *layers, int *layerCount, bool *spatialIndexDirty);
bool History_Redo(CommandHistory *history, GridElement *elements, int *elementCount, Layer *layers, int *layerCount, bool *spatialIndexDirty);

#endif // COMMANDS_H