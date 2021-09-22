# Universal Intrinsic for Vector

This is an experimental repository about universal intrinsic for vector.

More specifically, we want to know how to abstract the native intrinsic of an ISA with variable vector length (such as RISC-V V extension and ARM SVE).

A more specific goal is to provide variable-length RVV support for the UI(WUI) in OpenCV.

We want to be as compatible as possible, which means modify the existing code as little as possible.

Compared with existing Universal Intrinsic in OpenCV, Universal Intrinsic for Vector supports variable vector length and setVL[1], maintaining interface compatibility.

[1] setVL is a feature in RVV Native Intrinsic. It is useful when dealing with the end of a loop. But it modified the UI interface. And there is *NOT* plan to submit this feature upstream currently, unless we evaluate that all backends of the UI can support setVL at an acceptable cost.

## Technical information and decision

Again, this is an experimental repository, so all the following technical information is open to discussion, modify and rewrite.

Tips: All `RVV` appearing below refer to the RVV version 0.10, and the toolchain and intrinsic are also 0.10 version.

### Runtime VLEN or Compile-time VLEN

**Compile-time VLEN for now.**

In fact, there is no compile-time VLEN in RVV. In other words, the compiler does NOT and should NOT know VLEN during compiling time. The RVV binary code should run on every RVV target with different VLEN and use the entire register as much as possible according to the runtime VLEN.

However, it is not easy to support vector in a SIMD framework. Because one of the main differences between SIMD and Vector is whether the length of the vector register is different in different hardware.

In order to avoid modifying the existing framework (maybe refactored in the future, but not now), using (manually) compile-time VLEN is a feasible solution.

For a specific hardware device, VLEN is a constant. We can run a script to check VLEN and define the macro like `__RVV_VLEN__` as "input" for the source code.

In this case, although a binary code cannot run on all RVV devices, the source code can (by build from source).

### API

We use `__RVV_VLEN__` for `nlanes`, it is inherently compatible with the constructor `v_float32(vfloat32m1_t v)`, because there are exactly `nlanes` elements in a register(v), but how about construct from number of `nlanes` scalars? Do we have to consider all possible `nlanes`?

Before answering this question, let us introduce some knowledge of RVV. In the RVV specification, the legal value of VLEN in an application Processors is `2^n` where `n` in [7, 16] (defined by extension "Zvl128b").

So, do we have to consider all possible `nlanes`? Does it means 9 different macros for each VLEN?

Maybe not. we use the parameter pack that a constructor can accept any number of parameters (also any type as ). We think it is acceptable, although it requires the caller to ensure that the type and number of parameters passed in are correct.

Here are some strategies for dealing with parameter types and numbers

1. When the parameter type is incorrect, we try to generate a compiler warning.
2. When the number of parameters is greater than `nlanes`, an assertion will be triggered.
3. When the number of parameters is less than `nlanes`, add 0 at the end.

## How to run

The following instructions assume that you have built the RVV toolchain. If not, please refer to this [page](https://github.com/riscv-collab/riscv-gnu-toolchain/tree/rvv-intrinsic).

This is an example about cross-compilation.

1. `cd VectorUI`

2. `mkdir build && cd build`

3. `cmake -DRISCV_GCC_INSTALL_ROOT=<Path to RVV toolchain root> -DVLEN=256 ..`

4. `make`

5. `qemu-riscv64 -cpu rv64,x-v=true,vlen=256 testAPI.out`

There is also a script named `builder.sh` designed to test and set VLEN on a real RVV device. Just use `./builder.sh` to replace steps 2 and 3.

But for now, we have not found any available RVV device (with 0.10 or newer), so in order to show how the script works, we use QEMU instead. This means both `./builder.sh` and instruction in step 3 are work for cross-compilation now.

## Examples

### testAPI.cpp

This file tests the compatibility and robustness of the API.

### gemm.cpp

This file implements GEMM using Universal intrinsic interface.

In this case, the loop increment is variable, so as VLEN increases, the number of loops will decrease.

Specifically,

| VLEN | Elements per loop(nlanes)    | Number of loop |
| ---- | ---------------------------- | -------------- |
| 128  | 4                            | 4              |
| 256  | 8                            | 2              |
| 512  | 16                           | 1              |
| 1024 | 32 (only 16 Elements totaly) | 1              |
