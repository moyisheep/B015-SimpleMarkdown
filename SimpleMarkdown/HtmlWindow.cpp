#include "HtmlWindow.h"
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/font.h>
#include <wx/filesys.h>
#include <wx/uri.h>
#include <wx/dataobj.h>
#include <wx/clipbrd.h>
#include <wx/dcbuffer.h>



#include <algorithm>
#include <litehtml/el_text.h>
#include <litehtml/render_item.h>
#include <litehtml/utf8_strings.h>

#include "HtmlDumper.h"
#include "debug.h"
#include "Timer.h"
#include <filesystem>

namespace fs = std::filesystem;



HtmlWindow::HtmlWindow(wxWindow* parent)
    : wxScrolled<wxPanel>(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_totalHeight = 0;
    m_scrollPos = 0;


    m_container = std::make_unique<wxContainer>(this);
    m_vfs = std::make_shared< LocalVFS>();
    m_container->set_vfs(m_vfs);

    m_link_ctrl = std::make_unique<wxStaticText>(this, wxID_ANY, "");
    m_link_ctrl->Hide();
    
    Bind(wxEVT_PAINT, &HtmlWindow::OnPaint, this);
    Bind(wxEVT_SCROLLWIN_BOTTOM, &HtmlWindow::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_LINEDOWN, &HtmlWindow::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_LINEUP, &HtmlWindow::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_PAGEDOWN, &HtmlWindow::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_PAGEUP, &HtmlWindow::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_THUMBRELEASE, &HtmlWindow::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_THUMBTRACK, &HtmlWindow::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_TOP, &HtmlWindow::OnScroll, this);
    Bind(wxEVT_MOUSEWHEEL, &HtmlWindow::OnMouseWheel, this);
    Bind(wxEVT_DROP_FILES, &HtmlWindow::OnDropFiles, this);
    Bind(wxEVT_LEFT_UP, &HtmlWindow::OnLeftUp, this);
    Bind(wxEVT_LEFT_DOWN, &HtmlWindow::OnLeftDown, this);
    Bind(wxEVT_MOTION, &HtmlWindow::OnMouseMove, this);
    Bind(wxEVT_LEAVE_WINDOW, &HtmlWindow::OnMouseLeave, this);
    Bind(wxEVT_KEY_DOWN, &HtmlWindow::OnKeyDown, this);
    Bind(wxEVT_SIZE, &HtmlWindow::OnSize, this);
}
HtmlWindow::~HtmlWindow()
{
    if (m_doc)
    {
        m_doc.reset();
    }
}

void HtmlWindow::set_html(const std::string& html)
{
    TimerOutput::Instance().start("[createFromString]");
    clear();
    m_html = std::string(html);
    m_doc = litehtml::document::createFromString({ html.c_str() , litehtml::encoding::utf_8}, 
        m_container.get(), litehtml::master_css, m_user_css);

    TimerOutput::Instance().end();
    
    TimerOutput::Instance().start("[render]");
    if (m_doc)
    {
        int width = GetClientSize().GetWidth();
        m_doc->render(width);
        record_char_boxes();
        SetupScrollbars(); 
    }
    TimerOutput::Instance().end();
    //debug::print_render_tree(m_doc->root_render());
    //debug::print_element_tree(m_doc->root());
    Refresh();


}
void HtmlWindow::record_char_boxes()
{
    // store all character's position, [type] std::vector<litehtml::position>
    m_char_boxes.clear();
    
    // store all character, [type] std::u32string
    m_plain_text.clear();
    record_char_boxes_recursive(m_doc->root());

}
void HtmlWindow::record_char_boxes_recursive(litehtml::element::ptr el)
{
    for(auto child: el->children())
    {
        // if it's el_text node, start record
        if (child->is_text() )
        {
            std::string txt = "";
            auto pos = child->get_placement();
            child->get_text(txt);
            std::u32string u32txt = (const char32_t*)litehtml::utf8_to_utf32(txt);
            std::u32string ch;
            auto hfont = child->parent()->css().get_font();
            float x = pos.left();
            
            // split every word -> character, and record
            for(auto c : u32txt)
            {
                ch += c;
                auto ch_width = m_container->text_width(litehtml::utf32_to_utf8(ch), hfont);

                litehtml::position char_pos{ x, pos.y, ch_width, pos.height };
                m_char_boxes.push_back(char_pos);
                x += ch_width;

                m_plain_text += ch;
                ch.clear();
            }
          
        }
        else
        {
            record_char_boxes_recursive(child);
        }
    }
}

