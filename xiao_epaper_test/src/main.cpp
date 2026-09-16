#include <Arduino.h>
#include <GxEPD2_BW.h>

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

// =====================================================
// Hardware
// Seeed Studio XIAO ESP32-S3 Sense
// Seeed Studio ePaper Driver Board
// GDEY0213B74, 2.13 Zoll, 122 x 250 Pixel, SSD1680
// =====================================================

#define EPD_CS    D1
#define EPD_DC    D3
#define EPD_RST   D0
#define EPD_BUSY  D2

GxEPD2_BW<
    GxEPD2_213_GDEY0213B74,
    GxEPD2_213_GDEY0213B74::HEIGHT
> display(
    GxEPD2_213_GDEY0213B74(
        EPD_CS,
        EPD_DC,
        EPD_RST,
        EPD_BUSY
    )
);

using DrawFunction = void (*)();

// =====================================================
// Hilfsfunktionen
// =====================================================

void drawCenteredText(
    const char* text,
    int16_t baselineY,
    const GFXfont* font,
    uint16_t color = GxEPD_BLACK
)
{
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;

    display.setFont(font);
    display.setTextColor(color);
    display.getTextBounds(text, 0, baselineY, &x1, &y1, &width, &height);

    const int16_t cursorX = (display.width() - static_cast<int16_t>(width)) / 2 - x1;
    display.setCursor(cursorX, baselineY);
    display.print(text);
}

void showScreen(DrawFunction drawFunction, uint32_t pauseMilliseconds)
{
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        drawFunction();
    }
    while (display.nextPage());

    delay(pauseMilliseconds);
}

void drawWifiIcon(int16_t x, int16_t y, uint16_t color)
{
    display.drawCircle(x, y, 2, color);
    display.drawCircle(x, y, 6, color);
    display.drawCircle(x, y, 10, color);

    // Untere Haelfte ausblenden, damit ein WLAN-Symbol entsteht.
    display.fillRect(x - 11, y, 23, 12, GxEPD_BLACK);
    display.fillCircle(x, y, 2, color);
}

void drawBatteryIcon(int16_t x, int16_t y, uint8_t percent, uint16_t color)
{
    percent = constrain(percent, 0, 100);

    display.drawRect(x, y, 22, 10, color);
    display.fillRect(x + 22, y + 3, 2, 4, color);

    const int16_t innerWidth = map(percent, 0, 100, 0, 18);
    display.fillRect(x + 2, y + 2, innerWidth, 6, color);
}

void drawSunIcon(int16_t x, int16_t y)
{
    display.drawCircle(x, y, 8, GxEPD_BLACK);
    display.fillCircle(x, y, 4, GxEPD_BLACK);

    for (int16_t angle = 0; angle < 360; angle += 45)
    {
        const float radians = angle * PI / 180.0F;
        const int16_t x1 = x + static_cast<int16_t>(cos(radians) * 11.0F);
        const int16_t y1 = y + static_cast<int16_t>(sin(radians) * 11.0F);
        const int16_t x2 = x + static_cast<int16_t>(cos(radians) * 15.0F);
        const int16_t y2 = y + static_cast<int16_t>(sin(radians) * 15.0F);
        display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
    }
}

// =====================================================
// Bildschirm 1: Startbild
// =====================================================

void drawSplashScreen()
{
    const int16_t width = display.width();
    const int16_t height = display.height();

    display.drawRoundRect(0, 0, width, height, 8, GxEPD_BLACK);
    display.drawRoundRect(3, 3, width - 6, height - 6, 6, GxEPD_BLACK);

    display.fillRoundRect(12, 11, width - 24, 31, 5, GxEPD_BLACK);
    drawCenteredText("E-PAPER", 34, &FreeMonoBold12pt7b, GxEPD_WHITE);

    drawCenteredText("DISPLAY TEST", 65, &FreeMonoBold9pt7b);
    drawCenteredText("GDEY0213B74", 85, &FreeMono9pt7b);

    display.drawLine(25, 94, width - 25, 94, GxEPD_BLACK);

    display.setFont(nullptr);
    display.setTextSize(1);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(39, 104);
    display.print("XIAO ESP32-S3  |  250x122");
}

// =====================================================
// Bildschirm 2: Grafiktest
// =====================================================

