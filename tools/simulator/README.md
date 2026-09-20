# PC-Simulator für die Displaybilder

Übersetzt `xiao_epaper_test/src/main.cpp` unverändert für den PC (mit der echten
Adafruit-GFX-Bibliothek und ihren Schriften) und speichert jedes gezeichnete
Bild. So lassen sich Layouts prüfen, ohne zu flashen.

## Voraussetzungen

- Windows mit Visual Studio (Workload "Desktopentwicklung mit C++")
- Python 3
- Einmal `pio run` im Ordner `xiao_epaper_test`, damit die Bibliotheken
  heruntergeladen werden

## Benutzung

    powershell -File tools\simulator\simulate.ps1

Das Skript baut drei Varianten von `main.cpp`, indem es die Schalter
`SIMULATE_*_FAILURE` umstellt:

- `ok`: alles funktioniert
- `err`: Akku, Luftfeuchtigkeit und WLAN fallen aus (Temperatur funktioniert)
- `errall`: zusätzlich fällt der Temperatursensor aus

Die PNGs landen in `docs/design/renders/`. Die Zuordnung Bild zu Datei steht in
`render.py` (Liste `SCREENS`, Positionen zählen vom Ende der Bildfolge). Kommt
ein Bildschirm hinzu oder fällt einer weg, muss diese Liste angepasst werden.

## Dateien

- `Arduino.h`, `Print.h`, `GxEPD2_BW.h`: schlanke Ersatz-Header, die nur das
  bereitstellen, was `main.cpp` braucht. `GxEPD2_BW.h` hält statt eines echten
  Displays einen Pixelspeicher und schreibt ihn bei `nextPage()` als Textdatei.
- `harness.cpp`: `main()` ruft `setup()` auf.
- `render.py`: Textdateien zu PNGs (nur Python-Standardbibliothek).
- `build/` entsteht beim Lauf und ist von Git ausgeschlossen.

Der Simulator ist auf MSVC zugeschnitten (`_byteswap_ushort` in `Arduino.h`).
