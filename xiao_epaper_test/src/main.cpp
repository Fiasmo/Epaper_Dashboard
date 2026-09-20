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

// Standardschrift (6x8 Pixel pro Zeichen). y ist wie bei den GFX-Schriften die
// Grundlinie, nicht die Oberkante.
void drawSmallText(int16_t x, int16_t baselineY, const char* text, uint16_t color = GxEPD_BLACK)
{
    display.setFont(nullptr);
    display.setTextSize(1);
    display.setTextColor(color);
    display.setCursor(x, baselineY - 7);
    display.print(text);
}

// Die Standardschrift kennt keinen Fettdruck, daher doppelt mit 1 Pixel Versatz.
void drawSmallBoldText(int16_t x, int16_t baselineY, const char* text, uint16_t color = GxEPD_BLACK)
{
    drawSmallText(x, baselineY, text, color);
    drawSmallText(x + 1, baselineY, text, color);
}

void drawSmallTextCentered(
    int16_t centerX,
    int16_t baselineY,
    const char* text,
    uint16_t color = GxEPD_BLACK,
    bool bold = false
)
{
    const int16_t textWidth = static_cast<int16_t>(strlen(text)) * 6 + (bold ? 1 : 0);
    const int16_t x = centerX - textWidth / 2;

    if (bold)
    {
        drawSmallBoldText(x, baselineY, text, color);
    }
    else
    {
        drawSmallText(x, baselineY, text, color);
    }
}

void drawFontText(
    int16_t x,
    int16_t baselineY,
    const char* text,
    const GFXfont* font,
    uint16_t color = GxEPD_BLACK
)
{
    display.setFont(font);
    display.setTextColor(color);
    display.setCursor(x, baselineY);
    display.print(text);
}

// Zentriert die Umrisse des Textes um (centerX, centerY), z.B. fuer ein grosses "?".
void drawGlyphCentered(
    const char* text,
    int16_t centerX,
    int16_t centerY,
    const GFXfont* font,
    uint16_t color
)
{
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;

    display.setFont(font);
    display.setTextColor(color);
    display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
    display.setCursor(
        centerX - x1 - static_cast<int16_t>(width) / 2,
        centerY - y1 - static_cast<int16_t>(height) / 2
    );
    display.print(text);
}

// Dicke Linie: drei versetzte Linien ergeben etwa 2 Pixel Strichstaerke.
void drawThickLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    display.drawLine(x0, y0, x1, y1, color);
    display.drawLine(x0 + 1, y0, x1 + 1, y1, color);
    display.drawLine(x0, y0 + 1, x1, y1 + 1, color);
}

// Durchgestrichen-Balken: zuerst eine breite Luecke in der Hintergrundfarbe
// freiraeumen, dann die eigentliche Linie darueber legen.
void drawSlash(
    int16_t x0,
    int16_t y0,
    int16_t x1,
    int16_t y1,
    int16_t lineThickness,
    int16_t gap,
    uint16_t lineColor,
    uint16_t gapColor
)
{
    for (int16_t offset = -gap; offset < lineThickness + gap; ++offset)
    {
        display.drawLine(x0 + offset, y0, x1 + offset, y1, gapColor);
    }

    for (int16_t offset = 0; offset < lineThickness; ++offset)
    {
        display.drawLine(x0 + offset, y0, x1 + offset, y1, lineColor);
    }
}

// WLAN-Faecher: Kreisringe mit den Radien r1..r3 um (cx, cy), begrenzt auf den
// 90-Grad-Keil nach oben. Der Punkt in der Mitte wird vom Aufrufer gezeichnet.
void drawWifiFan(
    int16_t cx,
    int16_t cy,
    int16_t r1,
    int16_t r2,
    int16_t r3,
    float thickness,
    uint16_t color
)
{
    const float half = thickness / 2.0F;
    const int16_t radii[3] = { r1, r2, r3 };

    for (int16_t dy = -(r3 + 2); dy < 0; ++dy)
    {
        for (int16_t dx = dy; dx <= -dy; ++dx)
        {
            const float distance = sqrtf(static_cast<float>(dx * dx + dy * dy));

            for (int16_t radius : radii)
            {
                if (fabsf(distance - radius) <= half)
                {
                    display.drawPixel(cx + dx, cy + dy, color);
                    break;
                }
            }
        }
    }
}

