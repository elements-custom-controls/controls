/*=============================================================================
   Copyright (c) 2021 Johann Philippe

   Distributed under the MIT License (https://opensource.org/licenses/MIT)
=============================================================================*/
#include <elements.hpp>
#include"expandable.h"

using namespace cycfi::elements;
using namespace cycfi::artist;

// Main window background color
auto constexpr bkd_color = rgba(35, 35, 37, 255);
auto background = box(bkd_color);

int main(int argc, char* argv[])
{
   app _app(argc, argv, "Expandable", "com.cycfi.expandable");
   window _win(_app.name());
   _win.on_close = [&_app]() { _app.stop(); };

   view view_(_win);

   auto b = share(button("hello top"));
   auto c = share(button("button left"));
   auto d = share(button("button right"));
   auto e = share(button("button bottom"));
   auto z = label("center label");

   auto exp_t = top_expander(view_, b);
   auto exp_l = left_expander(view_, c);
   auto exp_r = right_expander(view_, d);
   auto exp_b = bottom_expander(view_, e);

   view_.content(
               vtile(
                   align_top(link(exp_t)),
                   align_left_middle(link(exp_l)),
                   align_right_middle(link(exp_r)),
                   align_bottom(link(exp_b))
                   ),
                   align_center_middle(z),
      background
   );

   _app.run();
   return 0;
}
