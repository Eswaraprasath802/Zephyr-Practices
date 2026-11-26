# Zephyr-Practices

Hands-on practice repository for learning Zephyr RTOS on embedded boards (Raspberry Pi Pico and ESP32). Topics: Zephyr architecture, west workspace, devicetree and overlays, GPIO, interrupts, PWM, threads, message queues, mutexes, UART, I2C, SPI and drivers.

## Latest example: multithreaded sensor logger with a mutex (`src/sensor_logger.c`)

Builds on the message queue example. Two producer threads send readings through a `k_msgq`, a logger thread stores them in shared data, and a reporter thread reads that data. A mutex (`K_MUTEX_DEFINE(log_mtx)`) keeps the shared data consistent.

1. **Producers:** `soil_th` (id 1, every 500 ms) and `ldr_th` (id 2, every 300 ms) share one `producer()` function and put a `struct sensor_msg` (`id`, `ts`, `value`) on the queue with a 10 ms timeout.
2. **Logger (`log_th`, priority 4):** takes messages from the queue and, while holding the mutex, writes them into a 32-entry ring buffer (`log_buf`) and updates per-sensor counts and sums in `struct stats`. A 20 ms `k_busy_wait()` inside the locked section pretends to be a slow flash write, which widens the race window on purpose. It also toggles the LED (`gled`).
3. **Reporter (`rep_th`, priority 3, the highest):** every 500 ms it locks, copies the stats, unlocks, then checks the invariant `count[1] + count[2] == total` and prints a summary every 10 rounds.
4. **Race demo:** set `USE_LOCK` to `0` at the top of the file. The reporter can then preempt the logger between `count[id]++` and `total++` and prints `!! INCONSISTENT`. With `USE_LOCK 1` the message never appears.
5. **Design points:** the reporter copies the stats and prints after unlocking so the lock is held for as short a time as possible. Zephyr mutexes use priority inheritance, so the higher-priority reporter waiting on the lock boosts the logger.

Tested hardware: Raspberry Pi Pico (`rpi_pico`, LED on GPIO 25) and ESP32 DevKitC (`esp32_devkitc_wroom/esp32/procpu`, LED on GPIO 2, Zephyr 3.7.0).

## Other examples in `src/`

| File | What it does |
|------|--------------|
| `sensor_logger.c` | Producers + logger + reporter, shared data protected by a mutex (`USE_LOCK` race demo) |
| `msgq_sample.c` | Two producer threads + one consumer over a message queue (`k_msgq`, 8 slots, 10 ms put timeout) |
| `pwm_blink.c` | GPIO blink + button interrupt + PWM breathing thread (Pico, GPIO 20 LED + GPIO 21 button) |
| `interrupt.c` | GPIO blink + button interrupt callback |
| `main.c` | RGB LED on/off sequence on three GPIOs (needs its own `redled`/`greenled`/`blueled` overlay nodes, the old `app.overlay` was removed) |

Every file defines its own `main()`, so only **one** file can be built at a time. Choose it in `CMakeLists.txt`:

```cmake
target_sources(app PRIVATE src/sensor_logger.c)
```

`target_sources()` does not expand wildcards, so list the file name explicitly (not `src/*.c`).

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
    ├── msgq_sample.c       # Message queue: producers + consumer
    └── sensor_logger.c     # Message queue + mutex-protected shared data
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
| `gled` | `gpio-leds` | `gpio0` pin 2 | active high, on-board LED used by `msgq_sample.c` and `sensor_logger.c` |

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
- [x] Multithreaded sensor data logger with synchronization (mutex, race demo)
- [ ] Preemptive vs cooperative threads (two LEDs)
- [ ] Semaphore (button ISR wakes a thread to turn on an LED)
- [ ] Mutex with a shared peripheral (two threads sharing an I2C LCD)
- [ ] Priority inversion with three threads
- [ ] Timers: one-shot, periodic, start/stop/restart with buttons
- [ ] Timer + workqueue, GPIO interrupt + workqueue
- [ ] UART
- [ ] I2C
- [ ] SPI
- [ ] Custom drivers

## Author

Eswaraprasath
