#pragma once

#include "common.h"

enum LightType {
	POINT_LIGHT=0,
	DIRECTIONAL_LIGHT=1,
	NUM_LIGHT_TYPES
};

struct PointLight {
    Vector3f location,radiance;
    double location2[3];
    double radiance2[3];
};

struct DirectionalLight {
    float direction[3];
    float radiance[3];
};

struct Light {
    LightType type;
    //union {
        PointLight pointLight;
        DirectionalLight directionalLight;
    //};
};