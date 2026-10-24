#pragma once
class Servo {
 public:
  Servo();
  void attach(int pin);
  void detach();
  void write(int angle);
  int read() const { return cmd_; }
  void powerCycle() { cmd_ = 90; attached_ = false; }  // library state after a reset
 private:
  int pin_ = -1;
  int cmd_ = 90;
  bool attached_ = false;
};
