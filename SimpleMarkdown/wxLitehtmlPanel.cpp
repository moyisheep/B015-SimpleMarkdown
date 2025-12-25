#include "wxLitehtmlPanel.h"
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/font.h>
#include <wx/filesys.h>
#include <wx/uri.h>
#include <wx/tokenzr.h>
#include <wx/fontenum.h>
#include <wx/clipbrd.h>
#include <algorithm>

//wxLitehtmlPanel::wxLitehtmlPanel(wxFrame* parent) : wxPanel(parent), m_parent(parent)
//{
//    SetBackgroundStyle(wxBG_STYLE_PAINT);
//    Bind(wxEVT_PAINT, &wxLitehtmlPanel::OnPaint, this);
//    Bind(wxEVT_SIZE, [this](wxSizeEvent&) { Refresh(); });
//}
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


    m_doc = litehtml::document::createFromString(html.c_str() , this);
    if (m_doc)
    {
        int width = GetClientSize().GetWidth();
        m_doc->render(width);
        SetupScrollbars(); // 添加这行
    }
    Refresh();
}

litehtml::element::ptr wxLitehtmlPanel::create_element(const char* tag_name,
    const litehtml::string_map& attributes, const std::shared_ptr<litehtml::document>& doc)
{
    return nullptr;
}

litehtml::uint_ptr wxLitehtmlPanel::create_font(const litehtml::font_description& descr,
    const litehtml::document* doc, litehtml::font_metrics* fm)
{
    wxFont font;
    int size = (int)(descr.size * 96.0f / 72.0f); // Convert pt to px

    // Parse font family list (comma-separated)
    wxArrayString fontFamilies;
    wxStringTokenizer tokenizer(wxString::FromUTF8(descr.family.c_str()), ",");
    while (tokenizer.HasMoreTokens())
    {
        wxString family = tokenizer.GetNextToken().Trim(true).Trim(false);
        if (!family.empty())
        {
            fontFamilies.Add(family);
        }
    }

    // Try to find the first available font from the family list
    wxFontFamily family = wxFONTFAMILY_DEFAULT;
    bool foundFont = false;

    for (const auto& familyName : fontFamilies)
    {
        if (wxFontEnumerator::IsValidFacename(familyName))
        {
            // Create font directly with the family name
            font = wxFont(size, wxFONTFAMILY_DEFAULT,
                descr.style == litehtml::font_style_italic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
                descr.weight >= 700 ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
                false, familyName);
            foundFont = true;
            break;
        }
    }

    // If no specific font family found, fall back to generic families
    if (!foundFont)
    {
        // Check if any generic family is specified
        for (const auto& familyName : fontFamilies)
        {
            if (familyName.IsSameAs("serif", false))
            {
                family = wxFONTFAMILY_ROMAN;
                foundFont = true;
                break;
            }
            else if (familyName.IsSameAs("sans-serif", false) || familyName.IsSameAs("sans serif", false))
            {
                family = wxFONTFAMILY_SWISS;
                foundFont = true;
                break;
            }
            else if (familyName.IsSameAs("monospace", false))
            {
                family = wxFONTFAMILY_TELETYPE;
                foundFont = true;
                break;
            }
        }

        if (!foundFont)
        {
            family = wxFONTFAMILY_DEFAULT;
        }

        font = wxFont(size, family,
            descr.style == litehtml::font_style_italic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
            descr.weight >= 700 ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
            false, wxEmptyString);
    }

    // Handle font decorations
    if (descr.decoration_line & litehtml::text_decoration_line_underline)
    {
        font.SetUnderlined(true);
    }
    if (descr.decoration_line & litehtml::text_decoration_line_line_through)
    {
        font.SetStrikethrough(true);
    }

    // Get font metrics if requested
    if (fm)
    {
        wxMemoryDC dc;
        dc.SetFont(font);
        wxCoord height, descent, externalLeading;
        dc.GetTextExtent("x", nullptr, &height, &descent, &externalLeading);

        fm->ascent = height - descent;
        fm->descent = descent;
        fm->height = height;

        // Better approximation for x-height
        wxCoord xHeight;
        dc.GetTextExtent("x", nullptr, &xHeight);
        fm->x_height = xHeight;
    }

    return (litehtml::uint_ptr) new wxFont(font);
}
litehtml::pixel_t wxLitehtmlPanel::get_default_font_size() const
{
    return 16; // Default font size in pixels
}

