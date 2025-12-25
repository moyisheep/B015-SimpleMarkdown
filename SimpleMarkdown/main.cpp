
#include "wxContainer.h"
#include <wx/app.h>
#include <wx/frame.h>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

bool MyApp::OnInit()
{
    wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "LiteHtml with wxWidgets", wxDefaultPosition, wxSize(800, 600));
    wxContainer* container = new wxContainer(frame);
    frame->Show(true);

    std::string html = "<html><body><h1>Hello, World!</h1><p>This is a simple HTML page rendered using LiteHtml and wxWidgets.</p></body></html>";
    container->set_html(html);

    return true;
}

wxIMPLEMENT_APP(MyApp);