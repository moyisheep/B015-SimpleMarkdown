#include "el_mtext.h"  
#include "document.h"  

namespace litehtml
{
    // https://www.w3.org/TR/MathML3/chapter3.html#presm.mn
    el_mtext::el_mtext(const std::shared_ptr<document>& doc) : el_math_token(doc)
    {

    }

    void el_mtext::parse_attributes()
    {





        // Call parent implementation to handle standard MathML attributes      
        el_math_token::parse_attributes();
    }

} // namespace litehtml