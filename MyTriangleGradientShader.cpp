#include "MyTriangleShader.h"
#include "GColor.h"
#include "GMatrix.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <memory>

MyTriangleGradientShader::MyTriangleGradientShader(GPoint p[], const GColor c[]) {
	GPoint dp1 = p[1] - p[0];
	GPoint dp2 = p[2] - p[0];

	GMatrix m = GMatrix(dp1.x(), dp2.x(), p[0].x(),
		dp1.y(), dp2.y(), p[0].y());

	m.invert(&m);
	localMatrix = m;

	for (int i = 0; i < 3; i++) {
		colors[i] = c[i];
	}

}
 
MyTriangleBitmapShader::MyTriangleBitmapShader(GShader* shader, GPoint p[], GPoint t[]) : fShader(shader) {
	GPoint dp1 = p[1] - p[0];
	GPoint dp2 = p[2] - p[0];

	GPoint dt1 = t[1] - t[0];
	GPoint dt2 = t[2] - t[0];

	GMatrix m1 = GMatrix(dp1.x(), dp2.x(), p[0].x(),
						dp1.y(), dp2.y(), p[0].y());

	GMatrix m2 = GMatrix(dt1.x(), dt2.x(), t[0].x(),
						dt1.y(), dt2.y(), t[0].y());

	m2.invert(&m2);
	localMatrix = m1 * m2;

	
}

bool MyTriangleGradientShader::isOpaque()
{
	for (int i = 0; i < 3; i++) {
		if (colors[i].a != 1) return false;
	}
	return true;
}
bool MyTriangleBitmapShader::isOpaque() {
	return fShader->isOpaque();
}


bool MyTriangleGradientShader::setContext(const GMatrix& ctm) {
	float det = (ctm[0] * ctm[4]) - (ctm[1] * ctm[3]);
	if (det == 0.0) return false;

	context = ctm;
	return true;
}


bool MyTriangleBitmapShader::setContext(const GMatrix& ctm) {
	return fShader->setContext(ctm * localMatrix);
}


void MyTriangleGradientShader::shadeRow(int x, int y, int count, GPixel row[]) {
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
		float dx = shaderPoints[i].x();
		float dy = shaderPoints[i].y();


		float r, g, b, a;
		r = colors[0].r * (1.0f - dx - dy) + colors[1].r * dx + colors[2].r * dy;
		g = colors[0].g * (1.0f - dx - dy) + colors[1].g * dx + colors[2].g * dy;
		b = colors[0].b * (1.0f - dx - dy) + colors[1].b * dx + colors[2].b * dy;
		a = colors[0].a * (1.0f - dx - dy) + colors[1].a * dx + colors[2].a * dy;

		//cant use premul func for some reason!
		int alp = GRoundToInt(255 * a);
		int red = GRoundToInt(255 * a * r);
		int gre = GRoundToInt(255 * a * g);
		int blu = GRoundToInt(255 * a * b);

		row[i] = GPixel_PackARGB(alp, red, gre, blu);
	}
}


void MyTriangleBitmapShader::shadeRow(int x, int y, int count, GPixel row[]) {
	fShader->shadeRow(x, y, count, row);
}



std::unique_ptr<GShader> GCreateTriangleGradient(GPoint p[], const GColor colors[]) {
	MyTriangleGradientShader* myTriangleGradientShader = new MyTriangleGradientShader(p, colors);
	return std::unique_ptr<GShader>(myTriangleGradientShader);
}


std::unique_ptr<GShader> GCreateTriangleBitmapShader(GShader* shader, GPoint p[], GPoint t[]) {
	MyTriangleBitmapShader* myTriangleBitmapShader = new MyTriangleBitmapShader(shader, p, t);
	return std::unique_ptr<GShader>(myTriangleBitmapShader);
}