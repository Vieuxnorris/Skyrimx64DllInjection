#include "geom.h"

vec_3 Add(vec_3 src, vec_3 dst)
{
	vec_3 sum;
	sum.x = src.x + dst.x;
	sum.y = src.y + dst.y;
	sum.z = src.z + dst.z;
	return sum;
}

vec_3 Subtract(vec_3 src, vec_3 dst)
{
	vec_3 diff;
	diff.x = src.x - dst.x;
	diff.y = src.y - dst.y;
	diff.z = src.z - dst.z;
	return diff;
}

vec_3 Divide(vec_3 src, float num)
{
	vec_3 vec;
	vec.x = src.x / num;
	vec.y = src.y / num;
	vec.z = src.z / num;

	return vec;
}

float DotProduct(vec_3 src, vec_3 dst)
{
	return src.x * dst.x + src.y * dst.y + src.z * dst.z;
}

float Magnitude(vec_3 vec)
{
	return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

float Distance(vec_3 src, vec_3 dst)
{
	vec_3 diff = Subtract(src, dst);
	return Magnitude(diff);
}

vec_3 Normalize(vec_3 src)
{
	vec_3 vec = Divide(src, Magnitude(src));
	return vec;
}

vec_3 CalcAngle(vec_3 src, vec_3 dst)
{
	/* Skyrim Special Edition
		Pitch -> -90 to 90
		Yaw -> 0 to 360
	*/
	vec_3 angles;
	vec_3 deltaVec = { src.x - dst.x , src.y - dst.y , src.z - dst.z };
	float Distance = sqrtf(pow(deltaVec.x, 2) + pow(deltaVec.y, 2) + pow(deltaVec.z, 2));
	angles.x = atan2f(deltaVec.x, deltaVec.y) / PI * 180.f + 180.0f;
	angles.y = (asinf(deltaVec.z / Distance) * (180.f / PI));
	angles.z = 0.0f;

	return angles;
}

//openGL
bool WorldToScreen(vec_3 pos, vec_3& screen, float matrix[16], int windowWidth, int windowHeight)
{
	//Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
	vec_4 clipCoords;
	clipCoords.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
	clipCoords.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
	clipCoords.z = pos.x * matrix[2] + pos.y * matrix[6] + pos.z * matrix[10] + matrix[14];
	clipCoords.w = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

	if (clipCoords.w < 0.1f)
		return false;

	//perspective division, dividing by clip.W = Normalized Device Coordinates
	vec_3 NDC;
	NDC.x = clipCoords.x / clipCoords.w;
	NDC.y = clipCoords.y / clipCoords.w;
	NDC.z = clipCoords.z / clipCoords.w;

	screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
	screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
	return true;
}

bool WorldToScreenDX(vec_3 pos, vec_2& screen, float matrix[16], int windowWidth, int windowHeight)
{
	//Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
	vec_4 clipCoords;
	clipCoords.x = pos.x * matrix[0] + pos.y * matrix[1] + pos.z * matrix[2] + matrix[3];
	clipCoords.y = pos.x * matrix[4] + pos.y * matrix[5] + pos.z * matrix[6] + matrix[7];
	clipCoords.z = pos.x * matrix[8] + pos.y * matrix[9] + pos.z * matrix[10] + matrix[11];
	clipCoords.w = pos.x * matrix[12] + pos.y * matrix[13] + pos.z * matrix[14] + matrix[15];

	if (clipCoords.w < 0.1f)
		return false;

	//perspective division, dividing by clip.W = Normalized Device Coordinates
	vec_3 NDC;
	NDC.x = clipCoords.x / clipCoords.w;
	NDC.y = clipCoords.y / clipCoords.w;
	NDC.z = clipCoords.z / clipCoords.w;

	screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
	screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
	return true;
}

bool WorldToScreen2(vec_3 src, vec_3 dst, vec_3& screen, float fovx, float fovy, float windowWidth, float windowHeight, vec_3 left, vec_3 up, vec_3 forward)
{
	vec_3 transform;
	float xc, yc;
	float px, py;
	float z;

	px = tan(fovx * PI / 360.0f);
	py = tan(fovy * PI / 360.0f);

	transform = Subtract(dst, src);

	xc = windowWidth / 2.0f;
	yc = windowHeight / 2.0f;

	z = DotProduct(transform, left);

	if (z <= 0.1)
	{
		return false;
	}

	screen.x = xc - DotProduct(transform, up) * xc / (z * px);
	screen.y = yc - DotProduct(transform, forward) * yc / (z * py);

	return true;
}

float RadianToDegree(float radian)
{
	return radian * (180 / PI);
}

float DegreeToRadian(float degree)
{
	return degree * (PI / 180);

}

vec_3 RadianToDegree(vec_3 radians)
{
	vec_3 degrees;
	degrees.x = radians.x * (180 / PI);
	degrees.y = radians.y * (180 / PI);
	degrees.z = radians.z * (180 / PI);
	return degrees;
}

vec_3 DegreeToRadian(vec_3 degrees)
{
	vec_3 radians;
	radians.x = degrees.x * (PI / 180);
	radians.y = degrees.y * (PI / 180);
	radians.z = degrees.z * (PI / 180);
	return radians;
}