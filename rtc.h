
#ifndef _RTC_H
#define _RTC_H

typedef uint32_t time_t;

void RTC_Init(void);
void rtc_start_reset(void);
uint8_t rtc_check_start(void);
void rtc_vbat_enable(void);
uint8_t rtc_check_vbat(void);
void rtc_mk_config(void);
uint8_t rtc_check_config(void);
void print_time(void);

int set_time(time_t timestamp);
time_t time();

int i2c_read_bytes(uint16_t eeaddr, int len, uint8_t *buf);
int i2c_write_bytes(uint16_t eeaddr, int len, uint8_t *buf);

#endif