void drawCheckMark(int16_t cx, int16_t cy, bool large, uint16_t color)
{
    if (large)
    {
        drawThickLine(cx - 5, cy, cx - 1, cy + 4, color);
        drawThickLine(cx - 1, cy + 4, cx + 6, cy - 6, color);
        return;
    }

    for (int16_t offset = 0; offset < 2; ++offset)
    {
        display.drawLine(cx - 3 + offset, cy, cx - 1 + offset, cy + 2, color);
        display.drawLine(cx - 1 + offset, cy + 2, cx + 3 + offset, cy - 3, color);
    }
}

void drawCrossMark(int16_t cx, int16_t cy, bool large, uint16_t color)
{
    if (large)
    {
        drawThickLine(cx - 4, cy - 4, cx + 4, cy + 4, color);
        drawThickLine(cx + 4, cy - 4, cx - 4, cy + 4, color);
        return;
    }

    for (int16_t offset = 0; offset < 2; ++offset)
    {
        display.drawLine(cx - 3 + offset, cy - 2, cx + 1 + offset, cy + 2, color);
        display.drawLine(cx + 1 + offset, cy - 2, cx - 3 + offset, cy + 2, color);
    }
}

// Runde Statusmarke mit Haken (ok) oder Kreuz (Fehler). onDark = auf schwarzem
// Untergrund, dann werden die Farben getauscht.
void drawStatusBadge(int16_t cx, int16_t cy, int16_t radius, bool ok, bool onDark)
{
    const uint16_t badgeColor = onDark ? GxEPD_WHITE : GxEPD_BLACK;
    const uint16_t markColor = onDark ? GxEPD_BLACK : GxEPD_WHITE;
    const bool large = radius >= 8;

    display.fillCircle(cx, cy, radius, badgeColor);

    if (ok)
    {
        drawCheckMark(cx, cy, large, markColor);
    }
    else
    {
        drawCrossMark(cx, cy, large, markColor);
    }
}

// Thermometer mit Skala. hollow = nur Umriss, ohne Quecksilbersaeule (kein Messwert).
void drawThermometerIcon(int16_t cx, int16_t tubeTop, uint16_t foreground, uint16_t background, bool hollow)
{
    const int16_t bulbY = tubeTop + 22;

    display.drawRoundRect(cx - 2, tubeTop, 5, 19, 2, foreground);

    if (hollow)
    {
        display.fillCircle(cx, bulbY, 5, background);
        display.drawCircle(cx, bulbY, 5, foreground);
    }
    else
    {
        display.fillCircle(cx, bulbY, 5, foreground);
        display.drawFastVLine(cx, tubeTop + 6, 16, foreground);
    }

    for (int16_t tick = 0; tick < 4; ++tick)
    {
        display.drawFastHLine(cx + 5, tubeTop + 3 + tick * 4, (tick % 2 == 0) ? 4 : 3, foreground);
    }
}

void drawDropletShape(int16_t cx, int16_t cy, int16_t radius, int16_t tipDistance, uint16_t color)
{
    const float ratio = static_cast<float>(radius) / tipDistance;
    const int16_t tangentX = static_cast<int16_t>(lroundf(radius * sqrtf(1.0F - ratio * ratio)));
    const int16_t tangentY = static_cast<int16_t>(lroundf(radius * ratio));

    display.fillCircle(cx, cy, radius, color);
    display.fillTriangle(
        cx, cy - tipDistance,
        cx + tangentX, cy - tangentY,
        cx - tangentX, cy - tangentY,
        color
    );
}

// Tropfen mit Mittelpunkt (cx, cy). hollow = nur Umriss (kein Messwert).
void drawDropletIcon(
    int16_t cx,
    int16_t cy,
    int16_t radius,
    int16_t tipDistance,
    uint16_t foreground,
    uint16_t background,
    bool hollow
)
{
    drawDropletShape(cx, cy, radius, tipDistance, foreground);

    if (hollow)
    {
        constexpr int16_t wall = 2;
        drawDropletShape(cx, cy, radius - wall, tipDistance * (radius - wall) / radius, background);
    }
    else
    {
        display.fillCircle(cx - radius / 3, cy + radius / 3, 1, background);
    }
}

