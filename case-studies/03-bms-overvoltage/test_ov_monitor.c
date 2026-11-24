#include <stdio.h>
#include "ov_monitor.h"

static int failed;
#define CHECK(c) do { if (!(c)) { failed++; printf("  FAIL line %d: %s\n", __LINE__, #c); } } while (0)

int main(void)
{
    ov_monitor_t m; uint8_t i;
    ov_init(&m);

    CHECK(ov_update(&m, 3700u) == CELL_OK && ov_charging_allowed(&m));

    /* A single noise spike above 4200 mV must NOT trip the fault */
    CHECK(ov_update(&m, 4300u) == CELL_OK);
    CHECK(ov_update(&m, 4150u) == CELL_OK);

    /* Three samples in a row above the limit: fault, charging stops */
    CHECK(ov_update(&m, 4210u) == CELL_OK);
    CHECK(ov_update(&m, 4220u) == CELL_OK);
    CHECK(ov_update(&m, 4230u) == CELL_OV_FAULT);
    CHECK(!ov_charging_allowed(&m));

    /* Between 4100 and 4200 mV: still a fault (hysteresis stops on/off chattering) */
    for (i = 0u; i < 10u; i++) { CHECK(ov_update(&m, 4150u) == CELL_OV_FAULT); }

    /* Below 4100 mV, the fault clears only after 5 samples in a row */
    for (i = 0u; i < 4u; i++) { CHECK(ov_update(&m, 4050u) == CELL_OV_FAULT); }
    CHECK(ov_update(&m, 4050u) == CELL_OK);
    CHECK(ov_charging_allowed(&m));

    /* Broken sense wire reads 0 mV: must be a sensor fault, NOT a "very healthy" low cell */
    CHECK(ov_update(&m, 0u) == CELL_SENSOR_FAULT);
    CHECK(!ov_charging_allowed(&m));
    CHECK(ov_update(&m, 6000u) == CELL_SENSOR_FAULT);

    /* Sensor recovers: back to normal checking */
    CHECK(ov_update(&m, 3800u) == CELL_OK);

    printf("  case study 03 (BMS over-voltage): %s\n", failed ? "FAILED" : "all checks passed");
    return failed ? 1 : 0;
}
