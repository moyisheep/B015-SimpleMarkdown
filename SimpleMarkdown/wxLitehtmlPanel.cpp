#include "wxLitehtmlPanel.h"
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/font.h>
#include <wx/filesys.h>
#include <wx/uri.h>
#include <wx/tokenzr.h>
#include <wx/fontenum.h>
#include <algorithm>

wxLitehtmlPanel::wxLitehtmlPanel(wxFrame* parent) : wxPanel(parent), m_parent(parent)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &wxLitehtmlPanel::OnPaint, this);
    Bind(wxEVT_SIZE, [this](wxSizeEvent&) { Refresh(); });
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


    m_doc = litehtml::document::createFromString({ html.c_str() , litehtml::encoding::utf_8}, this);
    if (m_doc)
    {
        int width = GetClientSize().GetWidth();
        m_doc->render(width);
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

void wxLitehtmlPanel::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    dc.Clear();

    if (m_doc)
    {
        litehtml::position pos;
        get_viewport(pos);

        litehtml::uint_ptr hdc = (litehtml::uint_ptr)&dc;
        m_doc->draw(hdc, 0, 0, &pos);
    }
}

wxBEGIN_EVENT_TABLE(wxLitehtmlPanel, wxPanel)
EVT_PAINT(wxLitehtmlPanel::OnPaint)
wxEND_EVENT_TABLE()