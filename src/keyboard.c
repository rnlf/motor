#include <string.h>
#include <stdbool.h>
#include "keyboard.h"
#include "luaapi/keyboard.h"

typedef struct {
  SDL_Keycode keycode;
  char const * name;
} KeyName;

static const KeyName keynames[] = {
  {SDLK_UNKNOWN,      "unknown"},
  {SDLK_RETURN,       "return"},
  {SDLK_ESCAPE,       "escape"},
  {SDLK_BACKSPACE,    "backspace"},
  {SDLK_TAB,          "tab"},
  {SDLK_SPACE,        " "},
  {SDLK_EXCLAIM,      "!"},
  {SDLK_QUOTEDBL,     "\""},
  {SDLK_HASH,         "#"},
  {SDLK_DOLLAR,       "$"},
  {SDLK_AMPERSAND,    "&"},
  {SDLK_QUOTE,        "\'"},
  {SDLK_LEFTPAREN,    "("},
  {SDLK_RIGHTPAREN,   ")"},
  {SDLK_ASTERISK,     "*"},
  {SDLK_PLUS,         "+"},
  {SDLK_COMMA,        ","},
  {SDLK_MINUS,        "-"},
  {SDLK_PERIOD,       "."},
  {SDLK_SLASH,        "/"},
  {SDLK_0,            "0"},
  {SDLK_1,            "1"},
  {SDLK_2,            "2"},
  {SDLK_3,            "3"},
  {SDLK_4,            "4"},
  {SDLK_5,            "5"},
  {SDLK_6,            "6"},
  {SDLK_7,            "7"},
  {SDLK_8,            "8"},
  {SDLK_9,            "9"},
  {SDLK_COLON,        ":"},
  {SDLK_SEMICOLON,    ";"},
  {SDLK_LESS,         "<"},
  {SDLK_EQUALS,       "="},
  {SDLK_GREATER,      ">"},
  {SDLK_QUESTION,     "?"},
  {SDLK_AT,           "@"},
  {SDLK_LEFTBRACKET,  "["},
  {SDLK_BACKSLASH,    "\\"},
  {SDLK_RIGHTBRACKET, "]"},
  {SDLK_CARET,        "^"},
  {SDLK_UNDERSCORE,   "_"},
  {SDLK_BACKQUOTE,    "`"},
  {SDLK_a,            "a"},
  {SDLK_b,            "b"},
  {SDLK_c,            "c"},
  {SDLK_d,            "d"},
  {SDLK_e,            "e"},
  {SDLK_f,            "f"},
  {SDLK_g,            "g"},
  {SDLK_h,            "h"},
  {SDLK_i,            "i"},
  {SDLK_j,            "j"},
  {SDLK_k,            "k"},
  {SDLK_l,            "l"},
  {SDLK_m,            "m"},
  {SDLK_n,            "n"},
  {SDLK_o,            "o"},
  {SDLK_p,            "p"},
  {SDLK_q,            "q"},
  {SDLK_r,            "r"},
  {SDLK_s,            "s"},
  {SDLK_t,            "t"},
  {SDLK_u,            "u"},
  {SDLK_v,            "v"},
  {SDLK_w,            "w"},
  {SDLK_x,            "x"},
  {SDLK_y,            "y"},
  {SDLK_z,            "z"},
  {SDLK_CAPSLOCK,     "capslock"},
  {SDLK_F1,           "f1"},
  {SDLK_F2,           "f2"},
  {SDLK_F3,           "f3"},
  {SDLK_F4,           "f4"},
  {SDLK_F5,           "f5"},
  {SDLK_F6,           "f6"},
  {SDLK_F7,           "f7"},
  {SDLK_F8,           "f8"},
  {SDLK_F9,           "f9"},
  {SDLK_F10,          "f10"},
  {SDLK_F11,          "f11"},
  {SDLK_F12,          "f12"},
  {SDLK_PAUSE,        "pause"},
  {SDLK_INSERT,       "insert"},
  {SDLK_HOME,         "home"},
  {SDLK_PAGEUP,       "pageup"},
  {SDLK_DELETE,       "delete"},
  {SDLK_END,          "end"},
  {SDLK_PAGEDOWN,     "pagedown"},
  {SDLK_RIGHT,        "right"},
  {SDLK_LEFT,         "left"},
  {SDLK_DOWN,         "down"},
  {SDLK_UP,           "up"},
  {SDLK_POWER,        "power"},
  {SDLK_KP_EQUALS,    "kp="},
  {SDLK_HELP,         "help"},
  {SDLK_MENU,         "menu"},
  {SDLK_UNDO,         "undo"},
  {SDLK_SYSREQ,       "sysreq"},
  {SDLK_CLEAR,        "clear"},
  {SDLK_LCTRL,        "lctrl"},
  {SDLK_LSHIFT,       "lshift"},
  {SDLK_LALT,         "lalt"},
  {SDLK_RCTRL,        "rctrl"},
  {SDLK_RSHIFT,       "rshift"},
  {SDLK_RALT,         "ralt"},
  {SDLK_MODE,         "mode"},
#ifdef EMSCRIPTEN
  {SDLK_RGUI,         "rgui"},
  {SDLK_LGUI,         "lgui"},
  {SDLK_CURRENCYUNIT, "currencyunit"},
  {SDLK_KP_DIVIDE,    "kp/"},
  {SDLK_KP_MULTIPLY,  "kp*"},
  {SDLK_KP_MINUS,     "kp-"},
  {SDLK_KP_PLUS,      "kp+"},
  {SDLK_KP_ENTER,     "kpenter"},
  {SDLK_KP_PERIOD,    "kp."},
  {SDLK_KP_COMMA,     "kp,"},
  {SDLK_PERCENT,      "%"},
  {SDLK_PRINTSCREEN,  "printscreen"},
  {SDLK_SCROLLLOCK,   "scrolllock"},
  {SDLK_APPLICATION,  "application"},
  {SDLK_F13,          "f13"},
  {SDLK_F14,          "f14"},
  {SDLK_F15,          "f15"},
  {SDLK_F16,          "f16"},
  {SDLK_F17,          "f17"},
  {SDLK_F18,          "f18"},
  {SDLK_F19,          "f19"},
  {SDLK_F20,          "f20"},
  {SDLK_F21,          "f21"},
  {SDLK_F22,          "f22"},
  {SDLK_F23,          "f23"},
  {SDLK_F24,          "f24"},
  {SDLK_KP_1,         "kp1"},
  {SDLK_KP_2,         "kp2"},
  {SDLK_KP_3,         "kp3"},
  {SDLK_KP_4,         "kp4"},
  {SDLK_KP_5,         "kp5"},
  {SDLK_KP_6,         "kp6"},
  {SDLK_KP_7,         "kp7"},
  {SDLK_KP_8,         "kp8"},
  {SDLK_KP_9,         "kp9"},
  {SDLK_KP_0,         "kp0"},
  {SDLK_WWW,          "www"},
  {SDLK_MAIL,         "mail"},
  {SDLK_CALCULATOR,   "calculator"},
  {SDLK_COMPUTER,     "computer"},
  {SDLK_AC_SEARCH,    "appsearch"},
  {SDLK_AC_HOME,      "apphome"},
  {SDLK_AC_BACK,      "appback"},
  {SDLK_AC_FORWARD,   "appforward"},
  {SDLK_AC_STOP,      "appstop"},
  {SDLK_AC_REFRESH,   "apprefresh"},
  {SDLK_AC_BOOKMARKS, "appbookmarks"},
#else

#endif
};

