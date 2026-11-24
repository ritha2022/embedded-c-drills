/* Case study 02: end-to-end (E2E) protection of a safety-relevant CAN message,
 * in the style of AUTOSAR E2E: a CRC and an alive counter in every frame. */
#ifndef E2E_H
#define E2E_H

#include <stdint.h>
#include <stdbool.h>

/* Frame layout (8 bytes):
 *   byte 0      CRC-8 SAE J1850
 *   byte 1      bits 0-3: alive counter (0..15), bits 4-7: unused (0)
 *   bytes 2..7  application data
 * The CRC also covers a 16-bit Data ID that is never sent: it identifies the message. */

typedef struct {
    uint16_t data_id;
    uint8_t  counter;
} e2e_tx_t;

typedef enum {
    E2E_OK,             /* new frame, next counter value */
    E2E_OK_SOME_LOST,   /* new frame, a few frames were lost but within max_delta */
    E2E_INITIAL,        /* first frame after start-up: accepted, counter learned */
    E2E_REPEATED,       /* same counter as before: stale or replayed frame */
    E2E_WRONG_SEQUENCE, /* counter jumped too far: too many frames lost */
    E2E_ERROR           /* CRC mismatch: corrupted, or a different message (wrong Data ID) */
} e2e_status_t;

typedef struct {
    uint16_t data_id;
    uint8_t  max_delta;     /* largest allowed counter jump, e.g. 2 */
    uint8_t  last_counter;
    bool     initialised;
} e2e_rx_t;

void         e2e_tx_init(e2e_tx_t *tx, uint16_t data_id);
void         e2e_protect(e2e_tx_t *tx, uint8_t frame[8]);       /* fills bytes 0 and 1 */
void         e2e_rx_init(e2e_rx_t *rx, uint16_t data_id, uint8_t max_delta);
e2e_status_t e2e_check(e2e_rx_t *rx, const uint8_t frame[8]);
bool         e2e_data_usable(e2e_status_t s);                    /* may the application use it? */

#endif
