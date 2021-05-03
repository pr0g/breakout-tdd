#include <chrono>
#include <iostream>
#include <ncurses.h>
#include <thread>

#include "breakout.h"

struct display_console_t : public display_t {
  void output(int x, int y) override { mvprintw(y, x, "*"); }
};

int main(int argc, char** argv) {
  // enable support for unicode characters
  setlocale(LC_CTYPE, "");

  initscr(); // start curses mode
  curs_set(0); // hide cursor
  cbreak(); // line buffering disabled (respects Ctrl-C to quit)
  keypad(stdscr, true); // enable function keys
  nodelay(stdscr, TRUE); // do not block
  noecho(); // don't echo while we do getch

  breakout_t breakout;
  breakout.setup(10, 5, 101, 30);

  display_console_t display_console;
  for (bool running = true; running;) {
    switch (int key = getch(); key) {
      case KEY_LEFT:
        breakout.move_paddle_left(1);
        break;
      case KEY_RIGHT:
        breakout.move_paddle_right(1);
        break;
      case ' ': // space
        breakout.launch();
        break;
      case ERR:
        // do nothing
        break;
      default:
        break;
    }

    breakout.step();

    clear();
    breakout.display_board(display_console);
    breakout.display_paddle(display_console);
    breakout.display_blocks(display_console);
    breakout.display_ball(display_console);

    using std::chrono_literals::operator""ms;
    std::this_thread::sleep_for(100ms);
  }

  endwin();

  return 0;
}
