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

class HtmlWindow :  public wxScrolled<wxPanel>
{
public:
    HtmlWindow(wxWindow* parent);
    ~HtmlWindow();

    // 其他方法
    void set_html(const std::string& html);
    bool open_html(const std::string& file_path);

    // 滚动相关方法
    void SetupScrollbars();
    void ScrollToPosition(int pos);
    int GetScrollPosition() const;

    // 拖拽加载
    void EnableDragAndDrop(bool enable = true);



private:

    std::unique_ptr<wxContainer> m_container;
    litehtml::document::ptr m_doc;

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

    // Add these event handlers
    void OnDropFiles(wxDropFilesEvent& event);


    // Add to your existing private section
    std::string m_base_url;



    // 新增事件处理
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);

    bool m_selection = false;
    wxRect m_selection_rect;

    //void UpdateCursor(const wxPoint& pt);


    DECLARE_EVENT_TABLE()
};
