#pragma once

#include <deque>
#include <optional>
#include <utility>

struct display_t {
  virtual void output(int x, int y) = 0;

protected:
  ~display_t() = default;
};

struct paddle_t {
  std::pair<int, int> position_;
  int width_;

  [[nodiscard]] int left_edge() const { return position_.first - width_ / 2; }
  [[nodiscard]] int right_edge() const {
    return position_.first + (width_ / 2 - 1);
  }
};

struct ball_t {
  std::pair<int, int> position_;
  std::pair<int, int> velocity_;
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
    ball.position_.first >= paddle.left_edge()
    && ball.position_.first <= paddle.right_edge()
    && ball.position_.second >= paddle.position_.second
    && ball.position_.second <= paddle.position_.second) {
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
        ball.position_.first >= block_x
        && ball.position_.first <= block_x + blocks.block_width
        && ball.position_.second == block_y) {
        return lookup_t{col, row};
      }
    }
  }
  return {};
}

void step(const paddle_t& paddle, ball_t& ball) {
  ball.position_.first += ball.velocity_.first;
  ball.position_.second += ball.velocity_.second;
  if (intersects(paddle, ball)) {
    ball.velocity_.second *= -1;
  }
}

void bounce(blocks_t& blocks, ball_t& ball) {
  if (const auto block_col_row = intersects(blocks, ball)) {
    ball.velocity_.second *= -1;
    destroy_block(blocks, block_col_row->col_, block_col_row->row_);
  }
}

std::optional<std::pair<int, int>> block_position(
  const blocks_t& blocks, int col, int row) {
  if (
    col < 0 || col >= blocks.col_count || row < 0 || row >= blocks.row_count) {
    return {};
  }

  return std::pair{
    blocks.col_margin + ((blocks.block_width - 1) / 2)
      + ((blocks.block_width + blocks.col_spacing) * col),
    blocks.row_margin + ((blocks.block_height - 1) / 2)
      + ((blocks.block_height + blocks.row_spacing) * row)};
}

class breakout_t;
blocks_t create_blocks(const breakout_t& breakout);

class breakout_t {
public:
  enum class game_state_e { preparing, launched, lost_life };

  void setup(int x, int y, int width, int height) {
    board_size_ = {width, height};
    board_offset_ = {x, y};
    paddle_.position_ = {width / 2, height - 1};
    paddle_.width_ = 10; // default size
    ball_.position_ = {paddle_.position_.first, paddle_.position_.second - 1};
    ball_.velocity_ = {0, 0};
    state_ = game_state_e::preparing;
    lives_ = 3;
    bounce_fn_ = ::bounce;
  }

  using bounce_fn_t = std::function<void(blocks_t& blocks, ball_t& ball)>;
  void set_bounce_fn(const bounce_fn_t& bounce_fn) { bounce_fn_ = bounce_fn; }

  [[nodiscard]] std::pair<int, int> board_offset() const {
    return board_offset_;
  }
  [[nodiscard]] std::pair<int, int> board_size() const { return board_size_; }
  [[nodiscard]] std::pair<int, int> paddle_position() const {
    return paddle_.position_;
  }
  [[nodiscard]] int paddle_width() const { return paddle_.width_; }
  [[nodiscard]] std::pair<int, int> ball_position() const {
    return ball_.position_;
  }
  [[nodiscard]] std::pair<int, int> ball_velocity() const {
    return ball_.velocity_;
  }

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
  [[nodiscard]] int lives() const { return lives_; }
  [[nodiscard]] int score() const { return 0; }

  [[nodiscard]] int paddle_left_edge() const { return paddle_.left_edge(); }
  [[nodiscard]] int paddle_right_edge() const { return paddle_.right_edge(); }

  void launch_left() { launch({-1, -1}); }
  void launch_right() { launch({1, -1}); }

  void move_paddle_left(const int distance) {
    if (paddle_left_edge() > 1) {
      const int move = std::min(paddle_left_edge() - 1, distance);
      paddle_.position_.first -= move;
    }
    try_move_ball();
  }

  void move_paddle_right(const int distance) {
    if (paddle_right_edge() < board_size_.first) {
      const int move =
        std::min(board_size_.first - paddle_right_edge() - 1, distance);
      paddle_.position_.first += move;
    }
    try_move_ball();
  }

  void step() {
    if (state_ == game_state_e::launched) {
      ::step(paddle_, ball_);
      blocks_t blocks = create_blocks(*this);
      bounce_fn_(blocks, ball_);
      if (
        ball_.position_.first >= board_size_.first - 1
        || ball_.position_.first <= 1) {
        ball_.velocity_.first *= -1;
      }
      if (ball_.position_.second <= 0) {
        ball_.velocity_.second *= -1;
      }
      if (ball_.position_.second >= board_size_.second) {
        state_ = game_state_e::lost_life;
        lives_--;
      }
    }
  }

  void display_board(display_t& display) {
    const auto [board_width, board_height] = board_size_;
    const auto [board_x, board_y] = board_offset_;

    for (int x = board_x; x <= board_x + board_width; x++) {
      display.output(x, board_y);
      display.output(x, board_y + board_height);
    }
    for (int y = board_y + 1; y <= board_y + board_height - 1; y++) {
      display.output(board_x, y);
      display.output(board_x + board_width, y);
    }
  }

  void display_paddle(display_t& display) {
    const auto [board_x, board_y] = board_offset_;
    const auto [paddle_x, paddle_y] = paddle_position();
    const auto width = paddle_width();

    for (int i = 0; i < width; ++i) {
      display.output(board_x + paddle_left_edge() + i, board_y + paddle_y);
    }
  }

  void display_blocks(display_t& display) {
    const auto [board_x, board_y] = board_offset();
    for (int row = 0; row < block_rows(); ++row) {
      for (int col = 0; col < block_cols(); ++col) {
        for (int block_part = 0; block_part < block_width(); ++block_part) {
          display.output(
            board_x + col_margin() + block_part
              + ((block_width() + col_spacing()) * col),
            board_y + row_margin() + ((block_height() + row_spacing()) * row));
        }
      }
    }
  }

  void display_ball(display_t& display) {
    const auto [x, y] = ball_.position_;
    const auto [board_x, board_y] = board_offset();
    display.output(board_x + x, board_y + y);
  }

private:
  std::pair<int, int> board_size_;
  std::pair<int, int> board_offset_;
  paddle_t paddle_;
  ball_t ball_;
  int lives_;
  game_state_e state_;
  bounce_fn_t bounce_fn_;

  void try_move_ball() {
    if (state_ != game_state_e::launched) {
      ball_.position_.first = paddle_.position_.first;
    }
  }

  void launch(std::pair<int, int> velocity) {
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
