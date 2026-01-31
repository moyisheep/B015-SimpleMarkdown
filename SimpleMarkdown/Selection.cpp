#include "Selection.h"
#include <litehtml/render_item.h>
#include <math.h>

Selection::Selection()
{
    clear();
}

void Selection::load(litehtml::document::ptr doc)
{
    m_doc = doc;

}

void Selection::on_lbutton_down(float x, float y)
{
    auto el = get_text_element(x, y);
    if(el)
    {
        m_start_selection = true;
        m_start_el = el;
        m_end_el = el;
    }
}

void Selection::on_lbutton_up(float x, float y)
{
    if (m_start_selection)
    {
        auto el = get_nearest_text_element(x, y);
        if (el)
        {
            m_end_el = el;
        }
    }
    m_start_selection = false;
}

void Selection::on_mouse_move(float x, float y)
{
    if(m_start_selection)
    {
        auto el = get_nearest_text_element(x, y);
        if(el)
        {
            m_end_el = el;
        }
    }
}

void Selection::clear()
{
    m_rect = {};
    m_text = "";
    m_doc = nullptr;
    m_start_el = nullptr;
    m_end_el = nullptr;
    m_start_selection = false;
}

litehtml::position::vector Selection::get_rect()
{
    return m_rect;
}

std::string Selection::get_text()
{
    return m_text;
}

void Selection::add_rect(litehtml::position pos)
{
    if (!m_rect.empty())
    {
        auto back = m_rect.back();
        if (back.y == pos.y)
        {
            m_rect.pop_back();
            back.width += pos.width;
            m_rect.push_back(back);
            return;
        }
    }

    m_rect.push_back(pos);
}
litehtml::element::ptr Selection::get_text_element(float x, float y)
{
    if(m_doc)
    {
        auto root_render = m_doc->root_render();
        if(root_render)
        {
            auto el = root_render->get_element_by_point(x, y, 0, 0, nullptr);
            if(el)
            {
                if(el->is_text())
                {
                    return el;
                }
                return get_text_element_recursive(el, x, y);
            }
        }
    }
   
    return nullptr;
}
litehtml::element::ptr Selection::get_text_element_recursive(const litehtml::element::ptr& el, float x, float y) const
{
    for(auto& child: el->children())
    {
        if(child)
        {
            if(child->get_placement().is_point_inside(x, y) && child->is_text())
            {
                return child;
            }
            return get_text_element_recursive(child, x, y);
        }
    }
    return nullptr;
}

litehtml::element::ptr Selection::get_nearest_text_element(float x, float y)
{
    litehtml::element::ptr min_el;

    auto root = m_doc->root();

    if (root)
    {
        float min_distance = get_distance(root, x, y);
        for (auto& child : root->children())
        {
            if (child)
            {
                if (child->is_text())
                {

                    auto distance = get_distance(child, x, y);
                    if(distance < min_distance)
                    {
                        min_distance = distance;
                        min_el = child;
                    }
                }
                else
                {
                    auto el = get_nearest_text_element_recursive(child, x, y);
                    auto distance = get_distance(el, x, y);
                    if (distance < min_distance)
                    {
                        min_distance = distance;
                        min_el = el;
                    }
                }
            }
        }
    }
    return min_el;
}

litehtml::element::ptr Selection::get_nearest_text_element_recursive(litehtml::element::ptr element, float x, float y)
{
    litehtml::element::ptr min_el = element;

    if (element)
    {
        float min_distance = get_distance(element, x, y);
        for (auto& child : element->children())
        {
            if (child)
            {
                if (child->is_text())
                {

                    auto distance = get_distance(child, x, y);
                    if (distance < min_distance)
                    {
                        min_distance = distance;
                        min_el = child;
                    }
                }
                else
                {
                    auto el = get_nearest_text_element_recursive(child, x, y);
                    auto distance = get_distance(el, x, y);
                    if (distance < min_distance)
                    {
                        min_distance = distance;
                        min_el = el;
                    }
                }
            }
        }
    }
    return min_el;
}


litehtml::position Selection::get_char_rect(float x, float y)
{
    if (m_doc)
    {
        auto root_render = m_doc->root_render();
        if (root_render)
        {
            auto el = root_render->get_element_by_point(x, y, 0, 0, nullptr);
            if (el)
            {
                return get_char_rect_recursive(el);
            }
        }
    }


    return litehtml::position{};
}

litehtml::position Selection::get_char_rect_recursive(litehtml::element::ptr el)
{
    //for (auto child : el->children())
    //{
    //    // if it's el_text node, start record
    //    if (child->is_text())
    //    {
    //        std::string txt = "";
    //        auto pos = child->get_placement();
    //        child->get_text(txt);
    //        std::u32string u32txt = (const char32_t*)litehtml::utf8_to_utf32(txt);

    //        auto hfont = child->parent()->css().get_font();
    //        float x = pos.left();

    //        // split every word -> character, and record
    //        for (auto c : u32txt)
    //        {
    //            std::string ch = litehtml::utf32_to_utf8(c);
    //         
    //            auto ch_width = m_container->text_width(, hfont);

    //            litehtml::position char_pos{ x, pos.y, ch_width, pos.height };
    //            add_rect(char_pos);
    //            x += ch_width;

    //        }

    //    }
    //    else
    //    {
    //        get_char_rect_recursive(child);
    //    }
    //}
}

float Selection::get_distance(litehtml::element::ptr el, float x, float y)
{
    auto pos = el->get_placement();
    return sqrt(pow((pos.x - x), 2.0f) + pow((pos.y - y), 2.0f));
}
