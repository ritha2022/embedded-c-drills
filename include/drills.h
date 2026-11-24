/*
 * embedded-c-drills: small embedded C problems with automatic tests.
 *
 * How it works:
 *   1. Write your solution for a drill in src/drills.c
 *   2. Change its DONE_ flag below from 0 to 1
 *   3. Run `make test` and fix until it passes
 * Only drills marked DONE are tested, so the build stays green while you work.
 */
#ifndef DRILLS_H
#define DRILLS_H

#include <stdint.h>
#include <stdbool.h>

/* ---- Mark each drill 1 when you have solved it ---- */
#define DONE_01_BITS        1
#define DONE_02_POW2        0
#define DONE_03_COUNT_BITS  0
#define DONE_04_ENDIAN      0
#define DONE_05_CAN_SIGNAL  0
#define DONE_06_ALIVE       0
#define DONE_07_TIMEOUT     0
#define DONE_08_DEBOUNCE    0
#define DONE_09_RINGBUF     0
#define DONE_10_CRC8        0

/* 01. Bit manipulation: return the new register value / the bit value (0 or 1). */
uint32_t bit_set(uint32_t reg, uint8_t n);
uint32_t bit_clear(uint32_t reg, uint8_t n);
uint32_t bit_toggle(uint32_t reg, uint8_t n);
uint8_t  bit_check(uint32_t reg, uint8_t n);

/* 02. True if v is a power of two (0 is not). No loops allowed. */
bool is_pow2(uint32_t v);

/* 03. Number of 1 bits in v. Bonus: loop only once per set bit. */
uint8_t count_bits(uint32_t v);

/* 04. Read a 16-bit value from bytes p[0], p[1]. */
uint16_t read_be16(const uint8_t *p);   /* big-endian (Motorola): p[0] is the high byte */
uint16_t read_le16(const uint8_t *p);   /* little-endian (Intel): p[0] is the low byte  */

/* 05. CAN signal decoding.
 *     Bytes 2-3 of an 8-byte CAN frame hold battery temperature, big-endian,
 *     scale 0.1 degC per bit, offset -40 degC.
 *     Return the temperature in tenths of a degree (e.g. 30.0 degC -> 300). */
int16_t battery_temp_deci_c(const uint8_t data[8]);

/* 06. Alive (rolling) counter check for a 4-bit counter (0..15, then wraps to 0).
 *     Only the low 4 bits of each value are used. */
typedef enum { ALIVE_OK, ALIVE_STUCK, ALIVE_LOST } alive_t;
alive_t alive_check(uint8_t previous, uint8_t current);

/* 07. Timeout check that still works when the millisecond timer wraps past 0xFFFFFFFF.
 *     True if more than limit_ms has passed since last_ms. */
bool timed_out(uint32_t now_ms, uint32_t last_ms, uint32_t limit_ms);

/* 08. Switch debounce. Called every 10 ms with the raw pin level (0 or 1).
 *     The stable output changes only after DEBOUNCE_SAMPLES equal readings in a row. */
#define DEBOUNCE_SAMPLES 4u
typedef struct { uint8_t stable; uint8_t candidate; uint8_t count; } debounce_t;
void    debounce_init(debounce_t *d, uint8_t initial_level);
uint8_t debounce_update(debounce_t *d, uint8_t raw);   /* returns the stable level */

/* 09. Ring buffer (FIFO) of bytes, as used between a UART or CAN ISR and a task.
 *     Either RB_SIZE - 1 or RB_SIZE usable bytes is accepted. */
#define RB_SIZE 8u
typedef struct { uint8_t buf[RB_SIZE]; uint8_t head; uint8_t tail; uint8_t count; } ringbuf_t;
void    rb_init(ringbuf_t *rb);
bool    rb_put(ringbuf_t *rb, uint8_t byte);   /* false if full  */
bool    rb_get(ringbuf_t *rb, uint8_t *byte);  /* false if empty */
uint8_t rb_count(const ringbuf_t *rb);

/* 10. CRC-8/SAE-J1850 (poly 0x1D, init 0xFF, final XOR 0xFF, no reflection),
 *     the CRC used in AUTOSAR E2E Profile 1 and many CAN messages. */
uint8_t crc8_j1850(const uint8_t *data, uint32_t len);

#endif
