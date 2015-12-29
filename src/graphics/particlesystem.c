/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include <tgmath.h>
#include <string.h>
#include <stdlib.h>
#include "particlesystem.h"
#include "../math/minmax.h"
#include "../math/lerp.h"
#include "../math/randomgenerator.h"
#include "../math/util.h"


static struct {
  math_RandomGenerator rng;
} moduleData;


struct graphics_Particle {
  struct graphics_Particle *prev;
  struct graphics_Particle *next;

  float lifetime;
  float life;

  float position[2];

  float origin[2];

  float velocity[2];
  float linearAcceleration[2];
  float radialAcceleration;
  float tangentialAcceleration;

  float linearDamping;

  float size;
  float sizeOffset;
  float sizeIntervalSize;

  float rotation;
  float angle;
  float spinStart;
  float spinEnd;

  graphics_Color color;

  int quadIndex;
};

static const graphics_Color defaultColor = {1.0f, 1.0f, 1.0f, 1.0f};
static const graphics_Quad defaultQuad = {0.0f, 0.0f, 1.0f, 1.0f};
static const graphics_Quad *defaultQuadPtr = &defaultQuad;
static const float defaultSize = 1.0f;


static float calculateVariation(float inner, float outer, float var) {
  float v2 = (outer/2.0f) * var;
  float low = inner - v2;
  float high = inner + v2;
  float r = math_RandomGenerator_random(&moduleData.rng);
  return low * (1-r) + high * r;
}


static void initParticle(graphics_ParticleSystem const *ps, graphics_Particle *p, float t) {
  p->origin[0] = p->position[0] = lerp(ps->prevPosition[0], ps->position[0], t);
  p->origin[1] = p->position[1] = lerp(ps->prevPosition[1], ps->position[1], t);

  if(ps->particleLifeMin == ps->particleLifeMax) {
    p->life = ps->particleLifeMin;
  } else {
    p->life = (float) math_RandomGenerator_random2(&moduleData.rng, ps->particleLifeMin, ps->particleLifeMax);
  }
  p->lifetime = p->life;

  switch(ps->areaSpreadDistribution) {
  case graphics_AreaSpreadDistribution_uniform:
    p->position[0] += math_RandomGenerator_random2(&moduleData.rng, -1.0, 1.0) * ps->areaSpread[0];
    p->position[1] += math_RandomGenerator_random2(&moduleData.rng, -1.0, 1.0) * ps->areaSpread[1];
    break;
  case graphics_AreaSpreadDistribution_normal:
    p->position[0] += math_RandomGenerator_randomNormal(&moduleData.rng, ps->areaSpread[0]);
    p->position[1] += math_RandomGenerator_randomNormal(&moduleData.rng, ps->areaSpread[1]);
    break;
  default:
    break;
  }

  float direction = ps->direction + math_RandomGenerator_random2(&moduleData.rng, -1.0, 1.0) * ps->spread;

  float speed = math_RandomGenerator_random2(&moduleData.rng, ps->speedMin, ps->speedMax);
  p->velocity[0] = cos(direction) * speed;
  p->velocity[1] = sin(direction) * speed;

  p->linearAcceleration[0] = math_RandomGenerator_random2(&moduleData.rng, ps->linearAccelerationMin[0], ps->linearAccelerationMax[0]);
  p->linearAcceleration[1] = math_RandomGenerator_random2(&moduleData.rng, ps->linearAccelerationMin[1], ps->linearAccelerationMax[1]);

  p->radialAcceleration = math_RandomGenerator_random2(&moduleData.rng, ps->radialAccelerationMin, ps->radialAccelerationMax);

  p->tangentialAcceleration = math_RandomGenerator_random2(&moduleData.rng, ps->tangentialAccelerationMin, ps->tangentialAccelerationMax);

  p->linearDamping = math_RandomGenerator_random2(&moduleData.rng, ps->linearDampingMin, ps->linearDampingMax);

  p->sizeOffset = math_RandomGenerator_random(&moduleData.rng);
  p->sizeIntervalSize = (1.0 - math_RandomGenerator_random2(&moduleData.rng, 0.0, ps->sizeVariation)) - p->sizeOffset;
  p->size = ps->sizes[(size_t)(p->sizeOffset - 0.5f) * (ps->sizeCount -1 )];

  p->spinStart = calculateVariation(ps->spinStart, ps->spinEnd,   ps->spinVariation);
  p->spinEnd   = calculateVariation(ps->spinEnd,   ps->spinStart, ps->spinVariation);
  p->rotation  = math_RandomGenerator_random2(&moduleData.rng, ps->rotationMin, ps->rotationMax);

  p->angle = p->rotation;
  if(ps->relativeRotation) {
    p->angle += atan2(p->velocity[1], p->velocity[0]);
  }

  p->color = ps->colors[0];
  p->quadIndex = 0;

}


