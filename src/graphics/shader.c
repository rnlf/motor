#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "shader.h"
#include "../3rdparty/slre/slre.h"


static struct {
  graphics_Shader *activeShader;
  graphics_Shader defaultShader;
  int maxTextureUnits;
} moduleData;

GLchar const *defaultVertexSource = 
  "vec4 position(mat4 transform_projection, vec4 vertex_position) {\n"
  "  return transform_projection * vertex_position;\n"
  "}\n";

static GLchar const vertexHeader[] =
  "uniform   mat4 motor2d_transform;\n"  
  "uniform   mat4 motor2d_projection;\n"
  "uniform   mat2 motor2d_textureRect;\n"
  "uniform   vec2 motor2d_size;\n"
  "uniform   bool motor2d_useVertexColor;\n"
  "#define extern uniform\n"
  "#define number float\n"
  "attribute vec2 motor2d_vPos;\n"
  "attribute vec2 motor2d_vUV;\n"
  "attribute vec4 motor2d_vColor;\n"
  "varying   vec2 motor2d_fUV;\n"
  "varying   vec4 motor2d_fColor;\n"
  "#line 0\n";

static GLchar const vertexFooter[] =
  "void main() {\n"
  "  gl_Position = position(motor2d_projection * motor2d_transform, vec4(motor2d_vPos * motor2d_size, 1.0, 1.0));\n"
  "  motor2d_fUV = motor2d_vUV * motor2d_textureRect[1] + motor2d_textureRect[0];\n"
  "  if(useVertexColor) {\n"
  "    motor2d_fColor = motor2d_vColor;\n"
  "  } else {\n"
  "    motor2d_fColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
  "  }\n"
  "}\n";

static GLchar const *defaultFragmentSource =
  "vec4 effect( vec4 color, Image texture, vec2 texture_coords, vec2 screen_coords ) {\n"
  "  return Texel(texture, texture_coords) * color;\n"
  "}\n";

#define DEFAULT_SAMPLER "motor2d_tex"

static GLchar const fragmentHeader[] = 
  "precision mediump float;\n"
  "#define Image sampler2D\n"
  "#define Texel texture2D\n"
  "#define extern uniform\n"
  "#define number float\n"
  "varying vec2 motor2d_fUV;\n"
  "varying vec4 motor2d_fColor;\n"
  "uniform sampler2D " DEFAULT_SAMPLER ";\n"
  "uniform vec4 motor2d_color;\n"
  "#line 0\n";

static GLchar const fragmentFooter[] =
  "void main() {\n"
  "  gl_FragColor = effect(motor2d_color * motor2d_fColor, " DEFAULT_SAMPLER ", motor2d_fUV, vec2(0.0, 0.0));\n"
  "}\n";

bool graphics_Shader_compileAndAttachShaderRaw(graphics_Shader *program, GLenum shaderType, char const* code) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, (GLchar const **)&code, 0);
  glCompileShader(shader);

  glAttachShader(program->program, shader);

  int state;
  int infolen;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &state);
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolen);

  char *info = malloc(infolen);
  glGetShaderInfoLog(shader, infolen, 0, info);
  switch(shaderType) {
  case GL_VERTEX_SHADER:
    free(program->warnings.vertex);
    program->warnings.vertex = info;
    break;

  case GL_FRAGMENT_SHADER:
    free(program->warnings.fragment);
    program->warnings.fragment = info;
    break;
  }

  glDeleteShader(shader);
  return state;
}

bool graphics_Shader_compileAndAttachShader(graphics_Shader *shader, GLenum shaderType, char const* code) {
  GLchar const* header;
  GLchar const* footer;
  int headerlen;
  int footerlen;
  switch(shaderType) {
  case GL_VERTEX_SHADER:
    header = vertexHeader;
    headerlen = sizeof(vertexHeader) - 1;
    footer = vertexFooter;
    footerlen = sizeof(vertexFooter) - 1;
    break;
  case GL_FRAGMENT_SHADER:
    header = fragmentHeader;
    headerlen = sizeof(fragmentHeader) - 1;
    footer = fragmentFooter;
    footerlen = sizeof(fragmentFooter) - 1;
    break;
  }
  int codelen = strlen(code);
  GLchar *combinedCode = malloc(headerlen + footerlen + codelen + 1);
  memcpy(combinedCode, header, headerlen);
  memcpy(combinedCode + headerlen, (GLchar const*)code, codelen);
  memcpy(combinedCode + headerlen + codelen, footer, footerlen+1); // include zero terminator

  bool state = graphics_Shader_compileAndAttachShaderRaw(shader, shaderType, combinedCode);

  free(combinedCode);

  return state;
}

