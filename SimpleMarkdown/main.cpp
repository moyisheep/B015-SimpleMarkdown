
#include "MarkdownFrame.h"
#include <wx/app.h>
#include <wx/frame.h>
#include <filesystem>
#include <wx/sysopt.h>

namespace fs = std::filesystem;

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class DebugFrame : public wxFrame
{
public:
    DebugFrame() : wxFrame(nullptr, wxID_ANY, "Debug Window")
    {
        // 创建日志显示控件
        m_logCtrl = new wxTextCtrl(this, wxID_ANY, "",
            wxDefaultPosition, wxSize(600, 400),
            wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);

        // 设置 wxLog 到 TextCtrl
        wxLogTextCtrl* logger = new wxLogTextCtrl(m_logCtrl);
        wxLog::SetActiveTarget(logger);

        // 启动重定向
        startCoutRedirect();

        // 测试
        wxLogMessage("程序启动");
        std::cout << "标准输出测试" << std::endl;
        std::cerr << "标准错误测试" << std::endl;

        // 添加清除按钮
        wxButton* clearBtn = new wxButton(this, wxID_ANY, "清除日志");
        clearBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
            m_logCtrl->Clear();
            });

        // 布局
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(m_logCtrl, 1, wxEXPAND | wxALL, 5);
        sizer->Add(clearBtn, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
        SetSizer(sizer);

        // 启动定时器处理缓冲输出
        m_timer.Bind(wxEVT_TIMER, &DebugFrame::OnTimer, this);
        m_timer.Start(50); // 20次/秒
    }

    ~DebugFrame()
    {
        // 恢复标准输出
        if (m_oldCoutBuf)
            std::cout.rdbuf(m_oldCoutBuf);
        if (m_oldCerrBuf)
            std::cerr.rdbuf(m_oldCerrBuf);
    }

private:
    void startCoutRedirect()
    {
        // 保存原来的缓冲区
        m_oldCoutBuf = std::cout.rdbuf();
        m_oldCerrBuf = std::cerr.rdbuf();

        // 重定向到字符串流
        std::cout.rdbuf(m_coutStream.rdbuf());
        std::cerr.rdbuf(m_cerrStream.rdbuf());
    }

    void OnTimer(wxTimerEvent&)
    {
        // 检查 cout 缓冲区
        processStream(m_coutStream, "COUT");

        // 检查 cerr 缓冲区
        processStream(m_cerrStream, "CERR");
    }

    void processStream(std::ostringstream& stream, const wxString& prefix)
    {
        wxString content = stream.str();
        if (!content.empty())
        {
            // 分割为行
            wxArrayString lines = wxSplit(content, '\n', '\0');
            for (const wxString& line : lines)
            {
                if (!line.empty())
                {
                    // 可以选择添加前缀来区分来源
                    wxString output = line;
                    if (prefix == "CERR")
                        wxLogInfo("[stderr] %s", output);
                    else
                        wxLogMessage("[stdout] %s", output);
                }
            }
            // 清空流
            stream.str("");
        }
    }

private:
    wxTextCtrl* m_logCtrl;
    std::streambuf* m_oldCoutBuf = nullptr;
    std::streambuf* m_oldCerrBuf = nullptr;
    std::ostringstream m_coutStream;
    std::ostringstream m_cerrStream;
    wxTimer m_timer;
};

// 1. 创建一个空的日志目标
class NullLogTarget : public wxLog
{
public:
    // 重写所有日志输出方法为空实现
    virtual void DoLogText(const wxString& WXUNUSED(msg)) override
    {
        // 什么都不做
    }

    virtual void DoLogTextAtLevel(wxLogLevel level, const wxString& msg) override
    {

    }

    virtual void DoLogRecord(wxLogLevel level,
        const wxString& msg,
        const wxLogRecordInfo& info) override
    {

    }
};

bool MyApp::OnInit()
{
    wxImage::AddHandler(new wxPNGHandler);

    SetAppearance(Appearance::Dark);

    //auto* dbg_frame = new DebugFrame();
    //dbg_frame->Show(true);
    wxLog::SetActiveTarget(new NullLogTarget);




    MarkdownFrame* frame = new MarkdownFrame(nullptr, wxID_ANY, "Simple Markdown", wxDefaultPosition, wxSize(800, 600));
    frame->enable_drag_drop(true);

    wxIcon icon;
    if (icon.LoadFile("./resources/app.png", wxBITMAP_TYPE_PNG))
    {
        frame->SetIcon(icon);
    }
    wxArrayString args = wxAppConsole::argv.GetArguments();
    if (args.GetCount() > 1)
    {
        fs::path filePath(args[1].ToStdString());
        frame->load_markdown(filePath.generic_string());
    }


    frame->Show(true);
    frame->SetFocus();

    //std::string html = "<html><body><h1>Hello, World!</h1><p>This is a simple HTML page rendered using LiteHtml and wxWidgets.</p></body></html>";
    //container->set_html(html);
    








    return true;
}

wxIMPLEMENT_APP(MyApp);