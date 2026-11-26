# Zephyr-Practices

Hands-on practice repository for learning Zephyr RTOS on embedded boards (Raspberry Pi Pico and ESP32). Topics: Zephyr architecture, west workspace, devicetree and overlays, GPIO, interrupts, PWM, threads, message queues, UART, I2C, SPI and drivers.

## Current example: multithreaded message passing (`src/msgq_sample.c`)

This is the file built by `CMakeLists.txt` right now. It uses a Zephyr message queue (`k_msgq`) to pass sensor readings from two producer threads to one consumer thread:

1. **Message format:** `struct sensor_msg` carries the producer `id`, an uptime timestamp `ts` (ms) and a `value`.
2. **Queue:** `K_MSGQ_DEFINE(sensor_q, sizeof(struct sensor_msg), 8, 4)` holds up to 8 messages, 4-byte aligned.
3. **Producers:** `soil_th` (id 1, every 500 ms) and `ldr_th` (id 2, every 300 ms) share one `producer()` function. Each gets its own `struct producer_cfg` through `p1`. They send fake readings with `k_msgq_put()` and a 10 ms timeout, so a full queue drops the message and prints a warning instead of blocking forever.
4. **Consumer:** `cons_th` (priority 4, higher than the producers at 5) sleeps in `k_msgq_get(..., K_FOREVER)` until a message arrives, toggles the LED (`gled`) and prints the message with `printk()`, including how many messages are still queued.
5. **Threads:** all three are created statically with `K_THREAD_DEFINE` (1024 B stacks); `main()` only prints a start message and returns.

Tested on the Raspberry Pi Pico (`rpi_pico`, LED on GPIO 25) and on the ESP32 DevKitC (`esp32_devkitc_wroom/esp32/procpu`, LED on GPIO 2, Zephyr 3.7.0).

## Other sources in `src/`

| File | What it does |
|------|--------------|
| `msgq_sample.c` | Two producer threads + one consumer over a message queue (current build) |
| `pwm_blink.c` | GPIO blink + button interrupt + PWM breathing thread (Pico, GPIO 20 LED + GPIO 21 button) |
| `interrupt.c` | GPIO blink + button interrupt callback |
| `main.c` | RGB LED on/off sequence on three GPIOs (needs its own `redled`/`greenled`/`blueled` overlay nodes, the old `app.overlay` was removed) |

To build a different example, change the file in `target_sources(app PRIVATE ...)` in `CMakeLists.txt`.

## Project structure

```
.
├── CMakeLists.txt                          # Zephyr app build file (selects the source file)
├── prj.conf                                # Kconfig options
├── boards/
│   ├── rpi_pico.overlay                    # Pico overlay: LED, button, PWM LED, pinctrl
│   └── esp32_devkitc_wroom_procpu.overlay  # ESP32 overlay: LED on GPIO 2
└── src/
    ├── main.c              # RGB LED sequence
    ├── interrupt.c         # Button interrupt example
    ├── pwm_blink.c         # PWM breathing + interrupt + thread
    └── msgq_sample.c       # Message queue: producers + consumer
```

## Devicetree overlays (`boards/`)

Zephyr applies `boards/<board>.overlay` automatically when building for that board.

### `rpi_pico.overlay`

| Node label | Type | Pin / channel | Notes |
|------------|------|---------------|-------|
| `gled` | GPIO LED | `gpio0` pin 25 | active high |
| `first_button` | `gpio-keys` button | `gpio0` pin 21 | active low, pull-up |
| `pwm_led` | `pwm-leds` | PWM channel 4 (`pwm`), 1 MHz period | normal polarity |
| `pwm_ch2a_default` | pinctrl | `PWM_2A_P20` | PWM channel 2A on GPIO 20 |

The `&pwm` node is enabled with `divider-int-2 = <5>` and `divider-frac-2 = <15>`.

### `esp32_devkitc_wroom_procpu.overlay`

| Node label | Type | Pin | Notes |
|------------|------|-----|-------|
| `gled` | `gpio-leds` | `gpio0` pin 2 | active high, on-board LED used by `msgq_sample.c` |

## Requirements

- Zephyr RTOS and SDK, set up with a west workspace
- `ZEPHYR_BASE` environment variable set (used by `CMakeLists.txt`)
- Raspberry Pi Pico or ESP32 DevKitC
- For `pwm_blink.c` / `interrupt.c`: an LED on GPIO 20 (through a resistor) and a button on GPIO 21 to ground

## Build and flash

```bash
# from your west workspace, with the Zephyr environment active

# Raspberry Pi Pico
west build -b rpi_pico /path/to/Zephyr-Practices -p always
west flash

# ESP32 DevKitC
west build -b esp32_devkitc_wroom/esp32/procpu /path/to/Zephyr-Practices -p always
west flash
```

Use any serial terminal at the board's console baud rate to see the `printk()` output.

## Learning roadmap

- [x] GPIO output with devicetree and overlays
- [x] GPIO interrupts (button callback)
- [x] PWM
- [x] Threads (`K_THREAD_DEFINE`)
- [x] Message queues (multithreaded message passing)
- [ ] Multithreaded sensor data logger with synchronization
- [ ] Preemptive vs cooperative threads (two LEDs)
- [ ] Semaphore (button ISR wakes a thread to turn on an LED)
- [ ] Mutex (two threads sharing an I2C LCD)
- [ ] Priority inversion with three threads
- [ ] Timers: one-shot, periodic, start/stop/restart with buttons
- [ ] Timer + workqueue, GPIO interrupt + workqueue
- [ ] UART
- [ ] I2C
- [ ] SPI
- [ ] Custom drivers

## Author

Eswaraprasath