static int compareUniformInfo(graphics_ShaderUniformInfo const* a, graphics_ShaderUniformInfo const* b) {
  return strcmp(a->name, b->name);
}

int graphics_shader_toMotorComponents(GLenum type) {
  switch(type) {
  case GL_BOOL:
  case GL_INT:
  case GL_FLOAT:
    return 1;

  case GL_BOOL_VEC2:
  case GL_INT_VEC2:
  case GL_FLOAT_VEC2:
  case GL_FLOAT_MAT2:
    return 2;

  case GL_BOOL_VEC3:
  case GL_INT_VEC3:
  case GL_FLOAT_VEC3:
  case GL_FLOAT_MAT3:
    return 3;


  case GL_BOOL_VEC4:
  case GL_INT_VEC4:
  case GL_FLOAT_VEC4:
  case GL_FLOAT_MAT4:
    return 4;

  default:
    return 0;
  };
}

graphics_ShaderUniformType graphics_shader_toMotorType(GLenum type) {
  switch(type) {
  case GL_BOOL:
  case GL_BOOL_VEC2:
  case GL_BOOL_VEC3:
  case GL_BOOL_VEC4:
    return graphics_ShaderUniformType_bool;

  case GL_INT:
  case GL_INT_VEC2:
  case GL_INT_VEC3:
  case GL_INT_VEC4:
    return graphics_ShaderUniformType_int;

  case GL_FLOAT:
  case GL_FLOAT_VEC2:
  case GL_FLOAT_VEC3:
  case GL_FLOAT_VEC4:
  case GL_FLOAT_MAT2:
  case GL_FLOAT_MAT3:
  case GL_FLOAT_MAT4:
    return graphics_ShaderUniformType_float;

  case GL_SAMPLER_2D:
    return graphics_ShaderUniformType_sampler;

  default:
    return graphics_ShaderUniformType_none;
  };
}

static void readShaderUniforms(graphics_Shader *shader) {
  shader->uniformLocations.transform   = glGetUniformLocation(shader->program, "motor2d_transform");
  shader->uniformLocations.projection  = glGetUniformLocation(shader->program, "motor2d_projection");
  shader->uniformLocations.textureRect = glGetUniformLocation(shader->program, "motor2d_textureRect");
  shader->uniformLocations.tex         = glGetUniformLocation(shader->program, DEFAULT_SAMPLER);
  shader->uniformLocations.color       = glGetUniformLocation(shader->program, "motor2d_color");
  shader->uniformLocations.size        = glGetUniformLocation(shader->program, "motor2d_size");
  shader->uniformLocations.useVertCol  = glGetUniformLocation(shader->program, "motor2d_useVertexColor");

  int maxLength;
  glGetProgramiv(shader->program, GL_ACTIVE_UNIFORMS, &shader->uniformCount);
  glGetProgramiv(shader->program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);

  // This wastes some memory, but it's also relatively fast
  shader->uniforms = (graphics_ShaderUniformInfo*)malloc(sizeof(graphics_ShaderUniformInfo) * shader->uniformCount);
  for(int i = 0; i < shader->uniformCount; ++i) {
    graphics_ShaderUniformInfo* info = shader->uniforms+i;
    info->name = (char*)malloc(maxLength);
    glGetActiveUniform(shader->program, i, maxLength, NULL, &info->elements, &info->type, info->name);

    info->location = glGetUniformLocation(shader->program, info->name);
    info->extra = 0;

    char *suffix = strstr(info->name, "[0]");
    if(suffix) {
      *suffix = 0;
    }
  }

  qsort(shader->uniforms, shader->uniformCount, sizeof(graphics_ShaderUniformInfo), (int(*)(void const*,void const*))compareUniformInfo);
}


