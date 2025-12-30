#pragma once


#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <memory>

#include <litehtml.h>
#include <wx/wx.h>
#include <wx/tokenzr.h>
#include <wx/fontenum.h>
#include <wx/mstream.h>

#include "VirtualFileSystem.h"



class wxContainer : public litehtml::document_container
{
public:
    wxContainer(wxWindow* window);
    ~wxContainer();


    litehtml::element::ptr create_element(const char* tag_name, const litehtml::string_map& attributes, const std::shared_ptr<litehtml::document>& doc) override;
    litehtml::uint_ptr create_font(const litehtml::font_description& descr, const litehtml::document* doc, litehtml::font_metrics* fm) override;
    litehtml::pixel_t get_default_font_size() const override;
    const char* get_default_font_name() const override;
    void get_media_features(litehtml::media_features& media) const override;
    void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl)  override;
    void load_image(const char* src, const char* baseurl, bool redraw_on_ready) override;

    litehtml::pixel_t pt_to_px(float pt) const override;
    litehtml::pixel_t text_width(const char* text, litehtml::uint_ptr hFont) override;
    void transform_text(litehtml::string& text, litehtml::text_transform tt) override;
    void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) override;
    void set_caption(const char* caption) override;


    void get_viewport(litehtml::position& viewport) const override;
    void get_image_size(const char* src, const char* baseurl, litehtml::size& sz) override;

    // litehtml::document_container interface
    void del_clip() override;
    void delete_font(litehtml::uint_ptr hFont) override;
    void draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) override;
    void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override;
    void draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url) override;
    void draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color) override;
    void draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient) override;
    void draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient) override;
    void draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient) override;
    void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;


    void set_cursor(const char* cursor) override;
    void set_base_url(const char* base_url) override;
    void get_language(litehtml::string& language, litehtml::string& culture) const override;
    void link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override;
    void on_anchor_click(const char* url, const litehtml::element::ptr& el) override;
    bool on_element_click(const litehtml::element::ptr& el) override;
    void on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event) override;

    void set_vfs(std::shared_ptr<VirtualFileSystem> vfs) { m_vfs = vfs; }
    std::shared_ptr<VirtualFileSystem> get_vfs() const { return m_vfs; }
 private:

    // Add to your existing private section
    std::string m_base_url;

    // 文本位置缓存结构
    struct TextChunk {
        wxString text;
        wxRect rect;
        std::shared_ptr<wxFont> font;
    };

    // 选择相关成员变量
    std::vector<TextChunk> m_textChunks;
    wxPoint m_selectionStart;
    wxPoint m_selectionEnd;
    bool m_isSelecting;
    std::vector<wxRect> m_selectionRects;
    wxWindow* m_wnd;
    std::shared_ptr<VirtualFileSystem> m_vfs = nullptr;
    std::unordered_map<std::string, wxBitmap> m_imageCache; // 图片缓存

    void AddTextToCache(const wxString& text, const wxRect& rect, wxFont* font);

    // 辅助函数
    float CalculateLinearGradientPosition(const wxPoint& point,
        const wxPoint& start,
        const wxPoint& end);
    wxColor InterpolateColor(const std::vector<wxColor>& colors,
        const std::vector<float>& positions,
        float t);



};
