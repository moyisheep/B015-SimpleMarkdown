#include "wxContainer.h"
#include <wx/url.h>
#include <wx/dcgraph.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Timer.h"

wxContainer::wxContainer(wxWindow* window)
    :m_wnd(window)
{
    m_hover_link = "";
}

wxContainer::~wxContainer()
{
}

litehtml::element::ptr wxContainer::create_element(const char* tag_name,
    const litehtml::string_map& attributes, const std::shared_ptr<litehtml::document>& doc)
{
    return nullptr;
}

litehtml::uint_ptr wxContainer::create_font(const litehtml::font_description& descr,
    const litehtml::document* doc, litehtml::font_metrics* fm)
{
    std::unique_ptr<Timer> timer = std::make_unique<Timer>("create_font", 1);
    wxFont font;
    //int size = (int)(descr.size * 96.0f / 72.0f); // Convert pt to px
    int size = descr.size;
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

        fm->font_size = descr.size;
        fm->ascent = height - descent;
        fm->descent = descent;
        fm->height = height;

        // Better approximation for x-height
        wxCoord xHeight;
        dc.GetTextExtent("x", nullptr, &xHeight);
        fm->x_height = xHeight;
    }
    timer.reset();
    return (litehtml::uint_ptr) new wxFont(font);
}
litehtml::pixel_t wxContainer::get_default_font_size() const
{
    return 14; // Default font size in pixels
}

const char* wxContainer::get_default_font_name() const
{
    return "Arial";
}

void wxContainer::get_media_features(litehtml::media_features& media) const
{
    std::unique_ptr<Timer> timer = std::make_unique<Timer>("get_media_features", 1);
    wxSize sz = m_wnd->GetClientSize();

    media.type = litehtml::media_type_screen;
    media.width = sz.GetWidth();
    media.height = sz.GetHeight();
    media.device_width = sz.GetWidth();
    media.device_height = sz.GetHeight();
    media.color = 8;
    media.monochrome = 0;
    media.color_index = 256;
    media.resolution = 96;
    timer.reset();
}



void wxContainer::import_css(litehtml::string& text,
    const litehtml::string& url, litehtml::string& baseurl)
{
    std::unique_ptr<Timer> timer = std::make_unique<Timer>("import_css", 1);
    if (!m_vfs) return;

    try
    {
        
        
 
        // 获取CSS文件内容
        auto file_data = m_vfs->get_binary(wxURL(url).BuildUnescapedURI().ToStdString());
        if ( !file_data.empty())
        {
            // 将二进制数据转换为字符串
            text = litehtml::string(file_data.begin(), file_data.end());
            timer.reset();
            // 更新baseurl为CSS文件所在目录
            
        }
        else
        {
            timer.reset();
            wxLogWarning("Failed to load CSS file: %s", url);
        }
    }
    catch (const std::exception& e)
    {
        timer.reset();
        wxLogError("Error importing CSS: %s", e.what());
    }
}

void wxContainer::load_image(const char* src, const char* baseurl, bool redraw_on_ready)
{
    Timer("load_image", 1);
    if (!m_vfs || !src) return;
 
    //std::string src_str(src);
    //std::string baseurl_str(baseurl ? baseurl : "");

    //try
    //{
    //    // 检查图片是否已经在缓存中
    //    std::string cache_key = src_str + "|" + baseurl_str;
    //    auto it = m_imageCache.find(cache_key);
    //    if (it != m_imageCache.end())
    //    {
    //        // 图片已经在缓存中
    //        if (redraw_on_ready)
    //        {
    //            // 请求重绘
    //            m_wnd->Refresh();
    //        }
    //        return;
    //    }

    //    // 解析图片路径
    //    


    //    // 获取图片数据
    //    auto file_data = m_vfs->get_binary(wxURL(src).BuildUnescapedURI().ToStdString());;
    //    if (file_data.empty())
    //    {
    //        wxLogWarning("Failed to load image data: %s", src);
    //        return;
    //    }

    //    // 从内存数据创建wxImage
    //    wxMemoryInputStream mem_stream(file_data.data(), file_data.size());
    //    wxImage image;

    //    // 根据MIME类型确定格式
    //    bool image_loaded = false;



    //    image_loaded = image.LoadFile(mem_stream);
   
    //    if (image_loaded && image.IsOk())
    //    {
    //        // 缓存图片
    //        wxBitmap bitmap(image);
    //        m_imageCache[cache_key] = bitmap;

    //        if (redraw_on_ready)
    //        {
    //            m_wnd->Refresh();
    //        }
    //    }
    //    else
    //    {
    //        wxLogWarning("Failed to decode image: %s", src);
    //    }
    //}
    //catch (const std::exception& e)
    //{
    //    wxLogError("Error loading image: %s, error: %s", src, e.what());
    //}
}

