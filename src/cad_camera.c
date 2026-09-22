#include "cad_camera.h"
#include "cad_math.h"
#include "raymath.h"

void UpdateCameraInput(AppContext *app, bool overUI) {
    // Camera Pan
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || (g_CADState.activeTool == TOOL_PAN && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !overUI)) {
        Vector2 delta = GetMouseDelta();
        app->camera.target.x -= delta.x / app->camera.zoom; 
        app->camera.target.y -= delta.y / app->camera.zoom;
    }
    
    // Camera Zoom
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f && !overUI) {
        Vector2 mouseWorldBefore = GetScreenToWorld2D(g_CADState.mouseScreen, app->camera);
        app->camera.zoom += wheel * 0.1f * app->camera.zoom;
        if (app->camera.zoom < 0.05f) app->camera.zoom = 0.05f;
        if (app->camera.zoom > 20.0f) app->camera.zoom = 20.0f;
        Vector2 mouseWorldAfter = GetScreenToWorld2D(g_CADState.mouseScreen, app->camera);
        app->camera.target.x += (mouseWorldBefore.x - mouseWorldAfter.x);
        app->camera.target.y += (mouseWorldBefore.y - mouseWorldAfter.y);
    }
}