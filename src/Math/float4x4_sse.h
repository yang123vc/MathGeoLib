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

/** @file float4x4_sse.h
	@author Jukka Jyl�nki
	@brief SSE code for float4x4-related computations. */
#pragma once

#include "../MathBuildConfig.h"

#ifdef MATH_SSE

#include "SSEMath.h"
#include "float4_sse.h"

/// Compute the product M*v, where M is a 4x4 matrix denoted by an array of 4 __m128's, and v is a 4x1 vector.
#ifdef MATH_SSE41 // If we have SSE 4.1, we can use the dpps (dot product) instruction, _mm_dp_ps intrinsic.
inline __m128 mat4x4_mul_sse41(const __m128 *matrix, __m128 vector)
{
	__m128 x = _mm_dp_ps(matrix[0], vector, 0xF0 | 0x0F); // Choose to multiply x, y, z and w (0xF0 = 1111 0000), and store the output to all indices (0x0F == 0000 1111).
	__m128 y = _mm_dp_ps(matrix[1], vector, 0xF0 | 0x0F);
	__m128 z = _mm_dp_ps(matrix[2], vector, 0xF0 | 0x0F);
	__m128 w = _mm_dp_ps(matrix[3], vector, 0xF0 | 0x0F);

	__m128 xy = _mm_movelh_ps(x, y); // xy = [ _, y, _, x]
	__m128 zw = _mm_movelh_ps(z, w); // zw = [ _, w, _, z]

	return _mm_shuffle_ps(xy, zw, _MM_SHUFFLE(2, 0, 2, 0)); // ret = [w, z, y, x]
}
#endif

/// Compute the product M*v, where M is a 4x4 matrix denoted by an array of 4 __m128's, and v is a 4x1 vector.
#ifdef MATH_SSE3 // If we have SSE3, we can repeatedly use haddps to accumulate the result.
inline __m128 mat4x4_mul_sse3(const __m128 *matrix, __m128 vector)
{
	__m128 x = _mm_mul_ps(matrix[0], vector);
	__m128 y = _mm_mul_ps(matrix[1], vector);
	__m128 z = _mm_mul_ps(matrix[2], vector);
	__m128 w = _mm_mul_ps(matrix[3], vector);
	__m128 tmp1 = _mm_hadd_ps(x, y); // = [y2+y3, y0+y1, x2+x3, x0+x1]
	__m128 tmp2 = _mm_hadd_ps(z, w); // = [w2+w3, w0+w1, z2+z3, z0+z1]

	return _mm_hadd_ps(tmp1, tmp2); // = [w0+w1+w2+w3, z0+z1+z2+z3, y0+y1+y2+y3, x0+x1+x2+x3]
}
#endif

inline __m128 mat4x4_mul_sse1(const __m128 *matrix, __m128 vector)
{
	__m128 x = _mm_mul_ps(matrix[0], vector);
	__m128 y = _mm_mul_ps(matrix[1], vector);
	__m128 z = _mm_mul_ps(matrix[2], vector);
	__m128 w = _mm_mul_ps(matrix[3], vector);
	_MM_TRANSPOSE4_PS(x, y, z, w); // Contains 2x unpacklo's, 2x unpackhi's, 2x movelh's and 2x movehl's. (or 8 shuffles, depending on the compiler)

	return _mm_add_ps(_mm_add_ps(x, y), _mm_add_ps(z, w));
}

inline __m128 colmajor_mat4x4_mul_sse1(const __m128 *matrix, __m128 vector)
{
	__m128 x = _mm_shuffle_ps(vector, vector, _MM_SHUFFLE(0,0,0,0));
	__m128 y = _mm_shuffle_ps(vector, vector, _MM_SHUFFLE(1,1,1,1));
	__m128 z = _mm_shuffle_ps(vector, vector, _MM_SHUFFLE(2,2,2,2));
	__m128 w = _mm_shuffle_ps(vector, vector, _MM_SHUFFLE(3,3,3,3));
	x = _mm_mul_ps(x, matrix[0]);
	y = _mm_mul_ps(y, matrix[1]);
	z = _mm_mul_ps(z, matrix[2]);
	w = _mm_mul_ps(w, matrix[3]);

	return _mm_add_ps(_mm_add_ps(x, y), _mm_add_ps(z, w));
}

