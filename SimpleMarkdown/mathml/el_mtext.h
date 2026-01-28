#ifndef LH_EL_MTEXT_H
#define LH_EL_MTEXT_H

#include "el_math_token.h"

namespace litehtml
{
	class el_mtext : public el_math_token
	{
	public:
		explicit el_mtext(const std::shared_ptr<litehtml::document>& doc);

		void parse_attributes() override;
	};
}

#endif  // LH_EL_MTEXT_H