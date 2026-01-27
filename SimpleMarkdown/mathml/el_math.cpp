#include "el_math.h"  
#include "document.h"  

namespace litehtml
{
    
    el_math::el_math(const std::shared_ptr<document>& doc) : html_tag(doc)
    {
        // Set default display for math elements  
        m_css.set_display(display_inline_block);
    }

    void el_math::parse_attributes()
    {
        // https://www.w3.org/TR/MathML3/chapter2.html#interf.toplevel
        // Handle common math attributes if needed  
        const char* str = get_attr("display");
        if (str)
        {
            // Map display attribute to CSS display property  
            if (strcmp(str, "block") == 0)
            {
                m_style.add_property(_display_, "block");
            }
            else if (strcmp(str, "inline") == 0)
            {
                m_style.add_property(_display_, "inline");
            }
            else if (strcmp(str, "inline-block") == 0)
            {
                m_style.add_property(_display_, "inline-block");
            }
        }


        // Call parent implementation to handle standard attributes  
        html_tag::parse_attributes();
    }

} // namespace litehtml