bool HtmlWindow::load_html(const std::string &file_path)
{
    if (!m_vfs) { return false; }
    
    auto bin = m_vfs->get_binary(file_path);
    if(bin.empty()){ return false; }

    std::string html = std::string(reinterpret_cast<char*> (bin.data()), bin.size());
    
    if(!html.empty())
    {
        fs::path filePath = fs::path(file_path);
        fs::path parentDir = filePath.parent_path();
        if (fs::exists(parentDir))
        {
            fs::current_path(parentDir);
        }
        this->set_html(html);
        return true;
    }


    return false;

}

bool HtmlWindow::set_user_css(const std::string& css)
{
    if(!css.empty())
    {
        m_user_css = css;
        return true;
    }
    return false;
}

bool HtmlWindow::load_user_css(const std::string& path)
{
    if (m_vfs) 
    { 
        auto bin = m_vfs->get_binary(path);
        if (!bin.empty()) 
        { 
            m_user_css = std::string(reinterpret_cast<char*> (bin.data()), bin.size());
            return true;
        }
    }

 
    return false;
}

void HtmlWindow::clear()
{
    ClearSelection();
    m_scrollPos = 0;
    m_cursor_pos = -1;
    m_char_boxes.clear();
    m_plain_text.clear();
    m_totalHeight = 0;
    m_container->clear();
}

void HtmlWindow::set_vfs(std::shared_ptr<VirtualFileSystem>& vfs)
{
    if(vfs)
    {
        m_vfs = vfs;
        if(m_container)
        {
            m_container->set_vfs(vfs);
        }
    }
}

std::string HtmlWindow::get_html()
{
    return m_html;
}




void HtmlWindow::SetupScrollbars()
{
    if (!m_doc) return;

    int clientHeight = GetClientSize().GetHeight();
    m_totalHeight = m_doc->height();

    SetVirtualSize(-1, m_totalHeight);
    //SetScrollRate(0, 20);

    int scrollRange = m_totalHeight - clientHeight;
    if (scrollRange > 0) {
        EnableScrolling(false, true);
        SetScrollbars(0, 20, 0, scrollRange, 0, m_scrollPos);
    }
    else {
        EnableScrolling(false, false);
    }
}

void HtmlWindow::ScrollToPosition(int pos)
{
   //.auto delta = pos - m_scrollPos;

    //auto sz = GetClientSize();


    m_scrollPos = pos;
    SetScrollPos(wxVERTICAL, pos);
    //UpdateSelectionRect();
    //Refresh();

    //if (std::abs(delta) < sz.GetHeight())
    //{
 
    //    auto bitmap = wxBitmap(m_back_bitmap);
    //    auto memdc = wxMemoryDC();
    //    memdc.SelectObject(m_back_bitmap);
    //    memdc.DrawBitmap(wxNullBitmap, wxPoint(0, 0));
    //    memdc.SelectObject(wxNullBitmap);
    //    m_scrolling = true;
    //    if (delta > 0)
    //    {

    //        RefreshRect(wxRect{ 0, sz.GetHeight()- delta, m_back_bitmap.GetWidth(), delta });
    //    }
    //    else
    //    {

    //        RefreshRect(wxRect{ 0,  -delta, m_back_bitmap.GetWidth(), -delta });
    //    }
    //}
    //else
    {
        Refresh();
    }
}

int HtmlWindow::GetScrollPosition() const
{
    return m_scrollPos;
}



void HtmlWindow::EnableDragAndDrop(bool enable)
{
    if (enable)
    {
        DragAcceptFiles(true);
    }
    else
    {
        DragAcceptFiles(false);
    }
}

