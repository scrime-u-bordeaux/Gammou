#ifndef DIRECTORY_VIEW_H_
#define DIRECTORY_VIEW_H_

#include <map>
#include <memory>
#include <variant>
#include <string>
#include <functional>

#include "directory_model.h"
#include "panel.h"

namespace Gammou {

    namespace View {

        template<class Value>
        class directory_view : public widget {

            public:
                using model = directory_model<std::string, Value>;
                using directory_view_callback =
                    std::function<void(directory_view<Value>&, const std::string& key, const Value&)>;


                directory_view(
                    model& m,
                    const int x,
                    const int y,
                    const unsigned int width,
                    const unsigned int height,
                    const unsigned int displayed_cell_count,
                    const color selected_cell_color = cl_lightgrey,
                    const color hovered_cell_color = cl_lemonchiffon,
                    const color background = cl_white,
                    const color font_color = cl_black,
                    const unsigned int font_size = 11);
                ~directory_view();

                bool on_mouse_dbl_click(const int x, const int y) override;
                bool on_mouse_button_up(const mouse_button button, const int x, const int y) override;
                bool on_mouse_drag_end(const mouse_button button, const int x, const int y) override;

                bool on_mouse_wheel(const float distance) override;
                bool on_mouse_exit(void) override;
                bool on_mouse_move(const int x, const int y) override;

                void draw(cairo_t *cr) override;

                void set_value_select_event(directory_view_callback callback);
                void set_value_open_event(directory_view_callback callback);
            private:
                void scroll(const int distance);
                int cell_by_pos(const int y);
                void cell_select(
                    const unsigned int cell, bool dblclick);

                void draw_cell(
                    cairo_t *cr,
                    const typename model::node& node,
                    const unsigned int cell,
                    const unsigned int level);
                bool draw_directory(
                    cairo_t *cr,
                    model& m,
                    unsigned int& cell,
                    const unsigned int level);

                bool is_open(const typename model::directory& n) const;

                model& m_model;

                const color m_selected_color;
                const color m_hovered_color;
                const color m_background_color;
                const color m_font_color;

                const unsigned int m_font_size;
                const unsigned int m_displayed_cell_count;

                static const float epsilon;
                static const float subdirectory_offset;
                static const float arrow_size_scale;
                const float m_cell_width;
                const float m_cell_height;

                int m_selected_cell{-1};
                int m_hovered_cell{-1};
                int m_start_offset{0};

                std::vector<const typename model::node*> m_nodes;   //  updated by draw
                std::map<const typename model::directory*, bool> m_node_is_open;

                directory_view_callback m_value_select_callback;
                directory_view_callback m_value_open_callback;
        };


        //--------------------------------------------------------------------------------------

        //  Drawing constants

        template<class Value>
        const float directory_view<Value>::epsilon = 2.0f;

        template<class Value>
        const float directory_view<Value>::subdirectory_offset = 16.0f;

        template<class Value>
        const float directory_view<Value>::arrow_size_scale = 0.3f;

        //  Directory view implementation

        template<class Value>
        directory_view<Value>::directory_view(
            model& m,
            const int x,
            const int y,
            const unsigned int width,
            const unsigned int height,
            const unsigned int displayed_cell_count,
            const color selected_cell_color,
            const color hovered_cell_color,
            const color background,
            const color font_color,
            const unsigned int font_size)
        :   widget(x, y, width, height),
            m_model(m),
            m_selected_color(selected_cell_color),
            m_hovered_color(hovered_cell_color),
            m_background_color(background),
            m_font_color(font_color),
            m_font_size(font_size),
            m_displayed_cell_count(displayed_cell_count),
            m_cell_height(
              (static_cast<float>(get_height()) - 2.0f * epsilon) /
              static_cast<float>(displayed_cell_count)),
            m_cell_width(
                static_cast<float>(get_width() - 2.0f * epsilon)),
            m_nodes(displayed_cell_count)
        {}

        template<class Value>
        directory_view<Value>::~directory_view()
        {}

        template<class Value>
        bool directory_view<Value>::on_mouse_wheel(const float distance)
        {
            scroll(static_cast<int>(distance));
            return true;
        }

        template<class Value>
        bool directory_view<Value>::on_mouse_exit(void)
        {
            if (m_hovered_cell != -1) {
                m_hovered_cell = -1;
                redraw();
            }
            return true;
        }

        template<class Value>
        bool directory_view<Value>::on_mouse_dbl_click(const int x, const int y)
        {
            const int cell = cell_by_pos(y);
            if (cell >= 0)
                cell_select(static_cast<unsigned int>(cell), true);
            return true;
        }

        template<class Value>
        bool directory_view<Value>::on_mouse_button_up(const mouse_button button, const int x, const int y)
        {
            if (button == mouse_button::LeftButton) {
                const int cell = cell_by_pos(y);
                if (cell >= 0)
                    cell_select(static_cast<unsigned int>(cell), false);
                return true;
            }
            return false;
        }

        template<class Value>
        bool directory_view<Value>::on_mouse_drag_end(const mouse_button button, const int x, const int y)
        {
            if (button == mouse_button::LeftButton) {
                const int cell = cell_by_pos(y);
                if (cell >= 0)
                    cell_select(static_cast<unsigned int>(cell), false);
            }
            return false;
        }

        template<class Value>
        bool directory_view<Value>::on_mouse_move(const int x, const int y)
        {
            const int cell = cell_by_pos(y);

            if (cell != -1 && cell != m_hovered_cell) {
                m_hovered_cell = cell;
                redraw();
            }

            return true;
        }