static void insertTop(graphics_ParticleSystem *ps, graphics_Particle *p) {
  if(ps->pHead == 0) {
    ps->pHead = p;
    p->prev = 0;
  } else {
    ps->pTail->next = p;
    p->prev = ps->pTail;
  }

  p->next = 0;
  ps->pTail = p;
}


static void insertBottom(graphics_ParticleSystem *ps, graphics_Particle *p) {
  if(ps->pTail == 0) {
    ps->pTail = p;
    p->next = 0;
  } else {
    ps->pHead->prev = p;
    p->next = ps->pHead;
  }

  p->prev = 0;
  ps->pHead = p;
}


static void insertRandom(graphics_ParticleSystem *ps, graphics_Particle *p) {
  uint64_t pos = math_RandomGenerator_rand(&moduleData.rng) % ((int64_t) ps->activeParticles + 1);

  if(pos == ps->activeParticles) {
    graphics_Particle *pA = ps->pHead;
    if(pA) {
      pA->prev = p;
    }

    p->prev = 0;
    p->next = pA;
    ps->pHead = p;
    return;
  }

  graphics_Particle *pA = ps->pMem + pos;
  graphics_Particle *pB = pA->next;
  pA->next = p;
  if(pB) {
    pB->prev = p;
  } else {
    ps->pTail = p;
  }
  p->prev = pA;
  p->next = pB;
}


static graphics_Particle* removeParticle(graphics_ParticleSystem *ps, graphics_Particle *p) {
  graphics_Particle *pNext = 0;
  if(p->prev) {
    p->prev->next = p->next;
  } else {
    ps->pHead = p->next;
  }

  if(p->next) {
    p->next->prev = p->prev;
    pNext = p->next;
  } else {
    ps->pTail = p->prev;
  }

  --ps->pFree;

  if(p != ps->pFree) {
    *p = *ps->pFree;
    if(pNext == ps->pFree) {
      pNext = p;
    }

    if(p->prev) {
      p->prev->next = p;
    } else {
      ps->pHead = p;
    }

    if(p->next) {
      p->next->prev = p;
    } else {
      ps->pTail = p;
    }
  }

  --ps->activeParticles;
  return pNext;
}


static void addParticle(graphics_ParticleSystem *ps, float t) {
  graphics_Particle *p = ps->pFree++;
  initParticle(ps, p, t);

  switch(ps->insertMode) {
  case graphics_ParticleInsertMode_top:
    insertTop(ps, p);
    break;
  case graphics_ParticleInsertMode_bottom:
    insertBottom(ps, p);
    break;
  case graphics_ParticleInsertMode_random:
    insertRandom(ps, p);
    break;
  }

  ++ps->activeParticles;
}


