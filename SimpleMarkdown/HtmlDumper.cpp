#include "HtmlDumper.h"

void HtmlDumper::begin_node(const std::string& descr)
{
	m_html += "[begin_node] " + descr + "\n";

}

void HtmlDumper::end_node()
{
	m_html += "[end_node] \n";
}

void HtmlDumper::begin_attrs_group(const std::string& descr)
{
	m_html += "[begin_attrs_group] " + descr + "\n";
}

void HtmlDumper::end_attrs_group()
{
	m_html += "[end_attrs_group] \n";
}

void HtmlDumper::add_attr(const std::string& name, const std::string& value)
{
	m_html += "[add_attr] " + name + "=" + value + "\n";
}

std::string HtmlDumper::get_html()
{
	return m_html;
}
