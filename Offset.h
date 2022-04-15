#pragma once
#include "geom.h"

class EntityBase
{
public:
	char pad_0000[64]; //0x0000
	class EntityClass* EntityPtr; //0x0040
	float Pitch; //0x0048
	char pad_004C[4]; //0x004C
	float Yaw; //0x0050
	vec_3 Coordonnee; //0x0054
	class Verification1Class* Verification1ptr; //0x0060
	class Verification2Class* Verification2ptr; //0x0068
	class Verification3Class* Verification3ptr; //0x0070
	char pad_0078[488]; //0x0078
	class Bone11Class* Bone1ptr; //0x0260
	char pad_0268[3304]; //0x0268
}; //Size: 0x0F50
static_assert(sizeof(EntityBase) == 0xF50);

class EntityClass
{
public:
	char pad_0000[224]; //0x0000
	char* Name; //0x00E0
	char pad_00E8[1944]; //0x00E8
}; //Size: 0x0880
static_assert(sizeof(EntityClass) == 0x880);

class Verification1Class
{
public:
	char pad_0000[128]; //0x0000
}; //Size: 0x0080
static_assert(sizeof(Verification1Class) == 0x80);

class Verification2Class
{
public:
	char pad_0000[112]; //0x0000
}; //Size: 0x0070
static_assert(sizeof(Verification2Class) == 0x70);

class Verification3Class
{
public:
	char pad_0000[136]; //0x0000
}; //Size: 0x0088
static_assert(sizeof(Verification3Class) == 0x88);

class Bone11Class
{
public:
	char pad_0000[8]; //0x0000
	class Bone2Class* Bone2ptr; //0x0008
	char pad_0010[120]; //0x0010
}; //Size: 0x0088
static_assert(sizeof(Bone11Class) == 0x88);

class Bone2Class
{
public:
	char pad_0000[304]; //0x0000
	class BoneClass* Bone; //0x0130
	char pad_0138[1168]; //0x0138
}; //Size: 0x05C8
static_assert(sizeof(Bone2Class) == 0x5C8);

class BoneClass
{
public:
	char pad_0000[120]; //0x0000
	char* ValidationBone; //0x0078
	char pad_0080[1112]; //0x0080
	vec_3 footR; //0x04D8
	char pad_04E4[1652]; //0x04E4
	vec_3 GoatHead; //0x0B58
	char pad_0B64[1140]; //0x0B64
	vec_3 HeadHumanoid; //0x0FD8
	char pad_0FE4[8364]; //0x0FE4
}; //Size: 0x3090
static_assert(sizeof(BoneClass) == 0x3090);