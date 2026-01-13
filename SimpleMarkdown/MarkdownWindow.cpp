#include "MarkdownWindow.h"
#include <cmark-gfm.h>

std::string md_to_html(const std::string& markdown) {
    std::string html = cmark_markdown_to_html(
        markdown.c_str(),
        markdown.length(),
        CMARK_OPT_DEFAULT
    );


    return html;
}

MarkdownWindow::MarkdownWindow(wxWindow* parent)
    :HtmlWindow(parent)
{
}

MarkdownWindow::~MarkdownWindow()
{
}

bool MarkdownWindow::set_markdown(const std::string& md)
{
    if(!md.empty())
    {
        auto html = md_to_html(md);
        if (!html.empty())
        {
            set_html(html);
            return true;
        }
    }
    return false;
}

bool MarkdownWindow::open_markdown(const std::string& path)
{
    if (m_vfs) 
    { 
        auto bin = m_vfs->get_binary(path);
        if (!bin.empty()) 
        { 
            auto md = std::string(reinterpret_cast<char*> (bin.data()), bin.size());
            if(!md.empty())
            {
                auto html = md_to_html(md);
                if(!html.empty())
                {
                    set_html(html);
                    return true;
                }
            }
        }
    }




   

    return false;
}
