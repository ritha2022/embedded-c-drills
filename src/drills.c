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

/* 02. Power of two
 * A power of two has exactly one bit set. Subtracting 1 clears that bit and sets
 * every bit below it, so v & (v - 1) is 0. Zero is excluded explicitly, because
 * 0 & (0 - 1) is also 0 but 0 is not a power of two. No loops needed.
 */
bool is_pow2(uint32_t v)
{
    return (v != 0u) && ((v & (v - 1u)) == 0u);
}

/* 03. Count set bits
 * v & (v - 1) clears the lowest 1 bit (Brian Kernighan's trick),
 * so the loop runs once per set bit, not 32 times.
 */
uint8_t count_bits(uint32_t v)
{
    uint8_t count = 0u;
    while (v != 0u) {
        v &= (v - 1u);
        count++;
    }
    return count;
}

/* 04. Endianness
 * Big-endian (Motorola): the first byte is the most significant.
 * Little-endian (Intel): the first byte is the least significant.
 * Building the value with shifts works on any CPU, whatever its own byte order.
 */
uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[1] << 8) | p[0]);
}

/* 05. CAN signal decoding
 * physical = raw x 0.1 - 40 degC. Working in tenths of a degree avoids floats:
 * raw x 0.1 degC = raw tenths, and -40 degC = -400 tenths.
 */
int16_t battery_temp_deci_c(const uint8_t data[8])
{
    uint16_t raw = read_be16(&data[2]);          /* bytes 2-3, big-endian */
    return (int16_t)((int32_t)raw - 400);         /* int32_t avoids overflow before the cast */
}

/* 06. Alive counter
 * Compare 4-bit counters with modulo-16 arithmetic so 15 -> 0 counts as +1.
 */
alive_t alive_check(uint8_t previous, uint8_t current)
{
    uint8_t prev = (uint8_t)(previous & 0x0Fu);
    uint8_t cur  = (uint8_t)(current & 0x0Fu);
    uint8_t delta = (uint8_t)((cur - prev) & 0x0Fu);

    if (delta == 0u) { return ALIVE_STUCK; }      /* sender frozen or frame repeated */
    if (delta == 1u) { return ALIVE_OK; }
    return ALIVE_LOST;                             /* one or more frames missing */
}

/* 07. Timeout with wraparound
 * Unsigned subtraction wraps the same way the timer does,
 * so now - last is correct even when now has wrapped past 0.
 */
bool timed_out(uint32_t now_ms, uint32_t last_ms, uint32_t limit_ms)
{
    return (uint32_t)(now_ms - last_ms) > limit_ms;
}

/* 08. Debounce
 * The output changes only after DEBOUNCE_SAMPLES equal readings in a row
 * that differ from the current stable level. Any bounce restarts the count.
 */
void debounce_init(debounce_t *d, uint8_t initial_level)
{
    d->stable = initial_level;
    d->candidate = initial_level;
    d->count = 0u;
}

uint8_t debounce_update(debounce_t *d, uint8_t raw)
{
    if (raw == d->stable) {                        /* same as now: nothing to confirm */
        d->candidate = raw;
        d->count = 0u;
    } else if (raw != d->candidate) {              /* a new level appears: start counting */
        d->candidate = raw;
        d->count = 1u;
    } else {                                       /* the new level repeats */
        d->count++;
    }
    if (d->count >= DEBOUNCE_SAMPLES) {
        d->stable = d->candidate;
        d->count = 0u;
    }
    return d->stable;
}

/* 09. Ring buffer
 * head = where the next byte is written, tail = where the next byte is read.
 * A separate count tells full from empty, so all RB_SIZE slots are usable.
 */
void rb_init(ringbuf_t *rb)
{
    rb->head = 0u;
    rb->tail = 0u;
    rb->count = 0u;
}

bool rb_put(ringbuf_t *rb, uint8_t byte)
{
    if (rb->count >= RB_SIZE) { return false; }    /* full: never overwrite unread data */
    rb->buf[rb->head] = byte;
    rb->head = (uint8_t)((rb->head + 1u) % RB_SIZE);
    rb->count++;
    return true;
}

bool rb_get(ringbuf_t *rb, uint8_t *byte)
{
    if (rb->count == 0u) { return false; }         /* empty */
    *byte = rb->buf[rb->tail];
    rb->tail = (uint8_t)((rb->tail + 1u) % RB_SIZE);
    rb->count--;
    return true;
}

uint8_t rb_count(const ringbuf_t *rb)
{
    return rb->count;
}

/* 10. CRC-8 SAE J1850
 * For each byte: XOR it into the CRC, then for each of its 8 bits shift left,
 * and if a 1 fell off the top, XOR in the polynomial 0x1D.
 */
uint8_t crc8_j1850(const uint8_t *data, uint32_t len)
{
    uint8_t crc = 0xFFu;                           /* initial value */
    uint32_t i;
    uint8_t bit;

    for (i = 0u; i < len; i++) {
        crc ^= data[i];
        for (bit = 0u; bit < 8u; bit++) {
            if ((crc & 0x80u) != 0u) {
                crc = (uint8_t)((uint8_t)(crc << 1) ^ 0x1Du);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return (uint8_t)(crc ^ 0xFFu);                 /* final XOR */
}
