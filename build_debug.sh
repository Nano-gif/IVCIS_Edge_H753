#!/bin/bash
# Build script for Debug configuration
GCC_DIR="/e/Program Files/STM32CubeIDE_2.0.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.win32_1.0.100.202509120712/tools/bin"
MAKE="/e/Program Files/STM32CubeIDE_2.0.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.0.202409170845/tools/bin/make.exe"

export PATH="$GCC_DIR:$PATH"
cd "$(dirname "$0")/Debug" || exit 1
"$MAKE" "$@"
