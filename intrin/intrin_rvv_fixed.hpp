// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

// The original implementation has been contributed by Yin Zhang.
// Copyright (C) 2020, Institute of Software, Chinese Academy of Sciences.

////////////////////////////////////////////////////////
// This file is a subset of intrin_rvv.hpp in OpenCV repo, with some modifications.
// There are only 2 class(v_float32 and v_int32)
// and 4 UI functions (set, load, store and fma) in the current implementation.
////////////////////////////////////////////////////////

#include <stdio.h>
// #include <stdarg.h>
#include <initializer_list>
#include <assert.h>
#include <riscv_vector.h>

typedef unsigned char uchar;

struct v_float32
{
    typedef float lane_type;
    enum { nlanes = __RVV_VLEN__/32 };

    v_float32() {}
    explicit v_float32(vfloat32m1_t v)
    {
        vse32_v_f32m1(val, v, nlanes);
    }
    /* existing interfaces
    // v_float32(float v0, float v1, float v2, float v3)
    // {
    //     float v[] = {v0, v1, v2, v3};
    //     for (int i = 0; i < nlanes; ++i)
    //     {
    //         val[i] = v[i];
    //     }
    // }
    */

    // Advantage: Compatible with existing interfaces, aka, v_float32(float v0, float v1, float v2, float v3)
    // Disadvantage: Unsafe types cast
    template<typename... Targs>
    explicit v_float32(Targs... nScalars) // n * scalars
    {
        v_float32_({nScalars...});

    }

    // Advantage: Type safe, check when calling (only float is acceptable in this func)
    // Disadvantage: Incompatible with existing interfaces
    explicit v_float32(std::initializer_list<float> nScalars)
    {
        v_float32_(nScalars);
    }

    operator vfloat32m1_t() const
    {
        return vle32_v_f32m1(val, nlanes);
    }
    float get0() const
    {
        return val[0];
    }
    float val[nlanes];
private:
    void inline v_float32_(std::initializer_list<float> nScalars)
    {
        assert( nScalars.size() <= nlanes);
        auto it = nScalars.begin();
        unsigned int i = 0;
        for (; i < nScalars.size(); ++i) {
            val[i] = *(it + i);
        }
        for (; i < nlanes; ++i) {
            val[i] = 0;
        }
    }
};

struct v_int32
{
    typedef int lane_type;
    enum { nlanes = __RVV_VLEN__/32 };

    v_int32() {}
    explicit v_int32(vint32m1_t v)
    {
        vse32_v_i32m1(val, v, nlanes);
    }
    template<typename... Targs>
    explicit v_int32(Targs... nScalars) // n * scalars
    {
        v_int32_({nScalars...});
    }
    explicit v_int32(std::initializer_list<int> nScalars)
    {
        v_int32_(nScalars);
    }
    operator vint32m1_t() const
    {
        return vle32_v_i32m1(val, nlanes);
    }
    int get0() const
    {
        return val[0];
    }
    int val[nlanes];
private:
    void v_int32_(std::initializer_list<int> nScalars)
    {
        assert( nScalars.size() <= nlanes);
        auto it = nScalars.begin();
        unsigned int i = 0;
        for (; i < nScalars.size(); ++i) {
            val[i] = *(it + i);
        }
        for (; i < nlanes; ++i) {
            val[i] = 0;
        }
    }
};


//////////// Initial ////////////

#define OPENCV_HAL_IMPL_RVV_INIT_FP(_Tpv, _Tp, suffix, nlanes) \
inline v_##_Tpv v_setzero_##suffix() \
{ \
    return v_##_Tpv(vfmv_v_f_##suffix##m1(0, nlanes)); \
} \
inline v_##_Tpv v_setall_##suffix(_Tp v, size_t vl = nlanes) \
{ \
    return v_##_Tpv(vfmv_v_f_##suffix##m1(v, vl)); \
}

OPENCV_HAL_IMPL_RVV_INIT_FP(float32, float, f32, v_float32::nlanes)


////////////// Load/Store //////////////

#define OPENCV_HAL_IMPL_RVV_LOADSTORE_OP(_Tpvec, _nTpvec, _Tp, hvl, nlanes, width, suffix, vmv) \
inline _Tpvec v_load(const _Tp* ptr, size_t vl = nlanes) \
{ \
    return _Tpvec((_nTpvec)vle8_v_u8m1((uchar*)ptr, vl*4)); \
} \
inline void v_store(_Tp* ptr, const _Tpvec& a, size_t vl = nlanes) \
{ \
    vse8_v_u8m1((uchar*)ptr, vle8_v_u8m1((uchar*)a.val, vl*4), vl*4); \
} \

OPENCV_HAL_IMPL_RVV_LOADSTORE_OP(v_float32, vfloat32m1_t, float, 2, v_float32::nlanes, 32, f32, vfmv_v_f_f32m1)
OPENCV_HAL_IMPL_RVV_LOADSTORE_OP(v_int32, vint32m1_t, int, 2, v_int32::nlanes, 32, f32, vmv_v_x_i32m1)

////////////// Multiply-Add //////////////

inline v_float32 v_fma(const v_float32& a, const v_float32& b, const v_float32& c, size_t vl = v_float32::nlanes)
{
    return v_float32(vfmacc_vv_f32m1(c, a, b, vl));
}

inline v_float32 v_fma(const float a, const v_float32& b, const v_float32& c, size_t vl = v_float32::nlanes)
{
    return v_float32(vfmacc_vf_f32m1(c, a, b, vl));
}
