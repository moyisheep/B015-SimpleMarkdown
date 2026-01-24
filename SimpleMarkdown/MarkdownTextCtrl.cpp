// MarkdownTextCtrl.cpp
#include "MarkdownTextCtrl.h"
#include <cmark-gfm.h>
#include <cmark-gfm-extension_api.h>
#include <cmark-gfm-core-extensions.h>
#include <algorithm>
#include <wx/timer.h>

#include <toml++/toml.hpp>

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
    STYLE_STRIKETHROUGH, 
    STYLE_FOOTNOTE_REFERENCE, 
    STYLE_FOOTNOTE_DEFINITION
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
    m_parser = cmark_parser_new(CMARK_OPT_DEFAULT | CMARK_OPT_FOOTNOTES);

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

    //InitializeLightTheme();
    InitializeDarkTheme();



    // 设置代码块的边距
    SetMarginWidth(1, 16); // 代码块折叠边距
    SetMarginType(1, wxSTC_MARGIN_SYMBOL);
    SetMarginMask(1, wxSTC_MASK_FOLDERS);
    SetMarginSensitive(1, true);
    SetCaretLineVisible(true);
    //// 设置折叠
    //SetProperty("fold", "1");
    //SetProperty("fold.comment", "1");
    //SetProperty("fold.compact", "1");
    //SetProperty("fold.html", "1");
}

// 在类定义中添加私有方法


bool MarkdownTextCtrl::set_styles(const std::string& tomlString)
{
    try {
        // 读取TOML文件
        auto data = toml::parse(tomlString);

        // 基础样式
        auto defaultStyle = data["default"];
        if (defaultStyle) {
            auto fg = defaultStyle["foreground"].as_array();
            auto bg = defaultStyle["background"].as_array();
            if (fg && bg && fg->size() == 3 && bg->size() == 3) {
                StyleSetForeground(wxSTC_STYLE_DEFAULT,
                    wxColour((*fg)[0].as_integer()->get(),
                        (*fg)[1].as_integer()->get(),
                        (*fg)[2].as_integer()->get()));
                StyleSetBackground(wxSTC_STYLE_DEFAULT,
                    wxColour((*bg)[0].as_integer()->get(),
                        (*bg)[1].as_integer()->get(),
                        (*bg)[2].as_integer()->get()));
            }
        }

        // 行号样式
        auto lineNumber = data["line_number"];
        if (lineNumber) {
            auto fg = lineNumber["foreground"].as_array();
            auto bg = lineNumber["background"].as_array();
            if (fg && bg && fg->size() == 3 && bg->size() == 3) {
                StyleSetForeground(wxSTC_STYLE_LINENUMBER,
                    wxColour((*fg)[0].as_integer()->get(),
                        (*fg)[1].as_integer()->get(),
                        (*fg)[2].as_integer()->get()));
                StyleSetBackground(wxSTC_STYLE_LINENUMBER,
                    wxColour((*bg)[0].as_integer()->get(),
                        (*bg)[1].as_integer()->get(),
                        (*bg)[2].as_integer()->get()));
            }
        }

        // 光标样式
        auto caret = data["caret"];
        if (caret) {
            auto fg = caret["foreground"].as_array();
            if (fg && fg->size() == 3) {
                SetCaretForeground(wxColour((*fg)[0].as_integer()->get(),
                    (*fg)[1].as_integer()->get(),
                    (*fg)[2].as_integer()->get()));
            }
        }

        // 边距背景
        auto margin = data["margin"];
        if (margin) {
            auto bg = margin["background"].as_array();
            if (bg && bg->size() == 3) {
                SetMarginBackground(1, wxColour((*bg)[0].as_integer()->get(),
                    (*bg)[1].as_integer()->get(),
                    (*bg)[2].as_integer()->get()));
                SetMarginBackground(0, wxColour((*bg)[0].as_integer()->get(),
                    (*bg)[1].as_integer()->get(),
                    (*bg)[2].as_integer()->get()));
            }
        }

        // 折叠边距
        auto fold = data["fold_margin"];
        if (fold) {
            auto bg = fold["background"].as_array();
            auto hi = fold["highlight"].as_array();
            if (bg && bg->size() == 3) {
                SetFoldMarginColour(true, wxColour((*bg)[0].as_integer()->get(),
                    (*bg)[1].as_integer()->get(),
                    (*bg)[2].as_integer()->get()));
            }
            if (hi && hi->size() == 3) {
                SetFoldMarginHiColour(true, wxColour((*hi)[0].as_integer()->get(),
                    (*hi)[1].as_integer()->get(),
                    (*hi)[2].as_integer()->get()));
            }
        }

        // 边缘颜色
        auto edge = data["edge"];
        if (edge) {
            auto color = edge["color"].as_array();
            if (color && color->size() == 3) {
                SetEdgeColour(wxColour((*color)[0].as_integer()->get(),
                    (*color)[1].as_integer()->get(),
                    (*color)[2].as_integer()->get()));
            }
        }

        // 当前行背景
        auto caretLine = data["caret_line"];
        if (caretLine) {
            auto bg = caretLine["background"].as_array();
            if (bg && bg->size() == 3) {
                SetCaretLineBackground(wxColour((*bg)[0].as_integer()->get(),
                    (*bg)[1].as_integer()->get(),
                    (*bg)[2].as_integer()->get()));
            }
            if (caretLine["width"].is_integer()) {
                SetCaretWidth(caretLine["width"].as_integer()->get());
            }
        }

        // 加载所有自定义样式
        m_styles.clear();
        auto styles = data["styles"].as_array();
        if (styles) {
            for (auto& styleNode : *styles) {
                auto styleObj = styleNode.as_table();
                if (!styleObj) continue;

                MarkdownStyle style;
                style.id = styleObj->get("id")->as_integer()->get();

                // 前景色
                auto fg = styleObj->get("foreground")->as_array();
                if (fg && fg->size() == 3) {
                    style.foreground = wxColour((*fg)[0].as_integer()->get(),
                        (*fg)[1].as_integer()->get(),
                        (*fg)[2].as_integer()->get());
                }

                // 背景色
                auto bg = styleObj->get("background")->as_array();
                if (bg && bg->size() == 3) {
                    style.background = wxColour((*bg)[0].as_integer()->get(),
                        (*bg)[1].as_integer()->get(),
                        (*bg)[2].as_integer()->get());
                }

                // 字体属性
                if (styleObj->get("bold")) {
                    style.bold = styleObj->get("bold")->as_boolean()->get();
                }
                if (styleObj->get("italic")) {
                    style.italic = styleObj->get("italic")->as_boolean()->get();
                }
                if (styleObj->get("underline")) {
                    style.underline = styleObj->get("underline")->as_boolean()->get();
                }
                if (styleObj->get("fontSize")) {
                    style.fontSize = styleObj->get("fontSize")->as_integer()->get();
                }
                if (styleObj->get("fontName")) {
                    style.fontName = wxString::FromUTF8(
                        styleObj->get("fontName")->as_string()->get());
                }

                m_styles.push_back(style);
            }
        }

        // 应用所有样式（使用原有的代码）
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

        return true;

    }
    catch (const toml::parse_error& err) {
        wxLogError("TOML解析错误: %s", err.what());
        InitializeStyles();
        return false;
    }
    catch (const std::exception& err) {
        wxLogError("加载主题失败: %s", err.what());
        InitializeStyles();
        return false;
    }
}

