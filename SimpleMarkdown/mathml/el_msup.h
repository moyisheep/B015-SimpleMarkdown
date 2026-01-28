#ifndef LH_EL_MSUP_H
#define LH_EL_MSUP_H

#include "el_math.h"

namespace litehtml
{
	class el_msup : public el_math
	{
	public:
		explicit el_msup(const std::shared_ptr<litehtml::document>& doc);

		void parse_attributes() override;
	};
}

#endif  // LH_EL_MSUP_H