void drawGraphicsTest()
{
    const int16_t width = display.width();

    display.fillRect(0, 0, width, 19, GxEPD_BLACK);
    display.setFont(nullptr);
    display.setTextSize(1);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(8, 6);
    display.print("GRAFIK- UND KONTRASTTEST");

    // Schachbrett
    constexpr int16_t cellSize = 8;
    constexpr int16_t startX = 10;
    constexpr int16_t startY = 31;

    for (int16_t row = 0; row < 5; ++row)
    {
        for (int16_t column = 0; column < 5; ++column)
        {
            if ((row + column) % 2 == 0)
            {
                display.fillRect(
                    startX + column * cellSize,
                    startY + row * cellSize,
                    cellSize,
                    cellSize,
                    GxEPD_BLACK
                );
            }
            else
            {
                display.drawRect(
                    startX + column * cellSize,
                    startY + row * cellSize,
                    cellSize,
                    cellSize,
                    GxEPD_BLACK
                );
            }
        }
    }

    // Kreis und Dreieck
    display.drawCircle(80, 51, 20, GxEPD_BLACK);
    display.drawCircle(80, 51, 14, GxEPD_BLACK);
    display.fillCircle(80, 51, 5, GxEPD_BLACK);

    display.drawTriangle(106, 71, 126, 31, 146, 71, GxEPD_BLACK);
    display.drawTriangle(112, 67, 126, 39, 140, 67, GxEPD_BLACK);

    // Linienstaerken
    display.drawLine(160, 31, 238, 31, GxEPD_BLACK);
    display.drawLine(160, 40, 238, 40, GxEPD_BLACK);
    display.drawLine(160, 41, 238, 41, GxEPD_BLACK);
    display.fillRect(160, 50, 78, 4, GxEPD_BLACK);
    display.fillRect(160, 61, 78, 7, GxEPD_BLACK);

    // Graustufen-Simulation durch Punktmuster
    for (int16_t x = 160; x < 198; x += 2)
    {
        for (int16_t y = 78; y < 104; y += 2)
        {
            display.drawPixel(x, y, GxEPD_BLACK);
        }
    }

    for (int16_t x = 202; x < 240; x += 3)
    {
        for (int16_t y = 78; y < 104; y += 3)
        {
            display.drawPixel(x, y, GxEPD_BLACK);
        }
    }

    display.setFont(nullptr);
    display.setTextSize(1);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(10, 83);
    display.print("PIXEL");
    display.setCursor(62, 83);
    display.print("KREIS");
    display.setCursor(108, 83);
    display.print("FORM");

    display.drawRoundRect(6, 108, width - 12, 10, 4, GxEPD_BLACK);
    display.fillRoundRect(8, 110, width - 70, 6, 2, GxEPD_BLACK);
}

// =====================================================
// Bildschirm 3: Status-Dashboard
// =====================================================

void drawDashboard()
{
    const int16_t width = display.width();

    // Kopfzeile
    display.fillRect(0, 0, width, 20, GxEPD_BLACK);

    display.setFont(nullptr);
    display.setTextSize(1);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(7, 7);
    display.print("EPD STATUS");

    drawWifiIcon(190, 14, GxEPD_WHITE);
    drawBatteryIcon(216, 5, 82, GxEPD_WHITE);

    // Linker Hauptbereich
    display.drawRoundRect(6, 27, 133, 70, 7, GxEPD_BLACK);
    drawSunIcon(27, 49);

    display.setFont(&FreeMonoBold12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(48, 65);
    display.print("23.7");

    display.drawCircle(112, 42, 3, GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(119, 49);
    display.print("C");

    display.setFont(nullptr);
    display.setTextSize(1);
    display.setCursor(18, 84);
    display.print("RAUMTEMPERATUR");

    // Rechte Statusfelder
    display.drawRoundRect(146, 27, 98, 31, 6, GxEPD_BLACK);
    display.setCursor(155, 36);
    display.print("VERBINDUNG");
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(154, 53);
    display.print("ONLINE");

    display.drawRoundRect(146, 65, 98, 32, 6, GxEPD_BLACK);
    display.setFont(nullptr);
    display.setTextSize(1);
    display.setCursor(155, 74);
    display.print("DISPLAY");
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(154, 92);
    display.print("BEREIT");

    // Fusszeile mit Fortschrittsbalken
    display.setFont(nullptr);
    display.setTextSize(1);
    display.setCursor(7, 108);
    display.print("TEST ERFOLGREICH");

    display.drawRoundRect(140, 104, 104, 12, 4, GxEPD_BLACK);
    display.fillRoundRect(143, 107, 96, 6, 2, GxEPD_BLACK);
}

// =====================================================
// Arduino-Programm
// =====================================================

void setup()
{
    Serial.begin(115200);
    delay(1200);

    Serial.println();
    Serial.println("====================================");
    Serial.println("  XIAO ESP32-S3 E-Paper Displaytest");
    Serial.println("====================================");
    Serial.println("Display initialisieren ...");

    // 2 ms Resetimpuls fuer die Seeed-Treiberplatine.
    display.init(115200, true, 2, false);
    display.setRotation(1);  // Querformat: 250 x 122 Pixel

    Serial.print("Breite : ");
    Serial.println(display.width());
    Serial.print("Hoehe  : ");
    Serial.println(display.height());

    Serial.println("1/3 Startbild anzeigen");
    showScreen(drawSplashScreen, 3000);

    Serial.println("2/3 Grafiktest anzeigen");
    showScreen(drawGraphicsTest, 4000);

    Serial.println("3/3 Dashboard anzeigen");
    showScreen(drawDashboard, 1000);

    Serial.println("Displaytest abgeschlossen.");
    Serial.println("Display wird in den Ruhemodus versetzt.");

    // Der Bildinhalt bleibt auf einem E-Paper auch im Ruhemodus sichtbar.
    display.hibernate();
}

void loop()
{
    // Kein wiederholtes Aktualisieren: Das schont das E-Paper und verhindert
    // unnoetige Vollbild-Refreshes. Fuer einen neuen Test RESET druecken.
    delay(1000);
}
