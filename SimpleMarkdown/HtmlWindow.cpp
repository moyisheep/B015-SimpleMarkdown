#include "HtmlWindow.h"
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/font.h>
#include <wx/filesys.h>
#include <wx/uri.h>
#include <wx/dataobj.h>
#include <wx/clipbrd.h>
#include <wx/graphics.h>
#include <wx/dcgraph.h>


#include <algorithm>
#include <litehtml/el_text.h>
#include <litehtml/render_item.h>
#include <litehtml/utf8_strings.h>
#include <cmark-gfm.h>
#include "HtmlDumper.h"

#include <filesystem>

namespace fs = std::filesystem;


std::string md_to_html(const std::string& markdown) {
    std::string html = cmark_markdown_to_html(
        markdown.c_str(),
        markdown.length(),
        CMARK_OPT_DEFAULT
    );

    
    return html;
}

HtmlWindow::HtmlWindow(wxWindow* parent)
    : wxScrolled<wxPanel>(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_totalHeight = 0;
    m_scrollPos = 0;

    //SetScrollRate(0, 20); // 设置垂直滚动步长
    m_container = std::make_unique<wxContainer>(this);
    m_vfs = std::make_shared< LocalVFS>();
    m_container->set_vfs(m_vfs);
    m_link_ctrl = std::make_unique<wxStaticText>(this, wxID_ANY, "");
    m_link_ctrl->Hide();
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


    m_doc = litehtml::document::createFromString({ html.c_str() , litehtml::encoding::utf_8}, 
        m_container.get(), litehtml::master_css, m_user_css);

    auto el = m_doc->root()->select_one(".instructions-list");

    if (el)
    {
        std::string text = "";
        el->get_text(text);
        std::string txt = std::string(el->get_tagName()) + ":" + text;
        wxLogInfo(txt);
        for (auto& child : el->children())
        {
            text.clear();
            child->get_text(text);
            txt = std::string(child->get_tagName()) + ":" + text;
            wxLogInfo(txt);
        }
        m_doc->append_children_from_string(*el, "<li><strong>Hello World</strong>what <span style=\"color:blue\">《你好》</span> </li>", false);



    }
    
    
    if (m_doc)
    {
        int width = GetClientSize().GetWidth();
        m_doc->render(width);
        record_char_boxes();
        SetupScrollbars(); // 添加这行
    }

    Refresh();
    //HtmlDumper dumper;
    //m_doc->dump(dumper);
    //OutputDebugStringA(dumper.get_html().c_str());

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

bool HtmlWindow::open_html(const std::string &file_path)
{
    if (!m_vfs) { return false; }
    
    auto bin = m_vfs->get_binary(file_path);
    if(bin.empty()){ return false; }

    std::string raw = std::string(reinterpret_cast<char*> (bin.data()), bin.size());
    

    fs::path filePath = fs::path(file_path);
    fs::path parentDir = filePath.parent_path();
    if(fs::exists(parentDir))
    {
        fs::current_path(parentDir);
    }
    


    std::string ext = filePath.extension().generic_string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    
    std::string html = "";
    if (ext == ".md")
    {

        html = md_to_html(raw);

    }
    else
    {
        html = raw;
    }
    
    // Set the HTML content

    this->set_html(html);
    return true;
}

void HtmlWindow::set_user_css(const std::string& css)
{
    m_user_css = css;
}

void HtmlWindow::load_user_css(const std::string& path)
{
    if (!m_vfs) { return; }

    auto bin = m_vfs->get_binary(path);
    if (bin.empty()) { return; }

    m_user_css  = std::string(reinterpret_cast<char*> (bin.data()), bin.size());

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
   
    m_scrollPos = pos;
    SetScrollPos(wxVERTICAL, pos);
    UpdateSelectionRect();
    Refresh();
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


        if (!open_html(file_path.ToStdString()))
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
    //DoPrepareDC(dc); // 处理滚动偏移

            // 获取需要重绘的区域
    wxRegion updateRegion = GetUpdateRegion();
    wxRect updateRect = updateRegion.GetBox();

 

    // 绘制背景
    dc.SetBrush(*wxWHITE_BRUSH);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(updateRect);



 
    if (m_doc)
    {
        // 绘制选择区域

        
        litehtml::position clip{ (float)updateRect.x, 
            (float)updateRect.y, 
            (float)updateRect.width, 
            (float)updateRect.height };
        litehtml::uint_ptr hdc = (litehtml::uint_ptr)&dc;
        m_doc->draw(hdc, 0, -m_scrollPos, &clip);
        
    }

    if (!m_selection_rect.empty())
    {

        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(*wxBLUE_PEN);
        for (auto& rect : m_selection_rect.get_rect())
        {
            dc.DrawRectangle(rect);
        }
    }
    if(m_cursor_pos >= 0 && m_cursor_pos < m_char_boxes.size())
    {
        auto pos = m_char_boxes[m_cursor_pos];
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(*wxBLACK_PEN);
        wxPoint pt1(pos.right(), pos.top());
        wxPoint pt2(pos.right(), pos.bottom());
        dc.DrawLine(pt1, pt2);
    }
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(*wxRED_PEN);
    dc.DrawRectangle(updateRect);
}


void HtmlWindow::OnScroll(wxScrollWinEvent& event)
{
    int newPos = event.GetPosition();
    std::string txt = "OnScroll: " + std::to_string(newPos);
    wxLogInfo(txt);
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
    std::string txt = "OnMouseWheel: " + std::to_string(newPos);
    wxLogInfo(txt);
    if (newPos != m_scrollPos)
    {
        ScrollToPosition(newPos);
    }
}

void HtmlWindow::OnSize(wxSizeEvent& event)
{
    if (m_doc)
    {
        int width = GetClientSize().GetWidth();
        m_doc->render(width);
        record_char_boxes();
        wxLogInfo(wxString::FromUTF8(litehtml::utf32_to_utf8(m_plain_text)));
        m_doc->media_changed();
        SetupScrollbars();
        Refresh();
    }
    event.Skip();
}

//litehtml::position HtmlWindow::GetSelectedRect(float x, float y)
//{
//    for (auto pos: m_char_pos)
//    {
//        if(pos.is_point_inside(x, y))
//        {
//            return pos;
//        }
//    }
//    return {};
//}
//Selection HtmlWindow::GetSelectionChar(float x, float y)
//{
//    auto root_render = m_doc->root_render();
//    auto el = root_render->get_element_by_point(x, y, 0, 0);
//
//    if (el)
//    {
//        for (auto& child : el->children())
//        {
//            auto pos = child->get_placement();
//            if (pos.is_point_inside(x, y))
//            {
//                std::string text; child->get_text(text);
//                wxString u8text = wxString::FromUTF8(text);
//                auto* font = (wxFont*)child->parent()->css().get_font();
//                wxMemoryDC dc;
//                dc.SetFont(*font);
//                litehtml::position c_pos{ pos.left(), pos.top(), 0.0f, pos.height };
//                size_t index = 0;
//                for (auto c : u8text)
//                {
//                    wxCoord width;
//                    dc.GetTextExtent(c, &width, nullptr);
//                    c_pos.width = width;
//                    if (c_pos.is_point_inside(x, y))
//                    {
//                        return Selection{ child, index, c_pos };
//                    }
//                    c_pos.x += width;
//                    index += 1;
//
//                }
//
//
//            }
//        }
//    }
//    return Selection{};
//}


//void HtmlWindow::UpdateSelectionRect()
//{
//    if(m_selection_start.empty() || 
//        m_selection_end.empty() || 
//        m_selection_start == m_selection_end)
//    {
//        return;
//    }
//    m_selection_rect.clear();
//    m_selection_text = "";
//    m_selection_start_el = nullptr;
//    m_selection_end_el = nullptr;
//    Point start_pt{};
//    Point end_pt{};
//    if (m_selection_start.y >= m_selection_end.y)
//    {
//        start_pt = m_selection_end;
//        end_pt = m_selection_start;
//    }else
//    {
//        start_pt = m_selection_start;
//        end_pt = m_selection_end;
//    }
//    litehtml::position selection_rect{ std::min(m_selection_start.x, m_selection_end.x),
//                                      std::min(m_selection_start.y, m_selection_end.y),
//                                      std::abs(m_selection_end.x - m_selection_start.x),
//                                      std::abs(m_selection_end.y - m_selection_start.y) };
//
//
// 
//    auto root = m_doc->root();
//
//    UpdateSelectionElement(root, selection_rect);
//    
//    if (!m_selection_start_el || !m_selection_end_el) { return; }
//    if (m_selection_start_el == m_selection_end_el)
//    {
//        litehtml::position pos = m_selection_start_el->get_placement();
//        std::string text;
//        m_selection_start_el->get_text(text);
//        wxString u8text = wxString::FromUTF8(text);
//        auto* font = (wxFont*)m_selection_start_el->parent()->css().get_font();
//        wxMemoryDC dc;
//        dc.SetFont(*font);
//        litehtml::position c_pos{ pos.left(), pos.top(), 0.0f, pos.height };
//        for (auto c : u8text)
//        {
//            wxCoord width;
//            dc.GetTextExtent(c, &width, nullptr);
//            c_pos.width = width;
//            if (selection_rect.does_intersect(&c_pos))
//            {
//                m_selection_rect.add(c_pos);
//                m_selection_text += wxString(c).ToUTF8();
//            }
//            c_pos.x += width;
//
//
//        }
//    }
//    else
//    {
//        bool start = false;
//        bool end = false;
//        AddRecursive(root, selection_rect, start, end);
//    }
//
//
//    m_selection_rect.scroll(-m_scrollPos);
//    m_selection_text.erase(m_selection_text.find_last_not_of("\r\n") + 1);
//    if(!m_selection_text.empty())
//    {
//        wxLogInfo(m_selection_text);
//    }
//
//}
//
//void HtmlWindow::AddRecursive(litehtml::element::ptr el, litehtml::position sel_rect, bool& start, bool& end)
//{
//    if (end) { return; }
//
//    for (auto child: el->children())
//    {
//        if(child->is_text())
//        {
//            auto pos = child->get_placement();
//            
//            // different el_text
//            if(child == m_selection_end_el)
//            {
//                end = true;
//                if (sel_rect.intersect(pos) == pos)
//                {
//                    child->get_text(m_selection_text);
//                    m_selection_rect.add(pos);
//                }
//                else
//                {
//                    std::string text;
//                    child->get_text(text);
//                    wxString u8text = wxString::FromUTF8(text);
//                    auto* font = (wxFont*)child->parent()->css().get_font();
//                    wxMemoryDC dc;
//                    dc.SetFont(*font);
//                    litehtml::position c_pos{ pos.left(), pos.top(), 0.0f, pos.height };
//                    size_t index = 0;
//                    for (auto c : u8text)
//                    {
//                        wxCoord width;
//                        dc.GetTextExtent(c, &width, nullptr);
//                        c_pos.width = width;
//                        if (sel_rect.does_intersect(&c_pos))
//                        {
//                            m_selection_rect.add(c_pos);
//                           
//                        }
//                        else
//                        {
//                            
//                            break;
//                        }
//                        c_pos.x += width;
//                        index += 1;
//
//                    }
//                    m_selection_text += u8text.SubString(0, index-1).ToUTF8();
//                }
//                return;
//            }
//            if(start)
//            {
//                child->get_text(m_selection_text);
//                m_selection_rect.add(pos);
//                continue;
//            }
//            if(child == m_selection_start_el)
//            {
//                start = true;
//                if(sel_rect.intersect(pos) == pos)
//                {
//                    child->get_text(m_selection_text);
//                    m_selection_rect.add(pos);
//                }
//                else
//                {
//                    std::string text;
//                    child->get_text(text);
//                    wxString u8text = wxString::FromUTF8(text);
//                    auto* font = (wxFont*)child->parent()->css().get_font();
//                    wxMemoryDC dc;
//                    dc.SetFont(*font);
//                    litehtml::position c_pos{ pos.left(), pos.top(), 0.0f, pos.height };
//                    size_t index = 0;
//                    for (auto c : u8text)
//                    {
//                        wxCoord width;
//                        dc.GetTextExtent(c, &width, nullptr);
//                        c_pos.width = width;
//                        if (sel_rect.does_intersect(&c_pos))
//                        {
//                            m_selection_rect.add(c_pos);
//                            m_selection_text += wxString(c).ToUTF8();
//                        }
//                        c_pos.x += width;
//                        index += 1;
//
//                    }
//                    
//                }
//            }
//
//        }
//        else
//        {
//            AddRecursive(child, sel_rect, start, end);
//        }
//    }
//}

void HtmlWindow::ClearSelection()
{
    m_selection = false;
    m_selection_start = -1;
    m_selection_end = -1;
    m_selection_text = "";
    m_selection_rect.clear();
    
    //m_selection_start_el = nullptr;
    //m_selection_end_el = nullptr;
}
//void HtmlWindow::UpdateSelectionElement(litehtml::element::ptr el, const litehtml::position& sel_rect)
//{
//    for (auto child : el->children())
//    {
//        if(child->is_text())
//        {
//            auto pos = child->get_placement();
//            if(!m_selection_start_el && 
//                (pos.does_intersect(&sel_rect) ||
//                pos.is_point_inside(sel_rect.left(), sel_rect.top()))  )
//            {
//                m_selection_start_el = child;
//            }
//            if(pos.does_intersect(&sel_rect) ||
//                pos.is_point_inside(sel_rect.right(), sel_rect.bottom()))
//            {
//
//                    m_selection_end_el = child;
//            
//            }
//        }
//        else
//        {
//            UpdateSelectionElement(child, sel_rect);
//        }
//    }
//}
// 鼠标事件处理
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

        ClearSelection();

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
        //m_selection_rect.clear();
        auto pos = hit_test(x, y);
        if(pos >= 0)
        {
            //m_selection_rect.add(m_char_boxes[pos]);
            //m_selection_rect.scroll(-m_scrollPos);
            m_cursor_pos = pos;
            Refresh();
        }
    }
    //if (m_doc)
    //{
    //    auto sel = GetSelectionChar(pt.x, pt.y + m_scrollPos);
    //    m_selection_rect.add(sel.position());

    //}
    //event.Skip();
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
                m_selection_end = pos;
                UpdateSelectionRect();
            }
        }

 

        Refresh();
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
            }
        }
        m_selection = false;
        Refresh();
    }
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
    for(int i = start; i < end; i++)
    {
        m_selection_rect.add(m_char_boxes[i]);
    }

    // process scroll offset
    m_selection_rect.scroll(-m_scrollPos);

    // this is get selection text, and convert to utf-8
    if(!m_plain_text.empty() && 
        start < m_plain_text.size() && 
        end < m_plain_text.size())
    {
        m_selection_text = litehtml::utf32_to_utf8(m_plain_text.substr(start, end - start));
    }
    std::string selected_text;
    selected_text += "[original] " + std::to_string(m_selection_start) + ", " + std::to_string(m_selection_end) + "\n";

    selected_text += "[position] " + std::to_string(start) + ", " + std::to_string(end) + "\n";
    
    selected_text += m_selection_text + "\n";
    wxLogInfo(wxString::FromUTF8(selected_text));
    
}
void HtmlWindow::OnMouseLeave(wxMouseEvent& event)
{
    // 调用文档的鼠标离开事件
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
        // 清除剪贴板内容
        wxTheClipboard->Clear();

        // 设置文本数据
        wxTheClipboard->SetData(new wxTextDataObject(text));

        // 关闭剪贴板
        wxTheClipboard->Close();

        return true;
    }
    return false;
}
void HtmlWindow::OnKeyDown(wxKeyEvent& event)
{
    // 处理 ESC 键取消
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

wxBEGIN_EVENT_TABLE(HtmlWindow, wxScrolled<wxPanel>)
    EVT_PAINT(HtmlWindow::OnPaint)
    EVT_SCROLLWIN(HtmlWindow::OnScroll)
    EVT_MOUSEWHEEL(HtmlWindow::OnMouseWheel)
    EVT_SIZE(HtmlWindow::OnSize)
    EVT_DROP_FILES(HtmlWindow::OnDropFiles)
    EVT_LEFT_DOWN(HtmlWindow::OnLeftDown)
    EVT_LEFT_UP(HtmlWindow::OnLeftUp)
    EVT_MOTION(HtmlWindow::OnMouseMove)
    EVT_LEAVE_WINDOW(HtmlWindow::OnMouseLeave)  // 添加这行
    EVT_KEY_DOWN(HtmlWindow::OnKeyDown)
wxEND_EVENT_TABLE()