void HtmlWindow::ShowLinkWindow(std::string link)
{
    if (link.empty() || link == m_hover_link) { return; }
    m_hover_link = link;
    m_link_ctrl->SetLabelText(m_hover_link);
    auto sz = m_link_ctrl->GetBestSize();
    m_link_ctrl->SetSize(sz);

    auto csz = GetClientSize();
    wxPoint pt{ 0, csz.GetHeight() - sz.GetHeight() };
    m_link_ctrl->SetPosition(pt);
    m_link_ctrl->Show();
}

void HtmlWindow::HideLinkWindow()
{
    m_hover_link = "";
    m_link_ctrl->Hide();
}



void HtmlWindow::OnDropFiles(wxDropFilesEvent& event)
{
    if (event.GetNumberOfFiles() > 0)
    {
        wxString* dropped = event.GetFiles();
        wxString file_path = dropped[0];


        if (!load_html(file_path.ToStdString()))
        {
            wxMessageBox("Failed to load HTML file", "Error", wxICON_ERROR);
        }

    }
}




// 请求重绘指定区域
void HtmlWindow::RequestRedraw(const litehtml::position::vector& redraw_boxes)
{
    for (const auto& rect : redraw_boxes)
    {
        // 将 litehtml::position 转换为 wxRect
        wxRect wxRect(rect.x, rect.y - m_scrollPos, rect.width, rect.height);


        // 刷新区域
        RefreshRect(wxRect);
    }
}


// =============事件处理======================

void HtmlWindow::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    wxGraphicsContext* gc = wxGraphicsContext::Create(dc);

    if(gc && m_doc)
    {
        // get update rect
        wxRegion updateRegion = GetUpdateRegion();
        wxRect updateRect = updateRegion.GetBox();
     



        TimerOutput::Instance().start("[draw]");

        // clear background
        gc->SetBrush(*wxWHITE_BRUSH);
        gc->DrawRectangle(wxRect2DDouble(updateRect));

        // clip rectangle
        litehtml::position clip{ (float)updateRect.x,
            (float)updateRect.y,
            (float)updateRect.width,
            (float)updateRect.height };

        // draw 
        litehtml::uint_ptr hdc = (litehtml::uint_ptr)gc;
        if(hdc)
        {
            m_doc->draw(hdc, 0, -m_scrollPos, &clip);
        }
 

        // draw selection
        DrawSelection(gc, updateRect);

        // draw merged text
        m_container->draw_finished(hdc, clip);

        //DrawCaret(&memdc);
        TimerOutput::Instance().end();





        // draw update rect
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->SetPen(*wxRED_PEN);
        gc->DrawRectangle(wxRect2DDouble(updateRect));
    }

    delete gc;
 






}

void HtmlWindow::DrawCaret(wxDC* dc)
{
    // draw cursor
    if (m_cursor_pos >= 0 && m_cursor_pos < m_char_boxes.size())
    {
        auto pos = m_char_boxes[m_cursor_pos];
        pos.y = pos.y - m_scrollPos;
        dc->SetBrush(*wxTRANSPARENT_BRUSH);
        dc->SetPen(*wxBLACK_PEN);
        wxPoint pt1(pos.right(), pos.top());
        wxPoint pt2(pos.right(), pos.bottom());
        dc->DrawLine(pt1, pt2);
    }
}
void HtmlWindow::DrawSelection(wxGraphicsContext* dc, wxRect updateRect)
{

    // draw selection 
    if (!m_selection_rect.empty())
    {
        
        dc->SetBrush(wxBrush(wxColor(128, 128, 128, 255)));
        dc->SetPen(*wxTRANSPARENT_PEN);
        for (auto& rect : m_selection_rect.get_rect())
        {
            rect.y = rect.y - m_scrollPos;
            if (updateRect.Intersects(rect))
            {
                dc->DrawRectangle(wxRect2DDouble(rect));
            }

        }
    }
}
void HtmlWindow::OnScroll(wxScrollWinEvent& event)
{
    int newPos = event.GetPosition();
    //std::string txt = "OnScroll: " + std::to_string(newPos);
    //wxLogInfo(txt);
    //int newPos = GetScrollPos(wxVERTICAL);
    if (newPos != m_scrollPos)
    {
        ScrollToPosition(newPos);
    }
    //event.Skip();
}