static void allocateTextureUnits(graphics_Shader *shader) {
  shader->textureUnitCount = 0;
  for(int i = 0; i < shader->uniformCount; ++i) {
    if(shader->uniforms[i].type == GL_SAMPLER_2D) {
      if(strcmp(shader->uniforms[i].name, DEFAULT_SAMPLER)) {
        ++shader->textureUnitCount;
      }
    }
  }  

  shader->textureUnits = malloc(sizeof(graphics_ShaderTextureUnitInfo) * shader->textureUnitCount);

  int currentUnit = 0;
  glUseProgram(shader->program);
  for(int i = 0; i < shader->uniformCount; ++i) {
    if(shader->uniforms[i].type == GL_SAMPLER_2D) {
      if(strcmp(shader->uniforms[i].name, DEFAULT_SAMPLER)) {
        glUniform1i(shader->uniforms[i].location, currentUnit+1);
        shader->uniforms[i].extra = shader->textureUnits + currentUnit;
        shader->textureUnits[currentUnit].unit = currentUnit + 1;
        shader->textureUnits[currentUnit].boundTexture = 0;
        ++currentUnit;
      }
    }
  }
}


graphics_ShaderCompileStatus graphics_Shader_new(graphics_Shader *shader, char const* vertexCode, char const* fragmentCode) {

  memset(shader, 0, sizeof(*shader));
  shader->warnings.vertex = malloc(1);
  shader->warnings.fragment = malloc(1);
  shader->warnings.program = malloc(1);
  *shader->warnings.vertex = *shader->warnings.fragment = *shader->warnings.program = 0;

  if(!vertexCode) {
    vertexCode = defaultVertexSource;
  }

  if(!fragmentCode) {
    fragmentCode = defaultFragmentSource;
  }

  shader->program = glCreateProgram();

  if(!graphics_Shader_compileAndAttachShader(shader, GL_VERTEX_SHADER, vertexCode)) {
    return graphics_ShaderCompileStatus_vertexError;
  }

  if(!graphics_Shader_compileAndAttachShader(shader, GL_FRAGMENT_SHADER, fragmentCode)) {
    return graphics_ShaderCompileStatus_fragmentError;
  }

  glBindAttribLocation(shader->program, 0, "motor2d_vPos");
  glBindAttribLocation(shader->program, 1, "motor2d_vUV");
  glBindAttribLocation(shader->program, 2, "motor2d_vColor");
  glLinkProgram(shader->program);

  int linkState;
  int linkInfoLen;
  glGetProgramiv(shader->program, GL_LINK_STATUS, &linkState);
  glGetProgramiv(shader->program, GL_INFO_LOG_LENGTH, &linkInfoLen);
  shader->warnings.program = realloc(shader->warnings.program, linkInfoLen);
  glGetProgramInfoLog(shader->program, linkInfoLen, 0, shader->warnings.program);

  if(!linkState) {
    return graphics_ShaderCompileStatus_linkError;
  }

  readShaderUniforms(shader);

  allocateTextureUnits(shader);

  return graphics_ShaderCompileStatus_okay;
}


void graphics_Shader_free(graphics_Shader* shader) {
  glDeleteProgram(shader->program);

  for(int i = 0; i < shader->uniformCount; ++i) {
    free(shader->uniforms[i].name);
  }
  free(shader->textureUnits);
  free(shader->uniforms);
}

