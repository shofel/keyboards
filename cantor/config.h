#define MAX_DEFERRED_EXECUTORS 10

#define UNICODE_SELECTED_MODES UNICODE_MODE_LINUX
#define UNICODE_TYPE_DELAY 0

#define LEADER_PER_KEY_TIMING
#define LEADER_NO_TIMEOUT
#define LEADER_TIMEOUT 100

#define LAYER_STATE_8BIT
#define COMBO_ONLY_FROM_LAYER 0

/* MOUSE */

/*
|Define               |Default      |Description                                |
|---------------------|-------------|-------------------------------------------|
|`MK_3_SPEED`         |*Not defined*|Enable constant cursor speeds              |
|`MK_MOMENTARY_ACCEL` |*Not defined*|Enable momentary speed selection           |
|`MK_C_OFFSET_UNMOD`  |16           |Cursor offset per movement (unmodified)    |
|`MK_C_INTERVAL_UNMOD`|16           |Time between cursor movements (unmodified) |
|`MK_C_OFFSET_0`      |1            |Cursor offset per movement (`MS_ACL0`)     |
|`MK_C_INTERVAL_0`    |32           |Time between cursor movements (`MS_ACL0`)  |
|`MK_C_OFFSET_1`      |4            |Cursor offset per movement (`MS_ACL1`)     |
|`MK_C_INTERVAL_1`    |16           |Time between cursor movements (`MS_ACL1`)  |
|`MK_C_OFFSET_2`      |32           |Cursor offset per movement (`MS_ACL2`)     |
|`MK_C_INTERVAL_2`    |16           |Time between cursor movements (`MS_ACL2`)  |
|`MK_W_OFFSET_UNMOD`  |1            |Scroll steps per scroll action (unmodified)|
|`MK_W_INTERVAL_UNMOD`|40           |Time between scroll steps (unmodified)     |
|`MK_W_OFFSET_0`      |1            |Scroll steps per scroll action (`MS_ACL0`) |
|`MK_W_INTERVAL_0`    |360          |Time between scroll steps (`MS_ACL0`)      |
|`MK_W_OFFSET_1`      |1            |Scroll steps per scroll action (`MS_ACL1`) |
|`MK_W_INTERVAL_1`    |120          |Time between scroll steps (`MS_ACL1`)      |
|`MK_W_OFFSET_2`      |1            |Scroll steps per scroll action (`MS_ACL2`) |
|`MK_W_INTERVAL_2`    |20           |Time between scroll steps (`MS_ACL2`)      |
*/
#define MK_3_SPEED
#define MK_MOMENTARY_ACCEL
#define MK_C_OFFSET_UNMOD    7
#define MK_C_INTERVAL_UNMOD  10
#define MK_C_OFFSET_0        1
#define MK_C_INTERVAL_0      10
#define MK_C_OFFSET_1        3
#define MK_C_INTERVAL_1      10
#define MK_C_OFFSET_2        30
#define MK_C_INTERVAL_2      10
#define MK_W_OFFSET_UNMOD    1
#define MK_W_INTERVAL_UNMOD  40
#define MK_W_OFFSET_0        1
#define MK_W_INTERVAL_0      360
#define MK_W_OFFSET_1        1
#define MK_W_INTERVAL_1      120
#define MK_W_OFFSET_2        1
#define MK_W_INTERVAL_2      20
