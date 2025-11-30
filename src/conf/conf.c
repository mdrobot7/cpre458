#include "conf.h"

static inline void _conf_dummy() {}

void _conf_clocks() __attribute__((weak, alias("_conf_dummy")));
void _conf_adc() __attribute__((weak, alias("_conf_dummy")));
void _conf_dmac() __attribute__((weak, alias("_conf_dummy")));
void _conf_eic() __attribute__((weak, alias("_conf_dummy")));
void _conf_evsys() __attribute__((weak, alias("_conf_dummy")));
void _conf_port() __attribute__((weak, alias("_conf_dummy")));
void _conf_sercom() __attribute__((weak, alias("_conf_dummy")));
void _conf_tc() __attribute__((weak, alias("_conf_dummy")));
void _conf_tcc() __attribute__((weak, alias("_conf_dummy")));
void _conf_usb() __attribute__((weak, alias("_conf_dummy")));