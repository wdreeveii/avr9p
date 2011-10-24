#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include "usart.h"
#include "rtc.h"

void RTC_Init(void)
{
	/* frequency = cpuspeed / (16 + 2(TWBR) * 4^(TWSR)) */
	/* 50000 = 20000000 / (16 + 2(48) * 4^1) */
	
	/* set the prescaler to 4^1 */
	TWSR = 1;
	
	TWBR = 40;
	
	// configure pin change interrupt for the square wave output from the rtc
	// enable PCIE2
	PCICR = (1 << 1);
	PCMSK1 = (1 << 2);
	
	PORTB &= ~(1<<2);
	DDRB &= ~(1<<2);
	
	if(rtc_check_halt() == -1)
	{
		printf("halt Resetting..\n");
		rtc_halt_reset();	
	}
	if(rtc_check_osc_fail() == -1)
	{
		printf("osc Resetting..\n");
		rtc_osc_fail_reset();
	}
	if(rtc_check_stop() == -1)
	{
		printf("Resetting..\n");
		rtc_stop_reset();	
	}
	//rtc_start();
	rtc_squarewave_enable();
}

struct m41DateBlock {
	unsigned int secs : 4;
	unsigned int tsecs : 3;
	unsigned int st : 1;
	
	unsigned int mins : 4;
	unsigned int tmins : 3;
	unsigned int placeholder1 : 1;
	
	unsigned int hours : 4;
	unsigned int thours : 2;
	unsigned int CB0 : 1;
	unsigned int CB1 : 1;
	
	unsigned int DayOfWeek : 3;
	unsigned int placeholder2 : 5;
	
	unsigned int DayOfMonth : 4;
	unsigned int tDayOfMonth : 2;
	unsigned int placeholder3 : 2;
	
	unsigned int Month : 4;
	unsigned int tMonth : 1;
	unsigned int placeholder4 : 3;
	
