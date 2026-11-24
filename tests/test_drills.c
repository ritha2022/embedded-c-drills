/* Tests for every drill. Only drills marked DONE in drills.h are run. */
#include <stdio.h>
#include "drills.h"

static int passed, failed, skipped;
#define CHECK(cond) do { if (cond) { passed++; } else { failed++; \
    printf("  FAIL line %d: %s\n", __LINE__, #cond); } } while (0)
#define DRILL(flag, name) if (!(flag)) { skipped++; printf("  -    %s (not done yet)\n", name); } \
    else if (printf("  run  %s\n", name) >= 0)

int main(void)
{
    printf("embedded-c-drills\n");

    DRILL(DONE_01_BITS, "01 bits") {
        CHECK(bit_set(0x00u, 3) == 0x08u);
        CHECK(bit_set(0x08u, 3) == 0x08u);
        CHECK(bit_set(0u, 31) == 0x80000000u);
        CHECK(bit_clear(0xFFu, 0) == 0xFEu);
        CHECK(bit_clear(0x80000000u, 31) == 0u);
        CHECK(bit_toggle(0x0Fu, 4) == 0x1Fu);
        CHECK(bit_toggle(0x1Fu, 4) == 0x0Fu);
        CHECK(bit_check(0x20u, 5) == 1u);
        CHECK(bit_check(0x20u, 4) == 0u);
        CHECK(bit_check(0x80000000u, 31) == 1u);
    }
    DRILL(DONE_02_POW2, "02 power of two") {
        CHECK(!is_pow2(0u)); CHECK(is_pow2(1u)); CHECK(is_pow2(2u)); CHECK(!is_pow2(3u));
        CHECK(is_pow2(1024u)); CHECK(!is_pow2(1023u));
        CHECK(is_pow2(0x80000000u)); CHECK(!is_pow2(0xFFFFFFFFu));
    }
    DRILL(DONE_03_COUNT_BITS, "03 count bits") {
        CHECK(count_bits(0u) == 0u); CHECK(count_bits(1u) == 1u); CHECK(count_bits(0xF0u) == 4u);
        CHECK(count_bits(0xFFFFFFFFu) == 32u); CHECK(count_bits(0x80000001u) == 2u);
    }
    DRILL(DONE_04_ENDIAN, "04 endianness") {
        const uint8_t b[2] = { 0x12u, 0x34u };
        const uint8_t c[2] = { 0xFFu, 0x01u };
        CHECK(read_be16(b) == 0x1234u);
        CHECK(read_le16(b) == 0x3412u);
        CHECK(read_be16(c) == 0xFF01u);
        CHECK(read_le16(c) == 0x01FFu);
    }
    DRILL(DONE_05_CAN_SIGNAL, "05 CAN signal") {
        const uint8_t f1[8] = { 0, 0, 0x02u, 0xBCu, 0, 0, 0, 0 };           /* raw 700  ->  30.0 degC */
        const uint8_t f2[8] = { 0, 0, 0x00u, 0x00u, 0, 0, 0, 0 };           /* raw 0    -> -40.0 degC */
        const uint8_t f3[8] = { 0, 0, 0x01u, 0x90u, 0, 0, 0, 0 };           /* raw 400  ->   0.0 degC */
        const uint8_t f4[8] = { 0xAAu, 0xBBu, 0x05u, 0x14u, 0xCCu, 0, 0, 0 }; /* raw 1300 ->  90.0 degC */
        CHECK(battery_temp_deci_c(f1) == 300);
        CHECK(battery_temp_deci_c(f2) == -400);
        CHECK(battery_temp_deci_c(f3) == 0);
        CHECK(battery_temp_deci_c(f4) == 900);
    }
    DRILL(DONE_06_ALIVE, "06 alive counter") {
        CHECK(alive_check(3u, 4u) == ALIVE_OK);
        CHECK(alive_check(15u, 0u) == ALIVE_OK);         /* wraps */
        CHECK(alive_check(7u, 7u) == ALIVE_STUCK);
        CHECK(alive_check(7u, 9u) == ALIVE_LOST);
        CHECK(alive_check(14u, 1u) == ALIVE_LOST);
        CHECK(alive_check(0x13u, 0x24u) == ALIVE_OK);    /* only the low 4 bits count: 3 -> 4 */
    }
    DRILL(DONE_07_TIMEOUT, "07 timeout with wraparound") {
        CHECK(!timed_out(1100u, 1000u, 100u));              /* exactly 100 ms is not "more than" */
        CHECK(timed_out(1101u, 1000u, 100u));
        CHECK(!timed_out(0x00000010u, 0xFFFFFFF0u, 100u));  /* 32 ms across the wrap  */
        CHECK(timed_out(0x00000080u, 0xFFFFFFF0u, 100u));   /* 144 ms across the wrap */
    }
    DRILL(DONE_08_DEBOUNCE, "08 debounce") {
        debounce_t d;
        debounce_init(&d, 0u);
        CHECK(debounce_update(&d, 1u) == 0u);
        CHECK(debounce_update(&d, 1u) == 0u);
        CHECK(debounce_update(&d, 0u) == 0u);   /* a bounce restarts the run */
        CHECK(debounce_update(&d, 1u) == 0u);
        CHECK(debounce_update(&d, 1u) == 0u);
        CHECK(debounce_update(&d, 1u) == 0u);
        CHECK(debounce_update(&d, 1u) == 1u);   /* 4 equal samples: accept */
        CHECK(debounce_update(&d, 0u) == 1u);   /* one glitch does not flip it */
        CHECK(debounce_update(&d, 1u) == 1u);
    }
    DRILL(DONE_09_RINGBUF, "09 ring buffer") {
        ringbuf_t rb;
        uint8_t v = 0u, i, n = 0u;
        rb_init(&rb);
        CHECK(rb_count(&rb) == 0u);
        CHECK(!rb_get(&rb, &v));
        for (i = 0u; i < 20u && rb_put(&rb, i); i++) { n++; }
        CHECK(n == RB_SIZE || n == RB_SIZE - 1u);        /* both designs are accepted */
        CHECK(rb_count(&rb) == n);
        CHECK(!rb_put(&rb, 99u));
        CHECK(rb_get(&rb, &v) && v == 0u);                /* first in, first out */
        CHECK(rb_get(&rb, &v) && v == 1u);
        CHECK(rb_put(&rb, 50u) && rb_put(&rb, 51u));      /* wraps around the end */
        for (i = 2u; i < n; i++) { CHECK(rb_get(&rb, &v) && v == i); }
        CHECK(rb_get(&rb, &v) && v == 50u);
        CHECK(rb_get(&rb, &v) && v == 51u);
        CHECK(rb_count(&rb) == 0u);
    }
    DRILL(DONE_10_CRC8, "10 CRC-8 SAE J1850") {
        const uint8_t check[9] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };
        const uint8_t zeros[4] = { 0u, 0u, 0u, 0u };
        CHECK(crc8_j1850(check, 9u) == 0x4Bu);   /* the standard check value */
        CHECK(crc8_j1850(zeros, 4u) == 0x59u);
        CHECK(crc8_j1850(check, 0u) == 0x00u);   /* empty input: 0xFF xor 0xFF */
    }

    printf("\n%d checks passed, %d failed, %d drills not done yet\n", passed, failed, skipped);
    return failed ? 1 : 0;
}
