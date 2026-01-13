#include <litehtml.h>
#include <litehtml/render_item.h>
#include <wx/wx.h>
#include <string>
#include <sstream>
#include <iostream>

namespace debug
{
	static int render_count = 0;
	static int element_count = 0;

	void debug_print(std::string txt)
	{
		wxLogInfo(txt);
	}



	std::string to_raw_string(const std::string& str) {
		std::ostringstream oss;

		for (char c : str) {
			switch (c) {
			case '\n': oss << "\\n"; break;
			case '\t': oss << "\\t"; break;
			case '\r': oss << "\\r"; break;
			case '\b': oss << "\\b"; break;
			case '\f': oss << "\\f"; break;
			case '\v': oss << "\\v"; break;
			case '\a': oss << "\\a"; break;
			case '\\': oss << "\\\\"; break;
			case '\"': oss << "\\\""; break;
			case '\'': oss << "\\\'"; break;
			default:
				if (c >= 32 && c <= 126) {
					oss << c;
				}
				else {
					oss << "\\x" << std::hex << static_cast<int>(static_cast<unsigned char>(c));
				}
			}
		}

		return oss.str();
	}

	void print_render_tree_recursive(const std::shared_ptr<litehtml::render_item>& ri, int level)
	{

		auto el = ri->src_el();
		std::string tag_name = el->is_text() ? "el_text" : el->get_tagName();
		std::string content = "";
		el->get_text(content);


		std::stringstream txt;
		render_count += 1;
		txt << render_count;

		for (int i = 0; i < level * 2; i++)
	    {
			txt << "¡¤";
		}



		
		txt << "[" << level << "]"
			<< ri << " "
			<< el << " "
			<< " [" << tag_name << "]"
			<< " (" << to_raw_string(content) << ")" << "\n";
		debug_print(txt.str());
	

		for (auto& child_ri : ri->children())
		{
			print_render_tree_recursive(child_ri, level + 1);
		}

	}


	void print_render_tree(const std::shared_ptr<litehtml::render_item>& ri)
	{

		debug_print("========[start] Render Tree ==========\n");

		render_count = 0;

		std::stringstream txt;
		auto el = ri->src_el();
		std::string tag_name = el->is_text() ? "el_text" : el->get_tagName();
		std::string content = "";
		el->get_text(content);
		

		render_count += 1;
		txt << render_count << "[*]"
			<< ri << " "
			<< el << " "
			<< " [" << tag_name << "]"
			<< " (" << to_raw_string(content) << ")" << "\n";
		debug_print(txt.str());
	

		for (auto& child_ri : ri->children())
		{
			print_render_tree_recursive(child_ri, 1);
		}

		std::stringstream end_txt;
		end_txt << "[total]: " << render_count << "\n";
		debug_print(end_txt.str());
		debug_print("========[end] Render Tree ==========\n");

	}

	void print_element_tree_recursive(const litehtml::element::ptr& el, int level)
	{


		std::string tag_name = el->is_text() ? "el_text" : el->get_tagName();
		std::string content = "";
		el->get_text(content);


		element_count += 1;
	


		std::stringstream txt;
		txt << element_count;

		for (int i = 0; i < level * 2; i++)
		{
			txt << "¡¤";
		}


		txt << "[" << level << "]"
			<< el << " "
			<< " [" << tag_name << "]"
			<< " (" << to_raw_string(content) << ")" << "\n";
		debug_print(txt.str());
			



		for (auto& child : el->children())
		{
			print_element_tree_recursive(child, level + 1);
		}
	}
	void print_element_tree(const litehtml::element::ptr& el)
	{
		debug_print("========[start] Element Tree ==========\n");

		element_count = 0;


	
		std::string tag_name = el->is_text() ? "el_text" : el->get_tagName();
		std::string content = "";
		el->get_text(content);

		element_count += 1;

		std::stringstream txt;

		txt << element_count << "[*]"
			<< el << " "
			<< " [" << tag_name << "]"
			<< " (" << to_raw_string(content) << ")" << "\n";
		debug_print(txt.str());




		for (auto& child : el->children())
		{
			print_element_tree_recursive(child, 1);
		}

		std::stringstream end_txt;
		end_txt << "[total]: " << element_count << "\n";
		debug_print(end_txt.str());
		debug_print("========[end] Element Tree ==========\n");
	}

}