void HtmlWindow::OnMouseWheel(wxMouseEvent& event)
{
    int rotation = event.GetWheelRotation();
    int delta = event.GetWheelDelta();
    int lines = rotation / delta;

    int newPos = m_scrollPos - lines * 20; // 20是滚动步长
    newPos = wxMax(0, wxMin(newPos, m_totalHeight - GetClientSize().GetHeight()));
    //std::string txt = "OnMouseWheel: " + std::to_string(newPos);
    //wxLogInfo(txt);
    if (newPos != m_scrollPos)
    {
        ScrollToPosition(newPos);
    }
}

void HtmlWindow::OnSize(wxSizeEvent& event)
{
    if (m_doc)
    {
        
        auto sz = GetClientSize();
        int width = sz.GetWidth();
        int height = sz.GetHeight();
        if(width > 0 && height > 0)
        {
            m_doc->render(width);
            m_back_bitmap = wxBitmap(width, height);
            record_char_boxes();
            m_doc->media_changed();
            SetupScrollbars();
            Refresh();
        }

    }
    event.Skip();
}


void HtmlWindow::ClearSelection()
{
    m_selection = false;
    m_selection_start = -1;
    m_selection_end = -1;
    m_selection_text = "";
    m_selection_rect.clear();
    

}

void HtmlWindow::OnLeftDown(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();

    float x = pt.x;
    float y = pt.y + m_scrollPos;

    // 调用文档的鼠标按下事件
    litehtml::position::vector redraw_boxes;

    if (m_doc && m_doc->on_lbutton_down(x, y, 0, 0, redraw_boxes))
    {
        // 文档处理了事件，可能触发了重绘
        if (!redraw_boxes.empty())
        {
            // 请求重绘受影响区域
            RequestRedraw(redraw_boxes);
        }
    }

    if(m_doc)
    {

        auto redraw_boxes = m_selection_rect.get_raw_rect();
        ClearSelection();
        if(!redraw_boxes.empty())
        {
            RequestRedraw(redraw_boxes);
        }


        // test whether point inside, returen [m_char_boxes] index
        auto pos = hit_test(x, y);

        if(pos >= 0)
        {
           
            m_selection_start = pos;
            m_selection_end = pos;
        }

        m_selection = true;

    }

    if(m_doc)
    {

        auto index = hit_test(x, y);
        if(index >= 0)
        {

            m_cursor_pos = index;
            auto pos = m_char_boxes[index];
            litehtml::position::vector redraw_boxes;
            redraw_boxes.push_back(pos);
            RequestRedraw(redraw_boxes);
        }
    }

}

void HtmlWindow::OnMouseMove(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    float x = pt.x;
    float y = pt.y + m_scrollPos;

    if (m_doc)
    {
        litehtml::position::vector redraw_boxes;
        m_doc->on_mouse_over(x, y, 0, 0, redraw_boxes);


        if (!redraw_boxes.empty())
        {
            RequestRedraw(redraw_boxes);
        }
    }
    if (m_doc && m_selection)
    {
        if(m_selection_start < 0)
        {
            auto pos = hit_test(x, y);
            if (pos >= 0) 
            { 
                m_selection_start = pos;
                m_selection_end = pos;
            }
        }
        if(m_selection_end > 0)
        {
            auto pos = hit_test(x, y);
            if (pos >= 0)
            {
                m_cursor_pos = pos;
                m_selection_end = pos;
                UpdateSelectionRect();
                RequestRedraw(m_selection_rect.get_raw_rect());
            }
        }

 

   
    }
    if(m_doc)
    {
        auto link = m_container->get_hover_link();
        if (link.empty()) { HideLinkWindow(); }
        ShowLinkWindow(link);
    }

}

