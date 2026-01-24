// MarkdownTextCtrl.h
#ifndef MARKDOWNTEXTCTRL_H
#define MARKDOWNTEXTCTRL_H

#include <wx/wx.h>
#include <wx/stc/stc.h>  // 使用wxStyledTextCtrl作为基础
#include <string>
#include <vector>
#include <memory>

#include "VirtualFileSystem.h"

// 前向声明
struct cmark_parser;
struct cmark_node;
struct cmark_syntax_extension;

class MarkdownTextCtrl : public wxStyledTextCtrl
{
public:
    MarkdownTextCtrl(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name = wxTextCtrlNameStr);

    virtual ~MarkdownTextCtrl();



    // 设置是否启用实时语法高亮
    void enable_live_highlighting(bool enable = true);

    // 手动触发语法高亮
    void highlight_markdown();

    bool set_styles(const std::string& tomlString);
    bool load_styles(const std::string& tomlPath);
    void set_text(const std::string& text);
    std::string get_text();

    void set_vfs(std::shared_ptr<VirtualFileSystem>& vfs);
    std::shared_ptr<VirtualFileSystem> get_vfs();

 
protected:
    // 事件处理
    void OnTextChanged(wxCommandEvent& event);
    void OnIdle(wxIdleEvent& event);

    void OnTimer(wxTimerEvent& event);

private:
    // 初始化样式
    void InitializeStyles();
    void InitializeLightTheme();
    void InitializeDarkTheme();


    // 解析Markdown并应用样式
    void ParseAndStyle();

    void SetLineBackground(int line, const wxColour& color);

    // 应用节点样式
    void ApplyNodeStyle(cmark_node* node, int start_pos, int end_pos);

    // 清理样式
    void ClearAllStyling();




    // 设置特定范围的样式
    void SetStyleRange(int start, int end, int style);

private:
    bool m_liveHighlighting;
    bool m_needsHighlighting;
    wxTimer* m_highlightTimer;

    // 样式定义
    struct MarkdownStyle {
        int id;
        wxColour foreground;
        wxColour background;
        int fontSize;
        bool bold;
        bool italic;
        bool underline;
        wxString fontName;
    };

    std::vector<MarkdownStyle> m_styles;

    // 缓存上次文本用于比较

    wxString m_text;

    // cmark-gfm解析器
    cmark_parser* m_parser;
    cmark_syntax_extension* m_tableExtension;
    cmark_syntax_extension* m_strikethroughExtension;
    cmark_syntax_extension* m_autolinkExtension;


    std::shared_ptr<VirtualFileSystem> m_vfs;
    bool m_text_changed;
};

#endif // MARKDOWNTEXTCTRL_H