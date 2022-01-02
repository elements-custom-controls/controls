#include "spinbox.h"

template<typename T>
view_limits spinbox<T>::limits(basic_context const& ctx) const
{
    point p = measure_text(ctx.canvas, "1", theme().label_font);
    p.y *= height_index;
    return {{40, p.y}, {full_extent, p.y}};
}

template<typename T>
void spinbox<T>::draw(context const& ctx)
{
    auto size = ctx.bounds.size();
    auto pos = ctx.bounds.top_left();
    auto& thm = get_theme();
    float font_size = thm.icon_font._size * height_index;

    // Background
    ctx.canvas.fill_color(get_theme().panel_color);
    ctx.canvas.fill_round_rect(ctx.bounds, 8.0f);
    ctx.canvas.fill();

    left_arrow_bounds = ctx.bounds;
    left_arrow_bounds.width( size.x / 4.0f) ;
    right_arrow_bounds = ctx.bounds;
    right_arrow_bounds.left =  pos.x + size.x - (size.x / 4.0f);

    // Arrow icons
    color c = (focused == 1) ? colors::salmon.opacity(
                                   ((clicked) ? 1.0 : 0.75) ) :
                               colors::dark_slate_blue;
    draw_icon(ctx.canvas, left_arrow_bounds, icons::left_dir, font_size, c);
    c = (focused == 2) ? colors::light_blue.opacity(
                             ((clicked) ? 1.0 : 0.75)  ) :
                         colors::dark_slate_blue;
    draw_icon(ctx.canvas, right_arrow_bounds, icons::right_dir, font_size, c) ;

    const point middle(pos.x + (size.x / 2.0f), pos.y + (size.y / 2.0f) );
    const point t_point(middle.x, ( middle.y) );

    ctx.canvas.line_width(1);

    // Text
    ctx.canvas.stroke_color(get_theme().text_box_font_color);
    ctx.canvas.stroke_text(_text,t_point );

    // Borders
    ctx.canvas.stroke_color(get_theme().frame_color);
    ctx.canvas.stroke_round_rect(ctx.bounds, 8.0f);
    ctx.canvas.stroke();
}

template <typename T>
void spinbox<T>::long_click(context const& ctx, mouse_button btn, int long_press)
{
    if(!clicked)
        return;

    click_event(ctx, btn);
    if(long_press > 50) long_press -= 25;
    ctx.view.post(std::chrono::milliseconds( long_press), [=](){
        long_click(ctx, btn, long_press);
    });
}

template<typename T>
bool spinbox<T>::click(context const& ctx, mouse_button btn)
{

    if(btn.down){
        if(ctx.bounds.includes(btn.pos)) clicked = true;

        ctx.view.post(std::chrono::milliseconds(300), [=]() {long_click(ctx, btn, 200);});
        click_event(ctx, btn);
    } else // btn up
    {
        clicked = false;
        ctx.view.refresh(*this);
    }
    return true;
}

template<typename T>
void spinbox<T>::click_event(context const& ctx, mouse_button &btn)
{
    switch (focused) {
    case 1:
    {
        decrement();
        ctx.view.refresh(*this);
        on_change(controller.value);
        break;
    }
    case 2:
    {
        increment();
        ctx.view.refresh(*this);
        on_change(controller.value);
        break;
    }
    default:
    {
        click_pos = btn.pos;
        break;
    }
    }
}

template<typename T>
bool spinbox<T>::cursor(context const &ctx, point p, cursor_tracking)
{
    int f = focused;
    if(left_arrow_bounds.includes(p)) {
        focused = 1;
    } else if(right_arrow_bounds.includes(p)) {
        focused = 2;
    } else {
        focused = -1;
    }
    if(f != focused) ctx.view.refresh(*this);
    return true;
}

template<typename T>
bool spinbox<T>::scroll(context const&ctx, point dir, point )
{
    if(dir.y < 0) {
        decrement();
        ctx.view.refresh(*this);
        on_change(controller.value);
    }
    else if(dir.y > 0) {
        increment();
        ctx.view.refresh(*this);
        on_change(controller.value);
    }
    return true;
}

template<typename T>
void spinbox<T>::drag(const context &ctx, mouse_button btn)
{
    const float step = 5.0f;
    if(focused != -1) return;
    float diff = btn.pos.y - click_pos.y;
    if(diff > step)
    {
        click_pos = btn.pos;
        decrement();
        ctx.view.refresh(*this);
        on_change(controller.value);
    } else if(diff < -step)
    {
        click_pos = btn.pos;
        increment();
        ctx.view.refresh(*this);
        on_change(controller.value);
    }
}

template class spinbox<int>;
template class spinbox<float>;
template class spinbox<double>;
template class spinbox<size_t>;
