# Wortuhr

Ein deutsches Wortuhr-Watchface (QLOCKTWO-Stil) für Pebble. Statt Ziffern leuchten
im 11×10-Buchstabenraster genau die Wörter auf, die die aktuelle Zeit als Satz ergeben:

    18:43  ->  ES IST ZWANZIG VOR SIEBEN

## Raster

    E S K I S T A F Ü N F
    Z E H N Z W A N Z I G
    D R E I V I E R T E L
    V O R F U N K N A C H
    H A L B A E L F Ü N F
    E I N S X A M Z W E I
    D R E I P M J V I E R
    S E C H S N L A C H T
    S I E B E N Z W Ö L F
    Z E H N E U N K U H R

Die Zeit wird auf 5 Minuten abgerundet. Bei :45 wird die süddeutsche Form
"DREIVIERTEL <Stunde>" benutzt, bei :15 "VIERTEL NACH <Stunde>".

## Dateien

- `src/c/wordclock.h` / `wordclock.c` — Die Wort-Logik: welches Feld leuchtet wann.
  Hängt bewusst nicht von `pebble.h` ab, damit sie sich nativ testen lässt.
- `src/c/wortuhr.c` — Pebble-App: Fenster, Font, Rendering, Minuten-Tick.

## Bauen & testen

    pebble build
    pebble install --emulator emery     # emery = Pebble Time 2
    pebble screenshot --emulator emery shot.png

Andere Plattformen: `basalt` (Pebble Time), `diorite` (Pebble 2 / Pebble 2 Duo),
`chalk` (Pebble Time Round), `aplite` (Pebble Classic).

Logik gegen alle 288 Fünf-Minuten-Schritte prüfen:

    cc -std=c11 -Wall -Wextra -I src/c -o /tmp/wc_test test/wc_test.c src/c/wordclock.c
    /tmp/wc_test

## Auf die echte Uhr

`pebble build` legt `build/wortuhr.pbw` an. Entweder über die Developer Connection
der Pebble-App installieren:

    pebble install --phone <IP-der-Pebble-App>

oder die `.pbw` direkt aufs Handy schieben und in der Pebble-App öffnen.
