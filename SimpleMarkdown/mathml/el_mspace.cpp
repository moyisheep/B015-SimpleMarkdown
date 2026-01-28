#include "el_mspace.h"  
#include "document.h"  

namespace litehtml
{
    // https://www.w3.org/TR/MathML3/chapter3.html#presm.mn
    el_mspace::el_mspace(const std::shared_ptr<document>& doc) : el_math_token(doc)
    {

    }

    void el_mspace::parse_attributes()
    {

        // https://www.w3.org/TR/MathML3/chapter3.html#presm.mspace
        const char* width = get_attr("width");
        if(width)
        {
            map_to_dimension_property_ignoring_zero(_width_, width);
        }

        // https://www.w3.org/TR/MathML3/chapter3.html#presm.mspace
        const char* height = get_attr("height");
        if (height)
        {
            map_to_dimension_property_ignoring_zero(_height_, height);
        }

        // https://www.w3.org/TR/MathML3/chapter3.html#presm.mspace
        const char* depth = get_attr("depth");
        if (depth)
        {
            // TODO
        }

        // https://www.w3.org/TR/MathML3/chapter3.html#presm.mspace
        const char* linebreak = get_attr("linebreak");
        if (linebreak)
        {
            // TODO
        }

        // Call parent implementation to handle standard MathML attributes      
        el_math_token::parse_attributes();
    }

} // namespace litehtml