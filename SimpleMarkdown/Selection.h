#pragma once

#include <string>

#include <litehtml.h>


class Selection
{
public:
    Selection();
    void load(litehtml::document::ptr doc);
    void on_lbutton_down(float x, float y);
    void on_lbutton_up(float x, float y);
    void on_mouse_move(float x, float y);

    void clear();

    litehtml::position::vector get_rect();
    std::string get_text();
private:

    std::vector<litehtml::position> m_rect;
    std::string m_text;
    litehtml::document::ptr m_doc;
    litehtml::element::ptr m_start_el;
    litehtml::element::ptr m_end_el;
    bool m_start_selection;
  
private:
    void add_rect(litehtml::position pos);
    litehtml::element::ptr get_text_element(float x, float y);
    litehtml::element::ptr get_text_element_recursive(const litehtml::element::ptr& el, float x, float y) const;
    litehtml::element::ptr get_nearest_text_element(float x, float y);
    litehtml::element::ptr get_nearest_text_element_recursive(litehtml::element::ptr el, float x, float y);
    litehtml::position get_char_rect(float x, float y);
    litehtml::position get_char_rect_recursive(litehtml::element::ptr el);
    float get_distance(litehtml::element::ptr el, float x, float y);
};