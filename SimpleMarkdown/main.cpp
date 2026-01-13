
#include "MarkdownWindow.h"
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
    wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "Simple Markdown", wxDefaultPosition, wxSize(800, 600));



    MarkdownWindow* container = new MarkdownWindow(frame);
    frame->Show(true);

    //std::string html = "<html><body><h1>Hello, World!</h1><p>This is a simple HTML page rendered using LiteHtml and wxWidgets.</p></body></html>";
    //container->set_html(html);
    container->EnableDragAndDrop(true);
    container->load_user_css("./resources/markdown.css");

    fs::path filePath("./test.md");
    std::string ext = filePath.extension().generic_string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

    std::string html = "";
    if (ext == ".md")
    {
        container->open_markdown(filePath.generic_string());
    }
    else
    {
        container->open_html(filePath.generic_string());
    }

    

    return true;
}

wxIMPLEMENT_APP(MyApp);