void wxContainer::get_image_size(const char* src, const char* baseurl, litehtml::size& sz)
{
    std::unique_ptr<Timer> timer = std::make_unique<Timer>("get_image_size", 1);
    sz.width = 0;
    sz.height = 0;

    if (!m_vfs || !src) 
    {
        timer.reset();
        return;
    } 
  

    std::string src_str(src);
    std::string baseurl_str(baseurl ? baseurl : "");

    try
    {
        // 检查图片是否已经在缓存中
        std::string cache_key = src_str + "|" + baseurl_str;
        auto it = m_imageCache.find(cache_key);
        if (it != m_imageCache.end())
        {
            // 从缓存中获取尺寸
            const wxBitmap& bitmap = it->second;
            if (bitmap.IsOk())
            {
                sz.width = bitmap.GetWidth();
                sz.height = bitmap.GetHeight();
            }
            timer.reset();
            return;
        }

 

        // 尝试只读取图片头信息来获取尺寸（避免加载整个图片）
        auto file_data = m_vfs->get_binary(wxURL(src).BuildUnescapedURI().ToStdString());;
        if (file_data.empty())
        {
            timer.reset();
            return;
        }

        // 使用 stb_image 只获取图片信息（不加载像素数据）
        int width = 0, height = 0, channels = 0;
        int success = stbi_info_from_memory(
            reinterpret_cast<const stbi_uc*>(file_data.data()),
            static_cast<int>(file_data.size()),
            &width, &height, &channels);

        if (success && width > 0 && height > 0)
        {
            sz.width = width;
            sz.height = height;
        }

        timer.reset();
    }
    catch (const std::exception& e)
    {
        timer.reset();
        wxLogError("Error getting image size: %s, error: %s", src, e.what());
    }
}

