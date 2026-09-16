#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <cstring>

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

    // Untere Haelfte ausblenden, damit ein WLAN-Symbol entsteht. y ist so
    // gewaehlt (siehe Aufrufer), dass der Punkt (= unterer Rand des Symbols)
    // mittig in der Kopfzeile liegt und die Maske nicht darunter ins Weisse
    // hineinragt.
    display.fillRect(x - 11, y, 22, 10, GxEPD_BLACK);
    display.fillCircle(x, y, 2, color);
}

void drawGearIcon(int16_t x, int16_t y, uint16_t color)
{
    constexpr int16_t bodyRadius = 5;
    constexpr int16_t toothLength = 3;

    display.drawCircle(x, y, bodyRadius, color);
    display.fillCircle(x, y, 1, color);

    for (int16_t angle = 0; angle < 360; angle += 45)
    {
        const float radians = angle * PI / 180.0F;
        const int16_t x1 = x + static_cast<int16_t>(cos(radians) * (bodyRadius - 1));
        const int16_t y1 = y + static_cast<int16_t>(sin(radians) * (bodyRadius - 1));
        const int16_t x2 = x + static_cast<int16_t>(cos(radians) * (bodyRadius + toothLength));
        const int16_t y2 = y + static_cast<int16_t>(sin(radians) * (bodyRadius + toothLength));
        display.drawLine(x1, y1, x2, y2, color);
    }
}

void drawBatteryIcon(int16_t x, int16_t y, uint8_t percent, uint16_t color)
{
    percent = constrain(percent, 0, 100);

    display.drawRect(x, y, 22, 10, color);
    display.fillRect(x + 22, y + 3, 2, 4, color);

    const int16_t innerWidth = map(percent, 0, 100, 0, 18);
    display.fillRect(x + 2, y + 2, innerWidth, 6, color);
}

void drawSunIcon(int16_t x, int16_t y, int16_t radius)
{
    const int16_t rayInner = radius + 3;
    const int16_t rayOuter = radius + 7;

    display.drawCircle(x, y, radius, GxEPD_BLACK);
    display.fillCircle(x, y, radius / 2, GxEPD_BLACK);

    for (int16_t angle = 0; angle < 360; angle += 45)
    {
        const float radians = angle * PI / 180.0F;
        const int16_t x1 = x + static_cast<int16_t>(cos(radians) * rayInner);
        const int16_t y1 = y + static_cast<int16_t>(sin(radians) * rayInner);
        const int16_t x2 = x + static_cast<int16_t>(cos(radians) * rayOuter);
        const int16_t y2 = y + static_cast<int16_t>(sin(radians) * rayOuter);
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

    // Kopfbalken: groesser und breiter gezogen als die restlichen Zeilen.
    display.fillRoundRect(8, 11, width - 16, 38, 5, GxEPD_BLACK);
    drawCenteredText("E-PAPER", 40, &FreeMonoBold18pt7b, GxEPD_WHITE);

    drawCenteredText("DISPLAY TEST", 70, &FreeMonoBold12pt7b);
    drawCenteredText("PANEL: GDEY0213B74", 88, &FreeMonoBold9pt7b);

    display.drawLine(25, 96, width - 25, 96, GxEPD_BLACK);

    // Letzte Zeile von Hand gesetzt (statt drawCenteredText), damit der
    // Abstand links/rechts vom "|" unabhaengig von der Zeichenbreite der
    // Schrift frei einstellbar ist.
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);

    constexpr int16_t charWidth = 11; // xAdvance von FreeMonoBold9pt7b
    constexpr int16_t pipeGap = 6;
    const char* leftPart = "XIAO ESP32-S3";
    const char* rightPart = "250x122";
    const int16_t leftWidth = static_cast<int16_t>(strlen(leftPart)) * charWidth;
    const int16_t rightWidth = static_cast<int16_t>(strlen(rightPart)) * charWidth;
    const int16_t totalWidth = leftWidth + pipeGap + charWidth + pipeGap + rightWidth;
    const int16_t startX = (width - totalWidth) / 2;
    constexpr int16_t bottomBaseline = 112;

    display.setCursor(startX, bottomBaseline);
    display.print(leftPart);
    display.setCursor(startX + leftWidth + pipeGap, bottomBaseline);
    display.print("|");
    display.setCursor(startX + leftWidth + pipeGap + charWidth + pipeGap, bottomBaseline);
    display.print(rightPart);
}