inline __m128 colmajor_mat4x4_mul_sse1_2(const __m128 *matrix, __m128 vector)
{
	__m128 x = shuffle1_ps(vector, _MM_SHUFFLE(0,0,0,0));
	__m128 y = shuffle1_ps(vector, _MM_SHUFFLE(1,1,1,1));
	__m128 z = shuffle1_ps(vector, _MM_SHUFFLE(2,2,2,2));
	__m128 w = shuffle1_ps(vector, _MM_SHUFFLE(3,3,3,3));
	x = _mm_mul_ps(x, matrix[0]);
	y = _mm_mul_ps(y, matrix[1]);
	z = _mm_mul_ps(z, matrix[2]);
	w = _mm_mul_ps(w, matrix[3]);

	return _mm_add_ps(_mm_add_ps(x, y), _mm_add_ps(z, w));
}

/// Compute the product M*v, where M is a 4x4 matrix denoted by an array of 4 __m128's, and v is a 4x1 vector.
inline __m128 mat4x4_mul_sse(const __m128 *matrix, __m128 vector)
{
#ifdef MATH_SSE41
	return mat4x4_mul_sse41(matrix, vector);
#elif defined(MATH_SSE3)
	return mat4x4_mul_sse3(matrix, vector);
#else
	return mat4x4_mul_sse1(matrix, vector);
#endif
}

/// Compute the product M*v, where M is a 3x4 matrix denoted by an array of 4 __m128's, and v is a 4x1 vector.
inline __m128 mat3x4_mul_sse(const __m128 *matrix, __m128 vector)
{
	__m128 x = dot4_ps(matrix[0], vector);
	__m128 y = dot4_ps(matrix[1], vector);
	__m128 z = dot4_ps(matrix[2], vector);

	// Take the 'w' component of the vector unmodified.
	__m128 xy = _mm_movelh_ps(x, y); // xy = [ _, y, _, x]
	__m128 zw = _mm_movehl_ps(vector, z); // zw = [ w, _, z, _]
	return _mm_shuffle_ps(xy, zw, _MM_SHUFFLE(3, 1, 2, 0)); // ret = [w, z, y, x]
}

inline float3 mat3x4_mul_vec(const __m128 *matrix, __m128 vector)
{
	__m128 x = dot4_ps(matrix[0], vector);
	__m128 y = dot4_ps(matrix[1], vector);
	__m128 z = dot4_ps(matrix[2], vector);

	return float3(M128_TO_FLOAT(x), M128_TO_FLOAT(y), M128_TO_FLOAT(z));
}

#define _mm_transpose_matrix_intel(row0, row1, row2, row3) \
	__m128 tmp0, tmp1, tmp2, tmp3; \
	tmp0 = _mm_unpacklo_ps(row0, row1); \
	tmp2 = _mm_unpacklo_ps(row2, row3); \
	tmp1 = _mm_unpackhi_ps(row0, row1); \
	tmp3 = _mm_unpackhi_ps(row2, row3); \
	row0 = _mm_movelh_ps(tmp0, tmp2); \
	row1 = _mm_movehl_ps(tmp2, tmp0); \
	row2 = _mm_movelh_ps(tmp1, tmp3); \
	row3 = _mm_movehl_ps(tmp3, tmp1);

