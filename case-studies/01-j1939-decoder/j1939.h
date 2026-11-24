/* Case study 01: decoding J1939 messages (heavy vehicles, buses, tractors, gensets). */
#ifndef J1939_H
#define J1939_H

#include <stdint.h>

#define J1939_PGN_EEC1    61444u   /* 0xF004: Electronic Engine Controller 1 */
#define J1939_ADDR_GLOBAL 0xFFu    /* "everyone" destination for broadcast PGNs */

typedef struct {
    uint8_t  priority;   /* 0 = highest, 7 = lowest */
    uint32_t pgn;        /* parameter group number: WHAT the message is */
    uint8_t  dest;       /* destination address, or 0xFF for broadcast */
    uint8_t  source;     /* source address: WHO sent it */
} j1939_id_t;

typedef enum {
    J1939_VALID,          /* normal value */
    J1939_ERROR,          /* sender reports a sensor or ECU error (0xFE00-0xFEFF) */
    J1939_NOT_AVAILABLE,  /* sender doesn't support this value (0xFF00-0xFFFF) */
    J1939_INVALID         /* reserved range: treat as unusable */
} j1939_status_t;

/* Split a 29-bit extended CAN identifier into its J1939 fields. */
void j1939_parse_id(uint32_t can_id, j1939_id_t *out);

/* Engine speed (SPN 190) from an EEC1 frame: bytes 4-5, little-endian, 0.125 rpm per bit.
 * On J1939_VALID, *rpm is set to whole rpm (fraction dropped). */
j1939_status_t j1939_eec1_engine_speed(const uint8_t data[8], uint16_t *rpm);

#endif
