/* Copyright Jukka Jyl�nki

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

/** @file float2.cpp
	@author Jukka Jyl�nki
	@brief */
#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "MathFunc.h"
#include "../Algorithm/Random/LCG.h"
#include "assume.h"
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#ifdef MATH_ENABLE_STL_SUPPORT
#include "myassert.h"
#include <iostream>
#include <utility>
#include <algorithm>
#endif

MATH_BEGIN_NAMESPACE

using namespace std;

float2::float2(float x_, float y_)
:x(x_), y(y_)
{
}

float2::float2(float scalar)
:x(scalar), y(scalar)
{
}

float2::float2(const float *data)
{
	assume(data);
#ifndef MATH_ENABLE_INSECURE_OPTIMIZATIONS
	if (!data)
		return;
#endif
	x = data[0];
	y = data[1];
}

CONST_WIN32 float float2::At(int index) const
{
	assume(index >= 0);
	assume(index < Size);
#ifndef MATH_ENABLE_INSECURE_OPTIMIZATIONS
	if (index < 0 || index >= Size)
		return FLOAT_NAN;
#endif
	return ptr()[index];
}

float &float2::At(int index)
{
	assume(index >= 0);
	assume(index < Size);
#ifndef MATH_ENABLE_INSECURE_OPTIMIZATIONS
	if (index < 0 || index >= Size)
		return ptr()[0];
#endif
	return ptr()[index];
}

float2 float2::Swizzled(int i, int j) const
{
	return float2(At(i), At(j));
}

float3 float2::Swizzled(int i, int j, int k) const
{
	return float3(At(i), At(j), At(k));
}

float4 float2::Swizzled(int i, int j, int k, int l) const
{
	return float4(At(i), At(j), At(k), At(l));
}

float float2::LengthSq() const
{
	return x*x + y*y;
}

float float2::Length() const
{
	return Sqrt(LengthSq());
}

void float2::SetFromPolarCoordinates(float theta, float length)
{
	float sin, cos;
	SinCos(theta, sin, cos);
	x = cos * length;
	y = sin * length;
}

float2 float2::FromPolarCoordinates(float theta, float length)
{
	float2 euclidean;
	euclidean.SetFromPolarCoordinates(theta, length);
	return euclidean;
}

float2 float2::ToPolarCoordinates() const
{
	float radius = Length();
	if (radius > 1e-4f)
		return float2(atan2(y, x), radius);
	else
		return float2::zero;
}

float float2::AimedAngle() const
{
	assume(!IsZero());
	return atan2(y, x);
}

float float2::Normalize()
{
	assume(IsFinite());
	float lengthSq = LengthSq();
	if (lengthSq > 1e-6f)
	{
		float length = Sqrt(lengthSq);
		*this *= 1.f / length;
		return length;
	}
	else
	{
		Set(1.f, 0.f); // We will always produce a normalized vector.
		return 0; // But signal failure, so user knows we have generated an arbitrary normalization.
	}
}

float2 float2::Normalized() const
{
	float2 copy = *this;
	float oldLength = copy.Normalize();
	assume(oldLength > 0.f && "float2::Normalized() failed!");
	MARK_UNUSED(oldLength);
	return copy;
}

float float2::ScaleToLength(float newLength)
{
	float length = LengthSq();
	if (length < 1e-6f)
		return 0.f;

	length = Sqrt(length);
	float scalar = newLength / length;
	x *= scalar;
	y *= scalar;
	return length;
}

float2 float2::ScaledToLength(float newLength) const
{
	assume(!IsZero());

	float2 v = *this;
	v.ScaleToLength(newLength);
	return v;
}

bool float2::IsNormalized(float epsilonSq) const
{
	return MATH_NS::Abs(LengthSq()-1.f) <= epsilonSq;
}

bool float2::IsZero(float epsilonSq) const
{
	return LengthSq() <= epsilonSq;
}

bool float2::IsFinite() const
{
	return MATH_NS::IsFinite(x) && MATH_NS::IsFinite(y);
}

bool float2::IsPerpendicular(const float2 &other, float epsilonSq) const
{
	float dot = Dot(other);
	return dot*dot <= epsilonSq * LengthSq() * other.LengthSq();
}

