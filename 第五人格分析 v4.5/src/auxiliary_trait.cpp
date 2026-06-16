#include "auxiliary_trait.h"

#include <algorithm>

auxiliary_trait::auxiliary_trait()
    : data_{ 0, "未选择", 0.0, 0.0, 0.0, {}, "未选择辅助特质。" }
{
}

auxiliary_trait::auxiliary_trait(const auxiliary_trait_static_data& data)
    : data_(data)
{
}

int auxiliary_trait::id() const { return data_.id; }
const std::string& auxiliary_trait::name() const { return data_.name; }
double auxiliary_trait::opening_cooldown_seconds() const { return data_.opening_cooldown_seconds; }
double auxiliary_trait::first_hit_delta_seconds() const { return data_.first_hit_delta_seconds; }
double auxiliary_trait::down_delta_seconds() const { return data_.down_delta_seconds; }
const std::vector<std::string>& auxiliary_trait::tags() const { return data_.tags; }
const std::string& auxiliary_trait::effect() const { return data_.effect; }

bool auxiliary_trait::has_tag(const std::string& tag) const
{
    return std::find(data_.tags.begin(), data_.tags.end(), tag) != data_.tags.end();
}
