// MarkdownTextCtrl.h
#ifndef MARKDOWNTEXTCTRL_H
#define MARKDOWNTEXTCTRL_H

#include <wx/wx.h>
#include <wx/stc/stc.h>  // 使用wxStyledTextCtrl作为基础
#include <string>
#include <vector>
#include <memory>

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

    void InitializeLightTheme();
    void InitializeDarkTheme();

    // 设置是否启用实时语法高亮
    void EnableLiveHighlighting(bool enable = true);

    // 手动触发语法高亮
    void HighlightMarkdown();



    // 设置自定义样式
    void SetHeaderStyle(int level, const wxColour& color, int fontSize = 0, bool bold = true);
    void SetCodeStyle(const wxColour& background, const wxColour& textColor);
    void SetLinkStyle(const wxColour& color, bool underline = true);
    void SetQuoteStyle(const wxColour& color, const wxColour& background);

protected:
    // 事件处理
    void OnTextChanged(wxCommandEvent& event);
    void OnIdle(wxIdleEvent& event);

    void OnTimer(wxTimerEvent& event);

private:
    // 初始化样式
    void InitializeStyles();

    // 解析Markdown并应用样式
    void ParseAndStyle();

    // 应用节点样式
    void ApplyNodeStyle(cmark_node* node, int start_pos, int end_pos);

    // 清理样式
    void ClearAllStyling();

    // 应用基础样式
    void ApplyBaseStyle();

    // 高亮行内代码
    void HighlightInlineCode();

    // 高亮代码块
    void HighlightCodeBlocks();

    // 高亮链接
    void HighlightLinks();

    // 高亮强调文本（粗体、斜体）
    void HighlightEmphasis();

    // 高亮标题
    void HighlightHeaders();

    // 高亮引用
    void HighlightBlockquotes();

    // 高亮列表
    void HighlightLists();

    // 高亮分隔线
    void HighlightHorizontalRules();

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
    wxString m_lastText;

    // cmark-gfm解析器
    cmark_parser* m_parser;
    cmark_syntax_extension* m_tableExtension;
    cmark_syntax_extension* m_strikethroughExtension;
    cmark_syntax_extension* m_autolinkExtension;


};

#endif // MARKDOWNTEXTCTRL_H