cmake_minimum_required(VERSION 3.5)

project (fixed)

set(RISCV_GCC_INSTALL_ROOT /opt/RISCV CACHE PATH "Path to GCC for RISC-V cross compiler installation directory")
set(CMAKE_SYSROOT ${RISCV_GCC_INSTALL_ROOT}/sysroot CACHE PATH "RISC-V sysroot")

set(CMAKE_C_COMPILER ${RISCV_GCC_INSTALL_ROOT}/bin/riscv64-unknown-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER ${RISCV_GCC_INSTALL_ROOT}/bin/riscv64-unknown-linux-gnu-g++)

set(VLEN 128 CACHE STRING "RISC-V VLEN, default=128")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" CACHE STRING "Set C++ Compiler Flags" FORCE)
add_definitions("-D__RVV_VLEN__=${VLEN}")

add_executable(testAPI.out testAPI.cpp)
add_executable(gemm.out gemm.cpp)
