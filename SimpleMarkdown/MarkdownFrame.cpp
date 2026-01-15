#include "MarkdownFrame.h"
#include "MarkdownTextCtrl.h"

static const int ID_TOGGLE_EDIT_MODE = wxNewId();

MarkdownFrame::MarkdownFrame(wxWindow* parent, 
	wxWindowID id, const 
	wxString& title, 
	const wxPoint& pos, 
	const wxSize& size, 
	long style, 
	const wxString& name)
	:wxFrame(parent, id, title, pos, size, style, name)
{
	m_view_wnd = std::make_unique<MarkdownWindow>(this);
	m_edit_wnd = std::make_unique<MarkdownTextCtrl>(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	m_edit_wnd->Hide();
	m_edit_wnd->EnableLiveHighlighting(true);
	m_view_wnd->Show();



	m_mode = MarkdownMode::view;
	//m_edit_wnd->SetBackgroundColour(wxColour("black"));
	//m_view_wnd->MoveAfterInTabOrder(m_edit_wnd.get());

	
	// 创建加速器表
	wxAcceleratorEntry entries[1];
	entries[0].Set(wxACCEL_NORMAL, WXK_F2, ID_TOGGLE_EDIT_MODE);

	wxAcceleratorTable accel(1, entries);
	SetAcceleratorTable(accel);

	// 绑定菜单/按钮事件
	Bind(wxEVT_MENU, &MarkdownFrame::OnToggleEditMode, this, ID_TOGGLE_EDIT_MODE);

}

MarkdownFrame::~MarkdownFrame()
{
}

bool MarkdownFrame::set_markdown(const std::string& md)
{
	if(m_view_wnd)
	{

		return m_view_wnd->set_markdown(md);
	}
	return false;
}

bool MarkdownFrame::open_markdown(const std::string& path)
{
	if (m_view_wnd)
	{
		return m_view_wnd->open_markdown(path);
	}
	return false;
	
}

bool MarkdownFrame::set_user_css(const std::string& css)
{
	if (m_view_wnd)
	{
		return m_view_wnd->set_user_css(css);
	}
	return false;
}

bool MarkdownFrame::load_user_css(const std::string& path)
{
	if (m_view_wnd)
	{
		return m_view_wnd->load_user_css(path);
	}
	return false;
}

void MarkdownFrame::enable_drag_drop(bool enable)
{
	if (enable)
	{
		DragAcceptFiles(true);
	}
	else
	{
		DragAcceptFiles(false);
	}
}

void MarkdownFrame::OnDropFiles(wxDropFilesEvent& event)
{
	if (event.GetNumberOfFiles() > 0)
	{
		wxString* dropped = event.GetFiles();
		wxString file_path = dropped[0];


		if (!open_markdown(file_path.ToStdString()))
		{
			wxMessageBox("Failed to load Markdown file", "Error", wxICON_ERROR);
		}

	}
}

void MarkdownFrame::ToggleEditMode()
{
	if(m_mode == MarkdownMode::view)
	{
		m_mode = MarkdownMode::edit;
	}else
	{
		m_mode = MarkdownMode::view;
	}

	if(m_mode == MarkdownMode::view)
	{
		auto sz = GetClientSize();
		m_view_wnd->SetSize(sz);

		auto text = m_edit_wnd->GetValue();
		m_view_wnd->set_markdown(std::string(text.ToUTF8()));
		m_view_wnd->Show();
		m_edit_wnd->Hide();

		m_view_wnd->SetFocus();

	



	}
	if(m_mode == MarkdownMode::edit)
	{
	
		auto sz = GetClientSize();
		m_edit_wnd->SetSize(sz);
		//m_edit_wnd->SetLabelText(m_markdown_text);
		auto text = m_view_wnd->get_markdown();
		m_edit_wnd->SetValue(wxString::FromUTF8(text));
		m_edit_wnd->Show();
		m_view_wnd->Hide();
		m_edit_wnd->HighlightMarkdown();
		m_edit_wnd->SetFocus();


	
	}
}
void MarkdownFrame::OnToggleEditMode(wxEvent& event)
{
	ToggleEditMode();
}

void MarkdownFrame::OnSize(wxSizeEvent& event)
{
	auto sz = GetClientSize();
	if(m_mode == MarkdownMode::view)
	{
		m_view_wnd->SetSize(sz);
	}
	if(m_mode == MarkdownMode::edit)
	{
		m_edit_wnd->SetSize(sz);
	}
}
wxBEGIN_EVENT_TABLE(MarkdownFrame, wxFrame)
EVT_DROP_FILES(MarkdownFrame::OnDropFiles)
EVT_SIZE(MarkdownFrame::OnSize)

wxEND_EVENT_TABLE()