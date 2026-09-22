#include "resource_loader.h"
#include <stdio.h>
#include <string.h>

void ResourceManager_Init(ResourceManager *mgr) {
    if (!mgr) return;
    mgr->count = 0;
    memset(mgr->items, 0, sizeof(mgr->items));
}

void ResourceManager_UnloadAll(ResourceManager *mgr) {
    if (!mgr) return;
    for (int i = 0; i < mgr->count; i++) {
        if (!mgr->items[i].loaded) continue;

        switch (mgr->items[i].type) {
            case RES_FONT:
                if (mgr->items[i].font.texture.id != GetFontDefault().texture.id) {
                    UnloadFont(mgr->items[i].font);
                }
                break;
            case RES_TEXTURE:
                UnloadTexture(mgr->items[i].texture);
                break;
            case RES_SOUND:
                UnloadSound(mgr->items[i].sound);
                break;
            case RES_CUSTOM:
                break;
        }
        mgr->items[i].loaded = false;
    }
    mgr->count = 0;
}

static int FindResourceSlot(ResourceManager *mgr, const char *name) {
    for (int i = 0; i < mgr->count; i++) {
        if (strcmp(mgr->items[i].name, name) == 0) return i;
    }
    if (mgr->count < MAX_RESOURCES) return mgr->count++;
    return -1;
}

bool ResourceManager_LoadFont(ResourceManager *mgr, const char *name, const char *filePath, int fontSize) {
    if (!mgr || !name) return false;
    int slot = FindResourceSlot(mgr, name);
    if (slot < 0) return false;

    Font font = { 0 };
    if (filePath && FileExists(filePath)) {
        // Load up to 256 default ASCII characters
        font = LoadFontEx(filePath, fontSize, NULL, 250);
        SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        font = GetFontDefault();
    }

    if (mgr->items[slot].loaded && mgr->items[slot].type == RES_FONT) {
        if (mgr->items[slot].font.texture.id != GetFontDefault().texture.id) {
            UnloadFont(mgr->items[slot].font);
        }
    }

    strncpy(mgr->items[slot].name, name, RESOURCE_NAME_LEN - 1);
    mgr->items[slot].name[RESOURCE_NAME_LEN - 1] = '\0';
    mgr->items[slot].type = RES_FONT;
    mgr->items[slot].font = font;
    mgr->items[slot].loaded = true;
    return true;
}

bool ResourceManager_LoadFontEx(ResourceManager *mgr, const char *name, const char *filePath, int fontSize, int *fontChars, int glyphCount) {
    if (!mgr || !name) return false;
    int slot = FindResourceSlot(mgr, name);
    if (slot < 0) return false;

    Font font = { 0 };
    if (filePath && FileExists(filePath)) {
        font = LoadFontEx(filePath, fontSize, fontChars, glyphCount);
        SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        font = GetFontDefault();
    }

    if (mgr->items[slot].loaded && mgr->items[slot].type == RES_FONT) {
        if (mgr->items[slot].font.texture.id != GetFontDefault().texture.id) {
            UnloadFont(mgr->items[slot].font);
        }
    }

    strncpy(mgr->items[slot].name, name, RESOURCE_NAME_LEN - 1);
    mgr->items[slot].name[RESOURCE_NAME_LEN - 1] = '\0';
    mgr->items[slot].type = RES_FONT;
    mgr->items[slot].font = font;
    mgr->items[slot].loaded = true;
    return true;
}

bool ResourceManager_RegisterFont(ResourceManager *mgr, const char *name, Font font) {
    if (!mgr || !name) return false;
    int slot = FindResourceSlot(mgr, name);
    if (slot < 0) return false;

    strncpy(mgr->items[slot].name, name, RESOURCE_NAME_LEN - 1);
    mgr->items[slot].name[RESOURCE_NAME_LEN - 1] = '\0';
    mgr->items[slot].type = RES_FONT;
    mgr->items[slot].font = font;
    mgr->items[slot].loaded = true;
    return true;
}

Font ResourceManager_GetFont(const ResourceManager *mgr, const char *name) {
    if (!mgr || !name) return GetFontDefault();
    for (int i = 0; i < mgr->count; i++) {
        if (mgr->items[i].loaded && mgr->items[i].type == RES_FONT && strcmp(mgr->items[i].name, name) == 0) {
            return mgr->items[i].font;
        }
    }
    return GetFontDefault();
}

bool ResourceManager_HasFont(const ResourceManager *mgr, const char *name) {
    if (!mgr || !name) return false;
    for (int i = 0; i < mgr->count; i++) {
        if (mgr->items[i].loaded && mgr->items[i].type == RES_FONT && strcmp(mgr->items[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

bool ResourceManager_LoadTexture(ResourceManager *mgr, const char *name, const char *filePath) {
    if (!mgr || !name || !filePath) return false;
    if (!FileExists(filePath)) return false;

    int slot = FindResourceSlot(mgr, name);
    if (slot < 0) return false;

    Texture2D tex = LoadTexture(filePath);
    if (mgr->items[slot].loaded && mgr->items[slot].type == RES_TEXTURE) {
        UnloadTexture(mgr->items[slot].texture);
    }

    strncpy(mgr->items[slot].name, name, RESOURCE_NAME_LEN - 1);
    mgr->items[slot].name[RESOURCE_NAME_LEN - 1] = '\0';
    mgr->items[slot].type = RES_TEXTURE;
    mgr->items[slot].texture = tex;
    mgr->items[slot].loaded = true;
    return true;
}

Texture2D ResourceManager_GetTexture(const ResourceManager *mgr, const char *name) {
    Texture2D empty = { 0 };
    if (!mgr || !name) return empty;
    for (int i = 0; i < mgr->count; i++) {
        if (mgr->items[i].loaded && mgr->items[i].type == RES_TEXTURE && strcmp(mgr->items[i].name, name) == 0) {
            return mgr->items[i].texture;
        }
    }
    return empty;
}

void ResourceManager_RegisterCustom(ResourceManager *mgr, const char *name, void *data) {
    if (!mgr || !name) return;
    int slot = FindResourceSlot(mgr, name);
    if (slot < 0) return;

    strncpy(mgr->items[slot].name, name, RESOURCE_NAME_LEN - 1);
    mgr->items[slot].name[RESOURCE_NAME_LEN - 1] = '\0';
    mgr->items[slot].type = RES_CUSTOM;
    mgr->items[slot].customData = data;
    mgr->items[slot].loaded = true;
}

void *ResourceManager_GetCustom(const ResourceManager *mgr, const char *name) {
    if (!mgr || !name) return NULL;
    for (int i = 0; i < mgr->count; i++) {
        if (mgr->items[i].loaded && mgr->items[i].type == RES_CUSTOM && strcmp(mgr->items[i].name, name) == 0) {
            return mgr->items[i].customData;
        }
    }
    return NULL;
}