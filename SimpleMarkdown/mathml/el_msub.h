#ifndef LH_EL_MSUB_H
#define LH_EL_MSUB_H

#include "el_math.h"

namespace litehtml
{
	class el_msub : public el_math
	{
	public:
		explicit el_msub(const std::shared_ptr<litehtml::document>& doc);

		void parse_attributes() override;
	};
}

#endif  // LH_EL_MSUB_H