static struct {
  char const * keynames[SDLK_LAST];
  bool keystate[SDLK_LAST];
  bool textActive;
} moduleData;

void keyboard_startText(void);

void keyboard_init(void) {

  for(int i = 0; i < SDLK_LAST; ++i) {
    moduleData.keynames[i] = "unknown";
  }

  for(int i = 0; i < sizeof(keynames) / sizeof(KeyName); ++i) {
    moduleData.keynames[keynames[i].keycode] = keynames[i].name;
  }

  memset(&moduleData.keystate, 0, sizeof(moduleData.keystate));
  keyboard_startText();
}

char const * keyboard_getKeyName(SDL_Keycode key) {
  return moduleData.keynames[key];
}

SDL_Keycode keyboard_getKeycode(char const* name) {
  // TODO this is really slow. use appropriate data structure
  for(int i = 0; i < SDLK_LAST; ++i) {
    if(!strcmp(name, keynames[i].name)) {
      return keynames[i].keycode;
    }
  }
  return 0;
}

void keyboard_keypressed(SDL_Keycode key) {
  bool repeat = moduleData.keystate[key];
  moduleData.keystate[key] = true;
  l_keyboard_keypressed(key, repeat);
}

void keyboard_keyreleased(SDL_Keycode key) {
  moduleData.keystate[key] = false;
  l_keyboard_keyreleased(key);
}

bool keyboard_ispressed(SDL_Keycode key) {
  return moduleData.keystate[key];
}

void keyboard_startText(void) {
#ifdef EMSCRIPTEN
  SDL_StartTextInput();
#else
# warning SDL_StartTextInput not available in SDL1.2
#endif
  moduleData.textActive = true;
}

void keyboard_stopText(void) {
#ifdef EMSCRIPTEN
  SDL_StopTextInput();
#else
# warning SDL_StopTextInput not available in SDL1.2
#endif
  moduleData.textActive = false;
}

bool keyboard_isTextEnabled(void) {
  return moduleData.textActive;
}

void keyboard_textInput(char const* text) {
  l_keyboard_textInput(text);
}
