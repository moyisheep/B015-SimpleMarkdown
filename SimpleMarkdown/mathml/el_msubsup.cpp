#include "el_msubsup.h"  
#include "document.h"  

namespace litehtml
{

    el_msubsup::el_msubsup(const std::shared_ptr<document>& doc) : el_math(doc)
    {

    }

    void el_msubsup::parse_attributes()
    {


        // https://www.w3.org/TR/MathML3/chapter3.html#presm.msubsup
        const char* subscriptshift = get_attr("subscriptshift");
        if(subscriptshift)
        {

        }

        // https://www.w3.org/TR/MathML3/chapter3.html#presm.msubsup
        const char* superscriptshift = get_attr("superscriptshift");
        if (superscriptshift)
        {

        }

        // Call parent implementation to handle standard MathML attributes      
        el_math::parse_attributes();
    }

} // namespace litehtml