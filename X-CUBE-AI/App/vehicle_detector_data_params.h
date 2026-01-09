/**
  ******************************************************************************
  * @file    vehicle_detector_data_params.h
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-01-10T04:11:13+0800
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#ifndef VEHICLE_DETECTOR_DATA_PARAMS_H
#define VEHICLE_DETECTOR_DATA_PARAMS_H

#include "ai_platform.h"

/*
#define AI_VEHICLE_DETECTOR_DATA_WEIGHTS_PARAMS \
  (AI_HANDLE_PTR(&ai_vehicle_detector_data_weights_params[1]))
*/

#define AI_VEHICLE_DETECTOR_DATA_CONFIG               (NULL)


#define AI_VEHICLE_DETECTOR_DATA_ACTIVATIONS_SIZES \
  { 129536, }
#define AI_VEHICLE_DETECTOR_DATA_ACTIVATIONS_SIZE     (129536)
#define AI_VEHICLE_DETECTOR_DATA_ACTIVATIONS_COUNT    (1)
#define AI_VEHICLE_DETECTOR_DATA_ACTIVATION_1_SIZE    (129536)



#define AI_VEHICLE_DETECTOR_DATA_WEIGHTS_SIZES \
  { 429468, }
#define AI_VEHICLE_DETECTOR_DATA_WEIGHTS_SIZE         (429468)
#define AI_VEHICLE_DETECTOR_DATA_WEIGHTS_COUNT        (1)
#define AI_VEHICLE_DETECTOR_DATA_WEIGHT_1_SIZE        (429468)



#define AI_VEHICLE_DETECTOR_DATA_ACTIVATIONS_TABLE_GET() \
  (&g_vehicle_detector_activations_table[1])

extern ai_handle g_vehicle_detector_activations_table[1 + 2];



#define AI_VEHICLE_DETECTOR_DATA_WEIGHTS_TABLE_GET() \
  (&g_vehicle_detector_weights_table[1])

extern ai_handle g_vehicle_detector_weights_table[1 + 2];


#endif    /* VEHICLE_DETECTOR_DATA_PARAMS_H */