// 实现方法
bool MarkdownTextCtrl::load_styles(const std::string& tomlPath)
{
    try
    {
        if (m_vfs && !tomlPath.empty())
        {
            auto txt = m_vfs->get_string(tomlPath);
            if (!txt.empty())
            {
                set_styles(txt);
                return true;

            }
        }
        return false;
    }
    catch (const std::exception& err) {
        wxLogError("load failed: %s", err.what());
        InitializeStyles();
        return false;
    }

}
void MarkdownTextCtrl::set_text(const std::string& text)
{

    m_text = wxString::FromUTF8(text);
    SetValue(m_text);
    highlight_markdown();
}
std::string MarkdownTextCtrl::get_text()
{

    return std::string(GetValue().ToUTF8());
}
void MarkdownTextCtrl::set_vfs(std::shared_ptr<VirtualFileSystem>& vfs)
{
    m_vfs = vfs;
}
std::shared_ptr<VirtualFileSystem> MarkdownTextCtrl::get_vfs()
{
    return m_vfs;
}
void MarkdownTextCtrl::InitializeLightTheme()
{
    m_styles.clear();
    StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColour(34, 34, 34));
    StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColour(250, 250, 250));


    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(34, 34, 34));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(250, 250, 250));
    SetCaretForeground(wxColour(0, 0, 0));  // 白色光标
    SetMarginBackground(1, wxColour(250, 250, 250));
    SetMarginBackground(0, wxColour(250, 250, 250)); // 暗色模式
    SetFoldMarginColour(true, wxColour(250, 250, 250));
    SetFoldMarginHiColour(true, wxColour(255, 255, 255));

    SetEdgeColour(wxColour(250, 250, 250));

    SetCaretLineBackground(wxColour(240, 240, 240)); // 浅灰当前行
    SetCaretWidth(2);





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

    // 脚注引用标记（正文中的[^1]）
    style.id = STYLE_FOOTNOTE_REFERENCE;
    style.foreground = wxColour(196, 26, 22);       // 醒目的红色，与正文区分
    style.background = wxColour(250, 250, 250);
    style.bold = true;                              // 加粗显示
    style.italic = false;
    style.fontSize = 9;                             // 比正文稍小，作为上标
    style.underline = false;
    m_styles.push_back(style);

    // 脚注内容文本
    style.id = STYLE_FOOTNOTE_DEFINITION;
    style.foreground = wxColour(102, 102, 102);     // 浅灰色，与正文区分
    style.background = wxColour(253, 252, 248);     // 淡米色背景，区分正文区域
    style.bold = false;
    style.italic = true;                            // 斜体表示引用性质
    style.fontSize = 10;                            // 比正文稍小
    style.underline = false;
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
}
void MarkdownTextCtrl::InitializeDarkTheme()
{

    // 创建暗色模式样式集合
    StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColour(30, 30, 30));
    StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColour(220, 220, 220));


    // STC_STYLE_LINENUMBER 是行号样式的索引
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(30, 30, 30)); // 深灰色背景
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(200, 200, 220)); // 浅灰色文字
    SetCaretForeground(wxColour(255, 255, 255));  // 白色光标
    
    SetMarginBackground(1, wxColour(30, 30, 30));
    SetMarginBackground(0, wxColour(40, 42, 48)); // 暗色模式
    SetFoldMarginColour(true, wxColour(30, 30, 30));
    SetFoldMarginHiColour(true, wxColour(40, 40, 40));
    SetEdgeColour(wxColour(40, 42, 48));
    SetCaretLineBackground( wxColour(45, 50, 55) ); // 浅灰当前行
    SetCaretWidth(2);

    m_styles.clear();

    // 标题样式
    MarkdownStyle style;

    // 基础正文 - 浅灰文字，深灰背景
    style.id = STYLE_NORMAL;
    style.foreground = wxColour(220, 220, 220);     // 浅灰色文字，减轻眼睛疲劳
    style.background = wxColour(30, 30, 30);        // 深灰蓝背景，比纯黑柔和
    style.bold = false;
    style.italic = false;
    style.fontSize = 11;
    style.underline = false;
    style.fontName = "Microsoft YaHei";
    m_styles.push_back(style);

    // 标题1 - 冰蓝色渐变
    style.id = STYLE_HEADER1;
    style.foreground = wxColour(100, 180, 255);     // 冰蓝色，醒目但不刺眼
    style.background = wxColour(30, 30, 35);
    style.fontSize = 24;
    style.bold = true;
    m_styles.push_back(style);

    // 标题2
    style.id = STYLE_HEADER2;
    style.foreground = wxColour(120, 195, 255);     // 稍浅的冰蓝
    style.fontSize = 20;
    m_styles.push_back(style);

    // 标题3
    style.id = STYLE_HEADER3;
    style.foreground = wxColour(140, 210, 255);     // 更浅的冰蓝
    style.fontSize = 18;
    m_styles.push_back(style);

    // 标题4
    style.id = STYLE_HEADER4;
    style.foreground = wxColour(160, 220, 255);     // 淡冰蓝
    style.fontSize = 16;
    m_styles.push_back(style);

    // 标题5
    style.id = STYLE_HEADER5;
    style.foreground = wxColour(180, 230, 255);     // 很淡的冰蓝
    style.fontSize = 14;
    m_styles.push_back(style);

    // 标题6
    style.id = STYLE_HEADER6;
    style.foreground = wxColour(200, 240, 255);     // 几乎白色的冰蓝
    style.fontSize = 13;
    m_styles.push_back(style);

    // 代码样式（行内代码）- 亮珊瑚色
    style.id = STYLE_CODE;
    style.foreground = wxColour(255, 120, 120);     // 亮珊瑚色
    style.background = wxColour(45, 45, 50);        // 比背景稍亮
    style.fontSize = 0;
    style.bold = false;
    style.fontName = "Consolas";
    m_styles.push_back(style);

    // 代码块样式 - 薄荷绿
    style.id = STYLE_CODEBLOCK;
    style.foreground = wxColour(160, 240, 160);     // 薄荷绿，护眼
    style.background = wxColour(40, 40, 45);        // 深灰背景
    style.fontSize = 0;
    style.bold = false;
    style.fontName = "Consolas";
    m_styles.push_back(style);

    // 链接样式 - 淡天蓝色
    style.id = STYLE_LINK;
    style.foreground = wxColour(100, 180, 255);     // 天蓝色，与标题1呼应
    style.background = wxColour(30, 30, 35);
    style.underline = true;
    m_styles.push_back(style);

    // 强调样式（斜体）- 浅米色
    style.id = STYLE_EMPHASIS;
    style.foreground = wxColour(230, 220, 200);     // 浅米色斜体
    style.background = wxColour(30, 30, 35);
    style.italic = true;
    style.underline = false;
    m_styles.push_back(style);

    // 粗体样式 - 亮白色
    style.id = STYLE_STRONG;
    style.foreground = wxColour(255, 255, 255);     // 纯白粗体，强调
    style.background = wxColour(30, 30, 35);
    style.bold = true;
    style.italic = false;
    m_styles.push_back(style);

    // 引用样式 - 深灰蓝
    style.id = STYLE_BLOCKQUOTE;
    style.foreground = wxColour(180, 195, 210);     // 灰蓝色文字
    style.background = wxColour(30, 30, 30);        // 稍亮的深灰蓝背景
    style.bold = false;
    style.italic = true;
    m_styles.push_back(style);

    // 列表样式 - 淡紫色
    style.id = STYLE_LIST;
    style.foreground = wxColour(200, 160, 255);     // 淡紫色，醒目
    style.background = wxColour(30, 30, 35);
    style.italic = false;
    m_styles.push_back(style);

    // 分隔线样式 - 深灰
    style.id = STYLE_HR;
    style.foreground = wxColour(80, 85, 90);        // 深灰分隔线
    style.background = wxColour(30, 30, 35);
    style.bold = false;
    m_styles.push_back(style);

    // 删除线样式 - 中灰
    style.id = STYLE_STRIKETHROUGH;
    style.foreground = wxColour(140, 140, 140);     // 中灰色删除线
    style.background = wxColour(30, 30, 35);
    m_styles.push_back(style);

    // 脚注引用标记 - 亮橙色
    style.id = STYLE_FOOTNOTE_REFERENCE;
    style.foreground = wxColour(255, 160, 80);      // 亮橙色，在暗色中醒目
    style.background = wxColour(30, 30, 35);
    style.bold = true;
    style.italic = false;
    style.fontSize = 9;
    style.underline = false;
    m_styles.push_back(style);

    // 脚注内容文本 - 浅灰绿
    style.id = STYLE_FOOTNOTE_DEFINITION;
    style.foreground = wxColour(180, 220, 180);     // 浅灰绿色
    style.background = wxColour(30, 30, 30);        // 稍亮的背景
    style.bold = false;
    style.italic = true;
    style.fontSize = 10;
    style.underline = false;
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

}
void MarkdownTextCtrl::enable_live_highlighting(bool enable)
{
    m_liveHighlighting = enable;
    if (enable) {
        highlight_markdown();
    }
}

