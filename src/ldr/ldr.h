#ifndef _LDR_H_
#define _LDR_H_

#include "Arduino.h"

#define LDR_PIN                 34

#define DEF_LDR_SAMPLE_INTERVAL 5 * 1000 // 5 seconds

#define DEF_LDR_GAMMA           0.7
#define DEF_LDR_RL10            33
#define ESP_VOLTAGE             3.3
#define ADC_RESOLUTION          4096.

#define MAX_INTENSITY           100916.58 // Obtained empirically
#define MIN_INTENSITY           0.1

extern long last_ldr_sample_time;
extern long ldr_sample_interval;

extern int max_ldr_samples;
extern int ldr_sample_index;
extern float* ldr_samples;

extern float light_intensity;
extern float ldr_average;

void sample_ldr();

#endif