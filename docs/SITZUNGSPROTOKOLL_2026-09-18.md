# Sitzungsprotokoll — 18.09.2026

Einrichtung der Git-Integration für das E-Paper-Projekt (XIAO ESP32-S3).

## Ausgangslage

Der Ordner mit dem C-Programm schien nach dem letzten Öffnen verschwunden zu
sein. Tatsächlich war nichts gelöscht: Ordnerfreigaben in der Claude-App gelten
**pro Chat**, nicht global. Ein neuer Chat startet ohne Ordnerzugriff, auch
innerhalb desselben Projekts. Der Ordner muss dort über den Button
"Ordner hinzufügen" erneut freigegeben werden. Die Dateien auf der Platte sind
davon nie betroffen.

Gefunden wurde der Ordner unter:
`C:\Users\Fiasmo\Development\ESP32\Epaper_Claude`

## Was eingerichtet wurde

- GitHub CLI (`gh`) in der Sitzungsumgebung installiert und per Gerätecode-Flow
  am Konto **Fiasmo** angemeldet (Scopes: repo, read:org, gist, workflow).
- Git-Identität gesetzt: `Fiasmo` / `Fiasmo.mr@gmail.com`, Default-Branch `main`.
- Der bis dahin nur aus einer ZIP entpackte Ordner `Epaper_Dashboard-master`
  wurde zu einem echten Git-Repository gemacht und mit
  `https://github.com/Fiasmo/Epaper_Dashboard` (Branch `master`) verbunden.
- Vorher geprüft: Der lokale Inhalt war **byte-identisch** mit dem Remote-Stand
  (Commit `94c150c`). Es wurde nichts überschrieben und nichts ging verloren.
- Löschrecht für den Ordner erteilt, damit Git seine `.lock`- und
  temporären Dateien wieder aufräumen kann. Ohne das schlägt `git commit` fehl.
- Überflüssige `Epaper_Dashboard-master.zip` gelöscht.
- Ordner umbenannt: `Epaper_Dashboard-master` → `Epaper_Dashboard`.
- `CLAUDE.md` im Repo angelegt (Projektkontext für Claude Code) und lokal
  committet als `f39c4da`.
- Push und Pull getestet, beides funktioniert.

## Offener Punkt

Der Commit `f39c4da` ("Projektkontext für Claude Code ergänzen") liegt bisher
**nur lokal**. Zum Hochladen:

    cd C:\Users\Fiasmo\Development\ESP32\Epaper_Claude\Epaper_Dashboard
    git push

## Was dauerhaft bleibt und was nicht

**Bleibt dauerhaft:** Das `.git`-Verzeichnis im Projektordner. Die Verbindung
zum GitHub-Repo besteht damit unabhängig von Claude weiter, auch für Git unter
Windows, VS Code oder GitHub Desktop.

**Bleibt nicht:** Der GitHub-Login der Claude-Sitzungsumgebung und die dortige
`gh`-Installation. Beides liegt in der Arbeitsumgebung dieser Sitzung und ist in
einem neuen Chat voraussichtlich weg. Ebenso die Ordnerfreigabe.

## Setup in einer neuen Claude-Sitzung wiederherstellen

1. Ordner über "Ordner hinzufügen" freigeben:
   `C:\Users\Fiasmo\Development\ESP32\Epaper_Claude`
2. Falls Git-Operationen nötig sind, das Löschrecht für diesen Ordner erteilen
   (sonst scheitern Commits an liegengebliebenen `.lock`-Dateien).
3. GitHub CLI erneut installieren und anmelden, oder unter Windows direkt mit
   GitHub Desktop bzw. dem Git Credential Manager arbeiten.

Die Repo-Verbindung selbst muss **nicht** neu eingerichtet werden.

## Projekt-Eckdaten

- Board: Seeed Studio XIAO ESP32-S3 (Sense) mit Seeed ePaper Driver Board
- Display: GDEY0213B74, 2,13 Zoll, 122 x 250 Pixel, SSD1680
- Pins: CS = D1, DC = D3, RST = D0, BUSY = D2
- Aktives Projekt: `Epaper_Dashboard/xiao_epaper_test/` (PlatformIO)
- Bibliotheken: GxEPD2, Adafruit GFX
- Details siehe `Epaper_Dashboard/CLAUDE.md`

## Claude Code im Terminal

    irm https://claude.ai/install.ps1 | iex     # Installation in PowerShell
    cd C:\Users\Fiasmo\Development\ESP32\Epaper_Claude\Epaper_Dashboard
    claude

Claude Code liest die `CLAUDE.md` im Ordner automatisch und kennt damit den
Projektkontext. Der Chatverlauf selbst lässt sich nicht übertragen.
