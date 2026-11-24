#include "j1939.h"

/* 29-bit identifier layout (bit 28 is the first bit sent):
 *   28..26  priority      (3 bits)
 *   25      extended data page (EDP)
 *   24      data page (DP)
 *   23..16  PDU format (PF)
 *   15..8   PDU specific (PS): destination address OR group extension
 *   7..0    source address (SA)
 */
void j1939_parse_id(uint32_t can_id, j1939_id_t *out)
{
    uint32_t edp = (can_id >> 25) & 0x1u;
    uint32_t dp  = (can_id >> 24) & 0x1u;
    uint32_t pf  = (can_id >> 16) & 0xFFu;
    uint32_t ps  = (can_id >> 8)  & 0xFFu;

    out->priority = (uint8_t)((can_id >> 26) & 0x7u);
    out->source   = (uint8_t)(can_id & 0xFFu);

    if (pf < 240u) {
        /* PDU1: peer-to-peer. PS is the destination and is NOT part of the PGN. */
        out->pgn  = (edp << 17) | (dp << 16) | (pf << 8);
        out->dest = (uint8_t)ps;
    } else {
        /* PDU2: broadcast. PS is a group extension and IS part of the PGN. */
        out->pgn  = (edp << 17) | (dp << 16) | (pf << 8) | ps;
        out->dest = J1939_ADDR_GLOBAL;
    }
}

j1939_status_t j1939_eec1_engine_speed(const uint8_t data[8], uint16_t *rpm)
{
    /* J1939 numbers bytes from 1, so "bytes 4-5" are data[3] and data[4]. Intel byte order. */
    uint16_t raw = (uint16_t)(((uint16_t)data[4] << 8) | data[3]);

    if (raw >= 0xFF00u) { return J1939_NOT_AVAILABLE; }
    if (raw >= 0xFE00u) { return J1939_ERROR; }
    if (raw >  0xFAFFu) { return J1939_INVALID; }   /* above the valid data range */

    *rpm = (uint16_t)(raw / 8u);                      /* 0.125 rpm per bit = divide by 8 */
    return J1939_VALID;
}
