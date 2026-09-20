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
  Hauptcode: `xiao_epaper_test/src/main.cpp` (~1350 Zeilen)
- `docs/design/` — alle Bildschirm-Designs als PNG (Normal- und Fehlerfälle) mit
  Übersicht in `docs/design/README.md`
- `docs/SITZUNGSPROTOKOLL_*.md` — Protokolle der Claude-Sitzungen mit
  Entscheidungen und offenen Punkten; neueste zuerst lesen
- `tools/simulator/` — PC-Simulator, der die Bilder aus `main.cpp` rendert
- `Epaper_Test/Epaper_Test.ino` — erster einfacher Arduino-IDE-Sketch (Text + Smiley)
- `Epaper_Test.ino/Epaper_Test.ino.ino` — leerer Stub, kann ignoriert werden
- `xiao_epaper_test_platformio.zip` — Backup-Archiv

## Bibliotheken

- `zinggjm/GxEPD2` für die E-Paper-Ansteuerung
- `adafruit/Adafruit GFX Library` für Grafikprimitive und die Fonts
  FreeMonoBold 9pt / 12pt / 18pt

## Aufbau von main.cpp

`setup()` zeigt nacheinander acht Bildschirme und legt das Display danach schlafen:

1. `drawSplashScreen()` — Startbild
2. `playGraphicsTestAnimation()` mit `drawGraphicsTest(step)` — Grafiktest in Schritten
3. `drawBatteryCheckScreen()` — Akku-Prüfung (1/3), davor `readBattery()`
4. `drawSensorCheckScreen()` — Sensor-Prüfung (2/3), davor `readSensors()`
5. `drawWifiCheckScreen()` — WLAN-Prüfung (3/3), davor `connectWifi()`
6. `drawSummaryScreen()` — "Test erfolgreich" oder "Test mit Fehlern"
7. `drawDashboard()` — Temperatur, Luftfeuchtigkeit, Verbindung, Display
8. `drawOptionsScreen()` mit `drawOptionRow(...)` — Optionsliste (unverändert)

`loop()` macht bewusst nichts. Ein E-Paper soll nicht dauernd neu gezeichnet
werden, für einen erneuten Durchlauf RESET drücken.

### Fehlerfälle

Die Messwerte liegen in den globalen Structs `battery`, `sensors` und
`wifiState`, jeweils mit `ok`-Flag. `readBattery()`, `readSensors()` und
`connectWifi()` liefern derzeit nur Beispielwerte; die echten Messungen sind mit
`TODO` markiert, weil Verdrahtung und Sensortypen dem Programm nicht bekannt sind.
Jeder Prüfbildschirm hat eine Fehlervariante, die Zusammenfassung zählt die
fehlgeschlagenen Prüfungen (`countFailedChecks()`), und das Dashboard zeigt
Ausfälle an (Warndreieck, durchgestrichenes WLAN, `?` im Akku, `--`, Marke
FEHLER, invertierte Box OFFLINE). Mit den Schaltern `SIMULATE_*_FAILURE` im
Abschnitt "Messwerte und Selbsttest" lässt sich jeder Ausfall ohne defekte
Hardware testen. Fehlgeschlagene Messungen werden nicht wiederholt.

### Hilfsfunktionen

`showScreen(drawFunction, pauseMilliseconds)` kapselt den Vollbild-Refresh-Zyklus
aus `setFullWindow` / `firstPage` / `nextPage`. Text: `drawCenteredText`,
`drawFontText` (GFX-Schriften) sowie `drawSmallText` und `drawSmallBoldText`
(Standardschrift, y ist die Grundlinie). Symbole: `drawWifiFan`,
`drawThermometerIcon`, `drawDropletIcon`, `drawMonitorIcon`, `drawGearIconBold`,
`drawHeaderBatteryIcon`, `drawWarningIcon`, `drawStatusBadge` und `drawSlash`
(Durchstreichen). `drawGearIcon` wird nur noch vom Optionen-Bildschirm benutzt.

## Vorschau am PC

`tools/simulator/simulate.ps1` rendert alle Bilder aus `main.cpp`, auch die
Fehlervarianten, nach `docs/design/renders/`. Nach Layoutänderungen ausführen und
die PNGs ansehen, bevor geflasht wird. Voraussetzungen stehen in
`tools/simulator/README.md`.

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
