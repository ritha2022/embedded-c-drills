/* Case study 03: cell over-voltage protection in a battery management system (BMS). */
#ifndef OV_MONITOR_H
#define OV_MONITOR_H

#include <stdint.h>
#include <stdbool.h>

/* Example values for a Li-ion NMC cell. Real limits come from the cell datasheet
 * and the project's safety concept, never from example code. */
#define OV_SET_MV          4200u  /* above this: over-voltage                       */
#define OV_CLEAR_MV        4100u  /* below this: over-voltage may clear (hysteresis) */
#define OV_SET_SAMPLES        3u  /* consecutive samples to confirm the fault        */
#define OV_CLEAR_SAMPLES      5u  /* consecutive samples to clear it                 */
#define PLAUSIBLE_MIN_MV   1000u  /* below this the reading can't be a real cell:    */
#define PLAUSIBLE_MAX_MV   5000u  /* broken wire, short or failed measurement chip   */

typedef enum { CELL_OK, CELL_OV_FAULT, CELL_SENSOR_FAULT } cell_state_t;

typedef struct {
    cell_state_t state;
    uint8_t set_count;
    uint8_t clear_count;
} ov_monitor_t;

void         ov_init(ov_monitor_t *m);
cell_state_t ov_update(ov_monitor_t *m, uint16_t cell_mv);   /* call every 100 ms */
bool         ov_charging_allowed(const ov_monitor_t *m);

#endif
