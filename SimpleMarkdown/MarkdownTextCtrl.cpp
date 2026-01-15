// MarkdownTextCtrl.cpp
#include "MarkdownTextCtrl.h"
#include <cmark-gfm.h>
#include <cmark-gfm-extension_api.h>
#include <cmark-gfm-core-extensions.h>
#include <algorithm>
#include <wx/timer.h>

// 样式ID定义
enum {
    STYLE_NORMAL = 0,
    STYLE_HEADER1,
    STYLE_HEADER2,
    STYLE_HEADER3,
    STYLE_HEADER4,
    STYLE_HEADER5,
    STYLE_HEADER6,
    STYLE_CODE,
    STYLE_CODEBLOCK,
    STYLE_LINK,
    STYLE_EMPHASIS,
    STYLE_STRONG,
    STYLE_BLOCKQUOTE,
    STYLE_LIST,
    STYLE_HR,
    STYLE_STRIKETHROUGH
};



MarkdownTextCtrl::MarkdownTextCtrl(wxWindow* parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size,
    long style, const wxString& name)
    : wxStyledTextCtrl(parent, id, pos, size, style, name),
    m_liveHighlighting(true),
    m_needsHighlighting(false),
    m_highlightTimer(nullptr),
    m_parser(nullptr)
{
    // 初始化cmark-gfm解析器
    m_parser = cmark_parser_new(CMARK_OPT_DEFAULT);

    // 启用Github风格Markdown扩展
    cmark_gfm_core_extensions_ensure_registered();

    m_tableExtension = cmark_find_syntax_extension("table");
    m_strikethroughExtension = cmark_find_syntax_extension("strikethrough");
    m_autolinkExtension = cmark_find_syntax_extension("autolink");

    if (m_tableExtension) {
        cmark_parser_attach_syntax_extension(m_parser, m_tableExtension);
    }
    if (m_strikethroughExtension) {
        cmark_parser_attach_syntax_extension(m_parser, m_strikethroughExtension);
    }
    if (m_autolinkExtension) {
        cmark_parser_attach_syntax_extension(m_parser, m_autolinkExtension);
    }

    // 初始化样式
    InitializeStyles();

    // 设置编辑器选项
    SetMarginWidth(0, 40); // 行号边距
    SetMarginType(0, wxSTC_MARGIN_NUMBER);

    // 启用多行选择
    SetMultipleSelection(true);
    SetAdditionalSelectionTyping(true);
    SetMultiPaste(1);

    // 设置制表符和缩进
    SetTabWidth(4);
    SetUseTabs(false);
    SetIndent(4);

    // 启用自动缩进
    SetIndentationGuides(wxSTC_IV_LOOKFORWARD);
    SetBackSpaceUnIndents(true);

    // 设置自动完成
    AutoCompSetIgnoreCase(true);
    AutoCompSetAutoHide(false);

    SetWrapMode(wxSTC_WRAP_WHITESPACE);  // 按空白字符换行
    // 创建高亮定时器
    m_highlightTimer = new wxTimer(this, wxID_ANY);


    // 修改这里：使用Bind方法替代Connect
    Bind(wxEVT_TIMER, &MarkdownTextCtrl::OnTimer, this, m_highlightTimer->GetId());
    Bind(wxEVT_IDLE, &MarkdownTextCtrl::OnIdle, this);
    Bind(wxEVT_STC_CHANGE, &MarkdownTextCtrl::OnTextChanged, this);
}

MarkdownTextCtrl::~MarkdownTextCtrl()
{
    if (m_parser) {
        cmark_parser_free(m_parser);
    }
    if (m_highlightTimer) {
        m_highlightTimer->Stop();
        delete m_highlightTimer;
    }
}

