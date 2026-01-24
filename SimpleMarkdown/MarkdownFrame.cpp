#include "MarkdownFrame.h"
#include "MarkdownTextCtrl.h"

#include <set>
#include <wx/stdpaths.h>
#include <wx/statusbr.h>

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


	// 创建状态栏（一行，默认在窗口底部）
	CreateStatusBar();

	// 设置状态栏文本
	SetStatusText("就绪");


	
	// 设置状态栏字段数量
	SetStatusBarPane(-1); // 单个面板
	m_vfs = std::make_shared<LocalVFS>();

	m_view_wnd = std::make_unique<MarkdownWindow>(this);



	m_edit_wnd = std::make_unique<MarkdownTextCtrl>(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	m_edit_wnd->enable_live_highlighting(true);
	
	m_edit_wnd->set_vfs(m_vfs);
	m_view_wnd->set_vfs(m_vfs);
	
	m_edit_wnd->Hide();


	m_exe_dir = std::string(GetExecutablePath().ToUTF8());
	m_vfs->set_current_path(m_exe_dir);
	
	
	m_edit_wnd->load_styles("./resources/markdown-edit-dark-charcoal.toml");
	m_view_wnd->load_user_css("./resources/markdown-view-dark-charcoal.css");
	m_view_wnd->load_markdown("./resources/homepage.md");

    // fix loading white screen
	m_view_wnd->SetSize({0, 0});




	m_mode = MarkdownMode::view;
	m_text = "";


	
	// 创建加速器表
	wxAcceleratorEntry entries[1];
	entries[0].Set(wxACCEL_NORMAL, WXK_F2, ID_TOGGLE_EDIT_MODE);

	wxAcceleratorTable accel(1, entries);
	SetAcceleratorTable(accel);

	// 绑定菜单/按钮事件
	Bind(wxEVT_MENU, &MarkdownFrame::OnToggleEditMode, this, ID_TOGGLE_EDIT_MODE);
	Bind(wxEVT_DROP_FILES, &MarkdownFrame::OnDropFiles, this);
	Bind(wxEVT_SIZE, &MarkdownFrame::OnSize, this);

	
}

MarkdownFrame::~MarkdownFrame()
{
}
wxString MarkdownFrame::GetExecutablePath() {
	// 获取可执行文件完整路径
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();

	// 提取目录部分
	return wxPathOnly(exePath);
}
bool MarkdownFrame::set_markdown(const std::string& md)
{

		
		//m_text = std::string(md);
		if(m_mode == MarkdownMode::view && m_view_wnd)
		{
			if(m_view_wnd->set_markdown(md))
			{
				return true;
			}
		}

		if(m_mode == MarkdownMode::edit && m_edit_wnd)
		{
			m_edit_wnd->set_text(md);
			return true;
		}

	return false;
}
bool MarkdownFrame::set_html(const std::string& html)
{
	if (m_mode == MarkdownMode::view && m_view_wnd)
	{
		m_view_wnd->set_user_css(" ");
		m_view_wnd->set_html(html);
		return true;
	}

	if (m_mode == MarkdownMode::edit && m_edit_wnd)
	{
		m_edit_wnd->set_text(html);
		return true;
	}

	return false;
}

bool MarkdownFrame::is_supported_format(std::string format)
{
	return is_markdown_format(format) || is_html_format(format);
}

bool MarkdownFrame::is_markdown_format(std::string format)
{
	static std::set<std::string> markdown_format =
	{ ".md", ".markdown", ".txt"};
	return markdown_format.count(format) == 1;
}
bool MarkdownFrame::is_html_format(std::string format)
{
	static std::set<std::string> html_format =
	{ ".html", ".xhtml" };
	return html_format.count(format) == 1;
}
bool MarkdownFrame::load_markdown(const std::string& path)
{
	auto ext = m_vfs->get_extension(path);
	if (m_view_wnd && m_vfs && is_supported_format(ext))
	{

		auto parent_path = m_vfs->get_parent_path(path);
		if(!parent_path.empty())
		{
			m_vfs->set_current_path(parent_path);
		}
		auto bin = m_vfs->get_binary(path);
		if(!bin.empty())
		{

			auto txt = std::string(reinterpret_cast<char*> (bin.data()), bin.size());
			if (!txt.empty())
			{
				if(is_markdown_format(ext))
				{
					return set_markdown(txt);
				}
				else if(is_html_format(ext))
				{
					return set_html(txt);
				}
				
			}
		}
		else
		{
			return set_markdown("");
		}
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

		
		if (!load_markdown(file_path.ToStdString()))
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

	
		std::string text = m_edit_wnd->get_text();
		if (text != m_text)
		{
			m_text = text;
			m_view_wnd->set_markdown(text);
		}
			

		
		m_view_wnd->Show();
		m_edit_wnd->Hide();

		m_view_wnd->SetFocus();

		//wxLogInfo(wxString::FromUTF8(m_view_wnd->get_html()));



	}
	if(m_mode == MarkdownMode::edit)
	{
	
		auto sz = GetClientSize();
		m_edit_wnd->SetSize(sz);
	
		auto text = m_view_wnd->get_markdown();
		if(text != m_text)
		{
			m_text = text;
			m_edit_wnd->set_text(m_text);
		}

		m_edit_wnd->Show();
		m_view_wnd->Hide();
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
	else if(m_mode == MarkdownMode::edit)
	{
		m_edit_wnd->SetSize(sz);
	}
}
