# Zephyr-Practices

Hands-on practice repository for learning Zephyr RTOS on embedded boards (Raspberry Pi Pico and ESP32). Topics: Zephyr architecture, west workspace, devicetree and overlays, GPIO, interrupts, PWM, threads, UART, I2C, SPI and drivers.

## Current example: PWM breathing LED + button interrupt (`src/pwm_blink.c`)

This is the file built by `CMakeLists.txt` right now. It targets the Raspberry Pi Pico (`rpi_pico`):

1. **GPIO LED:** the green LED (`gled`, GPIO 25) blinks in `main()`, 1 s on / 1 s off.
2. **Button interrupt:** a button (`first_button`, GPIO 21, active low with pull-up) is set up with `gpio_pin_interrupt_configure_dt()` on `GPIO_INT_EDGE_TO_INACTIVE`. The callback prints `my button pressed`.
3. **PWM breathing:** a PWM LED (`pwm_led`, PWM channel 2A on GPIO 20, 1 MHz period) fades from 0 to 100% duty and back.
4. **Thread:** the breathing runs in its own thread (`breathing_th1`, 4096 B stack, priority 5, 300 ms start delay) with a 50 ms step, printing brightness and direction through `printk()`.

## Other sources in `src/`

| File | What it does |
|------|--------------|
| `pwm_blink.c` | GPIO blink + button interrupt + PWM breathing thread (current build) |
| `interrupt.c` | GPIO blink + button interrupt callback |
| `main.c` | RGB LED on/off sequence on three GPIOs (needs its own `redled`/`greenled`/`blueled` overlay nodes, the old `app.overlay` was removed) |

To build a different example, change the file in `target_sources(app PRIVATE ...)` in `CMakeLists.txt`.

## Project structure

```
.
├── CMakeLists.txt          # Zephyr app build file (selects the source file)
├── prj.conf                # Kconfig options
├── boards/
│   └── rpi_pico.overlay    # Board overlay: LED, button, PWM LED, pinctrl
└── src/
    ├── main.c              # RGB LED sequence
    ├── interrupt.c         # Button interrupt example
    └── pwm_blink.c         # PWM breathing + interrupt + thread
```

## Devicetree overlay (`boards/rpi_pico.overlay`)

Zephyr applies `boards/<board>.overlay` automatically when building for that board.

| Node label | Type | Pin / channel | Notes |
|------------|------|---------------|-------|
| `gled` | GPIO LED | `gpio0` pin 25 | active high |
| `first_button` | `gpio-keys` button | `gpio0` pin 21 | active low, pull-up |
| `pwm_led` | `pwm-leds` | PWM channel 4 (`pwm`), 1 MHz period | normal polarity |
| `pwm_ch2a_default` | pinctrl | `PWM_2A_P20` | PWM channel 2A on GPIO 20 |

The `&pwm` node is enabled with `divider-int-2 = <5>` and `divider-frac-2 = <15>`.

## Requirements

- Zephyr RTOS and SDK, set up with a west workspace
- `ZEPHYR_BASE` environment variable set (used by `CMakeLists.txt`)
- Raspberry Pi Pico, an LED on GPIO 20 (through a resistor) and a button on GPIO 21 to ground

## Build and flash

```bash
# from your west workspace, with the Zephyr environment active
west build -b rpi_pico /path/to/Zephyr-Practices -p always
west flash
```

Use any serial terminal at the board's console baud rate to see the `printk()` output.

## Learning roadmap

- [x] GPIO output with devicetree and overlays
- [x] GPIO interrupts (button callback)
- [x] PWM
- [x] Threads (`K_THREAD_DEFINE`)
- [ ] Synchronization (mutex, semaphore)
- [ ] UART
- [ ] I2C
- [ ] SPI
- [ ] Custom drivers

## Author

Eswaraprasath
