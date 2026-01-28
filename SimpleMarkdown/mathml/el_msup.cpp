#include "el_msup.h"  
#include "document.h"  

namespace litehtml
{

    el_msup::el_msup(const std::shared_ptr<document>& doc) : el_math(doc)
    {

    }

    void el_msup::parse_attributes()
    {


        // https://www.w3.org/TR/MathML3/chapter3.html#presm.msup
        const char* superscriptshift = get_attr("superscriptshift");
        if (superscriptshift)
        {

        }

        // Call parent implementation to handle standard MathML attributes      
        el_math::parse_attributes();
    }

} // namespace litehtml