FORCE_INLINE void mat4x4_mul_dpps(__m128 *out, const __m128 *m1, const __m128 *m2)
{
	// Transpose m2:
	// m2[0] = [ 03, 02, 01, 00 ]     [ 30, 20, 10, 00 ]
	// m2[1] = [ 13, 12, 11, 10 ] --> [ 31, 21, 11, 01 ]
	// m2[2] = [ 23, 22, 21, 20 ] --> [ 32, 22, 12, 02 ]
	//         [ 33, 32, 31, 30 ]     [ 33, 23, 13, 03 ]

	__m128 low1 = _mm_movelh_ps(m2[0], m2[1]); // = [ 11, 10, 01, 00 ]
	__m128 low2 = _mm_movelh_ps(m2[2], m2[3]); // = [ 31, 30, 21, 20 ]
	__m128 hi1 = _mm_movehl_ps(m2[1], m2[0]);  // = [ 13, 12, 03, 02 ]
	__m128 hi2 = _mm_movehl_ps(m2[3], m2[2]);  // = [ 33, 32, 23, 22 ]

	__m128 row1 = _mm_shuffle_ps(low1, low2, _MM_SHUFFLE(2, 0, 2, 0)); // = [30, 20, 10, 00]
	__m128 row2 = _mm_shuffle_ps(low1, low2, _MM_SHUFFLE(3, 1, 3, 1)); // = [31, 21, 11, 01]
	__m128 row3 = _mm_shuffle_ps(hi1, hi2, _MM_SHUFFLE(2, 0, 2, 0));   // = [32, 22, 12, 02]
	__m128 row4 = _mm_shuffle_ps(hi1, hi2, _MM_SHUFFLE(3, 1, 3, 1));   // = [33, 23, 13, 03]

	__m128 _00 = dot4_ps(m1[0], row1);
	__m128 _01 = dot4_ps(m1[0], row2);
	__m128 _02 = dot4_ps(m1[0], row3);
	__m128 _03 = dot4_ps(m1[0], row4);
	out[0] = _mm_pack_4ss_to_ps(_00, _01, _02, _03);

	__m128 _10 = dot4_ps(m1[1], row1);
	__m128 _11 = dot4_ps(m1[1], row2);
	__m128 _12 = dot4_ps(m1[1], row3);
	__m128 _13 = dot4_ps(m1[1], row4);
	out[1] = _mm_pack_4ss_to_ps(_10, _11, _12, _13);

	__m128 _20 = dot4_ps(m1[2], row1);
	__m128 _21 = dot4_ps(m1[2], row2);
	__m128 _22 = dot4_ps(m1[2], row3);
	__m128 _23 = dot4_ps(m1[2], row4);
	out[2] = _mm_pack_4ss_to_ps(_20, _21, _22, _23);

	__m128 _30 = dot4_ps(m1[3], row1);
	__m128 _31 = dot4_ps(m1[3], row2);
	__m128 _32 = dot4_ps(m1[3], row3);
	__m128 _33 = dot4_ps(m1[3], row4);
	out[3] = _mm_pack_4ss_to_ps(_30, _31, _32, _33);
}

FORCE_INLINE void mat4x4_mul_dpps_2(__m128 *out, const __m128 *m1, const __m128 *m2)
{
	// Transpose m2:
	// m2[0] = [ 03, 02, 01, 00 ]     [ 30, 20, 10, 00 ]
	// m2[1] = [ 13, 12, 11, 10 ] --> [ 31, 21, 11, 01 ]
	// m2[2] = [ 23, 22, 21, 20 ] --> [ 32, 22, 12, 02 ]
	//         [ 33, 32, 31, 30 ]     [ 33, 23, 13, 03 ]
	__m128 row1 = m2[0];
	__m128 row2 = m2[1];
	__m128 row3 = m2[2];
	__m128 row4 = m2[3];
	_mm_transpose_matrix_intel(row1, row2, row3, row4);

	__m128 _00 = dot4_ps(m1[0], row1);
	__m128 _01 = dot4_ps(m1[0], row2);
	__m128 _02 = dot4_ps(m1[0], row3);
	__m128 _03 = dot4_ps(m1[0], row4);
	out[0] = _mm_pack_4ss_to_ps(_00, _01, _02, _03);

	__m128 _10 = dot4_ps(m1[1], row1);
	__m128 _11 = dot4_ps(m1[1], row2);
	__m128 _12 = dot4_ps(m1[1], row3);
	__m128 _13 = dot4_ps(m1[1], row4);
	out[1] = _mm_pack_4ss_to_ps(_10, _11, _12, _13);

	__m128 _20 = dot4_ps(m1[2], row1);
	__m128 _21 = dot4_ps(m1[2], row2);
	__m128 _22 = dot4_ps(m1[2], row3);
	__m128 _23 = dot4_ps(m1[2], row4);
	out[2] = _mm_pack_4ss_to_ps(_20, _21, _22, _23);

	__m128 _30 = dot4_ps(m1[3], row1);
	__m128 _31 = dot4_ps(m1[3], row2);
	__m128 _32 = dot4_ps(m1[3], row3);
	__m128 _33 = dot4_ps(m1[3], row4);
	out[3] = _mm_pack_4ss_to_ps(_30, _31, _32, _33);
}

