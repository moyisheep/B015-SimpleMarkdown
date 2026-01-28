#ifndef LH_EL_MSPACE_H
#define LH_EL_MSPACE_H

#include "el_math_token.h"

namespace litehtml
{
	class el_mspace : public el_math_token
	{
	public:
		explicit el_mspace(const std::shared_ptr<litehtml::document>& doc);

		void parse_attributes() override;
	};
}

#endif  // LH_EL_MSPACE_H