#ifndef CAD_TYPES_H
#define CAD_TYPES_H

#include "raylib.h"
#include <stdbool.h>

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1200
#define SCREEN_TITLE "Piping Cad Studio"

#define MAX_ELEMENTS 32768
#define MAX_LAYERS 16
#define LAYER_NAME_LEN 32

#define LEGACY_PROJECT_FILENAME "project.dat"
#define PROJECT_FILENAME "project.json"
#define CONFIG_FILENAME "config.ini"

#define CMD_BUFFER_SIZE 128
#define MAX_POLYLINE_POINTS 128
#define TEXT_NOTE_LEN 128

#define CURRENT_SCHEMA_VERSION 3
#define HANDLE_SIZE_PX 8.0f
#define ROTATION_HANDLE_OFFSET 30.0f
#define MIN_ELEMENT_SIZE 2.0f

#define QUADTREE_MAX_DEPTH 8
#define QUADTREE_NODE_CAPACITY 16
#define QUADTREE_MAX_NODES 16384
#define SPATIAL_WORLD_EXTENT 200000.0f

#define MAX_COMMAND_HISTORY 1024

typedef enum {
    ELEMENT_RECT = 0,
    ELEMENT_CIRCLE,
    ELEMENT_DIMENSION,
    ELEMENT_LINE,
    ELEMENT_POLYLINE,
    ELEMENT_FREEHAND,
    ELEMENT_ARC,
    ELEMENT_ELLIPSE,
    ELEMENT_TEXT_NOTE,
    ELEMENT_SYMBOL,
    ELEMENT_PID
} ElementType;

typedef ElementType CADEntityType;

typedef enum {
    UNIT_MM,
    UNIT_CM,
    UNIT_M,
    UNIT_IN,
    UNIT_FT
} MeasureUnit;

typedef enum {
    UI_BACKEND_MICROUI = 0,
    UI_BACKEND_RAYGUI  = 1
} UIBackend;

typedef enum {
    UI_THEME_DARK  = 0,
    UI_THEME_LIGHT = 1
} UITheme;

typedef enum {
    TOOL_SELECT = 0,
    TOOL_PID_PALETTE,
    TOOL_ADD_RECT,
    TOOL_ADD_CIRCLE,
    TOOL_DIMENSION,
    TOOL_ADD_LINE,
    TOOL_ADD_POLYLINE,
    TOOL_ADD_FREEHAND,
    TOOL_ADD_ARC,
    TOOL_ADD_ELLIPSE,
    TOOL_ADD_TEXT_NOTE,
    TOOL_PAN
} CADTool;

typedef CADTool ToolMode;

typedef enum {
    HANDLE_NONE = -1,
    HANDLE_TOP_LEFT = 0,
    HANDLE_TOP_CENTER,
    HANDLE_TOP_RIGHT,
    HANDLE_RIGHT_CENTER,
    HANDLE_BOTTOM_RIGHT,
    HANDLE_BOTTOM_CENTER,
    HANDLE_BOTTOM_LEFT,
    HANDLE_LEFT_CENTER,
    HANDLE_ROTATION
} HandleType;

typedef struct {
    Vector2 min;
    Vector2 max;
} AABB;

typedef struct CADEntityBase {
    unsigned int id;
    CADEntityType type;
    Vector2 pos;
    float rotation;
    Vector2 scale;
    AABB bbox;
} CADEntityBase;

typedef struct GridElement {
    union {
        CADEntityBase base;
        struct {
            unsigned int id;
            ElementType type;
            Vector2 pos;
            float rotation;
            Vector2 scale;
            AABB bbox;
        };
    };
    float width, height;
    float radius;
    Vector2 p1, p2, p3, dimPos;
    float lineThickness;
    float tickThickness;
    float startAngle;
    float endAngle;
    float radiusX, radiusY;
    Vector2 points[MAX_POLYLINE_POINTS];
    int pointCount;
    char text[TEXT_NOTE_LEN];
    Vector2 arrowTarget;
    bool showArrow;
    int textSize;
    Color color;
    bool useCustomColor;
    int layerIndex;
    bool selected;
} GridElement;

typedef struct Layer {
    unsigned int id;
    char name[LAYER_NAME_LEN];
    bool visible;
    bool locked;
    Color defaultColor;
    int renderOrder;
    unsigned int *entityIds;
    int entityCount;
    int entityCapacity;
} Layer;

typedef Layer GridLayer;

typedef struct CADGlobalState {
    int activeLayerIndex;
    unsigned int activeLayerId;
    CADTool activeTool;
    Vector2 mouseScreen;
    Vector2 mouseWorld;
} CADGlobalState;

extern CADGlobalState g_CADState;

typedef struct {
    GridElement *elements;
    int elementCount;
    Layer layers[MAX_LAYERS];
    int layerCount;
} ProjectState;

typedef struct {
    AABB bounds;
    int elementIndices[QUADTREE_NODE_CAPACITY];
    int count;
    int children[4];
    bool isLeaf;
} QuadTreeNode;

typedef struct {
    QuadTreeNode nodes[QUADTREE_MAX_NODES];
    int nodeCount;
} SpatialQuadTree;

static const Color PALETTE[] = { SKYBLUE, LIME, ORANGE, PURPLE, RED, GOLD, DARKGRAY, BLACK };
static const int PALETTE_SIZE = sizeof(PALETTE) / sizeof(PALETTE[0]);

typedef struct {
    float uiScale;
    bool isFullscreen;
    bool showHudPanel;
    bool showInspector;
    bool showLayersPanel;
    bool showElementsPanel;
    bool snapToGrid;
    bool snapEnabled;
    int currentUnit;
    int uiBackend;
    int uiTheme;
} UiConfig;

unsigned int GenerateEntityID(void);
unsigned int GenerateLayerID(void);
void SetNextEntityID(unsigned int id);
void SetNextLayerID(unsigned int id);

#endif // CAD_TYPES_H