const char* wxLitehtmlPanel::get_default_font_name() const
{
    return "Arial";
}

void wxLitehtmlPanel::get_media_features(litehtml::media_features& media) const
{
    wxSize sz = GetClientSize();

    media.type = litehtml::media_type_screen;
    media.width = sz.GetWidth();
    media.height = sz.GetHeight();
    media.device_width = sz.GetWidth();
    media.device_height = sz.GetHeight();
    media.color = 8;
    media.monochrome = 0;
    media.color_index = 256;
    media.resolution = 96;
}

void wxLitehtmlPanel::import_css(litehtml::string& text,
    const litehtml::string& url, litehtml::string& baseurl)
{
    // Not implemented in this basic version
}

void wxLitehtmlPanel::load_image(const char* src, const char* baseurl, bool redraw_on_ready)
{
    // Not implemented in this basic version
}

litehtml::pixel_t wxLitehtmlPanel::pt_to_px(float pt) const
{
    return (litehtml::pixel_t)(pt * 96.0f / 72.0f);
}

litehtml::pixel_t wxLitehtmlPanel::text_width(const char* text, litehtml::uint_ptr hFont)
{
    wxFont* font = (wxFont*)hFont;
    if (!font) return 0;

    wxMemoryDC dc;
    dc.SetFont(*font);
    wxCoord width;
    dc.GetTextExtent(wxString::FromUTF8(text), &width, nullptr);
    return width;
}

void wxLitehtmlPanel::transform_text(litehtml::string& text, litehtml::text_transform tt)
{
    switch (tt)
    {
    case litehtml::text_transform_capitalize:
        if (!text.empty()) text[0] = toupper(text[0]);
        break;
    case litehtml::text_transform_uppercase:
        std::transform(text.begin(), text.end(), text.begin(), ::toupper);
        break;
    case litehtml::text_transform_lowercase:
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        break;
    default:
        break;
    }
}

void wxLitehtmlPanel::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius)
{
    wxClientDC dc(this);
    dc.SetClippingRegion(pos.x, pos.y, pos.width, pos.height);
}

void wxLitehtmlPanel::set_caption(const char* caption)
{
    if (m_parent)
    {
        m_parent->SetLabel(wxString::FromUTF8(caption));
    }
}

void wxLitehtmlPanel::get_viewport(litehtml::position& viewport) const
{
    wxSize sz = GetClientSize();
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = sz.GetWidth();
    viewport.height = sz.GetHeight();
}

void wxLitehtmlPanel::get_image_size(const char* src, const char* baseurl, litehtml::size& sz)
{
    sz.width = 0;
    sz.height = 0;
    // Not implemented in this basic version
}

void wxLitehtmlPanel::del_clip()
{
    wxClientDC dc(this);
    dc.DestroyClippingRegion();
}

void wxLitehtmlPanel::delete_font(litehtml::uint_ptr hFont)
{
    wxFont* font = (wxFont*)hFont;
    delete font;
}

void wxLitehtmlPanel::draw_text(litehtml::uint_ptr hdc, const char* text,
    litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
    wxDC* dc = (wxDC*)hdc;
    wxFont* font = (wxFont*)hFont;

    if (!dc || !font) return;

    dc->SetFont(*font);
    dc->SetTextForeground(wxColour(color.red, color.green, color.blue));
    dc->DrawText(wxString::FromUTF8(text), pos.x, pos.y);

    // 缓存文本信息
    wxSize textSize = dc->GetTextExtent(wxString::FromUTF8(text));
    AddTextToCache(wxString::FromUTF8(text), wxRect(pos.x, pos.y, textSize.GetWidth(), textSize.GetHeight()), font);
}

void wxLitehtmlPanel::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
    // Basic implementation - just draw a bullet
    if (marker.marker_type == litehtml::list_style_type_disc)
    {
        wxDC* dc = (wxDC*)hdc;
        dc->SetBrush(*wxBLACK_BRUSH);
        dc->DrawCircle(marker.pos.x + marker.pos.width / 2,
            marker.pos.y + marker.pos.height / 2,
            std::min(marker.pos.width, marker.pos.height) / 2);
    }
}