bool float2::Equals(const float2 &rhs, float epsilon) const
{
	return EqualAbs(x, rhs.x, epsilon) && EqualAbs(y, rhs.y, epsilon);
}

bool float2::Equals(float x_, float y_, float epsilon) const
{
	return EqualAbs(x, x_, epsilon) && EqualAbs(y, y_, epsilon);
}

bool float2::BitEquals(const float2 &other) const
{
	return ReinterpretAsU32(x) == ReinterpretAsU32(other.x) &&
		ReinterpretAsU32(y) == ReinterpretAsU32(other.y);
}

/// It is too performance-heavy to set the locale in each serialization and deserialization function call.
/// Therefore expect the user to has a proper locale set up for the application at startup. This is assert()ed
/// at debug runs.
bool IsNeutralCLocale()
{
	// Android NDK locale.h does not have struct lconv or localeconv() implemented, and only contains stub 
	// symbols with a comment 'MISSING FROM BIONIC - DEFINED TO MAKE libstdc++-v3 happy'
#ifndef ANDROID
	lconv *lc = localeconv();
	if (strcmp(lc->decimal_point, "."))
		return false;
#endif
	return true;
}

#ifdef MATH_ENABLE_STL_SUPPORT
std::string float2::ToString() const
{
	char str[256];
	sprintf(str, "(%f, %f)", x, y);
	return std::string(str);
}

std::string float2::SerializeToString() const
{
	char str[256];
	char *s = SerializeFloat(x, str); *s = ','; ++s;
	s = SerializeFloat(y, s);
	assert(s+1 - str < 256);
	MARK_UNUSED(s);
	return str;
}

std::string float2::SerializeToCodeString() const
{
	return "float2(" + SerializeToString() + ")";
}
#endif

float2 float2::FromString(const char *str, const char **outEndStr)
{
	assert(IsNeutralCLocale());
	assume(str);
	if (!str)
		return float2::nan;
	MATH_SKIP_WORD(str, "float2");
	MATH_SKIP_WORD(str, "(");
	float2 f;
	f.x = DeserializeFloat(str, &str);
	f.y = DeserializeFloat(str, &str);
	if (*str == ')')
		++str;
	if (*str == ',')
		++str;
	if (outEndStr)
		*outEndStr = str;
	return f;
}

float float2::SumOfElements() const
{
	return x + y;
}

float float2::ProductOfElements() const
{
	return x * y;
}

float float2::AverageOfElements() const
{
	return (x + y) * 0.5f;
}

float float2::MinElement() const
{
	return MATH_NS::Min(x, y);
}

int float2::MinElementIndex() const
{
	return (x <= y) ? 0 : 1;
}

float float2::MaxElement() const
{
	return  MATH_NS::Max(x, y);
}

int float2::MaxElementIndex() const
{
	return (x > y) ? 0 : 1;
}

float2 float2::Abs() const
{
	return float2(MATH_NS::Abs(x), MATH_NS::Abs(y));
}

float2 float2::Neg() const
{
	return float2(-x, -y);
}

float2 float2::Recip() const
{
	return float2(1.f/x, 1.f/y);
}

float2 float2::Min(float floor) const
{
	return float2(MATH_NS::Min(x, floor),  MATH_NS::Min(x, floor));
}

float2 float2::Min(const float2 &floor) const
{
	return float2(MATH_NS::Min(x, floor.x),  MATH_NS::Min(x, floor.x));
}

float2 float2::Max(float ceil) const
{
	return float2(MATH_NS::Max(x, ceil),  MATH_NS::Max(x, ceil));
}

float2 float2::Max(const float2 &ceil) const
{
	return float2(MATH_NS::Max(x, ceil.x),  MATH_NS::Max(x, ceil.x));
}

float2 float2::Clamp(const float2 &floor, const float2 &ceil) const
{
	return float2(MATH_NS::Clamp(x, floor.x, ceil.x),  MATH_NS::Clamp(y, floor.y, ceil.y));
}

float2 float2::Clamp(float floor, float ceil) const
{
	return float2(MATH_NS::Clamp(x, floor, ceil),  MATH_NS::Clamp(y, floor, ceil));
}

float2 float2::Clamp01() const
{
	return Clamp(0.f, 1.f);
}

