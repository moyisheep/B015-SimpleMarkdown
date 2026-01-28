#include "el_mi.h"  
#include "document.h"  

namespace litehtml
{

    el_mi::el_mi(const std::shared_ptr<document>& doc) : el_math_token(doc)
    {

    }

    void el_mi::parse_attributes()
    {
        // Get the text content to determine default mathvariant  
        string content;
        get_text(content);

        // https://www.w3.org/TR/MathML3/chapter3.html#presm.mi

        // Default behavior: italic for single character, normal for multi-character  
        if (content.length() == 1) {
            m_style.add_property(_font_style_, "italic");
        }
        else {
            m_style.add_property(_font_style_, "normal");
        }
     


        el_math_token::parse_attributes();
    }

} // namespace litehtml