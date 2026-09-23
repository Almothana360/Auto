#ifndef CAD_CONNECTION_H
#define CAD_CONNECTION_H

#include "cad_types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CONN_PORT_NONE = -1,
    CONN_PORT_FLANGE_HEAD = 0,
    CONN_PORT_FLANGE_TAIL = 1
} ConnectionPortType;

typedef struct ConnectionPort {
    ConnectionPortType type;
    Vector2 worldPos;
    Vector2 normal;      // Outward pointing directional unit vector
    Vector2 triPts[3];   // 0: Apex (Tip), 1: Base top/side, 2: Base bottom/side
    Rectangle screenHitBox;
} ConnectionPort;

typedef struct ConnectionSystemState {
    bool hasActivePorts;
    int targetElementIndex;
    ConnectionPort ports[2];
    int portCount;

    // Context Menu State
    bool showContextMenu;
    Vector2 menuScreenPos;
    ConnectionPortType activePortType;
    int hoveredOption;
    bool justOpened;

    // Modal Message Box State
    bool showMessageBox;
    char messageBoxTitle[64];
    char messageBoxText[128];
} ConnectionSystemState;

struct AppContext;

void ConnectionSystem_Init(ConnectionSystemState *conn);
void ConnectionSystem_UpdatePorts(ConnectionSystemState *conn, struct AppContext *app);
bool ConnectionSystem_HandleInput(ConnectionSystemState *conn, struct AppContext *app, bool overUI);
void ConnectionSystem_RenderPortButtons(const ConnectionSystemState *conn, struct AppContext *app);
void ConnectionSystem_RenderContextMenu(ConnectionSystemState *conn, struct AppContext *app);
void ConnectionSystem_RenderMessageBox(ConnectionSystemState *conn, struct AppContext *app);
bool ConnectionSystem_IsHovered(const ConnectionSystemState *conn, const struct AppContext *app);

#ifdef __cplusplus
}
#endif

#endif // CAD_CONNECTION_H