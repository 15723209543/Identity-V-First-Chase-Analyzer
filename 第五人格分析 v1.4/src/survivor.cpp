#include "survivor.h"

#include <algorithm>
#include <sstream>

survivor::survivor()
    : data_{}, region_id_(0)
{
}

survivor::survivor(const survivor_static_data& data, int region_id)
    : data_(data), region_id_(region_id)
{
}

int survivor::id() const { return data_.id; }
const std::string& survivor::name() const { return data_.name; }
int survivor::decode_score() const { return data_.decode; }
int survivor::kite_score() const { return data_.kite; }
int survivor::assist_score() const { return data_.assist; }
int survivor::rescue_score() const { return data_.rescue; }
int survivor::target_value() const { return data_.target_value; }
int survivor::region_id() const { return region_id_; }
const std::vector<std::string>& survivor::tags() const { return data_.tags; }
const std::string& survivor::feature() const { return data_.feature; }

bool survivor::has_tag(const std::string& tag) const
{
    return std::find(data_.tags.begin(), data_.tags.end(), tag) != data_.tags.end();
}

std::string survivor::metric_line(const std::string& regionname) const
{
    std::ostringstream out;
    out << "破译" << score_level(decode_score()) << "(" << decode_score() << ")，"
        << "牵制" << score_level(kite_score()) << "(" << kite_score() << ")，"
        << "辅助" << score_level(assist_score()) << "(" << assist_score() << ")，"
        << "救援" << score_level(rescue_score()) << "(" << rescue_score() << ")，"
        << "当前位置：" << regionname;
    return out.str();
}

std::string survivor::short_profile() const
{
    std::ostringstream out;
    out << feature();
    return out.str();
}

std::string survivor::advice(double probability, const std::string& best_action) const
{
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(2);
    out << "监管者对他首追的意愿：" << probability * 100.0 << "%，"
        << "现在最应当：" << best_action;
    return out.str();
}

std::string score_level(int score)
{
    if (score >= 9) return "极强";
    if (score >= 7) return "较强";
    if (score >= 5) return "中等";
    if (score >= 3) return "偏弱";
    return "很弱";
}
