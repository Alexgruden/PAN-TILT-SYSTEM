### Dependencies
`brew install cmake ninja openocd`

### Set up
- Ensure you have:
    - STM32CubeMX
    - STM32CubeCLT
These can be downloaded from the official st website: st.com

- Configure STM32CubeMX to use CMake as the Toolchain/IDE
- Enable under Project Manager → Code Generator:
    - Generate peripheral initialization as a pair of .c/.h files per peripheral
    - Keep User Code when re-generating
    - Copy only the necessary library files”

- Pinout & Configuration → System Core → SYS
    - Debug: Serial Wire
    - Timebase Source: TIM6 (needed for rtos)