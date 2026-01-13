
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
    wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "Simple Markdown", wxDefaultPosition, wxSize(800, 600));

    HtmlWindow* container = new HtmlWindow(frame);
    frame->Show(true);

    //std::string html = "<html><body><h1>Hello, World!</h1><p>This is a simple HTML page rendered using LiteHtml and wxWidgets.</p></body></html>";
    //container->set_html(html);
    container->EnableDragAndDrop(true);
    container->load_user_css("./resources/markdown.css");
    container->open_html("./test.md");

    return true;
}

wxIMPLEMENT_APP(MyApp);