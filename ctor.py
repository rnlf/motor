#!/usr/bin/python3

import sys
import os
import glob

files = []
file_incdirs = {}
file_flags = {}
flags = []
incdirs = []
prebuild = []


def add_include_dirs(*dirs):
  for _, d in enumerate(dirs):
    incdirs.append(d)

def add_file_include_dirs(filename, *dirs):
  if filename not in file_incdirs:
    file_incdirs[filename] = []
  for _, d in enumerate(dirs):
    file_incdirs[filename].append(d)

def add_file_flags(filename, *flags):
  if filename not in file_flags:
    file_flags[filename] = []
  for _, d in enumerate(flags):
    file_flags[filename].append(d)
    
def add_compiler_flags(*newflags):
  for _, f in enumerate(newflags):
    flags.append(f)

def add_files(*filenames):
  for _, f in enumerate(filenames):
    files.append(f)
  
def add_prebuild_step(func, outputs=[], dependencies=[]):
  prebuild.append(func)


def load_modules(repo_directories):
  modules = {}
    
  def load_module_file(filename):
    print("Reading module " + fname)
    with open(fname) as f:
      global_vars = {
        'source_directory':      source_directory,
        'add_include_dirs':      add_include_dirs,
        'add_compiler_flags':    add_compiler_flags,
        'add_files':             add_files,
        'add_prebuild_step':     add_prebuild_step,
        'add_file_include_dirs': add_file_include_dirs,
        'add_file_flags':        add_file_flags,
        'os':                    os
      }
      module = {}

      code = compile(f.read(), filename, 'exec')
      exec(code, global_vars, module)
      if module["name"] in modules:
        if module["version"] < modules[module["name"]]["version"]:
          return

      modules[module["name"]] = module
          

  for d in repo_directories:
    repo = {}
    cfg_filename = os.path.join(d, "ctor.cfg")
    with open(cfg_filename) as cfg:
      exec(compile(cfg.read(), cfg_filename, 'exec'), {}, repo)

    print(repo["repository"], repo["source_dir"])

    for fname in glob.iglob(os.path.join(d, "modules", "*.ctor")):
      load_module_file(fname)

  return modules

def compile_file(filename):
  pass

def build(config):
  modules = load_modules(config["module_dirs"])
  
  for m, o in config["modules"].items():
    print("Configuring module " + m)
    modules[m]["configure"](config)

  print("configuration done")
  print("executing prebuild steps")
  for s in prebuild:
    s()
