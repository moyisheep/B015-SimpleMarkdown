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
    
    std::string ext = fs::path(file_path).extension().generic_string();
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
        wxRect wxRect(rect.x, rect.y, rect.width, rect.height);

        // 转换为滚动位置
        wxPoint scrollPos;
        CalcScrolledPosition(wxRect.x, wxRect.y, &scrollPos.x, &scrollPos.y);
        wxRect.SetPosition(scrollPos);

        // 刷新区域
        RefreshRect(wxRect);
    }
}


// =============事件处理======================

void HtmlWindow::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    DoPrepareDC(dc); // 处理滚动偏移

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

        litehtml::position pos;
        m_container->get_viewport(pos);
        
        litehtml::position clip{ (float)updateRect.x, 
            (float)updateRect.y, 
            (float)updateRect.width, 
            (float)updateRect.height };
        litehtml::uint_ptr hdc = (litehtml::uint_ptr)&dc;
        m_doc->draw(hdc, 0, -m_scrollPos, &clip);
        //DrawSelection(dc);
    }
    if(m_selection)
    {
        dc.DrawRectangle(m_selection_rect);
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
        SetupScrollbars();
    }
    event.Skip();
}

// 鼠标事件处理
void HtmlWindow::OnLeftDown(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    //CalcUnscrolledPosition(pt.x, pt.y, &pt.x, &pt.y);
    CalcScrolledPosition(pt.x, pt.y, &pt.x, &pt.y);
    auto render = m_doc->root_render();

    auto el = render->get_element_by_point(pt.x, pt.y, 0, 0);
    
    if (el)
    {
        
        std::string text;
        el->get_text(text);
        std::string tagName = el->get_tagName();
        litehtml::position pos = el->get_placement();
        std::string txt = tagName + " : " + text  + "\n";
        txt += "(" + std::to_string(pos.left()) + "," +
            std::to_string(pos.top()) + "," +
            std::to_string(pos.right()) + "," +
            std::to_string(pos.bottom()) + ")";
        wxLogInfo(wxString::FromUTF8(txt));

      
        //for (auto& child: el->children())
        //{
        //    if(child)
        //    {
        //        text = "";
        //        tagName = "";
        //        litehtml::size sz;
        //        child->get_text(text);
        //        child->get_content_size(sz, 1000);
        //        tagName = child->get_tagName();
        //        pos = child->get_placement();
        //        
        //        txt = "    " + text + " ";
        //        txt += "(" + std::to_string(pos.left()) + "," +
        //            std::to_string(pos.top()) + "," +
        //            std::to_string(pos.right()) + "," +
        //            std::to_string(pos.bottom()) + ")";
        //        wxLogInfo(wxString::FromUTF8(txt));
        //    }
        //}
    }

    // 调用文档的鼠标按下事件
    litehtml::position::vector redraw_boxes;

    if (m_doc && m_doc->on_lbutton_down(pt.x, pt.y, 0, 0, redraw_boxes))
    {
        // 文档处理了事件，可能触发了重绘
        if (!redraw_boxes.empty())
        {
            // 请求重绘受影响区域
            RequestRedraw(redraw_boxes);
        }
    }


    Refresh();
    event.Skip();
}

void HtmlWindow::OnMouseMove(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    //CalcUnscrolledPosition(pt.x, pt.y, &pt.x, &pt.y);

    // 首先调用文档的鼠标悬停事件
    litehtml::position::vector redraw_boxes;


    if (m_doc)
    {

        m_doc->on_mouse_over(pt.x, pt.y, 0, 0, redraw_boxes);


        if (!redraw_boxes.empty())
        {
            RequestRedraw(redraw_boxes);
        }
    }

    //Refresh();
    // 更新光标
    //UpdateCursor(pt);

    //event.Skip();
}

void HtmlWindow::OnLeftUp(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    CalcUnscrolledPosition(pt.x, pt.y, &pt.x, &pt.y);

    // 调用文档的鼠标释放事件
    litehtml::position::vector redraw_boxes;

    if (m_doc && m_doc->on_lbutton_up(pt.x, pt.y, 0, 0, redraw_boxes))
    {
        if (!redraw_boxes.empty())
        {
            RequestRedraw(redraw_boxes);
        }
    }
    if(m_doc)
    {
        auto render = m_doc->root_render();
        auto el = render->get_text_element_by_point(pt.x, pt.y, 0, 0);
        if (el)
        {
            auto pos = el->get_placement();
            m_selection = true;
            m_selection_rect = wxRect(pos.x, pos.y, pos.width, pos.height);
        }
    }

    Refresh();
    event.Skip();
}

void HtmlWindow::OnMouseLeave(wxMouseEvent& event)
{
    // 调用文档的鼠标离开事件
    //if (m_doc)
    //{
    //    litehtml::position::vector redraw_boxes;
    //    if (m_doc->on_mouse_leave(redraw_boxes))
    //    {
    //        if (!redraw_boxes.empty())
    //        {
    //            RequestRedraw(redraw_boxes);
    //        }
    //    }
    //}

    event.Skip();
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