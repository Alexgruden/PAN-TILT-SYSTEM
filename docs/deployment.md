## Toolchain

### Controller Node
```
| Tool              | Version | Purpose               |
|-------------------|---------|-----------------------|
| STM32CubeCLT      |         | Compiler and debugger |
| arm-none-eabi-gcc | 14.3.1  | C compiler            |
| CMake             |  4.4.4  | Build configuration   |
| Ninja             | 1.13.2  | Build execution       |
| OpenOCD           | 0.12.0  | Flashing and debug    |
| STM32CubeMX       |         | Code generation       |
```

These can be downloaded from the official st website: st.com

- Configure STM32CubeMX to use CMake as the Toolchain/IDE
- Enable under Project Manager → Code Generator:
    - Generate peripheral initialization as a pair of .c/.h files per peripheral
    - Keep User Code when re-generating
    - Copy only the necessary library files”

- Pinout & Configuration → System Core → SYS
    - Debug: Serial Wire
    - Timebase Source: TIM6 (needed for rtos)

### Handheld Node

```
| Tool    | Version | Purpose                                                     |
|---------|---------|-------------------------------------------------------------|
| ESP-IDF | v6.1    | Framework, compiler, and build/flash tool (`idf.py`)        |
```

Installed with EIM (Espressif Installation Manager)

Activate each new terminal:
 ```bash
source ~/.espressif/tools/activate_idf_v6.1.sh
```

Build, Flash, Monitor

From `firmware/esp32/`:

```bash
idf.py set-target esp32   # first time only
idf.py build
idf.py -p PORT flash monitor
```

`PORT` is the board's serial port (`/dev/cu.*` on macOS). Exit the monitor with `Ctrl+]`.