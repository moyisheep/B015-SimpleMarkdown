#pragma once

#include <wx/wx.h>

#include "MarkdownWindow.h"
#include "LocalVFS.h"

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

    wxString GetExecutablePath();

    bool set_markdown(const std::string& md);
    bool set_html(const std::string& html);
    bool is_supported_format(std::string format);
    bool is_markdown_format(std::string format);
    bool is_html_format(std::string format);
    bool load_markdown(const std::string& path);
    bool set_user_css(const std::string& css);
    bool load_user_css(const std::string& path);
    void enable_drag_drop(bool enable = true);
private:
    void OnDropFiles(wxDropFilesEvent& event);
    void ToggleEditMode();
    void OnToggleEditMode(wxEvent& event);
    void OnSize(wxSizeEvent& event);
private:
	std::unique_ptr<MarkdownWindow> m_view_wnd;
    std::unique_ptr<MarkdownTextCtrl> m_edit_wnd;
    MarkdownMode m_mode;
    std::string m_text = "";
    std::shared_ptr<VirtualFileSystem> m_vfs;
    std::string m_exe_dir = "";



};