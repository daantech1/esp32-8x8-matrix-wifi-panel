# esp32-8x8-matrix-wifi-panel
a wifi based led matrix panel powered by esp32

connections

# with WIFI panel

| ESP32 Pin | LEFT MATRIX |
|-----------|-------------|
| GPIO23    | DIN         |
| GPIO18    | CLK         |
| GPIO5     | CS          |
| 3V3        | VCC         |
| GND       | GND         |

| ESP32 Pin | RIGHT MATRIX |
|-----------|-------------|
| GPIO23    | DIN         |
| GPIO18    | CLK         |
| GPIO4     | CS          |
| 3V3        | VCC         |
| GND       | GND         |



# without WIFI pannel

| ESP32 Pin | RIGHT MATRIX |
|-----------|-------------|
| GPIO23    | DIN         |
| GPIO18    | CLK         |
| GPIO5     | CS          |
| 3V3        | VCC         |
| GND       | GND         |

| ESP32 Pin | RIGHT MATRIX |
|-----------|-------------|
| GPIO23    | DIN         |
| GPIO18    | CLK         |
| GPIO5     | CS          |
| 3V3        | VCC         |
| GND       | GND         |
for the second matrix, connect the upper matrix pins of matrix right to the bottom pins of matrix left
