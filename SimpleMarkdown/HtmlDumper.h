#pragma once

#include <litehtml\document.h>
#include <string>


class HtmlDumper: public litehtml::dumper
{
public:
	~HtmlDumper() {}
	void begin_node(const std::string& descr) override;
	void end_node() override;
	void begin_attrs_group(const std::string& descr);
	void end_attrs_group();
	void add_attr(const std::string& name, const std::string& value);
	std::string get_html();
private:
	std::string m_html = "";
	uint32_t m_pos = 0;
	std::vector<std::string> m_node_stack = {};
	
};	
