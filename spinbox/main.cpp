/*=============================================================================
   Copyright (c) 2021 Johann Philippe

   Distributed under the MIT License (https://opensource.org/licenses/MIT)
=============================================================================*/
#include <elements.hpp>
#include"spinbox.h"
using namespace cycfi::elements;
using namespace cycfi::artist;

// Main window background color
auto constexpr bkd_color = rgba(35, 35, 37, 255);
auto background = box(bkd_color);

int main(int argc, char* argv[])
{
   app _app(argc, argv, "SpinBox", "com.cycfi.spinbox");
   window _win(_app.name());
   _win.on_close = [&_app]() { _app.stop(); };

   view view_(_win);

   spinbox<int> s1 (spin_controller<int>(1, 100, 1, 1));
   spinbox<int> s2 (spin_controller<int>(1, 100, 1, 5));
   spinbox<double> s3 (spin_controller<double>(0, 0.75, 0.1, 0.01));
   spinbox<size_t> s4 (spin_controller<size_t>(100, 100'000, 100, 1000));

   view_.content(
               margin({10, 10, 10, 10},
                      vtile(
                          s1, s2, s3, s4
                          )),
                     // Add more content layers here. The order
                     // specifies the layering. The lowest layer
                     // is at the bottom of this list.

      background     // Replace background with your main element,
                     // or keep it and add another layer on top of it.
   );

   _app.run();
   return 0;
}
