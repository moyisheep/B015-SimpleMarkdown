#include "el_msub.h"  
#include "document.h"  

namespace litehtml
{

    el_msub::el_msub(const std::shared_ptr<document>& doc) : el_math(doc)
    {

    }

    void el_msub::parse_attributes()
    {


        // https://www.w3.org/TR/MathML3/chapter3.html#presm.msub
        const char* subscriptshift = get_attr("subscriptshift");
        if (subscriptshift)
        {

        }


        // Call parent implementation to handle standard MathML attributes      
        el_math::parse_attributes();
    }

} // namespace litehtml