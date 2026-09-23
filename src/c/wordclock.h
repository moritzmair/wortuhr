#pragma once

#include <stdbool.h>

#define WC_COLS 11
#define WC_ROWS 9

// Deutsches Wortuhr-Raster (QLOCKTWO-Layout, ohne die "ES IST"-Zeile).
// Jede Zelle ist ein eigener String, damit Umlaute (2 Byte in UTF-8) sauber bleiben.
extern const char * const wc_grid[WC_ROWS][WC_COLS];

// Optionale Kopfzeile "ES IST" im selben Raster; die übrigen Buchstaben sind Füller.
extern const char * const wc_es_ist_row[WC_COLS];
#define WC_ES_FROM 0
#define WC_ES_TO 1
#define WC_IST_FROM 3
#define WC_IST_TO 5

// Setzt in `on` genau die Buchstaben auf true, die den Satz zur Zeit hour:minute bilden.
void wc_compute(int hour, int minute, bool on[WC_ROWS][WC_COLS]);