void wxContainer::draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer,
    const std::string& url, const std::string& base_url)
{
    std::unique_ptr<Timer> timer = std::make_unique<Timer>("draw_image", 1);
    //wxPaintDC* dc = (wxPaintDC*)hdc;
    //if (!dc || !m_vfs || url.empty())
    //{
    //    timer.reset();
    //    return;
    //}

    //try
    //{
    //    // 查找缓存的图片
    //    std::string cache_key = url + "|" + base_url;
    //    auto it = m_imageCache.find(cache_key);
    //    if (it == m_imageCache.end())
    //    {
    //        // 图片未加载，尝试加载它（同步方式）
    //          // 使用 stb_image 解码
    //        auto file_data = m_vfs->get_binary(wxURL(url).BuildUnescapedURI().ToStdString());;
    //        if (file_data.empty())
    //        {
    //            timer.reset();
    //            return;
    //        }
    //        int width = 0, height = 0, channels = 0;
    //        stbi_uc* image_data = stbi_load_from_memory(
    //            reinterpret_cast<const stbi_uc*>(file_data.data()),
    //            static_cast<int>(file_data.size()),
    //            &width, &height, &channels, 0);

    //        if (image_data && width > 0 && height > 0)
    //        {
    //            // 根据通道数创建 wxImage
    //            wxImage image(width, height, false); // 不初始化数据

    //            if (channels == 1) // 灰度图
    //            {
    //                // 转换为RGB
    //                image.Create(width, height);
    //                unsigned char* rgb_data = image.GetData();
    //                for (int i = 0; i < width * height; ++i)
    //                {
    //                    unsigned char gray = image_data[i];
    //                    rgb_data[i * 3] = gray;
    //                    rgb_data[i * 3 + 1] = gray;
    //                    rgb_data[i * 3 + 2] = gray;
    //                }
    //            }
    //            else if (channels == 3) // RGB
    //            {
    //                // 直接复制数据（stb_image 的 RGB 顺序和 wxImage 一致）
    //                image.Create(width, height);
    //                memcpy(image.GetData(), image_data, width * height * 3);
    //            }
    //            else if (channels == 4) // RGBA
    //            {
    //                // 处理带透明度的图片
    //                image.Create(width, height);

    //                // 分配 Alpha 通道数据
    //                image.InitAlpha();

    //                // 复制 RGB 数据并设置 Alpha
    //                unsigned char* rgb_data = image.GetData();
    //                unsigned char* alpha_data = image.GetAlpha();

    //                for (int i = 0; i < width * height; ++i)
    //                {
    //                    rgb_data[i * 3] = image_data[i * 4];       // R
    //                    rgb_data[i * 3 + 1] = image_data[i * 4 + 1]; // G
    //                    rgb_data[i * 3 + 2] = image_data[i * 4 + 2]; // B
    //                    alpha_data[i] = image_data[i * 4 + 3];     // A
    //                }
    //            }

    //            // 释放 stb_image 数据
    //            stbi_image_free(image_data);

    //            if (image.IsOk())
    //            {
    //                // 缓存图片
    //                wxBitmap bitmap(image);
    //                m_imageCache[cache_key] = bitmap;

    //            }
    //            it = m_imageCache.find(cache_key);
    //            if (it == m_imageCache.end())
    //            {
    //                wxLogWarning("Image not loaded: %s", url);
    //                timer.reset();
    //                return;
    //            }
    //        }
    //        
    //    }
    //    const wxBitmap& bitmap = it->second;
    //    if (!bitmap.IsOk())
    //    {
    //        wxLogWarning("Invalid bitmap for: %s", url);
    //        timer.reset();
    //        return;
    //    }

    //    // 获取图片原始尺寸
    //    int img_width = bitmap.GetWidth();
    //    int img_height = bitmap.GetHeight();

    //    if (img_width <= 0 || img_height <= 0)
    //    {
    //        timer.reset();
    //        return;
    //    }

    //    // 计算绘制区域
    //    int border_x = layer.border_box.x;
    //    int border_y = layer.border_box.y;
    //    int border_width = layer.border_box.width;
    //    int border_height = layer.border_box.height;

    //    // 获取origin_box（背景定位参考点）
    //    int origin_x = layer.origin_box.x;
    //    int origin_y = layer.origin_box.y;
    //    int origin_width = layer.origin_box.width;
    //    int origin_height = layer.origin_box.height;

    //    // 计算背景位置（这里简化处理，实际CSS背景定位更复杂）
    //    int bg_x = origin_x;
    //    int bg_y = origin_y;

    //    // 处理background-repeat
    //    int draw_width = img_width;
    //    int draw_height = img_height;

    //    switch (layer.repeat)
    //    {
    //    case litehtml::background_repeat_repeat:
    //        // 平铺（默认）
    //        // 计算需要绘制的次数
    //        for (int y = bg_y; y < border_y + border_height; y += draw_height)
    //        {
    //            for (int x = bg_x; x < border_x + border_width; x += draw_width)
    //            {
    //                dc->DrawBitmap(bitmap, x, y, true);
    //            }
    //        }
    //        break;

    //    case litehtml::background_repeat_repeat_x:
    //        // 水平平铺
    //        for (int x = bg_x; x < border_x + border_width; x += draw_width)
    //        {
    //            dc->DrawBitmap(bitmap, x, bg_y, true);
    //        }
    //        break;

    //    case litehtml::background_repeat_repeat_y:
    //        // 垂直平铺
    //        for (int y = bg_y; y < border_y + border_height; y += draw_height)
    //        {
    //            dc->DrawBitmap(bitmap, bg_x, y, true);
    //        }
    //        break;

    //    case litehtml::background_repeat_no_repeat:
    //    default:
    //        // 不平铺，只绘制一次
    //    {
    //        // 确保图片在边框区域内
    //        if (bg_x + draw_width > border_x + border_width)
    //        {
    //            draw_width = border_x + border_width - bg_x;
    //        }
    //        if (bg_y + draw_height > border_y + border_height)
    //        {
    //            draw_height = border_y + border_height - bg_y;
    //        }
    //        if (draw_width > 0 && draw_height > 0)
    //        {
    //            // 如果尺寸不匹配，需要创建缩放后的位图
    //            if (draw_width != img_width || draw_height != img_height)
    //            {
    //                wxImage img = bitmap.ConvertToImage();
    //                if (img.IsOk())
    //                {
    //                    img.Rescale(draw_width, draw_height, wxIMAGE_QUALITY_HIGH);
    //                    wxBitmap scaled_bitmap(img);
    //                    dc->DrawBitmap(scaled_bitmap, bg_x, bg_y, true);
    //                }
    //            }
    //            else
    //            {
    //                dc->DrawBitmap(bitmap, bg_x, bg_y, true);
    //            }
    //        }
    //    }
    //    break;
    //    }

    //    // 处理background-attachment（这里简化处理，实际需要处理滚动）
    //    // background_attachment_fixed: 相对于视口固定
    //    // background_attachment_scroll: 随内容滚动（默认）
    //    // 在实际的浏览器中，这需要根据滚动位置调整绘制位置

    //    // 应用裁剪区域
    //    if (layer.clip_box.width > 0 && layer.clip_box.height > 0)
    //    {
    //        // 注意：wxWidgets的裁剪是叠加的，所以需要先清除之前的裁剪
    //        dc->DestroyClippingRegion();
    //        dc->SetClippingRegion(layer.clip_box.x, layer.clip_box.y,
    //            layer.clip_box.width, layer.clip_box.height);
    //    }

    //    // 处理border-radius（如果有的话）
    //    if (layer.border_radius.top_left_x > 0 || layer.border_radius.top_right_x > 0 ||
    //        layer.border_radius.bottom_left_x > 0 || layer.border_radius.bottom_right_x > 0)
    //    {
    //        // 这里可以添加圆角裁剪逻辑
    //        // 注意：wxWidgets没有内置的圆角裁剪，可能需要使用wxGraphicsContext
    //        // 或者创建路径进行裁剪
    //    }
    //    timer.reset();
    //}
    //catch (const std::exception& e)
    //{
    //    timer.reset();
    //    wxLogError("Error drawing image: %s, error: %s", url, e.what());
    //}
}

