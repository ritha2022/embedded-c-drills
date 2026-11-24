#include "ov_monitor.h"

void ov_init(ov_monitor_t *m)
{
    m->state = CELL_OK;
    m->set_count = 0u;
    m->clear_count = 0u;
}

cell_state_t ov_update(ov_monitor_t *m, uint16_t cell_mv)
{
    /* 1. Plausibility first: an impossible reading must never look like a safe one. */
    if ((cell_mv < PLAUSIBLE_MIN_MV) || (cell_mv > PLAUSIBLE_MAX_MV)) {
        m->state = CELL_SENSOR_FAULT;
        m->set_count = 0u;
        m->clear_count = 0u;
        return m->state;
    }
    if (m->state == CELL_SENSOR_FAULT) {
        m->state = CELL_OK;   /* reading is plausible again: re-evaluate from a clean start */
    }

    /* 2. Debounced fault detection with hysteresis. */
    if (m->state == CELL_OK) {
        if (cell_mv > OV_SET_MV) {
            if (++m->set_count >= OV_SET_SAMPLES) {   /* one noise spike is not enough */
                m->state = CELL_OV_FAULT;
                m->set_count = 0u;
            }
        } else {
            m->set_count = 0u;                         /* the run must be consecutive */
        }
    } else { /* CELL_OV_FAULT */
        if (cell_mv < OV_CLEAR_MV) {
            if (++m->clear_count >= OV_CLEAR_SAMPLES) {
                m->state = CELL_OK;
                m->clear_count = 0u;
            }
        } else {
            m->clear_count = 0u;   /* between 4100 and 4200 mV: stay in fault (hysteresis) */
        }
    }
    return m->state;
}

bool ov_charging_allowed(const ov_monitor_t *m)
{
    /* Fail safe: charge only when we positively know the cell is fine. */
    return m->state == CELL_OK;
}
