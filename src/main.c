#include "raylib.h"
#include "cad_context.h"
#include "cad_editor.h"
#include "input_handler.h"
#include "ui_manager.h"

int main(void) {
    AppContext app;
    AppContext_Init(&app);

    unsigned int flags = FLAG_WINDOW_RESIZABLE;
    if (app.uiConfig.isFullscreen) flags |= FLAG_FULLSCREEN_MODE;
    SetConfigFlags(flags);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
    SetExitKey(KEY_F1);
    SetTargetFPS(60);

    AppContext_InitFonts(&app);

    mu_Context mu_ctx;
    UIManager_Init(&app, &mu_ctx);

    while (!WindowShouldClose()) {
        // 1. Core State Update
        AppContext_Update(&app);

        // 2. Input Handling
        UIManager_ProcessInput(&app, &mu_ctx);
        HandleGlobalInput(&app, &mu_ctx);

        // 3. UI Logic Update
        bool overUI = UIManager_UpdateAndRenderPanels(&app, &mu_ctx);

        // 4. CAD Editor Logic
        UpdateCadEditor(&app, &mu_ctx, overUI);

        // 5. Render Pass
        BeginDrawing();
        ClearBackground(RAYWHITE);

        RenderCadEditorViewport(&app, overUI);
        UIManager_RenderOverlays(&app, &mu_ctx);

        EndDrawing();
    }

    AppContext_Cleanup(&app);
    CloseWindow();
    return 0;
}