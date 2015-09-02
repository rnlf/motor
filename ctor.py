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



class CompileFile():
  def __init__(self, src):
    self.source = src
    self.actions = []
    self.properties = {}

  def add_compiler_flags(self, *newflags):
    if "CFLAGS" not in self.properties:
      self.properties["CFLAGS"] = []

    for _, f in enumerate(newflags):
      self.properties["CFLAGS"].append(f)

  def add_include_dirs(self, *dirs):
    if "INCLUDE_DIRS" not in self.properties:
      self.properties["INCLUDE_DIRS"] = []

    for _, d in enumerate(dirs):
      self.properties["INCLUDE_DIRS"].append(d)

  def add_required_action(self, action):
    self.actions.append(action)


class Module:
  def __init__(self, source_dir):
    self.files = {}
    self.properties = {}
    self.source_dir = source_dir
    self.pre_actions = []

  def read_definition(self, definition):
    self.version = definition["version"]


  def add_files(self, *files):
    for _, f in enumerate(*files):
      print("FILE: " + f)
      self.files[f] = CompileFile(os.path.join(self.source_dir, f))

  def add_required_action(self, action):
    self.pre_actions.append(action)

  def add_compiler_flags(self, *newflags):
    if "CFLAGS" not in self.properties:
      self.properties["CFLAGS"] = []

    for _, f in enumerate(newflags):
      self.properties["CFLAGS"].append(f)

  def add_include_dirs(self, *dirs):
    if "INCLUDE_DIRS" not in self.properties:
      self.properties["INCLUDE_DIRS"] = []

    for _, d in enumerate(dirs):
      self.properties["INCLUDE_DIRS"].append(d)

  def add_file_compiler_flags(self, filename, *newflags):
    self.files[filename].add_compiler_flag(newflags)

  def add_file_include_dirs(self, filename, *newdirs):
    self.files[filename].add_include_dirs(newdirs)

  def add_file_required_action(self, action):
    self.files[filename].add_required_action(action)


def load_modules(repo_directories):
  modules = {}
    
  def load_module_file(source_dir, filename):
    print("Reading module " + fname)

    module = Module(source_dir)

    def sd():
      return source_dir

    def aid(*dirs):
      return module.add_include_dirs(dirs)

    def acf(*flags):
      return module.add_compiler_flags(flags)

    def af(*files):
      return module.add_files(files)

    def apbs(step):
      return module.add_required_action(step)

    def afid(f, *dirs):
      return module.add_file_include_dirs(f, dirs)

    def afcf(f, *flags):
      return module.add_file_compiler_flags(f, flags)

    with open(fname) as f:
      global_vars = {
        'source_directory':      sd,
        'add_include_dirs':      aid,
        'add_compiler_flags':    acf,
        'add_files':             af,
        'add_prebuild_step':     apbs,
        'add_file_include_dirs': afid,
        'add_file_flags':        afcf,
        'os':                    os
      }
      module_def = {}

      code = compile(f.read(), filename, 'exec')
      exec(code, global_vars, module_def)
      module.read_definition(module_def)
      if module_def["name"] in modules:
        if module_def["version"] < modules[module_def["name"]].version:
          return

      modules[module_def["name"]] = module
      module_def["configure"]({})
          

  for d in repo_directories:
    repo = {}
    cfg_filename = os.path.join(d, "ctor.cfg")
    with open(cfg_filename) as cfg:
      exec(compile(cfg.read(), cfg_filename, 'exec'), {}, repo)

    print(repo["repository"], repo["source_dir"])

    for fname in glob.iglob(os.path.join(d, "modules", "*.ctor")):
      load_module_file(repo["source_dir"], fname)

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
