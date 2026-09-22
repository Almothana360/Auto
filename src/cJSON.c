#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <strings.h>

cJSON *cJSON_New_Item(void) {
    cJSON* node = (cJSON*)calloc(1, sizeof(cJSON));
    return node;
}

void cJSON_Delete(cJSON *c) {
    cJSON *next;
    while (c) {
        next = c->next;
        if (!(c->type & cJSON_IsReference) && c->child) cJSON_Delete(c->child);
        if (!(c->type & cJSON_IsReference) && c->valuestring) free(c->valuestring);
        if (!(c->type & cJSON_StringIsConst) && c->string) free(c->string);
        free(c);
        c = next;
    }
}

cJSON *cJSON_CreateNull(void)                  { cJSON *item = cJSON_New_Item(); if(item) item->type = cJSON_NULL; return item; }
cJSON *cJSON_CreateTrue(void)                  { cJSON *item = cJSON_New_Item(); if(item) item->type = cJSON_True; return item; }
cJSON *cJSON_CreateFalse(void)                 { cJSON *item = cJSON_New_Item(); if(item) item->type = cJSON_False; return item; }
cJSON *cJSON_CreateBool(bool b)                { return b ? cJSON_CreateTrue() : cJSON_CreateFalse(); }
cJSON *cJSON_CreateNumber(double num)          { cJSON *item = cJSON_New_Item(); if(item) { item->type = cJSON_Number; item->valuedouble = num; item->valueint = (int)num; } return item; }
cJSON *cJSON_CreateString(const char *string)  { cJSON *item = cJSON_New_Item(); if(item) { item->type = cJSON_String; item->valuestring = strdup(string ? string : ""); } return item; }
cJSON *cJSON_CreateArray(void)                 { cJSON *item = cJSON_New_Item(); if(item) item->type = cJSON_Array; return item; }
cJSON *cJSON_CreateObject(void)                { cJSON *item = cJSON_New_Item(); if(item) item->type = cJSON_Object; return item; }

void cJSON_AddItemToArray(cJSON *array, cJSON *item) {
    cJSON *c;
    if (!item) return;
    if (!array->child) {
        array->child = item;
    } else {
        c = array->child;
        while (c && c->next) c = c->next;
        c->next = item;
        item->prev = c;
    }
}

void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item) {
    if (!item) return;
    if (item->string) free(item->string);
    item->string = strdup(string ? string : "");
    cJSON_AddItemToArray(object, item);
}

cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string) {
    cJSON *c = object ? object->child : NULL;
    while (c && strcasecmp(c->string, string) != 0) c = c->next;
    return c;
}

int cJSON_GetArraySize(const cJSON *array) {
    cJSON *c = array ? array->child : NULL;
    int i = 0;
    while (c) { i++; c = c->next; }
    return i;
}

cJSON *cJSON_GetArrayItem(const cJSON *array, int item) {
    cJSON *c = array ? array->child : NULL;
    while (c && item > 0) { item--; c = c->next; }
    return c;
}

typedef struct {
    char *buffer;
    size_t length;
    size_t offset;
    size_t depth;
} PrintBuffer;

static bool EnsureBuffer(PrintBuffer *p, size_t needed) {
    if (!p || !p->buffer) return false;
    size_t new_size = p->offset + needed;
    if (new_size > p->length) {
        size_t alloc_size = (new_size < p->length * 2) ? (p->length * 2) : (new_size + 1024);
        char *new_buf = (char*)realloc(p->buffer, alloc_size);
        if (!new_buf) return false;
        p->buffer = new_buf;
        p->length = alloc_size;
    }
    return true;
}

static void PrintValue(const cJSON *item, PrintBuffer *p, bool format);

static void PrintStringPtr(const char *str, PrintBuffer *p) {
    EnsureBuffer(p, strlen(str) * 2 + 3);
    p->buffer[p->offset++] = '\"';
    while (*str) {
        if ((unsigned char)*str > 31 && *str != '\"' && *str != '\\') {
            p->buffer[p->offset++] = *str++;
        } else {
            p->buffer[p->offset++] = '\\';
            switch (*str) {
                case '\\': p->buffer[p->offset++] = '\\'; break;
                case '\"': p->buffer[p->offset++] = '\"'; break;
                case '\b': p->buffer[p->offset++] = 'b'; break;
                case '\f': p->buffer[p->offset++] = 'f'; break;
                case '\n': p->buffer[p->offset++] = 'n'; break;
                case '\r': p->buffer[p->offset++] = 'r'; break;
                case '\t': p->buffer[p->offset++] = 't'; break;
                default: sprintf(p->buffer + p->offset, "u%04x", (unsigned char)*str); p->offset += 5; break;
            }
            str++;
        }
    }
    p->buffer[p->offset++] = '\"';
    p->buffer[p->offset] = '\0';
}