void graphics_ParticleSystem_new(graphics_ParticleSystem *ps, graphics_Image const *texture, size_t buffer) {
  ps->sizes = 0;
  ps->colors = 0;
  ps->quads = 0;
  ps->active = false;
  graphics_Batch_new(&ps->batch, texture, buffer, graphics_BatchUsage_stream);

  graphics_ParticleSystem_setBufferSize(ps, buffer);
  graphics_ParticleSystem_reset(ps);
  graphics_ParticleSystem_setTexture(ps, texture);
  graphics_ParticleSystem_setInsertMode(ps, graphics_ParticleInsertMode_top);
  graphics_ParticleSystem_setEmissionRate(ps, 0.0f);
  graphics_ParticleSystem_setPosition(ps, 0.0f, 0.0f);
  graphics_ParticleSystem_setAreaSpread(ps, graphics_AreaSpreadDistribution_none, 0.0f, 0.0f);
  graphics_ParticleSystem_setEmitterLifetime(ps, -1.0f);
  graphics_ParticleSystem_setParticleLifetime(ps, 0.0f, 0.0f);
  graphics_ParticleSystem_setDirection(ps, 0.0f);
  graphics_ParticleSystem_setSpread(ps, 0.0f);
  graphics_ParticleSystem_setSpeed(ps, 0.0f, 0.0f);
  graphics_ParticleSystem_setLinearAcceleration(ps, 0.0f, 0.0f, 0.0f, 0.0f);
  graphics_ParticleSystem_setRadialAcceleration(ps, 0.0f, 0.0f);
  graphics_ParticleSystem_setTangentialAcceleration(ps, 0.0f, 0.0f);
  graphics_ParticleSystem_setLinearDamping(ps, 0.0f, 0.0f);
  graphics_ParticleSystem_setSizes(ps, 1, &defaultSize);
  graphics_ParticleSystem_setSizeVariation(ps, 0.0f);
  graphics_ParticleSystem_setRotation(ps, 0.0f, 0.0f);
  graphics_ParticleSystem_setSpin(ps, 0.0f, 0.0f);
  graphics_ParticleSystem_setSpinVariation(ps, 0.0f);
  graphics_ParticleSystem_setOffset(ps, ((float)texture->width) * 0.5f, ((float)texture->height) * 0.5f);
  graphics_ParticleSystem_setColors(ps, 1, &defaultColor);
  graphics_ParticleSystem_setQuads(ps, 1, &defaultQuadPtr);
  graphics_ParticleSystem_setRelativeRotation(ps, false);
}


void graphics_ParticleSystem_clone(graphics_ParticleSystem const* ps, graphics_ParticleSystem *psNew) {
  psNew->sizes = 0;
  psNew->colors = 0;
  psNew->quads = 0;
  psNew->active = false;

  graphics_ParticleSystem_setBufferSize(psNew, ps->maxParticles);
  graphics_ParticleSystem_reset(psNew);
  graphics_ParticleSystem_setTexture(psNew, ps->texture);
  graphics_ParticleSystem_setInsertMode(psNew, ps->insertMode);
  graphics_ParticleSystem_setEmissionRate(psNew, ps->emissionRate);
  graphics_ParticleSystem_setPosition(psNew, ps->position[0], ps->position[1]);
  graphics_ParticleSystem_setAreaSpread(psNew, ps->areaSpreadDistribution, ps->areaSpread[0], ps->areaSpread[1]);
  graphics_ParticleSystem_setEmitterLifetime(psNew, ps->lifetime);
  graphics_ParticleSystem_setParticleLifetime(psNew, ps->particleLifeMin, ps->particleLifeMax);
  graphics_ParticleSystem_setDirection(psNew, ps->direction);
  graphics_ParticleSystem_setSpread(psNew, ps->spread);
  graphics_ParticleSystem_setSpeed(psNew, ps->speedMin, ps->speedMax);
  graphics_ParticleSystem_setLinearAcceleration(
    psNew,
    ps->linearAccelerationMin[0], ps->linearAccelerationMin[1], 
    ps->linearAccelerationMax[0], ps->linearAccelerationMax[1]);
  graphics_ParticleSystem_setRadialAcceleration(psNew, ps->radialAccelerationMin, ps->radialAccelerationMax);
  graphics_ParticleSystem_setTangentialAcceleration(psNew, ps->tangentialAccelerationMin, ps->tangentialAccelerationMax);
  graphics_ParticleSystem_setLinearDamping(psNew, ps->linearDampingMin, ps->linearDampingMax);
  graphics_ParticleSystem_setSizes(psNew, ps->sizeCount, ps->sizes);
  graphics_ParticleSystem_setSizeVariation(psNew, ps->sizeVariation);
  graphics_ParticleSystem_setRotation(psNew, ps->rotationMin, ps->rotationMax);
  graphics_ParticleSystem_setSpin(psNew, ps->spinStart, ps->spinEnd);
  graphics_ParticleSystem_setSpinVariation(psNew, ps->spinVariation);
  graphics_ParticleSystem_setOffset(psNew, ps->offsetX, ps->offsetY);
  graphics_ParticleSystem_setColors(psNew, ps->colorCount, ps->colors);
  graphics_ParticleSystem_setQuads(psNew, ps->quadCount, ps->quads);
  graphics_ParticleSystem_setRelativeRotation(psNew, ps->relativeRotation);
}


