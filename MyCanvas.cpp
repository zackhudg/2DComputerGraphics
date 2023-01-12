#include "MyCanvas.h"

#include "GRect.h"
#include "GColor.h"
#include "GBitmap.h"
#include "GMath.h"
#include "GPixel.h"
#include "GColor.h"
#include "MyUtils.h"
#include "GMatrix.h"
#include "GShader.h"
#include "GPath.h"
#include "GMath.h"
#include "MyTriangleShader.h"
#include <iostream>
#include <algorithm>
#include <memory>
#include <vector>

MyCanvas::MyCanvas(const GBitmap& device) : fDevice(device) {
    CTMStack.push_back(GMatrix());
}

void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint textures[4], int level, const GPaint& paint) {
    //std::cout << "x0: " << verts[0].x() << ", y0: " << verts[0].y() << ", x1: " << verts[1].x() << ", y1: " << verts[1].y() << ", x2: " << verts[2].x() << ", y2: " << verts[2].y() << ", x3: " << verts[3].x() << ", y3: " << verts[3].y() << "\n";
    
    int rowLength = 2 + level;
    int vertCount = rowLength * rowLength;
    GPoint vertsMesh[vertCount];
    GColor colorsMesh[vertCount];
    GPoint texturesMesh[vertCount];

    int triCount = 2 * pow(level + 1, 2);
    int indices[3 * triCount];
    float inverseRowLength = 1.0f / (rowLength-1);
    
    int currVert = 0;
    float v = 0.0f; 
    while(v <= 1.0f) {
        float u = 0.0f;
        while(u <= 1.0f) {
            GPoint a = (1.0f - u) * verts[0] + u * verts[1];
            GPoint b = (1.0f - u) * verts[3] + u * verts[2];
            vertsMesh[currVert] =  (1.0f - v) * a + v * b;
            //std::cout << "x: " << vertsMesh[currVert].x() << ", y: " << vertsMesh[currVert].y() << "\n";
        
           if (colors != nullptr) {
               GColor ac = (1.0f - u) * colors[0] + u * colors[1];
               GColor bc = (1.0f - u) * colors[3] + u * colors[2];
               colorsMesh[currVert] = (1.0f - v) * ac + v * bc;
           }
           
           if (textures != nullptr) {
               GPoint at = (1.0f - u) * textures[0] + u * textures[1];
               GPoint bt = (1.0f - u) * textures[3] + u * textures[2];
               texturesMesh[currVert] = (1.0f - v) * at + v * bt;
           }
           currVert++;
           u += inverseRowLength;
        }
        v += inverseRowLength;
    }

    //std::cout << "x0: " << vertsMesh[0].x() << ", y0: " << vertsMesh[0].y() << ", x1: " << vertsMesh[9].x() << ", y1: " << vertsMesh[9].y() << ", x2: " << vertsMesh[99].x() << ", y2: " << vertsMesh[99].y() << ", x3: " << vertsMesh[90].x() << ", y3: " << vertsMesh[90].y() << "\n";


    int index = 0;
    int i;
    int j;
    for (int v = 0; v < rowLength - 1; v++) {
        i = v * rowLength;
        j = (v + 1) * rowLength;
        for (int k = 0; k < rowLength - 1; k++) {
            //triangle 1
            indices[index] = i;
            i++;
            index++;
            indices[index] = i;
            index++;
            indices[index] = j;
            index++;
            //triangle 2
            indices[index] = i;
            index++;
            indices[index] = j;
            j++;
            index++;
            indices[index] = j;
            index++;
        }

    }

    //std::cout <<"TriCount: " << triCount << "\n";

    if (colors == nullptr) {
        if (textures == nullptr) drawMesh(vertsMesh, nullptr, nullptr, triCount, indices, paint);
        else drawMesh(vertsMesh, nullptr, texturesMesh, triCount, indices, paint);
    }
    else {
        if (textures == nullptr) drawMesh(vertsMesh, colorsMesh, nullptr,  triCount, indices, paint);
        else drawMesh(vertsMesh, colorsMesh, texturesMesh, triCount, indices, paint);
    }
}

