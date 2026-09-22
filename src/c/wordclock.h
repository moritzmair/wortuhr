#pragma once

#include <stdbool.h>

#define WC_COLS 11
#define WC_ROWS 10

// Deutsches Wortuhr-Raster (QLOCKTWO-Layout).
// Jede Zelle ist ein eigener String, damit Umlaute (2 Byte in UTF-8) sauber bleiben.
extern const char * const wc_grid[WC_ROWS][WC_COLS];

// Setzt in `on` genau die Buchstaben auf true, die den Satz zur Zeit hour:minute bilden.
void wc_compute(int hour, int minute, bool on[WC_ROWS][WC_COLS]);
