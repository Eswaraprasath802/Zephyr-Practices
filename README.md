# Zephyr-Practices

Hands-on practice repository for learning Zephyr RTOS on embedded boards (ESP32 and Raspberry Pi Pico). Topics: Zephyr architecture, west workspace, devicetree and overlays, GPIO, UART, I2C, SPI, drivers, threads and synchronization.

## Current example: RGB LED sequence (GPIO)

`src/main.c` drives three LEDs (red, green, blue) through the devicetree-based GPIO API:

1. Gets each LED from the devicetree by node label (`redled`, `greenled`, `blueled`).
2. Builds a `gpio_dt_spec` for each and checks it with `gpio_is_ready_dt()`.
3. Configures each pin as output with `gpio_pin_configure_dt()`.
4. Loops forever: turns red, green, blue ON one by one (1 s apart), then OFF one by one (1 s apart).

Errors at each step are reported with `printk()`.

## Project structure

```
.
├── CMakeLists.txt   # Zephyr app build file
├── prj.conf         # Kconfig options
├── app.overlay      # Devicetree overlay defining the LED nodes
└── src/
    └── main.c       # Application code
```

## Devicetree overlay

`app.overlay` defines the LEDs on `gpio0`, active high:

| Node label | GPIO | Pin | Label |
|------------|------|-----|-------|
| `redled`   | `gpio0` | 0 | RLED |
| `greenled` | `gpio0` | 1 | GLED |
| `blueled`  | `gpio0` | 2 | BLED |

Change the pin numbers or GPIO controller to match your board and wiring.

## Requirements

- Zephyr RTOS and SDK, set up with a west workspace
- `ZEPHYR_BASE` environment variable set (used by `CMakeLists.txt`)
- A supported board (e.g. ESP32 or Raspberry Pi Pico) and 3 LEDs with resistors

## Build and flash

```bash
# from your west workspace, with the Zephyr environment active
west build -b <board> /path/to/Zephyr-Practices -p always
west flash
```

Replace `<board>` with your target board name (e.g. `esp32_devkitc/esp32/procpu`, `rpi_pico`). The `app.overlay` in the app folder is picked up automatically.

## Monitor output

```bash
west espressif monitor   # ESP32
# or use any serial terminal at the board's console baud rate
```

## Learning roadmap

- [x] GPIO output with devicetree and overlays
- [ ] UART
- [ ] I2C
- [ ] SPI
- [ ] Threads and synchronization
- [ ] Custom drivers

## Author

Eswaraprasath
