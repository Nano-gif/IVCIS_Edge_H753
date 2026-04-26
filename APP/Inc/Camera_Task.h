#ifndef CAMERA_TASK_H
#define CAMERA_TASK_H

#include <stdbool.h>

#define CAMERA_FRAME_DONE_FLAG (1UL << 0)

bool Camera_TaskEventsReady(void);
bool Camera_RequestCapture(void);
bool Camera_RequestHealthCheck(void);
void Camera_NotifyFrameDoneFromISR(void);

#endif /* CAMERA_TASK_H */
