version = 1
name = "filesystem"
enable_by_default = True
options = {}

def depends():
  pass

def configure():
  add_files(
    'filesystem/filesystem.c',
  )