        template<class Value>
        void directory_view<Value>::draw(cairo_t *cr)
        {
            // Background
            cairo_rectangle(
                cr, 0, 0,
                static_cast<double>(get_width()),
                static_cast<double>(get_height()));
            cairo_helper::set_source_color(cr, m_background_color);
            cairo_fill(cr);

            //  Cells
            unsigned int cell = 0;
            draw_directory(cr, m_model, cell, 0);
        }

        template<class Value>
        void directory_view<Value>::set_value_select_event(directory_view_callback callback)
        {
            m_value_select_callback = callback;
        }

        template<class Value>
        void directory_view<Value>::set_value_open_event(directory_view_callback callback)
        {
            m_value_open_callback = callback;
        }

        template<class Value>
        void directory_view<Value>::scroll(const int distance)
        {
            const int n = static_cast<int>(m_start_offset) - distance;

            if (n < 0)
                m_start_offset = 0;
            else
                m_start_offset = n;

            redraw();
        }

        template<class Value>
        int directory_view<Value>::cell_by_pos(const int y)
        {
            const int n =
                static_cast<int>(
                    static_cast<float>(y - epsilon) /
                    m_cell_height);

            if (n >= 0 &&
                static_cast<unsigned int>(n) < m_displayed_cell_count)
                    return n + m_start_offset;
            else
                return -1;
        }

        template<class Value>
        void directory_view<Value>::cell_select(
                const unsigned int cell,
                const bool dbl_click)
        {
            const auto n = cell - m_start_offset;
            const auto ptr = m_nodes[n];
            if( ptr == nullptr)
                return;

            const auto& node = *ptr;

            if (m_model.is_directory(node)) {
                const auto& dir =
                    std::get<typename model::directory>(node.second);
                m_node_is_open[&dir] = !(is_open(dir));
            }
            else {
                const Value& v =
                    std::get<Value>(node.second);

                if (dbl_click) {
                    if(m_value_open_callback)
                        m_value_open_callback(*this, node.first, v);
                }
                else if (m_value_select_callback)
                    m_value_select_callback(*this, node.first, v);
            }

            m_selected_cell = cell;
            redraw();
        }

        template<class Value>
        void directory_view<Value>::draw_cell(
            cairo_t *cr,
            const typename model::node& node,
            const unsigned int cell,
            const unsigned int level)
        {
            const unsigned int n = cell - m_start_offset;
            const int top = static_cast<int>(epsilon + m_cell_height * n);
            const float offset = subdirectory_offset * level;
            const int left = static_cast<int>(epsilon + offset);
            const int width = static_cast<int>(m_cell_width - offset);

            //  cell background

            if (cell == m_selected_cell)
                cairo_helper::set_source_color(cr, m_selected_color);
            else if (cell == m_hovered_cell)
                cairo_helper::set_source_color(cr, m_hovered_color);
            else
                cairo_helper::set_source_color(cr, m_background_color);

            cairo_rectangle(
                cr, epsilon, top, m_cell_width, m_cell_height);
            cairo_fill(cr);

            // Text

            const bool is_directory = m_model.is_directory(node);
            unsigned int text_offset = epsilon;

            if (is_directory) {   //  Arrow
                const auto& dir =
                    std::get<typename model::directory>(node.second);

                const float arrow_size = m_cell_height * arrow_size_scale;
                const float arrow_offset = (m_cell_height - arrow_size) / 2.0f;

                cairo_helper::set_source_color(cr, m_font_color);

                if (is_open(dir)) {
                    cairo_move_to(cr, left + arrow_offset + arrow_size, top + arrow_offset);
                    cairo_line_to(cr, left + arrow_offset + arrow_size, top + arrow_offset + arrow_size);
                }
                else {
                    cairo_move_to(cr, left + arrow_offset, top + arrow_offset);
                    cairo_line_to(cr, left + arrow_offset + arrow_size, top + 0.5 * m_cell_height);
                }

                cairo_line_to(cr, left + arrow_offset, top + arrow_offset + arrow_size);
                cairo_fill(cr);

                text_offset += m_cell_height;
            }

            // Text
            const std::string& text = node.first;

            //  Set Font settings
            cairo_set_font_size(cr, m_font_size);
            cairo_select_font_face(
                cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, is_directory ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
            cairo_helper::set_source_color(cr, m_font_color);

            const rectangle
                rect{
                    left, top,
                    static_cast<unsigned int>(width),
                    static_cast<unsigned int>(m_cell_height)};
            cairo_helper::show_left_aligned_text(cr, rect, text, text_offset);

            //  Update
            m_nodes[n] = &node;
        }

        template<class Value>
        bool directory_view<Value>::draw_directory(
            cairo_t *cr,
            model& m,
            unsigned int& cell,
            const unsigned int level)
        {
           const unsigned int count = m.get_item_count();

            for (unsigned int i = 0; i < count; ++i) {
                const auto& node = m.get_node(i);

                if (cell >= m_start_offset)
                    draw_cell(
                        cr, node, cell, level);
                cell++;

                if (cell >= (m_displayed_cell_count + m_start_offset))
                    return true;

                if (m_model.is_directory(node)) {
                    const auto& dir =
                        std::get<typename model::directory>(node.second);

                    if (is_open(dir) &&
                        draw_directory(
                            cr, *dir, cell, level + 1))
                        return true;
                }
            }

            return false;
        }

        template<class Value>
        bool directory_view<Value>::is_open(const typename model::directory& n) const
        {
            const auto it = m_node_is_open.find(&n);

            if (it == m_node_is_open.end()) //  non registered node are closed
                return false;
            else
                return it->second;
        }

    } /* View */

}   /*  Gammou */

#endif
