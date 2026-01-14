#pragma once
#include <string.h>

#include "HtmlWindow.h"


class MarkdownWindow : public HtmlWindow
{
public:
	MarkdownWindow(wxWindow* parent);
	~MarkdownWindow();

	bool set_markdown(const std::string& md);
	bool open_markdown(const std::string& path);


private:
	std::unique_ptr<wxTextCtrl> m_ctrl;

private:
	
	void OnDropFiles(wxDropFilesEvent& event);
	void OnKeyDown(wxKeyEvent& event);

	

	DECLARE_EVENT_TABLE();
};