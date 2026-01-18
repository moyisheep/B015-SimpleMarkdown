#pragma once



#include <litehtml.h>
#include <wx/wx.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include "wxContainer.h"


#include <filesystem>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>



#include "LocalVFS.h"

class Selection
{
public:
    Selection(litehtml::element::ptr el = nullptr, size_t index=0, litehtml::position pos = litehtml::position{})
    {
        m_el = el;
        m_pos = pos;
        m_index = index;
    }
    size_t index() { return m_index; }
    bool empty()
    {
        if (!m_el || m_pos.empty()) { return true; }
        return false;
    }
    bool is_same_element(const Selection& val) { return m_el == val.m_el; }
    litehtml::position position() { return m_pos; }
    litehtml::element::ptr element() { return m_el; }
    void clear()
    {
        m_el = nullptr;
        m_index = 0;
        m_pos.clear();
    }
    bool operator==(const Selection& val)
    {
        return ((m_el == val.m_el) && (m_index = val.m_index) && (m_pos == val.m_pos));
    }
private:
    litehtml::element::ptr m_el = nullptr;
    size_t m_index = 0;
    litehtml::position m_pos{};

};
struct Point
{
    float x = 0;
    float y = 0;
    void clear()
    {
        x = 0;
        y = 0;
    }
    bool empty()
    {
        return (x == 0) && (y == 0);
    }
    bool operator==(const Point& val)
    {
        return (x == val.x) && (y == val.y);
    }


};
class SelectionRect
{
public:
    SelectionRect() { m_rect.clear(); };

    // m_selection_rect.add(...)
    void add(litehtml::position pos)
    {
        if(!m_rect.empty())
        {
            auto back = m_rect.back();
            if(back.y == pos.y)
            {
                m_rect.pop_back();
                back.width += pos.width;
                m_rect.push_back(back);
                return;
            }
        }
  
         m_rect.push_back(pos);

    }


    void scroll(float delta)
    {
        for (auto& rect: m_rect)
        {
            rect.y = rect.y + delta;
        }
    }

    std::vector<wxRect> get_rect()
    {
        std::vector<wxRect> rects;
        for (auto& rect : m_rect)
        {
            rects.push_back(wxRect((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height));
        }
        return rects;
    }
    std::vector<litehtml::position> get_raw_rect() { return m_rect; }
    bool empty()
    {
        return m_rect.empty();
    }
    void clear()
    {
        m_rect.clear();
    }
private:
    std::vector<litehtml::position> m_rect;
};
class HtmlWindow :  public wxScrolled<wxPanel>
{
public:
    HtmlWindow(wxWindow* parent);
    ~HtmlWindow();


    void set_html(const std::string& html);
    void record_char_boxes();
    void record_char_boxes_recursive(litehtml::element::ptr el);
    bool load_html(const std::string& file_path);

    bool set_user_css(const std::string& css);
    bool load_user_css(const std::string& path);
    void clear();

    void set_vfs(std::shared_ptr<VirtualFileSystem>& vfs);

    std::string get_html();
  
    void EnableDragAndDrop(bool enable = true);



protected:
    std::shared_ptr<VirtualFileSystem> m_vfs = nullptr;
private:
    int32_t m_cursor_pos = -1;
    std::vector<litehtml::position> m_char_boxes;
    int m_scrollPos;
    std::unique_ptr<wxContainer> m_container;
    litehtml::document::ptr m_doc;

    std::u32string m_plain_text;
    std::string m_html = "";
    wxFrame* m_parent;
    wxBitmap m_back_bitmap;

    // 滚动相关变量
    int m_totalHeight;
    // Add to your existing private section
    std::string m_base_url;
    std::unique_ptr<wxStaticText> m_link_ctrl;
    bool m_selection = false;
    bool m_scrolling = false;
    int m_scrolling_delta = 0;
    std::string m_user_css = "";
    SelectionRect m_selection_rect;

    int32_t m_selection_start;
    int32_t m_selection_end;

    std::string m_selection_text = "";

    std::string m_hover_link = "";
private:


    // 滚动相关方法
    void SetupScrollbars();
    void ScrollToPosition(int pos);
    int GetScrollPosition() const;

    void ShowLinkWindow(std::string link);
    void HideLinkWindow();

    void RequestRedraw(const litehtml::position::vector& redraw_boxes);


    void OnPaint(wxPaintEvent& event);
    
    void DrawCaret(wxDC* dc);

    void DrawSelection(wxDC* dc, wxRect updateRect);
    void OnScroll(wxScrollWinEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnSize(wxSizeEvent& event);



    // Add these event handlers
    void OnDropFiles(wxDropFilesEvent& event);






    // 新增事件处理
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);

    void UpdateSelectionRect();
    void ClearSelection();
    void OnKeyDown(wxKeyEvent& event);

    int32_t hit_test(float x, float y);


    //void UpdateCursor(const wxPoint& pt);

    bool CopyToClipboard(const wxString& text);
 

};