void graphics_Shader_activate(mat4x4 const* projection, mat4x4 const* transform, graphics_Quad const* textureRect, float const* useColor, float ws, float hs) {

  glUseProgram(moduleData.activeShader->program);

  float s[2] = { ws, hs };

  glUniform1i(       moduleData.activeShader->uniformLocations.tex,               0);
  glUniformMatrix4fv(moduleData.activeShader->uniformLocations.projection,  1, 0, (GLfloat const*)projection);
  glUniformMatrix2fv(moduleData.activeShader->uniformLocations.textureRect, 1, 0, (GLfloat const*)textureRect);
  glUniform4fv(      moduleData.activeShader->uniformLocations.color,       1,                    useColor);
  glUniform2fv(      moduleData.activeShader->uniformLocations.size,        1,                    s);
  glUniformMatrix4fv(moduleData.activeShader->uniformLocations.transform,   1, 0, (GLfloat const*)transform);
  glUniform1i(       moduleData.activeShader->uniformLocations.useVertCol,  useColor == 0);

  for(int i = 0; i < moduleData.activeShader->textureUnitCount; ++i) {
    glActiveTexture(GL_TEXTURE0 + moduleData.activeShader->textureUnits[i].unit);
    glBindTexture(GL_TEXTURE_2D, moduleData.activeShader->textureUnits[i].boundTexture);
  }
}

void graphics_setDefaultShader(void) {
  moduleData.activeShader = &moduleData.defaultShader;
}

void graphics_setShader(graphics_Shader* shader) {
  moduleData.activeShader = shader;
}

graphics_Shader* graphics_getShader(void) {
  return moduleData.activeShader;
}

void graphics_shader_init(void) {
  graphics_Shader_new(&moduleData.defaultShader, NULL, NULL);
  moduleData.activeShader = &moduleData.defaultShader;
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &moduleData.maxTextureUnits);
}

#define mkScalarSendFunc(name, type, glfunc) \
  void graphics_Shader_ ## name(graphics_Shader *shader, graphics_ShaderUniformInfo const* info, int count, type const* numbers) {  \
    glUseProgram(shader->program); \
    glfunc(info->location, count, numbers); \
  }

mkScalarSendFunc(sendIntegers, GLint,   glUniform1iv)
mkScalarSendFunc(sendBooleans, GLint,   glUniform1iv)
mkScalarSendFunc(sendFloats,   GLfloat, glUniform1fv)

#undef mkScalarSendFunc

#define mkVectorSendFunc(name, valuetype, abbr) \
  void graphics_Shader_ ## name(graphics_Shader *shader, graphics_ShaderUniformInfo const* info, int count, valuetype const* numbers) {  \
    glUseProgram(shader->program);                                \
    switch(graphics_shader_toMotorComponents(info->type)) {       \
    case 2:                                                       \
      glUniform2 ## abbr ## v(info->location, count, numbers);    \
      break;                                                      \
    case 3:                                                       \
      glUniform3 ## abbr ## v(info->location, count, numbers);    \
      break;                                                      \
    case 4:                                                       \
      glUniform4 ## abbr ## v(info->location, count, numbers);    \
      break;                                                      \
    }                                                             \
  }

mkVectorSendFunc(sendIntegerVectors, GLint,   i)
mkVectorSendFunc(sendBooleanVectors, GLint,   i)
mkVectorSendFunc(sendFloatVectors,   GLfloat, f)

#undef mkVectorSendFunc

void graphics_Shader_sendFloatMatrices(graphics_Shader *shader, graphics_ShaderUniformInfo const* info, int count, float const* numbers) {
  glUseProgram(shader->program);

  switch(graphics_shader_toMotorComponents(info->type)) {
  case 2:
    glUniformMatrix2fv(info->location, count, false, numbers);
    break;

  case 3:
    glUniformMatrix3fv(info->location, count, false, numbers);
    break;

  case 4:
    glUniformMatrix4fv(info->location, count, false, numbers);
    break;
  }
}


void graphics_Shader_sendTexture(graphics_Shader *shader, graphics_ShaderUniformInfo const* info, GLuint texture) {
  graphics_ShaderTextureUnitInfo *unit = (graphics_ShaderTextureUnitInfo*)info->extra;
  unit->boundTexture = texture;
}


graphics_ShaderUniformInfo const* graphics_Shader_getUniform(graphics_Shader const* shader, char const* name) {
  // Dirty trick to avoid duplicate code: Name will be treated as graphics_ShaderUniformInfo.
  return bsearch(&name, shader->uniforms, shader->uniformCount, sizeof(graphics_ShaderUniformInfo), (int(*)(void const*, void const*))compareUniformInfo);
}
