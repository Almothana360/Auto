#include "tween.h"
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- *
 * Internal Structures
 * -------------------------------------------------------------------------- */

typedef struct {
    float* target;
    float start_val;
    float end_val;
    float duration;
    float elapsed;
    
    TweenEasingFunc easing;
    TweenCallback on_complete;
    void* user_data;
    
    unsigned int generation; 
    bool active;
} Tween;

struct TweenContext {
    Tween* pool;
    size_t capacity;
    unsigned int current_generation;
};

/* 
 * Helper macro to pack index and generation into a 32-bit ID.
 * Top 16 bits: Generation, Bottom 16 bits: Index (limits pool to 65535 tweens)
 */
#define MAKE_ID(index, gen) ((((gen) & 0xFFFF) << 16) | ((index) & 0xFFFF))
#define GET_INDEX(id) ((id) & 0xFFFF)
#define GET_GEN(id) (((id) >> 16) & 0xFFFF)

/* -------------------------------------------------------------------------- *
 * Easing Functions (Implementation)
 * -------------------------------------------------------------------------- */

float TweenEase_Linear(float t) { return t; }
float TweenEase_QuadIn(float t) { return t * t; }
float TweenEase_QuadOut(float t) { return t * (2.0f - t); }
float TweenEase_QuadInOut(float t) { 
    return (t < 0.5f) ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; 
}
float TweenEase_CubicIn(float t) { return t * t * t; }
float TweenEase_CubicOut(float t) { 
    float f = t - 1.0f; 
    return f * f * f + 1.0f; 
}
float TweenEase_CubicInOut(float t) {
    if (t < 0.5f) return 4.0f * t * t * t;
    float f = (2.0f * t) - 2.0f;
    return 0.5f * f * f * f + 1.0f;
}

/* -------------------------------------------------------------------------- *
 * Core Library Logic
 * -------------------------------------------------------------------------- */

TweenContext* TweenContext_Create(size_t max_tweens) {
    if (max_tweens == 0 || max_tweens > 0xFFFF) return NULL;
    
    TweenContext* ctx = (TweenContext*)malloc(sizeof(TweenContext));
    if (!ctx) return NULL;
    
    ctx->pool = (Tween*)calloc(max_tweens, sizeof(Tween));
    if (!ctx->pool) {
        free(ctx);
        return NULL;
    }
    
    ctx->capacity = max_tweens;
    ctx->current_generation = 1;
    return ctx;
}

void TweenContext_Destroy(TweenContext* ctx) {
    if (ctx) {
        free(ctx->pool);
        free(ctx);
    }
}

static Tween* GetTweenFromId(TweenContext* ctx, TweenId id) {
    if (!ctx || id == 0) return NULL;
    
    size_t index = GET_INDEX(id);
    unsigned int gen = GET_GEN(id);
    
    if (index >= ctx->capacity) return NULL;
    
    Tween* tween = &ctx->pool[index];
    if (tween->active && tween->generation == gen) {
        return tween;
    }
    return NULL;
}

TweenId Tween_To(TweenContext* ctx, float* target, float end_val, float duration, TweenEasingFunc easing) {
    if (!ctx || !target) return 0;
    
    /* Find an inactive slot in the pool */
    for (size_t i = 0; i < ctx->capacity; ++i) {
        Tween* tween = &ctx->pool[i];
        if (!tween->active) {
            tween->target = target;
            tween->start_val = *target;
            tween->end_val = end_val;
            tween->duration = (duration > 0.0001f) ? duration : 0.0001f; /* Prevent div by zero */
            tween->elapsed = 0.0f;
            tween->easing = easing ? easing : TweenEase_Linear;
            tween->on_complete = NULL;
            tween->user_data = NULL;
            
            tween->generation = ctx->current_generation++;
            if (ctx->current_generation > 0xFFFF) ctx->current_generation = 1;
            
            tween->active = true;
            return MAKE_ID(i, tween->generation);
        }
    }
    
    return 0; /* Pool exhausted */
}

void Tween_OnComplete(TweenContext* ctx, TweenId id, TweenCallback callback, void* user_data) {
    Tween* tween = GetTweenFromId(ctx, id);
    if (tween) {
        tween->on_complete = callback;
        tween->user_data = user_data;
    }
}

void Tween_Cancel(TweenContext* ctx, TweenId id) {
    Tween* tween = GetTweenFromId(ctx, id);
    if (tween) {
        tween->active = false;
    }
}

void Tween_Update(TweenContext* ctx, float dt) {
    if (!ctx || dt <= 0.0f) return;
    
    for (size_t i = 0; i < ctx->capacity; ++i) {
        Tween* tween = &ctx->pool[i];
        if (!tween->active) continue;
        
        tween->elapsed += dt;
        float t = tween->elapsed / tween->duration;
        
        if (t >= 1.0f) {
            /* Tween finished */
            t = 1.0f;
            *tween->target = tween->end_val;
            tween->active = false; /* Mark inactive before callback to allow re-triggering safely */
            
            if (tween->on_complete) {
                tween->on_complete(tween->user_data);
            }
        } else {
            /* Tween interpolating */
            float eased_t = tween->easing(t);
            *tween->target = tween->start_val + (tween->end_val - tween->start_val) * eased_t;
        }
    }
}