void MarkdownTextCtrl::InitializeStyles()
{
    // 清除所有样式
    StyleClearAll();

    // 设置默认字体
    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    StyleSetFont(wxSTC_STYLE_DEFAULT, font);
    StyleSetForeground(wxSTC_STYLE_DEFAULT, *wxBLACK);
    StyleSetBackground(wxSTC_STYLE_DEFAULT, *wxWHITE);

    // 普通文本样式
    m_styles.clear();

    // 标题样式
    MarkdownStyle style;

    // 标题1
    style.id = STYLE_HEADER1;
    style.foreground = wxColour(0, 0, 128); // 深蓝色
    style.background = *wxWHITE;
    style.fontSize = 24;
    style.bold = true;
    m_styles.push_back(style);

    // 标题2
    style.id = STYLE_HEADER2;
    style.foreground = wxColour(0, 0, 160);
    style.fontSize = 20;
    m_styles.push_back(style);

    // 标题3
    style.id = STYLE_HEADER3;
    style.foreground = wxColour(0, 0, 192);
    style.fontSize = 16;
    m_styles.push_back(style);

    // 标题4
    style.id = STYLE_HEADER4;
    style.foreground = wxColour(0, 0, 224);
    style.fontSize = 14;
    m_styles.push_back(style);

    // 标题5
    style.id = STYLE_HEADER5;
    style.foreground = wxColour(64, 64, 255);
    style.fontSize = 12;
    m_styles.push_back(style);

    // 标题6
    style.id = STYLE_HEADER6;
    style.foreground = wxColour(128, 128, 255);
    style.fontSize = 11;
    m_styles.push_back(style);

    // 代码样式
    style.id = STYLE_CODE;
    style.foreground = wxColour(255, 0, 0); // 红色
    style.background = wxColour(240, 240, 240);
    style.fontSize = 0; // 使用默认大小
    style.bold = false;
    style.fontName = "Courier New";
    m_styles.push_back(style);

    // 代码块样式
    style.id = STYLE_CODEBLOCK;
    style.foreground = wxColour(0, 128, 0); // 绿色
    style.background = wxColour(248, 248, 248);
    m_styles.push_back(style);

    // 链接样式
    style.id = STYLE_LINK;
    style.foreground = wxColour(0, 0, 255); // 蓝色
    style.background = *wxWHITE;
    style.underline = true;
    m_styles.push_back(style);

    // 强调样式（斜体）
    style.id = STYLE_EMPHASIS;
    style.foreground = *wxBLACK;
    style.background = *wxWHITE;
    style.italic = true;
    style.underline = false;
    m_styles.push_back(style);

    // 粗体样式
    style.id = STYLE_STRONG;
    style.foreground = *wxBLACK;
    style.bold = true;
    style.italic = false;
    m_styles.push_back(style);

    // 引用样式
    style.id = STYLE_BLOCKQUOTE;
    style.foreground = wxColour(64, 64, 64);
    style.background = wxColour(248, 248, 248);
    style.bold = false;
    style.italic = true;
    m_styles.push_back(style);

    // 列表样式
    style.id = STYLE_LIST;
    style.foreground = wxColour(128, 0, 128); // 紫色
    style.background = *wxWHITE;
    style.italic = false;
    m_styles.push_back(style);

    // 分隔线样式
    style.id = STYLE_HR;
    style.foreground = wxColour(192, 192, 192);
    style.background = *wxWHITE;
    style.bold = true;
    m_styles.push_back(style);

    // 删除线样式
    style.id = STYLE_STRIKETHROUGH;
    style.foreground = wxColour(128, 128, 128);
    style.background = *wxWHITE;
    m_styles.push_back(style);

    // 应用所有样式
    for (const auto& s : m_styles) {
        StyleSetForeground(s.id, s.foreground);
        StyleSetBackground(s.id, s.background);
        if (s.fontSize > 0) {
            StyleSetSize(s.id, s.fontSize);
        }
        if (s.bold) {
            StyleSetBold(s.id, true);
        }
        if (s.italic) {
            StyleSetItalic(s.id, true);
        }
        if (s.underline) {
            StyleSetUnderline(s.id, true);
        }
        if (!s.fontName.IsEmpty()) {
            wxFont font(s.fontSize > 0 ? s.fontSize : 10,
                wxFONTFAMILY_MODERN,
                s.italic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
                s.bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
                false,
                s.fontName);
            StyleSetFont(s.id, font);
        }
    }

    // 设置代码块的边距
    SetMarginWidth(1, 16); // 代码块折叠边距
    SetMarginType(1, wxSTC_MARGIN_SYMBOL);
    SetMarginMask(1, wxSTC_MASK_FOLDERS);
    SetMarginSensitive(1, true);

    // 设置折叠
    SetProperty("fold", "1");
    SetProperty("fold.comment", "1");
    SetProperty("fold.compact", "1");
    SetProperty("fold.html", "1");
}