	unsigned int Year : 4;
	unsigned int tYear : 4;
};
unsigned long leapYearsBefore (unsigned long year)
{
	year--;
	return (year/4) - (year/100) + (year/400);
}
int isLeapYear(unsigned long year)
{
	if (year % 400 == 0)
		return 1;
	else if (year % 100 == 0)
		return 0;
	else if (year % 4 == 0)
		return 1;
	else	return 0;
}
int set_time(time_t timestamp)
{
	ioflip(1);
	uint8_t datestring[16];
	ultoa(timestamp, datestring, 10);
	USART_Send(0, datestring, strlen(datestring));
	USART_Send(0, "|\n", 2);
	
	struct m41DateBlock dateblock;
	uint8_t rv = 0;
	
	uint8_t month_lens[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	uint8_t month_index = 0;
	
	unsigned long total_days = timestamp/86400ul;
	unsigned long rem_secs = timestamp % 86400ul;
	
	unsigned long guess_leap_years = leapYearsBefore(1970ul + (total_days/365ul)) - leapYearsBefore(1970ul);
	unsigned long total_years = 1970ul + ((total_days - guess_leap_years)/365ul);
	unsigned long rem_days = ((total_days - guess_leap_years) % 365ul);

	dateblock.tYear = (total_years % 100) / 10ul;
	dateblock.Year = (total_years % 100) % 10ul;
	/*ultoa(dateblock.tYear, datestring, 10);
	USART_Send(0, datestring, strlen(datestring));
	ultoa(dateblock.Year, datestring, 10);
	USART_Send(0, datestring, strlen(datestring));
	USART_Send(0, ", ", 2);*/
	if (isLeapYear(total_years))
		month_lens[1] = 29;
		
	for (; rem_days > month_lens[month_index] && month_index < 12; month_index++)
		rem_days -= month_lens[month_index];
		
	dateblock.tMonth = (month_index + 1) / 10;
	dateblock.Month = (month_index + 1) % 10;
	/*ultoa(dateblock.tMonth, datestring, 10);
	USART_Send(0, datestring, strlen(datestring));
	ultoa(dateblock.Month, datestring, 10);
	USART_Send(0, datestring, strlen(datestring));
	USART_Send(0, ", ", 2);*/
	
	dateblock.tDayOfMonth = (rem_days) / 10;
	dateblock.DayOfMonth = (rem_days) % 10;
	/*ultoa(dateblock.tDayOfMonth, datestring, 10);
	USART_Send(0, datestring, strlen(datestring));
	ultoa(dateblock.DayOfMonth, datestring, 10);
	USART_Send(0, datestring, strlen(datestring));
	USART_Send(0, ", ", 2);*/
	dateblock.thours = (rem_secs / 3600) / 10;
	dateblock.hours = (rem_secs / 3600) % 10;
	/*
	ultoa(dateblock.thours, datestring, 10);
	USART_Send(0, datestring, strlen(datestring));
	ultoa(dateblock.hours,datestring,10);
	USART_Send(0, datestring, strlen(datestring));
	USART_Send(0, ", ", 2);*/
	
	rem_secs = (rem_secs % 3600);
	
	dateblock.tmins = (rem_secs / 60) / 10;
	dateblock.mins = (rem_secs / 60) % 10;

	/*ultoa(dateblock.tmins, datestring, 10);
	USART_Send(0, datestring, strlen(datestring));
	ultoa(dateblock.mins, datestring, 10);
	USART_Send(0, datestring, strlen(datestring));
	USART_Send(0, "\n", 1);*/
	rem_secs = (rem_secs % 60);

	dateblock.tsecs = rem_secs / 10;
	dateblock.secs = rem_secs % 10;
	rv = m41t83_write_bytes(0x01, 7,(void*) &dateblock);
	if (rv == -1) USART_Send(0, "WRITE ERROR\n", 12);
}

time_t time()
{
	uint8_t month_lens[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	uint8_t rv = 0;
	struct m41DateBlock dateblock;
	
	rv = m41t83_read_bytes(0x01, 7, (void *)(&dateblock));
	if (rv == -1) USART_Send(0, "READ ERROR\n", 11);
	
	unsigned long year = 2000 + dateblock.Year + (dateblock.tYear * 10);
	unsigned long year_days = ((year - 1970) * 365) + (leapYearsBefore(year) - leapYearsBefore(1970));
	
	uint8_t month = dateblock.Month + (dateblock.tMonth * 10);
	if (isLeapYear(year))
		month_lens[1] = 29;
	
	uint16_t month_days = 0;
	for (rv = 1; rv < month; rv++)
		month_days += month_lens[rv - 1];
	unsigned long total_days = year_days + month_days + dateblock.DayOfMonth + (dateblock.tDayOfMonth * 10);
	unsigned long total_secs = (total_days * 86400) + 
						 ((dateblock.hours + (dateblock.thours * 10)) * 3600) + 
						 ((dateblock.mins + (dateblock.tmins * 10)) * 60) +
						 (dateblock.secs + (dateblock.tsecs * 10));
	
	return total_secs;
}

void print_time(void)
{
	time_t timestamp = time();
	uint8_t datestring[16];
	ultoa(timestamp, datestring, 10);
	USART_Send(0, datestring, strlen(datestring));
	USART_Send(0, "\n", 1);
}

// rtc 1hz square wave tick
ISR(PCINT1_vect, ISR_NAKED)
{
	sei();
	// issue print request
	// The square wave is like any square wave, 
	// and thus it changes value twice in 1 period.
	if (PINB & (1<<2))
	{
		//ioflip(1);
		print_time();
		mucron_tick();
	}
	else
		{} // do nothing
	reti();
}

void rtc_squarewave_enable(void)
{
	uint8_t rv = 0;
	uint8_t databuffer[1];
	rv = m41t83_read_bytes(0x13, 1, databuffer);
	if (rv == -1) USART_Send(0, "READ ERROR\n", 11);
	
	databuffer[0] |= 0xF0;
	
	rv = m41t83_write_bytes(0x13, 1, databuffer);
	if (rv == -1) USART_Send(0, "WRITE ERROR\n", 12);
	
	rv = m41t83_read_bytes(0x0A, 1, databuffer);
	if (rv == -1) USART_Send(0, "READ ERROR\n", 11);
	
	databuffer[0] |= (1<<6);
	
	rv = m41t83_write_bytes(0x0A, 1, databuffer);
	if (rv == -1) USART_Send(0, "WRITE ERROR\n", 12);
}

int8_t rtc_check_halt(void)
{
	uint8_t rv = 0;
	uint8_t databuffer[1];
	rv = m41t83_read_bytes(0x0C, 1, databuffer);
	if (rv == -1) USART_Send(0, "RTC Init WRITE ERROR\n", 21);
	if (databuffer[0] & (1<<6))
	{
		//DSEND(0, "(check_halt)osc halt set\n");
		return -1;	
	}
	return 0;
}

void rtc_halt_reset(void)
{
	uint8_t rv = 0;
	uint8_t databuffer[1];
	databuffer[0] = 0x00;
	rv = m41t83_write_bytes(0x0C, 1, databuffer);
	if (rv == -1) USART_Send(0, "RTC Init WRITE ERROR\n", 21);
}

int8_t rtc_check_stop(void)
{
	uint8_t rv = 0;
	uint8_t databuffer[1];
	rv = m41t83_read_bytes(0x01, 1, databuffer);
	if (rv == -1) USART_Send(0, "READ ERROR\n", 11);
	if (databuffer[0] & 0x80)
	{
		USART_Send(0, "WARNING: STOP BIT SET\n", 22);
		return -1;
	}
	return 0;
}

void rtc_stop_reset(void)
{
	uint8_t rv = 0;
	uint8_t databuffer[1];
	// pump the oscillator
	databuffer[0] = 0x80;
	rv = m41t83_write_bytes(0x01, 1, databuffer);
	if (rv == -1) USART_Send(0, "WRITE ERROR\n", 12);
	databuffer[0] = 0x00;
	rv = m41t83_write_bytes(0x01, 1, databuffer);
	if (rv == -1) USART_Send(0, "WRITE ERROR\n", 12);
	
	delay_1s();
	delay_1s();
	delay_1s();
	delay_1s();
	
}
int8_t rtc_check_osc_fail(void)
{
	uint8_t rv = 0;
	uint8_t databuffer[1];
	rv = m41t83_read_bytes(0x0F, 1, databuffer);
	if (rv == -1) USART_Send(0, "READ ERROR\n", 11);
	if (databuffer[0] & 0x04)
	{
		USART_Send(0, "WARNING: OSCILLATOR FAIL BIT SET\n", 33);
		return -1;
	}
	return 0;
	
}
void rtc_osc_fail_reset(void)
{
	uint8_t rv = 0;
	uint8_t databuffer[1];
	rv = m41t83_read_bytes(0x0F, 1, databuffer);
	if (rv == -1) USART_Send(0, "READ ERROR\n", 11);
	
	databuffer[0] &= ~0x04;
	
	rv = m41t83_write_bytes(0x0F, 1, databuffer);
	if (rv == -1) USART_Send(0, "WRITE ERROR\n", 12);
}

#define M41T83_SLAVE_ADDR	0xD0

// bus cycles to wait before timeout on lengthy operations like write
#define MAX_ITER	200

/*
 * Number of bytes that can be written in a row.
 */
#define PAGE_SIZE 8

/*
 * Saved TWI status register.
 */
uint8_t twst;

/*
 * Note [7]
 *
 * Read "len" bytes from EEPROM starting at "eeaddr" into "buf".
 *
 * This requires two bus cycles: during the first cycle, the device
 * will be selected (master transmitter mode), and the address
 * transfered.
 * Address bits exceeding 256 are transfered in the
 * E2/E1/E0 bits (subaddress bits) of the device selector.
 * Address is sent in two dedicated 8 bit transfers
 * for 16 bit address devices (larger EEPROM devices)
 *
 * The second bus cycle will reselect the device (repeated start
 * condition, going into master receiver mode), and transfer the data
 * from the device to the TWI master.  Multiple bytes can be
 * transfered by ACKing the client's transfer.  The last transfer will
 * be NACKed, which the client will take as an indication to not
 * initiate further transfers.
 */
int m41t83_read_bytes(uint8_t eeaddr, int len, uint8_t *buf)
{
  uint8_t sla, twcr, n = 0;
  int rv = 0;

  sla = M41T83_SLAVE_ADDR;
  
  /*
   * Note [8]
   * First cycle: master transmitter mode
   */
  restart:
  if (n++ >= MAX_ITER)
    return -1;
  begin:

  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_REP_START:		/* OK, but should not happen */
    case TW_START:
      break;

    case TW_MT_ARB_LOST:	/* Note [9] */
      goto begin;

    default:
      return -1;		/* error: not in start condition */
				/* NB: do /not/ send stop condition */
    }

  /* Note [10] */
  /* send SLA+W */
  TWDR = sla | TW_WRITE;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
				/* Note [11] */
      goto restart;

    case TW_MT_ARB_LOST:	/* re-arbitrate */
      goto begin;

    default:
      goto error;		/* must send stop condition */
    }

  TWDR = eeaddr;		/* low 8 bits of addr */
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MT_DATA_ACK:
      break;

    case TW_MT_DATA_NACK:
      goto quit;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      goto error;		/* must send stop condition */
    }

  /*
   * Note [12]
   * Next cycle(s): master receiver mode
   */
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send (rep.) start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_START:		/* OK, but should not happen */
    case TW_REP_START:
      break;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      goto error;
    }

  /* send SLA+R */
  TWDR = sla | TW_READ;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MR_SLA_ACK:
      break;

    case TW_MR_SLA_NACK:
      goto quit;

    case TW_MR_ARB_LOST:
      goto begin;

    default:
      goto error;
    }

  for (twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWEA) /* Note [13] */;
       len > 0;
       len--)
    {
      if (len == 1)
	twcr = _BV(TWINT) | _BV(TWEN); /* send NAK this time */
      TWCR = twcr;		/* clear int to start transmission */
      while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
      switch ((twst = TW_STATUS))
	{
	case TW_MR_DATA_NACK:
	  len = 0;		/* force end of loop */
	  /* FALLTHROUGH */
	case TW_MR_DATA_ACK:
	  *buf++ = TWDR;
	  rv++;
	  break;

	default:
	  goto error;
	}
    }
  quit:
  /* Note [14] */
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */

  return rv;

  error:
  rv = -1;
  goto quit;
}

