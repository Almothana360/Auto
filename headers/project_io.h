#ifndef PROJECT_IO_H
#define PROJECT_IO_H

#include "cad_types.h"

void SaveProject(const char *filename, GridElement *elements, int elementCount, Layer *layers, int layerCount);
int LoadProject(const char *filename, GridElement *elements, int *elementCount, Layer *layers, int *layerCount);
UiConfig LoadUiConfig(const char *filename);
void SaveUiConfig(const char *filename, const UiConfig *config);
void DeselectAllElements(GridElement *elements, int elementCount);
int CountSelectedElements(GridElement *elements, int elementCount);
int GetFirstSelectedIndex(GridElement *elements, int elementCount);

#endif // PROJECT_IO_H