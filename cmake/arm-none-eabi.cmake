set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_CROSSCOMPILING   TRUE)

set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

# Compile-only smoke tests; no startup/linker script available.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT DEFINED ARM_CPU)
    set(ARM_CPU cortex-m33 CACHE STRING "Target ARM Cortex-M core")
endif()

set(_arm_flags "-mcpu=${ARM_CPU} -mthumb -ffunction-sections -fdata-sections")

set(CMAKE_C_FLAGS_INIT   "${_arm_flags}")
set(CMAKE_CXX_FLAGS_INIT "${_arm_flags} -fno-exceptions -fno-rtti")