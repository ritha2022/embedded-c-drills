#include "e2e.h"

/* CRC-8 SAE J1850: polynomial 0x1D, initial value 0xFF, final XOR 0xFF. */
static uint8_t crc8_step(uint8_t crc, uint8_t byte)
{
    uint8_t bit;
    crc ^= byte;
    for (bit = 0u; bit < 8u; bit++) {
        crc = ((crc & 0x80u) != 0u) ? (uint8_t)((uint8_t)(crc << 1) ^ 0x1Du) : (uint8_t)(crc << 1);
    }
    return crc;
}

static uint8_t e2e_crc(uint16_t data_id, const uint8_t frame[8])
{
    uint8_t crc = 0xFFu;
    uint8_t i;
    crc = crc8_step(crc, (uint8_t)(data_id & 0xFFu));   /* Data ID is part of the CRC ... */
    crc = crc8_step(crc, (uint8_t)(data_id >> 8));      /* ... but is never sent on the bus */
    for (i = 1u; i < 8u; i++) {                          /* byte 0 holds the CRC itself */
        crc = crc8_step(crc, frame[i]);
    }
    return (uint8_t)(crc ^ 0xFFu);
}

void e2e_tx_init(e2e_tx_t *tx, uint16_t data_id)
{
    tx->data_id = data_id;
    tx->counter = 0u;
}

void e2e_protect(e2e_tx_t *tx, uint8_t frame[8])
{
    frame[1] = (uint8_t)(tx->counter & 0x0Fu);
    frame[0] = e2e_crc(tx->data_id, frame);
    tx->counter = (uint8_t)((tx->counter + 1u) & 0x0Fu);   /* 15 wraps to 0 */
}

void e2e_rx_init(e2e_rx_t *rx, uint16_t data_id, uint8_t max_delta)
{
    rx->data_id = data_id;
    rx->max_delta = max_delta;
    rx->last_counter = 0u;
    rx->initialised = false;
}

e2e_status_t e2e_check(e2e_rx_t *rx, const uint8_t frame[8])
{
    uint8_t counter = (uint8_t)(frame[1] & 0x0Fu);
    uint8_t delta;

    if (frame[0] != e2e_crc(rx->data_id, frame)) {
        return E2E_ERROR;                    /* never learn a counter from a bad frame */
    }
    if (!rx->initialised) {
        rx->initialised = true;
        rx->last_counter = counter;
        return E2E_INITIAL;
    }

    delta = (uint8_t)((counter - rx->last_counter) & 0x0Fu);   /* works across 15 -> 0 */
    if (delta == 0u) {
        return E2E_REPEATED;                 /* don't move last_counter */
    }
    rx->last_counter = counter;              /* resynchronise on every new counter */
    if (delta == 1u)             { return E2E_OK; }
    if (delta <= rx->max_delta)  { return E2E_OK_SOME_LOST; }
    return E2E_WRONG_SEQUENCE;
}

bool e2e_data_usable(e2e_status_t s)
{
    return (s == E2E_OK) || (s == E2E_OK_SOME_LOST) || (s == E2E_INITIAL);
}
