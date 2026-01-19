
#include "MarkdownFrame.h"
#include <wx/app.h>
#include <wx/frame.h>
#include <filesystem>

namespace fs = std::filesystem;

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class DebugFrame : public wxFrame {
public:
    DebugFrame() : wxFrame(nullptr, wxID_ANY, "debug window") {
        // display log
        wxTextCtrl* logCtrl = new wxTextCtrl(this, wxID_ANY, "",
            wxDefaultPosition, wxDefaultSize,
            wxTE_MULTILINE | wxTE_READONLY);

        
        wxLogTextCtrl* logger = new wxLogTextCtrl(logCtrl);
        wxLog::SetActiveTarget(logger);

        // test
        wxLogMessage("program start");
        wxLogDebug("debug information");
    }
};

bool MyApp::OnInit()
{
    auto* dbg_frame = new DebugFrame();
    dbg_frame->Show(true);
    MarkdownFrame* frame = new MarkdownFrame(nullptr, wxID_ANY, "Simple Markdown", wxDefaultPosition, wxSize(800, 600));
    frame->enable_drag_drop(true);
 

    wxArrayString args = wxAppConsole::argv.GetArguments();
    if (args.GetCount() > 1)
    {
        fs::path filePath(args[1].ToStdString());

        frame->load_markdown(filePath.generic_string());
    }


    frame->Show(true);

    //std::string html = "<html><body><h1>Hello, World!</h1><p>This is a simple HTML page rendered using LiteHtml and wxWidgets.</p></body></html>";
    //container->set_html(html);
    








    return true;
}

wxIMPLEMENT_APP(MyApp);