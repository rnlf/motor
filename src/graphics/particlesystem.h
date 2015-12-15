#pragma once

#include <stdbool.h>
#include "graphics.h"
#include "image.h"
#include "quad.h"
#include "batch.h"

struct graphics_Particle;
typedef struct graphics_Particle graphics_Particle;

typedef enum {
  graphics_ParticleInsertMode_top,
  graphics_ParticleInsertMode_bottom,
  graphics_ParticleInsertMode_random
} graphics_ParticleInsertMode;


typedef enum {
  graphics_AreaSpreadDistribution_uniform,
  graphics_AreaSpreadDistribution_normal,
  graphics_AreaSpreadDistribution_none
} graphics_AreaSpreadDistribution;


typedef struct {
  graphics_Particle *pMem;
  graphics_Particle *pFree;
  graphics_Particle *pHead;
  graphics_Particle *pTail;

  graphics_Image const* texture;
  graphics_Batch batch;
  bool active;
  
  graphics_ParticleInsertMode insertMode;

  size_t maxParticles;
  size_t activeParticles;

  float emissionRate;
  float emitCounter;
  
  float position[2];
  float prevPosition[2];

  graphics_AreaSpreadDistribution areaSpreadDistribution;
  float areaSpread[2];
  
  float lifetime;
  float life;

  float particleLifeMin;
  float particleLifeMax;

  float direction;
  float spread;

  float speedMin;
  float speedMax;

  float linearAccelerationMin[2];
  float linearAccelerationMax[2];

  float radialAccelerationMin;
  float radialAccelerationMax;

  float tangentialAccelerationMin;
  float tangentialAccelerationMax;

  float linearDampingMin;
  float linearDampingMax;

  float *sizes;
  size_t sizeCount;
  float sizeVariation;

  float rotationMin;
  float rotationMax;

  float spinStart;
  float spinEnd;
  float spinVariation;

  float offsetX;
  float offsetY;

  graphics_Color *colors;
  size_t colorCount;

  graphics_Quad const **quads;
  size_t quadCount;

  bool relativeRotation;
} graphics_ParticleSystem;


void graphics_ParticleSystem_new(graphics_ParticleSystem *ps, graphics_Image const *texture, size_t buffer);
void graphics_ParticleSystem_free(graphics_ParticleSystem *ps);

void graphics_ParticleSystem_setAreaSpread(graphics_ParticleSystem *ps, graphics_AreaSpreadDistribution mode, float dx, float dy);
void graphics_ParticleSystem_getAreaSpread(graphics_ParticleSystem const *ps, graphics_AreaSpreadDistribution *mode, float *dx, float *dy);
void graphics_ParticleSystem_setBufferSize(graphics_ParticleSystem *ps, size_t size);
size_t graphics_ParticleSystem_getBufferSize(graphics_ParticleSystem const *ps);
void graphics_ParticleSystem_setColors(graphics_ParticleSystem *ps, size_t count, graphics_Color const *colors);
graphics_Color const* graphics_ParticleSystem_getColors(graphics_ParticleSystem const *ps, size_t *count);
void graphics_ParticleSystem_setTexture(graphics_ParticleSystem *ps, graphics_Image const* texture);
graphics_Image const* graphics_ParticleSystem_getTexture(graphics_ParticleSystem const *ps);
float const *graphics_ParticleSystem_getSizes(graphics_ParticleSystem const *ps, size_t *count);
void graphics_ParticleSystem_setSizes(graphics_ParticleSystem *ps, size_t count, float const *sizes);
void graphics_ParticleSystem_setQuads(graphics_ParticleSystem *ps, size_t count, graphics_Quad const * const *quads);
void graphics_ParticleSystem_setRelativeRotation(graphics_ParticleSystem *ps, bool enable);
void graphics_ParticleSystem_setInsertMode(graphics_ParticleSystem *ps, graphics_ParticleInsertMode mode);
graphics_ParticleInsertMode graphics_ParticleSystem_getInsertMode(graphics_ParticleSystem const *ps);

size_t graphics_ParticleSystem_getCount(graphics_ParticleSystem const *ps);

void graphics_ParticleSystem_setDirection(graphics_ParticleSystem *ps, float dir);
void graphics_ParticleSystem_setEmissionRate(graphics_ParticleSystem *ps, float rate);
void graphics_ParticleSystem_setEmitterLifetime(graphics_ParticleSystem *ps, float lifetime);
void graphics_ParticleSystem_setSizeVariation(graphics_ParticleSystem *ps, float variation);
void graphics_ParticleSystem_setSpinVariation(graphics_ParticleSystem *ps, float variation);
void graphics_ParticleSystem_setSpread(graphics_ParticleSystem *ps, float spread);