litehtml::pixel_t wxContainer::pt_to_px(float pt) const
{
    return (litehtml::pixel_t)(pt * 96.0f / 72.0f);
}

litehtml::pixel_t wxContainer::text_width(const char* text, litehtml::uint_ptr hFont)
{
    std::unique_ptr<Timer> timer = std::make_unique<Timer>("text_width", 1);
    wxFont* font = (wxFont*)hFont;
    if (font) 
    {
        wxString wtext = wxString::FromUTF8(text);


        
        wxMemoryDC dc;
        dc.SetFont(*font);

        int width = 0;
        for(int i=0; i<wtext.length(); i++)
        {
            auto ch = wtext[i];
            bool found = false;
            // char cache
            for(auto& cache: m_charWidthCache)
            {
                if(cache.hFont == hFont && cache.ch == ch)
                {
                    width += cache.width;
                    found = true;
                    break;
                }
            }
            if(!found)
            {
                int w;
                dc.GetTextExtent(ch, &w, nullptr);
                width += w;
                m_charWidthCache.push_back(CharWidthCache{ hFont, ch, w });
            }
        }


        timer.reset();
        return width;
    }
     
    timer.reset();
    return 0;

}

void wxContainer::transform_text(litehtml::string& text, litehtml::text_transform tt)
{
    std::unique_ptr<Timer> timer = std::make_unique<Timer>("transform_text", 1);
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
    timer.reset();
}

void wxContainer::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius)
{
    //std::unique_ptr<Timer> timer = std::make_unique<Timer>("set_clip", 1);
    //wxClientDC dc(m_wnd);
    //dc.SetClippingRegion(pos.x, pos.y, pos.width, pos.height);
    //timer.reset();
}

void wxContainer::set_caption(const char* caption)
{
    Timer("set_caption", 1);
    if (m_wnd->GetParent())
    {
        m_wnd->GetParent()->SetLabel(wxString::FromUTF8(caption));
    }
}

