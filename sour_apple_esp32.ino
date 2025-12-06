#include <WiFi.h>
#include <WebServer.h>
#include <time.h>
#include <Preferences.h>

#define DIN 23
#define CLK 18
#define CS1 5   // linker matrix
#define CS2 4   // rechter matrix

// AP instellingen
const char* AP_SSID = "matrix clock";
const char* AP_PASS = "admin1234567890";
IPAddress apIP(10,77,55,1);
IPAddress apGW(10,77,55,1);
IPAddress apMASK(255,255,255,0);

// WiFi / tijd state
bool wifiConnected = false;
bool timeSynced    = false;

// Webserver
WebServer server(80);
Preferences prefs;

// 16 kolommen buffer (2 matrices naast elkaar)
uint8_t buffer[16];

// Display mode: 0 = tijd, 1 = datum, 2 = tekst
uint8_t currentMode = 0;      // standaard tijd
String  customText  = " HALLO "; 
uint8_t speedLevel  = 5;      // 1..10

// -------------------- VERTICAAL SPIEGELEN --------------------
uint8_t flipVertical(uint8_t b) {
  uint8_t r = 0;
  for (int i = 0; i < 7; i++)
    if (b & (1 << i))
      r |= (1 << (6 - i));
  return r;
}

// -------------------- LETTER-FONT (A-Z) ------------------
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

// -------------------- CIJFER-FONT (0-9) ------------------
const uint8_t fontDigits[10][5] = {
  {0b0111110,0b1010001,0b1001001,0b1000101,0b0111110}, // 0
  {0b0000000,0b1000010,0b1111111,0b1000000,0b0000000}, // 1
  {0b1000010,0b1100001,0b1010001,0b1001001,0b1000110}, // 2
  {0b1000001,0b1001001,0b1001001,0b1001001,0b0110110}, // 3
  {0b0011000,0b0010100,0b0010010,0b1111111,0b0010000}, // 4
  {0b0100111,0b1000101,0b1000101,0b1000101,0b0111001}, // 5
  {0b0111100,0b1001010,0b1001001,0b1001001,0b0110000}, // 6
  {0b0000001,0b1110001,0b0001001,0b0000101,0b0000011}, // 7
  {0b0110110,0b1001001,0b1001001,0b1001001,0b0110110}, // 8
  {0b0000110,0b1001001,0b1001001,0b0101001,0b0011110}  // 9
};

// : en -
const uint8_t glyphColon[5] = {0b0000000,0b0000000,0b0110110,0b0000000,0b0000000};
const uint8_t glyphDash[5]  = {0b0001000,0b0001000,0b0001000,0b0001000,0b0001000};

// ---------------- SEND FUNCTION ----------------
void sendMax(int matrix, uint8_t reg, uint8_t data)
{
  digitalWrite(matrix == 0 ? CS1 : CS2, LOW);
  shiftOut(DIN, CLK, MSBFIRST, reg);
  shiftOut(DIN, CLK, MSBFIRST, data);
  digitalWrite(matrix == 0 ? CS1 : CS2, HIGH);
}

void initMax()
{
  for (int m = 0; m < 2; m++) {
    sendMax(m,0x09,0x00);
    sendMax(m,0x0A,0x08);
    sendMax(m,0x0B,0x07);
    sendMax(m,0x0C,0x01);
    sendMax(m,0x0F,0x00);
  }
}

void clearMatrix()
{
  for (int r = 0; r < 8; r++) {
    buffer[r]   = 0;
    buffer[r+8] = 0;
    sendMax(0, r+1, 0);
    sendMax(1, r+1, 0);
  }
}

// ---------------- SMOOTH SCROLL ----------------
void scrollUp_SMOOTH(uint8_t newCol)
{
  for (int r = 15; r > 0; r--)
    buffer[r] = buffer[r-1];

  buffer[0] = newCol;

  for (int r = 0; r < 8; r++) {
    // let op: omgewisseld voor jouw fysieke volgorde
    sendMax(0, r+1, buffer[r+8]);  // linker matrix
    sendMax(1, r+1, buffer[r]);    // rechter matrix
  }
}

// ------------ GLYPH PER KARAKTER -------------
void getGlyph(char c, uint8_t out[5])
{
  if (c >= 'A' && c <= 'Z') {
    for (int i=0;i<5;i++) out[i] = fontLetters[c-'A'][i];
    return;
  }
  if (c >= 'a' && c <= 'z') {
    for (int i=0;i<5;i++) out[i] = fontLetters[c-'a'][i];
    return;
  }
  if (c >= '0' && c <= '9') {
    for (int i=0;i<5;i++) out[i] = fontDigits[c-'0'][i];
    return;
  }
  if (c == ':') {
    for (int i=0;i<5;i++) out[i] = glyphColon[i];
    return;
  }
  if (c == '-') {
    for (int i=0;i<5;i++) out[i] = glyphDash[i];
    return;
  }
  for (int i=0;i<5;i++) out[i] = 0x00;
}

