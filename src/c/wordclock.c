#include "wordclock.h"

#include <string.h>

const char * const wc_grid[WC_ROWS][WC_COLS] = {
  {"E","S","K","I","S","T","A","F","Ü","N","F"},
  {"Z","E","H","N","Z","W","A","N","Z","I","G"},
  {"D","R","E","I","V","I","E","R","T","E","L"},
  {"V","O","R","F","U","N","K","N","A","C","H"},
  {"H","A","L","B","A","E","L","F","Ü","N","F"},
  {"E","I","N","S","X","A","M","Z","W","E","I"},
  {"D","R","E","I","P","M","J","V","I","E","R"},
  {"S","E","C","H","S","N","L","A","C","H","T"},
  {"S","I","E","B","E","N","Z","W","Ö","L","F"},
  {"Z","E","H","N","E","U","N","K","U","H","R"},
};

// Markiert ein Wort: Zeile, Start- und Endspalte (beide inklusive).
static void light(bool on[WC_ROWS][WC_COLS], int row, int from, int to) {
  for (int c = from; c <= to; c++) {
    on[row][c] = true;
  }
}

// Stunde als 0..11, wobei 0 = zwölf.
// full_hour unterscheidet "ES IST EIN UHR" von "... NACH EINS".
static void light_hour(bool on[WC_ROWS][WC_COLS], int hour12, bool full_hour) {
  switch (hour12) {
    case 0:  light(on, 8, 6, 10); break;                    // ZWÖLF
    case 1:  light(on, 5, 0, full_hour ? 2 : 3); break;     // EIN / EINS
    case 2:  light(on, 5, 7, 10); break;                    // ZWEI
    case 3:  light(on, 6, 0, 3); break;                     // DREI
    case 4:  light(on, 6, 7, 10); break;                    // VIER
    case 5:  light(on, 4, 7, 10); break;                    // FÜNF
    case 6:  light(on, 7, 0, 4); break;                     // SECHS
    case 7:  light(on, 8, 0, 5); break;                     // SIEBEN
    case 8:  light(on, 7, 7, 10); break;                    // ACHT
    case 9:  light(on, 9, 3, 6); break;                     // NEUN
    case 10: light(on, 9, 0, 3); break;                     // ZEHN
    case 11: light(on, 4, 5, 7); break;                     // ELF
  }
}

void wc_compute(int hour, int minute, bool on[WC_ROWS][WC_COLS]) {
  memset(on, 0, sizeof(bool) * WC_ROWS * WC_COLS);

  int block = (minute / 5) * 5;
  int next_hour = 0;

  light(on, 0, 0, 1);   // ES
  light(on, 0, 3, 5);   // IST

  switch (block) {
    case 0:
      break;
    case 5:
      light(on, 0, 7, 10); light(on, 3, 7, 10);                     // FÜNF NACH
      break;
    case 10:
      light(on, 1, 0, 3);  light(on, 3, 7, 10);                     // ZEHN NACH
      break;
    case 15:
      light(on, 2, 4, 10); light(on, 3, 7, 10);                     // VIERTEL NACH
      break;
    case 20:
      light(on, 1, 4, 10); light(on, 3, 7, 10);                     // ZWANZIG NACH
      break;
    case 25:
      light(on, 0, 7, 10); light(on, 3, 0, 2); light(on, 4, 0, 3);  // FÜNF VOR HALB
      next_hour = 1;
      break;
    case 30:
      light(on, 4, 0, 3);                                           // HALB
      next_hour = 1;
      break;
    case 35:
      light(on, 0, 7, 10); light(on, 3, 7, 10); light(on, 4, 0, 3); // FÜNF NACH HALB
      next_hour = 1;
      break;
    case 40:
      light(on, 1, 4, 10); light(on, 3, 0, 2);                      // ZWANZIG VOR
      next_hour = 1;
      break;
    case 45:
      light(on, 2, 0, 10);                                          // DREIVIERTEL
      next_hour = 1;
      break;
    case 50:
      light(on, 1, 0, 3);  light(on, 3, 0, 2);                      // ZEHN VOR
      next_hour = 1;
      break;
    case 55:
      light(on, 0, 7, 10); light(on, 3, 0, 2);                      // FÜNF VOR
      next_hour = 1;
      break;
  }

  bool full_hour = (block == 0);
  light_hour(on, (hour + next_hour) % 12, full_hour);
  if (full_hour) {
    light(on, 9, 8, 10);                                            // UHR
  }
}
