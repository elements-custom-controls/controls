/*=============================================================================
   Copyright (c) 2022 Johann Philippe

   Distributed under the MIT License (https://opensource.org/licenses/MIT)
=============================================================================*/
#include <elements.hpp>
#include<string>
using namespace cycfi::elements;
using namespace cycfi::artist;

// Main window background color
auto constexpr bkd_color = rgba(35, 35, 37, 255);
auto background = box(bkd_color);

struct color_selector
{
    bool selected = false;
    point pos;
    color selected_color;
};

template<typename T>
T wrap_number(T min, T max, T val)
{
    if(val > max)
        return max - (val - max);
    if(val < min)
        return min  +  (min - val);
    return val;
}

template<typename T>
T limit_number(T min, T max, T val)
{
    if(val > max) return max;
    if(val < min) return min;
    return val;
}


constexpr const double segment_division = 1.0 / 6.0;
size_t find_segment(double pos)
{
    return std::floor(pos * 6);
}
color find_color(double pos)
{
    color c(0, 0, 0);
    size_t seg = find_segment(pos);

    switch (seg) {
    case 0:
    {
        c.red = 1;
        c.green = pos * 6;
        break;
    }
    case 1:
    {
        c.green = 1;
        c.red = 1.0 - ((pos - segment_division) * 6);
        break;
    }
    case 2:
    {
        c.green = 1;
        c.blue = (pos - (2 * segment_division)) * 6;
        break;
    }
    case 3:
    {
        c.blue = 1;
        c.green = 1 - (pos - (3 * segment_division)) * 6;
        break;
    }
    case 4:
    {
        c.blue = 1;
        c.red = (pos - (4 * segment_division)) * 6;
        break;
    }
    case 5:
    case 6:
    {
        c.red = 1;
        c.blue = 1.0 - (pos - (5 * segment_division)) * 6;
        break;
    }
    default: break;
    }
    return c;
}


struct color_picker : tracker<>
{
    color_picker()
        : tracker<>()
    {}

    void draw(context const& ctx) override
    {
        auto &cnv = ctx.canvas;
        auto gr = canvas::linear_gradient{ { ctx.bounds.left, ctx.bounds.top}, { ctx.bounds.right, ctx.bounds.top} };
        gr.add_color_stop(0.0, colors::red);
        gr.add_color_stop(0.33, colors::green);
        gr.add_color_stop(0.66, colors::blue);
        gr.add_color_stop(1.0, colors::red);
        cnv.fill_style(gr);
        cnv.fill_round_rect(ctx.bounds, 8);

        // Alpha mask
        auto gr2 = canvas::linear_gradient{ { ctx.bounds.left, ctx.bounds.top}, {ctx.bounds.left, ctx.bounds.bottom} };
        gr2.add_color_stop(0.0, colors::black.opacity(0.));
        gr2.add_color_stop(1.0, colors::black);
        cnv.fill_style(gr2);
        cnv.fill_round_rect(ctx.bounds, 8);

        if(sel.selected)
        {
            point circle_pos(sel.pos.x * ctx.bounds.width() + ctx.bounds.left, sel.pos.y * ctx.bounds.height() + ctx.bounds.top);
            ctx.canvas.stroke_color(sel.selected_color.level(1.5));
            cnv.add_circle(circle(circle_pos, 8));
            ctx.canvas.fill_color(sel.selected_color);
            ctx.canvas.fill();
            cnv.add_circle(circle(circle_pos, 8));
            cnv.stroke();
        }
    }


    bool click(const context &ctx, mouse_button btn) override
    {
        if(btn.down)
        {
            sel.selected = true;
            sel.pos.x = (btn.pos.x - ctx.bounds.left) / ctx.bounds.width();
            sel.pos.y = (btn.pos.y - ctx.bounds.top) / ctx.bounds.height();
            calculate_color();
            ctx.view.refresh();
        }
        return true;
    }

    void drag(const context &ctx, mouse_button btn) override
    {
        if(btn.down && ctx.bounds.includes(btn.pos))
        {
            sel.pos.x = (btn.pos.x - ctx.bounds.left) / ctx.bounds.width();
            sel.pos.y = (btn.pos.y - ctx.bounds.top) / ctx.bounds.height();
            calculate_color();
            ctx.view.refresh();
        }
    }

