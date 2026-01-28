#ifndef LH_EL_MN_H
#define LH_EL_MN_H

#include "el_math_token.h"

namespace litehtml
{
	class el_mn : public el_math_token
	{
	public:
		explicit el_mn(const std::shared_ptr<litehtml::document>& doc);

		void parse_attributes() override;
	};
}

#endif  // LH_EL_MN_H