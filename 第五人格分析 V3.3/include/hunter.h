#pragma once

#include <string>
#include <vector>

// 监管者静态数据：追击、守椅、控场是首追判断中最核心的三项。
// extra_power 表示版本强势度/综合上限，用于小幅校准。
struct hunter_static_data
{
    int id;
    std::string name;
    int chase;
    int camp;
    int control;
    int info;
    int extra_power;
    std::vector<std::string> tags;
    std::string feature;
};

class hunter
{
public:
    hunter();
    explicit hunter(const hunter_static_data& data);

    int id() const;
    const std::string& name() const;
    int chase_score() const;
    int camp_score() const;
    int control_score() const;
    int info_score() const;
    int extra_power() const;
    const std::vector<std::string>& tags() const;
    const std::string& feature() const;

    bool has_tag(const std::string& tag) const;
    std::string strength_line() const;

private:
    hunter_static_data data_;
};
