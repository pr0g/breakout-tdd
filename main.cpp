#include <chrono>
#include <iostream>
#include <ncurses.h>
#include <thread>

#include "breakout.h"

struct display_console_t : public display_t {
  void output(int x, int y, std::string_view glyph) override {
    mvprintw(y, x, "%.*s", int(glyph.length()), glyph.data());
  }
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
        breakout.move_paddle_left(2);
        break;
      case KEY_RIGHT:
        breakout.move_paddle_right(2);
        break;
      case ' ': // space
        switch (breakout.state()) {
          case breakout_t::game_state_e::game_over:
            breakout.restart();
            break;
          default:
            breakout.launch_left();
            break;
        }
      case ERR:
        // do nothing
        break;
      default:
        break;
    }

    breakout.step();

    clear();
    breakout.display_board(
      display_console, std::string_view{"\xE2\x94\x81"},
      std::string_view{"\xE2\x94\x83"}, std::string_view{"\xE2\x94\x8F"},
      std::string_view{"\xE2\x94\x93"}, std::string_view{"\xE2\x94\x97"},
      std::string_view{"\xE2\x94\x9b"});

    const auto show_final_score = [&breakout] {
      const std::string final_score =
        "Final Score: " + std::to_string(breakout.score());
      mvprintw(
        breakout.board_offset().y_ + breakout.board_size().y_ / 2 + 2,
        breakout.board_offset().x_ + (breakout.board_size().x_ / 2)
          - (final_score.size() / 2),
        final_score.c_str());
    };

    switch (breakout.state()) {
      case breakout_t::game_state_e::game_over: {
        constexpr std::string_view game_over_msg = "Game Over";
        mvprintw(
          breakout.board_offset().y_ + breakout.board_size().y_ / 2,
          breakout.board_offset().x_ + (breakout.board_size().x_ / 2)
            - (game_over_msg.size() / 2),
          "%.*s", int(game_over_msg.length()), game_over_msg.data());
        show_final_score();
      } break;
      case breakout_t::game_state_e::game_complete: {
        constexpr std::string_view game_over_msg = "You Won!";
        mvprintw(
          breakout.board_offset().y_ + breakout.board_size().y_ / 2,
          breakout.board_offset().x_ + (breakout.board_size().x_ / 2)
            - (game_over_msg.size() / 2),
          "%.*s", int(game_over_msg.length()), game_over_msg.data());
        show_final_score();

      } break;
      default:
        breakout.display_paddle(
          display_console, std::string_view{"\xE2\x96\x91"});
        breakout.display_blocks(
          display_console, std::string_view{"\xE2\x96\x92"});
        breakout.display_ball(
          display_console, std::string_view{"\xE2\x98\xBB"});
        mvprintw(
          breakout.board_offset().y_ + 1,
          breakout.board_offset().x_ + breakout.board_size().x_ + 5,
          "Lives: %d", breakout.lives());
        mvprintw(
          breakout.board_offset().y_ + 3,
          breakout.board_offset().x_ + breakout.board_size().x_ + 5,
          "Score: %d", breakout.score());
        break;
    }

    using std::chrono_literals::operator""ms;
    std::this_thread::sleep_for(100ms);
  }

  endwin();

  return 0;
}
