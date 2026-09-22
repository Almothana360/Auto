#ifndef CAD_SELECTION_H
#define CAD_SELECTION_H

#include "cad_context.h"

void UpdateSelectionAndHandles(AppContext *app, Vector2 activeToolPoint, bool overUI, int selectedCount);
void RenderSelectionGizmos(AppContext *app);

#endif // CAD_SELECTION_H