    void calculate_color()
    {

        sel.selected_color = find_color(sel.pos.x);
        sel.selected_color.alpha = 1.0f - sel.pos.y;
        on_color_change(sel.selected_color);
    }

    std::function<void(color c)> on_color_change;

    color_selector sel;
};



struct color_surface : tracker<>
{
  color_surface(color c) : tracker<>(), base(c)
  {}


  void draw(context const& ctx) override
  {
      calculate_color();
      auto gr = canvas::linear_gradient{{ctx.bounds.left, ctx.bounds.top}, {ctx.bounds.right, ctx.bounds.top}};
      gr.add_color_stop(0.0, colors::white);
      gr.add_color_stop(1.0, base);
      ctx.canvas.fill_style(gr);
      ctx.canvas.fill_round_rect(ctx.bounds, 8);

      auto gr2 = canvas::linear_gradient{{ctx.bounds.left, ctx.bounds.top}, {ctx.bounds.left, ctx.bounds.bottom}};
      gr2.add_color_stop(0.0, colors::black.opacity(0.0));
      gr2.add_color_stop(1.0, colors::black);
      ctx.canvas.fill_style(gr2);
      ctx.canvas.fill_round_rect(ctx.bounds, 8);
      if(sel.selected)
      {
          point circle_pos(sel.pos.x * ctx.bounds.width() + ctx.bounds.left, sel.pos.y * ctx.bounds.height() + ctx.bounds.top);
          ctx.canvas.add_circle(circle(circle_pos, 10));
          ctx.canvas.fill_color(sel.selected_color);
          ctx.canvas.fill();
          ctx.canvas.add_circle(circle(circle_pos, 10));
          ctx.canvas.stroke_color(sel.selected_color.level(1.5));
          ctx.canvas.stroke();
      }
  }

  bool click(const context &ctx, mouse_button btn) override
  {
      if(!btn.down) return false;
      sel.selected = true;
      sel.pos.x = (btn.pos.x - ctx.bounds.left) / ctx.bounds.width();
      sel.pos.y = (btn.pos.y - ctx.bounds.top) / ctx.bounds.height();
      calculate_color();
      if(on_color_change)
          on_color_change(sel.selected_color);
      ctx.view.refresh();
      return true;
  }

  void drag(const context &ctx, mouse_button btn) override
  {
      if(!btn.down) return;
      if(!ctx.bounds.includes(btn.pos)) return;
      sel.pos.x = (btn.pos.x - ctx.bounds.left) / ctx.bounds.width();
      sel.pos.y = (btn.pos.y - ctx.bounds.top) / ctx.bounds.height();
      calculate_color();
      if(on_color_change)
          on_color_change(sel.selected_color);
      ctx.view.refresh();
  }

  void set_base(color c)
  {
      base = c;
      // calculate
  }

  void calculate_color()
  {
      float base1_x = sel.pos.x;
      float inv_base1_x = 1.0f - base1_x;
      float inv_base1_y = 1.0f - sel.pos.y;

      color s(inv_base1_x + (base.red * base1_x), inv_base1_x + (base.green * base1_x), inv_base1_x + (base.blue * base1_x) );
      sel.selected_color = s.level(inv_base1_y);
  }

  color_selector sel;
  color base;
  std::function<void(color c)> on_color_change;
};

struct color_bar : tracker<>
{
  color_bar()
    : tracker<>()
  {
  }

