#ifndef RESOURCE_LOADER_H
#define RESOURCE_LOADER_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_RESOURCES 128
#define RESOURCE_NAME_LEN 128

typedef enum {
    RES_FONT = 0,
    RES_TEXTURE,
    RES_SOUND,
    RES_CUSTOM
} ResourceType;

typedef struct Resource {
    char name[RESOURCE_NAME_LEN];
    ResourceType type;
    union {
        Font font;
        Texture2D texture;
        Sound sound;
        void *customData;
    };
    bool loaded;
} Resource;

typedef struct ResourceManager {
    Resource items[MAX_RESOURCES];
    int count;
} ResourceManager;

// Logical slot names used across UI and canvas
#define FONT_SLOT_MENU   "font_menu"
#define FONT_SLOT_TITLE  "font_title"
#define FONT_SLOT_BODY   "font_body"
#define FONT_SLOT_NOTE   "font_note"

// Custom Font File Paths
#define FONT_PATH_MENU   "../resources/GoogleSansCodeNerdFont-ExtraBold.ttf"
#define FONT_PATH_TITLE  "../resources/GoogleSansCodeNerdFont-ExtraBoldItalic.ttf"
#define FONT_PATH_BODY   "../resources/GoogleSansCodeNerdFont-Medium.ttf"
#define FONT_PATH_NOTE   "../resources/GoogleSansCodeNerdFontPropo-Regular.ttf"

void ResourceManager_Init(ResourceManager *mgr);
void ResourceManager_UnloadAll(ResourceManager *mgr);

bool ResourceManager_LoadFont(ResourceManager *mgr, const char *name, const char *filePath, int fontSize);
bool ResourceManager_LoadFontEx(ResourceManager *mgr, const char *name, const char *filePath, int fontSize, int *fontChars, int glyphCount);
bool ResourceManager_RegisterFont(ResourceManager *mgr, const char *name, Font font);

Font ResourceManager_GetFont(const ResourceManager *mgr, const char *name);
bool ResourceManager_HasFont(const ResourceManager *mgr, const char *name);

bool ResourceManager_LoadTexture(ResourceManager *mgr, const char *name, const char *filePath);
Texture2D ResourceManager_GetTexture(const ResourceManager *mgr, const char *name);

void ResourceManager_RegisterCustom(ResourceManager *mgr, const char *name, void *data);
void *ResourceManager_GetCustom(const ResourceManager *mgr, const char *name);

#endif // RESOURCE_LOADER_H