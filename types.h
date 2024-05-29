#pragma once

#include <stdint.h>
#include <windows.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef float f32;
typedef double f64;

typedef i32 bool;

typedef struct MyBitmap
{
    u32 width;
    u32 height;
    u32 bytesPerPixel;
    u32 *pixels;
} MyBitmap;
