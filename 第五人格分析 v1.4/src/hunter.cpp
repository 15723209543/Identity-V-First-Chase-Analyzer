#include "hunter.h"
#include "survivor.h"

#include <algorithm>
#include <sstream>

hunter::hunter()
    : data_{}
{
}

hunter::hunter(const hunter_static_data& data)
    : data_(data)
{
}

int hunter::id() const { return data_.id; }
const std::string& hunter::name() const { return data_.name; }
int hunter::chase_score() const { return data_.chase; }
int hunter::camp_score() const { return data_.camp; }
int hunter::control_score() const { return data_.control; }
int hunter::info_score() const { return data_.info; }
int hunter::extra_power() const { return data_.extra_power; }
const std::vector<std::string>& hunter::tags() const { return data_.tags; }
const std::string& hunter::feature() const { return data_.feature; }

bool hunter::has_tag(const std::string& tag) const
{
    return std::find(data_.tags.begin(), data_.tags.end(), tag) != data_.tags.end();
}

std::string hunter::strength_line() const
{
    std::ostringstream out;
    out << "追击" << score_level(chase_score()) << "(" << chase_score() << ")，"
        << "守椅" << score_level(camp_score()) << "(" << camp_score() << ")，"
        << "控场" << score_level(control_score()) << "(" << control_score() << ")，"
        << "信息" << score_level(info_score()) << "(" << info_score() << ")。"
        << "强度分析：" << feature();
    return out.str();
}
