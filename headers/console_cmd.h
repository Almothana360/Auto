#ifndef CONSOLE_CMD_H
#define CONSOLE_CMD_H

#include "cad_types.h"
#include "commands.h"

void ProcessCommand(const char *cmdStr, GridElement *elements, int *elementCount, 
                    Layer *layers, int *layerCount, int *activeLayerIndex, 
                    CADTool *currentTool, Camera2D *camera, bool *showHudPanel, 
                    bool *showInspector, bool *showLayersPanel, bool *showElementsPanel, 
                    float *uiScale, char *statusMessage, float *statusMessageTimer, 
                    int *dimStep, CommandHistory *cmdHistory, bool *spatialIndexDirty, 
                    MeasureUnit currentUnit);

#endif // CONSOLE_CMD_H