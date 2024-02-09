#pragma once

#include <cfloat>

namespace PLANS {

	class Maths {
		public:
			// Absolute of given float. 
			static float abs(float f) {
				return f < 0.0F ? -f : f;
			}

			static bool compareFloat(float a, float b, float epsilon = FLT_MIN) {
				return Maths::abs(a - b) < epsilon;
			}

			template<typename T>
			static T max(T a, T b) {
				return a >= b ? a : b;
			}

			template<typename T>
			static T min(T a, T b) {
				return a < b ? a : b;
			}
			template<typename T>
			static T clamp(T value, T minimum, T maximum) {
				return max(minimum, min(maximum, value));
			}
		protected:
		private:
	};

}
