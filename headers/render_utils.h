#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include "cad_types.h"
#include "resource_loader.h"
#include "microui.h"

Color GetElementColor(const GridElement *el, const Layer *layers, int layerCount);
void DrawDimensionElement(GridElement *el, MeasureUnit currentUnit, float zoom, bool isSelected, Font font);
void DrawTextNoteElement(GridElement *el, float zoom, bool isSelected, Font font);
void DrawElementSelectionGizmo(const GridElement *el, float zoom);

void SetActiveUIFont(Font font);
int TextWidthCallback(mu_Font font, const char *str, int len);
int TextHeightCallback(mu_Font font);
void RenderMicroui(mu_Context *ctx, Font uiFont);

#endif // RENDER_UTILS_H