FORCE_INLINE void mat4x4_mul_dpps_3(__m128 *out, const __m128 *m1, const __m128 *m2)
{
	// Transpose m2:
	// m2[0] = [ 03, 02, 01, 00 ]     [ 30, 20, 10, 00 ]
	// m2[1] = [ 13, 12, 11, 10 ] --> [ 31, 21, 11, 01 ]
	// m2[2] = [ 23, 22, 21, 20 ] --> [ 32, 22, 12, 02 ]
	//         [ 33, 32, 31, 30 ]     [ 33, 23, 13, 03 ]

	__m128 low1 = _mm_movelh_ps(m2[0], m2[1]); // = [ 11, 10, 01, 00 ]
	__m128 low2 = _mm_movelh_ps(m2[2], m2[3]); // = [ 31, 30, 21, 20 ]
	__m128 hi1 = _mm_movehl_ps(m2[1], m2[0]);  // = [ 13, 12, 03, 02 ]
	__m128 hi2 = _mm_movehl_ps(m2[3], m2[2]);  // = [ 33, 32, 23, 22 ]

	__m128 row1 = _mm_shuffle_ps(low1, low2, _MM_SHUFFLE(2, 0, 2, 0)); // = [30, 20, 10, 00]
	__m128 row2 = _mm_shuffle_ps(low1, low2, _MM_SHUFFLE(3, 1, 3, 1)); // = [31, 21, 11, 01]
	__m128 row3 = _mm_shuffle_ps(hi1, hi2, _MM_SHUFFLE(2, 0, 2, 0));   // = [32, 22, 12, 02]
	__m128 row4 = _mm_shuffle_ps(hi1, hi2, _MM_SHUFFLE(3, 1, 3, 1));   // = [33, 23, 13, 03]

	__m128 _00 = dot4_ps(m1[0], row1);
	__m128 _01 = dot4_ps(m1[0], row2);
	__m128 _02 = dot4_ps(m1[0], row3);
	__m128 _03 = dot4_ps(m1[0], row4);

	__m128 xy = _mm_movelh_ps(_00, _01); // xy = [ _, y, _, x]
	__m128 zw = _mm_movelh_ps(_02, _03); // zw = [ _, w, _, z]
	out[0] = _mm_shuffle_ps(xy, zw, _MM_SHUFFLE(2, 0, 2, 0)); // ret = [w, z, y, x]

//	out[0] = _mm_pack_4ss_to_ps(_00, _01, _02, _03);

	__m128 _10 = dot4_ps(m1[1], row1);
	__m128 _11 = dot4_ps(m1[1], row2);
	__m128 _12 = dot4_ps(m1[1], row3);
	__m128 _13 = dot4_ps(m1[1], row4);

	__m128 xy2 = _mm_movelh_ps(_10, _11); // xy = [ _, y, _, x]
	__m128 zw2 = _mm_movelh_ps(_12, _13); // zw = [ _, w, _, z]
	out[1] = _mm_shuffle_ps(xy2, zw2, _MM_SHUFFLE(2, 0, 2, 0)); // ret = [w, z, y, x]

//	out[1] = _mm_pack_4ss_to_ps(_10, _11, _12, _13);

	__m128 _20 = dot4_ps(m1[2], row1);
	__m128 _21 = dot4_ps(m1[2], row2);
	__m128 _22 = dot4_ps(m1[2], row3);
	__m128 _23 = dot4_ps(m1[2], row4);

	__m128 xy3 = _mm_movelh_ps(_20, _21); // xy = [ _, y, _, x]
	__m128 zw3 = _mm_movelh_ps(_22, _23); // zw = [ _, w, _, z]
	out[2] = _mm_shuffle_ps(xy3, zw3, _MM_SHUFFLE(2, 0, 2, 0)); // ret = [w, z, y, x]

//	out[2] = _mm_pack_4ss_to_ps(_20, _21, _22, _23);

	__m128 _30 = dot4_ps(m1[3], row1);
	__m128 _31 = dot4_ps(m1[3], row2);
	__m128 _32 = dot4_ps(m1[3], row3);
	__m128 _33 = dot4_ps(m1[3], row4);

	__m128 xy4 = _mm_movelh_ps(_30, _31); // xy = [ _, y, _, x]
	__m128 zw4 = _mm_movelh_ps(_32, _33); // zw = [ _, w, _, z]
	out[3] = _mm_shuffle_ps(xy4, zw4, _MM_SHUFFLE(2, 0, 2, 0)); // ret = [w, z, y, x]

//	out[3] = _mm_pack_4ss_to_ps(_30, _31, _32, _33);
}

