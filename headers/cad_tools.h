#ifndef CAD_TOOLS_H
#define CAD_TOOLS_H

#include "cad_context.h"

void UpdateTools(AppContext *app, Vector2 activeToolPoint, bool overUI);
void RenderToolPreviews(AppContext *app, Vector2 activeToolPoint, Font noteFont);

#endif // CAD_TOOLS_H