void graphics_ParticleSystem_setLinearDamping(graphics_ParticleSystem *ps, float min, float max) {
  ps->linearDampingMin = min;
  ps->linearDampingMax = max;
}


void graphics_ParticleSystem_getLinearDamping(graphics_ParticleSystem const *ps, float *min, float *max) {
  *min = ps->linearDampingMin;
  *max = ps->linearDampingMax;
}


void graphics_ParticleSystem_free(graphics_ParticleSystem *ps) {
  graphics_Batch_free(&ps->batch);
  free(ps->pMem);
  free(ps->colors);
  free(ps->quads);
  free(ps->sizes);
}


void graphics_ParticleSystem_setBufferSize(graphics_ParticleSystem *ps, size_t size) {
  ps->pMem = malloc(sizeof(graphics_Particle) * size);
  ps->pFree = ps->pMem;
  ps->maxParticles = size;
}


size_t graphics_ParticleSystem_getBufferSize(graphics_ParticleSystem const *ps) {
  return ps->maxParticles;
}


void graphics_ParticleSystem_setAreaSpread(graphics_ParticleSystem *ps, graphics_AreaSpreadDistribution mode, float dx, float dy) {
  ps->areaSpreadDistribution = mode;
  ps->areaSpread[0] = dx;
  ps->areaSpread[1] = dy;
}


void graphics_ParticleSystem_getAreaSpread(graphics_ParticleSystem const *ps, graphics_AreaSpreadDistribution *mode, float *dx, float *dy) {
  *mode = ps->areaSpreadDistribution;
  *dx   = ps->areaSpread[0];
  *dy   = ps->areaSpread[1];
}


void graphics_ParticleSystem_setColors(graphics_ParticleSystem *ps, size_t count, graphics_Color const *colors) {
  size_t size = sizeof(graphics_Color) * count;
  if(count != ps->colorCount) {
    free(ps->colors);
    ps->colors = malloc(size);
  }

  memcpy(ps->colors, colors, size);
  ps->colorCount = count;
}


graphics_Color const* graphics_ParticleSystem_getColors(graphics_ParticleSystem const *ps, size_t *count) {
  *count = ps->colorCount;
  return ps->colors;
}


size_t graphics_ParticleSystem_getCount(graphics_ParticleSystem const *ps) {
  return ps->activeParticles;
}

void graphics_ParticleSystem_setDirection(graphics_ParticleSystem *ps, float dir) {
  ps->direction = dir;
}

void graphics_ParticleSystem_setEmissionRate(graphics_ParticleSystem *ps, float rate) {
  ps->emissionRate = rate;
}

void graphics_ParticleSystem_setEmitterLifetime(graphics_ParticleSystem *ps, float lifetime) {
  ps->lifetime = lifetime;
}

void graphics_ParticleSystem_setSizeVariation(graphics_ParticleSystem *ps, float variation) {
  ps->sizeVariation = variation;
}

void graphics_ParticleSystem_setSpinVariation(graphics_ParticleSystem *ps, float variation) {
  ps->spinVariation = variation;
}

void graphics_ParticleSystem_setSpread(graphics_ParticleSystem *ps, float spread) {
  ps->spread = spread;
}


float graphics_ParticleSystem_getDirection(graphics_ParticleSystem const *ps) {
  return ps->direction;
}

float graphics_ParticleSystem_getEmissionRate(graphics_ParticleSystem const *ps) {
  return ps->emissionRate;
}