float float2::DistanceSq(const float2 &rhs) const
{
	float dx = x - rhs.x;
	float dy = y - rhs.y;
	return dx*dx + dy*dy;
}

float float2::Distance(const float2 &rhs) const
{
	return Sqrt(DistanceSq(rhs));
}

float float2::Dot(const float2 &rhs) const
{
	return x * rhs.x + y * rhs.y;
}

float2 float2::Perp() const
{
	return float2(-y, x);
}

float float2::PerpDot(const float2 &rhs) const
{
	return x * rhs.y - y * rhs.x;
}

float2 float2::Reflect(const float2 &normal) const
{
	assume2(normal.IsNormalized(), normal.SerializeToCodeString(), normal.Length());
	return 2.f * this->ProjectToNorm(normal) - *this;
}

/// Implementation from http://www.flipcode.com/archives/reflection_transmission.pdf .
float2 float2::Refract(const float2 &normal, float negativeSideRefractionIndex, float positiveSideRefractionIndex) const
{
	// This code is duplicated in float3::Refract.
	float n = negativeSideRefractionIndex / positiveSideRefractionIndex;
	float cosI = this->Dot(normal);
	float sinT2 = n*n*(1.f - cosI*cosI);
	if (sinT2 > 1.f) // Total internal reflection occurs?
		return (-*this).Reflect(normal);
	return n * *this - (n + Sqrt(1.f - sinT2)) * normal;
}

float2 float2::ProjectTo(const float2 &direction) const
{
	assume(!direction.IsZero());
	return direction * this->Dot(direction) / direction.LengthSq();
}

float2 float2::ProjectToNorm(const float2 &direction) const
{
	assume(direction.IsNormalized());
	return direction * this->Dot(direction);
}

float float2::AngleBetween(const float2 &other) const
{
	return acos(Dot(other)) / Sqrt(LengthSq() * other.LengthSq());
}

float float2::AngleBetweenNorm(const float2 &other) const
{
	assume(this->IsNormalized());
	assume(other.IsNormalized());
	return acos(Dot(other));
}

float2 float2::Lerp(const float2 &b, float t) const
{
	assume(0.f <= t && t <= 1.f);
	return (1.f - t) * *this + t * b;
}

float2 float2::Lerp(const float2 &a, const float2 &b, float t)
{
	return a.Lerp(b, t);
}

void float2::Decompose(const float2 &direction, float2 &outParallel, float2 &outPerpendicular) const
{
	assume(direction.IsNormalized());
	outParallel = this->Dot(direction) * direction;
	outPerpendicular = *this - outParallel;
}

void float2::Orthogonalize(const float2 &a, float2 &b)
{
	assume(!a.IsZero());
	b -= a.Dot(b) / a.Length() * a;
}

bool float2::AreOrthogonal(const float2 &a, const float2 &b, float epsilon)
{
	return a.IsPerpendicular(b, epsilon);
}


void float2::Orthonormalize(float2 &a, float2 &b)
{
	assume(!a.IsZero());
	a.Normalize();
	b -= a.Dot(b) * a;
}

float2 float2::FromScalar(float scalar)
{
	return float2(scalar, scalar);
}

void float2::SetFromScalar(float scalar)
{
	x = scalar;
	y = scalar;
}

void float2::Set(float x_, float y_)
{
	x = x_;
	y = y_;
}

void float2::Rotate90CW()
{
	float oldX = x;
	x = y;
	y = -oldX;
}

float2 float2::Rotated90CW() const
{
	return float2(y, -x);
}

void float2::Rotate90CCW()
{
	float oldX = x;
	x = -y;
	y = oldX;
}

float2 float2::Rotated90CCW() const
{
	return float2(-y, x);
}

bool float2::OrientedCCW(const float2 &a, const float2 &b, const float2 &c)
{
	// Compute the determinant
	// | ax ay 1 |
	// | bx by 1 |
	// | cx cy 1 |
	// See Christer Ericson, Real-Time Collision Detection, p.32.
	return (a.x-c.x)*(b.y-c.y) - (a.y-c.y)*(b.x-c.x) >= 0.f;
}

class SortByPolarAngle
{
public:
	float2 perspective;

	bool operator()(const float2 &a, const float2 &b) const
	{
		float2 A = a - perspective;
		float2 B = b - perspective;
		return B.x*A.y < A.x*B.y;
	}
};

