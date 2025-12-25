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


class LocalVFS : public VirtualFileSystem
{
private:
    std::string base_dir;

    std::string get_extension(const std::string& path)
    {
        size_t dot = path.find_last_of('.');
        if (dot == std::string::npos) return "";

        std::string ext = path.substr(dot);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }

public:
    LocalVFS(const std::string& dir = "") : base_dir(dir)
    {
        
    }

    std::vector<unsigned char> get(const std::string& path) override
    {
        std::vector<unsigned char> data;

        std::string full_path =  path;

        // 打开文件
        std::ifstream file(full_path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            return data;  // 返回空vector
        }

        // 获取大小
        std::streamsize size = file.tellg();
        if (size <= 0)
        {
            return data;
        }

        // 读取数据
        file.seekg(0, std::ios::beg);
        data.resize(static_cast<size_t>(size));

        if (file.read(reinterpret_cast<char*>(data.data()), size))
        {
            return data;
        }

        return std::vector<unsigned char>();  // 读取失败返回空
    }

    bool exists(const std::string& path) override
    {
        
        return std::filesystem::exists(path);
    }

    std::string resolve(const std::string& url, const std::string& base) override
    {
        if (url.empty()) return "";
        return base + url;
    }
};
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

    void SelectAll();

    void RequestRedraw(const litehtml::position::vector& redraw_boxes);

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
    void OnMouseLeave(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);

    // 辅助方法
    void UpdateSelection(const wxPoint& pos);
    void DrawSelection(wxDC& dc);
    wxString ExtractTextFromSelection() const;

    void UpdateCursor(const wxPoint& pt);


    DECLARE_EVENT_TABLE()
};
