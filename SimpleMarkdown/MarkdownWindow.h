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

	std::string get_markdown();

private:
	std::string  m_markdown_text;
private:
	
	void OnDropFiles(wxDropFilesEvent& event);


	

	DECLARE_EVENT_TABLE();
};