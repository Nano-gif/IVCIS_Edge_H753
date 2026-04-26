#ifndef RADAR_MANAGER_H
#define RADAR_MANAGER_H

#include "shared_types.h"
#include <stdbool.h>

void Radar_Init(void);
bool Radar_UpdateSummary(const RadarSummary_t *summary);
bool Radar_Poll(RadarSummary_t *out_summary, bool *out_capture_request);
bool Radar_GetSummaryCopy(RadarSummary_t *out_summary);
RadarSummary_t Radar_GetSummary(void);
bool Radar_IsTimedOut(void);

#endif /* RADAR_MANAGER_H */
