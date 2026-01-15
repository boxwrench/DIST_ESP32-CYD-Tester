#ifndef CYD_2432S028R_H
#define CYD_2432S028R_H

// Default Hardware Configuration for Cheap Yellow Display (ESP32-2432S028R)

// Touch Screen Calibration Data (Standard Resistive Layer)
// These are typical values for the XPT2046 on this board.
// If your touch is inverted or inaccurate, you may need to re-run
// the manual calibration by commenting out this block or editing values.
#define TOUCH_MIN_X 300
#define TOUCH_MAX_X 3800
#define TOUCH_MIN_Y 200
#define TOUCH_MAX_Y 3750

#endif // CYD_2432S028R_H
