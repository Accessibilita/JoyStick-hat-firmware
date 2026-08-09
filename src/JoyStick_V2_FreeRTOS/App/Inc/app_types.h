#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    APP_SAFETY_RESET = 0,
    APP_SAFETY_BOOT_SELF_TEST,
    APP_SAFETY_CONFIGURATION_CHECK,
    APP_SAFETY_WAIT_FOR_LINK,
    APP_SAFETY_WAIT_FOR_NEUTRAL,
    APP_SAFETY_READY,
    APP_SAFETY_DRIVE_AUTHORIZED,
    APP_SAFETY_RECOVERABLE_INHIBIT,
    APP_SAFETY_LATCHED_FAULT
} AppSafetyState;

typedef uint32_t AppFaultMask;

#define APP_FAULT_NONE                         ((AppFaultMask)0U)
#define APP_FAULT_CONFIGURATION_INVALID        ((AppFaultMask)(1UL << 0))
#define APP_FAULT_INPUT_STALE                  ((AppFaultMask)(1UL << 1))
#define APP_FAULT_JOYSTICK_X_RANGE             ((AppFaultMask)(1UL << 2))
#define APP_FAULT_JOYSTICK_Y_RANGE             ((AppFaultMask)(1UL << 3))
#define APP_FAULT_LINK_INVALID                 ((AppFaultMask)(1UL << 4))
#define APP_FAULT_LINK_STALE                   ((AppFaultMask)(1UL << 5))
#define APP_FAULT_TASK_HEALTH                  ((AppFaultMask)(1UL << 6))
#define APP_FAULT_ADC_OVERRUN                  ((AppFaultMask)(1UL << 7))
#define APP_FAULT_POWER_GOOD_LOST               ((AppFaultMask)(1UL << 8))
#define APP_FAULT_INPUT_INVALID                 ((AppFaultMask)(1UL << 9))
#define APP_FAULT_PROTOCOL_CRC                 ((AppFaultMask)(1UL << 10))
#define APP_FAULT_PROTOCOL_FRAME               ((AppFaultMask)(1UL << 11))
#define APP_FAULT_PROTOCOL_SEQUENCE            ((AppFaultMask)(1UL << 12))
#define APP_FAULT_PROTOCOL_SESSION             ((AppFaultMask)(1UL << 13))
#define APP_FAULT_REMOTE_CONTROLLER            ((AppFaultMask)(1UL << 14))
#define APP_FAULT_REMOTE_NOT_READY             ((AppFaultMask)(1UL << 15))
#define APP_FAULT_COMMAND_STALE                ((AppFaultMask)(1UL << 16))
#define APP_FAULT_PROTOCOL_ADDRESS             ((AppFaultMask)(1UL << 17))
#define APP_FAULT_INTERNAL_INVARIANT           ((AppFaultMask)(1UL << 31))

#define APP_FAULT_CRITICAL_MASK                \
    (APP_FAULT_INTERNAL_INVARIANT)

typedef struct
{
    uint32_t sequence;
    uint32_t captured_at_ms;
    uint16_t joystick_y_counts;
    uint16_t joystick_x_counts;
    uint16_t buttons_active_low;
    uint8_t rotary_1_active_low;
    uint8_t rotary_2_active_low;
    bool power_good;
    bool dma_overrun_detected;
} AppRawInputSnapshot;

typedef struct
{
    uint32_t receive_sequence;
    uint32_t last_valid_packet_ms;
    uint32_t remote_faults;
    uint32_t link_fault_history;
    uint32_t controller_session_id;
    uint32_t acknowledged_command_sequence;
    uint8_t local_node_address;
    uint8_t remote_node_address;
    uint8_t protocol_state;
    bool link_valid;
    bool remote_drive_ready;
    bool brakes_confirmed;
    bool command_accepted;
} AppMotorLinkStatus;

typedef struct
{
    uint32_t publication_sequence;
    uint32_t input_sequence;
    uint32_t generated_at_ms;
    int16_t forward_q15;
    int16_t turn_q15;
    uint16_t maximum_speed_q15;
    AppSafetyState safety_state;
    AppFaultMask active_faults;
    bool drive_authorized;
} AppAuthorizedDriveCommand;

typedef struct
{
    bool configuration_valid;
    bool input_valid;
    bool input_neutral;
    bool link_valid;
    bool link_fresh;
    bool enable_request;
    bool mandatory_tasks_healthy;
    uint32_t now_ms;
    AppFaultMask observed_faults;
} AppSafetyObservation;

typedef struct
{
    AppSafetyState state;
    AppFaultMask active_faults;
    uint32_t state_entered_at_ms;
    uint32_t neutral_started_at_ms;
    bool neutral_timer_running;
} AppSafetyContext;

#endif /* APP_TYPES_H */
