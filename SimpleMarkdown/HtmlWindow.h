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
    void add(litehtml::position pos)
    {
        m_rect.push_back(pos);
        //if (m_rect.empty()) 
        //{ 
        //    m_rect.push_back(pos); 
        //    return; 
        //}

        //auto back = m_rect.back();
        //if(back.y == pos.y && back.height == pos.height)
        //{
        //    m_rect.pop_back();
        //    back.width = pos.right() - back.left();
        //    m_rect.push_back(back);
        //}
        //else
        //{
        //    m_rect.push_back(pos);
        //}
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

    // 其他方法
    void set_html(const std::string& html);
    void record_char_boxes();
    void record_char_boxes_recursive(litehtml::element::ptr el);
    bool open_html(const std::string& file_path);

    void set_user_css(const std::string& css);
    void load_user_css(const std::string& path);

    // 滚动相关方法
    void SetupScrollbars();
    void ScrollToPosition(int pos);
    int GetScrollPosition() const;

    // 拖拽加载
    void EnableDragAndDrop(bool enable = true);

    void ShowLinkWindow(std::string link);
    void HideLinkWindow();



private:

    std::unique_ptr<wxContainer> m_container;
    litehtml::document::ptr m_doc;
    std::vector<litehtml::position> m_char_boxes;
    std::u32string m_plain_text;
    wxFrame* m_parent;

    // 滚动相关变量
    int m_totalHeight;
    int m_scrollPos;
    std::shared_ptr<VirtualFileSystem> m_vfs = nullptr;


    void RequestRedraw(const litehtml::position::vector& redraw_boxes);

    // 重写事件处理
    void OnPaint(wxPaintEvent& event);
    void OnScroll(wxScrollWinEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnSize(wxSizeEvent& event);

    litehtml::position GetSelectedRect(float x, float y);

    // Add these event handlers
    void OnDropFiles(wxDropFilesEvent& event);



    // Add to your existing private section
    std::string m_base_url;
    std::unique_ptr<wxStaticText> m_link_ctrl;


    // 新增事件处理
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    Selection GetSelectionChar(float x, float y);
    void UpdateSelectionRect();
    void ClearSelection();
    void OnKeyDown(wxKeyEvent& event);

    int32_t hit_test(float x, float y);

    bool m_selection = false;
    std::string m_user_css = "";
    SelectionRect m_selection_rect;
    //Selection m_selection_start{};
    //Selection m_selection_end{};
    int32_t m_selection_start;
    int32_t m_selection_end;
    //litehtml::element::ptr m_selection_start_el = nullptr;
    //litehtml::element::ptr m_selection_end_el = nullptr;
    std::string m_selection_text = "";

    std::string m_hover_link = "";
    //void UpdateCursor(const wxPoint& pt);
    void AddRecursive(litehtml::element::ptr el, litehtml::position sel_rect, bool& start, bool& end);
    void UpdateSelectionElement(litehtml::element::ptr el, const litehtml::position& sel_rect);
    bool CopyToClipboard(const wxString& text);
 
    DECLARE_EVENT_TABLE()
};
