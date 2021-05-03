#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "breakout.h"

#include <numeric>
#include <string>
#include <vector>

namespace doctest {
  template<>
  struct StringMaker<std::pair<int, int>> {
    static String convert(const std::pair<int, int>& value) {
      return String(
        (std::to_string(value.first) + ", " + std::to_string(value.second))
          .c_str());
    }
  };
} // namespace doctest

struct display_test_t : public display_t {
  std::vector<std::pair<int, int>> positions_;
  void output(int x, int y) override { positions_.push_back({x, y}); }
};

struct display_block_test_t : public display_t {
  display_block_test_t(int block_size) : block_size_(block_size) {}
  const int block_size_;
  struct block_t {
    std::pair<int, int> begin_;
    std::pair<int, int> end_;
  };
  int counter = 0;
  block_t active_block_;
  std::vector<block_t> blocks_;
  void output(int x, int y) override {
    if (counter++ == 0) {
      active_block_.begin_ = {x, y};
    }
    if (counter == block_size_) {
      active_block_.end_ = {x, y};
      blocks_.push_back(active_block_);
      counter = 0;
    }
  }
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
    CHECK(
      y_max == breakout.board_size().second + breakout.board_offset().second);
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

  SUBCASE("blocks are displayed") {
    display_test_t display_test;
    breakout.display_blocks(display_test);

    const int output_count = breakout.blocks_horizontal()
                           * breakout.blocks_vertical()
                           * breakout.block_width();

    CHECK(display_test.positions_.size() == output_count);
  }

  SUBCASE("blocks are laid out correctly") {
    display_block_test_t block_display_test(breakout.block_width());
    breakout.display_blocks(block_display_test);

    const auto [board_x, board_y] = breakout.board_offset();

    CHECK(
      block_display_test.blocks_.size()
      == breakout.blocks_horizontal() * breakout.blocks_vertical());
    CHECK(
      block_display_test.blocks_.front().begin_
      == std::pair{
        board_x + breakout.horizontal_padding(),
        board_y + breakout.vertical_padding()});
    CHECK(
      block_display_test.blocks_.front().end_
      == std::pair{
        board_x + breakout.horizontal_padding() + breakout.block_width() - 1,
        board_y + breakout.vertical_padding()});
    CHECK(
      block_display_test.blocks_.back().begin_
      == std::pair{
        board_x + breakout.horizontal_padding()
          + (breakout.blocks_horizontal() - 1)
              * (breakout.block_width() + breakout.horizontal_spacing()),
        board_y + breakout.vertical_padding()
          + (breakout.block_height() + breakout.vertical_spacing())
              * (breakout.blocks_vertical() - 1)});
    CHECK(
      block_display_test.blocks_.back().end_
      == std::pair{
        board_x + breakout.horizontal_padding()
          + (breakout.blocks_horizontal() - 1)
              * (breakout.block_width() + breakout.horizontal_spacing())
          + (breakout.block_width() - 1),
        board_y + breakout.vertical_padding()
          + (breakout.block_height() + breakout.vertical_spacing())
              * (breakout.blocks_vertical() - 1)});
  }

  SUBCASE("ball position begins above paddle") {
    const auto [ball_x, ball_y] = breakout.ball_position();
    const auto [paddle_x, paddle_y] = breakout.paddle_position();
    CHECK(ball_x == paddle_x);
    CHECK(ball_y == paddle_y - 1);
  }

  SUBCASE("ball displayed") {
    display_test_t display_test;
    breakout.display_ball(display_test);

    CHECK(display_test.positions_.size() == 1);
    CHECK(
      display_test.positions_.front().first
      == breakout.board_offset().first + breakout.ball_position().first);
    CHECK(
      display_test.positions_.front().second
      == breakout.board_offset().second + breakout.ball_position().second);
  }

  SUBCASE("ball moves with paddle before launch") {
    breakout.move_paddle_left(10);
    const auto [paddle_x, paddle_y] = breakout.paddle_position();
    const auto [ball_x, ball_y] = breakout.ball_position();
    CHECK(ball_x == paddle_x);
    CHECK(ball_y == paddle_y - 1);
  }

  SUBCASE("ball does not move with paddle after launch") {
    const auto [ball_x, ball_y] = breakout.ball_position();
    breakout.launch();
    breakout.move_paddle_right(10);
    const auto [next_ball_x, next_ball_y] = breakout.ball_position();
    CHECK(ball_x == next_ball_x);
    CHECK(ball_y == next_ball_y);
  }

  SUBCASE("ball moves after launch") {
    const auto [ball_x, ball_y] = breakout.ball_position();
    breakout.launch();

    {
      breakout.step();
      const auto [next_ball_x, next_ball_y] = breakout.ball_position();
      CHECK(next_ball_x == ball_x + 1);
      CHECK(next_ball_y == ball_y - 1);
    }

    {
      breakout.step();
      const auto [next_ball_x, next_ball_y] = breakout.ball_position();
      CHECK(next_ball_x == ball_x + 2);
      CHECK(next_ball_y == ball_y - 2);
    }
  }

  SUBCASE("ball bounces off of vertical walls") {
    breakout.move_paddle_right(100);
    breakout.launch();
    const auto [launch_x_vel, launch_y_vel] = breakout.ball_velocity();
    CHECK(launch_x_vel == 1);
    for (int i = 0; i < 3; ++i) {
      breakout.step();
    }
    const auto [bounce_x_vel, bounce_y_vel] = breakout.ball_velocity();
    CHECK(bounce_x_vel == -1);
    CHECK(bounce_y_vel == -1);
    CHECK(bounce_y_vel == launch_y_vel);
  }
}
