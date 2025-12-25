#include "wxLitehtmlPanel.h"
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/font.h>
#include <wx/filesys.h>
#include <wx/uri.h>


#include <wx/clipbrd.h>
#include <algorithm>


wxLitehtmlPanel::wxLitehtmlPanel(wxWindow* parent)
    : wxScrolled<wxPanel>(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_totalHeight = 0;
    m_scrollPos = 0;
    m_isSelecting = false;
    m_selectionStart = wxDefaultPosition;
    m_selectionEnd = wxDefaultPosition;
    SetScrollRate(0, 20); // 设置垂直滚动步长
    m_container = std::make_unique<wxContainer>(this);
}
wxLitehtmlPanel::~wxLitehtmlPanel()
{
    if (m_doc)
    {
        m_doc.reset();
    }
}

void wxLitehtmlPanel::set_html(const std::string& html)
{


    m_doc = litehtml::document::createFromString(html.c_str() , m_container.get());
    if (m_doc)
    {
        int width = GetClientSize().GetWidth();
        m_doc->render(width);
        SetupScrollbars(); // 添加这行
    }
    Refresh();
}




bool wxLitehtmlPanel::open_html(const wxString& file_path)
{
    // Check if file exists
    if (!wxFileExists(file_path)) {
        wxLogError("HTML file not found: %s", file_path);
        return false;
    }

    // Read the file content
    wxFile file(file_path);
    if (!file.IsOpened()) {
        wxLogError("Failed to open HTML file: %s", file_path);
        return false;
    }

    wxString html_content;
    if (!file.ReadAll(&html_content, wxConvAuto())) {
        wxLogError("Failed to read HTML file: %s", file_path);
        return false;
    }
    file.Close();

    // Set base URL to file's directory for relative paths
    wxFileName fn(file_path);
    fn.MakeAbsolute();
    std::string base_url = fn.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME).ToStdString();
    m_container->set_base_url(base_url.c_str());

    // Set the HTML content
    this->set_html(html_content.ToUTF8().data());
    return true;
}






void wxLitehtmlPanel::SetupScrollbars()
{
    if (!m_doc) return;

    int clientHeight = GetClientSize().GetHeight();
    m_totalHeight = m_doc->height();

    SetVirtualSize(-1, m_totalHeight);
    SetScrollRate(0, 20);

    int scrollRange = m_totalHeight - clientHeight;
    if (scrollRange > 0) {
        EnableScrolling(false, true);
        SetScrollbars(0, 20, 0, scrollRange / 20 + 1, 0, m_scrollPos / 20);
    }
    else {
        EnableScrolling(false, false);
    }
}

void wxLitehtmlPanel::ScrollToPosition(int pos)
{
    m_scrollPos = pos;
    SetScrollPos(wxVERTICAL, pos);
    Refresh();
}

int wxLitehtmlPanel::GetScrollPosition() const
{
    return m_scrollPos;
}

void wxLitehtmlPanel::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    DoPrepareDC(dc); // 处理滚动偏移

    dc.Clear();
    // 清除文本缓存（每次绘制时重新缓存）
    m_textChunks.clear();

    if (m_doc)
    {
        // 绘制选择区域
 
        litehtml::position pos;
        m_container->get_viewport(pos);

        litehtml::uint_ptr hdc = (litehtml::uint_ptr)&dc;
        m_doc->draw(hdc, 0, -m_scrollPos, &pos);
        DrawSelection(dc);
    }
}

void wxLitehtmlPanel::OnScroll(wxScrollWinEvent& event)
{
    int newPos = GetScrollPos(wxVERTICAL);
    if (newPos != m_scrollPos)
    {
        m_scrollPos = newPos;
        Refresh();
    }
    event.Skip();
}

void wxLitehtmlPanel::OnMouseWheel(wxMouseEvent& event)
{
    int rotation = event.GetWheelRotation();
    int delta = event.GetWheelDelta();
    int lines = rotation / delta;

    int newPos = m_scrollPos - lines * 20; // 20是滚动步长
    newPos = wxMax(0, wxMin(newPos, m_totalHeight - GetClientSize().GetHeight()));

    if (newPos != m_scrollPos)
    {
        ScrollToPosition(newPos);
    }
}

void wxLitehtmlPanel::OnSize(wxSizeEvent& event)
{
    if (m_doc)
    {
        int width = GetClientSize().GetWidth();
        m_doc->render(width);
        SetupScrollbars();
    }
    event.Skip();
}


void wxLitehtmlPanel::EnableDragAndDrop(bool enable)
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

bool wxLitehtmlPanel::CanOpenFile(const wxString& file_path)
{
    //wxString ext = wxFileName(file_path).GetExt().Lower();
    //return ext == "html" || ext == "htm";
    return true;
}

void wxLitehtmlPanel::OnDropFiles(wxDropFilesEvent& event)
{
    if (event.GetNumberOfFiles() > 0)
    {
        wxString* dropped = event.GetFiles();
        wxString file_path = dropped[0];

        if (CanOpenFile(file_path))
        {
            if (!open_html(file_path))
            {
                wxMessageBox("Failed to load HTML file", "Error", wxICON_ERROR);
            }
        }
        else
        {
            wxMessageBox("Only HTML files (.html, .htm) are supported", "Error", wxICON_WARNING);
        }
    }
}





// 获取选中文本
wxString wxLitehtmlPanel::GetSelectedText() const
{
    return ExtractTextFromSelection();
}

