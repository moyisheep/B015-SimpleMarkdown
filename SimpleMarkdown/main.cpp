
#include "HtmlWindow.h"
#include <wx/app.h>
#include <wx/frame.h>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class DebugFrame : public wxFrame {
public:
    DebugFrame() : wxFrame(nullptr, wxID_ANY, "调试窗口") {
        // 创建文本控件用于显示日志
        wxTextCtrl* logCtrl = new wxTextCtrl(this, wxID_ANY, "",
            wxDefaultPosition, wxDefaultSize,
            wxTE_MULTILINE | wxTE_READONLY);

        // 设置日志输出到文本控件
        wxLogTextCtrl* logger = new wxLogTextCtrl(logCtrl);
        wxLog::SetActiveTarget(logger);

        // 测试日志输出
        wxLogMessage("程序启动");
        wxLogDebug("调试信息");
    }
};

bool MyApp::OnInit()
{
    auto* dbg_frame = new DebugFrame();
    dbg_frame->Show(true);
    wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "LiteHtml with wxWidgets", wxDefaultPosition, wxSize(800, 600));

    HtmlWindow* container = new HtmlWindow(frame);
    frame->Show(true);

    //std::string html = "<html><body><h1>Hello, World!</h1><p>This is a simple HTML page rendered using LiteHtml and wxWidgets.</p></body></html>";
    //container->set_html(html);
    container->EnableDragAndDrop(true);
    container->open_html("./test.md");

    return true;
}

wxIMPLEMENT_APP(MyApp);