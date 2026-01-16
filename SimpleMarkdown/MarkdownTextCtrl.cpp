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

    StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColour(34, 34, 34));
    StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColour(250, 250, 250));


    // 标题样式
    MarkdownStyle style;

    // 基础正文
    style.id = STYLE_NORMAL;
    style.foreground = wxColour(34, 34, 34);     // 深灰黑，比纯黑更柔和
    style.background = wxColour(250, 250, 250);  // 浅灰背景，护眼
    style.bold = false;
    style.italic = false;
    style.fontSize = 11;                         // 稍大一点更易读
    style.underline = false;
    style.fontName = "Microsoft YaHei";          // 微软雅黑，中文更清晰
    m_styles.push_back(style);

    // 标题1 - 深海蓝，渐变
    style.id = STYLE_HEADER1;
    style.foreground = wxColour(26, 54, 93);     // 深海蓝
    style.background = wxColour(250, 250, 250);
    style.fontSize = 24;
    style.bold = true;
    m_styles.push_back(style);

    // 标题2
    style.id = STYLE_HEADER2;
    style.foreground = wxColour(37, 78, 137);    // 稍浅的蓝色
    style.fontSize = 20;
    m_styles.push_back(style);

    // 标题3
    style.id = STYLE_HEADER3;
    style.foreground = wxColour(54, 105, 184);   // 天蓝色
    style.fontSize = 18;
    m_styles.push_back(style);

    // 标题4
    style.id = STYLE_HEADER4;
    style.foreground = wxColour(79, 129, 189);   // 浅蓝灰
    style.fontSize = 16;
    m_styles.push_back(style);

    // 标题5
    style.id = STYLE_HEADER5;
    style.foreground = wxColour(102, 153, 204);  // 更浅的蓝
    style.fontSize = 14;
    m_styles.push_back(style);

    // 标题6
    style.id = STYLE_HEADER6;
    style.foreground = wxColour(140, 175, 217);  // 淡蓝色
    style.fontSize = 13;
    m_styles.push_back(style);

    // 代码样式（行内代码）
    style.id = STYLE_CODE;
    style.foreground = wxColour(199, 37, 78);    // 深粉红/品红
    style.background = wxColour(248, 248, 248);  // 浅灰背景
    style.fontSize = 0;
    style.bold = false;
    style.fontName = "Consolas";                 // 更好的等宽字体
    m_styles.push_back(style);

    // 代码块样式
    style.id = STYLE_CODEBLOCK;
    style.foreground = wxColour(0, 100, 0);      // 森林绿
    style.background = wxColour(245, 247, 250);  // 蓝灰背景
    m_styles.push_back(style);

    // 链接样式
    style.id = STYLE_LINK;
    style.foreground = wxColour(0, 102, 204);    // 中蓝色
    style.background = wxColour(250, 250, 250);
    style.underline = true;
    m_styles.push_back(style);

    // 强调样式（斜体）
    style.id = STYLE_EMPHASIS;
    style.foreground = wxColour(85, 85, 85);     // 深灰色
    style.background = wxColour(250, 250, 250);
    style.italic = true;
    style.underline = false;
    m_styles.push_back(style);

    // 粗体样式
    style.id = STYLE_STRONG;
    style.foreground = wxColour(34, 34, 34);     // 黑色加粗
    style.bold = true;
    style.italic = false;
    m_styles.push_back(style);

    // 引用样式
    style.id = STYLE_BLOCKQUOTE;
    style.foreground = wxColour(102, 102, 102);  // 灰色文字
    style.background = wxColour(248, 249, 250);  // 浅灰蓝背景
    style.bold = false;
    style.italic = true;
    m_styles.push_back(style);

    // 列表样式
    style.id = STYLE_LIST;
    style.foreground = wxColour(153, 51, 153);   // 紫色偏深
    style.background = wxColour(250, 250, 250);
    style.italic = false;
    m_styles.push_back(style);

    // 分隔线样式
    style.id = STYLE_HR;
    style.foreground = wxColour(220, 223, 228);  // 浅灰分隔线
    style.background = wxColour(250, 250, 250);
    style.bold = false;                         // 不需要粗体
    m_styles.push_back(style);

    // 删除线样式
    style.id = STYLE_STRIKETHROUGH;
    style.foreground = wxColour(153, 153, 153);  // 中灰色
    style.background = wxColour(250, 250, 250);
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
        StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColour(34, 34, 34));
        StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColour(250, 250, 250));
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