void MarkdownTextCtrl::highlight_markdown()
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
    if (currentText == m_text) {
        return;
    }
    m_text = currentText;
    highlight_markdown();
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

   
    //ApplyBaseStyle();

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
void MarkdownTextCtrl::SetLineBackground(int line, const wxColour& color) {
    // 方法1：使用标记
    static int highlightMarker = 10;

    MarkerDefine(highlightMarker, wxSTC_MARK_BACKGROUND);
    MarkerSetBackground(highlightMarker, color);
    MarkerSetForeground(highlightMarker, color);

    // 添加标记到指定行
    MarkerAdd(line, highlightMarker);
}
void MarkdownTextCtrl::ApplyNodeStyle(cmark_node* node, int start_pos, int end_pos)
{
    cmark_node_type node_type = cmark_node_get_type(node);

    switch (node_type) {

    case CMARK_NODE_CODE_BLOCK:
    {
        auto start = cmark_node_get_start_line(node);
        auto end = cmark_node_get_end_line(node);
        auto color = StyleGetBackground(STYLE_CODEBLOCK);
        for(int i=start; i<end; i++)
        {
            SetLineBackground(i, color);
        }
        SetStyleRange(start_pos, end_pos, STYLE_CODEBLOCK);
        break;
    }
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
    case CMARK_NODE_FOOTNOTE_DEFINITION:
        SetStyleRange(start_pos, end_pos, STYLE_FOOTNOTE_DEFINITION);
        break;
    case CMARK_NODE_FOOTNOTE_REFERENCE:
        SetStyleRange(start_pos, end_pos, STYLE_FOOTNOTE_REFERENCE);
        break;
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



