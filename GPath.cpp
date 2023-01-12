#include "GPath.h"
#include <iostream>
#include <array>

GPath& GPath::addRect(const GRect& r, Direction dir) {
	GPath path = moveTo({ r.left(), r.top() });
	switch (dir) {
	case kCW_Direction:
		lineTo({ r.right(), r.top() });
		lineTo({ r.right(), r.bottom() });
		lineTo({ r.left(), r.bottom() });
		break;
	case kCCW_Direction:
		lineTo({ r.left(), r.bottom() });
		lineTo({ r.right(), r.bottom() });
		lineTo({ r.right(), r.top() });
		break;
	}
	return path;
}

GPath& GPath::addPolygon(const GPoint pts[], int count) {
	//check count >= 1
	GPath path = moveTo(pts[0]);
	for (int i = 1; i < count; i++) {
		lineTo(pts[i]);
	}
	return path;
}

GRect GPath::bounds() const {
	//can maybe add a class to statically hold bounds as they are made. For now though:

	if (fPts.empty()) return GRect::LTRB(0, 0, 0, 0);

	float left, top, right, bottom;
	GPoint point = fPts.at(0);
	left = fPts.at(0).x();
	right = fPts.at(0).x();
	top = fPts.at(0).y();
	bottom = fPts.at(0).y();

	for (int i = 1; i < fPts.size(); i++) {
		point = fPts.at(i);
		if (point.x() < left) left = point.x();
		if (point.x() > right) right = point.x();
		if (point.y() < top) top = point.y();
		if (point.y() > bottom) bottom = point.y();
	}

	return GRect::LTRB(left, top, right, bottom);
}

void GPath::transform(const GMatrix& m) {
	GPoint* p = &fPts[0];
	m.mapPoints(p, p, fPts.size());
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
	//find AB and BC at t
	GPoint a = src[0];
	GPoint b = src[1];
	GPoint c = src[2];

	GPoint t0 = { (1 - t) * a.x() + t * b.x(), (1 - t) * a.y() + t * b.y() };
	GPoint t1 = { (1 - t) * b.x() + t * c.x(), (1 - t) * b.y() + t * c.y() };

	//eval at t
	GPoint t2 = { (1 - t) * t0.x() + t * t1.x(), (1 - t) * t0.y() + t * t1.y() };

	dst[0] = a;
	dst[1] = t0;
	dst[2] = t2;
	dst[3] = t1;
	dst[4] = c;
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
	GPoint a = src[0];
	GPoint b = src[1];
	GPoint c = src[2];
	GPoint d = src[3];

	GPoint dstQ1[5];
	ChopQuadAt(src, dstQ1, t);
	GPoint dstQ2[5];
	ChopQuadAt(&src[1], dstQ2, t);

	GPoint t2 = dstQ1[2];
	GPoint t3 = dstQ2[2];
	GPoint t5 = { (1 - t) * t2.x() + t * t3.x(), (1 - t) * t2.y() + t * t3.y() };

	dst[0] = a;
	dst[1] = dstQ1[1];
	dst[2] = t2;
	dst[3] = t5;
	dst[4] = t3;
	dst[5] = dstQ2[3];
	dst[6] = d;
}

GPath& GPath::addCircle(GPoint center, float radius, Direction dir) {
	float tan = 0.41421356237;
	float halfSqt2 = 0.70710678118;
	GPath path;
	path.moveTo({ 1.0f, 0.0f });
	switch (dir) {
	case kCW_Direction:
		path.quadTo({ 1.0f, tan }, { halfSqt2, halfSqt2 });
		path.quadTo({ tan, 1.0f }, { 0.0f, 1.0f });

		path.quadTo({ -1.0f * tan, 1.0f }, { -1.0f * halfSqt2, halfSqt2 });
		path.quadTo({ -1.0f, tan }, { -1.0f, 0.0f });

		path.quadTo({ -1.0f, -1.0f * tan }, { -1.0f * halfSqt2, -1.0 * halfSqt2 });
		path.quadTo({ -1.0f * tan, -1.0f }, { 0.0f, -1.0f });

		path.quadTo({ tan, -1.0f }, { halfSqt2, -1.0f * halfSqt2 });
		path.quadTo({ 1.0f, -1.0f * tan }, { 1.0f, 0.0f });
		break;
	case kCCW_Direction:
		path.quadTo({ 1.0f, -1.0f * tan }, { halfSqt2, -1.0f * halfSqt2 });
		path.quadTo({ tan, -1.0f }, { 0.0f, -1.0f });

		path.quadTo({ -1.0f * tan, -1.0f }, { -1.0f * halfSqt2, -1.0 * halfSqt2 });
		path.quadTo({ -1.0f, -1.0f * tan }, { -1.0f, 0.0f });

		path.quadTo({ -1.0f, tan }, { -1.0f * halfSqt2, halfSqt2 });
		path.quadTo({ -1.0f * tan, 1.0f }, { 0.0f, 1.0f });

		path.quadTo({ tan, 1.0f }, { halfSqt2, halfSqt2 });
		path.quadTo({ 1.0f, tan }, { 1.0f, 0.0f });
		break;
	}

	GMatrix m = GMatrix::Translate(center.x(), center.y()) * GMatrix::Scale(radius, radius);
	path.transform(m);

	fPts.insert(fPts.end(), path.fPts.begin(), path.fPts.end());
	fVbs.insert(fVbs.end(), path.fVbs.begin(), path.fVbs.end());

	return path;
}