FORCE_INLINE void mat4x4_mul_sse(__m128 *out, const __m128 *m1, const __m128 *m2)
{
	__m128 s0 = shuffle1_ps(m1[0], _MM_SHUFFLE(0,0,0,0));
	__m128 s1 = shuffle1_ps(m1[0], _MM_SHUFFLE(1,1,1,1));
	__m128 s2 = shuffle1_ps(m1[0], _MM_SHUFFLE(2,2,2,2));
	__m128 s3 = shuffle1_ps(m1[0], _MM_SHUFFLE(3,3,3,3));
	__m128 r0 = _mm_mul_ps(s0, m2[0]);
	__m128 r1 = _mm_mul_ps(s1, m2[1]);
	__m128 r2 = _mm_mul_ps(s2, m2[2]);
	__m128 r3 = _mm_mul_ps(s3, m2[3]);
	out[0] = _mm_add_ps(_mm_add_ps(r0, r1), _mm_add_ps(r2, r3));

	s0 = shuffle1_ps(m1[1], _MM_SHUFFLE(0,0,0,0));
	s1 = shuffle1_ps(m1[1], _MM_SHUFFLE(1,1,1,1));
	s2 = shuffle1_ps(m1[1], _MM_SHUFFLE(2,2,2,2));
	s3 = shuffle1_ps(m1[1], _MM_SHUFFLE(3,3,3,3));
	r0 = _mm_mul_ps(s0, m2[0]);
	r1 = _mm_mul_ps(s1, m2[1]);
	r2 = _mm_mul_ps(s2, m2[2]);
	r3 = _mm_mul_ps(s3, m2[3]);
	out[1] = _mm_add_ps(_mm_add_ps(r0, r1), _mm_add_ps(r2, r3));

	s0 = shuffle1_ps(m1[2], _MM_SHUFFLE(0,0,0,0));
	s1 = shuffle1_ps(m1[2], _MM_SHUFFLE(1,1,1,1));
	s2 = shuffle1_ps(m1[2], _MM_SHUFFLE(2,2,2,2));
	s3 = shuffle1_ps(m1[2], _MM_SHUFFLE(3,3,3,3));
	r0 = _mm_mul_ps(s0, m2[0]);
	r1 = _mm_mul_ps(s1, m2[1]);
	r2 = _mm_mul_ps(s2, m2[2]);
	r3 = _mm_mul_ps(s3, m2[3]);
	out[2] = _mm_add_ps(_mm_add_ps(r0, r1), _mm_add_ps(r2, r3));

	s0 = shuffle1_ps(m1[3], _MM_SHUFFLE(0,0,0,0));
	s1 = shuffle1_ps(m1[3], _MM_SHUFFLE(1,1,1,1));
	s2 = shuffle1_ps(m1[3], _MM_SHUFFLE(2,2,2,2));
	s3 = shuffle1_ps(m1[3], _MM_SHUFFLE(3,3,3,3));
	r0 = _mm_mul_ps(s0, m2[0]);
	r1 = _mm_mul_ps(s1, m2[1]);
	r2 = _mm_mul_ps(s2, m2[2]);
	r3 = _mm_mul_ps(s3, m2[3]);
	out[3] = _mm_add_ps(_mm_add_ps(r0, r1), _mm_add_ps(r2, r3));
}

