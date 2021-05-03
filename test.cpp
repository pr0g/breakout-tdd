#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "breakout.h"

#include <numeric>
#include <vector>

struct display_test_t : public display_t {
  std::vector<std::pair<int, int>> positions_;
  void output(int x, int y) override { positions_.push_back({x, y}); }
};

TEST_CASE("breakout game") {
  breakout_t breakout;

  int test_x = 10;
  int test_y = 5;
  int test_width = 101;
  int test_height = 80;
  breakout.setup(test_x, test_y, test_width, test_height);

  SUBCASE("initial score is zero") { CHECK(breakout.score() == 0); }
  SUBCASE("board play area") {
    const auto [x, y] = breakout.board_offset();
    CHECK(x == test_x);
    CHECK(y == test_y);

    const auto [width, height] = breakout.board_size();
    CHECK(width == test_width);
    CHECK(height == test_height);
  }

  SUBCASE("board outline displayed") {
    display_test_t display_test;
    breakout.display_board(display_test);

    const int outline_character_count =
      ((breakout.board_size().first + 1) * 2)
      + ((breakout.board_size().second - 1) * 2);
    CHECK(display_test.positions_.size() == outline_character_count);

    int x_max = 0;
    int x_min = INT_MAX;
    int y_max = 0;
    int y_min = INT_MAX;
    for (const auto& position : display_test.positions_) {
      x_max = std::max(position.first, x_max);
      x_min = std::min(position.first, x_min);
      y_max = std::max(position.second, y_max);
      y_min = std::min(position.second, y_min);
    }

    CHECK(x_min == breakout.board_offset().first);
    CHECK(x_max == breakout.board_size().first + breakout.board_offset().first);
    CHECK(y_min == breakout.board_offset().second);
    CHECK(y_max == breakout.board_size().second + breakout.board_offset().second);
  }

  SUBCASE("paddle begins centered (board space)") {
    auto [board_width, board_height] = breakout.board_size();
    auto [paddle_x, paddle_y] = breakout.paddle_position();
    CHECK(paddle_x == board_width / 2);
    CHECK(paddle_y == board_height - 1);
  }

  //       ####       4 (6 4 6) (board even, paddle even)
  // ################ 16
  //       ###        3 (6 3 6) (board odd, paddle odd)
  // ###############  15
  SUBCASE("paddle size (board space)") {
    auto [paddle_x, paddle_y] = breakout.paddle_position();
    auto [paddle_width, paddle_height] = breakout.paddle_size();

    CHECK(breakout.paddle_left_edge() == 45);
    CHECK(breakout.paddle_right_edge() == 54);
    CHECK(paddle_height == 1);
  }

  SUBCASE("board outline displayed") {
    display_test_t display_test;
    breakout.display_paddle(display_test);

    const auto [board_x, board_y] = breakout.board_offset();
    auto [paddle_x, paddle_y] = breakout.paddle_position();
    auto [paddle_width, paddle_height] = breakout.paddle_size();

    int offset = 0;
    for (const auto& position : display_test.positions_) {
      CHECK(position.first == board_x + breakout.paddle_left_edge() + offset++);
      CHECK(position.second == board_y + breakout.board_size().second - 1);
    }
  }

  SUBCASE("paddle can be moved left") {
    const int distance = 2;
    auto [start_paddle_x, _1] = breakout.paddle_position();
    breakout.move_paddle_left(distance);
    auto [paddle_x, _2] = breakout.paddle_position();
    CHECK(paddle_x == start_paddle_x - distance);
  }

  SUBCASE("paddle can be moved right") {
    const int distance = 4;
    auto [start_paddle_x, _1] = breakout.paddle_position();
    breakout.move_paddle_right(distance);
    auto [paddle_x, _2] = breakout.paddle_position();
    CHECK(paddle_x == start_paddle_x + distance);
  }

  SUBCASE("paddle cannot be moved passed left edge") {
    const auto start_paddle_left_x = breakout.paddle_left_edge();
    breakout.move_paddle_left(start_paddle_left_x - 1);
    const auto before_paddle_left_x = breakout.paddle_left_edge();
    breakout.move_paddle_left(1);
    const auto after_paddle_left_x = breakout.paddle_left_edge();
    CHECK(after_paddle_left_x == before_paddle_left_x);
  }

  SUBCASE("paddle cannot be moved passed right edge") {
    const auto start_paddle_right_x = breakout.paddle_right_edge();
    breakout.move_paddle_right(
      breakout.board_size().first - start_paddle_right_x - 1);
    const auto before_paddle_right_x = breakout.paddle_right_edge();
    breakout.move_paddle_right(1);
    const auto after_paddle_right_x = breakout.paddle_right_edge();
    CHECK(after_paddle_right_x == before_paddle_right_x);
  }

  SUBCASE("overshoot left/right is constrained") {
    breakout.move_paddle_right(1000);
    CHECK(breakout.paddle_right_edge() == breakout.board_size().first - 1);
    breakout.move_paddle_left(500);
    CHECK(breakout.paddle_left_edge() == 1);
  }
}
