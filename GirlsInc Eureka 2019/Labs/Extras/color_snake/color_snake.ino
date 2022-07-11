/* 
 *  An advanced example of what can be done with the color strip, which may be hard to code in the ArduBlock interface,
 *  This is essentially a demo and not a task students would complete.
 *  However, this code could be a useful reference for anyone wanting to code directly in C.
 */

#include <Adafruit_NeoPixel.h>
#include <math.h>

/*
 * These are user-controlled variables.
 */
const unsigned int N_LED = 10;  // Number of LEDs on the strip
const unsigned int LED_PIN = 2;  // The pin the LED strip is plugged in to
const unsigned int LED_BRIGHTNESS = 200;  // 0-255; Brightness of the LEDs
const unsigned int COLOR_REPEAT_MS = 5 * 1000;  // How long (ms) should it take to cycle through the entire color map?
const unsigned int N_COLORS = 128;  // What is the resolution (number of samples) to reprsent in the color map?
/*
 * End of user-controlled variables. (Except for potentially the color map function).
 */

// Calculate how long each update cycle (i.e., the loop function) should take
const unsigned long CYCLE_TIME_MS = (unsigned long)(COLOR_REPEAT_MS / N_COLORS);
Adafruit_NeoPixel led_strip = Adafruit_NeoPixel(N_LED, LED_PIN, NEO_RGB + NEO_KHZ800);

struct COLOR {
  byte red;
  byte green;
  byte blue;
};

/*
 * This function is a silent error check used by the color map functions. 
 * Inputs less than 0 will become 0 and inputs greater than 1 will become 1.
 */
inline float correct_colormap_idx(float idx) {
  if (idx < 0.0) {
    idx = 0.0;
  } else if (idx > 1.0) {
    idx = 1.0;
  }
  return idx;
}

/*
 * This color map is just different shades of blue.
 * Accepts a color map index from 0-1 and returns the correspond color.
 */
COLOR colormap_blue(float idx) {
  idx = correct_colormap_idx(idx);
  // Just return a blue values in sequence
  return (COLOR){0, 0, (byte)round(idx * 255)};
}

/*
 * Convert a color in HSV format to RGB format.
 * Hue is from 0-360, saturation and value are from 0-1.
 * Adapted from: https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
 */
inline COLOR hsv2rgb(float h, float s, float v) {
  float p, q, t, ff;
  int i;
  COLOR out;

  if (h >= 360) {
    h = 0;
  }
  h /= 60.0;
  i = (float)h;
  ff = h - i;
  p = v * (1.0 - s);
  q = v * (1.0 - (s * ff));
  t = v * (1.0 - (s * (1.0 - ff)));

  switch(i) {
    case 0:
        out.red = (byte)(255 * v);
        out.green = (byte)(255 * t);
        out.blue = (byte)(255 * p);
        break;
    case 1:
        out.red = (byte)(255 * q);
        out.green = (byte)(255 * v);
        out.blue = (byte)(255 * p);
        break;
    case 2:
        out.red = (byte)(255 * p);
        out.green = (byte)(255 * v);
        out.blue = (byte)(255 * t);
        break;
  
    case 3:
        out.red = (byte)(255 * p);
        out.green = (byte)(255 * q);
        out.blue = (byte)(255 * v);
        break;
    case 4:
        out.red = (byte)(255 * t);
        out.green = (byte)(255 * p);
        out.blue = (byte)(255 * v);
        break;
    case 5:
    default:
        out.red = (byte)(255 * v);
        out.green = (byte)(255 * p);
        out.blue = (byte)(255 * q);
        break;
  }
  
  return out;     
}

/*
 * This color map cycles through different hues in the HSV color mapping.
 * Accepts a color map index from 0-1 and returns the correspond color.
 */
COLOR colormap_hsv(float idx) {
  float hue = correct_colormap_idx(idx);
  return hsv2rgb(hue * 360, 1.0, 1.0);
}

/*
 * Use the following function pointer to select a color map function
 */
COLOR (*colormap)(float) = colormap_hsv;

COLOR led_colors[N_LED];  // To keep rack of which color each LED should be
float color_idx = 0.0;  // Where in the color map are we?
const float COLOR_IDX_STEP = 1.0 / (float)N_COLORS;  // How far to advance in the color map every update cycle.

/*
 * Initial setup.
 * Initialize the color strip, set the brightness, and set the initial LED values.
 */
void setup() {
  led_strip.begin();
  led_strip.setBrightness(LED_BRIGHTNESS);
  led_strip.show();

  // Each LED should be one step along the color map
  // Colors should advance along the strip, so update backwards.
  for (int i = N_LED - 1; i >= 0; i--) {
    led_colors[i] = colormap(color_idx);
    color_idx += COLOR_IDX_STEP;
  }
}

/*
 * The upate cycle
 * Advance colors along the strip, and add a new color to the beginning from the color map.
 * Each cycle should take CYCLE_TIME_MS milliseconds. Timing logic tries to ensure evenly spaced timing.
 */
void loop() {
  unsigned long cycle_start_time = millis();  // Keep track of when this cycle began for later

  // Get the next color in the color map and advance our position for the next cycle.
  COLOR next_color = colormap(color_idx);
  color_idx += COLOR_IDX_STEP;
  // If the color index ever gets larger than 1, throw away the integer part to get it back in the range of 0-1.
  if (color_idx > 1.0) {
    double throwaway;
    color_idx = modf(color_idx, &throwaway);
  }

  // Advance every color one position along the strip.
  for (int i = N_LED - 1; i > 0; i--) {
    led_colors[i] = led_colors[i-1];
  }
  led_colors[0] = next_color;  // Add the new color to the beginning of the strip.

  // Update the LED strip with what the new colors should be.
  for (int i = 0; i < N_LED; i++) {
    COLOR c = led_colors[i];
    led_strip.setPixelColor(i, c.red, c.green, c.blue);
  }
  led_strip.show();

  // If this cycle took longer than the cycle time, then just continue.
  // Otherwise, wait long enough so that the update cycles are consistently spaced at the proper interval.
  unsigned long cycle_time = millis() - cycle_start_time;  // How long did this cycle take?
  if (cycle_time < CYCLE_TIME_MS) {
    delay(CYCLE_TIME_MS - cycle_time);
  }
}
