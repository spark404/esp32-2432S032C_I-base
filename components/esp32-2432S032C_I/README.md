ESP32 2432S032C_I
===

This is a board support package for the 2432S032C_I board

Board components
---

 * ESP32-WROOM-32 (520KB SRAM, 448KB ROM, 4MB Flash)
 * TFT panel ST7789 240px*320px, 16 BIT RGB 65K
   * IO15 - CS
   * IO14 - SCK
   * IO2  - RS
   * IO13 - SDI
   * IO12 - TFT RST
   * IO27 - LED (Backlight)
 * Capacitive touch  - GT911 chip
   * NC   - CTP_INT // Not Connected, bridge R25 to IO21
   * IO25 - CTP_RST
   * IO32 - CTP_SCL
   * IO33 - CTP_SDA
 * Amplifier - MD8002A
   * IO26 - AUDIO IN - (unbalanced)
 * RGB Led
   * IO4   - LED_K_R
   * IO17  - LED_K_G
   * IO16  - LED_K_B
 * Light Sensor
   * ??
 * Extension connector A
   * ??
 * Extension connector B
   * ??
