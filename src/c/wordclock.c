#include "wordclock.h"

#include <string.h>

static const WcRow s_grid_de[WC_ROWS] = {
  {"Z","E","H","N","Z","W","A","N","Z","I","G"},
  {"F","Ü","N","F","V","I","E","R","T","E","L"},
  {"V","O","R","F","U","N","K","N","A","C","H"},
  {"H","A","L","B","A","E","L","F","Ü","N","F"},
  {"E","I","N","S","X","A","M","Z","W","E","I"},
  {"D","R","E","I","P","M","J","V","I","E","R"},
  {"S","E","C","H","S","N","L","A","C","H","T"},
  {"S","I","E","B","E","N","Z","W","Ö","L","F"},
  {"Z","E","H","N","E","U","N","K","U","H","R"},
};

static const WcRow s_grid_en[WC_ROWS] = {
  {"A","C","Q","U","A","R","T","E","R","D","C"},
  {"T","W","E","N","T","Y","F","I","V","E","X"},
  {"H","A","L","F","S","T","E","N","F","T","O"},
  {"P","A","S","T","E","R","U","N","I","N","E"},
  {"O","N","E","S","I","X","T","H","R","E","E"},
  {"F","O","U","R","F","I","V","E","T","W","O"},
  {"E","I","G","H","T","E","L","E","V","E","N"},
  {"S","E","V","E","N","T","W","E","L","V","E"},
  {"T","E","N","S","E","O","C","L","O","C","K"},
};

static const char * const s_header_de[WC_COLS] =
  {"E","S","K","I","S","T","L","A","U","N","E"};
static const char * const s_header_en[WC_COLS] =
  {"I","T","L","I","S","A","S","A","M","P","M"};

const WcRow *wc_grid(WcLang lang) {
  return lang == WC_LANG_EN ? s_grid_en : s_grid_de;
}

const char * const *wc_header(WcLang lang) {
  return lang == WC_LANG_EN ? s_header_en : s_header_de;
}

void wc_header_on(WcLang lang, bool on[WC_COLS]) {
  for (int c = 0; c < WC_COLS; c++) {
    on[c] = lang == WC_LANG_EN
        ? (c <= 1 || (c >= 3 && c <= 4))    // IT IS
        : (c <= 1 || (c >= 3 && c <= 5));   // ES IST
  }
}

// Markiert ein Wort: Zeile, Start- und Endspalte (beide inklusive).
static void light(bool on[WC_ROWS][WC_COLS], int row, int from, int to) {
  for (int c = from; c <= to; c++) {
    on[row][c] = true;
  }
}

// Stunde als 0..11, wobei 0 = zwölf.
// full_hour unterscheidet "EIN UHR" von "... NACH EINS".
static void light_hour_de(bool on[WC_ROWS][WC_COLS], int hour12, bool full_hour) {
  switch (hour12) {
    case 0:  light(on, 7, 6, 10); break;                    // ZWÖLF
    case 1:  light(on, 4, 0, full_hour ? 2 : 3); break;     // EIN / EINS
    case 2:  light(on, 4, 7, 10); break;                    // ZWEI
    case 3:  light(on, 5, 0, 3); break;                     // DREI
    case 4:  light(on, 5, 7, 10); break;                    // VIER
    case 5:  light(on, 3, 7, 10); break;                    // FÜNF
    case 6:  light(on, 6, 0, 4); break;                     // SECHS
    case 7:  light(on, 7, 0, 5); break;                     // SIEBEN
    case 8:  light(on, 6, 7, 10); break;                    // ACHT
    case 9:  light(on, 8, 3, 6); break;                     // NEUN
    case 10: light(on, 8, 0, 3); break;                     // ZEHN
    case 11: light(on, 3, 5, 7); break;                     // ELF
  }
}