float graphics_ParticleSystem_getDirection(graphics_ParticleSystem const *ps);
float graphics_ParticleSystem_getEmissionRate(graphics_ParticleSystem const *ps);
float graphics_ParticleSystem_getEmitterLifetime(graphics_ParticleSystem const *ps);
float graphics_ParticleSystem_getSizeVariation(graphics_ParticleSystem const *ps);
float graphics_ParticleSystem_getSpinVariation(graphics_ParticleSystem const *ps);
float graphics_ParticleSystem_getSpread(graphics_ParticleSystem const *ps);

void graphics_ParticleSystem_setOffset(graphics_ParticleSystem *ps, float x, float y);
void graphics_ParticleSystem_setParticleLifetime(graphics_ParticleSystem *ps, float min, float max);
void graphics_ParticleSystem_setPosition(graphics_ParticleSystem *ps, float x, float y);
void graphics_ParticleSystem_setRadialAcceleration(graphics_ParticleSystem *ps, float min, float max);
void graphics_ParticleSystem_setRotation(graphics_ParticleSystem *ps, float min, float max);
void graphics_ParticleSystem_setSpeed(graphics_ParticleSystem *ps, float min, float max);
void graphics_ParticleSystem_setSpin(graphics_ParticleSystem *ps, float min, float max);
void graphics_ParticleSystem_setTangentialAcceleration(graphics_ParticleSystem *ps, float min, float max);
void graphics_ParticleSystem_setLinearDamping(graphics_ParticleSystem *ps, float min, float max);

void graphics_ParticleSystem_getOffset(graphics_ParticleSystem const *ps, float *x, float *y);
void graphics_ParticleSystem_getParticleLifetime(graphics_ParticleSystem const *ps, float *min, float *max);
void graphics_ParticleSystem_getPosition(graphics_ParticleSystem const *ps, float *x, float *y);
void graphics_ParticleSystem_getRadialAcceleration(graphics_ParticleSystem const *ps, float *min, float *max);
void graphics_ParticleSystem_getRotation(graphics_ParticleSystem const *ps, float *min, float *max);
void graphics_ParticleSystem_getSpeed(graphics_ParticleSystem const *ps, float *min, float *max);
void graphics_ParticleSystem_getSpin(graphics_ParticleSystem const *ps, float *min, float *max);
void graphics_ParticleSystem_getTangentialAcceleration(graphics_ParticleSystem const *ps, float *min, float *max);
void graphics_ParticleSystem_getLinearDamping(graphics_ParticleSystem const *ps, float *min, float *max);

bool graphics_ParticleSystem_isActive(graphics_ParticleSystem const *ps);
bool graphics_ParticleSystem_isPaused(graphics_ParticleSystem const *ps);
bool graphics_ParticleSystem_isStopped(graphics_ParticleSystem const *ps);
bool graphics_ParticleSystem_hasRelativeRotation(graphics_ParticleSystem const *ps);

void graphics_ParticleSystem_start(graphics_ParticleSystem *ps);
void graphics_ParticleSystem_stop(graphics_ParticleSystem *ps);
void graphics_ParticleSystem_reset(graphics_ParticleSystem *ps);
void graphics_ParticleSystem_pause(graphics_ParticleSystem *ps);

void graphics_ParticleSystem_emit(graphics_ParticleSystem *ps, size_t count);
void graphics_ParticleSystem_update(graphics_ParticleSystem *ps, float dt);
void graphics_ParticleSystem_moveTo(graphics_ParticleSystem *ps, float x, float y);

void graphics_ParticleSystem_clone(graphics_ParticleSystem const* ps, graphics_ParticleSystem *psNew);

void graphics_ParticleSystem_draw(graphics_ParticleSystem const* ps, float x, float y, float r, float sx, float sy, float ox, float oy, float kx, float ky);
void graphics_ParticleSystem_setLinearAcceleration(graphics_ParticleSystem *ps, float xmin, float ymin, float xmax, float ymax);
void graphics_ParticleSystem_getLinearAcceleration(graphics_ParticleSystem const *ps, float *xmin, float *ymin, float *xmax, float *ymax);

void graphics_particlesystem_init();