  void draw(context const& ctx) override
  {

      if(!sel.selected)
      {
          sel.selected = true;
          sel.pos.x = 0.5;
          sel.pos.y = 0.5;
          sel.selected_color.alpha = 1;
          calculate_color();
      }

      rect bnds = ctx.bounds;
      bnds.top += 10;
      bnds.bottom -= 10;

      auto &cnv = ctx.canvas;
      auto gr = canvas::linear_gradient{ { ctx.bounds.left, ctx.bounds.top}, { ctx.bounds.right, ctx.bounds.top} };
      gr.add_color_stop(0.0, colors::red);
      gr.add_color_stop(0.33, colors::green);
      gr.add_color_stop(0.66, colors::blue);
      gr.add_color_stop(1.0, colors::red);
      cnv.fill_style(gr);
      cnv.fill_round_rect(bnds, 8);

      point circle_pos(sel.pos.x * ctx.bounds.width() + ctx.bounds.left, sel.pos.y *ctx.bounds.height() + ctx.bounds.top);
      cnv.add_circle(circle(circle_pos, 10));
      cnv.stroke_color(sel.selected_color.level(1.5));
      cnv.line_width(1.5);
      cnv.stroke();
      cnv.add_circle(circle(circle_pos, 10));
      cnv.fill_color(sel.selected_color);
      cnv.fill();
  }

  bool click(const context &ctx, mouse_button btn) override
  {
      if(btn.down)
      {
          sel.selected = true;
          sel.pos.x = (btn.pos.x - ctx.bounds.left) / ctx.bounds.width();
          calculate_color();
          ctx.view.refresh();
      }
      return true;

  }
  void drag(const context &ctx, mouse_button btn) override
  {
      if(btn.down && ctx.bounds.includes(btn.pos))
      {
          sel.pos.x = (btn.pos.x - ctx.bounds.left) / ctx.bounds.width();
          calculate_color();
          ctx.view.refresh();
      }
  }

  view_limits limits(basic_context const&) const override
  {
      return {{200, 30}, {full_extent, 30}};
  }

  void calculate_color()
  {
      sel.selected_color = find_color(sel.pos.x);
      if(on_color_change)
          on_color_change(sel.selected_color);
  }

  color_selector sel;

  std::function<void(color c)> on_color_change;
};

struct color_shower : tracker<>
{
    void draw(context const & ctx ) override
    {
        ctx.canvas.fill_color(c);
        ctx.canvas.fill_round_rect(ctx.bounds, 8);
    }

    color c = colors::navy;
};

int main(int argc, char* argv[])
{
   app _app(argc, argv, "Color Picker", "com.cycfi.color-picker");
   window _win(_app.name());
   _win.on_close = [&_app]() { _app.stop(); };

   view view_(_win);

   label l_red("");
   label l_green("");
   label l_blue("");
   label l_alpha("");

   auto update_label = [&](color c)
   {
     l_red.set_text(std::to_string((int)(c.red * 255)));
     l_green.set_text(std::to_string((int)(c.green * 255) ));
     l_blue.set_text(std::to_string((int)(c.blue * 255)));
     l_alpha.set_text(std::to_string((int)(c.alpha * 255)));
   };

   update_label(colors::black);

   color_shower shower;
   color_bar bar;
   color_surface s(colors::green);
   s.on_color_change = [&](color c)
   {
     shower.c = c;
     update_label(c);
     view_.layout();
     view_.refresh(shower);
   };

   bar.on_color_change = [&](color c)
   {
       s.base = c;
       shower.c = c;
       update_label(c);
       view_.refresh();
   };

   color_picker picker;
   color_shower shower2;

   picker.on_color_change = [&](color c)
   {
     shower2.c =c;
     view_.refresh(shower2);
   };


   view_.content(
               margin({10, 10, 10, 10},

               htile(
                          pane("Picked ",
                                 vtile(
                                     simple_heading(link(l_red), "red"),
                                     simple_heading(link(l_green), "green"),
                                     simple_heading(link(l_blue), "blue"),
                                     simple_heading(link(l_alpha), "alpha")
                                     )
                                 )
                          , hsize(500, pane("Composite picker",
                               vtile(
                                   vsize(150, link(shower)),
                                   vspacer(10),
                                   vsize(300,
                                         link(s)),
                                   vspacer(10),
                                   bar
                                   )
                               ))
                          , hspacer(20)
                          , hsize(500, pane("Simple picker",
                                 vtile(link(shower2),
                                       vspacer(10),
                                       picker
                                       )
                                 ))
                          )
                     ),
      background
   );

   _app.run();
   return 0;
}
