version = 1
name = "lua_timer"
enable_by_default = True
options = {}

def depends():
  pass

def configure():
  add_files(
    'luaapi/timer.c',
  )
