#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "cad_context.h"
#include "microui.h"

void UIManager_Init(AppContext *app, mu_Context *mu_ctx);
void UIManager_ProcessInput(AppContext *app, mu_Context *mu_ctx);
bool UIManager_UpdateAndRenderPanels(AppContext *app, mu_Context *mu_ctx);
void UIManager_RenderOverlays(AppContext *app, mu_Context *mu_ctx);

#endif // UI_MANAGER_H