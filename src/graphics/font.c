#define _POSIX_SOURCE
#include <string.h>
#include <tgmath.h>
#include "font.h"
#include <stdlib.h>
#include "quad.h"
#include "graphics.h"
#include "../tools/utf8.h"
#include "../math/minmax.h"
#include "shader.h"
#include "batch.h"
#include "vera_ttf.c"
#include "../filesystem/filesystem.h"

#include FT_GLYPH_H

static const int GlyphTexturePadding = 1;

typedef struct {
  char const* ptr;
  int len;
  int width;
} Word;

static struct {
  FT_Library ft;
  Word *line;
  int wordcount;
  graphics_Batch *batches;
  int batchcount;
  int batchsize;
} moduleData;


void graphics_GlyphMap_newTexture(graphics_GlyphMap *map) {
  map->textures = realloc(map->textures, sizeof(GLuint) * (map->numTextures + 1));
  glGenTextures(1, &map->textures[map->numTextures]);
  glBindTexture(GL_TEXTURE_2D, map->textures[map->numTextures]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, map->textureWidth, map->textureHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  if(map->numTextures > 0) {
    graphics_Filter filter;
    graphics_Texture_getFilter(map->textures[map->numTextures-1], &filter);
    graphics_Texture_setFilter(map->textures[map->numTextures], &filter);
  }
  ++map->numTextures;

}

void graphics_GlyphMap_free(graphics_GlyphMap* map) {
  glDeleteTextures(map->numTextures, map->textures);
  free(map->textures);
}

graphics_Glyph const* graphics_Font_findGlyph(graphics_Font *font, unsigned unicode) {
  // GlyphMap is a very simple hash table that stores an array of known glyphs with
  // the same lower 8 bit unicode code point. The array is kept sorted in ascending order.
  // The idea is, that for most western languages, these arrays will probably be only 1 or 2
  // entries long, each. This means finding the glyph requires almost constant time for such
  // languages.

  // TODO error handling if missing completely

  unsigned idx = unicode & 0xFF;
  graphics_GlyphSet *set = &font->glyphs.glyphs[idx];

  // Do we already have the glyph?
  int i = 0;
  for(; i < set->numGlyphs; ++i) {
    graphics_Glyph const* glyph = &set->glyphs[i];
    // The list is sorted, break early if possible
    if(glyph->code > unicode) {
      break;
    } else if(glyph->code == unicode) {
      return glyph;
    }
  }

  // Not found -> insert before last checked position
  if(i > 0)
    --i;

  // Allocate new glyphs, copy old glyphs over and make space for the new one in the
  // right position (leave space at insert point)
  graphics_Glyph * newGlyphs = malloc(sizeof(graphics_Glyph) * (set->numGlyphs+1));
  memcpy(newGlyphs, set->glyphs, i * sizeof(graphics_Glyph));
  memcpy(newGlyphs + i + 1, set->glyphs + i, (set->numGlyphs - i) * sizeof(graphics_Glyph));
  free((void*)set->glyphs);
  set->glyphs = newGlyphs;
  ++set->numGlyphs;

  // The storage for the new glyph data has been prepared in the last step,
  // we can just use it.
  graphics_Glyph * newGlyph = newGlyphs + i;

  // Load and render the glyph at the desired size
  unsigned index = FT_Get_Char_Index(font->face, unicode);
  FT_Load_Glyph(font->face, index, FT_LOAD_DEFAULT);
  FT_Glyph g;
  FT_Get_Glyph(font->face->glyph, &g);
  FT_Glyph_To_Bitmap(&g, FT_RENDER_MODE_NORMAL, 0, 1);
  FT_BitmapGlyph fg = (FT_BitmapGlyph)g;
  FT_Bitmap b = fg->bitmap;

  // Create LUMINANCE_ALPHA texture data
  // TODO: Maybe just alpha (or GL_R on modern core profiles) is
  //       enough?
  uint8_t *buf = malloc(2*b.rows*b.width);
  uint8_t *row = b.buffer;
  for(int i = 0; i < b.rows; ++i) {
    for(int c = 0; c < b.width; ++c) {
      buf[2*(i*b.width + c)    ] = 255;
      buf[2*(i*b.width + c) + 1] = row[c];
    }
    row += b.pitch;
  }

  // Is the current row in the texture too full for the glyph?
  if(font->glyphs.currentX + GlyphTexturePadding + b.width > font->glyphs.textureWidth) {
    font->glyphs.currentX = GlyphTexturePadding;
    font->glyphs.currentY += font->glyphs.currentRowHeight;
    font->glyphs.currentRowHeight = GlyphTexturePadding;
  }

  if(font->glyphs.currentY + GlyphTexturePadding + b.rows > font->glyphs.textureHeight) {
    font->glyphs.currentX = GlyphTexturePadding;
    font->glyphs.currentY = GlyphTexturePadding;
    font->glyphs.currentRowHeight = GlyphTexturePadding;
    graphics_GlyphMap_newTexture(&font->glyphs);
  }

// TODO: create new texture if current texture is full

  // Bind current texture and upload data to the appropriate position.
  // This assumes pixel unpack alignment is set to 1 (glPixelStorei)
  glBindTexture(GL_TEXTURE_2D, font->glyphs.textures[font->glyphs.numTextures-1]);
  glTexSubImage2D(GL_TEXTURE_2D, 0, font->glyphs.currentX, font->glyphs.currentY,
                  b.width, b.rows, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, buf);

  // Store geometric information for the glyph
  newGlyph->code = unicode;
  newGlyph->bearingX        = font->face->glyph->metrics.horiBearingX >> 6;
  newGlyph->bearingY        = font->face->glyph->metrics.horiBearingY >> 6;
  newGlyph->advance         = font->face->glyph->metrics.horiAdvance  >> 6;
  newGlyph->textureIdx      = font->glyphs.numTextures - 1;
  newGlyph->textureCoords.x = (float)font->glyphs.currentX / (float)font->glyphs.textureWidth;
  newGlyph->textureCoords.y = (float)font->glyphs.currentY / (float)font->glyphs.textureHeight;
  newGlyph->textureCoords.w = (float)b.width / (float)font->glyphs.textureWidth;
  newGlyph->textureCoords.h = (float)b.rows  / (float)font->glyphs.textureHeight;

  // Advance to render position for next glyph
  font->glyphs.currentX += b.width + GlyphTexturePadding;
  font->glyphs.currentRowHeight = max(font->glyphs.currentRowHeight, b.rows + GlyphTexturePadding);

  free(buf);
  FT_Done_Glyph(g);

  return newGlyph;
}

static int const TextureWidths[] =  {128, 128, 256, 256, 512,  512, 1024};
static int const TextureHeights[] = {128, 256, 256, 512, 512, 1024, 1024};
static int const TextureSizeCount = sizeof(TextureWidths) / sizeof(int);

int graphics_Font_new(graphics_Font *dst, char const* filename, int ptsize) {
  // TODO use error
  int error;
  if(filename) {
    char const* realFilename = filesystem_locateReadableFile(filename);
    FT_New_Face(moduleData.ft, realFilename, 0, &dst->face);
  } else {
    FT_New_Memory_Face(moduleData.ft, defaultFontData, defaultFontSize, 0, &dst->face);
  }
  (void)error;
  FT_Set_Pixel_Sizes(dst->face, 0, ptsize);

  memset(&dst->glyphs, 0, sizeof(graphics_GlyphMap));
  int sizeIdx = TextureSizeCount - 1;
  dst->height = dst->face->size->metrics.height >> 6;
  int estArea = dst->height * dst->height * 80;
  for(int i = 0; i < TextureSizeCount; ++i) {
    if(estArea <= TextureWidths[i] * TextureHeights[i]) {
      sizeIdx = i;
      break;
    }
  }

  dst->glyphs.textureWidth = TextureWidths[sizeIdx];
  dst->glyphs.textureHeight = TextureHeights[sizeIdx];

  graphics_GlyphMap_newTexture(&dst->glyphs);

  dst->ascent = dst->face->size->metrics.ascender >> 6;
  dst->descent = dst->face->size->metrics.descender >> 6;
  dst->lineHeight = 1.0f;

  return 0;
}

void graphics_Font_free(graphics_Font *obj) {
  FT_Done_Face(obj->face);
  graphics_GlyphMap_free(&obj->glyphs);
}


typedef struct {
  int start;
  int stop;
  int width;
} WrappedWord;

typedef struct {

} WrappedLine;

int graphics_Font_getWrap(graphics_Font * font, char const* text, int width, char ** wrapped) {
  // TODO
  return 0;
}

static void drawLine(graphics_Font* font, int x, int y, int start, int end, int rest, int spacewidth, float leftScale, float centerScale) {
  if(rest < 0)
    rest = 0;
  x += rest * leftScale;
  spacewidth += centerScale / (end - start - 1) * rest;
  for(int i = start; i < end; ++i) {
    for(int j = 0; j < moduleData.line[i].len; ++j) {
      uint32_t cp = utf8_scan(&moduleData.line[i].ptr);

      graphics_Glyph const* glyph = graphics_Font_findGlyph(font, cp);
      graphics_Batch_add(&moduleData.batches[glyph->textureIdx], &glyph->textureCoords, x+glyph->bearingX, y-glyph->bearingY, 0, 1, 1, 0,0, 0, 0);

      x += glyph->advance;
    }
    x += spacewidth;
  }
}

static void prepareBatches(graphics_Font const* font, int chars) {
  int newSize = max(chars, moduleData.batchsize);
  if(font->glyphs.numTextures > moduleData.batchcount) {
    moduleData.batches = realloc(moduleData.batches, font->glyphs.numTextures * sizeof(graphics_Batch));
    for(int i = moduleData.batchcount; i < font->glyphs.numTextures; ++i) {
      graphics_Image *img = malloc(sizeof(graphics_Image));
      img->texID = font->glyphs.textures[i];
      img->width = font->glyphs.textureWidth;
      img->height = font->glyphs.textureHeight;
      graphics_Batch_new(&moduleData.batches[i], img, newSize, graphics_BatchUsage_stream);
      graphics_Batch_bind(&moduleData.batches[i]);
    }
  }
  if(moduleData.batchsize < newSize) {
    for(int i = 0; i < moduleData.batchcount; ++i) {
      graphics_Batch_bind(&moduleData.batches[i]);
      graphics_Batch_setBufferSizeClearing(&moduleData.batches[i], newSize);
      ((graphics_Image*)moduleData.batches[i].texture)->texID = font->glyphs.textures[i];
      ((graphics_Image*)moduleData.batches[i].texture)->width = font->glyphs.textureWidth;
      ((graphics_Image*)moduleData.batches[i].texture)->height = font->glyphs.textureHeight;
    }
  } else {
    for(int i = 0; i < moduleData.batchcount; ++i) {
      graphics_Batch_bind(&moduleData.batches[i]);
      graphics_Batch_clear(&moduleData.batches[i]);
      ((graphics_Image*)moduleData.batches[i].texture)->texID = font->glyphs.textures[i];
      ((graphics_Image*)moduleData.batches[i].texture)->width = font->glyphs.textureWidth;
      ((graphics_Image*)moduleData.batches[i].texture)->height = font->glyphs.textureHeight;
    }
  }
  moduleData.batchcount = font->glyphs.numTextures;
  moduleData.batchsize = newSize;
}

void graphics_Font_render(graphics_Font* font, char const* text, int px, int py, float r, float sx, float sy, float ox, float oy, float kx, float ky) {
  prepareBatches(font, strlen(text));
  uint32_t cp;
  graphics_Shader* shader = graphics_getShader();
  graphics_setDefaultShader();
  int x = 0;
  //py += font->ascent;
  int y = font->ascent;
  while((cp = utf8_scan(&text))) {
    if(cp == '\n') {
      x = 0;
      y += floor(font->height * font->lineHeight + 0.5f);
      continue;
    }
    // This will create the glyph if required
    graphics_Glyph const* glyph = graphics_Font_findGlyph(font, cp);

    graphics_Batch_add(&moduleData.batches[glyph->textureIdx], &glyph->textureCoords, x+glyph->bearingX, y-glyph->bearingY, 0, 1, 1, 0, 0, 0, 0);

    x += glyph->advance;
  }

  for(int i = 0; i < moduleData.batchcount; ++i) {
    graphics_Batch_unbind(&moduleData.batches[i]);
    graphics_Batch_draw(&moduleData.batches[i], px, py, r, sx, sy, ox, oy, kx, ky);
  }

  graphics_setShader(shader);
}


void graphics_Font_printf(graphics_Font* font, char const* text, int px, int py, int limit, graphics_TextAlign align, float r, float sx, float sy, float ox, float oy, float kx, float ky) {
  // Very cheap estimate of upper limit for actual string length
  int count = strlen(text);
  prepareBatches(font, count);

  int currentWord = 0;

  //py += font->ascent;
  int y = font->ascent;
  int x = 0;
  char const *lastPos = text;
  int c = utf8_scan(&text);
  int currentWidth = 0;
  int spaceWidth = graphics_Font_findGlyph(font, ' ')->advance;

  graphics_Shader* shader = graphics_getShader();
  graphics_setDefaultShader();
  for(;;) {
    // Find beginning of current word, process newlines
    // in the process
    while(c == ' ' || c == '\t' || c == '\n' || c == '\0') {

      if(c == '\n' || c == '\0') {
        if(currentWord > 0) {
          // Print line, split if required

          int xword = 1;
          int width = moduleData.line[0].width;
          int start = 0;
          while(xword < currentWord) {
            int newWidth = width + spaceWidth + moduleData.line[xword].width;
            if(newWidth > limit) {
              float leftScale = 0.0f;
              float centerScale = 0.0f;
              switch(align) {
              case graphics_TextAlign_center:
                leftScale = 0.5f;
                break;
              case graphics_TextAlign_right:
                leftScale = 1.0f;
                break;
              case graphics_TextAlign_justify:
                centerScale = 1.0f;
                break;
              default:
                break;
              }
              drawLine(font, x, y, start, xword, limit - width, spaceWidth, leftScale, centerScale);
              start = xword;
              width = moduleData.line[xword].width;
              y += floor(font->height * font->lineHeight + 0.5f);
            } else {
              width = newWidth;
            }
            ++xword;
          }
          float leftScale = 0.0f;
          float centerScale = 0.0f;
          switch(align) {
          case graphics_TextAlign_center:
            leftScale = 0.5f;
            break;
          case graphics_TextAlign_right:
            leftScale = 1.0f;
            break;
          default:
            break;
          }
          drawLine(font, x, y, start, xword, limit - width, spaceWidth, leftScale, centerScale);
          y += floor(font->height * font->lineHeight + 0.5f);

          currentWord = 0;
          currentWidth = 0;
        }

        if(c == '\0') {
          graphics_setShader(shader);
          goto exitPreparation; // TODO evil? maybe, but fun!
        }
      }

      lastPos = text;
      c = utf8_scan(&text);
    }

    if(currentWord >= moduleData.wordcount) {
      moduleData.wordcount *= 2;
      moduleData.line = realloc(moduleData.line, moduleData.wordcount * sizeof(Word));
    }
    moduleData.line[currentWord].ptr = lastPos;
    moduleData.line[currentWord].len = 0;
    moduleData.line[currentWord].width = 0;

    // Find end of current word, sum up length and width
    while(c != ' ' && c != '\t' && c != '\n' && c != '\0') {
      ++moduleData.line[currentWord].len;
      graphics_Glyph const* g = graphics_Font_findGlyph(font, c);
      moduleData.line[currentWord].width += g->advance;
      c = utf8_scan(&text);
    }

    ++currentWord;
  }

exitPreparation:
  for(int i = 0; i < moduleData.batchcount; ++i) {
    graphics_Batch_unbind(&moduleData.batches[i]);
    graphics_Batch_draw(&moduleData.batches[i], px, py, r, sx, sy, ox, oy, kx, ky);
  }

  graphics_setShader(shader);
}

void graphics_font_init(void) {
  // TODO use error
  int error = FT_Init_FreeType(&moduleData.ft);
  (void)error;
  moduleData.line = malloc(sizeof(Word) * 2);
  moduleData.wordcount = 2;
  moduleData.batchcount = 0;
  moduleData.batches = NULL;
  moduleData.batchsize = 0;
}

int graphics_Font_getHeight(graphics_Font const* font) {
  return font->height;
}

int graphics_Font_getAscent(graphics_Font const* font) {
  return font->ascent;
}

int graphics_Font_getDescent(graphics_Font const* font) {
  return font->descent;
}

int graphics_Font_getBaseline(graphics_Font const* font) {
  return floor(font->height / 1.25f + 0.5f);
}

int graphics_Font_getWidth(graphics_Font * font, char const* line) {
  uint32_t uni;
  int width=0;
  while((uni = utf8_scan(&line))) {
    graphics_Glyph const* g = graphics_Font_findGlyph(font, uni);
    width += g->advance;
  }
  return width;
}

void graphics_Font_setFilter(graphics_Font *font, graphics_Filter const* filter) {
  for(int i = 0; i < font->glyphs.numTextures; i++) {
    graphics_Texture_setFilter(font->glyphs.textures[i], filter);
  }
}

void graphics_Font_getFilter(graphics_Font *font, graphics_Filter *filter) {
  graphics_Texture_getFilter(font->glyphs.textures[0], filter);
}
