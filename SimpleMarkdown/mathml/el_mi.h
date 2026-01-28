#ifndef LH_EL_MI_H
#define LH_EL_MI_H

#include "el_math_token.h"

namespace litehtml
{
	class el_mi : public el_math_token
	{
	public:
		explicit el_mi(const std::shared_ptr<litehtml::document>& doc);

		void parse_attributes() override;
	};
}

#endif  // LH_EL_MI_H