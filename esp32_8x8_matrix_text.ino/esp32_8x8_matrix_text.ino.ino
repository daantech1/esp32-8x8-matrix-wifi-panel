// ======================================
// ESP32 + 2x MAX7219 MATRIX DISPLAY
// BEIDE SCHERMEN 180 GRADEN GEDRAAID
// ======================================

// ====== HIER AANPASSEN ======

String TEXT = "JACK";

// lager = sneller
// hoger = langzamer
int SPEED = 120;

// ======================================

#define DIN 23
#define CLK 18
#define CS  5

uint8_t buffer[16];

// ================= FONT =================

const uint8_t fontLetters[26][5] = {

  {0b0111110,0b0001001,0b0001001,0b0001001,0b0111110}, // A
  {0b0111111,0b0100101,0b0100101,0b0100101,0b0011010}, // B
  {0b0011110,0b0100001,0b0100001,0b0100001,0b0010001}, // C
  {0b0111111,0b0100001,0b0100001,0b0011110,0b0000000}, // D
  {0b0111111,0b0100101,0b0100101,0b0100101,0b0100001}, // E
  {0b0111111,0b0000101,0b0000101,0b0000001,0b0000001}, // F
  {0b0011110,0b0100001,0b0100101,0b0110101,0b0011101}, // G
  {0b0111111,0b0000100,0b0000100,0b0000100,0b0111111}, // H
  {0b0001000,0b0001000,0b0111111,0b0001000,0b0001000}, // I
  {0b0010000,0b0100000,0b0100001,0b0011111,0b0000000}, // J
  {0b0111111,0b0001000,0b0001100,0b0010011,0b0100001}, // K
  {0b0111111,0b0100000,0b0100000,0b0100000,0b0000000}, // L
  {0b0111111,0b0000010,0b0001100,0b0000010,0b0111111}, // M
  {0b0111111,0b0000110,0b0001000,0b0010000,0b0111111}, // N
  {0b0011110,0b0100001,0b0100001,0b0100001,0b0011110}, // O
  {0b0111111,0b0000101,0b0000101,0b0000010,0b0000000}, // P
  {0b0011110,0b0100001,0b0110001,0b0011001,0b0111110}, // Q
  {0b0111111,0b0000101,0b0001101,0b0010010,0b0100000}, // R
  {0b0100010,0b0100101,0b0100101,0b0011001,0b0000000}, // S
  {0b0000001,0b0000001,0b0111111,0b0000001,0b0000001}, // T
  {0b0111111,0b0100000,0b0100000,0b0100000,0b0111111}, // U
  {0b0000111,0b0011000,0b0100000,0b0011000,0b0000111}, // V
  {0b0111111,0b0010000,0b0001100,0b0010000,0b0111111}, // W
  {0b0110011,0b0001100,0b0001100,0b0110011,0b0000000}, // X
  {0b0000011,0b0000100,0b0111000,0b0000100,0b0000011}, // Y
  {0b0110001,0b0101001,0b0100101,0b0100011,0b0000000}  // Z
};

// ================= MAX7219 =================

void sendChain(uint8_t reg2, uint8_t data2,
               uint8_t reg1, uint8_t data1)
{
  digitalWrite(CS, LOW);

  shiftOut(DIN, CLK, MSBFIRST, reg2);
  shiftOut(DIN, CLK, MSBFIRST, data2);

  shiftOut(DIN, CLK, MSBFIRST, reg1);
  shiftOut(DIN, CLK, MSBFIRST, data1);

  digitalWrite(CS, HIGH);
}

void sendAll(uint8_t reg, uint8_t data)
{
  sendChain(reg, data, reg, data);
}

void initMax()
{
  sendAll(0x09, 0x00);
  sendAll(0x0A, 0x08);
  sendAll(0x0B, 0x07);
  sendAll(0x0C, 0x01);
  sendAll(0x0F, 0x00);
}

// ================= DISPLAY =================

void refreshDisplay()
{
  for (int row = 0; row < 8; row++) {

    uint8_t leftData  = 0;
    uint8_t rightData = 0;

    // LINKER DISPLAY 180°
    for (int bit = 0; bit < 8; bit++) {
      if (buffer[row + 8] & (1 << bit)) {
        leftData |= (1 << (7 - bit));
      }
    }

    // RECHTER DISPLAY 180°
    for (int bit = 0; bit < 8; bit++) {
      if (buffer[row] & (1 << bit)) {
        rightData |= (1 << (7 - bit));
      }
    }

    sendChain(
      8 - row, leftData,
      8 - row, rightData
    );
  }
}

void shiftDisplay(uint8_t newCol)
{
  for (int i = 15; i > 0; i--) {
    buffer[i] = buffer[i - 1];
  }

  buffer[0] = newCol;

  refreshDisplay();
}

// ================= HELPERS =================

uint8_t flipVertical(uint8_t b)
{
  uint8_t r = 0;

  for (int i = 0; i < 7; i++) {

    if (b & (1 << i)) {
      r |= (1 << (6 - i));
    }
  }

  return r;
}

void getGlyph(char c, uint8_t out[5])
{
  if (c >= 'A' && c <= 'Z') {

    for (int i = 0; i < 5; i++) {
      out[i] = fontLetters[c - 'A'][i];
    }

    return;
  }

  if (c >= 'a' && c <= 'z') {

    for (int i = 0; i < 5; i++) {
      out[i] = fontLetters[c - 'a'][i];
    }

    return;
  }

  for (int i = 0; i < 5; i++) {
    out[i] = 0x00;
  }
}

// ================= SCROLL =================

void scrollText(String text)
{
  for (int t = 0; t < text.length(); t++) {

    uint8_t glyph[5];

    getGlyph(text[t], glyph);

    for (int col = 0; col < 5; col++) {

      uint8_t colBits = flipVertical(glyph[col]);

      shiftDisplay(colBits);

      delay(SPEED);
    }

    shiftDisplay(0x00);

    delay(SPEED);
  }
}

void clearScrollOut()
{
  for (int i = 0; i < 16; i++) {

    shiftDisplay(0x00);

    delay(SPEED);
  }
}

// ================= SETUP =================

void setup()
{
  pinMode(DIN, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(CS, OUTPUT);

  digitalWrite(CS, HIGH);

  initMax();

  for (int i = 0; i < 16; i++) {
    buffer[i] = 0;
  }

  refreshDisplay();
}

// ================= LOOP =================

void loop()
{
  scrollText(TEXT);
  clearScrollOut();
}