void MyCanvas::drawMesh(const GPoint verts[], const GColor colors[], const GPoint textures[], int count, const int indices[], const GPaint& paint) {
    for (int i = 0; i < count * 3; i += 3) {
        std::unique_ptr<GShader> shader;
        int i0 = indices[i];
        int i1 = indices[i + 1];
        int i2 = indices[i + 2];
        //std::cout << "Indices: " << i0 << ", " << i1 << ", " << i2 << "\n";

        GPoint triVerts[3];
        GPoint triTextures[3];
        GColor triColors[3];

        triVerts[0] = verts[i0];
        triVerts[1] = verts[i1];
        triVerts[2] = verts[i2];
        GPaint myPaint = GPaint(paint);

        if (colors != nullptr) {
            triColors[0] = colors[i0];
            triColors[1] = colors[i1];
            triColors[2] = colors[i2];
            //TODO: paint?
            shader = GCreateTriangleGradient(triVerts, triColors);
            myPaint.setShader(shader.get());
        }
        if (textures != nullptr) {
            triTextures[0] = textures[i0];
            triTextures[1] = textures[i1];
            triTextures[2] = textures[i2];
            //TODO paint?
            shader = GCreateTriangleBitmapShader(paint.getShader(), triVerts, triTextures);
            myPaint.setShader(shader.get());
        }

        //drawTriangle
        //std::cout << "0: " << triVerts[0].x() << ", " << triVerts[0].y() << " \n";
        //std::cout << "1: " << triVerts[1].x() << ", " << triVerts[1].y() << " \n";
        //std::cout << "2: " << triVerts[2].x() << ", " << triVerts[2].y() << " \n";
        drawConvexPolygon(triVerts, 3, myPaint);
    }

}

void MyCanvas::drawPath(const GPath& og_path, const GPaint& paint) {
    GShader* shader = paint.getShader();
    if (shader != nullptr) {
        shader->setContext(CTMStack.back());
    }

    GPath path = og_path;

    path.transform(CTMStack.back());

    GRect bounds = path.bounds();

    std::vector<Edge> edges;

    GPoint pts[GPath::kMaxNextPoints];
    GPoint a, b, c, d;
    GPath::Edger edger(path);
    GPath::Verb v;

    int maxX = fDevice.width();
    int maxY = fDevice.height();

    while ((v = edger.next(pts)) != GPath::kDone) {
        a = pts[0];
        b = pts[1];
        c = pts[2];
        d = pts[3];
        GPoint p1, p2;
        float dx, dy, distance, increment, t;
        int numSegments;
        GPoint t2;

        switch (v) {
        case GPath::kLine:
            createEdge(a, b, maxX, maxY, edges);
            break;

        case GPath::kQuad:
            dx = 0.25f * (a.x() - 2 * b.x() + c.x());
            dy = 0.25f * (a.y() - 2 * b.y() + c.y());
            distance = sqrtf(dx * dx + dy * dy);
            numSegments = GCeilToInt(sqrtf(distance / 0.25f));
            increment = 1.0f / numSegments;
            p1 = a;
            t = increment; 

            for (int i = 0; i < numSegments; i++) {
                t2 = { ((a.x() - 2 * b.x() + c.x()) * t + 2 * (b.x() - a.x())) * t + a.x(), ((a.y() - 2 * b.y() + c.y()) * t + 2 * (b.y() - a.y())) * t + a.y() };
                p2 = t2;
                createEdge(p1, p2, maxX, maxY, edges);
                p1 = p2;
                t += increment;
            }
            break;

        case GPath::kCubic:
            //error could be very wrong
            dx = std::max(std::abs(a.x() + 2.0f * b.x() + c.x()), std::abs(b.x() + 2.0f * c.x() + d.x()));
            dy = std::max(std::abs(a.y() + 2.0f * b.y() + c.y()), std::abs(b.y() + 2.0f * c.y() + d.y()));
            distance = sqrtf(dx * dx + dy * dy);
            numSegments = GCeilToInt(sqrtf(3 * distance)); // (3 * d) / (4 * 0.25)
            increment = 1.0f / numSegments;
            p1 = a;
            t = increment;

            for (int i = 0; i < numSegments; i++) {
                t2 = { (((d.x() + 3 * (b.x() - c.x()) - a.x()) * t + 3 * (a.x() - 2 * b.x() + c.x())) * t + 3 * (b.x() - a.x())) * t + a.x(), (((d.y() + 3 * (b.y() - c.y()) - a.y()) * t + 3 * (a.y() - 2 * b.y() + c.y())) * t + 3 * (b.y() - a.y())) * t + a.y() };
                p2 = t2;
                createEdge(p1, p2, maxX, maxY, edges);
                p1 = p2;
                t += increment;
            }
            break;
        }
    }

    if (edges.size() < 2) return;
    std::sort(edges.begin(), edges.end());

    //BOUNDS AND POINTS NOT CONSISTENT
    for (int i = edges.at(0).bot; i < edges.back().top; i++) {
        int size = edges.size();
        if (size < 2) return;

        int windCount = 0;
        //can maybe integrate into loop, idk
        Edge first = edges.at(0);
        windCount += (first.wind);
        int indexCounter = 1;

        //hopefully doesn't corrupt!
        Edge last;

        //watch for null pointer
        while (indexCounter < size && edges.at(indexCounter).bot == i) {
            last = edges.at(indexCounter);
            windCount += last.wind;

            if (windCount == 0) {
                blit(GRoundToInt(first.x), GRoundToInt(last.x), i, paint);
            }
            if (windCount == last.wind) first = last;

            indexCounter++;
        }

        //Update array!
        std::vector<int> toBeErased;
        for (int j = 0; j < indexCounter; j++) {
            edges.at(j).bot += 1;
            edges.at(j).x += edges.at(j).m;
            if (edges.at(j).bot >= edges.at(j).top) toBeErased.push_back(j);
        }

        while (!toBeErased.empty()) {
            edges.erase(edges.begin() + toBeErased.back());
            toBeErased.pop_back();
        }

        std::sort(edges.begin(), edges.end());
    }

    return;
}

