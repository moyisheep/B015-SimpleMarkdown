#include "MarkdownWindow.h"
#include <cmark-gfm.h>
#include <cmark-gfm-core-extensions.h>


std::string md_to_html(const std::string& markdown) {
    // Register all core extensions  
    cmark_gfm_core_extensions_ensure_registered();

    // Create parser with all options enabled  
    int options = CMARK_OPT_DEFAULT | CMARK_OPT_FOOTNOTES |
        CMARK_OPT_GITHUB_PRE_LANG | CMARK_OPT_HARDBREAKS |
        CMARK_OPT_SMART | CMARK_OPT_VALIDATE_UTF8;

    cmark_parser* parser = cmark_parser_new(options);

    // Attach all available extensions  
    const char* extension_names[] = { "autolink", "strikethrough", "table", "tagfilter", NULL };
    for (const char** it = extension_names; *it; ++it) {
        cmark_syntax_extension* syntax_extension = cmark_find_syntax_extension(*it);
        if (syntax_extension) {
            cmark_parser_attach_syntax_extension(parser, syntax_extension);
        }
    }

    // Parse and render  
    cmark_parser_feed(parser, markdown.c_str(), markdown.length());
    cmark_node* doc = cmark_parser_finish(parser);
    char* html = cmark_render_html(doc, options, NULL);

    // Convert to std::string and cleanup  
    std::string result(html);
    free(html);
    cmark_node_free(doc);
    cmark_parser_free(parser);

    return result;
}

MarkdownWindow::MarkdownWindow(wxWindow* parent)
    :HtmlWindow(parent)
{
    Bind(wxEVT_DROP_FILES, &MarkdownWindow::OnDropFiles, this);
}

MarkdownWindow::~MarkdownWindow()
{
}

bool MarkdownWindow::set_markdown(const std::string& md)
{
    if(!md.empty())
    {
        m_markdown_text = std::string(md);
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
                m_markdown_text = std::string(md);
                auto html = md_to_html(md);
                if(!html.empty())
                {
                    //wxLogMessage(wxString::FromUTF8(html));
                    set_html(html);
                    return true;
                }
            }
        }
    }




   

    return false;
}

std::string MarkdownWindow::get_markdown()
{
    return m_markdown_text;
}

void MarkdownWindow::OnDropFiles(wxDropFilesEvent& event)
{
    if (event.GetNumberOfFiles() > 0)
    {
        wxString* dropped = event.GetFiles();
        wxString file_path = dropped[0];


        if (!open_markdown(file_path.ToStdString()))
        {
            wxMessageBox("Failed to load Markdown file", "Error", wxICON_ERROR);
        }

    }
}


