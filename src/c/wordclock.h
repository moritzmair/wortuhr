#pragma once

#include <stdbool.h>

#define WC_COLS 11
#define WC_ROWS 9

typedef enum {
  WC_LANG_DE = 0,
  WC_LANG_EN = 1,
} WcLang;

typedef const char * const WcRow[WC_COLS];

// Buchstabenraster (QLOCKTWO-Layout ohne die "ES IST"/"IT IS"-Zeile).
// Jede Zelle ist ein eigener String, damit Umlaute (2 Byte in UTF-8) sauber bleiben.
const WcRow *wc_grid(WcLang lang);

// Optionale Kopfzeile "ES IST" bzw. "IT IS"; die übrigen Buchstaben sind Füller.
const char * const *wc_header(WcLang lang);
void wc_header_on(WcLang lang, bool on[WC_COLS]);

// Setzt in `on` genau die Buchstaben auf true, die den Satz zur Zeit hour:minute bilden.
void wc_compute(WcLang lang, int hour, int minute, bool on[WC_ROWS][WC_COLS]);
