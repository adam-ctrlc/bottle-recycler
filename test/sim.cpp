// Host-side simulation of the bottle recycler sketch: fake time, fake servos
// with a physical angle that survives power cuts, fake bottle in front of the
// sensor, fake LCD. Runs scenarios and checks coins paid.
#include <cstdio>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "Arduino.h"
#include "EEPROM.h"
#include "Servo.h"
#include "Wire.h"
#include "hd44780ioClass/hd44780_I2Cexp.h"

void setup();
void loop();

EEPROMClass EEPROM;
TwoWire Wire;
bool lcdPresent = true;
std::vector<Screen> screens;

// ---- world state ----
static uint64_t now_us = 0;
static int phys[20];          // physical servo angle per pin (survives power cuts)
static int coins = 0;
static int flapOpenings = 0;
static int coinMaxAngle = 0;
static int led13 = LOW;
static int boots = 0;

// bottle model
static bool bottleOnFlap = false;     // a bottle is sitting at the flap
static bool bottleVisible = true;     // sensor can see the bottle while it sits there
static bool bottleFlickers = false;   // clear plastic: sensor drops out briefly
static int stuckForOpenings = 0;      // flap openings that fail to take the bottle
static uint64_t pulseFrom = 0, pulseTo = 0;  // short "rolling past" pulse
static uint64_t handHeldUntil = 0;    // something held in front of the sensor
static std::function<void()> atTime;  // scenario events
static uint64_t eventAt = UINT64_MAX;
static std::function<bool()> cutWhen; // power cut trigger (fires once)

struct PowerCut {};
struct Deadline {};
static uint64_t deadline_us = UINT64_MAX;
static bool hung = false;

static void checkCut() {
  if (cutWhen && cutWhen()) {
    cutWhen = nullptr;
    throw PowerCut();
  }
}

static void tick(uint64_t us) {
  now_us += us;
  if (now_us > deadline_us) throw Deadline();
  if (now_us >= eventAt) {
    eventAt = UINT64_MAX;
    auto f = atTime;
    atTime = nullptr;
    if (f) f();
  }
  checkCut();
}

unsigned long millis() { tick(1); return (unsigned long)(now_us / 1000); }
void delay(unsigned long ms) { for (unsigned long i = 0; i < ms; i++) tick(1000); }
void pinMode(int, int) {}
void digitalWrite(int pin, int v) { if (pin == 13) led13 = v; }

int digitalRead(int pin) {
  tick(5);
  if (pin != 4) return HIGH;
  bool sees = false;
  if (bottleOnFlap && bottleVisible) {
    sees = !(bottleFlickers && ((now_us / 1000) % 200) < 40);
  }
  if (now_us >= pulseFrom && now_us < pulseTo) sees = true;
  if (now_us < handHeldUntil) sees = true;
  return sees ? LOW : HIGH;  // module pulls D0 LOW when it sees something
}

void EEPROMClass::update(int a, uint8_t v) { mem[a] = v; }

static std::vector<Servo *> &registry() { static std::vector<Servo *> r; return r; }

static void physMove(int pin, int angle) {
  int before = phys[pin];
  phys[pin] = angle;
  if (pin == 2) {
    if (before < 180 && angle >= 180) coins++;
    if (angle > coinMaxAngle) coinMaxAngle = angle;
  }
  if (pin == 3 && before < 45 && angle >= 45) {
    flapOpenings++;
    if (bottleOnFlap) {
      if (stuckForOpenings > 0) stuckForOpenings--;
      else bottleOnFlap = false;  // bottle drops in
    }
  }
  checkCut();
}

Servo::Servo() { registry().push_back(this); }
void Servo::attach(int pin) { pin_ = pin; attached_ = true; physMove(pin, cmd_); }
void Servo::detach() { attached_ = false; }
void Servo::write(int a) { cmd_ = a; if (attached_) physMove(pin_, a); }

// ---- harness ----
static void resetWorld(bool freshMagic) {
  now_us = 0; coins = 0; flapOpenings = 0; coinMaxAngle = 0; boots = 0; led13 = LOW;
  for (int &p : phys) p = 0;
  bottleOnFlap = false; bottleVisible = true; bottleFlickers = false; stuckForOpenings = 0;
  pulseFrom = pulseTo = 0; handHeldUntil = 0;
  atTime = nullptr; eventAt = UINT64_MAX; cutWhen = nullptr;
  lcdPresent = true; screens.clear(); hung = false;
  for (auto &b : EEPROM.mem) b = 0xFF;
  if (freshMagic) {  // as left by a previous clean run of this sketch
    EEPROM.mem[0] = 0xB7; EEPROM.mem[1] = 0; EEPROM.mem[2] = 0; EEPROM.mem[3] = 0;
  }
}

static void runUntil(uint64_t until_us) {
  bool needBoot = (boots == 0);
  deadline_us = until_us + 5000000;  // stuck in one call for 5 s past the end = hung
  while (now_us < until_us) {
    try {
      if (needBoot) {
        for (Servo *s : registry()) s->powerCycle();
        boots++;
        needBoot = false;
        setup();
      }
      loop();
    } catch (PowerCut &) {
      needBoot = true;
    } catch (Deadline &) {
      hung = true;
      break;
    }
  }
  deadline_us = UINT64_MAX;
}

static uint64_t sec(double s) { return (uint64_t)(s * 1e6); }

static bool sawScreen(const char *line1) {
  for (auto &s : screens) if (s.line[0].find(line1) != std::string::npos) return true;
  return false;
}

static std::string lastScreen() {
  if (screens.empty()) return "(none)";
  return screens.back().line[0] + " | " + screens.back().line[1];
}

