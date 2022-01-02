#ifndef EXPANDABLE_H
#define EXPANDABLE_H

#include<elements.hpp>
#include<iostream>
using namespace cycfi::elements;

enum class expandable_origin_mode
{
    left, top, right, bottom
};

template<expandable_origin_mode Mode>
struct expandable_state
{
    expandable_state()
    {
        switch (Mode) {
        case expandable_origin_mode::top:
        {
            first_icon = icons::down_dir;
            second_icon = icons::up_dir;
            break;
        }
        case expandable_origin_mode::left:
        {
            first_icon = icons::right_dir;
            second_icon = icons::left_dir;
            break;
        }
        case expandable_origin_mode::right:
        {
            first_icon = icons::left_dir;
            second_icon = icons::right_dir;
            break;
        }
        case expandable_origin_mode::bottom:
        {
            first_icon = icons::up_dir;
            second_icon = icons::down_dir;
            break;
        }
        }
    }
    uint32_t first_icon, second_icon;
    bool expanded = false;
};

template<bool hilite, expandable_origin_mode Mode>
struct expandable_icon_button_element : public plain_icon_button_element<hilite>
{
  expandable_icon_button_element(uint32_t code, float size)
    : plain_icon_button_element<hilite>(code, size)
  {}

  view_limits limits(basic_context const&) const override
  {
      auto  size = this->_size * get_theme().icon_font._size * 1.8f;
      if(Mode == expandable_origin_mode::top || Mode == expandable_origin_mode::bottom)
      {
          return {{size, size}, {full_extent, size}};
      } else
      {
          return {{size, size}, {size, full_extent}};
      }
  }
  void draw(context const& ctx)override
  {
      ctx.canvas.stroke_color(get_theme().text_box_hilite_color);
      ctx.canvas.line_width(1);
      ctx.canvas.stroke_round_rect(ctx.bounds, 4);
      ctx.canvas.stroke();
     draw_plain_icon_button(
        ctx, this->_code, this->_size * get_theme().icon_font._size, hilite
     );
  }
};

template<expandable_origin_mode Mode>
inline basic_toggle_button<> expandable_toggle_icon_button(
   uint32_t code1
 , uint32_t code2
 , float size
)
{
   return {
      expandable_icon_button_element<true, Mode>{code1, size}
    , expandable_icon_button_element<true, Mode>{code2, size}
   };
}

template<expandable_origin_mode Mode>
class expandable_bar :
        public expandable_state<Mode>,
        public basic_toggle_button<>
{
public:
    expandable_bar()
        : basic_toggle_button<>(expandable_toggle_icon_button<Mode>(this->first_icon, this->second_icon, 1.0))
    {}
};

// Now useless -> to remove (prefer the expander_base class)
template<expandable_origin_mode Mode, typename T>
inline auto base_expander(view &v, element_ptr ptr, size_t pos = 0)
{
    auto vc =  T();
    auto but = expandable_bar<Mode>();

    but.on_click = [&, ptr, pos](bool b)
    {
      if(b)
      {
          vc.insert(vc.begin() + pos, ptr );
      }
      else
      {
          vc.end_focus();
          vc.erase(vc.begin() + pos);
      }

      v.layout();
      v.refresh();
    };
    vc.push_back(share( but ));
    v.layout();
    return vc;
}

////////////////////////////////////////////////////////////////////////////
// Expander view - allows its child to expand or not on button click
// T must be htile_composite or vtile_composite
////////////////////////////////////////////////////////////////////////////
template< expandable_origin_mode Mode, typename T = htile_composite >
class expander_base : public T
{
public:
    expander_base(view &v, element_ptr ptr, size_t pos = 0)
        : T(), _ptr(ptr), but(share(expandable_bar<Mode>())), _pos(pos)
    {
        but->on_click = [this, &v, ptr, pos](bool b)
        {
            but->expanded = b;
            if(b)
            {
                this->insert(this->begin() + pos,  ptr);
            }
            else
            {
                this->end_focus();
                this->erase(this->begin() + pos);
            }
            v.layout();
            v.refresh();
        };

        this->push_back(but);
        v.layout();
        v.refresh();
    }

    view_limits limits(basic_context const& ctx) const override
    {
        view_limits bar_limits = but->limits(ctx);
        if(!but->expanded) return bar_limits;
        view_limits sub_limits = _ptr->limits(ctx);
        if(Mode == expandable_origin_mode::top || Mode == expandable_origin_mode::bottom)
            return {{bar_limits.min.x, bar_limits.min.y},{bar_limits.max.x , bar_limits.max.y  + sub_limits.max.y}};
        else
            return {{bar_limits.min.x, bar_limits.min.y},{bar_limits.max.x + sub_limits.max.x, bar_limits.max.y }};
    }

    // useless, just for tests
    bool click(context const& ctx, mouse_button btn) override
    {

        element * el  = this->hit_test(ctx, btn.pos);
        if(el == but.get())
        {
            but->click(ctx, btn);
            return true;
        }
        else if(but->expanded && el == _ptr.get())
        {
            composite_base::hit_info info = this->hit_element(ctx, btn.pos, true);
            return _ptr->click(context{ctx, info.bounds}, btn);

            // find new context :
            this->end_focus();
            this->focus(info.index);
            this->begin_focus();
            info.element->click(ctx, btn);

        }
        return true;
    }

    element_ptr _ptr;
    std::shared_ptr<expandable_bar<Mode>> but;
    size_t _pos;
};

inline auto top_expander(view& v, element_ptr ptr)
{
    return expander_base< expandable_origin_mode::top, vtile_composite>(v, ptr, 0);
}

inline auto left_expander(view &v, element_ptr ptr)
{
    return expander_base< expandable_origin_mode::left, htile_composite>(v, ptr, 0);
}

inline auto right_expander(view &v, element_ptr ptr)
{
    return expander_base<expandable_origin_mode::right, htile_composite>(v, ptr, 1);
}

inline auto bottom_expander(view &v, element_ptr ptr)
{
    return expander_base< expandable_origin_mode::bottom, vtile_composite>(v, ptr, 1);
}
#endif // EXPANDABLE_H
