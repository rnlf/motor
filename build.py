#!/usr/bin/python3

# 
#   motor2d
#
#   Copyright (C) 2015 Florian Kesseler
#
#   This project is free software; you can redistribute it and/or modify it
#   under the terms of the MIT license. See LICENSE.md for details.
#

import sys
import os
import re
import shutil

optimize = 2
link_time_optimize = 0

sources = [
  'audio/audio.c',
  'audio/listener.c',
  'audio/source.c',
  'audio/staticbuffer.c',
  'audio/staticsource.c',
  'audio/streamsource.c',
  'audio/vorbis_decoder.c',
  'filesystem/filesystem.c',
  'graphics/batch.c',
  'graphics/canvas.c',
  'graphics/font.c',
  'graphics/geometry.c',
  'graphics/gltools.c',
  'graphics/graphics.c',
  'graphics/image.c',
  'graphics/matrixstack.c',
  'graphics/mesh.c',
  'graphics/particlesystem.c',
  'graphics/quad.c',
  'graphics/shader.c',
  'image/imagedata.c',
  'luaapi/audio.c',
  'luaapi/boot.c',
  'luaapi/event.c',
  'luaapi/math.c',
  'luaapi/filesystem.c',
  'luaapi/joystick.c',
  'luaapi/keyboard.c',
  'luaapi/math_bezier.c',
  'luaapi/motor.c',
  'luaapi/mouse.c',
  'luaapi/graphics.c',
  'luaapi/graphics_batch.c',
  'luaapi/graphics_canvas.c',
  'luaapi/graphics_font.c',
  'luaapi/graphics_geometry.c',
  'luaapi/graphics_image.c',
  'luaapi/graphics_mesh.c',
  'luaapi/graphics_particlesystem.c',
  'luaapi/graphics_quad.c',
  'luaapi/graphics_shader.c',
  'luaapi/graphics_texture.c',
  'luaapi/graphics_window.c',
  'luaapi/image.c',
  'luaapi/polygon.c',
  'luaapi/timer.c',
  'luaapi/tools.c',
  'math/bezier.c',
  'math/gamma.c',
  'math/lerp.c',
  'math/math.c',
  'math/minmax.c',
  'math/randomgenerator.c',
  'math/triangulate.c',
  'math/util.c',
  'math/vector.c',
  'errorhandler.c',
  'keyboard.c',
  'joystick.c',
  'main.c',
  'motor.c',
  'mouse.c',
  'tools/utf8.c',
  'timer/timer.c',

  # simplexnoise1234
  '3rdparty/noise1234/simplexnoise1234.c',

  # SLRE
  '3rdparty/slre/slre.c',

  # Lua
  '3rdparty/lua/src/lapi.c',
  '3rdparty/lua/src/lauxlib.c',
  '3rdparty/lua/src/lbaselib.c',
  '3rdparty/lua/src/lcode.c',
  '3rdparty/lua/src/ldblib.c',
  '3rdparty/lua/src/ldebug.c',
  '3rdparty/lua/src/ldo.c',
  '3rdparty/lua/src/ldump.c',
  '3rdparty/lua/src/lfunc.c',
  '3rdparty/lua/src/lgc.c',
  '3rdparty/lua/src/linit.c',
  '3rdparty/lua/src/liolib.c',
  '3rdparty/lua/src/llex.c',
  '3rdparty/lua/src/lmathlib.c',
  '3rdparty/lua/src/lmem.c',
  '3rdparty/lua/src/loadlib.c',
  '3rdparty/lua/src/lobject.c',
  '3rdparty/lua/src/lopcodes.c',
  '3rdparty/lua/src/loslib.c',
  '3rdparty/lua/src/lparser.c',
  '3rdparty/lua/src/lstate.c',
  '3rdparty/lua/src/lstring.c',
  '3rdparty/lua/src/lstrlib.c',
  '3rdparty/lua/src/ltable.c',
  '3rdparty/lua/src/ltablib.c',
  '3rdparty/lua/src/ltm.c',
  '3rdparty/lua/src/lundump.c',
  '3rdparty/lua/src/lvm.c',
  '3rdparty/lua/src/lzio.c',

  # FreeType
  '3rdparty/freetype/src/base/ftbitmap.c',
  '3rdparty/freetype/src/base/ftcalc.c',
  '3rdparty/freetype/src/base/ftgloadr.c',
  '3rdparty/freetype/src/base/ftglyph.c',
  '3rdparty/freetype/src/base/ftinit.c',
  '3rdparty/freetype/src/base/ftobjs.c',
  '3rdparty/freetype/src/base/ftoutln.c',
  '3rdparty/freetype/src/base/ftrfork.c',
  '3rdparty/freetype/src/base/ftstream.c',
  '3rdparty/freetype/src/base/ftsystem.c',
  '3rdparty/freetype/src/base/fttrigon.c',
  '3rdparty/freetype/src/base/ftutil.c',
  '3rdparty/freetype/src/gzip/ftgzip.c',
  '3rdparty/freetype/src/sfnt/sfnt.c',
  '3rdparty/freetype/src/smooth/smooth.c',
  '3rdparty/freetype/src/truetype/truetype.c',
]



