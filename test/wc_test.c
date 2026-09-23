#include <stdio.h>
#include "wordclock.h"

int main(void) {
  bool on[WC_ROWS][WC_COLS];
  for (int lang = WC_LANG_DE; lang <= WC_LANG_EN; lang++) {
    const WcRow *grid = wc_grid(lang);
    for (int h = 0; h < 24; h++) {
      for (int m = 0; m < 60; m += 5) {
        wc_compute(lang, h, m, on);
        printf("%02d:%02d  ", h, m);
        for (int r = 0; r < WC_ROWS; r++) {
          bool prev = false;
          for (int c = 0; c < WC_COLS; c++) {
            if (on[r][c]) { fputs(grid[r][c], stdout); prev = true; }
            else if (prev) { fputc(' ', stdout); prev = false; }
          }
          if (prev) fputc(' ', stdout);
        }
        fputc('\n', stdout);
      }
    }
  }
  return 0;
}
