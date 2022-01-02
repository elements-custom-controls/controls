#ifndef SPINBOX_H
#define SPINBOX_H

#include<elements.hpp>
#include<iostream>
using namespace cycfi::elements;
using namespace cycfi::artist;

//gets a string from a number with custom precision
template<typename T>
static std::string get_string(T val, int precision = 5) {
        std::ostringstream ss;
        ss.precision(precision);
        ss << val;
        return ss.str();
}

// Limits a number
template<typename T>
T limit_number(T min, T max, T val)
{
    if(val >= max) return max;
    else if(val <= min) return min;
    else return val;
}

template<typename T>
struct spin_controller
{
    spin_controller(T min_, T max_, T init_, T step_) :
        value(init_), min(min_), max(max_), step(step_)
    {}
    T value, min, max, step;
};

/*****************************************
 * A scrollable, clickable, draggable Spinbox
 * (Number box with range and step)
*****************************************/
template<typename T>
class spinbox : public tracker<>
{

public:

    constexpr static const float height_index = 1.55f;

    spinbox(spin_controller<T> control_) : tracker<>(), controller(control_),
        _text(std::to_string(controller.value)),
        on_change([&](T){}),
    focused(-1), clicked(false)
    {
        update_text();
    }

    spinbox() : spinbox(spin_controller<T>(0, 1, 1, 1))
    {}

    virtual view_limits limits(basic_context const& ctx) const override;
    void draw(context const &ctx) override;
    bool click(context const &ctx, mouse_button btn) override;
    bool cursor(context const &ctx, point p, cursor_tracking)  override;
    bool scroll(context const&ctx, point dir, point ) override;
    void drag(const context &ctx, mouse_button btn) override;
    void long_click(context const& ctx, mouse_button btn, int _long_press);

    void set_controller(spin_controller<T> ctrl)
    {
        this->controller = ctrl;
        update_text();
    }

    virtual void increment()
    {
        controller.value =
                limit_number<T>
                (controller.min, controller.max, controller.value + controller.step);
        update_text();
    }

    virtual void decrement()
    {
        controller.value =
                limit_number<T>
                (controller.min, controller.max, controller.value - controller.step);
        update_text();
    }

    virtual void set_value(T val)
    {
        controller.value =
                limit_number<T>
                (controller.min, controller.max, val);
        update_text();
    }

    T get_value() {return controller.value;}

    spin_controller<T> controller;
    std::function<void(T)> on_change;

protected:

    void update_text()
    {
        _text = get_string<T>(controller.value);
    }

private:

    void click_event(context const &ctx, mouse_button &btn);

    std::string _text;
    int focused;
    point click_pos;
    rect left_arrow_bounds, right_arrow_bounds;;
    bool clicked;

    std::string name;
    bool labeled = false;
};

template <typename T>
class spinbox_list_controller
{
public:
    spinbox_list_controller(T value_, std::vector<T> values_)
        : value(value_)
        , values(values_)
    {
        // verify value is in values
        bool is_found = false;
        for(auto & it : values)
        {
            std::cout << "vector value : "  << it << std::endl;
            if(value == it)
            {
                std::cout << "is found " << std::endl;
                is_found = true;
                break;
            }
        }
        // if it is not, add it inside
        if(!is_found)
            values.push_back(value);
        // then sort
        std::sort(values.begin(), values.end());
        // find index of val
        for(size_t i = 0; i < values.size(); ++i)
        {
            if(value == values[i])
            {
                val_index = i;
                break;
            }
        }
    }

    int find_value_position(T v)
    {
        for(size_t i = 0; i < values.size(); ++i)
        {
            if(v == values[i])
                return i;
        }
        return -1;
    }

    int has_next()
    {
        if(val_index < values.size() - 1)
        {
            return (value = values[++val_index]);
        }
        return -1;
    }

    int has_previous()
    {
        if(val_index > 0)
        {
            return (value = values[--val_index]);
        }
        return -1;
    }

    T value;
    std::vector<T> values;
    size_t val_index;
};

template <typename T>
struct spinbox_list : public spinbox<T>
{
    spinbox_list(spinbox_list_controller<T> control_)
        : spinbox<T>()
        , _list_controller(control_)
    {
        this->controller.value = _list_controller.value;
        this->update_text();
    }

    void increment() override
    {
        if(_list_controller.has_next() != -1)
        {
            this->controller.value = _list_controller.value;
            this->update_text();
        }
    }

    void decrement() override
    {
        if(_list_controller.has_previous() != -1)
        {
            this->controller.value = _list_controller.value;
            this->update_text();
        }
    }

    void set_value(T v) override
    {
        int pos = _list_controller.find_value_position(v);
        if(pos != -1)
        {
            _list_controller.val_index = pos;
            _list_controller.value = v;
            this->controller.value = _list_controller.value;
            this->update_text();
        }
    }


private:
    spinbox_list_controller<T> _list_controller;

};


template<typename T, size_t Width>
class fixed_width_spinbox : public spinbox<T>
{
public:
    fixed_width_spinbox(spin_controller<T> ctrl) : spinbox<T>(ctrl) {}
    fixed_width_spinbox() : spinbox<T>() {}
    view_limits limits(basic_context const& ctx) const override
    {
        view_limits l = spinbox<T>::limits(ctx);
        return {{l.min.x, l.min.y}, {Width, l.max.y}};
    }

};

template<typename T>
inline auto make_labeled_spinbox(std::string text, spin_controller<T> ctrl, std::function<void(T)> on_change)
{
    spinbox<T> c(ctrl);
    c.on_change = on_change;
    return group(text, top_margin(40, margin({1,1,1,1}, vtile(hstretch(2, c)) ) ));
}



#endif // SPINBOX_H
