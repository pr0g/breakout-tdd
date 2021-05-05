#pragma once

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

void step(const paddle_t& paddle, ball_t& ball) {
  ball.position_.first += ball.velocity_.first;
  ball.position_.second += ball.velocity_.second;
  if (intersects(paddle, ball)) {
    ball.velocity_.second *= -1;
  }
}

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
  }

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

  [[nodiscard]] int blocks_horizontal() const { return 11; }
  [[nodiscard]] int blocks_vertical() const { return 9; }
  [[nodiscard]] int block_width() const { return 8; }
  [[nodiscard]] int block_height() const { return 1; }
  [[nodiscard]] int vertical_padding() const { return 1; }
  [[nodiscard]] int horizontal_padding() const { return 2; }
  [[nodiscard]] int horizontal_spacing() const { return 1; }
  [[nodiscard]] int vertical_spacing() const { return 1; }

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
    int move = std::min(paddle_left_edge() - 1, distance);
    if (paddle_left_edge() > 1) {
      paddle_.position_.first -= move;
    }
    try_move_ball();
  }

  void move_paddle_right(const int distance) {
    int move = std::min(board_size_.first - paddle_right_edge() - 1, distance);
    if (paddle_right_edge() < board_size_.first) {
      paddle_.position_.first += move;
    }
    try_move_ball();
  }

  void step() {
    if (state_ == game_state_e::launched) {
      ::step(paddle_, ball_);
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
    for (int row = 0; row < blocks_vertical(); ++row) {
      for (int col = 0; col < blocks_horizontal(); ++col) {
        for (int block_part = 0; block_part < block_width(); ++block_part) {
          display.output(
            board_x + horizontal_padding() + block_part
              + ((block_width() + horizontal_spacing()) * col),
            board_y + vertical_padding()
              + ((block_height() + vertical_spacing()) * row));
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