float graphics_ParticleSystem_getEmitterLifetime(graphics_ParticleSystem const *ps) {
  return ps->lifetime;
}

float graphics_ParticleSystem_getSizeVariation(graphics_ParticleSystem const *ps) {
  return ps->sizeVariation;
}

float graphics_ParticleSystem_getSpinVariation(graphics_ParticleSystem const *ps) {
  return ps->spinVariation;
}

float graphics_ParticleSystem_getSpread(graphics_ParticleSystem const *ps) {
  return ps->spread;
}


void graphics_ParticleSystem_setOffset(graphics_ParticleSystem *ps, float x, float y) {
  ps->offsetX = x;
  ps->offsetY = y;
}

void graphics_ParticleSystem_setParticleLifetime(graphics_ParticleSystem *ps, float min, float max) {
  ps->particleLifeMin = min;
  ps->particleLifeMax = max;
}

void graphics_ParticleSystem_setPosition(graphics_ParticleSystem *ps, float x, float y) {
  ps->prevPosition[0] = ps->position[0] = x;
  ps->prevPosition[1] = ps->position[1] = y;
}

void graphics_ParticleSystem_setRadialAcceleration(graphics_ParticleSystem *ps, float min, float max) {
  ps->radialAccelerationMin = min;
  ps->radialAccelerationMax = max;
}

void graphics_ParticleSystem_setRotation(graphics_ParticleSystem *ps, float min, float max) {
  ps->rotationMin = min;
  ps->rotationMax = max;
}

void graphics_ParticleSystem_setSpeed(graphics_ParticleSystem *ps, float min, float max) {
  ps->speedMin = min;
  ps->speedMax = max;
}

void graphics_ParticleSystem_setSpin(graphics_ParticleSystem *ps, float min, float max) {
  ps->spinStart = min;
  ps->spinEnd   = max;
}

void graphics_ParticleSystem_setTangentialAcceleration(graphics_ParticleSystem *ps, float min, float max) {
  ps->tangentialAccelerationMin = min;
  ps->tangentialAccelerationMax = max;
}


void graphics_ParticleSystem_getOffset(graphics_ParticleSystem const *ps, float *x, float *y) {
  *x = ps->offsetX;
  *y = ps->offsetY;
}

void graphics_ParticleSystem_getParticleLifetime(graphics_ParticleSystem const *ps, float *min, float *max) {
  *min = ps->particleLifeMin;
  *max = ps->particleLifeMax;
}

void graphics_ParticleSystem_getPosition(graphics_ParticleSystem const *ps, float *x, float *y) {
  *x = ps->position[0];
  *y = ps->position[1];
}

void graphics_ParticleSystem_getRadialAcceleration(graphics_ParticleSystem const *ps, float *min, float *max) {
  *min = ps->radialAccelerationMin;
  *max = ps->radialAccelerationMax;
}

void graphics_ParticleSystem_getRotation(graphics_ParticleSystem const *ps, float *min, float *max) {
  *min = ps->rotationMin;
  *max = ps->rotationMax;
}

void graphics_ParticleSystem_getSpeed(graphics_ParticleSystem const *ps, float *min, float *max) {
  *min = ps->speedMin;
  *max = ps->speedMax;
}

void graphics_ParticleSystem_getSpin(graphics_ParticleSystem const *ps, float *min, float *max) {
  *min = ps->spinStart;
  *max = ps->spinEnd;
}

void graphics_ParticleSystem_getTangentialAcceleration(graphics_ParticleSystem const *ps, float *min, float *max) {
  *min = ps->tangentialAccelerationMin;
  *max = ps->tangentialAccelerationMax;
}


void graphics_ParticleSystem_setTexture(graphics_ParticleSystem *ps, graphics_Image const* texture) {
  ps->texture = texture;
}


graphics_Image const* graphics_ParticleSystem_getTexture(graphics_ParticleSystem const *ps) {
  return ps->texture;
}


float const *graphics_ParticleSystem_getSizes(graphics_ParticleSystem const *ps, size_t *count) {
  *count = ps->sizeCount;
  return ps->sizes;
}