SRCDIR = os.path.dirname(sys.argv[0]) + "/src"

ftinc = " ".join(map(lambda x: "-I" + os.path.relpath(SRCDIR) + "/3rdparty/freetype/src/" + x, ["truetype", "sfnt", "autofit", "smooth", "raster", "psaux", "psnames"])) + " -I" + os.path.relpath(SRCDIR) + "/3rdparty/freetype/include"

output = ''
CFLAGS = ''
LDFLAGS = ''
CC = ''
LD = ''


if SRCDIR == '.' or SRCDIR == '':
  print("Please build out-of-source")
  sys.exit(1)

includeregex = re.compile('^\s*#\s*include\s*"([^"]+)"\s*$')

os.system('sed -e "s/FT_CONFIG_OPTIONS_H/<ftoption.h>/" -e "s/FT_CONFIG_STANDARD_LIBRARY_H/<ftstdlib.h>/" -e "s?/undef ?#undef ?" <{srcdir}/3rdparty/freetype/builds/unix/ftconfig.in >ftconfig.h'.format(srcdir=os.path.relpath(SRCDIR)))
os.system('mkdir -p config; sed -e \'/tt_driver\\|sfnt_module\\|ft_smooth_renderer/ !d\' < {srcdir}/3rdparty/freetype/include/config/ftmodule.h >config/ftmodule.h'.format(srcdir=os.path.relpath(SRCDIR)))


def newestDependency(filename, trace=[]):
  newest = os.path.getmtime(sys.argv[0])
  with open(filename) as f:
    for line in f:
      res = includeregex.match(line)
      if res:
        dep = os.path.dirname(filename) + "/" + res.group(1)
        if os.path.exists(dep) and dep not in trace:
          newest = max(newest, os.path.getmtime(dep), newestDependency(dep, trace + [dep]))

  return newest


def makeFontFile():
  sourcefile = os.path.join(os.path.dirname(sys.argv[0]), "data", "vera.ttf")
  compiled = os.path.exists("vera_ttf.c") and os.path.getmtime("vera_ttf.c") or 0
  source   = os.path.getmtime(sourcefile)

  if compiled > source:
    return False
  
  with open(sourcefile, "rb") as datafile, open("vera_ttf.c", "w") as outputfile:
    content = bytearray(datafile.read())
    outputfile.write("static unsigned char const defaultFontData[] = {\n")

    for i in range(len(content)):
      outputfile.write("0x{:02x}, ".format(content[i]))
      if i % 16 == 15:
        outputfile.write("\n")

    outputfile.write("}};\nstatic size_t defaultFontSize = {};".format(len(content)))
  
  return True


def needsRebuild(filename):
  return not os.path.exists(getObjFilename(filename)) or \
    os.path.getmtime(SRCDIR + "/" + filename) > os.path.getmtime(getObjFilename(filename)) or \
    newestDependency(SRCDIR + "/" + filename) > os.path.getmtime(getObjFilename(filename))


def getObjFilename(filename):
  return os.path.splitext(filename)[0] + ".o"


def compile(filename):
  objfile = getObjFilename(filename)
  outdir = os.path.dirname(filename)
  if outdir == "":
    outdir = "."
  if not os.path.exists(outdir):
    print("Making directory " + outdir)
    os.makedirs(outdir)

  print("Compile {filename} -> {objfile}".format(filename=filename, objfile=objfile))
  cmd = "{CC} {CFLAGS} -c -o {objfile} {filename}".format(CC=CC, CFLAGS=CFLAGS, filename=SRCDIR + "/" + filename, objfile=objfile)
  return os.system(cmd) == 0

