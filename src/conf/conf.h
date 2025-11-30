#ifndef _CONF_H
#define _CONF_H

void _conf_clocks();
void _conf_adc();
void _conf_dmac();
void _conf_eic();
void _conf_evsys();
void _conf_port();
void _conf_sercom();
void _conf_tc();
void _conf_tcc();
void _conf_usb();

/**
 * @brief Run all config functions
 *
 */
static inline void conf() {
  _conf_clocks();
  _conf_port();
  _conf_adc();
  _conf_dmac();
  _conf_eic();
  _conf_evsys();
  _conf_sercom();
  _conf_tc();
  _conf_tcc();
  _conf_usb();
}

#endif