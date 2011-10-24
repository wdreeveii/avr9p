
#ifndef _RTC_H
#define _RTC_H

typedef uint32_t time_t;

void RTC_Init(void);
int8_t rtc_check_halt(void);
void rtc_halt_reset(void);
int8_t rtc_check_osc_fail(void);
void rtc_stop_reset(void);
int8_t rtc_check_stop(void);
void rtc_squarewave_enable(void);
void rtc_osc_fail_reset(void);
void print_time(void);

int set_time(time_t timestamp);
time_t time();

int m41t83_read_bytes(uint8_t eeaddr, int len, uint8_t *buf);
int m41t83_write_bytes(uint16_t eeaddr, int len, uint8_t *buf);

#endif