void wxContainer::get_viewport(litehtml::position& viewport) const
{
    Timer("get_viewport", 1);
    wxSize sz = m_wnd->GetClientSize();
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = sz.GetWidth();
    viewport.height = sz.GetHeight();
}



void wxContainer::del_clip()
{
    //std::unique_ptr<Timer> timer = std::make_unique<Timer>("del_clip", 1);
    //wxClientDC dc(m_wnd);
    //dc.DestroyClippingRegion();
    //timer.reset();
}

void wxContainer::delete_font(litehtml::uint_ptr hFont)
{
    Timer("delete_font", 1);
    wxFont* font = (wxFont*)hFont;
    delete font;
}

void wxContainer::draw_text(litehtml::uint_ptr hdc, const char* text,
    litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
    std::unique_ptr<Timer> timer = std::make_unique<Timer>("draw_text", 1);
    m_drawTextCache.add(hFont, text, color, pos);
    timer.reset();
}

void wxContainer::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
    std::unique_ptr<Timer> timer = std::make_unique<Timer>("draw_list_marker", 1);
    // Basic implementation - just draw a bullet
    if (marker.marker_type == litehtml::list_style_type_disc)
    {
        auto* dc = (wxGraphicsContext*)hdc;
        dc->SetBrush(*wxBLACK_BRUSH);

        dc->DrawEllipse(marker.pos.x + marker.pos.width / 2,
            marker.pos.y + marker.pos.height / 2,
            marker.pos.width, marker.pos.height);
    }
    timer.reset();
}



void wxContainer::draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer,
    const litehtml::web_color& color)
{
    std::unique_ptr<Timer> timer = std::make_unique<Timer>("draw_solid_fill", 1);
    wxGraphicsContext* dc = (wxGraphicsContext*)hdc;
    dc->SetBrush(wxBrush(wxColour(color.red, color.green, color.blue, color.alpha)));
    dc->SetPen(*wxTRANSPARENT_PEN);
    dc->DrawRectangle(layer.border_box.x, layer.border_box.y,
        layer.border_box.width, layer.border_box.height);
    timer.reset();
}
void wxContainer::draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer,
    const litehtml::background_layer::linear_gradient& gradient)
{

    std::unique_ptr<Timer> timer = std::make_unique<Timer>("draw_linear_gradient", 1);
    auto* dc = (wxGraphicsContext*)hdc;
    if (!dc) 
    {
        wxLogError("draw_linear_gradient: null graphics context");
        return; 
    }
    
    auto stops = wxGraphicsGradientStops();
    for(const auto& color_point: gradient.color_points)
    {
        const auto& color = color_point.color;
        stops.Add(wxColour(color.red, color.green, color.blue, color.alpha), color_point.offset);
    }
    auto brush = dc->CreateLinearGradientBrush(gradient.start.x, gradient.start.y, 
                                               gradient.end.x, gradient.end.y, 
        stops);

    dc->SetBrush(brush);
    dc->DrawRoundedRectangle(wxDouble(layer.border_box.x),
        wxDouble(layer.border_box.y),
        wxDouble(layer.border_box.width),
        wxDouble(layer.border_box.height), 1);


    timer.reset();
}

void wxContainer::draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer,
    const litehtml::background_layer::radial_gradient& gradient)
{

}

void wxContainer::draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer,
    const litehtml::background_layer::conic_gradient& gradient)
{

}

