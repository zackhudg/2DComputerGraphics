#include "GPixel.h"
#include "GColor.h"
#include "GMath.h"
#include "GBlendMode.h"
#include <iostream>
#include <algorithm>
#include <array>
#include <vector>

int div255(int value) {
	return (value + 128) * 257 >> 16;
}

GPixel do_premul(GColor color) {
	int alp = GRoundToInt(255 * color.a);
	int red = GRoundToInt(255 * color.a * color.r);
	int gre = GRoundToInt(255 * color.a * color.g);
	int blu = GRoundToInt(255 * color.a * color.b);
	return GPixel_PackARGB(alp, red, gre, blu);
}

GPixel doBlendMath(int a1, int c1, int a2, int c2) {
	return div255(a1 * c1 + (255 - a2) * c2);
}

std::array<int, 4> getARGB(GPixel pixel) {
	std::array<int, 4> ret = { GPixel_GetA(pixel), GPixel_GetR(pixel), GPixel_GetG(pixel), GPixel_GetB(pixel) };
	return ret;
}

GPixel kSrcOver(GPixel src, GPixel dst) {
	std::array<int, 4> s = getARGB(src);
	std::array<int, 4> d = getARGB(dst);

	int so[4] = { 0,0,0,0 };
	for (int i = 0; i < 4; i++) {
		so[i] = doBlendMath(255, s[i], s[0], d[i]);
	}

	return GPixel_PackARGB(so[0], so[1], so[2], so[3]);
}

GPixel kDstOver(GPixel src, GPixel dst) {
	//TODO: div255
	std::array<int, 4> s = getARGB(src);
	std::array<int, 4> d = getARGB(dst);

	int dst_o[4] = { 0,0,0,0 };
	for (int i = 0; i < 4; i++) {
		dst_o[i] = doBlendMath(255, d[i], d[0], s[i]);
	}

	return GPixel_PackARGB(dst_o[0], dst_o[1], dst_o[2], dst_o[3]);
}

GPixel kSrcIn(GPixel src, GPixel dst) {
	//TODO: div255
	std::array<int, 4> s = getARGB(src);
	std::array<int, 4> d = getARGB(dst);

	int si[4] = { 0,0,0,0 };
	for (int i = 0; i < 4; i++) {
		si[i] = doBlendMath(d[0], s[i], 255, 0);
	}

	return GPixel_PackARGB(si[0], si[1], si[2], si[3]);
}

GPixel kDstIn(GPixel src, GPixel dst) {
	//TODO: div255
	std::array<int, 4> s = getARGB(src);
	std::array<int, 4> d = getARGB(dst);

	int di[4] = { 0,0,0,0 };
	for (int i = 0; i < 4; i++) {
		di[i] = doBlendMath(s[0], d[i], 255, 0);
	}

	return GPixel_PackARGB(di[0], di[1], di[2], di[3]);
}

GPixel kSrcOut(GPixel src, GPixel dst) {
	//TODO: div255
	std::array<int, 4> s = getARGB(src);
	std::array<int, 4> d = getARGB(dst);

	int so[4] = { 0,0,0,0 };
	for (int i = 0; i < 4; i++) {
		so[i] = doBlendMath(0, 0, d[0], s[i]);
	}

	return GPixel_PackARGB(so[0], so[1], so[2], so[3]);
}

GPixel kDstOut(GPixel src, GPixel dst) {
	//TODO: div255
	std::array<int, 4> s = getARGB(src);
	std::array<int, 4> d = getARGB(dst);

	int d_o[4] = { 0,0,0,0 };
	for (int i = 0; i < 4; i++) {
		d_o[i] = doBlendMath(0, 0, s[0], d[i]);
	}

	return GPixel_PackARGB(d_o[0], d_o[1], d_o[2], d_o[3]);
}

GPixel kSrcATop(GPixel src, GPixel dst) {
	//TODO: div255
	std::array<int, 4> s = getARGB(src);
	std::array<int, 4> d = getARGB(dst);

	int sat[4] = { 0,0,0,0 };
	for (int i = 0; i < 4; i++) {
		sat[i] = doBlendMath(d[0], s[i], s[0], d[i]);
	}

	return GPixel_PackARGB(sat[0], sat[1], sat[2], sat[3]);
}

GPixel kDstATop(GPixel src, GPixel dst) {
	//TODO: div255
	std::array<int, 4> s = getARGB(src);
	std::array<int, 4> d = getARGB(dst);

	int dat[4] = { 0,0,0,0 };
	for (int i = 0; i < 4; i++) {
		dat[i] = doBlendMath(s[0], d[i], d[0], s[i]);
	}

	return GPixel_PackARGB(dat[0], dat[1], dat[2], dat[3]);
}

GPixel kXor(GPixel src, GPixel dst) {
	//TODO: div255
	std::array<int, 4> s = getARGB(src);
	std::array<int, 4> d = getARGB(dst);

	int x[4] = { 0,0,0,0 };
	for (int i = 0; i < 4; i++) {
		x[i] = doBlendMath(255 - s[0], d[i], d[0], s[i]);
	}

	return GPixel_PackARGB(x[0], x[1], x[2], x[3]);
}

GPixel do_blend(GPixel src, GPixel dst, GBlendMode blendMode) {
	switch (blendMode) {
	case GBlendMode::kClear:
		return GPixel_PackARGB(0, 0, 0, 0);
		break;
	case GBlendMode::kSrc:
		return src;
		break;
	case GBlendMode::kDst:
		return dst;
		break;
	case GBlendMode::kSrcOver:
		return kSrcOver(src, dst);
		break;
	case GBlendMode::kDstOver:
		return kDstOver(src, dst);
		break;
	case GBlendMode::kSrcIn:
		return kSrcIn(src, dst);
		break;
	case GBlendMode::kDstIn:
		return kDstIn(src, dst);
		break;
	case GBlendMode::kSrcOut:
		return kSrcOut(src, dst);
		break;
	case GBlendMode::kDstOut:
		return kDstOut(src, dst);
		break;
	case GBlendMode::kSrcATop:
		return kSrcATop(src, dst);
		break;
	case GBlendMode::kDstATop:
		return kDstATop(src, dst);
		break;
	case GBlendMode::kXor:
		return kXor(src, dst);
		break;
	}
}

struct Edge {
	float m, x;
	int top, bot, wind;

	bool operator < (const Edge& e2) const
	{
		if (bot != e2.bot) {
			return bot < e2.bot;
		}
		else if (x != e2.x) {
			return x < e2.x;
		}
		else {
			return m < e2.m;
		}
	}
};

void createEdge(GPoint p1, GPoint p2, int maxX, int maxY, std::vector<Edge>& edges) {
	Edge edge;

	float p1x = p1.x();
	float p1y = p1.y();
	float p2x = p2.x();
	float p2y = p2.y();

	int dy = GRoundToInt(p2y) - GRoundToInt(p1y);
	if (dy == 0) {
		return;
	}

	if (dy > 0) {
		edge.wind = 1;
	}
	else {
		edge.wind = -1;
	}

	//fully out of bounds. Can improve this by redefining edges partially OOB x
	//in y
	if (p1y < 0 && p2y < 0) {
		return;
	}
	if (p1y > maxY && p2y > maxY) {
		return;
	}
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
	float diff;
	if (edge.bot == GRoundToInt(p1y)) {
		diff = 0.5 + (edge.bot - p1y);
		edge.x = p1x + (diff * edge.m);
	}
	else {
		diff = 0.5f + (edge.bot - p2y);
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
