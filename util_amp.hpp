#pragma once
#include <amp_math.h>
using namespace concurrency;

struct ampVector2d {
	double x, y;
};

struct ampVector2f {
	float x, y;
};

struct ampColor {
	int r, g, b;
	ampColor() {};
	ampColor(float r, float g, float b) restrict(amp) : r(r), g(g), b(b) {}
};

struct ampVertex {
	ampVector2d position;
	ampColor color;

	ampVertex() {};
};


ampColor amp_hsv_to_rgb(float h, float s, float v) restrict(amp) {
	float rH = h / 60;
	float C = s * v; //(1 - fabsf(2 * hsl.z - 1))*hsl.y;
	float X = C*(1 - fast_math::fabsf((fast_math::fmodf(rH, 2) - 1)));
	float m = v - C; //hsl.z - C / 2;
	C += m; X += m;

	switch (int(rH)) {
	case 0:
		return ampColor(255 * C, 255 * X, 255 * m);
	case 1:
		return ampColor(255 * X, 255 * C, 255 * m);
	case 2:
		return ampColor(255 * m, 255 * C, 255 * X);
	case 3:
		return ampColor(255 * m, 255 * X, 255 * C);
	case 4:
		return ampColor(255 * X, 255 * m, 255 * C);
	case 5:
		return ampColor(255 * C, 255 * m, 255 * X);
	default:
		return ampColor(255, 255, 255);
		break;
	}
}