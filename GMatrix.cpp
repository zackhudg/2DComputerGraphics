#include "GMatrix.h"
#include <iostream>

GMatrix::GMatrix() : GMatrix(1, 0, 0, 0, 1, 0) {}

GMatrix GMatrix::Translate(float tx, float ty) {
	return GMatrix(1, 0, tx, 0, 1, ty);
}

GMatrix GMatrix::Scale(float sx, float sy) {
	return GMatrix(sx, 0, 0, 0, sy, 0);
}

GMatrix GMatrix::Rotate(float radians) {
	return GMatrix(cos(radians), (-1) * sin(radians), 0, sin(radians), cos(radians), 0);
}
GMatrix GMatrix::Concat(const GMatrix& secundo, const GMatrix& primo) {
	float a, b, c, d, e, f;
	a = (secundo[0] * primo[0]) + (secundo[1] * primo[3]);
	b = (secundo[0] * primo[1]) + (secundo[1] * primo[4]);
	c = (secundo[0] * primo[2]) + (secundo[1] * primo[5]) + secundo[2];
	d = (secundo[3] * primo[0]) + (secundo[4] * primo[3]);
	e = (secundo[3] * primo[1]) + (secundo[4] * primo[4]);
	f = (secundo[3] * primo[2]) + (secundo[4] * primo[5]) + secundo[5];
	return GMatrix(a, b, c, d, e, f);
}

bool GMatrix::invert(GMatrix* inverse) const {
	const GMatrix i = *this;
	float det = (i[0] * i[4]) - (i[1] * i[3]);
	if (det == 0) return false;
	else {
		float a, b, c, d, e, f;
		a = i[4] * (1 / det);
		b = i[1] * (1 / det) * (-1);
		c = ((i[1] * i[5]) - (i[4] * i[2])) * (1 / det);
		d = i[3] * (1 / det) * (-1);
		e = i[0] * (1 / det);
		f = ((i[0] * i[5]) - (i[3] * i[2])) * (1 / det) * (-1);
		*inverse = GMatrix(a, b, c, d, e, f);
		return true;
	}
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
	GMatrix matrix = *this; 
	for (int i = 0; i < count; i++) {
		GPoint point = src[i];
		float x = (matrix[0] * point.x()) + (matrix[1] * point.y()) + matrix[2];
		float y = (matrix[3] * point.x()) + (matrix[4] * point.y()) + matrix[5];;
		dst[i] = {x, y};
	}
}