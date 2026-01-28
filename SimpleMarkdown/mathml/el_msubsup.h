#ifndef LH_EL_MSUBSUP_H
#define LH_EL_MSUBSUP_H

#include "el_math.h"

namespace litehtml
{
	class el_msubsup : public el_math
	{
	public:
		explicit el_msubsup(const std::shared_ptr<litehtml::document>& doc);

		void parse_attributes() override;
	};
}

#endif  // LH_EL_MSUBSUP_H