void MarkdownTextCtrl::EnableLiveHighlighting(bool enable)
{
    m_liveHighlighting = enable;
    if (enable) {
        HighlightMarkdown();
    }
}

void MarkdownTextCtrl::HighlightMarkdown()
{
    if (!m_liveHighlighting) return;

    m_needsHighlighting = true;

    // 使用定时器延迟高亮，避免频繁触发
    if (m_highlightTimer) {
        m_highlightTimer->Start(500, true); // 500ms后触发
    }
}

void MarkdownTextCtrl::OnTextChanged(wxCommandEvent& event)
{
    event.Skip(); // 继续处理事件

    // 检查文本是否真的改变了
    wxString currentText = GetText();
    if (currentText == m_lastText) {
        return;
    }
    m_lastText = currentText;

    HighlightMarkdown();
}

void MarkdownTextCtrl::OnIdle(wxIdleEvent& event)
{
    if (m_needsHighlighting) {
        m_needsHighlighting = false;
        ParseAndStyle();
    }
    event.Skip();
}
void MarkdownTextCtrl::OnTimer(wxTimerEvent& event)
{
    if (m_needsHighlighting) {
        m_needsHighlighting = false;
        ParseAndStyle();
    }
    event.Skip();
}

void MarkdownTextCtrl::ParseAndStyle()
{
    wxString text = GetText();
    if (text.IsEmpty()) {
        ClearAllStyling();
        return;
    }

    // 创建新的解析器（替代cmark_parser_reset）
    cmark_parser* parser = cmark_parser_new(CMARK_OPT_DEFAULT);
    if (!parser) {
        return;
    }

    // 将wxString转换为char*
    std::string utf8_text = text.ToUTF8().data();

    // 解析Markdown
    cmark_parser_feed(m_parser, utf8_text.c_str(), utf8_text.length());
    cmark_node* document = cmark_parser_finish(m_parser);

    if (!document) {
        return;
    }

    // 首先清除所有样式
    ClearAllStyling();

   
    ApplyBaseStyle();

    // 遍历文档树并应用样式
    cmark_iter* iter = cmark_iter_new(document);
    cmark_event_type event;

    while ((event = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
        cmark_node* node = cmark_iter_get_node(iter);

        if (event == CMARK_EVENT_ENTER) {
            int start_line = cmark_node_get_start_line(node) - 1;
            int end_line = cmark_node_get_end_line(node) - 1;

            if (start_line >= 0 && end_line >= start_line) {
                int start_pos = cmark_node_get_start_column(node) - 1;
                int end_pos = cmark_node_get_end_column(node) - 1;
                int length = end_pos - start_pos + 1;
                start_pos += PositionFromLine(start_line);

                if(end_line == start_line)
                {
          
                 
                    end_pos = start_pos + length;
                }else
                {
                    end_pos += PositionFromLine(end_line) + 1;
                }

                ApplyNodeStyle(node, start_pos, end_pos);
            }
        }
    }

    cmark_iter_free(iter);
    cmark_node_free(document);


}

void MarkdownTextCtrl::ApplyNodeStyle(cmark_node* node, int start_pos, int end_pos)
{
    cmark_node_type node_type = cmark_node_get_type(node);

    switch (node_type) {

    case CMARK_NODE_CODE_BLOCK:
        SetStyleRange(start_pos, end_pos, STYLE_CODEBLOCK);
        break;
    case CMARK_NODE_CODE:
    {
        SetStyleRange(start_pos, end_pos, STYLE_CODEBLOCK);
        break;
    }
    case CMARK_NODE_BLOCK_QUOTE:
        SetStyleRange(start_pos, end_pos, STYLE_BLOCKQUOTE);
        break;

    case CMARK_NODE_LIST:
        SetStyleRange(start_pos, end_pos, STYLE_LIST);
        break;

    case CMARK_NODE_HEADING: {
        int level = cmark_node_get_heading_level(node);
        switch (level) {
        case 1: SetStyleRange(start_pos, end_pos, STYLE_HEADER1); break;
        case 2: SetStyleRange(start_pos, end_pos, STYLE_HEADER2); break;
        case 3: SetStyleRange(start_pos, end_pos, STYLE_HEADER3); break;
        case 4: SetStyleRange(start_pos, end_pos, STYLE_HEADER4); break;
        case 5: SetStyleRange(start_pos, end_pos, STYLE_HEADER5); break;
        case 6: SetStyleRange(start_pos, end_pos, STYLE_HEADER6); break;
        }
        break;
    }
    case CMARK_NODE_STRONG:
    {
        SetStyleRange(start_pos, end_pos, STYLE_STRONG);
        break;
    }
    case CMARK_NODE_EMPH:
    {
        SetStyleRange(start_pos, end_pos, STYLE_EMPHASIS);
        break;
    }
    case CMARK_NODE_LINK:
    {
        SetStyleRange(start_pos, end_pos, STYLE_LINK);
        break;
    }
    case CMARK_NODE_HRULE:
    {
        SetStyleRange(start_pos, end_pos, STYLE_HR);
        break;
    }
    default:
        break;
    }
}

void MarkdownTextCtrl::ClearAllStyling()
{
    // 将所有文本重置为默认样式
    int length = GetTextLength();
    if (length > 0) {
        StartStyling(0); // 31是所有样式位
        SetStyling(length, STYLE_NORMAL);
    }
}

void MarkdownTextCtrl::ApplyBaseStyle()
{
    // 设置整个文档为普通样式
    int length = GetTextLength();
    if (length > 0) {
        StyleSetForeground(STYLE_NORMAL, *wxBLACK);
        StyleSetBackground(STYLE_NORMAL, *wxWHITE);
        StartStyling(0);
        SetStyling(length, STYLE_NORMAL);
    }
}




void MarkdownTextCtrl::SetStyleRange(int start, int end, int style)
{
    if (start < 0 || end < start || start >= GetTextLength()) {
        return;
    }

    int length = wxMin(end, GetTextLength()) - start;
    if (length <= 0) return;

    StartStyling(start);
    SetStyling(length, style);
}

wxString MarkdownTextCtrl::GetRenderedHTML()
{
    wxString text = GetText();
    if (text.IsEmpty()) {
        return wxEmptyString;
    }

    std::string utf8_text = text.ToUTF8().data();

    // 创建新的解析器（替代cmark_parser_reset）
    cmark_parser* parser = cmark_parser_new(CMARK_OPT_DEFAULT);
    if (!parser) {
        return wxEmptyString;
    }

    // 解析Markdown
    cmark_parser_feed(m_parser, utf8_text.c_str(), utf8_text.length());
    cmark_node* document = cmark_parser_finish(m_parser);

    if (!document) {
        return wxEmptyString;
    }

    // 渲染为HTML
    char* html = cmark_render_html(document, CMARK_OPT_DEFAULT, NULL);
    wxString result = wxString::FromUTF8(html);

    cmark_node_free(document);
    free(html);

    return result;
}

void MarkdownTextCtrl::SetHeaderStyle(int level, const wxColour& color, int fontSize, bool bold)
{
    if (level < 1 || level > 6) return;

    int style_id = STYLE_HEADER1 + (level - 1);
    StyleSetForeground(style_id, color);
    if (fontSize > 0) {
        StyleSetSize(style_id, fontSize);
    }
    StyleSetBold(style_id, bold);
}

void MarkdownTextCtrl::SetCodeStyle(const wxColour& background, const wxColour& textColor)
{
    StyleSetBackground(STYLE_CODE, background);
    StyleSetForeground(STYLE_CODE, textColor);
    StyleSetBackground(STYLE_CODEBLOCK, background);
    StyleSetForeground(STYLE_CODEBLOCK, textColor);
}

void MarkdownTextCtrl::SetLinkStyle(const wxColour& color, bool underline)
{
    StyleSetForeground(STYLE_LINK, color);
    StyleSetUnderline(STYLE_LINK, underline);
}

void MarkdownTextCtrl::SetQuoteStyle(const wxColour& color, const wxColour& background)
{
    StyleSetForeground(STYLE_BLOCKQUOTE, color);
    StyleSetBackground(STYLE_BLOCKQUOTE, background);
}