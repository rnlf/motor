#include "motor.h"

motor_Version const * motor_getVersion(void) {
  static const motor_Version version = {
    .major = 0,
    .minor = 0,
    .revision = 3,
    .codename = "Rotten Rodent"
  };

  return &version;
}
