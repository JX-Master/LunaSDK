/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MathTest.cpp
* @author JXMaster
* @date 2022/2/16
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Math/Matrix.hpp>

namespace Luna
{
	void math_test()
	{
		//TEST(VectorTest, Vector_Add)
		{
			Float2 a(1, 2);
			Float2 b(3, 4);

			Float2 c = a + b;

			lutest(c == Float2(4, 6));

			Float3 a1(1, 2, 3);
			Float3 b1(3, 4, 5);

			Float3 c1 = a1 + b1;
			lutest(c1 == Float3(4, 6, 8));

			Float4 a2(1, 2, 3, 4);
			Float4 b2(3, 4, 6, 8);

			Float4 c2 = a2 + b2;
			lutest(c2 == Float4(4, 6, 9, 12));
		}

		//TEST(VectorTest, Vector_Sub)
		{
			Float2 a(1, 2);
			Float2 b(3, 4);

			Float2 c = a - b;

			lutest(c == Float2(-2, -2));

			Float3 a1(1, 2, 3);
			Float3 b1(3, 4, 5);

			Float3 c1 = a1 - b1;
			lutest(c1 == Float3(-2, -2, -2));

			Float4 a2(1, 2, 3, 4);
			Float4 b2(3, 4, 6, 8);

			Float4 c2 = a2 - b2;
			lutest(c2 == Float4(-2, -2, -3, -4));
		}
		//TEST(VectorTest, Vector_Multiply_Scalar)
		{
			Float2 a(1, 2);
			Float3 a1(1, 2, 3);
			Float4 a2(1, 2, 3, 4);

			lutest(a * 3 == Float2(3, 6));
			lutest(a1 * 3 == Float3(3, 6, 9));
			lutest(a2 * 3 == Float4(3, 6, 9, 12));

			lutest(3 * a == Float2(3, 6));
			lutest(3 * a1 == Float3(3, 6, 9));
			lutest(3 * a2 == Float4(3, 6, 9, 12));
		}
		//TEST(VectorTest, DotProduct)
		{
			Float2 a(1, 2);
			Float2 b(3, 4);

			lutest(dot(a, b) == 11);

			Float3 a1(1, 2, 3);
			Float3 b1(3, 4, 5);

			lutest(dot(a1, b1) == 26);

			Float4 a2(1, 2, 3, 4);
			Float4 b2(3, 4, 6, 8);

			lutest(dot(a2, b2) == 61);
		}
		//TEST(VectorTest, CrossProduct)
		{
			Float3 a1(1, 2, 3);
			Float3 b1(3, 4, 5);
			lutest(cross(a1, b1) == Float3(-2, 4, -2));
		}
		//TEST(VectorTest, Length)
		{
			Float2 a(3.0f, 4.0f);

			luasset_eq_float(length(a), 5.0f);

			Float3 a1(3.0f, 4.0f, 5.0f);

			luasset_eq_float(length(a1), sqrt(50.0f));

			Float4 a2(1, 2, 3, 4);

			luasset_eq_float(length(a2), sqrt(30.0f));
		}
		//TEST(VectorTest, Normalize)
		{
			Float2 a(3.0f, 4.0f);
			Float2 b = normalize(a);
			luasset_eq_float(length(b), 1.0f);

			Float3 a1(3.0f, 4.0f, 5.0f);
			Float3 b1 = normalize(a1);
			luasset_eq_float(length(b1), 1.0f);

			Float4 a2(1.0f, 2.0f, 3.0f, 4.0f);
			Float4 b2 = normalize(a2);
			luasset_eq_float(length(b2), 1.0f);
		}
		//TEST(VectorTest, Lerp)
		{
			Float2 a(1.0f, 2.0f);
			Float2 b(3.0f, 4.0f);
			Float2 c = lerp(a, b, 0.5f);
			luasset_eq_float(c.x, 2.0f);
			luasset_eq_float(c.y, 3.0f);

			Float3 a1(1, 2, 3);
			Float3 b1(3, 4, 5);
			Float3 c1 = lerp(a1, b1, 0.4f);
			luasset_eq_float(c1.x, 1.8f);
			luasset_eq_float(c1.y, 2.8f);
			luasset_eq_float(c1.z, 3.8f);

			Float4 a2(1, 2, 3, 4);
			Float4 b2(3, 4, 6, 8);
			Float4 c2 = lerp(a2, b2, 0.7f);
			luasset_eq_float(c2.x, 2.4f);
			luasset_eq_float(c2.y, 3.4f);
			luasset_eq_float(c2.z, 5.1f);
			luasset_eq_float(c2.w, 6.8f);
		}
		//TEST(MatrixTest, Multiply)
		{
			Float3x3 a{ 2,4,7,
				3,1,9,
				6,2,8 };
			Float3x3 b{ 1,3,9,
				4,6,12,
				4,1,6 };
			lutest(mul(a, b) == Float3x3(
				46, 37, 108,
				43, 24, 93,
				46, 38, 126
			));

			Float4x4 c{
				45,13,23,4,
				2,31,1,34,
				1,3,4,1,
				412,5,74,56
			};
			Float4x4 d{
				5,4,1,5,
				63,2,5,87,
				41,5,6,5,
				2,3,3,4,
			};
			lutest(mul(c, d) == Float4x4(
				1995, 333, 260, 1487,
				2072, 177, 265, 2848,
				360, 33, 43, 290,
				5521, 2196, 1049, 3089
			));
		}
		//TEST(MatrixTest, Transpose)
		{
			Float3x3 a{
				2,4,7,
				3,1,9,
				6,2,8 };
			Float3x3 b = transpose(a);
			lutest(b == Float3x3(
				2, 3, 6,
				4, 1, 2,
				7, 9, 8));

			Float4x4 c{
				45,13,23,4,
				2,31,1,34,
				1,3,4,1,
				412,5,74,56
			};
			Float4x4 d = transpose(c);
			lutest(d == Float4x4(
				45, 2, 1, 412,
				13, 31, 3, 5,
				23, 1, 4, 74,
				4, 34, 1, 56
			));
		}
	}
}