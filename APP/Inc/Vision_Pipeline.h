#ifndef VISION_PIPELINE_H
#define VISION_PIPELINE_H

#include "shared_types.h"
#include <stdint.h>

/**
 * (VisionMode_t defined in shared_types.h)
 */

/** Initialize OV5640 camera + DCMI, default JPEG mode. */
int8_t Vision_Init(void);

/** Switch between JPEG and Gray modes. */
void Vision_SetMode(VisionMode_t mode);

/** Start a single-frame DCMI+DMA capture (synchronous). */
void Vision_CaptureStart(void);

/** Check if a complete frame has been received. */
uint8_t Vision_IsFrameReady(void);

/** Get pointer to the completed frame buffer (NULL if not ready). */
uint8_t *Vision_GetFrameBuffer(void);

/** Get the actual byte count of the completed frame (0 if not ready). */
uint32_t Vision_GetFrameSize(void);

/** Send current JPEG frame over UART for XCAM viewer. */
void Vision_SendFrameUART(void);

#endif /* VISION_PIPELINE_H */