static void compute_de(int hour, int minute, bool on[WC_ROWS][WC_COLS]) {
  int block = (minute / 5) * 5;
  int next_hour = 0;

  switch (block) {
    case 0:
      break;
    case 5:
      light(on, 1, 0, 3);  light(on, 2, 7, 10);                     // FÜNF NACH
      break;
    case 10:
      light(on, 0, 0, 3);  light(on, 2, 7, 10);                     // ZEHN NACH
      break;
    case 15:
      light(on, 1, 4, 10); light(on, 2, 7, 10);                     // VIERTEL NACH
      break;
    case 20:
      light(on, 0, 4, 10); light(on, 2, 7, 10);                     // ZWANZIG NACH
      break;
    case 25:
      light(on, 1, 0, 3);  light(on, 2, 0, 2); light(on, 3, 0, 3);  // FÜNF VOR HALB
      next_hour = 1;
      break;
    case 30:
      light(on, 3, 0, 3);                                           // HALB
      next_hour = 1;
      break;
    case 35:
      light(on, 1, 0, 3);  light(on, 2, 7, 10); light(on, 3, 0, 3); // FÜNF NACH HALB
      next_hour = 1;
      break;
    case 40:
      light(on, 0, 4, 10); light(on, 2, 0, 2);                      // ZWANZIG VOR
      next_hour = 1;
      break;
    case 45:
      light(on, 1, 4, 10); light(on, 2, 0, 2);                      // VIERTEL VOR
      next_hour = 1;
      break;
    case 50:
      light(on, 0, 0, 3);  light(on, 2, 0, 2);                      // ZEHN VOR
      next_hour = 1;
      break;
    case 55:
      light(on, 1, 0, 3);  light(on, 2, 0, 2);                      // FÜNF VOR
      next_hour = 1;
      break;
  }

  bool full_hour = (block == 0);
  light_hour_de(on, (hour + next_hour) % 12, full_hour);
  if (full_hour) {
    light(on, 8, 8, 10);                                            // UHR
  }
}

static void light_hour_en(bool on[WC_ROWS][WC_COLS], int hour12) {
  switch (hour12) {
    case 0:  light(on, 7, 5, 10); break;                    // TWELVE
    case 1:  light(on, 4, 0, 2); break;                     // ONE
    case 2:  light(on, 5, 8, 10); break;                    // TWO
    case 3:  light(on, 4, 6, 10); break;                    // THREE
    case 4:  light(on, 5, 0, 3); break;                     // FOUR
    case 5:  light(on, 5, 4, 7); break;                     // FIVE
    case 6:  light(on, 4, 3, 5); break;                     // SIX
    case 7:  light(on, 7, 0, 4); break;                     // SEVEN
    case 8:  light(on, 6, 0, 4); break;                     // EIGHT
    case 9:  light(on, 3, 7, 10); break;                    // NINE
    case 10: light(on, 8, 0, 2); break;                     // TEN
    case 11: light(on, 6, 5, 10); break;                    // ELEVEN
  }
}

static void compute_en(int hour, int minute, bool on[WC_ROWS][WC_COLS]) {
  int block = (minute / 5) * 5;
  // Ab :35 zählt die Zeit auf die nächste Stunde zu ("TWENTY FIVE TO").
  int mins = block <= 30 ? block : 60 - block;

  switch (mins) {
    case 5:  light(on, 1, 6, 9); break;                     // FIVE
    case 10: light(on, 2, 5, 7); break;                     // TEN
    case 15: light(on, 0, 2, 8); break;                     // QUARTER
    case 20: light(on, 1, 0, 5); break;                     // TWENTY
    case 25: light(on, 1, 0, 9); break;                     // TWENTY FIVE
    case 30: light(on, 2, 0, 3); break;                     // HALF
  }

  if (block == 0) {
    light(on, 8, 5, 10);                                    // OCLOCK
  } else if (block <= 30) {
    light(on, 3, 0, 3);                                     // PAST
  } else {
    light(on, 2, 9, 10);                                    // TO
  }

  light_hour_en(on, (hour + (block > 30 ? 1 : 0)) % 12);
}

void wc_compute(WcLang lang, int hour, int minute, bool on[WC_ROWS][WC_COLS]) {
  memset(on, 0, sizeof(bool) * WC_ROWS * WC_COLS);
  if (lang == WC_LANG_EN) {
    compute_en(hour, minute, on);
  } else {
    compute_de(hour, minute, on);
  }
}