// =====================================================
// Bildschirm 2: Grafiktest
// =====================================================

constexpr uint8_t GRAPHICS_ANIMATION_STEPS = 10;

// step laeuft von 0 (leer/Startzustand) bis GRAPHICS_ANIMATION_STEPS (fertiges
// Testbild). Wird von playGraphicsTestAnimation() wiederholt mit steigendem
// step aufgerufen, um die Elemente schrittweise aufzubauen.
void drawGraphicsTest(uint8_t step)
{
    const int16_t width = display.width();
    const float t = static_cast<float>(step) / GRAPHICS_ANIMATION_STEPS;

    display.fillRect(0, 0, width, 19, GxEPD_BLACK);
    display.setFont(nullptr);
    display.setTextSize(2);
    drawCenteredText("GRAFIK/KONTRAST", 2, nullptr, GxEPD_WHITE);
    display.setTextSize(1);

    // Schachbrett: Fuellung/Umriss wechseln bei jedem Schritt.
    constexpr int16_t cellSize = 8;
    constexpr int16_t startX = 10;
    constexpr int16_t startY = 31;
    const bool flipColors = (step % 2) == 1;

    for (int16_t row = 0; row < 5; ++row)
    {
        for (int16_t column = 0; column < 5; ++column)
        {
            const bool filled = ((row + column) % 2 == 0) != flipColors;

            if (filled)
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

    // Kreise: wachsen von innen nach aussen auf ihre Zielgroesse.
    const int16_t outerRadius = static_cast<int16_t>(20 * t);
    const int16_t middleRadius = static_cast<int16_t>(14 * t);
    const int16_t innerRadius = static_cast<int16_t>(5 * t);

    if (outerRadius > 0)
    {
        display.drawCircle(80, 51, outerRadius, GxEPD_BLACK);
    }
    if (middleRadius > 0)
    {
        display.drawCircle(80, 51, middleRadius, GxEPD_BLACK);
    }
    if (innerRadius > 0)
    {
        display.fillCircle(80, 51, innerRadius, GxEPD_BLACK);
    }

    // Dreiecke: schrumpfen von aussen (vergroessert) nach innen auf ihre
    // Zielgroesse, also die entgegengesetzte Bewegungsrichtung der Kreise.
    constexpr int16_t triangleCenterX = 126;
    constexpr int16_t triangleCenterY = 58;
    const float triangleScale = 1.0F + (1.0F - t) * 0.6F;

    const int16_t outerAx = static_cast<int16_t>(triangleCenterX + (106 - triangleCenterX) * triangleScale);
    const int16_t outerAy = static_cast<int16_t>(triangleCenterY + (71 - triangleCenterY) * triangleScale);
    const int16_t outerBx = static_cast<int16_t>(triangleCenterX + (126 - triangleCenterX) * triangleScale);
    const int16_t outerBy = static_cast<int16_t>(triangleCenterY + (31 - triangleCenterY) * triangleScale);
    const int16_t outerCx = static_cast<int16_t>(triangleCenterX + (146 - triangleCenterX) * triangleScale);
    const int16_t outerCy = static_cast<int16_t>(triangleCenterY + (71 - triangleCenterY) * triangleScale);
    display.drawTriangle(outerAx, outerAy, outerBx, outerBy, outerCx, outerCy, GxEPD_BLACK);

    const int16_t innerAx = static_cast<int16_t>(triangleCenterX + (112 - triangleCenterX) * triangleScale);
    const int16_t innerAy = static_cast<int16_t>(triangleCenterY + (67 - triangleCenterY) * triangleScale);
    const int16_t innerBx = static_cast<int16_t>(triangleCenterX + (126 - triangleCenterX) * triangleScale);
    const int16_t innerBy = static_cast<int16_t>(triangleCenterY + (39 - triangleCenterY) * triangleScale);
    const int16_t innerCx = static_cast<int16_t>(triangleCenterX + (140 - triangleCenterX) * triangleScale);
    const int16_t innerCy = static_cast<int16_t>(triangleCenterY + (67 - triangleCenterY) * triangleScale);
    display.drawTriangle(innerAx, innerAy, innerBx, innerBy, innerCx, innerCy, GxEPD_BLACK);

    // Balken: einzelne Striche fallen nacheinander von oben herab und
    // stapeln sich zu einem grossen schwarzen Rechteck, das kurz vor den
    // gepunkteten Rechtecken endet.
    constexpr int16_t barTop = 31;
    constexpr int16_t barBottom = 75;
    constexpr int16_t fallOffset = 6;

    for (uint8_t stripe = 0; stripe < step; ++stripe)
    {
        const int16_t stripeTop = barTop + (stripe * (barBottom - barTop)) / GRAPHICS_ANIMATION_STEPS;
        const int16_t stripeBottom = barTop + ((stripe + 1) * (barBottom - barTop)) / GRAPHICS_ANIMATION_STEPS;
        display.fillRect(160, stripeTop, 78, stripeBottom - stripeTop, GxEPD_BLACK);
    }

    if (step > 0 && step <= GRAPHICS_ANIMATION_STEPS)
    {
        const int16_t landingTop = barTop + ((step - 1) * (barBottom - barTop)) / GRAPHICS_ANIMATION_STEPS;
        const int16_t landingBottom = barTop + (step * (barBottom - barTop)) / GRAPHICS_ANIMATION_STEPS;
        const int16_t fallingTop = (landingTop - fallOffset > barTop) ? (landingTop - fallOffset) : barTop;
        display.fillRect(160, fallingTop, 78, landingBottom - landingTop, GxEPD_BLACK);
    }

    // Graustufen-Simulation durch Punktmuster: beide Bloecke bleiben an ihrem
    // Platz, nur das Muster darin "laeuft" wie ein Laufbild in eine Richtung
    // (aehnlich dem Farbwechsel beim Schachbrett), statt sich zu verschieben.
    constexpr int16_t leftSpacing = 2;
    constexpr int16_t rightSpacing = 3;
    const int16_t leftPhase = step % leftSpacing;
    const int16_t rightPhase = (rightSpacing - (step % rightSpacing)) % rightSpacing;

    for (int16_t x = 160 + leftPhase; x < 198; x += leftSpacing)
    {
        for (int16_t y = 78; y < 104; y += 2)
        {
            display.drawPixel(x, y, GxEPD_BLACK);
        }
    }

    for (int16_t x = 202 + rightPhase; x < 240; x += rightSpacing)
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

    // Fortschrittsbalken: waechst zusammenhaengend von leer (step 0) bis
    // ganz voll (step 10). Die 10 Teilstriche bleiben als duenne Markierungen
    // sichtbar (weiss auf dem bereits gefuellten Teil, schwarz auf dem noch
    // leeren Teil), damit die Unterteilung erkennbar bleibt.
    constexpr int16_t trackLeft = 8;
    constexpr int16_t trackTop = 110;
    constexpr int16_t trackWidth = 234;
    constexpr int16_t trackHeight = 6;
    const int16_t fillWidth = static_cast<int16_t>(trackWidth * t);

    display.drawRoundRect(6, 108, width - 12, 10, 4, GxEPD_BLACK);
    display.fillRect(trackLeft, trackTop, fillWidth, trackHeight, GxEPD_BLACK);

    for (uint8_t tick = 1; tick < GRAPHICS_ANIMATION_STEPS; ++tick)
    {
        const int16_t tickX = trackLeft + (trackWidth * tick) / GRAPHICS_ANIMATION_STEPS;
        const uint16_t tickColor = (tickX < trackLeft + fillWidth) ? GxEPD_WHITE : GxEPD_BLACK;
        display.drawLine(tickX, trackTop, tickX, trackTop + trackHeight - 1, tickColor);
    }
}

constexpr uint8_t GRAPHICS_ANIMATION_LOOPS = 2;

void playGraphicsTestAnimation()
{
    // Jeder Schritt ist ein vollstaendiger E-Paper-Refresh; die tatsaechliche
    // Dauer haengt stark vom Panel ab und liegt meist deutlich ueber
    // frameDelayMilliseconds. Werte hier nach Bedarf anpassen.
    constexpr uint32_t frameDelayMilliseconds = 300;
    constexpr uint32_t finalHoldMilliseconds = 1500;

    for (uint8_t loopCount = 0; loopCount < GRAPHICS_ANIMATION_LOOPS; ++loopCount)
    {
        // Ist der Balken einmal voll (step == GRAPHICS_ANIMATION_STEPS),
        // beginnt der naechste Durchlauf wieder bei step 0 statt rueckwaerts
        // abzuspielen.
        for (uint8_t step = 0; step <= GRAPHICS_ANIMATION_STEPS; ++step)
        {
            display.setFullWindow();
            display.firstPage();

            do
            {
                display.fillScreen(GxEPD_WHITE);
                drawGraphicsTest(step);
            }
            while (display.nextPage());

            delay(frameDelayMilliseconds);
        }
    }

    delay(finalHoldMilliseconds);
}

// =====================================================
// Bildschirm 3: Status-Dashboard
// =====================================================

// Kopfzeile jetzt 30px hoch, damit das WLAN-Symbol in Originalgroesse
// (Radius 10) hineinpasst. Alles darunter ist entsprechend enger gestaffelt,
// damit die Gesamthoehe von 122px wieder aufgeht.
constexpr int16_t DASHBOARD_HEADER_HEIGHT = 30;
constexpr int16_t DASHBOARD_CONTENT_TOP = 34;
constexpr int16_t DASHBOARD_CONTENT_HEIGHT = 63;
constexpr int16_t DASHBOARD_FOOTER_TOP = 101;
constexpr int16_t DASHBOARD_FOOTER_HEIGHT = 12;

void drawDashboard()
{
    const int16_t width = display.width();

    // Kopfzeile
    display.fillRect(0, 0, width, DASHBOARD_HEADER_HEIGHT, GxEPD_BLACK);

    display.setFont(nullptr);
    display.setTextSize(1);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(7, 11);
    display.print("EPD STATUS");

    drawGearIcon(165, 15, GxEPD_WHITE);
    // y=16, Radius 10 (Originalgroesse): der Punkt (unterer Rand des
    // Symbols) liegt damit mittig in der 30px hohen Kopfzeile, siehe
    // drawWifiIcon().
    drawWifiIcon(190, 16, GxEPD_WHITE);
    drawBatteryIcon(216, 10, 82, GxEPD_WHITE);

    // Linker Hauptbereich
    display.drawRoundRect(6, DASHBOARD_CONTENT_TOP, 133, DASHBOARD_CONTENT_HEIGHT, 7, GxEPD_BLACK);
    drawSunIcon(18, 56, 4);

    display.setFont(&FreeMonoBold18pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(34, 60);
    display.print("23.7");

    display.drawCircle(120, 37, 2, GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(126, 44);
    display.print("C");

    display.setFont(nullptr);
    display.setTextSize(1);
    display.setCursor(18, 86);
    display.print("RAUMTEMPERATUR");

    // Rechte Statusfelder
    display.drawRoundRect(146, DASHBOARD_CONTENT_TOP, 98, 26, 6, GxEPD_BLACK);
    display.setCursor(155, DASHBOARD_CONTENT_TOP + 3);
    display.print("VERBINDUNG");
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(154, DASHBOARD_CONTENT_TOP + 23);
    display.print("ONLINE");

    constexpr int16_t secondBoxTop = DASHBOARD_CONTENT_TOP + 32;
    display.drawRoundRect(146, secondBoxTop, 98, 26, 6, GxEPD_BLACK);
    display.setFont(nullptr);
    display.setTextSize(1);
    display.setCursor(155, secondBoxTop + 3);
    display.print("DISPLAY");
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(154, secondBoxTop + 23);
    display.print("BEREIT");

    // Fusszeile mit Fortschrittsbalken
    display.setFont(nullptr);
    display.setTextSize(1);
    display.setCursor(7, DASHBOARD_FOOTER_TOP + 2);
    display.print("TEST ERFOLGREICH");

    display.drawRoundRect(140, DASHBOARD_FOOTER_TOP, 104, DASHBOARD_FOOTER_HEIGHT, 4, GxEPD_BLACK);
    display.fillRoundRect(143, DASHBOARD_FOOTER_TOP + 3, 96, 6, 2, GxEPD_BLACK);
}

// =====================================================
// Bildschirm 4: Optionen
// =====================================================

constexpr int16_t OPTIONS_HEADER_HEIGHT = 22;
constexpr int16_t OPTIONS_ROW_HEIGHT = 25; // (122 - OPTIONS_HEADER_HEIGHT) / 4

// Zeichnet eine Optionszeile: Bezeichnung links, aktueller Wert rechts.
// "selected" zeigt beispielhaft, wie eine per Taster/Encoder ausgewaehlte
// Zeile hervorgehoben werden koennte (invertierte Farben + ">"-Marker).
void drawOptionRow(int16_t index, const char* label, const char* value, bool selected)
{
    const int16_t width = display.width();
    const int16_t rowTop = OPTIONS_HEADER_HEIGHT + index * OPTIONS_ROW_HEIGHT;
    const uint16_t foreground = selected ? GxEPD_WHITE : GxEPD_BLACK;

    if (selected)
    {
        display.fillRect(0, rowTop, width, OPTIONS_ROW_HEIGHT, GxEPD_BLACK);
        display.setFont(nullptr);
        display.setTextSize(1);
        display.setTextColor(GxEPD_WHITE);
        display.setCursor(6, rowTop + 9);
        display.print(">");
    }

    display.setFont(nullptr);
    display.setTextSize(1);
    display.setTextColor(foreground);
    display.setCursor(selected ? 18 : 10, rowTop + 9);
    display.print(label);

    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(foreground);
    int16_t boundsX;
    int16_t boundsY;
    uint16_t valueWidth;
    uint16_t valueHeight;
    display.getTextBounds(value, 0, 0, &boundsX, &boundsY, &valueWidth, &valueHeight);
    display.setCursor(width - 10 - static_cast<int16_t>(valueWidth), rowTop + 16);
    display.print(value);

    if (index > 0)
    {
        display.drawLine(0, rowTop, width, rowTop, GxEPD_BLACK);
    }
}

void drawOptionsScreen()
{
    const int16_t width = display.width();

    display.fillRect(0, 0, width, OPTIONS_HEADER_HEIGHT, GxEPD_BLACK);
    display.setFont(nullptr);
    display.setTextSize(1);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(7, 8);
    display.print("OPTIONEN");
    drawGearIcon(width - 16, OPTIONS_HEADER_HEIGHT / 2, GxEPD_WHITE);

    drawOptionRow(0, "SPRACHE", "DEUTSCH", true);
    drawOptionRow(1, "UHRZEIT", "AUTO (WLAN)", false);
    drawOptionRow(2, "ANZEIGE", "KONTRAST 50%", false);
    drawOptionRow(3, "WLAN/GERAET", "VERBUNDEN", false);
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

    Serial.println("1/4 Startbild anzeigen");
    showScreen(drawSplashScreen, 3000);

    Serial.println("2/4 Grafiktest anzeigen");
    playGraphicsTestAnimation();

    Serial.println("3/4 Dashboard anzeigen");
    showScreen(drawDashboard, 3000);

    Serial.println("4/4 Optionen anzeigen");
    showScreen(drawOptionsScreen, 3000);

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
