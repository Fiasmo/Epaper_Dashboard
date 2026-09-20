# Bildschirm-Designs

Alle Bilder sind **nicht gemalt**, sondern aus dem echten Zeichencode in
`xiao_epaper_test/src/main.cpp` erzeugt (PC-Simulator, siehe
`tools/simulator/README.md`). Sie zeigen also exakt, was das 250 x 122 Pixel
große E-Paper-Display zeigt, inklusive der echten Schriften.

Neu erzeugen: `powershell -File tools\simulator\simulate.ps1`

## Ablauf beim Start

| Nr. | Bildschirm | Normalfall | Fehlerfall |
| --- | --- | --- | --- |
| 1 | Startbild | ![](renders/01_startbild.png) | — |
| 2 | Grafik-/Kontrasttest (Endbild der Animation) | ![](renders/02_grafiktest.png) | — |
| 3 | Akku-Prüfung (1/3) | ![](renders/03_akku_ok.png) | ![](renders/03_akku_fehler.png) |
| 4 | Sensor-Prüfung (2/3) | ![](renders/04_sensoren_ok.png) | Luftfeuchtigkeit fällt aus: ![](renders/04_sensoren_fehler_luftfeuchte.png) |
| 4 | Sensor-Prüfung, beide Sensoren fallen aus | — | ![](renders/04_sensoren_fehler_beide.png) |
| 5 | WLAN-Prüfung (3/3) | ![](renders/05_wlan_ok.png) | ![](renders/05_wlan_fehler.png) |
| 6 | Zusammenfassung | "Test erfolgreich": ![](renders/06_test_erfolgreich.png) | "Test mit Fehlern": ![](renders/06_test_mit_fehlern.png) |
| 7 | Status-Dashboard | ![](renders/07_dashboard_ok.png) | Akku, Luftfeuchtigkeit, WLAN fallen aus: ![](renders/07_dashboard_fehler.png) |
| 7 | Status-Dashboard, alle Messungen fallen aus | — | ![](renders/07_dashboard_alle_fehler.png) |
| 8 | Optionen (unverändert) | ![](renders/08_optionen.png) | — |

## Gestaltungsregeln, die im Gespräch festgelegt wurden

- Dashboard: große Box links (oben Temperatur, unten Luftfeuchtigkeit), rechts
  zwei Statusfelder (Verbindung, Display). Keine Fußzeile mehr.
- Die Trennlinie zwischen Temperatur und Luftfeuchtigkeit läuft **durchgehend
  von Rand zu Rand**.
- Die Prüfbilder (Akku, Sensoren, WLAN) und die Zusammenfassung teilen sich
  Rahmen und Titelbalken mit dem Startbild.
- Erfolg = Haken (Umriss oder schwarzer Kreis), Fehler = schwarzer Kreis mit
  Kreuz bzw. **invertierte** (schwarze) Fläche mit weißer Schrift.
- Fehlender Messwert = `--` (Temperatur `--.-`), Umriss-Symbol statt gefülltem
  Symbol, im Akku ein `?`, durchgestrichenes WLAN-Symbol, Warndreieck in der
  Kopfzeile des Dashboards.
