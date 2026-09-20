# Rendert alle Bildschirme aus xiao_epaper_test/src/main.cpp am PC und schreibt
# PNGs nach docs/design/renders. Voraussetzungen: Visual Studio mit C++-Werkzeugen,
# Python 3 und einmal "pio run" (holt die Adafruit-GFX-Bibliothek).
$ErrorActionPreference = "Stop"

$root = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$source = Join-Path $root "xiao_epaper_test\src\main.cpp"
$gfx = Join-Path $root "xiao_epaper_test\.pio\libdeps\seeed_xiao_esp32s3\Adafruit GFX Library"
$build = Join-Path $PSScriptRoot "build"

if (-not (Test-Path $gfx)) {
    throw "Adafruit GFX fehlt. Erst 'pio run' im Ordner xiao_epaper_test ausfuehren."
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    throw "Visual Studio mit C++-Werkzeugen nicht gefunden."
}
$vcvars = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"

# Varianten: ok = alles gut, err = Akku, Luftfeuchtigkeit und WLAN fallen aus,
# errall = zusaetzlich faellt der Temperatursensor aus.
$variants = [ordered]@{
    ok     = @()
    err    = @("BATTERY", "HUMIDITY", "WIFI")
    errall = @("BATTERY", "TEMPERATURE", "HUMIDITY", "WIFI")
}

foreach ($name in $variants.Keys) {
    $dir = Join-Path $build $name
    New-Item -ItemType Directory -Force (Join-Path $dir "obj") | Out-Null
    Get-ChildItem $dir -Filter "frame_*.txt" -ErrorAction SilentlyContinue | Remove-Item

    $text = [System.IO.File]::ReadAllText($source)
    foreach ($flag in $variants[$name]) {
        $text = $text.Replace("SIMULATE_${flag}_FAILURE = false", "SIMULATE_${flag}_FAILURE = true")
    }
    [System.IO.File]::WriteAllText((Join-Path $dir "main.cpp"), $text)

    $compile = "cd /d `"$dir`" && call `"$vcvars`" >nul && cl /nologo /EHsc /std:c++17 /utf-8 /W1 " +
        "/I `"$PSScriptRoot`" /I `"$gfx`" /DARDUINO=100 /Fo:obj\ /Fe:sim.exe " +
        "`"$PSScriptRoot\harness.cpp`" main.cpp `"$gfx\Adafruit_GFX.cpp`""
    cmd /c $compile | Out-Null

    $exe = Join-Path $dir "sim.exe"
    if (-not (Test-Path $exe)) {
        throw "Build der Variante '$name' fehlgeschlagen."
    }

    & $exe $dir | Out-Null
    Write-Output "Variante '$name' gerendert."
}

python (Join-Path $PSScriptRoot "render.py")
