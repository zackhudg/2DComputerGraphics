#include "MyShader.h"
#include "GColor.h"
#include "GMatrix.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <memory>

MyShader::MyShader() {}
MyShader::MyShader(const GBitmap& shader, const GMatrix& matrix, GShader::TileMode tilemode) : fShader(shader), localMatrix(matrix), tilemode(tilemode) {}

MyGradientShader::MyGradientShader(GPoint pre_p0, GPoint pre_p1, const GColor c[], int count, GShader::TileMode tilemode) : tilemode(tilemode) {
	for (int i = 0; i < count; i++) {
		colors.push_back(c[i]);
	}

	dx = pre_p1.x() - pre_p0.x();
	dy = pre_p1.y() - pre_p0.y();
	length = sqrt(dx * dx + dy * dy);
	float sin = dy / length;
	float cos = dx / length;
	GMatrix m = GMatrix::Translate(-1 * pre_p0.x(), -1 * pre_p0.y());

	if (pre_p1.y() < pre_p0.y()) {
		m = GMatrix(cos, -1 * sin, 0, sin, cos, 0) * m;
	}
	else if (pre_p1.y() > pre_p0.y()) {
		m = GMatrix(cos, sin, 0, -1 * sin, cos, 0) * m;
	}
	m = GMatrix::Scale(1.0f / length, 1) * m;
	p1 = m * pre_p1;
	p0 = m * pre_p0;

	if (count != 1) segmentLength = 1.0f / (count - 1);
	else segmentLength = 1.0f;
	localMatrix = m;
}


bool MyShader::isOpaque() {
	for (int y = 0; y < fShader.height(); y++) {
		for (int x = 0; x < fShader.width(); x++) {
			if (GPixel_GetA(*fShader.getAddr(x, y)) != 255) return false;
		}
	}
	return true;
}

bool MyGradientShader::isOpaque() {
	for (int i = 0; i < colors.size(); i++) {
		if (colors.at(i).a != 1) return false;
	}
	return true;
}


bool MyShader::setContext(const GMatrix& ctm) {
	float det = (ctm[0] * ctm[4]) - (ctm[1] * ctm[3]);
	if (det == 0.0) return false;

	context = ctm * localMatrix;
	return true;
}

bool MyGradientShader::setContext(const GMatrix& ctm) {
	float det = (ctm[0] * ctm[4]) - (ctm[1] * ctm[3]);
	if (det == 0.0) return false;

	context = ctm;
	return true;
}


void MyShader::shadeRow(int x, int y, int count, GPixel row[]) {
	GMatrix iCTM;
	if (!context.invert(&iCTM)) return; // idk check this?
	GPoint shaderPoints[count];
	for (int i = 0; i < count; i++) {
		shaderPoints[i] = { x + i + .5, y + .5 };
	}
	//can optimize I think. putting the inverse before all the blits and whatnot. Just do once, then invert it (again).
	iCTM.mapPoints(shaderPoints, shaderPoints, count);
	for (int i = 0; i < count; i++) {
		int ix, iy;
		switch (tilemode) {
		case(kRepeat):
			ix = GFloorToInt(fShader.width() * (shaderPoints[i].x() / fShader.width() - floor(shaderPoints[i].x() / fShader.width())));
			iy = GFloorToInt(fShader.height() * (shaderPoints[i].y() / fShader.height() - floor(shaderPoints[i].y() / fShader.height())));
			break;

		case(kClamp):
			ix = std::max(std::min(fShader.width() - 1, GFloorToInt(shaderPoints[i].x())), 0);
			iy = std::max(std::min(fShader.height() - 1, GFloorToInt(shaderPoints[i].y())), 0);
			break;

		case(kMirror):
			int x = GFloorToInt(shaderPoints[i].x());
			int y = GFloorToInt(shaderPoints[i].y());
			if (x < 0) x = std::abs(x + 1);
			if (y < 0) y = std::abs(y + 1);

			int mirror_x = std::abs(x / fShader.width()) % 2;
			int mirror_y = std::abs(y / fShader.height()) % 2;

			x = x % fShader.width();
			if (mirror_x) x = fShader.width() - x - 1;
			y = y % fShader.height();
			if (mirror_y) y = fShader.height() - y - 1;
			ix = x;
			iy = y;
			break;
		}
		row[i] = *fShader.getAddr(ix, iy);
	}
	return;
}

void MyGradientShader::shadeRow(int x, int y, int count, GPixel row[]) {
	GMatrix iCTM;
	if (!context.invert(&iCTM)) return; // idk check this?

	GPoint shaderPoints[count];
	for (int i = 0; i < count; i++) {
		shaderPoints[i] = { x + i + .5, y + .5 };
	}

	GMatrix m = GMatrix::Concat(localMatrix, iCTM);
	//can optimize I think. putting the inverse before all the blits and whatnot. Just do once, then invert it (again).
	m.mapPoints(shaderPoints, shaderPoints, count);
	for (int i = 0; i < count; i++) {
		float distance = shaderPoints[i].x();
		switch (tilemode) {
		case(kRepeat):
			distance = distance - floor(distance);
			break;

		case(kClamp):
			distance = std::max(0.0f, std::min(distance, 1.0f));
			break;

		case(kMirror):
			distance = distance * std::pow(-1.0, floor(distance));
			distance = distance - floor(distance);
			break;
		}

		int segment = GFloorToInt(distance / segmentLength);
		float weight = std::max(0.0f, std::min(fmod(distance, segmentLength) / segmentLength, 1.0f));
		float r, g, b, a;
		if (segment == static_cast<int>(colors.size()) - 1) {
			r = colors.at(segment).r;
			g = colors.at(segment).g;
			b = colors.at(segment).b;
			a = colors.at(segment).a;
		}
		else {
			r = colors.at(segment).r * (1.0f - weight) + colors.at(segment + 1).r * weight;
			g = colors.at(segment).g * (1.0f - weight) + colors.at(segment + 1).g * weight;
			b = colors.at(segment).b * (1.0f - weight) + colors.at(segment + 1).b * weight;
			a = colors.at(segment).a * (1.0f - weight) + colors.at(segment + 1).a * weight;
		}

		//cant use premul func for some reason!
		int alp = GRoundToInt(255 * a);
		int red = GRoundToInt(255 * a * r);
		int gre = GRoundToInt(255 * a * g);
		int blu = GRoundToInt(255 * a * b);

		row[i] = GPixel_PackARGB(alp, red, gre, blu);
	}
	return;
}


std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& shader, const GMatrix& localMatrix, GShader::TileMode tilemode) {
	if (&shader == nullptr) return NULL;
	float det = (localMatrix[0] * localMatrix[4]) - (localMatrix[1] * localMatrix[3]);
	if (det == 0.0) return NULL;

	MyShader* myShader = new MyShader(shader, localMatrix, tilemode);
	return std::unique_ptr<GShader>(myShader);
}


std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode tilemode) {
	MyGradientShader* myGradientShader = new MyGradientShader(p0, p1, colors, count, tilemode);
	return std::unique_ptr<GShader>(myGradientShader);
}
