#include "GShader.h"
#include "GMatrix.h"
#include "GBitmap.h"

class MyTriangleGradientShader : public GShader {
public:
	GColor colors[3];
	GMatrix context;

	MyTriangleGradientShader(GPoint p[], const GColor colors[]);

	bool setContext(const GMatrix& ctm);

	bool isOpaque();

	void shadeRow(int x, int y, int count, GPixel row[]);

private:
	GMatrix localMatrix;
};

class MyTriangleBitmapShader : public GShader {
public:

	MyTriangleBitmapShader(GShader* shader, GPoint p[], GPoint t[]);

	bool setContext(const GMatrix& ctm);

	bool isOpaque();

	void shadeRow(int x, int y, int count, GPixel row[]);

private:
	GMatrix localMatrix;
	GShader* fShader;
};


std::unique_ptr<GShader> GCreateTriangleGradient(GPoint p[], const GColor colors[]);
std::unique_ptr<GShader> GCreateTriangleBitmapShader(GShader*, GPoint p[], GPoint t[]);