void MyCanvas::save() {
    CTMStack.push_back(CTMStack.back());
}

void MyCanvas::restore() {
    if (CTMStack.size() < 2) return;
    CTMStack.pop_back();
}

void MyCanvas::concat(const GMatrix& matrix) {
    CTMStack.back() = CTMStack.back() * matrix;
}

void MyCanvas::blit(int left, int right, int y, const GPaint paint) {
    GBlendMode blendMode = paint.getBlendMode();
    GShader* shader = paint.getShader();

    left = std::min(fDevice.width(), std::max(left, 0));
    right = std::max(0, std::min(right, fDevice.width()));
    //assert y in bounds. should be done before this is called but never know!
    if (shader == nullptr) {
        GPixel src = do_premul(paint.getColor());
        for (int x = left; x < right; x++) {
            GPixel dst = *fDevice.getAddr(x, y);
            *fDevice.getAddr(x, y) = do_blend(src, dst, blendMode);
        }
    }
    else {
        int count = right - left;
        GPixel shaderRow[count];
        shader->shadeRow(left, y, count, shaderRow);
        for (int x = left; x < right; x++) {
            GPixel dst = *fDevice.getAddr(x, y);
            GPixel src = shaderRow[x - left];
            *fDevice.getAddr(x, y) = do_blend(src, dst, blendMode);
        }
    }
}

void MyCanvas::drawPaint(const GPaint& paint) {
    GShader* shader = paint.getShader();
    if (shader != nullptr) shader->setContext(CTMStack.back());

    for (int y = 0; y < fDevice.height(); y++) {
        blit(0, fDevice.width(), y, paint);
    }
}

void MyCanvas::drawRect(const GRect& rect, const GPaint& paint) {
    GShader* shader = paint.getShader();
    GMatrix context = CTMStack.back();

    float fTop = rect.top();
    float fBottom = rect.bottom();
    float fLeft = rect.left();
    float fRight = rect.right();

    //Point mapping  happens here!
    //non-scalar matrix -> sent off to dCP where points get mapped
    if (context[1] != 0 || context[3] != 0) {
        GPoint points[4];
        points[0] = { fLeft, fTop };
        points[1] = { fRight, fTop };
        points[2] = { fRight, fBottom };
        points[3] = { fLeft, fBottom };

        drawConvexPolygon(points, 4, paint);
        return;
    }

    //scalar matrix -> just scale and move center. Don't need to map really
    fTop = (fTop * context[4]) + context[5];
    fBottom = (fBottom * context[4]) + context[5];
    fLeft = (fLeft * context[0]) + context[2];
    fRight = (fRight * context[0]) + context[2];

    if (shader != nullptr) shader->setContext(CTMStack.back());
    GIRect irect = GIRect::LTRB(GRoundToInt(fLeft), GRoundToInt(fTop), GRoundToInt(fRight), GRoundToInt(fBottom));

    int y_start = std::max(0, irect.top());
    int x_start = std::max(0, irect.left());
    int y_end = std::min(irect.bottom(), fDevice.height());
    int x_end = std::min(irect.right(), fDevice.width());

    for (int y = y_start; y < y_end; y++) {
        blit(x_start, x_end, y, paint);
    }
}

