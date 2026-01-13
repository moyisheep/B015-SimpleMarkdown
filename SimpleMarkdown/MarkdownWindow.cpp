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
    m_ctrl = std::make_unique<wxTextCtrl>(this, wxID_ANY);
    m_ctrl->Hide();
    SetFocus();
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

void MarkdownWindow::OnKeyDown(wxKeyEvent& event)
{

    if (event.GetKeyCode() == WXK_RETURN && event.ControlDown())
    {
        
        auto pos = m_char_boxes[m_cursor_pos];
        pos.y = pos.y - m_scrollPos;
        auto sz = GetClientSize();
        m_ctrl->SetPosition(wxPoint(0, pos.y));
        m_ctrl->SetSize(wxSize(sz.GetWidth(), pos.height));
        m_ctrl->SetLabelText("hello world");
        m_ctrl->Show();
    }

    event.Skip();
}
wxBEGIN_EVENT_TABLE(MarkdownWindow, HtmlWindow)
   EVT_DROP_FILES(MarkdownWindow::OnDropFiles)
    EVT_KEY_DOWN(MarkdownWindow::OnKeyDown)
wxEND_EVENT_TABLE()