void wxContainer::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders,
    const litehtml::position& draw_pos, bool root)
{
    std::unique_ptr<Timer> timer = std::make_unique<Timer>("draw_borders", 1);
    auto* dc = (wxGraphicsContext*)hdc;

    // Draw each border if it exists
    if (borders.top.width > 0)
    {
        dc->SetPen(wxPen(wxColour(borders.top.color.red, borders.top.color.green, borders.top.color.blue),
            borders.top.width));
        //dc->DrawLine(draw_pos.left(), draw_pos.top(), draw_pos.right(), draw_pos.top());
        dc->StrokeLine(draw_pos.left(), draw_pos.top(), draw_pos.right(), draw_pos.top());
    }

    if (borders.right.width > 0)
    {
        dc->SetPen(wxPen(wxColour(borders.right.color.red, borders.right.color.green, borders.right.color.blue),
            borders.right.width));
        //dc->DrawLine(draw_pos.right(), draw_pos.top(), draw_pos.right(), draw_pos.bottom());
        dc->StrokeLine(draw_pos.right(), draw_pos.top(), draw_pos.right(), draw_pos.bottom());

    }

    if (borders.bottom.width > 0)
    {
        dc->SetPen(wxPen(wxColour(borders.bottom.color.red, borders.bottom.color.green, borders.bottom.color.blue),
            borders.bottom.width));
        //dc->DrawLine(draw_pos.left(), draw_pos.bottom(), draw_pos.right(), draw_pos.bottom());
        dc->StrokeLine(draw_pos.left(), draw_pos.bottom(), draw_pos.right(), draw_pos.bottom());

    }

    if (borders.left.width > 0)
    {
        dc->SetPen(wxPen(wxColour(borders.left.color.red, borders.left.color.green, borders.left.color.blue),
            borders.left.width));
        //dc->DrawLine(draw_pos.left(), draw_pos.top(), draw_pos.left(), draw_pos.bottom());
        dc->StrokeLine(draw_pos.left(), draw_pos.top(), draw_pos.left(), draw_pos.bottom());

    }
    timer.reset();
}

void wxContainer::set_cursor(const char* cursor)
{
    // Basic cursor support
    //if (strcmp(cursor, "pointer") == 0)
    //{
    //    SetCursor(wxCURSOR_HAND);
    //}
    //else
    //{
    //    SetCursor(wxCURSOR_ARROW);
    //}
}

void wxContainer::set_base_url(const char* base_url)
{
    // Not implemented in this basic version
}

void wxContainer::get_language(litehtml::string& language, litehtml::string& culture) const
{
    language = "en";
    culture = "";
}

void wxContainer::link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el)
{
    // Not implemented in this basic version
}

void wxContainer::on_anchor_click(const char* url, const litehtml::element::ptr& el)
{
    // Not implemented in this basic version
}

bool wxContainer::on_element_click(const litehtml::element::ptr& el)
{
    return false;
}

void wxContainer::on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event)
{
    if(event == litehtml::mouse_event_enter)
    {
        if(el)
        {
            std::string tag_name = el->get_tagName();
            if (tag_name == "a")
            {
                auto txt = el->get_attr("href");
                if (txt) { m_hover_link = txt; }
                else { m_hover_link = ""; }
            }
        }

    }
    else if(event == litehtml::mouse_event_leave)
    {
        m_hover_link = "";
    }
}

void wxContainer::draw_finished(litehtml::uint_ptr hdc, litehtml::position updateRect)
{
    std::unique_ptr<Timer> timer = std::make_unique<Timer>("draw_finished", 1);

    auto* dc = (wxGraphicsContext*)hdc;
    if(dc)
    {
        auto cache_list = m_drawTextCache.get_cache();
        m_drawTextCache.clear();
        for (auto& cache: cache_list)
        {
            if(updateRect.does_intersect(&cache.pos))
            {
                //std::unique_ptr<Timer> timer = std::make_unique<Timer>("draw_finished", 1);
                wxFont* font = (wxFont*)cache.hFont;
                if (font)
                {
                    wxString wtext = wxString::FromUTF8(cache.text);
                    //wxString txt = wtext +": " + 
                    //    wxString(std::to_string(cache.pos.x)) + ", " + 
                    //    wxString(std::to_string(cache.pos.y));
                    //wxLogInfo(txt);
                    
                    dc->SetFont(*font, 
                        wxColour(cache.color.red, cache.color.green, cache.color.blue, cache.color.alpha));
                    //dc->SetTextForeground(wxColour(cache.color.red, cache.color.green, cache.color.blue));
                    dc->DrawText(wtext, cache.pos.x, cache.pos.y);
                }
                //timer.reset();
            }

        }
    }
   
    timer.reset();

}

void wxContainer::clear()
{
    m_hover_link = "";
    m_imageCache.clear();
    m_charWidthCache.clear();
    m_drawTextCache.clear();
}



