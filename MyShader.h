#include "GShader.h"
#include "GBitmap.h"
#include "GMatrix.h"
#include <vector>

class MyShader : public GShader {
public:
	GMatrix context;
	GMatrix localMatrix;
	GShader::TileMode tilemode;

	MyShader();
	MyShader(const GBitmap& shader, const GMatrix& localMatrix, GShader::TileMode tilemode);

	bool isOpaque();

	bool setContext(const GMatrix& ctm);

	void shadeRow(int x, int y, int count, GPixel row[]);

private:
	const GBitmap fShader;
};

class MyGradientShader : public MyShader {
public:
	GPoint p0, p1;
	std::vector<GColor> colors;
	GMatrix context;
	GShader::TileMode tilemode;

	MyGradientShader(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode tilemode);

	bool setContext(const GMatrix& ctm);
	
	bool isOpaque();

	void shadeRow(int x, int y, int count, GPixel row[]);

private:
	GMatrix localMatrix;
	float dx, dy, length, segmentLength;
};


