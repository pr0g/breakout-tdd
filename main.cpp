#include <iostream>
#include <ncurses.h>

#include "breakout.h"

struct display_console_t : public display_t {
  void output(int x, int y) override { mvprintw(y, x, "*"); }
};

int main(int argc, char** argv) {
  initscr(); // start curses mode
  curs_set(0); // hide cursor

  breakout_t breakout;
  breakout.setup(10, 5, 100, 30);

  display_console_t display_console;
  breakout.display_board(display_console);
  breakout.display_paddle(display_console);

  [[maybe_unused]] int key = getch();

  endwin();

  return 0;
}
