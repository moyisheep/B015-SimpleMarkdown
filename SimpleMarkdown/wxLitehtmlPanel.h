#pragma once



#include <litehtml.h>
#include <wx/wx.h>
#include <memory>

#include "wxContainer.h"


class wxLitehtmlPanel :  public wxScrolled<wxPanel>
{
public:
    wxLitehtmlPanel(wxWindow* parent);
    ~wxLitehtmlPanel();

    // 其他方法
    void set_html(const std::string& html);
    bool open_html(const wxString& file_path);

    // 滚动相关方法
    void SetupScrollbars();
    void ScrollToPosition(int pos);
    int GetScrollPosition() const;

    // 拖拽加载
    void EnableDragAndDrop(bool enable = true);

    // 新增文本选择和复制功能
    wxString GetSelectedText() const;
    void CopySelectedText();
    void ClearSelection();

private:

    std::unique_ptr<wxContainer> m_container;
    litehtml::document::ptr m_doc;

    wxFrame* m_parent;
 
    // 滚动相关变量
    int m_totalHeight;
    int m_scrollPos;

    // 重写事件处理
    void OnPaint(wxPaintEvent& event);
    void OnScroll(wxScrollWinEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnSize(wxSizeEvent& event);

    // Add these event handlers
    void OnDropFiles(wxDropFilesEvent& event);
    bool CanOpenFile(const wxString& file_path);

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

    // 新增事件处理
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);

    // 辅助方法
    void UpdateSelection(const wxPoint& pos);
    void DrawSelection(wxDC& dc);
    wxString ExtractTextFromSelection() const;


    DECLARE_EVENT_TABLE()
};