void HtmlWindow::OnLeftUp(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    float x = pt.x;
    float y = pt.y + m_scrollPos;

    // 调用文档的鼠标释放事件
    litehtml::position::vector redraw_boxes;

    if (m_doc && m_doc->on_lbutton_up(x, y, 0, 0, redraw_boxes))
    {
        if (!redraw_boxes.empty())
        {
            RequestRedraw(redraw_boxes);
        }
    }

    if (m_doc && m_selection)
    {
        if (m_selection_end >= 0)
        {
            auto pos = hit_test(x, y);
            if (pos >= 0)
            {
                m_selection_end = pos;
                UpdateSelectionRect();
                RequestRedraw(m_selection_rect.get_raw_rect());
            }
        }
     

    }
    m_selection = false;
}

void HtmlWindow::UpdateSelectionRect()
{
    if (m_selection_start < 0 || 
        m_selection_end < 0 ||
        m_selection_start == m_selection_end || 
        m_char_boxes.empty()) 
    { 
        return;
    }
    m_selection_rect.clear();
    m_selection_text.clear();

    // set start to min value, end to max value, and clamp to [0, m_char_boxes.size()]
    int32_t start = std::clamp((int32_t)std::min(m_selection_start, m_selection_end), 0, (int32_t)(m_char_boxes.size() - 1));
    int32_t end = std::clamp((int32_t)std::max(m_selection_start, m_selection_end), 0, (int32_t)(m_char_boxes.size() - 1));

    // adding character postion to drawing rect
    for(int i = start; i <= end; i++)
    {
        m_selection_rect.add(m_char_boxes[i]);
    }

    // process scroll offset
    //m_selection_rect.scroll(-m_scrollPos);

    // this is get selection text, and convert to utf-8
    if(!m_plain_text.empty() && 
        start < m_plain_text.size() && 
        end < m_plain_text.size())
    {
        m_selection_text = litehtml::utf32_to_utf8(m_plain_text.substr(start, end - start + 1));
    }
    //std::string selected_text;
    //selected_text += "[original] " + std::to_string(m_selection_start) + ", " + std::to_string(m_selection_end) + "\n";

    //selected_text += "[position] " + std::to_string(start) + ", " + std::to_string(end) + "\n";
    //
    //selected_text += m_selection_text + "\n";
    //wxLogInfo(wxString::FromUTF8(selected_text));
    
}
void HtmlWindow::OnMouseLeave(wxMouseEvent& event)
{
   
    if (m_doc)
    {
        litehtml::position::vector redraw_boxes;
        if (m_doc->on_mouse_leave(redraw_boxes))
        {
            if (!redraw_boxes.empty())
            {
                RequestRedraw(redraw_boxes);
            }
        }
    }

    //event.Skip();
}


bool HtmlWindow::CopyToClipboard(const wxString& text)
{
    if (wxTheClipboard->Open())
    {
        
        wxTheClipboard->Clear();

     
        wxTheClipboard->SetData(new wxTextDataObject(text));

    
        wxTheClipboard->Close();

        return true;
    }
    return false;
}
void HtmlWindow::OnKeyDown(wxKeyEvent& event)
{

    if (event.GetKeyCode() == WXK_ESCAPE)
    {
        if (m_doc)
        {
            litehtml::position::vector redraw_boxes;
            if (m_doc->on_button_cancel(redraw_boxes))
            {
                if (!redraw_boxes.empty())
                {
                    RequestRedraw(redraw_boxes);
                }
            }
        }

    }
    else if (event.GetKeyCode() == 'A' && event.ControlDown())
    {

    }
    else if (event.GetKeyCode() == 'C' && event.ControlDown())
    {
        if(!m_selection_text.empty())
        {
            CopyToClipboard(wxString::FromUTF8(m_selection_text));
        }
    }
    else if(event.GetKeyCode() == WXK_LEFT)
    {
        m_cursor_pos = std::max(0, m_cursor_pos - 1);
        Refresh();
    }
    else if (event.GetKeyCode() == WXK_RIGHT)
    {
        m_cursor_pos = std::clamp( m_cursor_pos + 1, 0, (int32_t)m_char_boxes.size());
        Refresh();
    }

    event.Skip();
}

int32_t HtmlWindow::hit_test(float x, float y)
{
    int32_t index = 0;
    for(auto& pos: m_char_boxes)
    {
      
        if (pos.is_point_inside(x, y))
        {
            return index;
        }
        index += 1;
    }
    return -1;
}

