#ifndef TWEEN_H
#define TWEEN_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- *
 * Types & Enums
 * -------------------------------------------------------------------------- */

typedef struct TweenContext TweenContext;

/* Handle to a specific tween. 0 is considered invalid/null. */
typedef unsigned int TweenId;

/* Easing function signature. Takes a normalized time (0.0 to 1.0) */
typedef float (*TweenEasingFunc)(float t);

/* Callback function signature for tween completion */
typedef void (*TweenCallback)(void* user_data);

/* -------------------------------------------------------------------------- *
 * Built-in Easing Functions
 * -------------------------------------------------------------------------- */

float TweenEase_Linear(float t);
float TweenEase_QuadIn(float t);
float TweenEase_QuadOut(float t);
float TweenEase_QuadInOut(float t);
float TweenEase_CubicIn(float t);
float TweenEase_CubicOut(float t);
float TweenEase_CubicInOut(float t);

/* -------------------------------------------------------------------------- *
 * Context Management
 * -------------------------------------------------------------------------- */

/**
 * Creates a new tween context with a fixed capacity.
 * Allocates memory for the tween pool.
 */
TweenContext* TweenContext_Create(size_t max_tweens);

/**
 * Destroys the context and frees resources.
 */
void TweenContext_Destroy(TweenContext* ctx);

/* -------------------------------------------------------------------------- *
 * Tween Operations
 * -------------------------------------------------------------------------- */

/**
 * Animates the float pointed to by 'target' to 'end_val' over 'duration' seconds.
 * Returns a TweenId that can be used to modify or cancel the tween.
 */
TweenId Tween_To(TweenContext* ctx, float* target, float end_val, float duration, TweenEasingFunc easing);

/**
 * Attaches a completion callback to a specific tween.
 */
void Tween_OnComplete(TweenContext* ctx, TweenId id, TweenCallback callback, void* user_data);

/**
 * Cancels an active tween prematurely.
 */
void Tween_Cancel(TweenContext* ctx, TweenId id);

/**
 * Advances the state of all active tweens. 
 * Should be called once per frame.
 * @param dt Delta time in seconds since the last frame.
 */
void Tween_Update(TweenContext* ctx, float dt);

#ifdef __cplusplus
}
#endif

#endif /* TWEEN_H */