#pragma once

#include <string>
#include <vector>

// 求生者静态数据：只保存角色本身的能力，不保存本局位置。
// 所有评分均为 1-10 分，分数越高表示该项越强。
struct survivor_static_data
{
    int id;
    std::string name;
    int decode;
    int kite;
    int assist;
    int rescue;
    int target_value;
    std::vector<std::string> tags;
    std::string feature;
};

// 求生者对象：静态角色数据 + 本局所在地图区域。
class survivor
{
public:
    survivor();
    survivor(const survivor_static_data& data, int region_id);

    int id() const;
    const std::string& name() const;
    int decode_score() const;
    int kite_score() const;
    int assist_score() const;
    int rescue_score() const;
    int target_value() const;
    int region_id() const;
    const std::vector<std::string>& tags() const;
    const std::string& feature() const;

    bool has_tag(const std::string& tag) const;
    std::string metric_line(const std::string& regionname) const;
    std::string short_profile() const;
    std::string advice(double probability, const std::string& best_action) const;

private:
    survivor_static_data data_;
    int region_id_;
};

std::string score_level(int score);