void drawMonitorIcon(int16_t x, int16_t y)
{
    display.drawRoundRect(x, y, 16, 11, 2, GxEPD_BLACK);
    display.fillTriangle(x + 3, y + 9, x + 7, y + 4, x + 11, y + 9, GxEPD_BLACK);
    display.fillCircle(x + 12, y + 4, 1, GxEPD_BLACK);
    display.fillRect(x + 6, y + 11, 4, 2, GxEPD_BLACK);
    display.fillRect(x + 2, y + 13, 12, 2, GxEPD_BLACK);
}

void drawWarningIcon(int16_t cx, int16_t top, uint16_t color)
{
    display.drawTriangle(cx, top, cx + 9, top + 15, cx - 9, top + 15, color);
    display.fillRect(cx - 1, top + 5, 2, 5, color);
    display.fillRect(cx - 1, top + 11, 2, 2, color);
}

// Zahnrad fuer die Kopfzeile der Statusanzeige: Doppelring, Nabe und 8 Zaehne.
void drawGearIconBold(int16_t x, int16_t y, uint16_t color)
{
    display.drawCircle(x, y, 5, color);
    display.drawCircle(x, y, 6, color);
    display.drawCircle(x, y, 2, color);

    for (int16_t angle = 0; angle < 360; angle += 45)
    {
        const float radians = angle * PI / 180.0F;
        const int16_t toothX = x + static_cast<int16_t>(lroundf(cosf(radians) * 8.0F));
        const int16_t toothY = y + static_cast<int16_t>(lroundf(sinf(radians) * 8.0F));
        display.fillRect(toothX - 1, toothY - 1, 3, 3, color);
    }
}