static void PrintArray(const cJSON *item, PrintBuffer *p, bool format) {
    EnsureBuffer(p, 4);
    p->buffer[p->offset++] = '[';
    p->depth++;
    cJSON *child = item->child;
    while (child) {
        if (format) {
            EnsureBuffer(p, p->depth * 2 + 2);
            p->buffer[p->offset++] = '\n';
            for (size_t i = 0; i < p->depth; i++) { p->buffer[p->offset++] = ' '; p->buffer[p->offset++] = ' '; }
        }
        PrintValue(child, p, format);
        if (child->next) {
            EnsureBuffer(p, 2);
            p->buffer[p->offset++] = ',';
            if (!format) p->buffer[p->offset++] = ' ';
        }
        child = child->next;
    }
    p->depth--;
    if (format && item->child) {
        EnsureBuffer(p, p->depth * 2 + 2);
        p->buffer[p->offset++] = '\n';
        for (size_t i = 0; i < p->depth; i++) { p->buffer[p->offset++] = ' '; p->buffer[p->offset++] = ' '; }
    }
    EnsureBuffer(p, 2);
    p->buffer[p->offset++] = ']';
    p->buffer[p->offset] = '\0';
}

static void PrintObject(const cJSON *item, PrintBuffer *p, bool format) {
    EnsureBuffer(p, 4);
    p->buffer[p->offset++] = '{';
    p->depth++;
    cJSON *child = item->child;
    while (child) {
        if (format) {
            EnsureBuffer(p, p->depth * 2 + 2);
            p->buffer[p->offset++] = '\n';
            for (size_t i = 0; i < p->depth; i++) { p->buffer[p->offset++] = ' '; p->buffer[p->offset++] = ' '; }
        }
        PrintStringPtr(child->string, p);
        EnsureBuffer(p, 2);
        p->buffer[p->offset++] = ':';
        if (format) p->buffer[p->offset++] = ' ';
        PrintValue(child, p, format);
        if (child->next) {
            EnsureBuffer(p, 2);
            p->buffer[p->offset++] = ',';
        }
        child = child->next;
    }
    p->depth--;
    if (format && item->child) {
        EnsureBuffer(p, p->depth * 2 + 2);
        p->buffer[p->offset++] = '\n';
        for (size_t i = 0; i < p->depth; i++) { p->buffer[p->offset++] = ' '; p->buffer[p->offset++] = ' '; }
    }
    EnsureBuffer(p, 2);
    p->buffer[p->offset++] = '}';
    p->buffer[p->offset] = '\0';
}

static void PrintValue(const cJSON *item, PrintBuffer *p, bool format) {
    if (!item) return;
    switch (item->type & 0xFF) {
        case cJSON_NULL:   EnsureBuffer(p, 5); strcpy(p->buffer + p->offset, "null"); p->offset += 4; break;
        case cJSON_False:  EnsureBuffer(p, 6); strcpy(p->buffer + p->offset, "false"); p->offset += 5; break;
        case cJSON_True:   EnsureBuffer(p, 5); strcpy(p->buffer + p->offset, "true"); p->offset += 4; break;
        case cJSON_Number: {
            EnsureBuffer(p, 64);
            int written = 0;
            if (fabs(item->valuedouble - (double)item->valueint) <= DBL_EPSILON && item->valuedouble >= -2147483648.0 && item->valuedouble <= 2147483647.0) {
                written = sprintf(p->buffer + p->offset, "%d", item->valueint);
            } else {
                written = sprintf(p->buffer + p->offset, "%.6f", item->valuedouble);
                char *dot = strchr(p->buffer + p->offset, '.');
                if (dot) {
                    char *end = p->buffer + p->offset + written - 1;
                    while (end > dot && *end == '0') { *end-- = '\0'; written--; }
                    if (end == dot) { *end = '\0'; written--; }
                }
            }
            p->offset += written;
            break;
        }
        case cJSON_String: PrintStringPtr(item->valuestring, p); break;
        case cJSON_Array:  PrintArray(item, p, format); break;
        case cJSON_Object: PrintObject(item, p, format); break;
    }
}

char *cJSON_Print(const cJSON *item) {
    PrintBuffer p;
    p.length = 2048;
    p.buffer = (char*)malloc(p.length);
    p.offset = 0;
    p.depth = 0;
    if (!p.buffer) return NULL;
    PrintValue(item, &p, true);
    return p.buffer;
}