void MyCanvas::drawConvexPolygon(const GPoint* og_points, int count, const GPaint& paint) {
    GShader* shader = paint.getShader();
    if (shader != nullptr) {
        shader->setContext(CTMStack.back());
    }

    GPoint points[count];
    CTMStack.back().mapPoints(points, og_points, count);

    int maxX = fDevice.width();
    int maxY = fDevice.height();

    std::vector<Edge> edges;
    for (int i = 0; i < count; i++) {
        GPoint p1 = points[i];
        GPoint p2 = points[(i + 1) % count];
        Edge edge;
        float p1x = p1.x();
        float p1y = p1.y();
        float p2x = p2.x();
        float p2y = p2.y();

        int dy = GRoundToInt(p2y - p1y);
        if (dy == 0) continue;

        if (dy > 0) {
            edge.wind = 1;
        }
        else {
            edge.wind = -1;
        }

        //fully out of bounds. Can improve this by redefining edges partially OOB x
        //in y
        if (p1y < 0 && p2y < 0) continue;
        if (p1y > maxY && p2y > maxY) continue;
        //in x
        if (p1x < 0 && p2x < 0) {
            p1x = 0;
            p2x = 0;
        }
        if (p1x > maxX && p2x > maxX) {
            p1x = static_cast<float> (maxX);
            p2x = static_cast<float> (maxX);
        }

        edge.m = (p2x - p1x) / (p2y - p1y);
        edge.bot = GRoundToInt(std::min(p1y, p2y));
        edge.top = GRoundToInt(std::max(p1y, p2y));
        if (edge.bot == GRoundToInt(p1y)) {
            float diff = 0.5 + (edge.bot - p1y);
            edge.x = p1x + (diff * edge.m);
        }
        else {
            float diff = 0.5f + (edge.bot - p2y);
            edge.x = p2x + (diff * edge.m);
        }

        //partially out of bounds. quick clipping - maybe it works!
        if (edge.bot < 0) {
            edge.x += edge.m * edge.bot * (-1);
            edge.bot = 0;
        }
        if (edge.top > maxY) {
            edge.top = maxY;
        }

        edges.push_back(edge);
    }

    if (edges.size() < 2) return;
    std::sort(edges.begin(), edges.end());

    //idk assert that there are edges
    //assert(array.size() > 0);

    for (int i = edges.at(0).bot; i < edges.back().top; i++) {
        if (edges.size() < 2) return;

        int left = GRoundToInt(edges.at(0).x);
        int right = GRoundToInt(edges.at(1).x);

        blit(left, right, i, paint);

        edges.at(0).bot += 1;
        edges.at(0).x += edges.at(0).m;
        edges.at(1).bot += 1;
        edges.at(1).x += edges.at(1).m;

        //Update Array!
        if (edges.at(1).bot >= edges.at(1).top) {
            edges.erase(edges.begin() + 1);
        }
        if (edges.at(0).bot >= edges.at(0).top) {
            edges.erase(edges.begin());
        }

        std::sort(edges.begin(), edges.end());
    }
}

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    auto sky_gradient = GCreateLinearGradient({ 128, 0 }, { 128, 128 }, { 0, .1, .8, 1 }, { 0, .7, 1, 1 });
    GPaint sky(sky_gradient.get());

    canvas->drawRect(GRect::LTRB(0, 0, 256, 256), sky);

    auto sun_gradient = GCreateLinearGradient({ 0,0 }, { 0,15 }, { 1,1,0,1 }, { 1,.6,0,.8 }, GShader::kMirror);
    GPaint sun = GPaint(sun_gradient.get());
    GPath sunpath;
    sunpath.moveTo(64, 128);
    sunpath.quadTo({ 128, 64 }, { 192, 128 });
    canvas->drawPath(sunpath, sun);

    GPaint ocean({ 0, 0, 1, 0.5 });
    canvas->drawRect(GRect::LTRB(0, 128, 256, 256), ocean); //ocean

    canvas->drawRect(GRect::LTRB(0, 192, 256, 256), GPaint({ 1, 1, 0, 1 })); //beach

    GPoint treePoints[] = { { 50, 200 }, { 60, 50 }, { 80, 50 }, { 90, 200 } };
    auto tree_gradient = GCreateLinearGradient({ 0,0 }, { 0,50 }, { .9,.8,.5, 1 }, { .6,.6,.3,1 }, GShader::kRepeat);
    GPaint tree = GPaint(tree_gradient.get());
    canvas->drawConvexPolygon(treePoints, 4, tree);//tree!

    GPoint leafPoints[] = { { 0, 0 }, { -25, -25 }, { -50, -25 }, { -75, 0 }, { -50, 25 }, { -25, 25 } };
    GPaint green({ 0, 1, 0, .8 });
    canvas->save();
    canvas->translate(70, 50);
    canvas->drawConvexPolygon(leafPoints, 6, green);
    canvas->rotate(.62);
    canvas->drawConvexPolygon(leafPoints, 6, green);
    canvas->rotate(3.14);
    canvas->drawConvexPolygon(leafPoints, 6, green);
    canvas->rotate(-.62);
    canvas->drawConvexPolygon(leafPoints, 6, green);
    canvas->restore(); //leaves

    
    GColor raycolors[] = { {1,1,1,1}, {1,1,0,1},{1,1,1,1},{1,1,0,1} };
    GPoint raypts[] = { { 50,100 }, { 55, 95 }, { 25, 70 }, { 20, 75 } };
    GPoint raypts2[] = {{ 210,100 }, { 215, 95 }, { 240, 70 }, { 235, 75 }};

    canvas->drawQuad(raypts, raycolors, nullptr, 2, GPaint());
    canvas->drawQuad(raypts2,raycolors, nullptr, 4, GPaint());


    return "a beautiful sea";
}