/* Key-level typing instrument — public surface for the keymap.
 *
 * Counting is automatic (the community-module hook process_record_keylog), so
 * the keymap only needs these two actions to read the data out.
 */
#pragma once

#include "quantum.h"
#include "keylog_stats.h"

/* Print the counters to the HID console (`qmk console`). Counts only. */
void keylog_dump(void);

/* Zero every counter — start a fresh measurement window. */
void keylog_clear(void);

/* Read-only access, for anything that wants the numbers in-firmware. */
const keylog_stats_t *keylog_get(void);