static int failures = 0;
static void expect(const char *name, bool ok, const std::string &detail) {
  printf("%s %-46s %s\n", ok ? "PASS" : "FAIL", name, detail.c_str());
  if (!ok) failures++;
}

static std::string summary() {
  char b[200];
  snprintf(b, sizeof b, "coins=%d flaps=%d boots=%d%s last=[%s]", coins, flapOpenings, boots,
           hung ? " HUNG" : "", lastScreen().c_str());
  return b;
}

static bool readyAtEnd() { return !hung && lastScreen().find("Insert a bottle") != std::string::npos; }

// Boot, wait for the ready screen, then drop a bottle in at t = 10 s.
static void bootIdle() { runUntil(sec(10)); }

int main() {
  setvbuf(stdout, NULL, _IONBF, 0);
  // A: bottle rolls past the sensor in 20 ms and lands on the flap out of view
  resetWorld(true); bootIdle();
  pulseFrom = now_us; pulseTo = now_us + 20000; bottleOnFlap = true; bottleVisible = false;
  runUntil(sec(30));
  expect("A fast rolling bottle -> 1 coin", coins == 1 && flapOpenings == 1 && readyAtEnd(), summary());

  // B: clear bottle sits in front of the sensor, flickering, until the flap takes it
  resetWorld(true); bootIdle();
  bottleOnFlap = true; bottleFlickers = true;
  runUntil(sec(30));
  expect("B flickering clear bottle -> 1 coin, not 2", coins == 1 && flapOpenings == 1 && readyAtEnd(), summary());

  // C: bottle jammed for good, removed by hand at t = 40 s
  resetWorld(true); bootIdle();
  bottleOnFlap = true; stuckForOpenings = 99;
  eventAt = sec(40); atTime = [] { bottleOnFlap = false; };
  runUntil(sec(60));
  expect("C stuck bottle -> 3 tries, no coin", coins == 0 && flapOpenings == 3 && sawScreen("Bottle is stuck") &&
         readyAtEnd() && EEPROM.mem[3] == 0, summary());

  // D: jammed on the first try, goes in on the second
  resetWorld(true); bootIdle();
  bottleOnFlap = true; stuckForOpenings = 1;
  runUntil(sec(40));
  expect("D retry frees bottle -> 1 coin", coins == 1 && flapOpenings == 2 && sawScreen("One moment") && readyAtEnd(), summary());

  // E: power cut while the flap is opening (bottle already dropped)
  resetWorld(true); bootIdle();
  bottleOnFlap = true;
  cutWhen = [] { return phys[3] >= 60 && phys[3] < 90; };
  runUntil(sec(40));
  expect("E power cut during flap -> coin paid on restart", coins == 1 && boots == 2 && readyAtEnd() && EEPROM.mem[3] == 0, summary());

  // F: power cut halfway through the coin push
  resetWorld(true); bootIdle();
  bottleOnFlap = true;
  cutWhen = [] { return phys[2] == 90 && coins == 0; };
  runUntil(sec(40));
  expect("F power cut during coin push -> exactly 1 coin", coins == 1 && boots == 2 && readyAtEnd(), summary());

  // G: power cut on the coin's return stroke (coin already out)
  resetWorld(true); bootIdle();
  bottleOnFlap = true;
  cutWhen = [] { return phys[2] == 90 && coins == 1; };
  runUntil(sec(40));
  expect("G power cut on return stroke -> still 1 coin", coins == 1 && boots == 2 && readyAtEnd(), summary());

  // H: power cut while the coin servo holds at 180
  resetWorld(true); bootIdle();
  bottleOnFlap = true;
  {
    static uint64_t reached = 0;
    reached = 0;
    cutWhen = [] {
      if (phys[2] == 180 && reached == 0) reached = now_us;
      return reached && now_us > reached + 200000;
    };
  }
  runUntil(sec(40));
  expect("H power cut during coin hold -> still 1 coin", coins == 1 && boots == 2 && readyAtEnd(), summary());

  // I: EEPROM full of bytes from some other sketch
  resetWorld(false);
  EEPROM.mem[0] = 0x12; EEPROM.mem[1] = 150; EEPROM.mem[2] = 77; EEPROM.mem[3] = 1;
  runUntil(sec(20));
  expect("I leftover EEPROM -> no servo swing, no coin", coins == 0 && coinMaxAngle == 0 && flapOpenings == 0 &&
         readyAtEnd() && EEPROM.mem[0] == 0xB7, summary());

  // J: second bottle put in while "Thank you!" is showing
  resetWorld(true); bootIdle();
  bottleOnFlap = true;
  {
    static bool queued = false;
    queued = false;
    cutWhen = [] {  // not a cut: just watches for the thank-you screen
      if (!queued && sawScreen("Thank you")) { queued = true; bottleOnFlap = true; }
      return false;
    };
  }
  runUntil(sec(60));
  expect("J second bottle during thank-you -> 2 coins", coins == 2 && flapOpenings == 2 && readyAtEnd(), summary());

  // K: LCD missing: machine still pays, LED on
  resetWorld(true); lcdPresent = false; bootIdle();
  bottleOnFlap = true;
  runUntil(sec(30));
  expect("K no LCD -> still 1 coin, onboard LED on", coins == 1 && led13 == HIGH, summary());

  // L: nothing happens for a minute -> no coins, servos parked
  resetWorld(true);
  runUntil(sec(60));
  expect("L idle -> no coins", coins == 0 && flapOpenings == 0 && readyAtEnd() && phys[2] == 0 && phys[3] == 0, summary());

  printf("\n%s (%d failed)\n", failures ? "SOME FAILED" : "ALL PASSED", failures);
  return failures ? 1 : 0;
}