// 复制选中文本到剪贴板
void wxLitehtmlPanel::CopySelectedText()
{
    wxString selectedText = GetSelectedText();
    if (!selectedText.IsEmpty() && wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new wxTextDataObject(selectedText));
        wxTheClipboard->Close();
    }
}

// 清除选择
void wxLitehtmlPanel::ClearSelection()
{
    m_selectionStart = wxDefaultPosition;
    m_selectionEnd = wxDefaultPosition;
    m_selectionRects.clear();
    Refresh();
}

// 鼠标事件处理
void wxLitehtmlPanel::OnLeftDown(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    CalcUnscrolledPosition(pt.x, pt.y, &pt.x, &pt.y);

    m_selectionStart = pt;
    m_selectionEnd = pt;
    m_isSelecting = true;
    m_textChunks.clear(); // 清除旧缓存

    SetFocus();
    Refresh();
    event.Skip();
}

void wxLitehtmlPanel::OnMouseMove(wxMouseEvent& event)
{
    if (m_isSelecting && event.Dragging())
    {
        wxPoint pt = event.GetPosition();
        CalcUnscrolledPosition(pt.x, pt.y, &pt.x, &pt.y);

        m_selectionEnd = pt;
        UpdateSelection(pt);
        Refresh();
    }
    event.Skip();
}

void wxLitehtmlPanel::OnLeftUp(wxMouseEvent& event)
{
    if (m_isSelecting)
    {
        m_isSelecting = false;
        Refresh();
    }
    event.Skip();
}

void wxLitehtmlPanel::OnKeyDown(wxKeyEvent& event)
{
    if (event.GetKeyCode() == 'A' && event.ControlDown())
    {
        // 全选
        wxSize size = GetClientSize();
        m_selectionStart = wxPoint(0, 0);
        m_selectionEnd = wxPoint(size.GetWidth(), size.GetHeight());
        UpdateSelection(m_selectionEnd);
        Refresh();
    }
    else if (event.GetKeyCode() == 'C' && event.ControlDown())
    {
        CopySelectedText();
    }
    else if (event.GetKeyCode() == WXK_ESCAPE)
    {
        ClearSelection();
    }
    event.Skip();
}

// 更新选择区域
void wxLitehtmlPanel::UpdateSelection(const wxPoint& pt)
{
    m_selectionRects.clear();

    // 手动标准化选择区域（替代Normalize()）
    int left = wxMin(m_selectionStart.x, m_selectionEnd.x);
    int right = wxMax(m_selectionStart.x, m_selectionEnd.x);
    int top = wxMin(m_selectionStart.y, m_selectionEnd.y);
    int bottom = wxMax(m_selectionStart.y, m_selectionEnd.y);
    wxRect selectionRect(left, top, right - left, bottom - top);

    // 查找与选择区域相交的文本块
    for (const auto& chunk : m_textChunks)
    {
        if (chunk.rect.Intersects(selectionRect))
        {
            // 计算精确的交集区域
            wxRect intersect = chunk.rect;
            intersect.Intersect(selectionRect);
            m_selectionRects.push_back(intersect);
        }
    }
}

// 绘制选择高亮
void wxLitehtmlPanel::DrawSelection(wxDC& dc)
{
    if (m_selectionRects.empty() || m_textChunks.empty()) return;

    // 设置选择高亮样式
    wxColour highlightColor(51, 153, 255, 120); // 半透明蓝色
    dc.SetBrush(wxBrush(highlightColor));
    dc.SetPen(*wxTRANSPARENT_PEN);

    // 遍历所有文本块
    for (const auto& chunk : m_textChunks)
    {
        // 检查文本块是否与任何选择矩形相交
        for (const auto& selRect : m_selectionRects)
        {
            if (chunk.rect.Intersects(selRect))
            {
                // 计算精确的交集区域
                wxRect highlightRect = chunk.rect;
                highlightRect.Intersect(selRect);

                // 绘制高亮背景
                dc.DrawRectangle(highlightRect);


            }
        }
    }
}


// 从选择区域提取文本
wxString wxLitehtmlPanel::ExtractTextFromSelection() const
{
    wxString result;

    // 按Y坐标排序选择区域
    std::vector<wxRect> sortedRects = m_selectionRects;
    std::sort(sortedRects.begin(), sortedRects.end(),
        [](const wxRect& a, const wxRect& b) {
            return a.y < b.y || (a.y == b.y && a.x < b.x);
        });

    // 提取文本
    for (const auto& selRect : sortedRects)
    {
        for (const auto& chunk : m_textChunks)
        {
            if (chunk.rect.Intersects(selRect))
            {
                // 简单实现：添加整个文本块
                // 更精确的实现需要计算字符级别的选择
                result += chunk.text + " ";
                break;
            }
        }
    }

    return result.Trim();
}

wxBEGIN_EVENT_TABLE(wxLitehtmlPanel, wxScrolled<wxPanel>)
    EVT_PAINT(wxLitehtmlPanel::OnPaint)
    EVT_SCROLLWIN(wxLitehtmlPanel::OnScroll)
    EVT_MOUSEWHEEL(wxLitehtmlPanel::OnMouseWheel)
    EVT_SIZE(wxLitehtmlPanel::OnSize)
    EVT_DROP_FILES(wxLitehtmlPanel::OnDropFiles)
    EVT_LEFT_DOWN(wxLitehtmlPanel::OnLeftDown)
    EVT_LEFT_UP(wxLitehtmlPanel::OnLeftUp)
    EVT_MOTION(wxLitehtmlPanel::OnMouseMove)
    EVT_KEY_DOWN(wxLitehtmlPanel::OnKeyDown)
wxEND_EVENT_TABLE()