void graphics_ParticleSystem_setSizes(graphics_ParticleSystem *ps, size_t count, float const *sizes) {
  if(ps->sizeCount != count) {
    free(ps->sizes);
    ps->sizes = malloc(sizeof(float) * count);
    ps->sizeCount = count;
  }

  memcpy(ps->sizes, sizes, sizeof(float) * count);
}

void graphics_ParticleSystem_setQuads(graphics_ParticleSystem *ps, size_t count, graphics_Quad const * const *quads) {
  if(ps->quadCount != count) {
    free(ps->quads);
    ps->quads = malloc(sizeof(graphics_Quad*) * count);
    ps->quadCount = count;
  }

  memcpy(ps->quads, quads, sizeof(graphics_Quad*) * count);
}


void graphics_ParticleSystem_setRelativeRotation(graphics_ParticleSystem *ps, bool enable) {
  ps->relativeRotation = enable;
}


bool graphics_ParticleSystem_hasRelativeRotation(graphics_ParticleSystem const *ps) {
  return ps->relativeRotation;
}

void graphics_ParticleSystem_setInsertMode(graphics_ParticleSystem *ps, graphics_ParticleInsertMode mode) {
  ps->insertMode = mode;
}

graphics_ParticleInsertMode graphics_ParticleSystem_getInsertMode(graphics_ParticleSystem const *ps) {
  return ps->insertMode;
}

bool graphics_ParticleSystem_isActive(graphics_ParticleSystem const *ps) {
  return ps->active;
}

bool graphics_ParticleSystem_isPaused(graphics_ParticleSystem const *ps) {
  return !ps->active && ps->life < ps->lifetime;
}

bool graphics_ParticleSystem_isStopped(graphics_ParticleSystem const *ps) {
  return !ps->active && ps->life >= ps->lifetime;
}

void graphics_ParticleSystem_moveTo(graphics_ParticleSystem *ps, float x, float y) {
  memcpy(ps->prevPosition, ps->position, sizeof(ps->position));
  ps->position[0] = x;
  ps->position[1] = y;
}

void graphics_ParticleSystem_emit(graphics_ParticleSystem *ps, size_t count) {
  if(!ps->active) {
    return;
  }

  size_t num = min(count, ps->maxParticles - ps->activeParticles);
  while(num) {
    addParticle(ps, 1.0f);
    --num;
  }
}

void graphics_ParticleSystem_start(graphics_ParticleSystem *ps) {
  ps->active = true;
}

void graphics_ParticleSystem_stop(graphics_ParticleSystem *ps) {
  ps->active = false;
  ps->life = ps->lifetime;
  ps->emitCounter = 0;
}

void graphics_ParticleSystem_reset(graphics_ParticleSystem *ps) {
  ps->pFree = ps->pMem;
  ps->pHead = 0;
  ps->pTail = 0;
  ps->activeParticles = 0;
  ps->life = ps->lifetime;
  ps->emitCounter = 0;
}


void graphics_ParticleSystem_setLinearAcceleration(graphics_ParticleSystem *ps, float xmin, float ymin, float xmax, float ymax) {
  ps->linearAccelerationMin[0] = xmin;
  ps->linearAccelerationMin[1] = ymin;
  ps->linearAccelerationMax[0] = xmax;
  ps->linearAccelerationMax[1] = ymax;
}


void graphics_ParticleSystem_getLinearAcceleration(graphics_ParticleSystem const *ps, float *xmin, float *ymin, float *xmax, float *ymax) {
  *xmin = ps->linearAccelerationMin[0];
  *ymin = ps->linearAccelerationMin[1];
  *xmax = ps->linearAccelerationMax[0];
  *ymax = ps->linearAccelerationMax[1];
}

void graphics_ParticleSystem_pause(graphics_ParticleSystem *ps) {
  ps->active = false;
}