#ifdef MATH_ENABLE_STL_SUPPORT
void float2::ConvexHull(const float2 *pointArray, int numPoints, std::vector<float2> &outConvexHull)
{
	outConvexHull.clear();
	if (numPoints == 0)
		return;
	outConvexHull.insert(outConvexHull.end(), pointArray, pointArray + numPoints);
	int convexHullSize = ConvexHullInPlace(&outConvexHull[0], (int)outConvexHull.size());
	outConvexHull.resize(convexHullSize);
}
#endif

#ifdef MATH_ENABLE_STL_SUPPORT
/** This function implements the Graham's Scan algorithm for finding the convex hull of
	a 2D point set. The running time is O(nlogn). For details, see
	"Introduction to Algorithms, 2nd ed.", by Cormen, Leiserson, Rivest, p.824, or
	a lecture by Shai Simonson: http://www.aduni.org/courses/algorithms/index.php?view=cw , lecture 02-13-01. */
int float2::ConvexHullInPlace(float2 *p, int n)
{
	if (n <= 2)
		return n;

	if (n >= 50)
	{
		/* Perform Akl�Toussaint heuristic. The limit n=50 is arbitrary and based on quick profiling:
		 Without heuristic:
		   n=10: 1143 ticks
		   n=50: 8657 ticks
		   n=100: 19533 ticks
		 With heuristic:
		   n=10: 1322 ticks
		   n=50: 6759 ticks
		   n=100: 14448 ticks
		*/
		int minX = 0, minY = 0, maxX = 0, maxY = 0;
		for(int i = 1; i < n; ++i)
		{
			if (p[i].x < p[minX].x) minX = i;
			else if (p[i].x > p[maxX].x) maxX = i;
			if (p[i].y < p[minY].y) minY = i;
			else if (p[i].y > p[maxY].y) maxY = i;
		}
		// Direction vectors which point inside the convex hull.
		float2 e0 = (p[maxX] - p[minY]).Rotated90CCW();
		float2 e1 = (p[maxY] - p[maxX]).Rotated90CCW();
		float2 e2 = (p[minX] - p[maxY]).Rotated90CCW();
		float2 e3 = (p[minY] - p[minX]).Rotated90CCW();

		// Add a small epsilon so that the four extreme points on the convex hull will not get pruned
		// due to floating point imprecision.
		const float eps = 1e-6f;
		float e0_d = e0.Dot(p[minY]) + eps;
		float e1_d = e1.Dot(p[maxX]) + eps;
		float e2_d = e2.Dot(p[maxY]) + eps;
		float e3_d = e3.Dot(p[minX]) + eps;

		for(int i = 0; i < n; ++i)
			if (e0.Dot(p[i]) > e0_d && e1.Dot(p[i]) > e1_d && e2.Dot(p[i]) > e2_d && e3.Dot(p[i]) > e3_d)
				Swap(p[i--], p[--n]);
	}

	// Find the lowest point of the set.
	SortByPolarAngle pred;
	pred.perspective = p[0];
	int smallestY = 0;
	for(int i = 1; i < n; ++i)
		if (p[i].y < pred.perspective.y || (p[i].y == pred.perspective.y && p[i].x < pred.perspective.x))
		{
			pred.perspective = p[i];
			smallestY = i;
		}
	Swap(p[0], p[smallestY]);

	// For robustness, remove duplicates of the perspective pivot points.
	// This is because duplicates on that element will cause the sorting to be nontransitive and break
	// the whole sort.
	int d = 0;
	for(int i = 1; i < n; ++i)
		if (!p[i].Equals(p[0]))
			p[++d] = p[i];
	n = d+1;

	std::sort(&p[1], &p[n], pred);

	// For robustness, remove duplicate input values.
	d = 0;
	for(int i = 1; i < n; ++i)
		if (!p[i].Equals(p[d]))
			p[++d] = p[i];
	n = d+1;

	int h = 1; // Points to the index of the last point added to the hull so far. The first two points are in the hull to start.

	float2 a = p[h] - p[h-1];
	const float epsilon = 1e-5f;
	for(int i = 2; i < n; ++i)
	{
		// The last two added points determine a line, check which side of that line the next point to be added lies in.
		float2 d = p[i] - p[h-1];
		float dir = d.x*a.y - d.y*a.x;
		// Remove previous points from the convex hull until we have a left turn. Also for numerical stability,
		// in the case of three collinear points, remove the middle point.
		while(dir > epsilon || (dir > -epsilon && d.Dot(d) >= a.Dot(a)))
		{
			--h;
			if (h >= 1)
			{
				a = p[h] - p[h-1];
				d = p[i] - p[h-1];
				dir = d.x*a.y - d.y*a.x;
			}
			else
				break;
		}
		p[++h] = p[i];
		a = p[i] - p[h-1];
	}

	// Return the number of points on the new hull.
	return h+1;
}

