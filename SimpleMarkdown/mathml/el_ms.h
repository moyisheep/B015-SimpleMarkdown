#ifndef LH_EL_MS_H
#define LH_EL_MS_H

#include "el_math_token.h"

namespace litehtml
{
	class el_ms : public el_math_token
	{
	public:
		explicit el_ms(const std::shared_ptr<litehtml::document>& doc);

		void parse_attributes() override;
	};
}

#endif  // LH_EL_MS_H