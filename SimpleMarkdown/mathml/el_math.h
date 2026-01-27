#ifndef LH_EL_MATH_H
#define LH_EL_MATH_H

#include "html_tag.h"

namespace litehtml
{
	class el_math : public html_tag
	{
	public:
		explicit el_math(const std::shared_ptr<litehtml::document>& doc);

		void parse_attributes() override;
	};
}

#endif  // LH_EL_MATH_H