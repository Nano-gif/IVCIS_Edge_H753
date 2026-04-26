#include "Radar_Manager.h"
#include "app_config.h"
#include "cmsis_os.h"
#include "stm32h7xx_hal.h"
#include <string.h>

static RadarSummary_t s_summary;
static uint32_t s_last_update_ms;
static osMutexId_t s_radar_lock;

static bool radar_lock(void) {
  if (s_radar_lock == NULL) {
    return true;
  }
  return (osMutexAcquire(s_radar_lock, osWaitForever) == osOK);
}

static void radar_unlock(void) {
  if (s_radar_lock != NULL) {
    (void)osMutexRelease(s_radar_lock);
  }
}

__attribute__((weak)) bool Radar_DriverRead(RadarSummary_t *out_summary) {
  (void)out_summary;
  return false;
}

static bool radar_in_capture_zone(const RadarSummary_t *summary) {
  if ((summary == NULL) || !summary->valid || (summary->target_count == 0U)) {
    return false;
  }

  if (summary->confidence < RADAR_MIN_CONFIDENCE) {
    return false;
  }

  if ((summary->range_cm < RADAR_CAPTURE_MIN_RANGE_CM) ||
      (summary->range_cm > RADAR_CAPTURE_MAX_RANGE_CM)) {
    return false;
  }

  if (summary->direction == RADAR_DIR_LEAVING) {
    return false;
  }

  return true;
}

void Radar_Init(void) {
  if (s_radar_lock == NULL) {
    s_radar_lock = osMutexNew(NULL);
  }

  bool locked = radar_lock();
  memset(&s_summary, 0, sizeof(s_summary));
  s_last_update_ms = HAL_GetTick();
  if (locked) {
    radar_unlock();
  }
}

bool Radar_UpdateSummary(const RadarSummary_t *summary) {
  if (summary == NULL) {
    return false;
  }

  if (!radar_lock()) {
    return false;
  }
  s_summary = *summary;
  s_summary.tick_ms = HAL_GetTick();
  s_last_update_ms = s_summary.tick_ms;
  bool capture = radar_in_capture_zone(&s_summary);
  radar_unlock();

  return capture;
}

bool Radar_Poll(RadarSummary_t *out_summary, bool *out_capture_request) {
  RadarSummary_t latest;
  if (!Radar_DriverRead(&latest)) {
    if (out_capture_request != NULL) {
      *out_capture_request = false;
    }
    return false;
  }

  bool capture = Radar_UpdateSummary(&latest);
  (void)Radar_GetSummaryCopy(out_summary);
  if (out_capture_request != NULL) {
    *out_capture_request = capture;
  }

  return true;
}

bool Radar_GetSummaryCopy(RadarSummary_t *out_summary) {
  if (out_summary == NULL) {
    return false;
  }

  if (!radar_lock()) {
    return false;
  }
  *out_summary = s_summary;
  radar_unlock();
  return true;
}

RadarSummary_t Radar_GetSummary(void) {
  RadarSummary_t summary = {0};
  (void)Radar_GetSummaryCopy(&summary);
  return summary;
}

bool Radar_IsTimedOut(void) {
  uint32_t last_update_ms = 0U;
  if (!radar_lock()) {
    return true;
  }
  last_update_ms = s_last_update_ms;
  radar_unlock();

  return ((HAL_GetTick() - last_update_ms) > RADAR_TARGET_TIMEOUT_MS);
}
