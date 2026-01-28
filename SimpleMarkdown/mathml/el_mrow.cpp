#include "el_mrow.h"  
#include "document.h"  

namespace litehtml
{

    el_mrow::el_mrow(const std::shared_ptr<document>& doc) : el_math(doc)
    {

    }

    void el_mrow::parse_attributes()
    {
        // https://www.w3.org/TR/MathML3/chapter3.html#presm.mrow





        // Call parent implementation to handle standard attributes    
        el_math::parse_attributes();
    }

} // namespace litehtml