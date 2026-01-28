#include "el_ms.h"  
#include "document.h"  

namespace litehtml
{
    // https://www.w3.org/TR/MathML3/chapter3.html#presm.mn
    el_ms::el_ms(const std::shared_ptr<document>& doc) : el_math_token(doc)
    {

    }

    void el_ms::parse_attributes()
    {

        const char* str = get_attr("lquote");
        if(str)
        {
            // TODO
        }

        str = get_attr("rquote");
        if(str)
        {
            // TODO
        }

        // Call parent implementation to handle standard MathML attributes      
        el_math_token::parse_attributes();
    }

} // namespace litehtml