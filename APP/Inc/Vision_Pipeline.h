#ifndef VISION_PIPELINE_H
#define VISION_PIPELINE_H

#include <stdint.h>
#include "app_config.h"

int8_t Vision_Init(void);

extern uint32_t half_transfer_count;
extern uint32_t full_transfer_count;
extern uint32_t last_jpeg_actual_size;
extern uint8_t  DCMI_Strip_Buf[2][STRIP_BUFFER_SIZE];
extern uint8_t  JPEG_Out_Buf[JPEG_OUT_BUFFER_SIZE];
extern volatile uint8_t jpeg_encode_complete;

#endif
