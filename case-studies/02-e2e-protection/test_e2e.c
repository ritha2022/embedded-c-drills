#include <stdio.h>
#include <string.h>
#include "e2e.h"

static int failed;
#define CHECK(c) do { if (!(c)) { failed++; printf("  FAIL line %d: %s\n", __LINE__, #c); } } while (0)

static void make_frame(e2e_tx_t *tx, uint8_t f[8], uint8_t speed)
{
    memset(f, 0, 8);
    f[2] = speed;            /* e.g. vehicle speed in km/h */
    e2e_protect(tx, f);
}

int main(void)
{
    e2e_tx_t tx; e2e_rx_t rx; uint8_t f[8], copy[8]; uint8_t i;
    e2e_tx_init(&tx, 0x0123u);
    e2e_rx_init(&rx, 0x0123u, 2u);

    make_frame(&tx, f, 60u);  CHECK(e2e_check(&rx, f) == E2E_INITIAL);
    make_frame(&tx, f, 61u);  CHECK(e2e_check(&rx, f) == E2E_OK);
    memcpy(copy, f, 8);

    /* Replay attack or frozen sender: the same frame again */
    CHECK(e2e_check(&rx, copy) == E2E_REPEATED);
    CHECK(!e2e_data_usable(E2E_REPEATED));

    /* Corruption: one data bit flips on the bus (and the CAN CRC somehow missed it) */
    make_frame(&tx, f, 62u);
    f[2] ^= 0x01u;
    CHECK(e2e_check(&rx, f) == E2E_ERROR);

    /* One frame lost: counter jumps by 2, still within max_delta */
    make_frame(&tx, f, 63u);
    CHECK(e2e_check(&rx, f) == E2E_OK_SOME_LOST);

    /* Many frames lost: counter jumps by 5 */
    for (i = 0u; i < 4u; i++) { make_frame(&tx, f, 64u); }
    make_frame(&tx, f, 65u);
    CHECK(e2e_check(&rx, f) == E2E_WRONG_SEQUENCE);
    CHECK(!e2e_data_usable(E2E_WRONG_SEQUENCE));

    /* Counter wraps 15 -> 0 without a false error */
    for (i = 0u; i < 20u; i++) { make_frame(&tx, f, 70u); CHECK(e2e_check(&rx, f) == E2E_OK); }

    /* Masquerade: a different message (Data ID 0x0456) on the same CAN ID is rejected */
    { e2e_tx_t other; e2e_tx_init(&other, 0x0456u); make_frame(&other, f, 99u); }
    CHECK(e2e_check(&rx, f) == E2E_ERROR);

    printf("  case study 02 (E2E protection): %s\n", failed ? "FAILED" : "all checks passed");
    return failed ? 1 : 0;
}
