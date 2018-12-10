#pragma once

#include <math.h>

#define Pi32 3.14159265359f
#define TwoPi32 Pi32 * 2.0f
#define I16_MAX 32767

inline i32
RoundF32ToI32(f32 value)
{
    i32 result = (i32)roundf(value);
    return result;
}

inline u32
RoundF32ToU32(f32 value)
{
    u32 result = (u32)roundf(value);
    return result;
}

inline i32
FloorF32ToI32(f32 value)
{
    i32 result = (i32)floorf(value);
    return result;
}

inline i32
CeilF32ToI32(f32 value)
{
    i32 result = (i32)ceilf(value);
    return result;
}

inline i32
TruncateF32ToI32(f32 value)
{
    i32 result = (i32)value;
    return result;
}

inline u32
TruncateU64(u64 value)
{
    u32 result = (u32)value;
    return result;
}

inline f64
LimitF(f64 value, f64 minValue, f64 maxValue)
{
    if (value > maxValue)
    {
        return maxValue;
    }
    else if (value < minValue)
    {
        return minValue;
    }

    return value;
}

inline i64
LimitI(i64 value, i64 minValue, i64 maxValue)
{
    if (value > maxValue)
    {
        return maxValue;
    }
    else if (value < minValue)
    {
        return minValue;
    }

    return value;
}

inline void
F64toF32(f64 input, f32 *output)
{
    *output = (f32)LimitF(input, -1.0, 1.0);
}

inline void
F32toF64(f64 input, f32 *output)
{
    *output = (f32)LimitF(input, -1.0f, 1.0f);
}

inline void
F64toU8(f64 input, u8 *output)
{
    i16 adjustedInput = (i16)(input * 128.0);
    adjustedInput += 128;
    adjustedInput = LimitI(adjustedInput, 0, 255);
    *output = (u8)(adjustedInput);
}

inline void
U8toF64(u8 input, f64 *output)
{
    f64 adjustedInput = input - 0x80;
    *output = adjustedInput / (0x80 - 1.0);
    *output = LimitF(*output, -1.0, 1.0);
}

inline void
F64toI16(f64 input, i16 *output)
{
    i32 adjustedInput = (i32)(input * 0x8000);
    *output = (i16)LimitI(adjustedInput, -0x8000, 0x8000 - 1);
}

inline void
I16toF64(i16 input, f64 *output)
{
    f64 adjustedInput = (f64)(input) / (0x8000 - 1.0);
    *output = LimitF(adjustedInput, -1.0, 1.0);
}

inline void F64to24Bit(f64 input, u8 *pOutput)
{
    i32 adjustedInput = (i32)(input * 0x7FFFFF);
    adjustedInput = LimitI(adjustedInput, -0x7FFFFF, 0x7FFFFF - 1);

    u8 *pBytes = (u8 *)(&adjustedInput);
    for (auto i = 0; i != 3; ++i)
    {
        *pOutput++ = *pBytes++;
    }
}

inline void F64from24Bit(u8 *pInput, f64 *output)
{
    i32 adjustedInput = 0;
    if (pInput[2] & 0x80)
    {
        adjustedInput = (0xff << 24) | (pInput[2] << 16) | (pInput[1] << 8) | (pInput[0] << 0);
    }
    else
    {
        adjustedInput = (pInput[2] << 16) | (pInput[1] << 8) | (pInput[0] << 0);
    }
    *output = (f64)(adjustedInput) / (0x7FFFFF - 1.0);
    *output = LimitF(*output, -1.0, 1.0);
}

union vector2 {
    struct
    {
        f32 X;
        f32 Y;
    };
    f32 E[2];
};

inline vector2
Vector2(f32 X, f32 Y)
{
    vector2 result;

    result.X = X;
    result.Y = Y;

    return result;
}

inline vector2
operator*(f32 A, vector2 B)
{
    vector2 result;

    result.X = A * B.X;
    result.Y = A * B.Y;

    return result;
}

inline vector2
operator*(vector2 B, f32 A)
{
    vector2 result = A * B;

    return result;
}

inline vector2 &
operator*=(vector2 &B, f32 A)
{
    B = A * B;

    return B;
}

inline vector2
operator-(vector2 A)
{
    vector2 result;

    result.X = -A.X;
    result.Y = -A.Y;

    return result;
}

inline vector2
operator+(vector2 A, vector2 B)
{
    vector2 result;

    result.X = A.X + B.X;
    result.Y = A.Y + B.Y;

    return result;
}

inline vector2 &
operator+=(vector2 &A, vector2 B)
{
    A = A + B;

    return A;
}

inline vector2
operator-(vector2 A, vector2 B)
{
    vector2 result;

    result.X = A.X - B.X;
    result.Y = A.Y - B.Y;

    return result;
}
