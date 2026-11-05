// ShrikeTFT
// Driving a 2.4" ST7789 (240x320) SPI display from the ESP32-S3 on a
// Vicharak Shrike-Fi board. Pins and driver settings are in platformio.ini.
//
// On boot it runs a quick colour test, shows a splash screen with the
// wiring, and then bounces a box around with an FPS counter on top.

#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft;

const int BAR_HEIGHT = 30;   // status bar at the top
const int BOX_SIZE   = 30;

int boxX = 40, boxY = 60;
int speedX = 3, speedY = 2;

unsigned long frameCount = 0;
unsigned long lastFpsTime = 0;

// Fill the screen with a few solid colours. If these come out wrong,
// check TFT_INVERSION_* and TFT_RGB_ORDER in platformio.ini.
void colourTest() {
  struct { uint16_t colour; const char *name; } steps[] = {
    {TFT_RED, "red"}, {TFT_GREEN, "green"}, {TFT_BLUE, "blue"},
    {TFT_WHITE, "white"}, {TFT_BLACK, "black"},
  };

  for (auto &step : steps) {
    Serial.printf("colour test: %s\n", step.name);
    tft.fillScreen(step.colour);
    delay(300);
  }
}

void showSplash() {
  int centreX = tft.width() / 2;

  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Shrike-Fi + ST7789", centreX, 40, 4);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(String(tft.width()) + " x " + String(tft.height()), centreX, 75, 4);

  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("SCK  -> IO36   MOSI -> IO35", 20, 105, 2);
  tft.drawString("MISO -> IO37   CS   -> IO34", 20, 127, 2);
  tft.drawString("DC   -> IO33   RST  -> IO38", 20, 149, 2);
  tft.drawString("LED  -> IO18   VCC  -> 3.3V", 20, 171, 2);

  delay(2500);
}

void drawStatusBar(float fps) {
  tft.fillRect(0, 0, tft.width(), BAR_HEIGHT, TFT_NAVY);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);

  tft.setTextDatum(ML_DATUM);
  tft.drawString("Shrike-Fi TFT", 6, BAR_HEIGHT / 2, 2);

  tft.setTextDatum(MR_DATUM);
  tft.drawString(String(fps, 1) + " fps", tft.width() - 6, BAR_HEIGHT / 2, 2);
}

void moveBox() {
  // Rub out the old box, move it, bounce off the edges
  tft.fillRect(boxX, boxY, BOX_SIZE, BOX_SIZE, TFT_BLACK);

  boxX += speedX;
  boxY += speedY;

  if (boxX <= 0 || boxX + BOX_SIZE >= tft.width()) {
    speedX = -speedX;
    boxX += speedX;
  }
  if (boxY <= BAR_HEIGHT || boxY + BOX_SIZE >= tft.height()) {
    speedY = -speedY;
    boxY += speedY;
  }

  // Colour shifts with position, just so it's nicer to look at
  uint16_t colour = tft.color565(boxX * 255 / tft.width(), 128, boxY * 255 / tft.height());
  tft.fillRect(boxX, boxY, BOX_SIZE, BOX_SIZE, colour);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nShrikeTFT starting");

  tft.init();           // also switches the backlight on
  tft.setRotation(1);   // landscape, 320x240
  Serial.printf("display ready: %d x %d\n", tft.width(), tft.height());

  colourTest();
  showSplash();

  tft.fillScreen(TFT_BLACK);
  drawStatusBar(0);
  lastFpsTime = millis();
}

void loop() {
  moveBox();
  frameCount++;

  // Update the FPS reading once a second
  unsigned long now = millis();
  if (now - lastFpsTime >= 1000) {
    float fps = frameCount * 1000.0f / (now - lastFpsTime);
    drawStatusBar(fps);
    Serial.printf("%.1f fps\n", fps);

    frameCount = 0;
    lastFpsTime = now;
  }

  delay(10);
}
