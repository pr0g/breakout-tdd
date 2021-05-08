#pragma once

#include <deque>
#include <optional>
#include <string_view>

struct vec2 {
  int x_;
  int y_;
};

inline bool operator==(const vec2 lhs, const vec2 rhs) {
  return lhs.x_ == rhs.x_ && lhs.y_ == rhs.y_;
}

struct display_t {
  virtual void output(int x, int y, std::string_view glyph) = 0;

protected:
  ~display_t() = default;
};

struct paddle_t {
  vec2 position_;
  int width_;

  [[nodiscard]] int left_edge() const { return position_.x_ - width_ / 2; }
  [[nodiscard]] int right_edge() const {
    return position_.x_ + (width_ / 2 - 1);
  }
};

struct ball_t {
  vec2 position_;
  vec2 velocity_;
};

struct blocks_t {
  int col_margin;
  int row_margin;
  int col_spacing;
  int row_spacing;
  int col_count;
  int row_count;
  int block_height;
  int block_width;

  std::deque<bool> destroyed_;
};

bool intersects(const paddle_t& paddle, const ball_t& ball) {
  if (
    ball.position_.x_ >= paddle.left_edge()
    && ball.position_.x_ <= paddle.right_edge()
    && ball.position_.y_ >= paddle.position_.y_
    && ball.position_.y_ <= paddle.position_.y_) {
    return true;
  }
  return false;
}

struct lookup_t {
  int col_;
  int row_;
};

bool block_destroyed(const blocks_t blocks, const int col, const int row) {
  return blocks.destroyed_[row * blocks.col_count + col];
}

void destroy_block(blocks_t& blocks, const int col, const int row) {
  blocks.destroyed_[row * blocks.col_count + col] = true;
}

std::optional<lookup_t> intersects(const blocks_t& blocks, const ball_t& ball) {
  for (int row = 0; row < blocks.row_count; row++) {
    for (int col = 0; col < blocks.col_count; col++) {
      if (block_destroyed(blocks, col, row)) {
        continue;
      }
      const int block_x =
        blocks.col_margin + ((blocks.block_width + blocks.col_spacing) * col);
      const int block_y =
        blocks.row_margin + ((blocks.block_height + blocks.row_spacing) * row);
      if (
        ball.position_.x_ >= block_x
        && ball.position_.x_ <= block_x + blocks.block_width
        && ball.position_.y_ == block_y) {
        return lookup_t{col, row};
      }
    }
  }
  return {};
}

void step(const paddle_t& paddle, ball_t& ball) {
  ball.position_.x_ += ball.velocity_.x_;
  ball.position_.y_ += ball.velocity_.y_;
  if (intersects(paddle, ball)) {
    ball.velocity_.y_ *= -1;
  }
}

bool block_bounce(blocks_t& blocks, ball_t& ball) {
  if (const auto block_col_row = intersects(blocks, ball)) {
    ball.velocity_.y_ *= -1;
    destroy_block(blocks, block_col_row->col_, block_col_row->row_);
    return true;
  }
  return false;
}

std::optional<vec2> block_position(const blocks_t& blocks, int col, int row) {
  if (
    col < 0 || col >= blocks.col_count || row < 0 || row >= blocks.row_count) {
    return {};
  }

  return vec2{
    blocks.col_margin + ((blocks.block_width - 1) / 2)
      + ((blocks.block_width + blocks.col_spacing) * col),
    blocks.row_margin + ((blocks.block_height - 1) / 2)
      + ((blocks.block_height + blocks.row_spacing) * row)};
}

void display_blocks(
  const blocks_t& blocks, vec2 offset, display_t& display,
  std::string_view glyph) {
  for (int row = 0; row < blocks.row_count; ++row) {
    for (int col = 0; col < blocks.col_count; ++col) {
      if (block_destroyed(blocks, col, row)) {
        continue;
      }
      for (int block_part = 0; block_part < blocks.block_width; ++block_part) {
        display.output(
          offset.x_ + blocks.col_margin + block_part
            + ((blocks.block_width + blocks.col_spacing) * col),
          offset.y_ + blocks.row_margin
            + ((blocks.block_height + blocks.row_spacing) * row),
          glyph);
      }
    }
  }
}

class breakout_t;
blocks_t create_blocks(const breakout_t& breakout);

class breakout_t {
public:
  enum class game_state_e { preparing, launched, lost_life, game_over };

  void setup(int x, int y, int width, int height) {
    board_size_ = {width, height};
    board_offset_ = {x, y};
    paddle_.position_ = {width / 2, height - 1};
    paddle_.width_ = 10; // default size
    ball_.position_ = {paddle_.position_.x_, paddle_.position_.y_ - 1};
    ball_.velocity_ = {0, 0};
    state_ = game_state_e::preparing;
    lives_ = 3;
    score_ = 0;
    block_bounce_fn_ = ::block_bounce;
    blocks_ = create_blocks(*this);
  }

  using block_bounce_fn_t = std::function<bool(blocks_t& blocks, ball_t& ball)>;
  void set_block_bounce_fn(const block_bounce_fn_t& bounce_fn) {
    block_bounce_fn_ = bounce_fn;
  }

  [[nodiscard]] vec2 board_offset() const { return board_offset_; }
  [[nodiscard]] vec2 board_size() const { return board_size_; }
  [[nodiscard]] vec2 paddle_position() const { return paddle_.position_; }
  [[nodiscard]] int paddle_width() const { return paddle_.width_; }
  [[nodiscard]] vec2 ball_position() const { return ball_.position_; }
  [[nodiscard]] vec2 ball_velocity() const { return ball_.velocity_; }

