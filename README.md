# Wortuhr

Ein Wortuhr-Watchface (QLOCKTWO-Stil) für Pebble, auf Deutsch oder Englisch. Statt
Ziffern leuchten im 11×9-Buchstabenraster genau die Wörter auf, die die aktuelle Zeit
als Satz ergeben:

    18:43  ->  ZWANZIG VOR SIEBEN
    18:43  ->  TWENTY TO SEVEN

## Raster

Deutsch:

    Z E H N Z W A N Z I G
    F Ü N F V I E R T E L
    V O R F U N K N A C H
    H A L B A E L F Ü N F
    E I N S X A M Z W E I
    D R E I P M J V I E R
    S E C H S N L A C H T
    S I E B E N Z W Ö L F
    Z E H N E U N K U H R

Englisch:

    A C Q U A R T E R D C
    T W E N T Y F I V E X
    H A L F S T E N F T O
    P A S T E R U N I N E
    O N E S I X T H R E E
    F O U R F I V E T W O
    E I G H T E L E V E N
    S E V E N T W E L V E
    T E N S E O C L O C K

Die Zeit wird auf 5 Minuten abgerundet. Bei :15 heißt es "VIERTEL NACH <Stunde>" bzw.
"QUARTER PAST <hour>", bei :45 "VIERTEL VOR <Stunde>" bzw. "QUARTER TO <hour>".

Die optionale Kopfzeile ist `ESKISTLAUNE` ("ES IST") bzw. `ITLISASAMPM` ("IT IS").

## Einstellungen

Über die Pebble-App (Zahnrad am Watchface) lassen sich einstellen:

- Sprache: Deutsch oder Englisch. Ohne Einstellung gilt die Sprache der Uhr; auch die
  Einstellungsseite selbst erscheint in der gewählten Sprache.
- Farben für Hintergrund, Text und hervorgehobenen Text
- Optionale Kopfzeile "ES IST" / "IT IS" im Raster-Stil
- Infozeile über dem Raster mit Datum und/oder Außentemperatur (°C oder °F),
  wahlweise im Buchstabenraster (ein Zeichen pro Feld) oder als Textzeile.
  Datum auf Deutsch als `MI 23.9`, auf Englisch als `WE 9/23`.
  Die Textzeile ist ein eigener Balken mit Trennlinie, standardmäßig invertiert zum
  Raster; Hintergrund, Text und Linie lassen sich auch einzeln einstellen.
  Ist alles aus, verteilt sich das Raster auf die volle Höhe.

Die Temperatur holt das Handy alle 30 Minuten über seinen Standort von
[open-meteo.com](https://open-meteo.com) (kein API-Key nötig).

## Dateien

- `src/c/wordclock.h` / `wordclock.c` — Die Wort-Logik: welches Feld leuchtet wann.
  Hängt bewusst nicht von `pebble.h` ab, damit sie sich nativ testen lässt.
- `src/c/wortuhr.c` — Pebble-App: Fenster, Font, Rendering, Minuten-Tick, Einstellungen.
- `src/pkjs/index.js` — Handy-Seite: Einstellungsseite und Wetterabfrage.
- `src/pkjs/config.js` — Aufbau der Einstellungsseite, auf Deutsch und Englisch.
- `src/pkjs/config-custom.js` — Blendet auf der Einstellungsseite die Textzeilen-Farben
  nur ein, wenn sie gebraucht werden.
- `src/pkjs/clay.js` — [Clay](https://github.com/pebble/clay) 1.0.4, eingebettet statt per npm,
  weil das npm-Paket flint nicht als Plattform kennt (und dort den Build abbricht).
  Angepasst: flint bekommt wie aplite/diorite den Schwarzweiß-Farbwähler.

## Bauen & testen

    pebble build
    pebble install --emulator emery     # emery = Pebble Time 2
    pebble screenshot --emulator emery shot.png

Andere Plattformen: `basalt` (Pebble Time), `diorite` (Pebble 2), `flint` (Pebble 2 Duo),
`aplite` (Pebble Classic). Runde Uhren (`chalk`, `gabbro`) werden bewusst nicht
unterstützt – das rechteckige Buchstabenraster passt nicht ins runde Display.

Vorschaubilder für den Store (landen in `store/<plattform>/`):

    python3 scripts/store_screenshots.py

Logik gegen alle 288 Fünf-Minuten-Schritte pro Sprache prüfen:

    cc -std=c11 -Wall -Wextra -I src/c -o /tmp/wc_test test/wc_test.c src/c/wordclock.c
    /tmp/wc_test

## Auf die echte Uhr

Jeder Push auf `main` baut per GitHub Actions die `.pbw` und legt sie hier ab:

    https://github.com/moritzmair/wortuhr/releases/latest/download/wortuhr.pbw

Den Link auf dem Handy öffnen, die Datei mit der Pebble-App öffnen, fertig.

Lokal geht es auch:

`pebble build` legt `build/wortuhr.pbw` an. Entweder über die Developer Connection
der Pebble-App installieren:

    pebble install --phone <IP-der-Pebble-App>

oder die `.pbw` direkt aufs Handy schieben und in der Pebble-App öffnen.
