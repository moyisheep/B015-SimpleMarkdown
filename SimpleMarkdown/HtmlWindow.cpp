#include "HtmlWindow.h"
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/font.h>
#include <wx/filesys.h>
#include <wx/uri.h>


#include <wx/clipbrd.h>
#include <algorithm>
#include <litehtml/el_text.h>
#include <litehtml/render_item.h>
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


    m_doc = litehtml::document::createFromString({ html.c_str() , litehtml::encoding::utf_8}, m_container.get());

    //auto el = m_doc->root()->select_one(".instructions-list");

    //if (el)
    //{
    //    std::string text = "";
    //    el->get_text(text);
    //    std::string txt = std::string(el->get_tagName()) + ":"  + text;
    //    wxLogInfo(txt);
    //    for (auto& child: el->children())
    //    {
    //        text.clear();
    //        child->get_text(text);
    //        txt = std::string(child->get_tagName()) + ":" + text;
    //        wxLogInfo(txt);
    //    }
    //    m_doc->append_children_from_string(*el, "<li><strong>Hello World</strong> </li>");


    //    
    //}
    //
    
    if (m_doc)
    {
        int width = GetClientSize().GetWidth();
        m_doc->render(width);
        SetupScrollbars(); // 添加这行
    }
    Refresh();
    //HtmlDumper dumper;
    //m_doc->dump(dumper);
    //OutputDebugStringA(dumper.get_html().c_str());

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
        html += "<html><style>a:hover{background:blue;}:selected {  background:#3297fd;color: white;}</style><body>";
        html += md_to_html(raw);
        html += "</body></html>";
    }
    else
    {
        html = raw;
    }
    
    // Set the HTML content

    this->set_html(html);
    return true;
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


    if (!m_selection_rect.empty())
    {

        dc.SetBrush(*wxMEDIUM_GREY_BRUSH);
        dc.SetPen(*wxTRANSPARENT_PEN);
        for (auto& rect : m_selection_rect.get_rect())
        {
            dc.DrawRectangle(rect);
        }
    }
 
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
        m_doc->media_changed();
        SetupScrollbars();
        Refresh();
    }
    event.Skip();
}


Selection HtmlWindow::GetSelectionChar(float x, float y)
{
    auto root_render = m_doc->root_render();
    auto el = root_render->get_element_by_point(x, y, 0, 0);

    if (el)
    {
        for (auto& child : el->children())
        {
            auto pos = child->get_placement();
            if (pos.is_point_inside(x, y))
            {
                std::string text; child->get_text(text);
                wxString u8text = wxString::FromUTF8(text);
                auto* font = (wxFont*)child->parent()->css().get_font();
                wxMemoryDC dc;
                dc.SetFont(*font);
                litehtml::position c_pos{ pos.left(), pos.top(), 0.0f, pos.height };
                size_t index = 0;
                for (auto c : u8text)
                {
                    wxCoord width;
                    dc.GetTextExtent(c, &width, nullptr);
                    c_pos.width = width;
                    if (c_pos.is_point_inside(x, y))
                    {
                        return Selection{ child, index, c_pos };
                    }
                    c_pos.x += width;
                    index += 1;

                }


            }
        }
    }
    return Selection{};
}


void HtmlWindow::UpdateSelectionRect()
{
    if(m_selection_start.empty() || 
        m_selection_end.empty() || 
        m_selection_start == m_selection_end)
    {
        return;
    }
    m_selection_rect.clear();
    // process selection in same el_text element;
    if(m_selection_start.is_same_element(m_selection_end))
    {
        auto start_pos = m_selection_start.position();
        auto end_pos = m_selection_end.position();
        m_selection_rect.add(start_pos);
        m_selection_rect.add(end_pos);
        std::string txt = "";
        m_selection_start.element()->get_text(txt);
        m_selection_text += txt.substr(m_selection_start.index(), m_selection_end.index() - m_selection_start.index() + 1);
    }
    // process selection in multiple el_text element;
    else
    {
        // process start selection element
        auto start_pos = m_selection_start.position();
        m_selection_rect.add(start_pos);
        std::string txt = "";
        m_selection_start.element()->get_text(txt);
        m_selection_text += txt.substr(m_selection_start.index());

        // process between start and end selection element
        auto start_el = m_selection_start.element();
        auto end_el = m_selection_end.element();

        auto parent_el = start_el->parent();
        bool start_selection = false;
        bool end_selection = false;
        for(auto child: parent_el->children())
        {
            if (child == start_el)
            {
                start_selection = true;
                continue;
            }
            if(child == end_el)
            {
                end_selection = true;
                break;
            }
            if(start_selection)
            {
                m_selection_rect.add(child->get_placement());
                child->get_text(m_selection_text);
            }
        }

        if(!end_selection)
        {
            wxLogInfo("还没选择完");
        }
        if(end_selection)
        {
            auto end_pos = m_selection_end.position();
            m_selection_rect.add(end_pos);
            std::string txt = "";
            m_selection_end.element()->get_text(txt);
            m_selection_text += txt.substr(0, m_selection_end.index());
        }
    }

    m_selection_rect.scroll(-m_scrollPos);
}


void HtmlWindow::ClearSelection()
{
    m_selection = false;
    m_selection_start.clear();
    m_selection_end.clear();
    m_selection_text = "";
    m_selection_rect.clear();
}

// 鼠标事件处理
void HtmlWindow::OnLeftDown(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();



    // 调用文档的鼠标按下事件
    litehtml::position::vector redraw_boxes;

    if (m_doc && m_doc->on_lbutton_down(pt.x, pt.y + m_scrollPos, 0, 0, redraw_boxes))
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
        float x = pt.x;
        float y = pt.y + m_scrollPos;
        ClearSelection();
        m_selection_start = GetSelectionChar(x, y);
        m_selection_end = m_selection_start;
        m_selection = true;

    }

    //event.Skip();
}

void HtmlWindow::OnMouseMove(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();


    if (m_doc)
    {
        litehtml::position::vector redraw_boxes;
        m_doc->on_mouse_over(pt.x, pt.y + m_scrollPos, 0, 0, redraw_boxes);


        if (!redraw_boxes.empty())
        {
            RequestRedraw(redraw_boxes);
        }
    }
    if (m_doc && m_selection)
    {
        m_selection_end = GetSelectionChar(pt.x, pt.y + m_scrollPos);
        UpdateSelectionRect();

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
    CalcUnscrolledPosition(pt.x, pt.y, &pt.x, &pt.y);

    // 调用文档的鼠标释放事件
    litehtml::position::vector redraw_boxes;

    if (m_doc && m_doc->on_lbutton_up(pt.x, pt.y + m_scrollPos, 0, 0, redraw_boxes))
    {
        if (!redraw_boxes.empty())
        {
            RequestRedraw(redraw_boxes);
        }
    }

    if (m_doc)
    {
        m_selection_end = GetSelectionChar(pt.x, pt.y + m_scrollPos);
        UpdateSelectionRect();
        m_selection = false;
        Refresh();
    }
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

    }


    event.Skip();
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