// ------------ SPEED MAPPING -------------
int getDelayMs() {
  uint8_t lvl = constrain(speedLevel, 1, 10);
  // 1 = langzaam (~220 ms), 10 = snel (~40 ms)
  return 220 - (int)lvl * 18;
}

// ------------ TEKST SCROLLEN -------------
void scrollText(const String &text)
{
  int d = getDelayMs();
  for (int t=0; t<text.length(); t++)
  {
    char c = text[t];
    uint8_t glyph[5];
    getGlyph(c, glyph);

    for (int col=0; col<5; col++) {
      uint8_t colBits = flipVertical(glyph[col]);
      scrollUp_SMOOTH(colBits);
      delay(d);
    }
    scrollUp_SMOOTH(0x00);
    delay(d);
  }
}

// ---------------- WEBPAGINA ----------------
String htmlPage()
{
  String page =
    "<html><head><meta charset='utf-8'>"
    "<title>Matrix Clock</title></head><body>"
    "<h2>Matrix Clock Control Panel</h2>";

  page += "<p><b>WiFi status:</b> ";
  page += (wifiConnected ? "CONNECTED" : "NOT CONNECTED");
  page += timeSynced ? " (time OK)" : " (no time)";
  page += "</p>";

  // ---- WiFi selectie ----
  page += "<h3>WiFi verbinden</h3>";
  int n = WiFi.scanNetworks();
  page += "<form action='/connect' method='post'>";
  page += "SSID:<br><select name='ssid'>";
  for (int i = 0; i < n; i++) {
    page += "<option value='";
    page += WiFi.SSID(i);
    page += "'>";
    page += WiFi.SSID(i);
    page += " (";
    page += WiFi.RSSI(i);
    page += " dBm)";
    page += "</option>";
  }
  page += "</select><br><br>";
  page += "Wachtwoord:<br><input type='password' name='pass'><br><br>";
  page += "<input type='submit' value='Connect'>";
  page += "</form><hr>";

  // ---- Matrix weergave instellingen ----
  page += "<h3>Matrix weergave</h3>";
  page += "<form action='/display' method='post'>";

  // Mode radio buttons
  page += "<p><b>Wat wil je laten zien?</b><br>";
  page += "<input type='radio' name='mode' value='time'";
  if (currentMode == 0) page += " checked";
  page += "> Tijd (HH:MM)<br>";

  page += "<input type='radio' name='mode' value='date'";
  if (currentMode == 1) page += " checked";
  page += "> Datum (DD-MM-JJJJ)<br>";

  page += "<input type='radio' name='mode' value='text'";
  if (currentMode == 2) page += " checked";
  page += "> Tekst: <input type='text' name='text' value='";
  page += customText;
  page += "' size='20'><br></p>";

  // Speed slider
  page += "<p><b>Scroll snelheid:</b><br>";
  page += "<input type='range' name='speed' min='1' max='10' value='";
  page += String(speedLevel);
  page += "' oninput='sp.value=this.value'>";
  page += " <output id='sp'>";
  page += String(speedLevel);
  page += "</output> (1=langzaam, 10=snel)</p>";

  page += "<input type='submit' value='Opslaan & toepassen'>";
  page += "</form>";

  page += "<hr><p>AP SSID: matrix clock<br>AP IP: 10.77.55.1</p>";
  page += "</body></html>";

  return page;
}

void handleRoot()
{
  server.send(200,"text/html", htmlPage());
}

