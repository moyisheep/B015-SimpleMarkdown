#include "el_math.h"  
#include "document.h"  

namespace litehtml
{
    
    el_math::el_math(const std::shared_ptr<document>& doc) : html_tag(doc)
    {
        // Set default display for math elements  
        css_w().set_display(display_inline_block);
    }

    void el_math::parse_attributes()
    {
        // https://www.w3.org/TR/MathML3/chapter2.html#interf.toplevel
        // Parse MathML display attribute (block vs inline)  
        const char* str = get_attr("display");
        if (str) {
            if (strcmp(str, "block") == 0) {
                css_w().set_display(display_block);
            }
            else {
                css_w().set_display(display_inline_block);
            }
        }

        // Parse MathML mode attribute (display vs inline math)  
        str = get_attr("maxwidth");
        if (str) {
            map_to_dimension_property_ignoring_zero(_max_width_, str);
        }


        // https://www.w3.org/TR/MathML3/chapter3.html#presm.presatt 
        const char* mathcolor = get_attr("mathcolor");
        if (mathcolor) {
            m_style.add_property(_color_, mathcolor, "", false, get_document()->container());
        }

        // https://www.w3.org/TR/MathML3/chapter3.html#presm.presatt
        const char* mathbackground = get_attr("mathbackground");
        if (mathbackground) {
            m_style.add_property(_background_color_, mathbackground, "", false, get_document()->container());
        }



        // Call parent implementation to handle standard attributes    
        html_tag::parse_attributes();
    }

} // namespace litehtml