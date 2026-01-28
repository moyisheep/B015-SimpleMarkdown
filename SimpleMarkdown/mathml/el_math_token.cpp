#include "el_math_token.h"  
#include "document.h"  

namespace litehtml
{

    el_math_token::el_math_token(const std::shared_ptr<document>& doc) : el_math(doc)
    {

    }

    void el_math_token::parse_attributes()
    {


        // https://www.w3.org/TR/MathML3/chapter3.html#presm.commatt
        const char* mathvariant = get_attr("mathvariant");
        if (mathvariant) {
            // Explicit mathvariant attribute - map to CSS font properties  
            if (strcmp(mathvariant, "normal") == 0) {
                m_style.add_property(_font_style_, "normal");
                m_style.add_property(_font_weight_, "normal");
            }
            else if (strcmp(mathvariant, "bold") == 0) {
                m_style.add_property(_font_weight_, "bold");
            }
            else if (strcmp(mathvariant, "italic") == 0) {
                m_style.add_property(_font_style_, "italic");
            }
            else if (strcmp(mathvariant, "bold-italic") == 0) {
                m_style.add_property(_font_weight_, "bold");
                m_style.add_property(_font_style_, "italic");
            }
            // TODO: Add support for other mathvariant values (double-struck, script, etc.)  
        }




        // https://www.w3.org/TR/MathML3/chapter3.html#presm.commatt
        const char* mathsize = get_attr("mathsize");
        if (mathsize) {
            m_style.add_property(_font_size_, mathsize);
        }


        // https://www.w3.org/TR/MathML3/chapter3.html#presm.commatt
        //const char* dir = get_attr("dir");
        //if (dir) {
        //    m_style.add_property(_direction_, dir);
        //}

        // Call parent implementation to handle standard MathML attributes      
        el_math::parse_attributes();
    }

} // namespace litehtml