bool float2::ConvexHullContains(const float2 *convexHull, int numPointsInConvexHull, const float2 &point)
{
	int j = numPointsInConvexHull-1;
	for(int i = 0; i < numPointsInConvexHull; ++i)
	{
		float2 d = (convexHull[i] - convexHull[j]).Rotated90CCW(); // Points inwards the convex hull.
		float2 n = point - convexHull[j];
		if (n.IsZero()) return true;
		if (n.Dot(d) < 0.f)
			return false;
		j = i;
	}
	return true;
}

#endif

#define NEXT_P(ptr) ((ptr)+1 < (pEnd) ? (ptr)+1 : (p))

float float2::MinAreaRectInPlace(float2 *p, int n, float2 &center, float2 &uDir, float2 &vDir, float &minU, float &maxU, float &minV, float &maxV)
{
	assume(p || n == 0);
	if (!p)
		return 0.f;
	float minArea = FLT_MAX;

	n = float2::ConvexHullInPlace(p, n);

	// Compute the initial AABB rectangle for the rotating calipers method.
	// Order the initial vertices minX -> minY -> maxX -> maxY to establish
	// a counter-clockwise orientation.
	float2 *e[4] = { p, p, p, p };
	for(int i = 1; i < n; ++i)
	{
		if (p[i].x < e[0]->x) e[0] = &p[i];
		else if (p[i].x > e[2]->x) e[2] = &p[i];
		if (p[i].y < e[1]->y) e[1] = &p[i];
		else if (p[i].y > e[3]->y) e[3] = &p[i];
	}

	// Direction vector of the edge that the currently tested rectangle is in contact with.
	float2 ed = -float2::unitY;

	// Starting guess for minimum area rectangle (AABB).
	minArea = (e[2]->x - e[0]->x) * (e[3]->y - e[1]->y);

	const float2 *pEnd = p + n;

	// These track directions the convex hull is pointing towards at each antipodal point.
	float2 d[4];
	d[0] = (*NEXT_P(e[0]) - *e[0]).Normalized();
	d[1] = (*NEXT_P(e[1]) - *e[1]).Normalized();
	d[2] = (*NEXT_P(e[2]) - *e[2]).Normalized();
	d[3] = (*NEXT_P(e[3]) - *e[3]).Normalized();

	// Rotate the calipers through each edge in the convex hull in order.
	for(int i = 0; i < n; ++i)
	{
		// Compute how much each edge will rotate before hitting the next vertex in the convex hull.
		float cosA0 =  ed.Dot(d[0]);
		float cosA1 =  ed.PerpDot(d[1]);
		float cosA2 = -ed.Dot(d[2]);
		float cosA3 = -ed.PerpDot(d[3]);

		float maxCos = MATH_NS::Max(MATH_NS::Max(cosA0, cosA1), MATH_NS::Max(cosA2, cosA3));
		// Pick the smallest angle (largest cosine of that angle) and increment the antipodal point index to travel the edge.
		if (cosA0 >= maxCos)      { ed = d[0];                e[0] = NEXT_P(e[0]); d[0] = (*NEXT_P(e[0]) - *e[0]).Normalized(); }
		else if (cosA1 >= maxCos) { ed = d[1].Rotated90CW();  e[1] = NEXT_P(e[1]); d[1] = (*NEXT_P(e[1]) - *e[1]).Normalized(); }
		else if (cosA2 >= maxCos) { ed = -d[2];               e[2] = NEXT_P(e[2]); d[2] = (*NEXT_P(e[2]) - *e[2]).Normalized(); }
		else                      { ed = d[3].Rotated90CCW(); e[3] = NEXT_P(e[3]); d[3] = (*NEXT_P(e[3]) - *e[3]).Normalized(); }

		// Check if the area of the new rectangle is smaller than anything seen so far.
		float minu = ed.PerpDot(*e[0]);
		float maxu = ed.PerpDot(*e[2]);
		float minv = ed.Dot(*e[1]);
		float maxv = ed.Dot(*e[3]);
		float area = MATH_NS::Abs(maxu-minu) * MATH_NS::Abs(maxv-minv);
		if (area < minArea)
		{
			vDir = ed;
			minArea = area;
			minU = MATH_NS::Min(minu, maxu);
			maxU = MATH_NS::Max(minu, maxu);
			minV = MATH_NS::Min(minv, maxv);
			maxV = MATH_NS::Max(minv, maxv);
		}
	}
	uDir = vDir.Rotated90CCW();
	center = 0.5f * (uDir * (minU+maxU) + vDir * (minV+maxV));

	return minArea;
}

