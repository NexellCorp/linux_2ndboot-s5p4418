#ifndef __COMMON_H__
#define __COMMON_H__

/* "startup.S" startup code function */
void system_sleep(void);

/* "armv7_lib.S" armv7 function */
void set_nonsecure_mode(void);
void set_secure_mode(void);

 int arm9_get_scr(void);
void arm9_set_scr(int reg);

 int arm9_get_auxctrl(void);
void arm9_set_auxctrl(int value);

 int armv7_get_cpuid(void);

void cache_delay_ms(int ms);

#endif
