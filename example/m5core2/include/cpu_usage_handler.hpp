#ifndef CPU_USAGE_MONITOR_HPP
#define CPU_USAGE_MONITOR_HPP

#include <M5Core2.h>


volatile static unsigned long idle_counter = 0;

static void idleTask(void * param)
{
  while (true) {
    idle_counter++;
    delay(1);
  }
}

static float getCPUUsage()
{
  static unsigned long prev_idle = 0;
  static unsigned long prev_time = 0;

  unsigned long current_idle = idle_counter;
  unsigned long current_time = millis();

  if (prev_time == 0) {
    prev_idle = current_idle;
    prev_time = current_time;
    return 0.0f;
  }

  unsigned long delta_idle = current_idle - prev_idle;
  unsigned long delta_time = current_time - prev_time;

  float idle_percent = (delta_idle * 1.0f) / delta_time;
  float usage_percent = 100.0f - idle_percent;

  prev_idle = current_idle;
  prev_time = current_time;

  return usage_percent;
}

static void encodeCPUUsage(float usage_percent, uint8_t out[4])
{
  union {
    float f;
    uint8_t b[4];
  } converter;

  converter.f = usage_percent;

  for (int i = 0; i < 4; ++i) {
    out[i] = converter.b[i];
  }
}

static void startCPUUsageMonitor()
{
  xTaskCreatePinnedToCore(idleTask, "IdleTask", 2048, NULL, 0, NULL, 1);  // Core 1
}

#endif  // CPU_USAGE_MONITOR_HPP
