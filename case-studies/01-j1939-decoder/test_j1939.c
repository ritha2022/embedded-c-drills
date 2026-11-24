#include <stdio.h>
#include "j1939.h"

static int failed;
#define CHECK(c) do { if (!(c)) { failed++; printf("  FAIL line %d: %s\n", __LINE__, #c); } } while (0)

int main(void)
{
    j1939_id_t id;
    uint16_t rpm = 0u;

    /* EEC1 from the engine (SA 0x00) at priority 3: CAN ID 0x0CF00400 */
    j1939_parse_id(0x0CF00400u, &id);
    CHECK(id.priority == 3u);
    CHECK(id.pgn == J1939_PGN_EEC1);
    CHECK(id.source == 0x00u);
    CHECK(id.dest == J1939_ADDR_GLOBAL);

    /* PDU1 request (PGN 59904 = 0xEA00) from SA 0xF9 to DA 0x00: CAN ID 0x18EA00F9 */
    j1939_parse_id(0x18EA00F9u, &id);
    CHECK(id.priority == 6u);
    CHECK(id.pgn == 59904u);   /* destination is NOT inside the PGN */
    CHECK(id.dest == 0x00u);
    CHECK(id.source == 0xF9u);

    /* Engine speed: raw 0x1F40 = 8000 -> 1000 rpm */
    const uint8_t idle[8]  = { 0xF0u, 0x7Du, 0x7Du, 0x40u, 0x1Fu, 0x00u, 0xF0u, 0x7Du };
    CHECK(j1939_eec1_engine_speed(idle, &rpm) == J1939_VALID && rpm == 1000u);

    /* raw 0x5DC0 = 24000 -> 3000 rpm */
    const uint8_t cruise[8] = { 0, 0, 0, 0xC0u, 0x5Du, 0, 0, 0 };
    CHECK(j1939_eec1_engine_speed(cruise, &rpm) == J1939_VALID && rpm == 3000u);

    const uint8_t na[8]    = { 0, 0, 0, 0xFFu, 0xFFu, 0, 0, 0 };
    const uint8_t err[8]   = { 0, 0, 0, 0x00u, 0xFEu, 0, 0, 0 };
    const uint8_t resv[8]  = { 0, 0, 0, 0x00u, 0xFBu, 0, 0, 0 };
    CHECK(j1939_eec1_engine_speed(na, &rpm) == J1939_NOT_AVAILABLE);
    CHECK(j1939_eec1_engine_speed(err, &rpm) == J1939_ERROR);
    CHECK(j1939_eec1_engine_speed(resv, &rpm) == J1939_INVALID);

    printf("  case study 01 (J1939 decoder): %s\n", failed ? "FAILED" : "all checks passed");
    return failed ? 1 : 0;
}
