#ifndef LH_EL_MATH_TOKEN_H
#define LH_EL_MATH_TOKEN_H

#include "el_math.h"

namespace litehtml
{
	class el_math_token : public el_math
	{
	public:
		explicit el_math_token(const std::shared_ptr<litehtml::document>& doc);

		void parse_attributes() override;
	};
}

#endif  // LH_EL_MATH_TOKEN_H