void graphics_ParticleSystem_update(graphics_ParticleSystem *ps, float dt) {
  if(ps->pMem == 0 || dt == 0.0f) {
    return;
  }

  graphics_Particle *p = ps->pHead;

  while(p) {
    p->life -= dt;

    if(p->life <= 0.0f) {
      p = removeParticle(ps, p);
      continue;
    }

    float radialX = p->position[0] - p->origin[0];
    float radialY = p->position[1] - p->origin[1];
    normalizeInPlace(&radialX, &radialY);
    float tangentialX = -radialY * p->tangentialAcceleration;
    float tangentialY =  radialX * p->tangentialAcceleration;

    radialX *= p->radialAcceleration;
    radialY *= p->radialAcceleration;

    p->velocity[0] += (radialX + tangentialX + p->linearAcceleration[0]) * dt;
    p->velocity[1] += (radialY + tangentialY + p->linearAcceleration[1]) * dt;

    p->velocity[0] *= 1.0f / (1.0f + p->linearDamping * dt);
    p->velocity[1] *= 1.0f / (1.0f + p->linearDamping * dt);
    
    p->position[0] += p->velocity[0] * dt;
    p->position[1] += p->velocity[1] * dt;

    float const t = 1.0f - p->life / p->lifetime;
    p->rotation += lerp(p->spinStart, p->spinEnd, t) * dt;
    p->angle = p->rotation;

    if(ps->relativeRotation) {
      p->angle += atan2(p->velocity[1], p->velocity[0]);
    }

    float s = (p->sizeOffset + t * p->sizeIntervalSize) * (float)(ps->sizeCount - 1);
    size_t i = (size_t)s;
    size_t k = (i == ps->sizeCount - 1) ? i : i + 1;
    s -= (float)i;
    p->size = lerp(ps->sizes[i], ps->sizes[k], s);

    s = t * (float)(ps->colorCount - 1);
    i = (size_t)s;
    k = (i == ps->colorCount - 1) ? i : i + 1;
    s -= (float)i;

    p->color.red   = lerp(ps->colors[i].red,   ps->colors[k].red,   s);
    p->color.green = lerp(ps->colors[i].green, ps->colors[k].green, s);
    p->color.blue  = lerp(ps->colors[i].blue,  ps->colors[k].blue,  s);
    p->color.alpha = lerp(ps->colors[i].alpha, ps->colors[k].alpha, s);

    k = ps->quadCount;
    if(k > 0) {
      s = t * (float)k;
      i = (s > 0.0f) ? (size_t) s : 0;
      p->quadIndex = (i < k) ? i : k - 1;
    }

    p = p->next;
  }

  if(ps->active) {
    float rate = 1.0f / ps->emissionRate;
    ps->emitCounter += dt;
    float total = ps->emitCounter - rate;
    while(ps->emitCounter > rate) {
      addParticle(ps, 1.0f - (ps->emitCounter - rate) / total);
      ps->emitCounter -= rate;
    }

    ps->life -= dt;
    if(ps->lifetime != -1 && ps->life < 0) {
      graphics_ParticleSystem_stop(ps);
    }
  }

  memcpy(ps->prevPosition, ps->position, sizeof(ps->position));
}

void graphics_ParticleSystem_draw(graphics_ParticleSystem *ps, float x, float y, float r, float sx, float sy, float ox, float oy, float kx, float ky) {
  //printf("Drawing particle system, %d particles\n", ps->activeParticles);

  graphics_Batch *b = &ps->batch;
  //graphics_Batch_bind(b);
  graphics_Batch_clear(b);
  for(graphics_Particle *p = ps->pHead; p; p = p->next) {
    // TODO move the following back to the draw function!
    graphics_Quad const* q = ps->quads[p->quadIndex];
    graphics_Batch_setColor(b, p->color.red, p->color.green, p->color.blue, p->color.alpha);
    graphics_Batch_add(b, q, p->position[0], p->position[1], p->angle, p->size, p->size, ps->offsetX, ps->offsetY, 0.0f, 0.0f);
  }
  graphics_Batch_flush(b);
  graphics_Batch_draw(&ps->batch, x, y, r, sx, sy, ox, oy, kx, ky);
}


void graphics_particlesystem_init() {
  math_RandomGenerator_init(&moduleData.rng);
}
