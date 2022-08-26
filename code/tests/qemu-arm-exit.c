// SPDX-License-Identifier: zlib-acknowledgement

#include <stdint.h>

void 
qemu_arm_exit(void) 
{
  register uint32_t r0 __asm__("r0");
  r0 = 0x18;
  register uint32_t r1 __asm__("r1");
  r1 = 0x20026;
  __asm__ volatile("bkpt #0xAB");
}
