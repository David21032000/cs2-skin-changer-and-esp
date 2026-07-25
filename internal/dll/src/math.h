#pragma once
#include <cmath>
#include <cstdint>
#include <array>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifndef RAD2DEG
#define RAD2DEG(x) ((x) * (180.0f / M_PI))
#endif

#ifndef DEG2RAD
#define DEG2RAD(x) ((x) * (M_PI / 180.0f))
#endif

struct Vec3 {
    float x, y, z;

    Vec3() : x(0.f), y(0.f), z(0.f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    Vec3 operator-(const Vec3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    Vec3 operator*(float s) const { return { x * s, y * s, z * s }; }
    Vec3 operator/(float s) const { return { x / s, y / s, z / s }; }
    Vec3 operator-() const { return { -x, -y, -z }; }

    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vec3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

    bool operator==(const Vec3& v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const Vec3& v) const { return !(*this == v); }

    float Dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    float LengthSqr() const { return x * x + y * y + z * z; }
    float Length2D() const { return std::sqrt(x * x + y * y); }
    float DistTo(const Vec3& v) const { return (*this - v).Length(); }
    float DistToSqr(const Vec3& v) const { return (*this - v).LengthSqr(); }

    Vec3 Normalized() const {
        float len = Length();
        if (len > 0.f) return *this / len;
        return *this;
    }

    void Normalize() {
        float len = Length();
        if (len > 0.f) *this /= len;
    }

    Vec3 Cross(const Vec3& v) const {
        return {
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        };
    }

    void Clamp() {
        if (x > 89.0f) x = 89.0f;
        if (x < -89.0f) x = -89.0f;
        if (y > 180.0f) y = 180.0f;
        if (y < -180.0f) y = -180.0f;
        z = 0.f;
    }
};

struct Matrix3x4 {
    float data[3][4];

    float* operator[](int i) { return data[i]; }
    const float* operator[](int i) const { return data[i]; }

    Vec3 GetOrigin() const {
        return { data[0][3], data[1][3], data[2][3] };
    }

    Vec3 GetBonePos(int bone) const {
        return { data[0][bone], data[1][bone], data[2][bone] };
    }

    static Matrix3x4 Identity() {
        Matrix3x4 m{};
        m.data[0][0] = 1.f; m.data[1][1] = 1.f; m.data[2][2] = 1.f;
        return m;
    }
};

struct ViewMatrix {
    float data[4][4];

    float* operator[](int i) { return data[i]; }
    const float* operator[](int i) const { return data[i]; }
};

inline void AngleVectors(const Vec3& angles, Vec3* forward, Vec3* right = nullptr, Vec3* up = nullptr) {
    float sp = std::sin(DEG2RAD(angles.x));
    float cp = std::cos(DEG2RAD(angles.x));
    float sy = std::sin(DEG2RAD(angles.y));
    float cy = std::cos(DEG2RAD(angles.y));

    if (forward) {
        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    }

    if (right || up) {
        float sr = std::sin(DEG2RAD(angles.z));
        float cr = std::cos(DEG2RAD(angles.z));

        if (right) {
            right->x = -1 * sr * sp * cy + -1 * cr * -sy;
            right->y = -1 * sr * sp * sy + -1 * cr * cy;
            right->z = -1 * sr * cp;
        }

        if (up) {
            up->x = cr * sp * cy + -sr * -sy;
            up->y = cr * sp * sy + -sr * cy;
            up->z = cr * cp;
        }
    }
}

inline Vec3 VectorAngles(const Vec3& forward) {
    Vec3 angles;
    float tmp, yaw, pitch;

    if (forward.y == 0.f && forward.x == 0.f) {
        yaw = 0.f;
        pitch = forward.z > 0.f ? 270.f : 90.f;
    } else {
        yaw = RAD2DEG(std::atan2(forward.y, forward.x));
        tmp = std::sqrt(forward.x * forward.x + forward.y * forward.y);
        pitch = RAD2DEG(std::atan2(-forward.z, tmp));
    }

    angles.x = pitch;
    angles.y = yaw;
    angles.z = 0.f;
    return angles;
}

inline Vec3 CalcAngle(const Vec3& src, const Vec3& dst) {
    Vec3 delta = src - dst;
    return VectorAngles(delta);
}

inline bool WorldToScreen(const Vec3& world, Vec3& screen, const ViewMatrix& vm, int screenW, int screenH) {
    float w = vm[3][0] * world.x + vm[3][1] * world.y + vm[3][2] * world.z + vm[3][3];

    if (w < 0.001f) return false;

    float invW = 1.f / w;

    screen.x = (vm[0][0] * world.x + vm[0][1] * world.y + vm[0][2] * world.z + vm[0][3]) * invW;
    screen.y = (vm[1][0] * world.x + vm[1][1] * world.y + vm[1][2] * world.z + vm[1][3]) * invW;

    screen.x = (screenW * 0.5f) + (screen.x * screenW) * 0.5f;
    screen.y = (screenH * 0.5f) - (screen.y * screenH) * 0.5f;

    return true;
}
