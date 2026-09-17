# Epaper_Dashboard — Projektkontext für Claude Code

## Worum es geht

Test- und Dashboard-Sketches für ein E-Paper-Display am Seeed XIAO ESP32-S3.
Gehört zum größeren Vorhaben "EpaperLayoutDesigner", das Layouts für dieses
Display entwirft.

## Hardware

- Board: Seeed Studio XIAO ESP32-S3 (Sense)
- Treiberplatine: Seeed Studio ePaper Driver Board (114993558)
- Display: GDEY0213B74, 2,13 Zoll, 122 x 250 Pixel, Controller SSD1680, schwarz/weiß
- Pinbelegung: CS = D1, DC = D3, RST = D0, BUSY = D2
- Serielle Ausgabe: 115200 Baud

## Verzeichnisse

- `xiao_epaper_test/` — aktives PlatformIO-Projekt, hier wird entwickelt.
  Hauptcode: `xiao_epaper_test/src/main.cpp` (~583 Zeilen)
- `Epaper_Test/Epaper_Test.ino` — erster einfacher Arduino-IDE-Sketch (Text + Smiley)
- `Epaper_Test.ino/Epaper_Test.ino.ino` — leerer Stub, kann ignoriert werden
- `xiao_epaper_test_platformio.zip` — Backup-Archiv

## Bibliotheken

- `zinggjm/GxEPD2` für die E-Paper-Ansteuerung
- `adafruit/Adafruit GFX Library` für Grafikprimitive und die Fonts
  FreeMonoBold 9pt / 12pt / 18pt

## Aufbau von main.cpp

`setup()` zeigt nacheinander vier Bildschirme und legt das Display danach schlafen:

1. `drawSplashScreen()` — Startbild
2. `playGraphicsTestAnimation()` mit `drawGraphicsTest(step)` — Grafiktest in Schritten
3. `drawDashboard()` — Statusanzeige mit Kopfzeile und WLAN-, Zahnrad- und Akkusymbol
4. `drawOptionsScreen()` mit `drawOptionRow(...)` — Optionsliste

`loop()` macht bewusst nichts. Ein E-Paper soll nicht dauernd neu gezeichnet
werden, für einen erneuten Durchlauf RESET drücken.

Hilfsfunktionen: `showScreen(drawFunction, pauseMilliseconds)` kapselt den
Vollbild-Refresh-Zyklus aus `setFullWindow` / `firstPage` / `nextPage`.
`drawCenteredText(...)` zentriert Text über `getTextBounds`. Symbole zeichnen
`drawWifiIcon`, `drawGearIcon`, `drawBatteryIcon` und `drawSunIcon`.

## Konventionen

- `display.setRotation(1)` bedeutet Querformat, es gilt dann 250 breit x 122 hoch
- Kommentare und Serial-Ausgaben auf Deutsch, Umlaute darin als ae/oe/ue geschrieben
- Einrückung in `main.cpp`: 4 Leerzeichen
- Layout-Maße als `constexpr int16_t` am Anfang des jeweiligen Abschnitts
- Nach dem Zeichnen `display.hibernate()`, der Bildinhalt bleibt dabei sichtbar

## Bauen und flashen

    cd xiao_epaper_test
    pio run              # kompilieren
    pio run -t upload    # flashen
    pio device monitor   # serielle Ausgabe, 115200 Baud

## Git

- Remote: https://github.com/Fiasmo/Epaper_Dashboard (Branch `master`)
- `.gitignore` schließt `.pio/` und `*_backup_*.zip` aus
