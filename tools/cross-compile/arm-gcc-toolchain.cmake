set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR ARM)

if(MINGW OR CYGWIN OR WIN32)
    set(UTIL_SEARCH_CMD where)
elseif(UNIX OR APPLE)
    set(UTIL_SEARCH_CMD which)
endif()


# ---- for arm-linux begin ----
# cmake -DCMAKE_TOOLCHAIN_FILE=../tools/cross-compile/arm-gcc-toolchain.cmake ../
#
# install: sudo apt install gcc-arm-linux-gnueabihf
# set(TOOLCHAIN_PREFIX arm-linux-gnueabihf-)

# install: sudo apt install gcc-arm-linux-gnueabi
set(TOOLCHAIN_PREFIX arm-linux-gnueabi-)

# for other embedded systems.
# set(TOOLCHAIN_PREFIX arm-none-eabi-)
# ---- for arm-linux end ----


execute_process(
  COMMAND ${UTIL_SEARCH_CMD} ${TOOLCHAIN_PREFIX}gcc
  OUTPUT_VARIABLE BINUTILS_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Without that flag CMake is not able to pass test compilation check
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

# set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)

# set(ARM_TOOLCHAIN_DIR "")
# or 
# set(ARM_TOOLCHAIN_DIR "/path/to/your/custom/directory/bin")
# set(CMAKE_OBJCOPY ${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
# set(CMAKE_SIZE_UTIL ${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}size CACHE INTERNAL "size tool")

set(CMAKE_FIND_ROOT_PATH ${BINUTILS_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