  [[nodiscard]] int block_cols() const { return 11; }
  [[nodiscard]] int block_rows() const { return 9; }
  [[nodiscard]] int block_width() const { return 8; }
  [[nodiscard]] int block_height() const { return 1; }
  [[nodiscard]] int row_margin() const { return 1; }
  [[nodiscard]] int col_margin() const { return 2; }
  [[nodiscard]] int col_spacing() const { return 1; }
  [[nodiscard]] int row_spacing() const { return 1; }

  [[nodiscard]] game_state_e state() const { return state_; }
  [[nodiscard]] bool launched() const {
    return state_ == game_state_e::launched;
  }

  [[nodiscard]] int block_score() const { return 10; }
  [[nodiscard]] int lives() const { return lives_; }
  [[nodiscard]] int score() const { return score_; }

  [[nodiscard]] int paddle_left_edge() const { return paddle_.left_edge(); }
  [[nodiscard]] int paddle_right_edge() const { return paddle_.right_edge(); }

  void launch_left() { launch({-1, -1}); }
  void launch_right() { launch({1, -1}); }

  void move_paddle_left(const int distance) {
    if (paddle_left_edge() > 1) {
      const int move = std::min(paddle_left_edge() - 1, distance);
      paddle_.position_.x_ -= move;
    }
    try_move_ball();
  }

  void move_paddle_right(const int distance) {
    if (paddle_right_edge() < board_size_.x_) {
      const int move =
        std::min(board_size_.x_ - paddle_right_edge() - 1, distance);
      paddle_.position_.x_ += move;
    }
    try_move_ball();
  }

  void step() {
    switch (state_) {
      case game_state_e::preparing:
        // noop
        break;
      case game_state_e::game_over:
        break;
      case game_state_e::launched: {
        ::step(paddle_, ball_);
        if (block_bounce_fn_(blocks_, ball_)) {
          score_ += block_score();
        }
        if (
          ball_.position_.x_ >= board_size_.x_ - 1 || ball_.position_.x_ <= 1) {
          ball_.velocity_.x_ *= -1;
        }
        if (ball_.position_.y_ <= 0) {
          ball_.velocity_.y_ *= -1;
        }
        if (ball_.position_.y_ >= board_size_.y_) {
          lives_--;
          state_ =
            lives_ == 0 ? game_state_e::game_over : game_state_e::lost_life;
        }
      } break;
      case game_state_e::lost_life: {
        ball_.velocity_ = vec2{0, 0};
        ball_.position_ = vec2{paddle_.position_.x_, paddle_.position_.y_ - 1};
        state_ = game_state_e::preparing;
      } break;
    }
  }

  void display_board(
    display_t& display, std::string_view horizontal_glyph,
    std::string_view vertical_glyph, std::string_view top_left_glyph,
    std::string_view top_right_glyph, std::string_view bottom_left_glyph,
    std::string_view bottom_right_glyph) {
    const auto [board_width, board_height] = board_size_;
    const auto [board_x, board_y] = board_offset_;
    display.output(board_x, board_y, top_left_glyph);
    display.output(board_x + board_width, board_y, top_right_glyph);
    display.output(board_x, board_y + board_height, bottom_left_glyph);
    display.output(
      board_x + board_width, board_y + board_height, bottom_right_glyph);
    for (int x = board_x + 1; x < board_x + board_width; x++) {
      display.output(x, board_y, horizontal_glyph);
      display.output(x, board_y + board_height, horizontal_glyph);
    }
    for (int y = board_y + 1; y <= board_y + board_height - 1; y++) {
      display.output(board_x, y, vertical_glyph);
      display.output(board_x + board_width, y, vertical_glyph);
    }
  }

  void display_paddle(display_t& display, std::string_view glyph) {
    const auto [board_x, board_y] = board_offset_;
    const auto [paddle_x, paddle_y] = paddle_position();
    const auto width = paddle_width();

    for (int i = 0; i < width; ++i) {
      display.output(
        board_x + paddle_left_edge() + i, board_y + paddle_y, glyph);
    }
  }

  void display_blocks(display_t& display, std::string_view glyph) {
    const auto [board_x, board_y] = board_offset();
    ::display_blocks(blocks_, vec2{board_x, board_y}, display, glyph);
  }

  void display_ball(display_t& display, std::string_view glyph) {
    const auto [x, y] = ball_.position_;
    const auto [board_x, board_y] = board_offset();
    display.output(board_x + x, board_y + y, glyph);
  }

private:
  vec2 board_size_;
  vec2 board_offset_;
  paddle_t paddle_;
  ball_t ball_;
  int lives_;
  int score_;
  game_state_e state_;
  block_bounce_fn_t block_bounce_fn_;
  blocks_t blocks_;

  void try_move_ball() {
    if (state_ != game_state_e::launched) {
      ball_.position_.x_ = paddle_.position_.x_;
    }
  }

  void launch(vec2 velocity) {
    state_ = game_state_e::launched;
    ball_.velocity_ = velocity;
  }
};

blocks_t create_blocks(const breakout_t& breakout) {
  blocks_t blocks;
  blocks.col_margin = breakout.col_margin();
  blocks.row_margin = breakout.row_margin();
  blocks.col_spacing = breakout.col_spacing();
  blocks.row_spacing = breakout.row_spacing();
  blocks.col_count = breakout.block_cols();
  blocks.row_count = breakout.block_rows();
  blocks.block_height = breakout.block_height();
  blocks.block_width = breakout.block_width();
  blocks.destroyed_ =
    std::deque<bool>(breakout.block_cols() * breakout.block_rows(), false);
  return blocks;
}
