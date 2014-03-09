// Smart Cocktail Shaker
// Scale Calibration Sketch

// Created by Tony DiCola (tony@tonydicola.com)
// Released under an MIT license (http://opensource.org/licenses/MIT).

// Configuration:
#define ADC_PIN            5              // Set this to the analog input which is connected to the instrument amp output.

#define ADC_SAMPLES        5              // Number of ADC readings to average for a sample.  This helps reduce noise from the ADC.

#define FILTER_SAMPLES     3              // Number of samples to use in moving average of samples.  This is a simple low pass
                                          // filter that helps reduce noise from the bridge & amplifier.

#define SAMPLE_PERIOD_MS   250            // How long to wait between measurements.

// Internal state used by the sketch.
float ZERO_OFFSET;
float GRAMS_PER_MEASUREMENT; 
float filterSamples[FILTER_SAMPLES];
int filterIndex = 0;

void setup(void) 
{
  Serial.begin(9600);

  analogReference(EXTERNAL);
  
  // Initialize all samples to 0.
  for (int i = 0; i < FILTER_SAMPLES; ++i) {
    filterSamples[i] = 0;
  }
 
  // Run through calibration process.
  Serial.println("Scale Calibration Sketch");
  Serial.println();
  Serial.println("Schreib OK und drücke Enter zum Starten.");
  while (!Serial.find("OK"));
  Serial.println();
  Serial.println("Entferne alles von der Waage, tippe OK und drücke Enter.");
  while (!Serial.find("OK"));
  ZERO_OFFSET = readADC(ADC_PIN);
  Serial.println();
  Serial.println("Lege etwas auf die Waage und gebe das Gewicht in Gramm ein. Dann drücke Enter.");
  float grams = Serial.parseFloat();
  while (grams <= 0) {
    grams = Serial.parseFloat();
  }
  float measure = readADC(ADC_PIN) - ZERO_OFFSET;
  GRAMS_PER_MEASUREMENT = grams / measure;
  Serial.println();
  Serial.println("Kalibrierung beendet!  Notiere folgende Kalibrierungswerte:");
  Serial.print("ZERO_OFFSET = ");
  Serial.println(ZERO_OFFSET, 5);
  Serial.print("GRAMS_PER_MEASUREMENT = ");
  Serial.println(GRAMS_PER_MEASUREMENT, 5);
  Serial.println();
  Serial.println("Type OK and press enter to see measured weight values printed periodically.");
  while (!Serial.find("OK"));
}

// Query the analog pin for a value.
// Takes a number of ADC readings and averages them to reduce noise.
float readADC(int pin) {
  float total = 0.0;
  for (int i = 0; i < ADC_SAMPLES; ++i) {
    total += analogRead(pin);
  }
  return total / (float) ADC_SAMPLES;  
}

// Apply simple low pass filter by computing a moving average of samples.
float filterSample(float newSample) {
  // Store the sample in the current index and increment the index appropriately.
  filterSamples[filterIndex] = newSample;
  filterIndex += 1;
  filterIndex = filterIndex < FILTER_SAMPLES ? filterIndex : 0;
  // Sum all the sample values.
  float total = 0;
  for (int i = 0; i < FILTER_SAMPLES; ++i) {
     total += filterSamples[i];
  }
  // Return the average value of the sample values.
  return total / (float) FILTER_SAMPLES;
}

void loop(void) 
{
  // Take a reading and filter it to generate a sample.
  float adc = readADC(ADC_PIN);
  float sample = filterSample(adc) - ZERO_OFFSET;
  // Clamp sample value to 0.
  sample = sample < 0.0 ? 0.0 : sample;
  // Compute the weight and print it out.
  Serial.print("Gewicht (Gramm): ");
  Serial.println(sample * GRAMS_PER_MEASUREMENT, 1);
  // Wait until it's time for the next sample.
  delay(SAMPLE_PERIOD_MS);
}
