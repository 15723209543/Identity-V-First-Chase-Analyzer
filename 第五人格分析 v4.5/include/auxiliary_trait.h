#pragma once

#include <string>
#include <vector>

// 监管者辅助特质静态数据。
// 这里的时间修正统一使用“秒”：负数表示能更快完成首追，正数表示会拖慢直接追击。
struct auxiliary_trait_static_data
{
    int id;
    std::string name;
    double opening_cooldown_seconds;
    double first_hit_delta_seconds;
    double down_delta_seconds;
    std::vector<std::string> tags;
    std::string effect;
};

// 辅助特质对象：保存本局监管者选择的辅助技能。
class auxiliary_trait
{
public:
    auxiliary_trait();
    explicit auxiliary_trait(const auxiliary_trait_static_data& data);

    int id() const;
    const std::string& name() const;
    double opening_cooldown_seconds() const;
    double first_hit_delta_seconds() const;
    double down_delta_seconds() const;
    const std::vector<std::string>& tags() const;
    const std::string& effect() const;

    bool has_tag(const std::string& tag) const;

private:
    auxiliary_trait_static_data data_;
};
