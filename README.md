# Bottle Recycler

An Arduino machine that gives a coin for every plastic bottle. A TCRT5000 sensor detects the bottle, one servo takes the bottle in, and a second servo pushes a coin out. A 16x2 I2C LCD tells the user what to do.

## How it works

1. The LCD shows **"Insert a bottle / to get a coin"**.
2. The TCRT5000 sensor sees the bottle (even one rolling past quickly).
3. Servo 2 opens the bottle flap (0° → 90° → 0°).
4. Servo 1 turns 0° → 180° → 0° to push one coin out.
5. The LCD shows **"Thank you! / Take your coin"**.
6. If the bottle is still in front of the sensor, the LCD asks the user to remove it, so one bottle can't pay twice.

The servos remember their last angle (saved in EEPROM), so after a power cut they sweep slowly back to 0° instead of snapping.

## Parts

- Arduino Uno
- 2 × hobby servo (SG90 / MG90S / MG996R)
- TCRT5000 reflective sensor module
- 16x2 LCD with I2C backpack
- 12V power adapter (1A for small servos, 2A for MG996R)
- LM2596 buck (step-down) converter, set to 5.0–5.2V
- 1000 µF capacitor (10V or higher)

## Wiring

| Part | Connects to |
|---|---|
| Servo 1 (coin) signal | D2 |
| Servo 2 (bottle) signal | D3 |
| TCRT5000 D0 | D4 |
| LCD SDA / SCL | Uno pins labeled SDA / SCL |
| LCD and TCRT5000 VCC / GND | Arduino 5V / GND |

### Power

```
12V adapter (+) ──┬──> Arduino barrel jack (or VIN)       → powers Arduino, LCD, sensor
                  └──> Buck IN+
12V adapter (−) ──┬──> Arduino GND
                  └──> Buck IN−

Buck OUT+ (5V) ───> Servo 1 red + Servo 2 red
Buck OUT− ────────> Servo 1 brown + Servo 2 brown + Arduino GND
1000 µF capacitor across Buck OUT+ / OUT− (stripe to −)
```

> ⚠️ Set the buck converter to 5.0–5.2V with a multimeter **before** connecting the servos. 12V will destroy them. Never connect 12V to the Arduino's 5V pin.

The servos must not be powered from the Arduino's 5V pin. They pull too much current, which makes the LCD flicker and the Arduino reset.

## Libraries

- **Servo** (comes with the Arduino IDE)
- **hd44780** by Bill Perry (install from the Library Manager). It finds the LCD's I2C address automatically.

## Uploading

The Arduino IDE needs the folder to have the same name as the `.ino` file, so download it into a folder called `bottle_recycler`:

```sh
git clone https://github.com/adam-ctrlc/bottle-recycler.git bottle_recycler
```

If you use **Code → Download ZIP** instead, rename the unzipped folder to `bottle_recycler`.

Then open `bottle_recycler.ino` in the Arduino IDE, choose **Arduino Uno**, and click Upload.

Or with `arduino-cli`, from inside the folder:

```sh
arduino-cli lib install hd44780
arduino-cli compile --fqbn arduino:avr:uno .
arduino-cli upload -p <your-port> --fqbn arduino:avr:uno .
```

## Files

| File | What's in it |
|---|---|
| `bottle_recycler.ino` | Main program and the LCD messages |
| `config.h` | **All pins, angles and timings — change settings here** |
| `display.h/.cpp` | LCD |
| `sensor.h/.cpp` | TCRT5000 bottle sensor |
| `servos.h/.cpp` | Coin servo and bottle servo |

## Troubleshooting

| Problem | Fix |
|---|---|
| Sensor triggers with no bottle, or never triggers | Turn the small screw on the TCRT5000 module. Its LED lights when it sees something. If it works backwards, set `SENSOR_DETECTED = HIGH` in `config.h`. |
| Fast bottles are missed | Mount the sensor 2–10 mm from where the bottle passes. Lower `MIN_DETECT_TIME` in `config.h`. |
| LCD flickers / servos don't move | Power problem. Use the buck converter wiring above. |
| LCD lights up but shows no text | Turn the contrast screw on the back of the LCD. |
| LCD stays dark, onboard LED blinks | LCD not found. Check the SDA, SCL, VCC and GND wires. |
