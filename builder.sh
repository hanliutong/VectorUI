RISCV_GCC_INSTALL_ROOT=/opt/RISCV

# mkdir build
ROOT_DIR=`pwd`
if [ -d "$ROOT_DIR/build" ]; then
    rm -rf $ROOT_DIR/build
fi
mkdir build
cd build

# check VLEN
echo "#include <riscv_vector.h>
#include <stdio.h>
int main() {
    printf(\"%d\n\", vsetvlmax_e32m1() * 32);
    return 0;
}" > getVLEN.cpp
${RISCV_GCC_INSTALL_ROOT}/bin/riscv64-unknown-linux-gnu-g++ getVLEN.cpp -o getVLEN.out
#####################
# For a RVV device, it will return the VLEN in the specific device.
# vlen=`(./getVLEN.out)`
#####################

#####################
# For cross compilation, check VLEN is not necessary. (VLEN should be specified by `$ cmake -DVLEN=...`)
# The following command is just an example for QEMU and should not be used. (Always returns 256)
vlen=`(${RISCV_GCC_INSTALL_ROOT}/bin/qemu-riscv64 -cpu rv64,x-v=true,vlen=256 ./getVLEN.out)`
#####################
rm getVLEN.*
echo $vlen

# call cmake
cmake -DRISCV_GCC_INSTALL_ROOT=$RISCV_GCC_INSTALL_ROOT -DVLEN=$vlen ..

# and then...
# cd build && make
# ${RISCV_GCC_INSTALL_ROOT}/qemu-riscv64 -cpu rv64,x-v=true,vlen=256 main.out
# or just `./main.out` on a RVV device
