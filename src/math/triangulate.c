#include <stdlib.h>
#include <stdbool.h>


typedef struct IntList {
  int num;
  struct IntList* next;
} IntList;

static struct {
  int vertexCount;
  int *next;
  int *prev;
  int *triangles;
  IntList *concavePool;
} moduleData;

typedef struct {
  float x;
  float y;
} vertex;

// Shamelessly stolen and ported to C from original Love2D

static bool isCounterClockWise(vertex const* a, vertex const* b, vertex const* c) {
  // det(b-a, c-a) >= 0?
  return (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x) >= 0;
}


static void resizeBuffers(int count) {
  if(moduleData.vertexCount < count) {
    free(moduleData.next);
    free(moduleData.prev);
    free(moduleData.triangles);
    free(moduleData.concavePool);
    moduleData.next        = malloc(sizeof(int) * count);
    moduleData.prev        = malloc(sizeof(int) * count);
    moduleData.triangles   = malloc(sizeof(int) * (count - 2) * 3);
    moduleData.concavePool = malloc(sizeof(IntList) * count);
  }
}

static bool onSameSide(vertex const* vert, vertex const * a, vertex const * b, vertex const * c) {
  float px = c->x - b->x;
  float py = c->y - b->y;
  float l = px * (vert->y - b->y) - py * (vert->x - b->x);
  float m = px * (a->y - b->y) - py * (a->x - b->x);
  return l*m >= 0;
}

static bool pointInTriangle(vertex const* vert, vertex const * a, vertex const * b, vertex const * c) {
  return onSameSide(vert,a,b,c) && onSameSide(vert,b,a,c) && onSameSide(vert,c,a,b);
}

static bool anyPointInTriangle(vertex const* vertices, IntList const *concave, vertex const *a, vertex const *b, vertex const *c) {
  while(concave) {
    vertex const * p = vertices+concave->num;
    if( (p != a) && (p != b) && (p != c) && pointInTriangle(p, a, b, c)) {
      return true;
    }
    concave = concave->next;
  }
  return false;
}

static bool isEar(vertex const* a, vertex const* b, vertex const* c, vertex const *vertices, IntList const *concave) {
  return isCounterClockWise(a,b,c) && !anyPointInTriangle(vertices, concave, a, b, c);
}


static void removeFromList(IntList **list, int value) {
  IntList *cur = *list;
  if(!cur) {
    return;
  }
  if(cur->num == value) {
    *list = (*list)->next;
    return;
  }
  IntList *prev = cur;
  cur = cur->next;
  while(cur) {
    if(cur->num == value) {
      prev->next = cur->next;
      return;
    }
    prev = cur;
    cur = cur->next;
  }
}


int math_triangulation_triangulate(float const* verts, int count, int **indices) {
  vertex const* vertices = (vertex const*)verts;

  resizeBuffers(count);
  int leftMost = 0;
  for(int i = 0; i < count; ++i) {
    moduleData.next[i] = i+1;
    moduleData.prev[i] = i-1;
    vertex const* lm = vertices+leftMost;
    vertex const* p  = vertices+i;

    if(p->x < lm->x || (p->x == lm->x && p->y < lm->y)) {
      leftMost = i;
    }
  }
  moduleData.prev[0] = count - 1;
  moduleData.next[count - 1] = 0;


  if(!isCounterClockWise(vertices+moduleData.prev[leftMost], vertices+leftMost, vertices+moduleData.next[leftMost])) {
    int *tmp = moduleData.prev;
    moduleData.prev = moduleData.next;
    moduleData.next = tmp;
  }
  
  IntList *concave = 0;
  IntList *concaveEnd = 0;
  int nextPoolEntry = 0;
  for(int i = 0; i < count; ++i) {
    if(!isCounterClockWise(vertices+moduleData.prev[i], vertices+i, vertices+moduleData.next[i])) {
      if(!concave) {
        concave = moduleData.concavePool;
        concaveEnd = concave;
      } else {
        concaveEnd->next = moduleData.concavePool + nextPoolEntry;
        concaveEnd = concaveEnd->next;
      }
      concaveEnd->next = 0;
      concaveEnd->num = i;
      ++nextPoolEntry;
    }
  }

  int current = 1;
  int skipped = 0;
  int* pushPos = moduleData.triangles;
  int remain = count;
  while(remain > 3) {
    int next = moduleData.next[current];
    int prev = moduleData.prev[current];

    vertex const * a = vertices+prev;
    vertex const * b = vertices+current;
    vertex const * c = vertices+next;

    if(isEar(a,b,c,vertices,concave)) {
      pushPos[0] = prev;
      pushPos[1] = current;
      pushPos[2] = next;
      pushPos += 3;
      moduleData.next[prev] = next;
      moduleData.prev[next] = prev;
      removeFromList(&concave, current);
      --remain;
      skipped = 0;
    } else if(++skipped > remain) {
      return -1;
    }
    current = next;
  }

  pushPos[0] = moduleData.prev[current];
  pushPos[1] = current;
  pushPos[2] = moduleData.next[current];

  *indices = moduleData.triangles;
  return count - 2;
}


static float crossVerts(float const* verts, int i, int j, int k) {
  float dx1 = verts[j*2]   - verts[i*2];
  float dy1 = verts[j*2+1] - verts[i*2+1];
  float dx2 = verts[k*2]   - verts[j*2];
  float dy2 = verts[k*2+1] - verts[j*2+1];
  return dx1 * dy2 - dy1 * dx2;
}


bool math_isConvex(float const* verts, int count) {
  int i = count - 2;
  int j = count - 1;
  int k = 0;

  float w = crossVerts(verts, i, j, k);

  while(k+1 < count) {
    i = j;
    j = k;
    ++k;

    float w2 = crossVerts(verts, i, j, k);

    if(w*w2 < 0.0f) {
      return false;
    }
  }

  return true;
}


void math_triangulation_init() {

}
