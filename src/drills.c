/*
 * Write your solutions here. Each function returns a placeholder until you solve it.
 * Keep the code MISRA-friendly: fixed-width types, no magic numbers without a comment,
 * no undefined behaviour (e.g. shifting a signed value, or shifting by 32 or more).
 */
#include "drills.h"

/* 01. Bit manipulation
 * Every function builds a mask with only bit n set: (1u << n).
 * 1u (unsigned) is used because shifting a signed 1 into bit 31 is undefined behaviour.
 * n must be 0..31: shifting a 32-bit value by 32 or more is also undefined.
 */
uint32_t bit_set(uint32_t reg, uint8_t n)
{
    return reg | (1u << n);           /* OR: forces bit n to 1, other bits unchanged */
}

uint32_t bit_clear(uint32_t reg, uint8_t n)
{
    return reg & ~(1u << n);          /* AND with the inverted mask: forces bit n to 0 */
}

uint32_t bit_toggle(uint32_t reg, uint8_t n)
{
    return reg ^ (1u << n);           /* XOR: flips bit n (1 -> 0, 0 -> 1) */
}

uint8_t bit_check(uint32_t reg, uint8_t n)
{
    return (uint8_t)((reg >> n) & 1u); /* move bit n down to bit 0, keep only that bit */
}

/* 02 */
//bool is_pow2(uint32_t v) { (void)v; return false; }

bool is_pow2(uint32_t v)
{
    return (v != 0u) && ((v & (v - 1u)) == 0u);
}

/* 03 */
uint8_t count_bits(uint32_t v) { (void)v; return 0u; }

/* 04 */
uint16_t read_be16(const uint8_t *p) { (void)p; return 0u; }
uint16_t read_le16(const uint8_t *p) { (void)p; return 0u; }

/* 05 */
int16_t battery_temp_deci_c(const uint8_t data[8]) { (void)data; return 0; }

/* 06 */
alive_t alive_check(uint8_t previous, uint8_t current) { (void)previous; (void)current; return ALIVE_OK; }

/* 07 */
bool timed_out(uint32_t now_ms, uint32_t last_ms, uint32_t limit_ms)
{
    (void)now_ms; (void)last_ms; (void)limit_ms;
    return false;
}

/* 08 */
void debounce_init(debounce_t *d, uint8_t initial_level) { (void)d; (void)initial_level; }
uint8_t debounce_update(debounce_t *d, uint8_t raw) { (void)d; (void)raw; return 0u; }

/* 09 */
void rb_init(ringbuf_t *rb) { (void)rb; }
bool rb_put(ringbuf_t *rb, uint8_t byte) { (void)rb; (void)byte; return false; }
bool rb_get(ringbuf_t *rb, uint8_t *byte) { (void)rb; (void)byte; return false; }
uint8_t rb_count(const ringbuf_t *rb) { (void)rb; return 0u; }

/* 10 */
uint8_t crc8_j1850(const uint8_t *data, uint32_t len) { (void)data; (void)len; return 0u; }
