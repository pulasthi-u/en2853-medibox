#include "Arduino.h"
#include "ldr.h"

long ldr_sample_interval = DEF_LDR_SAMPLE_INTERVAL;
long last_ldr_sample_time = 0;

int ldr_sample_index = 0;
float* ldr_samples = (float*)malloc(max_ldr_samples * sizeof(float));

float light_intensity = 0;
float ldr_average = 0;

void sample_ldr() {
  // Following calculation is based on Wokwi's Documentation
  int raw_value = analogRead(LDR_PIN);
  float voltage = (raw_value / ADC_RESOLUTION) * ESP_VOLTAGE;
  float resistance = 2000 * voltage / (1 - voltage / ESP_VOLTAGE);
  light_intensity = pow(DEF_LDR_RL10 * 1e3 * pow(10, DEF_LDR_GAMMA) / resistance, (1 / DEF_LDR_GAMMA));

  // Normalize light intensity to a range from 0 to 1
  light_intensity = (light_intensity - MIN_INTENSITY) / (MAX_INTENSITY - MIN_INTENSITY);

  if (millis() - last_ldr_sample_time > ldr_sample_interval) {
    // Add to the array of samples every sampling instant
    last_ldr_sample_time = millis();

    if (ldr_sample_index < max_ldr_samples) {
        // Get previous total by multiplying average by number of samples (which as of now is still ldr_sample_index), add the new sample,
        // and divide by the new number of samples (ldr_sample_index + 1)
        ldr_samples[ldr_sample_index] = light_intensity;
        ldr_average = ((ldr_average * ldr_sample_index) + light_intensity) / (ldr_sample_index + 1);
    } else {
        // Array is full. Get previous total by multiplying the average by the total number of samples (max_ldr_samples = size of array),
        // subtract the oldest sample (which would be at position [ldr_sample_index % max_ldr_samples]), add the new sample, and divide 
        // by the number of samples (max_ldr_samples)
        int oldest_sample_index = ldr_sample_index % max_ldr_samples;
        ldr_average = ((ldr_average * max_ldr_samples) - ldr_samples[oldest_sample_index] + light_intensity) / max_ldr_samples;
        
        // Replace the oldest sample with the new one
        ldr_samples[oldest_sample_index] = light_intensity;
    }

    ldr_sample_index++;
  }
}