#pragma once

#include <cstdint>

namespace PLANS {

	class HTTPHelper {
		public:
			static void postKeepAlive();
			static void postLastEpisodeFinished(uint32_t episodeCount);
		protected:
		private:
	};

}