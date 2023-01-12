/*
 *  Copyright 2022 <me>
 */

#ifndef MyCanvas_DEFINED
#define MyCanvas_DEFINED

#include <iostream>
#include <memory>
#include <vector>

#include "GCanvas.h"
#include "GBitmap.h"

class MyCanvas : public GCanvas {
public:

    MyCanvas(const GBitmap& device);// : fDevice(device) {}

    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
        int count, const int indices[], const GPaint& paint);

    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
        int level, const GPaint&);

    void drawPath(const GPath&, const GPaint&);

    void save();

    void restore();

    void concat(const GMatrix& matrix);

    void blit(int left, int right, int y, const GPaint paint);

    void drawPaint(const GPaint& paint);

    void drawRect(const GRect& rect, const GPaint& paint);

    void drawConvexPolygon(const GPoint* points, int count, const GPaint& paint);

private:
    // Note: we store a copy of the bitmap
    const GBitmap fDevice;
    std::vector<GMatrix> CTMStack;
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device);

std::string GDrawSomething(GCanvas* canvas, GISize dim);


#endif