def build():
  global output, CFLAGS, LDFLAGS, CC, LD
  if '--native' in sys.argv:
    output = 'motor2d'
    CFLAGS = '-g -Wall -O{optimize} $(sdl2-config --cflags) -DFT2_BUILD_LIBRARY -Wall -g -std=c11 -I{ftconfig}  -I{srcdir}/3rdparty/lua/src'.format(optimize=optimize, link_time_optimize=link_time_optimize, srcdir = os.path.relpath(SRCDIR), ftconfig=".") + " " + ftinc
    LDFLAGS = '-lm $(sdl2-config --libs) -lGL -lopenal -lGLEW -lSDL2_image -g'.format(optimize=optimize, link_time_optimize=link_time_optimize)
    CC = 'clang'
    LD = 'clang'
  else:
    output = 'motor2d.js'
    CFLAGS = '-s USE_SDL=2 -s FULL_ES2=1 -O{optimize} -Wall --memory-init-file 0 --llvm-lto {link_time_optimize} -DFT2_BUILD_LIBRARY -Wall -std=c11 -I{ftconfig}  -I{srcdir}/3rdparty/lua/src'.format(optimize=optimize, link_time_optimize=link_time_optimize, srcdir = os.path.relpath(SRCDIR), ftconfig=".") + " " + ftinc
    LDFLAGS = '-s USE_SDL=2 -O{optimize} --llvm-lto {link_time_optimize} --memory-init-file 0'.format(optimize=optimize, link_time_optimize=link_time_optimize)
    CC = 'emcc'
    LD = 'emcc'

  needsLinking = False
  fontChanged = makeFontFile()
  needsLinking = needsLinking or fontChanged
  for i in sources:
    filename = i
    if needsRebuild(filename):
      if not compile(filename):
        print("Failed")
        sys.exit(1)

      needsLinking = True


  if needsLinking:
    print("Linking {output}".format(output=output))
    cmd = "{LD} {LDFLAGS} -o {outfile} {infiles}".format(LD=LD, LDFLAGS=LDFLAGS, outfile=output, infiles=" ".join(map(getObjFilename, sources)))
    if os.system(cmd) != 0:
      print("Failed")

def buildLoader():
  if '--native' in sys.argv:
    print("Cannot build native version of JS loader, you won't need one!")

  closure = os.path.join(os.path.dirname(os.path.realpath(shutil.which("emcc"))), "third_party", "closure-compiler", "compiler.jar")
  if not os.path.exists(closure):
    closure = shutil.which('closure-compiler')
  else:
    java = shutil.which("java")
    closure = "{java} -jar {closure}".format(java=java, closure=closure)

  htmlpath = os.path.join(os.path.dirname(sys.argv[0]), "html")
  os.system("echo \"var logodata = 'data:image/png;base64,$(base64 -w 0 {logo})';\" >logo.js".format(logo=os.path.join(htmlpath, "motor.png")))
  os.system("echo \"var errordata = 'data:image/png;base64,$(base64 -w 0 {logo})';\" >error.js".format(logo=os.path.join(htmlpath, "error.png")))
  cmd = "{closure} --language_in ECMASCRIPT5 --js=logo.js --js=error.js --js={html}/zip.js --js={html}/inflate.js --js={html}/zip-ext.js --js={html}/motor2dloader.js --js_output_file=motor2dloader.js".format(html=htmlpath, closure=closure)
#  cmd = "cat logo.js error.js {html}/zip.js {html}/inflate.js {html}/zip-ext.js {html}/motor2dloader.js >motor2dloader.js".format(html=htmlpath)
  os.system(cmd)
  for i in ["motor2d.html"]:
    shutil.copyfile(os.path.join(htmlpath, i), i)

def remove(f):
  if os.path.exists(f):
    os.remove(f)

def clean():
  for i in sources:
    remove(getObjFilename(i))

  remove('motor2d.js')
  remove('motor2d')

  try:
    for i in extra_output:
      remove(i)
  except NameError:
    pass

def usage():
  print(sys.argv[0] + " (build|buildloader|clean) [--native]")
  print("  Verbs:")
  print("    build         build motor2d executable")
  print("    buildloader   build the JavaScript loader for engine and data")
  print("    clean         delete intermediate files and final executable (doesn't clean loader)")
  print("  Flags:")
  print("    --native      build native executable (not supported for buildloader)")

if len(sys.argv) == 1:
  usage()
elif sys.argv[1] == 'build':
  build()
elif sys.argv[1] == 'buildloader':
  buildLoader()
elif sys.argv[1] == 'clean':
  clean()

