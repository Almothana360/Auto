#ifndef GUI_PANELS_RAYGUI_H
#define GUI_PANELS_RAYGUI_H

#include "cad_context.h"
#include <stdbool.h>

void ApplyRayguiTheme(int theme);
bool CheckGuiHover_Raygui(AppContext *app);
void RenderAllGuiPanels_Raygui(AppContext *app);

#endif // GUI_PANELS_RAYGUI_H