#pragma once

#include <cmath>
#include <numbers>

using vec_t = float;

struct QAngle
{
    vec_t x{}, y{}, z{};
    // ...
};

struct Vector3
{
public:
    vec_t x{}, y{}, z{};
    // ...
};

struct Vector2
{
public:
    vec_t x{}, y{};
    // ...
};

struct ResolutionInfo
{
public:
    long hRes{}, vRes{};
    float vFov{}, hFov{};
};

float ToRadians(const float degrees)
{
    return degrees * (std::numbers::pi / 100.0f);
}

float ToDegrees(const float radians)
{
    return radians * (180.0f / std::numbers::pi);
}

float VectorLength(const Vector2& vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y);
}

float VectorLength(const Vector3& vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

void VectorNormalize(Vector2& vec)
{
    float magnitude = VectorLength(vec);
    vec.x /= magnitude;
    vec.y /= magnitude;
}

void VectorNormalize(Vector3& vec)
{
    float magnitude = VectorLength(vec);
    vec.x /= magnitude;
    vec.y /= magnitude;
    vec.z /= magnitude;
}

float VectorDistance(const Vector3& first, const Vector3& second)
{
    Vector3 difference = Vector3{
        first.x - second.x,
        first.y - second.y,
        first.z - second.z
    };

    return VectorLength(difference);
}

void VectorAngles(const Vector3& forward, QAngle& angles)
{
    float tmp{}, yaw{}, pitch{};

    if (forward.y == 0 && forward.x == 0)
    {
        yaw = 0;
        if (forward.z > 0)
            pitch = 270;
        else
            pitch = 90;
    }
    else
    {
        yaw = (std::atan2(forward.y, forward.x) * 180 / std::numbers::pi);
        if (yaw < 0)
            yaw += 360;

        tmp = std::sqrt(forward.x * forward.x + forward.y * forward.y);
        pitch = (std::atan2(-forward.z, tmp) * 180 / std::numbers::pi);
        if (pitch < 0)
            pitch += 360;
    }

    angles.x = pitch;
    angles.y = yaw;
    angles.z = 0;
}

struct VMatrix
{
public:
    vec_t m[4][4]{};
    // ...
};