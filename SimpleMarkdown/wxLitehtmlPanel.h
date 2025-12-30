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

class LocalVFS : public VirtualFileSystem
{
private:
    std::string get_extension(const std::string& path) const
    {
        std::filesystem::path fs_path(path);
        std::string ext = fs_path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return ext;
    }

    std::string normalize_path(const std::string& path) const
    {
        try {
            std::filesystem::path fs_path(path);

            // 转换为绝对路径并规范化
            if (fs_path.is_relative()) {
                // 相对路径相对于当前工作目录
                fs_path = std::filesystem::absolute(fs_path);
            }

            return std::filesystem::weakly_canonical(fs_path).string();
        }
        catch (const std::exception&) {
            return path; // 出错时返回原路径
        }
    }

public:
    LocalVFS() = default;

    std::vector<unsigned char> get(const std::string& path) override
    {
        std::string full_path = normalize_path(path);

        std::ifstream file(full_path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return {};
        }

        auto size = file.tellg();
        if (size <= 0) {
            return {};
        }

        file.seekg(0, std::ios::beg);
        std::vector<unsigned char> data(static_cast<size_t>(size));

        if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
            return {};
        }

        return data;
    }

    bool exists(const std::string& path) override
    {
        std::string full_path = normalize_path(path);
        return std::filesystem::exists(full_path);
    }

    std::string resolve(const std::string& url, const std::string& base) override
    {
        if (url.empty()) return base;

        try {
            // 1. 处理绝对URL（包含协议）
            if (url.find("://") != std::string::npos) {
                return url;
            }

            // 2. 处理绝对路径（以/开头或Windows盘符）
            std::filesystem::path url_path(url);
            if (url_path.is_absolute()) {
                return normalize_path(url);
            }

            // 3. 处理data URL
            if (url.starts_with("data:")) {
                return url;
            }

            // 4. 处理相对路径（需要base路径）
            if (base.empty()) {
                // 没有base，使用当前工作目录
                return normalize_path(url);
            }

            // 获取base路径的目录部分
            std::filesystem::path base_path(base);

            // 如果base是文件路径，取目录部分
            if (base_path.has_filename()) {
                base_path = base_path.parent_path();
            }

            // 构建完整路径：base目录 + 相对url
            std::filesystem::path resolved_path = base_path / url_path;

            // 规范化路径（处理../和./）
            return std::filesystem::weakly_canonical(resolved_path).string();
        }
        catch (const std::exception& e) {
            // 错误处理：返回拼接的路径
            return (std::filesystem::path(base).parent_path() / url).string();
        }
    }

    // 辅助方法：获取当前工作目录
    static std::string get_current_directory()
    {
        return std::filesystem::current_path().string();
    }

    // 辅助方法：将路径转换为相对于当前目录的相对路径
    std::string make_relative(const std::string& path) const
    {
        try {
            std::filesystem::path fs_path(path);
            std::filesystem::path current = std::filesystem::current_path();
            return std::filesystem::relative(fs_path, current).string();
        }
        catch (const std::exception&) {
            return path;
        }
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
