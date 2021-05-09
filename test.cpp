#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "breakout.h"

#include <numeric>
#include <string>
#include <vector>

namespace doctest {
  template<>
  struct StringMaker<vec2> {
    static String convert(const vec2& value) {
      return String(
        (std::to_string(value.x_) + ", " + std::to_string(value.y_)).c_str());
    }
  };
} // namespace doctest

struct display_test_t : public display_t {
  std::vector<vec2> positions_;
  void output(int x, int y, std::string_view) override {
    positions_.push_back({x, y});
  }
};

struct display_block_test_t : public display_t {
  display_block_test_t(int block_size) : block_size_(block_size) {}
  const int block_size_;
  struct block_t {
    vec2 begin_;
    vec2 end_;
  };
  int counter = 0;
  block_t active_block_;
  std::vector<block_t> blocks_;
  void output(int x, int y, std::string_view) override {
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

TEST_CASE("vec2") {
  SUBCASE("equal") {
    const auto lhs = vec2{2, 3};
    const auto rhs = vec2{2, 3};
    CHECK(lhs == rhs);
  }
  SUBCASE("not equal x") {
    const auto lhs = vec2{1, 3};
    const auto rhs = vec2{2, 3};
    CHECK(!(lhs == rhs));
  }

  SUBCASE("not equal y") {
    const auto lhs = vec2{2, 3};
    const auto rhs = vec2{2, 4};
    CHECK(!(lhs == rhs));
  }
}

TEST_CASE("breakout game") {
  breakout_t breakout;

  int test_x = 10;
  int test_y = 5;
  int test_width = 101;
  int test_height = 30;
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
    breakout.display_board(
      display_test, std::string_view{"*"}, std::string_view{"*"},
      std::string_view{"*"}, std::string_view{"*"}, std::string_view{"*"},
      std::string_view{"*"});

    const int outline_character_count = ((breakout.board_size().x_ + 1) * 2)
                                      + ((breakout.board_size().y_ - 1) * 2);
    CHECK(display_test.positions_.size() == outline_character_count);

    int x_max = 0;
    int x_min = INT_MAX;
    int y_max = 0;
    int y_min = INT_MAX;
    for (const auto& position : display_test.positions_) {
      x_max = std::max(position.x_, x_max);
      x_min = std::min(position.x_, x_min);
      y_max = std::max(position.y_, y_max);
      y_min = std::min(position.y_, y_min);
    }

    CHECK(x_min == breakout.board_offset().x_);
    CHECK(x_max == breakout.board_size().x_ + breakout.board_offset().x_);
    CHECK(y_min == breakout.board_offset().y_);
    CHECK(y_max == breakout.board_size().y_ + breakout.board_offset().y_);
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
    auto paddle_width = breakout.paddle_width();

    CHECK(breakout.paddle_left_edge() == 45);
    CHECK(breakout.paddle_right_edge() == 54);
    CHECK(breakout.paddle_left_edge() == paddle_x - paddle_width / 2);
    CHECK(breakout.paddle_right_edge() == paddle_x + paddle_width / 2 - 1);
  }

  SUBCASE("board outline displayed") {
    display_test_t display_test;
    breakout.display_paddle(display_test, std::string_view{"*"});

    const auto [board_x, board_y] = breakout.board_offset();
    auto [paddle_x, paddle_y] = breakout.paddle_position();

    int offset = 0;
    for (const auto& position : display_test.positions_) {
      CHECK(position.x_ == board_x + breakout.paddle_left_edge() + offset++);
      CHECK(position.y_ == board_y + breakout.board_size().y_ - 1);
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
      breakout.board_size().x_ - start_paddle_right_x - 1);
    const auto before_paddle_right_x = breakout.paddle_right_edge();
    breakout.move_paddle_right(1);
    const auto after_paddle_right_x = breakout.paddle_right_edge();
    CHECK(after_paddle_right_x == before_paddle_right_x);
  }

  SUBCASE("overshoot left/right is constrained") {
    breakout.move_paddle_right(1000);
    CHECK(breakout.paddle_right_edge() == breakout.board_size().x_ - 1);
    breakout.move_paddle_left(500);
    CHECK(breakout.paddle_left_edge() == 1);
  }

  SUBCASE("blocks are displayed") {
    display_test_t display_test;
    breakout.display_blocks(display_test, std::string_view{"*"});

    const int output_count =
      breakout.block_cols() * breakout.block_rows() * breakout.block_width();

    CHECK(display_test.positions_.size() == output_count);
  }

  SUBCASE("blocks are laid out correctly") {
    display_block_test_t block_display_test(breakout.block_width());
    breakout.display_blocks(block_display_test, std::string_view{"*"});

    const auto [board_x, board_y] = breakout.board_offset();

    CHECK(
      block_display_test.blocks_.size()
      == breakout.block_cols() * breakout.block_rows());
    CHECK(
      block_display_test.blocks_.front().begin_
      == vec2{
        board_x + breakout.col_margin(), board_y + breakout.row_margin()});
    CHECK(
      block_display_test.blocks_.front().end_
      == vec2{
        board_x + breakout.col_margin() + breakout.block_width() - 1,
        board_y + breakout.row_margin()});
    CHECK(
      block_display_test.blocks_.back().begin_
      == vec2{
        board_x + breakout.col_margin()
          + (breakout.block_cols() - 1)
              * (breakout.block_width() + breakout.col_spacing()),
        board_y + breakout.row_margin()
          + (breakout.block_height() + breakout.row_spacing())
              * (breakout.block_rows() - 1)});
    CHECK(
      block_display_test.blocks_.back().end_
      == vec2{
        board_x + breakout.col_margin()
          + (breakout.block_cols() - 1)
              * (breakout.block_width() + breakout.col_spacing())
          + (breakout.block_width() - 1),
        board_y + breakout.row_margin()
          + (breakout.block_height() + breakout.row_spacing())
              * (breakout.block_rows() - 1)});
  }

  SUBCASE("ball position begins above paddle") {
    const auto [ball_x, ball_y] = breakout.ball_position();
    const auto [paddle_x, paddle_y] = breakout.paddle_position();
    CHECK(ball_x == paddle_x);
    CHECK(ball_y == paddle_y - 1);
  }

  SUBCASE("ball displayed") {
    display_test_t display_test;
    breakout.display_ball(display_test, std::string_view{"*"});

    CHECK(display_test.positions_.size() == 1);
    CHECK(
      display_test.positions_.front().x_
      == breakout.board_offset().x_ + breakout.ball_position().x_);
    CHECK(
      display_test.positions_.front().y_
      == breakout.board_offset().y_ + breakout.ball_position().y_);
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
    breakout.launch_right();
    breakout.move_paddle_right(10);
    const auto [next_ball_x, next_ball_y] = breakout.ball_position();
    CHECK(ball_x == next_ball_x);
    CHECK(ball_y == next_ball_y);
  }

  SUBCASE("ball moves after launch") {
    const auto [ball_x, ball_y] = breakout.ball_position();
    breakout.launch_right();

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

  SUBCASE("ball bounces off of right wall") {
    breakout.move_paddle_right(100);
    breakout.launch_right();
    const auto [launch_x_vel, launch_y_vel] = breakout.ball_velocity();
    CHECK(launch_x_vel == 1);
    for (int i = 0; i < 4; ++i) {
      breakout.step();
    }
    const auto [bounce_x_vel, bounce_y_vel] = breakout.ball_velocity();
    CHECK(bounce_x_vel == -1);
    CHECK(bounce_y_vel == -1);
    CHECK(bounce_y_vel == launch_y_vel);
  }

  SUBCASE("ball bounces off of left wall") {
    breakout.move_paddle_left(100);
    breakout.launch_left();
    const auto [launch_x_vel, launch_y_vel] = breakout.ball_velocity();
    CHECK(launch_x_vel == -1);
    for (int i = 0; i < 5; ++i) {
      breakout.step();
    }
    const auto [bounce_x_vel, bounce_y_vel] = breakout.ball_velocity();
    CHECK(bounce_x_vel == 1);
    CHECK(bounce_y_vel == -1);
    CHECK(bounce_y_vel == launch_y_vel);
  }

  SUBCASE("ball bounces off of top wall") {
    breakout.set_block_bounce_fn([](blocks_t&, ball_t&) { return false; });

    const auto start_ball_y = breakout.ball_position().y_;
    breakout.launch_left();
    const auto [launch_x_vel, launch_y_vel] = breakout.ball_velocity();
    for (int i = 0; i < start_ball_y; ++i) {
      breakout.step();
    }
    const auto [bounce_x_vel, bounce_y_vel] = breakout.ball_velocity();
    CHECK(bounce_x_vel == -1);
    CHECK(bounce_y_vel == 1);
    CHECK(bounce_x_vel == launch_x_vel);
  }

  SUBCASE("miss ball lose life") {
    const auto [board_x, board_y] = breakout.board_size();
    const auto starting_lives = breakout.lives();
    breakout.launch_left();
    while (true) {
      breakout.step();
      const auto [ball_x, ball_y] = breakout.ball_position();
      if (ball_y >= board_y) {
        CHECK(breakout.state() == breakout_t::game_state_e::lost_life);
        break;
      }
    }
    CHECK(breakout.lives() == starting_lives - 1);
  }

  SUBCASE("ball position reset after losing life") {
    breakout.launch_left();
    while (true) {
      breakout.step();
      if (breakout.state() == breakout_t::game_state_e::lost_life) {
        breakout.step();
        break;
      }
    }
    CHECK(breakout.ball_position().x_ == breakout.paddle_position().x_);
    CHECK(breakout.ball_position().y_ == breakout.paddle_position().y_ - 1);
  }

  SUBCASE("miss ball, state returns to not launched") {
    const auto [board_x, board_y] = breakout.board_size();
    breakout.launch_left();
    while (true) {
      breakout.step();
      if (!breakout.launched()) {
        break;
      }
    }
    CHECK(breakout.state() == breakout_t::game_state_e::lost_life);
  }

  SUBCASE("ball velocity reset after losing life") {
    breakout.launch_left();
    while (true) {
      breakout.step();
      if (breakout.state() == breakout_t::game_state_e::lost_life) {
        breakout.step();
        break;
      }
    }
    CHECK(breakout.ball_velocity() == vec2{0, 0});
  }

  SUBCASE("paddle detects ball intersection") {
    paddle_t paddle;
    paddle.position_ = {50, 50};
    paddle.width_ = 10;

    ball_t ball;
    ball.position_ = {44, 50};
    CHECK(!intersects(paddle, ball));

    ball.position_ = {45, 50};
    CHECK(intersects(paddle, ball));

    ball.position_ = {54, 50};
    CHECK(intersects(paddle, ball));

    ball.position_ = {55, 50};
    CHECK(!intersects(paddle, ball));

    ball.position_ = {50, 49};
    CHECK(!intersects(paddle, ball));

    ball.position_ = {50, 51};
    CHECK(!intersects(paddle, ball));
  }

  SUBCASE("ball vertical velocity switches after paddle intersection") {
    paddle_t paddle;
    paddle.position_ = {50, 50};
    paddle.width_ = 10;

    ball_t ball;
    ball.position_ = {45, 45};
    ball.velocity_ = {1, 1};

    for (int i = 0; i < 10; ++i) {
      step(paddle, ball);
    }

    CHECK(ball.velocity_.y_ == -1);
    CHECK(ball.position_.x_ == 55);
    CHECK(ball.position_.y_ == 45);
  }

  SUBCASE("ball intersects block") {
    blocks_t blocks = create_blocks(breakout);

    ball_t ball;
    // top left block
    ball.position_ = {blocks.col_margin, blocks.row_margin};
    CHECK(intersects(blocks, ball));

    // top left (board outline)
    ball.position_ = {0, 0};
    CHECK(!intersects(blocks, ball));

    // top left (board)
    ball.position_ = {1, 1};
    CHECK(!intersects(blocks, ball));

    {
      const int block_x =
        blocks.col_margin + ((blocks.block_width + blocks.col_spacing) * 1);
      const int block_y =
        blocks.row_margin + ((blocks.block_height + blocks.row_spacing) * 2);
      ball.position_ = {block_x, block_y};
      CHECK(intersects(blocks, ball));
    }

    {
      const int block_x = blocks.col_margin
                        + ((blocks.block_width + blocks.col_spacing)
                           * (breakout.block_cols() - 1));
      const int block_y = blocks.row_margin
                        + ((blocks.block_height + blocks.row_spacing)
                           * (breakout.block_rows() - 1));
      ball.position_ = {block_x, block_y};
      CHECK(intersects(blocks, ball));
    }
  }

  SUBCASE("ball vertical velocity switches after block intersection") {
    blocks_t blocks = create_blocks(breakout);

    ball_t ball;
    ball.velocity_ = {1, 1};

    const int block_x = blocks.col_margin
                      + ((blocks.block_width + blocks.col_spacing)
                         * (breakout.block_cols() - 1));
    const int block_y = blocks.row_margin
                      + ((blocks.block_height + blocks.row_spacing)
                         * (breakout.block_rows() - 1));

    ball.position_ = {block_x, block_y};

    block_bounce(blocks, ball);

    CHECK(ball.velocity_.x_ == 1);
    CHECK(ball.velocity_.y_ == -1);
  }

  SUBCASE("block_bounce called in breakout step") {
    bool called = false;
    const auto bounce_fn = [&called](const blocks_t& blocks, ball_t ball) {
      called = true;
      return false;
    };

    breakout.set_block_bounce_fn(bounce_fn);
    breakout.launch_right();
    breakout.step();

    CHECK(called);
  }

  SUBCASE("a block can be destroyed") {
    blocks_t blocks = create_blocks(breakout);
    CHECK(!block_destroyed(blocks, 0, 1));
    destroy_block(blocks, 0, 1);
    CHECK(block_destroyed(blocks, 0, 1));
  }

  SUBCASE("an arbitrary block can be destroyed") {
    blocks_t blocks = create_blocks(breakout);

    CHECK(!block_destroyed(blocks, 2, 2));
    destroy_block(blocks, 2, 2);
    CHECK(block_destroyed(blocks, 2, 2));

    CHECK(!block_destroyed(blocks, 5, 4));
    destroy_block(blocks, 5, 4);
    CHECK(block_destroyed(blocks, 5, 4));

    CHECK(!block_destroyed(blocks, 6, 7));
    destroy_block(blocks, 6, 7);
    CHECK(block_destroyed(blocks, 6, 7));
  }

  SUBCASE("block position can be looked up") {
    const blocks_t blocks = create_blocks(breakout);

    const auto top_left_position = block_position(blocks, 0, 0).value();
    const auto center_position =
      block_position(blocks, blocks.col_count / 2, blocks.row_count / 2)
        .value();
    const auto bottom_right_position =
      block_position(blocks, blocks.col_count - 1, blocks.row_count - 1)
        .value();

    CHECK(
      top_left_position.x_
      == blocks.col_margin + ((blocks.block_width - 1) / 2));
    CHECK(top_left_position.y_ == blocks.row_margin);

    CHECK(center_position.x_ == breakout.board_size().x_ / 2);
    CHECK(
      center_position.y_
      == blocks.row_margin
           + ((blocks.row_count / 2) * (blocks.block_height + blocks.row_spacing)));

    CHECK(
      bottom_right_position.x_
      == breakout.board_size().x_ - blocks.col_margin
           - (blocks.block_width / 2));
    CHECK(
      bottom_right_position.y_
      == blocks.row_margin
           + ((blocks.row_count - 1) * (blocks.block_height + blocks.row_spacing)));
  }

  SUBCASE("invalid block position returns empty") {
    const blocks_t blocks = create_blocks(breakout);

    const auto empty_large = block_position(blocks, 100, 100);
    const auto empty_invalid = block_position(blocks, -1, -1);

    CHECK(!empty_large.has_value());
    CHECK(!empty_invalid.has_value());
  }

  SUBCASE("block_bounce destroys block") {
    blocks_t blocks = create_blocks(breakout);

    const int block_x = blocks.col_count / 2;
    const int block_y = blocks.row_count - 1;

    ball_t ball;
    ball.position_ = block_position(blocks, block_x, block_y).value();

    block_bounce(blocks, ball);

    CHECK(block_destroyed(blocks, block_x, block_y));
  }

  SUBCASE("ball cannot hit destroyed block") {
    blocks_t blocks = create_blocks(breakout);

    ball_t ball;
    ball.position_ = block_position(blocks, 0, 0).value();
    block_bounce(blocks, ball);

    CHECK(block_destroyed(blocks, 0, 0));
    CHECK(!intersects(blocks, ball));
  }

  SUBCASE("destroyed blocks are not displayed") {
    blocks_t blocks = create_blocks(breakout);

    destroy_block(blocks, 0, 0);

    display_block_test_t block_display_test(breakout.block_width());
    display_blocks(
      blocks, vec2{0, 0}, block_display_test, std::string_view{"*"});

    // first block was not drawn
    CHECK(
      block_display_test.blocks_.front().begin_.x_
      == blocks.col_margin + blocks.block_width + blocks.col_spacing);
  }

  SUBCASE("score increases after block is destroyed") {
    int hit_count = 0;
    breakout.set_block_bounce_fn([&hit_count](blocks_t& blocks, ball_t& ball) {
      if (::block_bounce(blocks, ball)) {
        hit_count++;
        return true;
      }
      return false;
    });

    breakout.launch_right();
    while (true) {
      breakout.step();
      if (hit_count == 1) {
        break;
      }
    }
    CHECK(breakout.block_score() * hit_count == breakout.score());
  }

  auto simulate_game_over = [](breakout_t& breakout) {
    while (true) {
      if (breakout.lives() == 0) {
        break;
      }
      breakout.launch_left();
      while (true) {
        breakout.step();
        if (!breakout.launched()) {
          break;
        }
      }
    }
  };

  SUBCASE("zero lives results in game over") {
    simulate_game_over(breakout);
    CHECK(breakout.state() == breakout_t::game_state_e::game_over);
  }

  SUBCASE("cannot relaunch while ball is launched") {
    breakout.launch_left();
    breakout.step();
    breakout.launch_right();
    CHECK(breakout.ball_velocity().x_ == -1);
  }

  SUBCASE("restart after game over") {
    simulate_game_over(breakout);

    breakout.restart();

    const auto [board_width, board_height] = breakout.board_size();
    const auto [paddle_x, paddle_y] = breakout.paddle_position();
    const auto [ball_x, ball_y] = breakout.ball_position();
    CHECK(breakout.state() == breakout_t::game_state_e::preparing);
    CHECK(breakout.lives() == breakout.starting_lives());
    CHECK(paddle_x == board_width / 2);
    CHECK(paddle_y == board_height - 1);
    CHECK(ball_x == paddle_x);
    CHECK(ball_y == paddle_y - 1);
  }

  SUBCASE("all blocks destroyed wins game") {
    const auto create_blocks_destroyed = [&breakout] {
      auto blocks = ::create_blocks(breakout);
      std::for_each(
        std::begin(blocks.destroyed_), std::end(blocks.destroyed_),
        [](auto& block) { block = false; });
      return blocks;
    };

    breakout.set_create_blocks_fn(create_blocks);

    breakout.restart();
    breakout.step();

    CHECK(breakout.state() == breakout_t::game_state_e::game_complete);
  }
}