static const char *SkipWhitespace(const char *in) {
    while (in && *in && (unsigned char)*in <= 32) in++;
    return in;
}

static const char *ParseValue(cJSON *item, const char *value);

static const char *ParseString(cJSON *item, const char *str) {
    const char *ptr = str + 1;
    char *out;
    int len = 0;
    if (*str != '\"') return NULL;
    while (*ptr != '\"' && *ptr && ++len) { if (*ptr++ == '\\') ptr++; }
    out = (char*)malloc(len + 1);
    if (!out) return NULL;
    ptr = str + 1;
    char *ptr2 = out;
    while (*ptr != '\"' && *ptr) {
        if (*ptr != '\\') *ptr2++ = *ptr++;
        else {
            ptr++;
            switch (*ptr) {
                case 'b': *ptr2++ = '\b'; break;
                case 'f': *ptr2++ = '\f'; break;
                case 'n': *ptr2++ = '\n'; break;
                case 'r': *ptr2++ = '\r'; break;
                case 't': *ptr2++ = '\t'; break;
                default:  *ptr2++ = *ptr; break;
            }
            ptr++;
        }
    }
    *ptr2 = 0;
    if (*ptr == '\"') ptr++;
    item->valuestring = out;
    item->type = cJSON_String;
    return ptr;
}

static const char *ParseNumber(cJSON *item, const char *num) {
    char *end;
    double d = strtod(num, &end);
    if (end == num) return NULL;
    item->valuedouble = d;
    item->valueint = (int)d;
    item->type = cJSON_Number;
    return end;
}

static const char *ParseArray(cJSON *item, const char *value) {
    cJSON *child;
    if (*value != '[') return NULL;
    item->type = cJSON_Array;
    value = SkipWhitespace(value + 1);
    if (*value == ']') return value + 1;

    item->child = child = cJSON_New_Item();
    if (!child) return NULL;
    value = SkipWhitespace(ParseValue(child, value));
    if (!value) return NULL;

    while (*value == ',') {
        cJSON *new_item = cJSON_New_Item();
        if (!new_item) return NULL;
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = SkipWhitespace(ParseValue(child, SkipWhitespace(value + 1)));
        if (!value) return NULL;
    }
    if (*value == ']') return value + 1;
    return NULL;
}

static const char *ParseObject(cJSON *item, const char *value) {
    cJSON *child;
    if (*value != '{') return NULL;
    item->type = cJSON_Object;
    value = SkipWhitespace(value + 1);
    if (*value == '}') return value + 1;

    item->child = child = cJSON_New_Item();
    if (!child) return NULL;
    value = SkipWhitespace(ParseString(child, value));
    if (!value) return NULL;
    child->string = child->valuestring;
    child->valuestring = NULL;
    value = SkipWhitespace(value);
    if (*value != ':') return NULL;
    value = SkipWhitespace(ParseValue(child, SkipWhitespace(value + 1)));
    if (!value) return NULL;

    while (*value == ',') {
        cJSON *new_item = cJSON_New_Item();
        if (!new_item) return NULL;
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = SkipWhitespace(ParseString(child,  SkipWhitespace(value + 1)));
        if (!value) return NULL;
        child->string = child->valuestring;
        child->valuestring = NULL;
        value = SkipWhitespace(value);
        if (*value != ':') return NULL;
        value = SkipWhitespace(ParseValue(child, SkipWhitespace(value + 1)));
        if (!value) return NULL;
    }
    if (*value == '}') return value + 1;
    return NULL;
}

static const char *ParseValue(cJSON *item, const char *value) {
    if (!value) return NULL;
    if (!strncmp(value, "null", 4))  { item->type = cJSON_NULL; return value + 4; }
    if (!strncmp(value, "false", 5)) { item->type = cJSON_False; return value + 5; }
    if (!strncmp(value, "true", 4))  { item->type = cJSON_True; item->valueint = 1; return value + 4; }
    if (*value == '\"')              { return ParseString(item, value); }
    if (*value == '-' || (*value >= '0' && *value <= '9')) { return ParseNumber(item, value); }
    if (*value == '[')              { return ParseArray(item, value); }
    if (*value == '{')              { return ParseObject(item, value); }
    return NULL;
}

cJSON *cJSON_Parse(const char *value) {
    cJSON *c = cJSON_New_Item();
    if (!c) return NULL;
    const char *end = ParseValue(c, SkipWhitespace(value));
    if (!end) { cJSON_Delete(c); return NULL; }
    return c;
}