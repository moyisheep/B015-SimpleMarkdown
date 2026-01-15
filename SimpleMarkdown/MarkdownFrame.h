#pragma once

#include <wx/wx.h>

#include "MarkdownWindow.h"

class MarkdownTextCtrl;
enum MarkdownMode{view, edit};

class MarkdownFrame: public wxFrame
{
public:
	MarkdownFrame(wxWindow* parent,
        wxWindowID id,
        const wxString& title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE,
        const wxString& name = wxASCII_STR(wxFrameNameStr));
	~MarkdownFrame();

    bool set_markdown(const std::string& md);
    bool open_markdown(const std::string& path);
    bool set_user_css(const std::string& css);
    bool load_user_css(const std::string& path);
    void enable_drag_drop(bool enable = true);
private:
    void OnDropFiles(wxDropFilesEvent& event);
    void ToggleEditMode();
    void OnToggleEditMode(wxEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnKeyDown(wxKeyEvent& event);
private:
	std::unique_ptr<MarkdownWindow> m_view_wnd;
    std::unique_ptr<MarkdownTextCtrl> m_edit_wnd;
    MarkdownMode m_mode;



};