FORCE_INLINE void mat4x4_mul_sse_2(__m128 *out, const __m128 *m1, const __m128 *m2)
{
	__m128 s0 = _mm_shuffle_ps(m1[0], m1[0], _MM_SHUFFLE(0,0,0,0));
	__m128 s1 = _mm_shuffle_ps(m1[0], m1[0], _MM_SHUFFLE(1,1,1,1));
	__m128 s2 = _mm_shuffle_ps(m1[0], m1[0], _MM_SHUFFLE(2,2,2,2));
	__m128 s3 = _mm_shuffle_ps(m1[0], m1[0], _MM_SHUFFLE(3,3,3,3));
	__m128 r0 = _mm_mul_ps(s0, m2[0]);
	__m128 r1 = _mm_mul_ps(s1, m2[1]);
	__m128 r2 = _mm_mul_ps(s2, m2[2]);
	__m128 r3 = _mm_mul_ps(s3, m2[3]);
	out[0] = _mm_add_ps(_mm_add_ps(r0, r1), _mm_add_ps(r2, r3));

	s0 = _mm_shuffle_ps(m1[1], m1[1], _MM_SHUFFLE(0,0,0,0));
	s1 = _mm_shuffle_ps(m1[1], m1[1], _MM_SHUFFLE(1,1,1,1));
	s2 = _mm_shuffle_ps(m1[1], m1[1], _MM_SHUFFLE(2,2,2,2));
	s3 = _mm_shuffle_ps(m1[1], m1[1], _MM_SHUFFLE(3,3,3,3));
	r0 = _mm_mul_ps(s0, m2[0]);
	r1 = _mm_mul_ps(s1, m2[1]);
	r2 = _mm_mul_ps(s2, m2[2]);
	r3 = _mm_mul_ps(s3, m2[3]);
	out[1] = _mm_add_ps(_mm_add_ps(r0, r1), _mm_add_ps(r2, r3));

	s0 = _mm_shuffle_ps(m1[2], m1[2], _MM_SHUFFLE(0,0,0,0));
	s1 = _mm_shuffle_ps(m1[2], m1[2], _MM_SHUFFLE(1,1,1,1));
	s2 = _mm_shuffle_ps(m1[2], m1[2], _MM_SHUFFLE(2,2,2,2));
	s3 = _mm_shuffle_ps(m1[2], m1[2], _MM_SHUFFLE(3,3,3,3));
	r0 = _mm_mul_ps(s0, m2[0]);
	r1 = _mm_mul_ps(s1, m2[1]);
	r2 = _mm_mul_ps(s2, m2[2]);
	r3 = _mm_mul_ps(s3, m2[3]);
	out[2] = _mm_add_ps(_mm_add_ps(r0, r1), _mm_add_ps(r2, r3));

	s0 = _mm_shuffle_ps(m1[3], m1[3], _MM_SHUFFLE(0,0,0,0));
	s1 = _mm_shuffle_ps(m1[3], m1[3], _MM_SHUFFLE(1,1,1,1));
	s2 = _mm_shuffle_ps(m1[3], m1[3], _MM_SHUFFLE(2,2,2,2));
	s3 = _mm_shuffle_ps(m1[3], m1[3], _MM_SHUFFLE(3,3,3,3));
	r0 = _mm_mul_ps(s0, m2[0]);
	r1 = _mm_mul_ps(s1, m2[1]);
	r2 = _mm_mul_ps(s2, m2[2]);
	r3 = _mm_mul_ps(s3, m2[3]);
	out[3] = _mm_add_ps(_mm_add_ps(r0, r1), _mm_add_ps(r2, r3));
}

inline void mat3x4_mul_sse(__m128 *out, const __m128 *m1, const __m128 *m2)
{
	const __m128 m2_3 = _mm_set_ps(1.f, 0.f, 0.f, 0.f);

	__m128 s0 = shuffle1_ps(m1[0], _MM_SHUFFLE(0,0,0,0));
	__m128 s1 = shuffle1_ps(m1[0], _MM_SHUFFLE(1,1,1,1));
	__m128 s2 = shuffle1_ps(m1[0], _MM_SHUFFLE(2,2,2,2));
	__m128 s3 = shuffle1_ps(m1[0], _MM_SHUFFLE(3,3,3,3));
	__m128 r0 = _mm_mul_ps(s0, m2[0]);
	__m128 r1 = _mm_mul_ps(s1, m2[1]);
	__m128 r2 = _mm_mul_ps(s2, m2[2]);
	__m128 r3 = _mm_mul_ps(s3, m2_3);
	out[0] = _mm_add_ps(_mm_add_ps(r0, r1), _mm_add_ps(r2, r3));

	s0 = shuffle1_ps(m1[1], _MM_SHUFFLE(0,0,0,0));
	s1 = shuffle1_ps(m1[1], _MM_SHUFFLE(1,1,1,1));
	s2 = shuffle1_ps(m1[1], _MM_SHUFFLE(2,2,2,2));
	s3 = shuffle1_ps(m1[1], _MM_SHUFFLE(3,3,3,3));
	r0 = _mm_mul_ps(s0, m2[0]);
	r1 = _mm_mul_ps(s1, m2[1]);
	r2 = _mm_mul_ps(s2, m2[2]);
	r3 = _mm_mul_ps(s3, m2_3);
	out[1] = _mm_add_ps(_mm_add_ps(r0, r1), _mm_add_ps(r2, r3));

	s0 = shuffle1_ps(m1[2], _MM_SHUFFLE(0,0,0,0));
	s1 = shuffle1_ps(m1[2], _MM_SHUFFLE(1,1,1,1));
	s2 = shuffle1_ps(m1[2], _MM_SHUFFLE(2,2,2,2));
	s3 = shuffle1_ps(m1[2], _MM_SHUFFLE(3,3,3,3));
	r0 = _mm_mul_ps(s0, m2[0]);
	r1 = _mm_mul_ps(s1, m2[1]);
	r2 = _mm_mul_ps(s2, m2[2]);
	r3 = _mm_mul_ps(s3, m2_3);
	out[2] = _mm_add_ps(_mm_add_ps(r0, r1), _mm_add_ps(r2, r3));
}

#endif // ~MATH_SSE