// Kleiner Akku (24 x 12) mit vier Segmenten. Ohne Messwert steht ein "?" darin.
void drawHeaderBatteryIcon(int16_t x, int16_t y, bool ok, uint8_t percent, uint16_t color)
{
    constexpr int16_t segmentCount = 4;
    constexpr int16_t segmentX[segmentCount] = { 3, 8, 13, 18 };
    constexpr int16_t segmentWidth[segmentCount] = { 4, 4, 4, 3 };

    display.drawRoundRect(x, y, 24, 12, 2, color);
    display.drawRoundRect(x + 1, y + 1, 22, 10, 1, color);
    display.fillRect(x + 24, y + 4, 3, 4, color);

    if (!ok)
    {
        drawSmallBoldText(x + 9, y + 10, "?", color);
        return;
    }

    const int16_t filledSegments = (percent * segmentCount + 50) / 100;

    for (int16_t segment = 0; segment < segmentCount; ++segment)
    {
        if (segment < filledSegments)
        {
            display.fillRect(x + segmentX[segment], y + 3, segmentWidth[segment], 6, color);
        }
        else
        {
            display.drawRect(x + segmentX[segment], y + 3, segmentWidth[segment], 6, color);
        }
    }
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
// Messwerte und Selbsttest
// =====================================================

// Testschalter: true simuliert den jeweiligen Ausfall, damit die Fehlerbilder
// ohne defekte Hardware geprueft werden koennen.
constexpr bool SIMULATE_BATTERY_FAILURE = false;
constexpr bool SIMULATE_TEMPERATURE_FAILURE = false;
constexpr bool SIMULATE_HUMIDITY_FAILURE = false;
constexpr bool SIMULATE_WIFI_FAILURE = false;

constexpr uint8_t TOTAL_CHECKS = 4; // Display, Sensoren, WLAN, Akku

// Beispielname; SSID und Passwort gehoeren nicht ins Repository.
const char WIFI_NETWORK_NAME[] = "MEIN-WLAN";

struct BatteryState
{
    bool ok;
    uint8_t percent;
    float voltage;
};

struct SensorState
{
    bool temperatureOk;
    float temperature;
    bool humidityOk;
    uint8_t humidity;
};

struct WifiState
{
    bool ok;
    int8_t signalDbm;
};

BatteryState battery = { false, 0, 0.0F };
SensorState sensors = { false, 0.0F, false, 0 };
WifiState wifiState = { false, 0 };

// Die drei Funktionen liefern hier Beispielwerte. Die echte Hardware kennt
// dieses Testprogramm nicht; die Stellen fuer echte Messungen sind mit TODO markiert.
// Schlaegt eine Messung fehl, bleibt der Zustand auf "nicht ok" und die
// Bildschirme zeigen den Fehlerfall statt eines erfundenen Wertes.

void readBattery()
{
    // TODO: echte Akkuspannung messen (ADC-Pin und Spannungsteiler haengen von der Verdrahtung ab).
    if (SIMULATE_BATTERY_FAILURE)
    {
        battery = { false, 0, 0.0F };
        Serial.println("Akku: FEHLER (keine Messung moeglich)");
        return;
    }

    battery = { true, 82, 3.98F };
    Serial.printf("Akku: OK, %u %%, %.2f V\n", battery.percent, battery.voltage);
}

void readSensors()
{
    // TODO: echte Sensoren auslesen (Typ und Anschluss haengen von der Verdrahtung ab).
    sensors.temperatureOk = !SIMULATE_TEMPERATURE_FAILURE;
    sensors.temperature = sensors.temperatureOk ? 23.7F : 0.0F;
    sensors.humidityOk = !SIMULATE_HUMIDITY_FAILURE;
    sensors.humidity = sensors.humidityOk ? 48 : 0;

    if (sensors.temperatureOk)
    {
        Serial.printf("Temperatur: OK, %.1f C\n", sensors.temperature);
    }
    else
    {
        Serial.println("Temperatur: FEHLER (Sensor antwortet nicht)");
    }

    if (sensors.humidityOk)
    {
        Serial.printf("Luftfeuchtigkeit: OK, %u %%\n", sensors.humidity);
    }
    else
    {
        Serial.println("Luftfeuchtigkeit: FEHLER (Sensor antwortet nicht)");
    }
}

void connectWifi()
{
    // TODO: echte WLAN-Verbindung aufbauen und Signalstaerke (RSSI) lesen.
    if (SIMULATE_WIFI_FAILURE)
    {
        wifiState = { false, 0 };
        Serial.println("WLAN: FEHLER (keine Verbindung)");
        return;
    }

    wifiState = { true, -58 };
    Serial.printf("WLAN: OK, %d dBm\n", wifiState.signalDbm);
}

bool sensorsOk()
{
    return sensors.temperatureOk && sensors.humidityOk;
}

uint8_t countFailedChecks()
{
    return static_cast<uint8_t>(!sensorsOk()) +
           static_cast<uint8_t>(!wifiState.ok) +
           static_cast<uint8_t>(!battery.ok);
}

// Ohne Messwert: "--.-" bzw. "--".
void formatTemperature(char* buffer, size_t size)
{
    if (sensors.temperatureOk)
    {
        snprintf(buffer, size, "%.1f", sensors.temperature);
    }
    else
    {
        snprintf(buffer, size, "--.-");
    }
}

void formatHumidity(char* buffer, size_t size)
{
    if (sensors.humidityOk)
    {
        snprintf(buffer, size, "%u", sensors.humidity);
    }
    else
    {
        snprintf(buffer, size, "--");
    }
}

// =====================================================
// Bildschirme 3 bis 6: Selbsttest (Akku, Sensoren, WLAN, Zusammenfassung)
// =====================================================

// Gemeinsamer Rahmen mit Titelbalken. step ("1/3") ist optional.
void drawCheckScreenFrame(const char* title, const char* step)
{
    const int16_t width = display.width();
    const int16_t height = display.height();

    display.drawRoundRect(0, 0, width, height, 8, GxEPD_BLACK);
    display.drawRoundRect(3, 3, width - 6, height - 6, 6, GxEPD_BLACK);
    display.fillRoundRect(8, 10, width - 16, 26, 5, GxEPD_BLACK);
    drawCenteredText(title, 28, &FreeMonoBold9pt7b, GxEPD_WHITE);

    if (step != nullptr)
    {
        drawSmallText(width - 14 - static_cast<int16_t>(strlen(step)) * 6, 27, step, GxEPD_WHITE);
    }
}

void drawBatteryCheckScreen()
{
    drawCheckScreenFrame("AKKU-PRUEFUNG", "1/3");

    display.drawRoundRect(24, 58, 66, 34, 5, GxEPD_BLACK);
    display.drawRoundRect(25, 59, 64, 32, 4, GxEPD_BLACK);
    display.fillRect(90, 68, 5, 14, GxEPD_BLACK);

    drawSmallText(112, 56, "LADUNG");

    char text[24];

    if (battery.ok)
    {
        constexpr int16_t segmentCount = 5;
        const int16_t filledSegments = (battery.percent * segmentCount + 50) / 100;

        for (int16_t segment = 0; segment < segmentCount; ++segment)
        {
            const int16_t segmentX = 28 + segment * 12;

            if (segment < filledSegments)
            {
                display.fillRect(segmentX, 62, 10, 26, GxEPD_BLACK);
            }
            else
            {
                display.drawRect(segmentX, 62, 10, 26, GxEPD_BLACK);
            }
        }

        snprintf(text, sizeof(text), "%u", battery.percent);
        drawFontText(112, 80, text, &FreeMonoBold12pt7b);
        drawFontText(112 + static_cast<int16_t>(strlen(text)) * 14 + 1, 80, "%", &FreeMonoBold9pt7b);

        snprintf(text, sizeof(text), "SPANNUNG: %.2f V", battery.voltage);
        drawSmallText(112, 98, text);
        drawSmallText(112, 109, "STATUS: GUT");
    }
    else
    {
        drawGlyphCentered("?", 57, 75, &FreeMonoBold18pt7b, GxEPD_BLACK);

        drawFontText(112, 80, "--", &FreeMonoBold12pt7b);
        drawFontText(112 + 2 * 14 + 1, 80, "%", &FreeMonoBold9pt7b);

        drawSmallText(112, 98, "SPANNUNG: --- V");
        drawSmallBoldText(112, 109, "STATUS: FEHLER");
    }

    drawStatusBadge(224, 68, 10, battery.ok, false);
}

// Sensorkachel: bei Fehler schwarz gefuellt mit weisser Schrift.
void drawSensorTileFrame(int16_t tileX, bool ok)
{
    if (ok)
    {
        display.drawRoundRect(tileX, 44, 115, 66, 7, GxEPD_BLACK);
    }
    else
    {
        display.fillRoundRect(tileX, 44, 115, 66, 7, GxEPD_BLACK);
    }
}

void drawTemperatureTile()
{
    constexpr int16_t tileX = 8;
    const bool ok = sensors.temperatureOk;
    const uint16_t foreground = ok ? GxEPD_BLACK : GxEPD_WHITE;
    const uint16_t background = ok ? GxEPD_WHITE : GxEPD_BLACK;
    char text[12];

    drawSensorTileFrame(tileX, ok);
    drawThermometerIcon(26, 61, foreground, background, !ok);
    drawSmallText(42, 62, "TEMPERATUR", foreground);

    formatTemperature(text, sizeof(text));
    drawFontText(42, 88, text, &FreeMonoBold12pt7b, foreground);

    const int16_t valueEnd = 42 + static_cast<int16_t>(strlen(text)) * 14;
    display.drawCircle(valueEnd + 4, 74, 2, foreground);
    drawFontText(valueEnd + 8, 88, "C", &FreeMonoBold9pt7b, foreground);

    if (ok)
    {
        drawSmallText(42, 102, "GEPRUEFT", foreground);
    }
    else
    {
        drawSmallBoldText(42, 102, "FEHLER", foreground);
    }

    drawStatusBadge(112, 98, 6, ok, !ok);
}

void drawHumidityTile()
{
    constexpr int16_t tileX = 127;
    const bool ok = sensors.humidityOk;
    const uint16_t foreground = ok ? GxEPD_BLACK : GxEPD_WHITE;
    const uint16_t background = ok ? GxEPD_WHITE : GxEPD_BLACK;
    char text[12];

    drawSensorTileFrame(tileX, ok);
    drawDropletIcon(144, 83, 6, 17, foreground, background, !ok);
    drawSmallText(160, 62, "LUFTFEUCHTE", foreground);

    formatHumidity(text, sizeof(text));
    drawFontText(160, 88, text, &FreeMonoBold12pt7b, foreground);
    drawFontText(160 + static_cast<int16_t>(strlen(text)) * 14 + 1, 88, "%", &FreeMonoBold9pt7b, foreground);

    if (ok)
    {
        drawSmallText(160, 102, "GEPRUEFT", foreground);
    }
    else
    {
        drawSmallBoldText(160, 102, "FEHLER", foreground);
    }

    drawStatusBadge(231, 98, 6, ok, !ok);
}

void drawSensorCheckScreen()
{
    drawCheckScreenFrame("SENSOR-PRUEFUNG", "2/3");
    drawTemperatureTile();
    drawHumidityTile();
}

// 1 bis 4 gefuellte Balken je nach Signalstaerke (RSSI in dBm).
uint8_t signalBarCount(int8_t signalDbm)
{
    if (signalDbm >= -55)
    {
        return 4;
    }
    if (signalDbm >= -67)
    {
        return 3;
    }
    if (signalDbm >= -75)
    {
        return 2;
    }
    return 1;
}

void drawWifiCheckScreen()
{
    drawCheckScreenFrame("WLAN-PRUEFUNG", "3/3");

    display.fillCircle(56, 92, 3, GxEPD_BLACK);
    drawWifiFan(56, 92, 12, 21, 30, 3.5F, GxEPD_BLACK);

    drawSmallText(110, 54, "NETZWERK");
    drawFontText(110, 72, WIFI_NETWORK_NAME, &FreeMonoBold9pt7b);

    if (wifiState.ok)
    {
        constexpr int16_t barCount = 4;
        constexpr int16_t barHeights[barCount] = { 4, 7, 10, 13 };
        const int16_t filledBars = signalBarCount(wifiState.signalDbm);
        char text[16];

        drawSmallText(110, 88, "SIGNAL");

        for (int16_t bar = 0; bar < barCount; ++bar)
        {
            const int16_t barX = 110 + bar * 8;
            const int16_t barTop = 106 - barHeights[bar];

            if (bar < filledBars)
            {
                display.fillRect(barX, barTop, 5, barHeights[bar], GxEPD_BLACK);
            }
            else
            {
                display.drawRect(barX, barTop, 5, barHeights[bar], GxEPD_BLACK);
            }
        }

        snprintf(text, sizeof(text), "%d DBM", wifiState.signalDbm);
        drawSmallBoldText(148, 106, text);
    }
    else
    {
        drawSlash(30, 104, 84, 62, 5, 3, GxEPD_BLACK, GxEPD_WHITE);

        drawSmallText(110, 88, "STATUS");
        drawSmallBoldText(110, 106, "KEINE VERBINDUNG");
    }

    drawStatusBadge(224, 68, 10, wifiState.ok, false);
}

void drawSummaryItem(int16_t circleX, int16_t circleY, const char* label, bool ok)
{
    if (ok)
    {
        display.drawCircle(circleX, circleY, 6, GxEPD_BLACK);
        drawCheckMark(circleX, circleY, false, GxEPD_BLACK);
    }
    else
    {
        drawStatusBadge(circleX, circleY, 6, false, false);
    }

    drawFontText(circleX + 11, circleY + 4, label, &FreeMonoBold9pt7b);
}

// "Test erfolgreich" oder "Test mit Fehlern": der Fortschrittsbalken zeigt den
// Anteil der bestandenen Pruefungen (das Display selbst gilt als bestanden,
// sonst waere dieses Bild nicht zu sehen).
void drawSummaryScreen()
{
    const uint8_t failedChecks = countFailedChecks();
    const uint8_t passedChecks = TOTAL_CHECKS - failedChecks;

    drawCheckScreenFrame(failedChecks == 0 ? "TEST ERFOLGREICH" : "TEST MIT FEHLERN", nullptr);

    drawSummaryItem(30, 54, "DISPLAY", true);
    drawSummaryItem(138, 54, "SENSOR", sensorsOk());
    drawSummaryItem(30, 72, "WLAN", wifiState.ok);
    drawSummaryItem(138, 72, "AKKU", battery.ok);

    constexpr int16_t barInnerLeft = 19;
    constexpr int16_t barInnerWidth = 212;
    const int16_t fillWidth = (barInnerWidth * passedChecks) / TOTAL_CHECKS;

    display.drawRoundRect(16, 86, 218, 11, 4, GxEPD_BLACK);
    display.fillRoundRect(barInnerLeft, 89, fillWidth, 5, 2, GxEPD_BLACK);

    for (int16_t tick = 1; tick < 10; ++tick)
    {
        const int16_t tickX = barInnerLeft + (barInnerWidth * tick) / 10;
        const uint16_t tickColor = (tickX < barInnerLeft + fillWidth) ? GxEPD_WHITE : GxEPD_BLACK;
        display.drawFastVLine(tickX, 89, 5, tickColor);
    }

    if (failedChecks == 0)
    {
        drawSmallTextCentered(125, 111, "ALLE PRUEFUNGEN BESTANDEN");
    }
    else
    {
        char text[24];
        snprintf(text, sizeof(text), "%u FEHLER GEFUNDEN", failedChecks);
        drawSmallTextCentered(125, 111, text, GxEPD_BLACK, true);
    }
}

// =====================================================
// Bildschirm 7: Status-Dashboard
// =====================================================

constexpr int16_t DASHBOARD_HEADER_HEIGHT = 30;
constexpr int16_t DASHBOARD_BOX_TOP = 34;
constexpr int16_t DASHBOARD_BOX_HEIGHT = 82;
constexpr int16_t DASHBOARD_LEFT_BOX_LEFT = 6;
constexpr int16_t DASHBOARD_LEFT_BOX_WIDTH = 133;
constexpr int16_t DASHBOARD_DIVIDER_Y = 78;
constexpr int16_t DASHBOARD_RIGHT_BOX_LEFT = 146;
constexpr int16_t DASHBOARD_RIGHT_BOX_WIDTH = 98;
constexpr int16_t DASHBOARD_RIGHT_BOX_HEIGHT = 38;
constexpr int16_t DASHBOARD_RIGHT_BOX_GAP = 6;

void drawDashboardHeader()
{
    display.fillRect(0, 0, display.width(), DASHBOARD_HEADER_HEIGHT, GxEPD_BLACK);
    drawSmallBoldText(8, 19, "EPD STATUS", GxEPD_WHITE);

    if (countFailedChecks() > 0)
    {
        drawWarningIcon(88, 8, GxEPD_WHITE);
    }

    drawGearIconBold(165, 15, GxEPD_WHITE);

    display.fillCircle(190, 22, 2, GxEPD_WHITE);
    drawWifiFan(190, 22, 6, 10, 14, 1.6F, GxEPD_WHITE);

    if (!wifiState.ok)
    {
        drawSlash(178, 26, 202, 6, 2, 3, GxEPD_WHITE, GxEPD_BLACK);
    }

    drawHeaderBatteryIcon(214, 9, battery.ok, battery.percent, GxEPD_WHITE);
}

// Linke Box: oben Temperatur, unten Luftfeuchtigkeit. Ein ausgefallener Sensor
// zeigt "--", einen Umriss-Icon und (bei der Luftfeuchtigkeit) die Marke FEHLER.
void drawDashboardClimateBox()
{
    char text[12];

    display.drawRoundRect(
        DASHBOARD_LEFT_BOX_LEFT,
        DASHBOARD_BOX_TOP,
        DASHBOARD_LEFT_BOX_WIDTH,
        DASHBOARD_BOX_HEIGHT,
        7,
        GxEPD_BLACK
    );

    drawThermometerIcon(21, 41, GxEPD_BLACK, GxEPD_WHITE, !sensors.temperatureOk);
    drawSmallText(38, 47, "RAUMTEMPERATUR");

    formatTemperature(text, sizeof(text));
    drawFontText(38, 70, text, &FreeMonoBold12pt7b);

    const int16_t temperatureEnd = 38 + static_cast<int16_t>(strlen(text)) * 14;
    display.drawCircle(temperatureEnd + 4, 55, 2, GxEPD_BLACK);
    drawFontText(temperatureEnd + 8, 70, "C", &FreeMonoBold9pt7b);

    // Durchgehende Trennlinie von Rand zu Rand.
    display.drawFastHLine(
        DASHBOARD_LEFT_BOX_LEFT,
        DASHBOARD_DIVIDER_Y,
        DASHBOARD_LEFT_BOX_WIDTH,
        GxEPD_BLACK
    );

    drawDropletIcon(21, 97, 6, 12, GxEPD_BLACK, GxEPD_WHITE, !sensors.humidityOk);
    drawSmallText(38, 91, "LUFTFEUCHTE");

    formatHumidity(text, sizeof(text));
    drawFontText(38, 109, text, &FreeMonoBold9pt7b);
    display.print("%");

    if (sensors.humidityOk)
    {
        constexpr int16_t gaugeInnerWidth = 43;
        const int16_t gaugeFill = (gaugeInnerWidth * sensors.humidity) / 100;

        display.drawRoundRect(84, 98, 47, 7, 3, GxEPD_BLACK);

        if (gaugeFill > 0)
        {
            display.fillRoundRect(86, 100, gaugeFill, 3, 1, GxEPD_BLACK);
        }
    }
    else
    {
        display.fillRoundRect(84, 98, 47, 10, 5, GxEPD_BLACK);
        drawSmallTextCentered(107, 106, "FEHLER", GxEPD_WHITE, true);
    }
}

// Rechte Boxen: Verbindung (bei Ausfall schwarz gefuellt) und Display.
void drawDashboardStatusBoxes()
{
    const bool online = wifiState.ok;
    const uint16_t foreground = online ? GxEPD_BLACK : GxEPD_WHITE;

    if (online)
    {
        display.drawRoundRect(
            DASHBOARD_RIGHT_BOX_LEFT,
            DASHBOARD_BOX_TOP,
            DASHBOARD_RIGHT_BOX_WIDTH,
            DASHBOARD_RIGHT_BOX_HEIGHT,
            6,
            GxEPD_BLACK
        );
    }
    else
    {
        display.fillRoundRect(
            DASHBOARD_RIGHT_BOX_LEFT,
            DASHBOARD_BOX_TOP,
            DASHBOARD_RIGHT_BOX_WIDTH,
            DASHBOARD_RIGHT_BOX_HEIGHT,
            6,
            GxEPD_BLACK
        );
    }

    display.fillCircle(162, 59, 2, foreground);
    drawWifiFan(162, 59, 6, 10, 14, 1.6F, foreground);

    if (!online)
    {
        drawSlash(151, 63, 172, 47, 2, 3, GxEPD_WHITE, GxEPD_BLACK);
    }

    drawSmallText(178, 52, "VERBINDUNG", foreground);
    drawSmallBoldText(178, 66, online ? "ONLINE" : "OFFLINE", foreground);

    constexpr int16_t secondBoxTop = DASHBOARD_BOX_TOP + DASHBOARD_RIGHT_BOX_HEIGHT + DASHBOARD_RIGHT_BOX_GAP;

    display.drawRoundRect(
        DASHBOARD_RIGHT_BOX_LEFT,
        secondBoxTop,
        DASHBOARD_RIGHT_BOX_WIDTH,
        DASHBOARD_RIGHT_BOX_HEIGHT,
        6,
        GxEPD_BLACK
    );
    drawMonitorIcon(154, 88);
    drawSmallText(178, 92, "DISPLAY");
    drawSmallBoldText(178, 106, "BEREIT");
}

void drawDashboard()
{
    drawDashboardHeader();
    drawDashboardClimateBox();
    drawDashboardStatusBoxes();
}

// =====================================================
// Bildschirm 8: Optionen
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

    Serial.println("1/8 Startbild anzeigen");
    showScreen(drawSplashScreen, 3000);

    Serial.println("2/8 Grafiktest anzeigen");
    playGraphicsTestAnimation();

    // Jede Pruefung misst direkt vor ihrem Bild; ein Fehlschlag wird im
    // Zustand vermerkt und in Zusammenfassung und Dashboard erneut gezeigt.
    Serial.println("3/8 Akku pruefen und anzeigen");
    readBattery();
    showScreen(drawBatteryCheckScreen, 2500);

    Serial.println("4/8 Sensoren pruefen und anzeigen");
    readSensors();
    showScreen(drawSensorCheckScreen, 2500);

    Serial.println("5/8 WLAN pruefen und anzeigen");
    connectWifi();
    showScreen(drawWifiCheckScreen, 2500);

    Serial.println("6/8 Zusammenfassung anzeigen");
    Serial.printf("Fehlgeschlagene Pruefungen: %u von %u\n", countFailedChecks(), TOTAL_CHECKS);
    showScreen(drawSummaryScreen, 3000);

    Serial.println("7/8 Dashboard anzeigen");
    showScreen(drawDashboard, 3000);

    Serial.println("8/8 Optionen anzeigen");
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
