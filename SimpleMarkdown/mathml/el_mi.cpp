#include "el_mi.h"  
#include "document.h"  

namespace litehtml
{

    el_mi::el_mi(const std::shared_ptr<document>& doc) : el_math(doc)
    {

    }

    void el_mi::parse_attributes()
    {
        // Get the text content to determine default mathvariant  
        string content;
        get_text(content);

        // https://www.w3.org/TR/MathML3/chapter3.html#presm.mi
        // Handle mathvariant attribute with special default behavior  
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
        else {
            // Default behavior: italic for single character, normal for multi-character  
            if (content.length() == 1) {
                m_style.add_property(_font_style_, "italic");
            }
            else {
                m_style.add_property(_font_style_, "normal");
            }
        }

        // Parse other MathML token element attributes  
        const char* mathcolor = get_attr("mathcolor");
        if (mathcolor) {
            m_style.add_property(_color_, mathcolor, "", false, get_document()->container());
        }

        const char* mathbackground = get_attr("mathbackground");
        if (mathbackground) {
            m_style.add_property(_background_color_, mathbackground, "", false, get_document()->container());
        }

        const char* mathsize = get_attr("mathsize");
        if (mathsize) {
            m_style.add_property(_font_size_, mathsize);
        }

        //const char* dir = get_attr("dir");
        //if (dir) {
        //    m_style.add_property(_direction_, dir);
        //}

        // Call parent implementation to handle standard MathML attributes      
        el_math::parse_attributes();
    }

} // namespace litehtml