#ifndef LH_EL_MROW_H
#define LH_EL_MROW_H

#include "el_math.h"

namespace litehtml
{
	class el_mrow : public el_math
	{
	public:
		explicit el_mrow(const std::shared_ptr<litehtml::document>& doc);

		void parse_attributes() override;
	};
}

#endif  // LH_EL_MROW_H