/*
 * Write "len" bytes into EEPROM starting at "eeaddr" from "buf".
 *
 * This is a bit simpler than the previous function since both, the
 * address and the data bytes will be transfered in master transmitter
 * mode, thus no reselection of the device is necessary.  However, the
 * EEPROMs are only capable of writing one "page" simultaneously, so
 * care must be taken to not cross a page boundary within one write
 * cycle.  The amount of data one page consists of varies from
 * manufacturer to manufacturer: some vendors only use 8-byte pages
 * for the smaller devices, and 16-byte pages for the larger devices,
 * while other vendors generally use 16-byte pages.  We thus use the
 * smallest common denominator of 8 bytes per page, declared by the
 * macro PAGE_SIZE above.
 *
 * The function simply returns after writing one page, returning the
 * actual number of data byte written.  It is up to the caller to
 * re-invoke it in order to write further data.
 */
int m41t83_write_page(uint8_t eeaddr, int len, uint8_t *buf)
{
  uint8_t sla, n = 0;
  int rv = 0;
  uint8_t endaddr;

  if (eeaddr + len < (eeaddr | (PAGE_SIZE - 1)))
    endaddr = eeaddr + len;
  else
    endaddr = (eeaddr | (PAGE_SIZE - 1)) + 1;
  len = endaddr - eeaddr;

  /* patch high bits of EEPROM address into SLA */
  sla = M41T83_SLAVE_ADDR;

  restart:
  if (n++ >= MAX_ITER)
    return -1;
  begin:

  /* Note [15] */
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_REP_START:		/* OK, but should not happen */
    case TW_START:
      break;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      return -1;		/* error: not in start condition */
				/* NB: do /not/ send stop condition */
    }

  /* send SLA+W */
  TWDR = sla | TW_WRITE;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:	/* re-arbitrate */
      goto begin;

    default:
      goto error;		/* must send stop condition */
    }

  TWDR = eeaddr;		/* low 8 bits of addr */
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MT_DATA_ACK:
      break;

    case TW_MT_DATA_NACK:
      goto quit;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      goto error;		/* must send stop condition */
    }

  for (; len > 0; len--)
    {
      TWDR = *buf++;
      TWCR = _BV(TWINT) | _BV(TWEN); /* start transmission */
      while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
      switch ((twst = TW_STATUS))
	{
	case TW_MT_DATA_NACK:
	  goto error;		/* device write protected -- Note [16] */

	case TW_MT_DATA_ACK:
	  rv++;
	  break;

	default:
	  goto error;
	}
    }
  quit:
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */

  return rv;

  error:
  rv = -1;
  goto quit;
}

/*
 * Wrapper around ee24xx_write_page() that repeats calling this
 * function until either an error has been returned, or all bytes
 * have been written.
 */
int m41t83_write_bytes(uint16_t eeaddr, int len, uint8_t *buf)
{
  int rv, total;

  total = 0;
  do
    {
      rv = m41t83_write_page(eeaddr, len, buf);

      if (rv == -1)
	return -1;
      eeaddr += rv;
      len -= rv;
      buf += rv;
      total += rv;
    }
  while (len > 0);

  return total;
}