// ---- WiFi connect handler ----
void handleConnect()
{
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");

  Serial.println("[CONNECT] SSID: " + ssid);
  Serial.println("[CONNECT] PASS: " + pass);

  if (ssid.isEmpty()) {
    server.send(200,"text/html","<html><body>Geen SSID gekozen.<br><a href='/'>Terug</a></body></html>");
    return;
  }

  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("[WiFi] Verbinden...");

  unsigned long start = millis();
  wifiConnected = false;
  while (millis() - start < 15000) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      break;
    }
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (!wifiConnected) {
    Serial.println("[WiFi] VERBINDING MISLUKT");
    server.send(200,"text/html","<html><body>Verbinding mislukt.<br><a href='/'>Terug</a></body></html>");
    return;
  }

  Serial.println("[WiFi] VERBONDEN!");
  Serial.print("STA IP: "); Serial.println(WiFi.localIP());

  // NTP instellen (Europa/Amsterdam: UTC+1, DST+1)
  configTime(3600, 3600, "pool.ntp.org", "time.nist.gov", "time.google.com");
  Serial.println("[NTP] Tijd synchroniseren...");

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    timeSynced = true;
    Serial.println("[NTP] Tijd OK!");
    Serial.printf("Nu: %02d-%02d-%04d %02d:%02d:%02d\n",
        timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year+1900,
        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    server.send(200,"text/html","<html><body>Verbonden en tijd gesynchroniseerd!<br><a href='/'>Terug</a></body></html>");
  } else {
    timeSynced = false;
    Serial.println("[NTP] Tijd NIET verkregen.");
    server.send(200,"text/html","<html><body>Verbonden met WiFi, maar geen tijd van NTP.<br><a href='/'>Terug</a></body></html>");
  }
}

// ---- Display settings handler ----
void handleDisplay()
{
  String mode = server.arg("mode");
  String txt  = server.arg("text");
  String sp   = server.arg("speed");

  Serial.println("[DISPLAY] mode=" + mode + " text=" + txt + " speed=" + sp);

  if (mode == "time") currentMode = 0;
  else if (mode == "date") currentMode = 1;
  else if (mode == "text") currentMode = 2;

  if (!txt.isEmpty()) {
    customText = " " + txt + " ";
  }

  int sVal = sp.toInt();
  if (sVal < 1) sVal = 1;
  if (sVal > 10) sVal = 10;
  speedLevel = (uint8_t)sVal;

  // instellingen opslaan in NVS
  prefs.putUChar("mode",  currentMode);
  prefs.putUChar("speed", speedLevel);
  prefs.putString("text", customText);

  server.send(200,"text/html","<html><body>Instellingen opgeslagen.<br><a href='/'>Terug</a></body></html>");
}

// ---------------- SETUP ----------------
void setup(){
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[BOOT] Matrix Clock start...");

  pinMode(DIN,OUTPUT);
  pinMode(CLK,OUTPUT);
  pinMode(CS1,OUTPUT);
  pinMode(CS2,OUTPUT);

  initMax();
  clearMatrix();

  prefs.begin("matrixcfg", false);
  currentMode = prefs.getUChar("mode", 0);
  speedLevel  = prefs.getUChar("speed", 5);
  customText  = prefs.getString("text", " HALLO ");

  Serial.print("[CFG] mode=");  Serial.println(currentMode);
  Serial.print("[CFG] speed="); Serial.println(speedLevel);
  Serial.print("[CFG] text=");  Serial.println(customText);

  WiFi.mode(WIFI_AP_STA);

  if (!WiFi.softAPConfig(apIP, apGW, apMASK)) {
    Serial.println("[AP] softAPConfig FAILED!");
  }

  if (WiFi.softAP(AP_SSID, AP_PASS)) {
    Serial.println("[AP] Gestart!");
    Serial.print("[AP] SSID: "); Serial.println(AP_SSID);
    Serial.print("[AP] PASS: "); Serial.println(AP_PASS);
    Serial.print("[AP] IP: ");   Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("[AP] Start mislukt!");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/connect", HTTP_POST, handleConnect);
  server.on("/display", HTTP_POST, handleDisplay);
  server.begin();
  Serial.println("[HTTP] Webserver gestart op http://10.77.55.1");

  String ipText = " AP 10.77.55.1 ";
  scrollText(ipText);
}

// ---------------- LOOP ----------------
void loop(){
  server.handleClient();

  if (wifiConnected && timeSynced) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char buf[32];

      if (currentMode == 0) {
        // tijd
        strftime(buf, sizeof(buf), "%H:%M", &timeinfo);
        String msg = " ";
        msg += buf;
        msg += " ";
        Serial.print("[TIME] "); Serial.println(buf);
        scrollText(msg);
      } else if (currentMode == 1) {
        // datum
        strftime(buf, sizeof(buf), "%d-%m-%Y", &timeinfo);
        String msg = " ";
        msg += buf;
        msg += " ";
        Serial.print("[DATE] "); Serial.println(buf);
        scrollText(msg);
      } else {
        // custom tekst
        Serial.print("[TEXT] "); Serial.println(customText);
        scrollText(customText);
      }
    } else {
      scrollText(" NO TIME ");
    }
  } else {
    scrollText(" SET WIFI VIA 10.77.55.1 ");
  }
}
