#include "intrin/intrin_rvv_fixed.hpp"
#include <iostream>
#include <algorithm>
template <typename vType>
void dump(vType v) {
    for (int i = 0; i < v.nlanes; ++i)
        std::cout << v.val[i] << (i == v.nlanes - 1 ? "\n": " ");
}

int main() {
    v_float32 v0, v1, v2;
    v0 = v_setall_f32(0);

    // Not expect warnings[-Wnarrowing] double to float
    // because compiler does NOT know if args is double or float when calling
    // so the template expands to v_float32(double... nScalars)
    v1 = v_float32(1.1, 2.2, 3.3, 4);

    // no warnings, float is asked when calling v_float32(initializer_list<float> nScalars)
    v2 = v_float32({1.1, 2.2, 3.3, 4});
    v0 = v_fma(v1,v2,v0);
    dump(v0);

    // Type matching
    v_int32 v3 = v_int32(1, 2, 3, 4, 5 ,6 ,7);

    // Expect type cast warnings: [-Wnarrowing], double to int
    v_int32 v4 = v_int32(1.1, 2.2, 3.3, 4.4);

    ////////////////////////////////////////////////////////////
    // error, args in initializer_list must be same type (int).
    /* v_int32 v4 = v_int32({1, 2, 3, 4.4}); */
    /* v_int32 v4 = v_int32({1.1, 2.2, 3.3, 4.4}); */
    ///////////////////////////////////////////////////////////
    dump(v3);
    dump(v4);
    return 0;
}