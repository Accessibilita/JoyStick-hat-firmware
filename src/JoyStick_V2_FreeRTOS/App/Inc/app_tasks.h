#ifndef APP_TASKS_H
#define APP_TASKS_H

void SafetyControlTask_Run(void *argument);
void Rs485LinkTask_Run(void *argument);
void HmiTask_Run(void *argument);
void DiagnosticsTask_Run(void *argument);

#endif /* APP_TASKS_H */
