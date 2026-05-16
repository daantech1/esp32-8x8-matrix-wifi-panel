# esp32-8x8-matrix-wifi-panel
a wifi based led matrix panel powered by esp32
these codes and information about it has been made by Tech Content Creator daan_tech1
                                                                                   
```text
       /$$                                       /$$                         /$$      
      | $$                                      | $$                        | $$      
  /$$$$$$$  /$$$$$$   /$$$$$$  /$$$$$$$        /$$$$$$    /$$$$$$   /$$$$$$$| $$$$$$$ 
 /$$__  $$ |____  $$ |____  $$| $$__  $$      |_  $$_/   /$$__  $$ /$$_____/| $$__  $$
| $$  | $$  /$$$$$$$  /$$$$$$$| $$  \ $$        | $$    | $$$$$$$$| $$      | $$  \ $$
| $$  | $$ /$$__  $$ /$$__  $$| $$  | $$        | $$ /$$| $$_____/| $$      | $$  | $$
|  $$$$$$$|  $$$$$$$|  $$$$$$$| $$  | $$        |  $$$$/|  $$$$$$$|  $$$$$$$| $$  | $$
 \_______/ \_______/ \_______/|__/  |__/         \___/   \_______/ \_______/|__/  |__/
```                                                                                      
                                                                                      

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
these matrixes need to be connected seperately


# without WIFI pannel

| ESP32 Pin | RIGHT MATRIX |
|-----------|-------------|
| GPIO23    | DIN         |
| GPIO18    | CLK         |
| GPIO5     | CS          |
| 3V3        | VCC         |
| GND       | GND         |
for the second matrix, connect the upper matrix pins of matrix right to the bottom pins of matrix left

# MATRIX FONT
this can be usefull for creating a new font for the text matrixes
O O O O O O O O
O O O O O O O O
O O O O O O O O
O O O O O O O O
O O O O O O O O
O O O O O O O O
O O O O O O O O
O O O O O O O O
