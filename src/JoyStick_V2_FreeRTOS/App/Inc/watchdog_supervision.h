#ifndef WATCHDOG_SUPERVISION_H
#define WATCHDOG_SUPERVISION_H

#include <stdbool.h>

void WatchdogSupervision_Init(void);
void WatchdogSupervision_RefreshIfHealthy(
    bool mandatory_tasks_healthy,
    bool no_latched_fault);

#endif /* WATCHDOG_SUPERVISION_H */