float2 float2::RandomDir(LCG &lcg, float r)
{
	assume(r > 1e-3f);
	for(int i = 0; i < 1000; ++i)
	{
		float x = lcg.Float(-r, r);
		float y = lcg.Float(-r, r);
		float lenSq = x*x + y*y;
		if (lenSq >= 1e-6f && lenSq <= r*r)
			return r / Sqrt(lenSq) * float2(x,y);
	}
	assume(false && "Failed to generate a random float2 direction vector!");
	return float2(r, 0);
}

MUST_USE_RESULT float2 float2::RandomBox(LCG &lcg, float minElem, float maxElem)
{
	float x = lcg.Float(minElem, maxElem);
	float y = lcg.Float(minElem, maxElem);
	return float2(x, y);
}

float2 float2::operator +(const float2 &rhs) const
{
	return float2(x + rhs.x, y + rhs.y);
}

float2 float2::operator -(const float2 &rhs) const
{
	return float2(x - rhs.x, y - rhs.y);
}

float2 float2::operator -() const
{
	return float2(-x, -y);
}

float2 float2::operator *(float scalar) const
{
	return float2(x * scalar, y * scalar);
}

float2 operator *(float scalar, const float2 &rhs)
{
	return float2(scalar * rhs.x, scalar * rhs.y);
}

float2 float2::operator /(float scalar) const
{
	float invScalar = 1.f / scalar;
	return float2(x * invScalar, y * invScalar);
}

float2 &float2::operator +=(const float2 &rhs)
{
	x += rhs.x;
	y += rhs.y;

	return *this;
}

float2 &float2::operator -=(const float2 &rhs)
{
	x -= rhs.x;
	y -= rhs.y;

	return *this;
}

float2 &float2::operator *=(float scalar)
{
	x *= scalar;
	y *= scalar;

	return *this;
}

float2 float2::Add(float s) const
{
	return float2(x + s, y + s);
}

float2 float2::Sub(float s) const
{
	return float2(x - s, y - s);
}

float2 float2::SubLeft(float s) const
{
	return float2(s - x, s - y);
}

float2 float2::DivLeft(float s) const
{
	return float2(s / x, s / y);
}

float2 float2::Mul(const float2 &rhs) const
{
	return float2(x * rhs.x, y * rhs.y);
}

float2 float2::Div(const float2 &rhs) const
{
	return float2(x / rhs.x, y / rhs.y);
}

float2 &float2::operator /=(float scalar)
{
	float invScalar = 1.f / scalar;
	x *= invScalar;
	y *= invScalar;

	return *this;
}

#ifdef MATH_ENABLE_STL_SUPPORT
std::ostream &operator <<(std::ostream &out, const float2 &rhs)
{
	std::string str = rhs.ToString();
	out << str;
	return out;
}
#endif

const float2 float2::zero = float2(0, 0);
const float2 float2::one = float2(1, 1);
const float2 float2::unitX = float2(1, 0);
const float2 float2::unitY = float2(0, 1);
const float2 float2::nan = float2(FLOAT_NAN, FLOAT_NAN);
const float2 float2::inf = float2(FLOAT_INF, FLOAT_INF);

MATH_END_NAMESPACE
