#pragma once
#include <algorithm>

const float PI = 3.1415927f;

//vector and matrix classes modified from AC source

#define YAW 0 //x
#define PITCH 1  //y
#define ROLL 2 //z


struct vec_2
{
    float x, y;
};

class vec_3
{
public:

    union
    {
        struct { float x, y, z; };
        float v[3];
    };

    vec_3() { x = y = z = 0; }
    vec_3(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    vec_3 operator + (const vec_3& rhs) const { return vec_3(x + rhs.x, y + rhs.y, z + rhs.z); }
    vec_3 operator - (const vec_3& rhs) const { return vec_3(x - rhs.x, y - rhs.y, z - rhs.z); }
    vec_3 operator * (const float& rhs) const { return vec_3(x * rhs, y * rhs, z * rhs); }
    vec_3 operator / (const float& rhs) const { return vec_3(x / rhs, y / rhs, z / rhs); }
    vec_3& operator += (const vec_3& rhs) { return *this = *this + rhs; }
    vec_3& operator -= (const vec_3& rhs) { return *this = *this - rhs; }
    vec_3& operator *= (const float& rhs) { return *this = *this * rhs; }
    vec_3& operator /= (const float& rhs) { return *this = *this / rhs; }
    float dot() const { return x * x + y * y + z * z; }
    float Length() const { return sqrtf(dot()); }
    vec_3 Normalize() const { return *this * (1 / Length()); }
    float Distance(const vec_3& rhs) const { return (*this - rhs).Length(); }
};

struct vec_4
{
	union
	{
		struct { float x, y, z, w; };
		float v[4];
	};
    vec_4() { x = y = z = w = 0; }
	explicit vec_4(const vec_3& p, float w = 0) : x(p.x), y(p.y), z(p.z), w(w) {}
	vec_4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	float& operator[](int i) { return v[i]; }
	float  operator[](int i) const { return v[i]; }
};

vec_3 Add(vec_3 src, vec_3 dst);

vec_3 Subtract(vec_3 src, vec_3 dst);

float Magnitude(vec_3 vec);

float Distance(vec_3 src, vec_3 dst);

vec_3 Normalize(vec_3 src);

vec_3 CalcAngle(vec_3 src, vec_3 dst);

bool WorldToScreen(vec_3 pos, vec_3& screen, float v[16], int windowWidth, int windowHeight);

bool WorldToScreenDX(vec_3 pos, vec_2& screen, float matrix[16], int windowWidth, int windowHeight);

float RadianToDegree(float radian);

float DegreeToRadian(float degree);

vec_3 RadianToDegree(vec_3 radians);

vec_3 DegreeToRadian(vec_3 degrees);

float DotProduct(vec_3 src, vec_3 dst);