void wxLitehtmlPanel::draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer,
    const std::string& url, const std::string& base_url)
{
    // Not implemented in this basic version
}

void wxLitehtmlPanel::draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer,
    const litehtml::web_color& color)
{
    wxDC* dc = (wxDC*)hdc;
    dc->SetBrush(wxBrush(wxColour(color.red, color.green, color.blue, color.alpha)));
    dc->SetPen(*wxTRANSPARENT_PEN);
    dc->DrawRectangle(layer.border_box.x, layer.border_box.y,
        layer.border_box.width, layer.border_box.height);
}

void wxLitehtmlPanel::draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer,
    const litehtml::background_layer::linear_gradient& gradient)
{
    // Not implemented in this basic version
    //draw_solid_fill(hdc, layer, gradient.colors.empty() ? litehtml::web_color() : gradient.colors[0]);
}

void wxLitehtmlPanel::draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer,
    const litehtml::background_layer::radial_gradient& gradient)
{
    // Not implemented in this basic version
    //draw_solid_fill(hdc, layer, gradient.colors.empty() ? litehtml::web_color() : gradient.colors[0]);
}

void wxLitehtmlPanel::draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer,
    const litehtml::background_layer::conic_gradient& gradient)
{
    // Not implemented in this basic version
    //draw_solid_fill(hdc, layer, gradient.colors.empty() ? litehtml::web_color() : gradient.colors[0]);
}

void wxLitehtmlPanel::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders,
    const litehtml::position& draw_pos, bool root)
{
    wxDC* dc = (wxDC*)hdc;

    // Draw each border if it exists
    if (borders.top.width > 0)
    {
        dc->SetPen(wxPen(wxColour(borders.top.color.red, borders.top.color.green, borders.top.color.blue),
            borders.top.width));
        dc->DrawLine(draw_pos.left(), draw_pos.top(), draw_pos.right(), draw_pos.top());
    }

    if (borders.right.width > 0)
    {
        dc->SetPen(wxPen(wxColour(borders.right.color.red, borders.right.color.green, borders.right.color.blue),
            borders.right.width));
        dc->DrawLine(draw_pos.right(), draw_pos.top(), draw_pos.right(), draw_pos.bottom());
    }

    if (borders.bottom.width > 0)
    {
        dc->SetPen(wxPen(wxColour(borders.bottom.color.red, borders.bottom.color.green, borders.bottom.color.blue),
            borders.bottom.width));
        dc->DrawLine(draw_pos.left(), draw_pos.bottom(), draw_pos.right(), draw_pos.bottom());
    }

    if (borders.left.width > 0)
    {
        dc->SetPen(wxPen(wxColour(borders.left.color.red, borders.left.color.green, borders.left.color.blue),
            borders.left.width));
        dc->DrawLine(draw_pos.left(), draw_pos.top(), draw_pos.left(), draw_pos.bottom());
    }
}

void wxLitehtmlPanel::set_cursor(const char* cursor)
{
    // Basic cursor support
    if (strcmp(cursor, "pointer") == 0)
    {
        SetCursor(wxCURSOR_HAND);
    }
    else
    {
        SetCursor(wxCURSOR_ARROW);
    }
}

void wxLitehtmlPanel::set_base_url(const char* base_url)
{
    // Not implemented in this basic version
}

void wxLitehtmlPanel::get_language(litehtml::string& language, litehtml::string& culture) const
{
    language = "en";
    culture = "";
}

void wxLitehtmlPanel::link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el)
{
    // Not implemented in this basic version
}

void wxLitehtmlPanel::on_anchor_click(const char* url, const litehtml::element::ptr& el)
{
    // Not implemented in this basic version
}

bool wxLitehtmlPanel::on_element_click(const litehtml::element::ptr& el)
{
    return false;
}

void wxLitehtmlPanel::on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event)
{
    // Not implemented in this basic version
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
    this->set_base_url(base_url.c_str());

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
        get_viewport(pos);

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



// 添加文本到缓存
void wxLitehtmlPanel::AddTextToCache(const wxString& text, const wxRect& rect, wxFont* font)
{
    TextChunk chunk;
    chunk.text = text;
    chunk.rect = rect;
    chunk.font = std::make_shared<wxFont>(*font);
    m_textChunks.push_back(chunk);
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