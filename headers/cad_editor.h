#ifndef CAD_EDITOR_H
#define CAD_EDITOR_H

#include "cad_context.h"

void UpdateCadEditor(AppContext *app, mu_Context *mu_ctx, bool overUI);
void RenderCadEditorViewport(AppContext *app, bool overUI);

#endif // CAD_EDITOR_H