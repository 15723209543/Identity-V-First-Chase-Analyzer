#pragma once

#include "hunter.h"

#include <vector>

// 监管者数据：到“牙医”为止。
inline const std::vector<hunter_static_data> k_hunter_data = {
    {1, "厂长", 5, 7, 6, 4, 4, {"camp", "area_control", "puppet"}, "傀儡守椅和换位能补节奏，但开局追击偏朴素。"},
    {2, "小丑", 8, 6, 4, 3, 5, {"fast_chase", "dash", "anti_mobility"}, "冲刺追击强，适合抓开阔区或转点失误。"},
    {3, "鹿头", 7, 8, 5, 4, 5, {"hook", "camp", "long_range"}, "钩子能惩罚直线转点，守椅压迫较强。"},
    {4, "杰克", 7, 6, 5, 4, 5, {"stealth", "fast_chase", "long_range"}, "隐身和雾刃提升拿刀效率，依赖路径判断。"},
    {5, "蜘蛛", 6, 8, 5, 4, 4, {"camp", "web", "anti_mobility"}, "蛛丝能限制转点，守椅和滚雪球不错。"},
    {6, "红蝶", 8, 5, 5, 4, 5, {"fast_chase", "anti_mobility"}, "刹那生灭追击强，但被卡视角会降速。"},
    {7, "黄衣之主", 6, 9, 7, 5, 6, {"camp", "area_control", "long_range"}, "触手守椅与区域压迫强，适合压低救援位容错。"},
    {8, "宿伞之魂", 6, 7, 8, 6, 6, {"teleport", "area_control"}, "传伞控场强，首追效率取决于形态切换和落点。"},
    {9, "摄影师", 5, 7, 8, 8, 6, {"camera", "area_control", "anti_decoder"}, "相中世界能压低修机节奏，对高破译低自保位威胁大。"},
    {10, "疯眼", 4, 8, 10, 7, 6, {"area_control", "wall", "anti_decoder"}, "控制台控场极强，但开局单追能力偏低。"},
    {11, "梦之女巫", 6, 8, 10, 8, 8, {"area_control", "multi_body", "info"}, "多信徒运营上限高，能同时压迫破译和救援。"},
    {12, "爱哭鬼", 7, 7, 6, 5, 5, {"long_range", "camp", "area_control"}, "怨灵远程拿刀，守椅和区域压迫均衡。"},
    {13, "孽蜥", 8, 6, 6, 4, 5, {"fast_chase", "anti_mobility", "leap"}, "跳跃跨地形能力强，能处理部分强转点。"},
    {14, "红夫人", 8, 7, 7, 5, 6, {"fast_chase", "mirror", "long_range"}, "镜像拿刀强，能惩罚贪板窗和修机位。"},
    {15, "26号守卫", 6, 10, 7, 5, 6, {"camp", "area_control", "bomb"}, "炸弹守椅极强，首追目标通常要服务于守椅收益。"},
    {16, "使徒", 7, 8, 6, 5, 5, {"stun", "camp", "anti_mobility"}, "猫步控制能打断转点，守椅压迫也不错。"},
    {17, "小提琴家", 7, 8, 6, 5, 5, {"long_range", "camp"}, "音符远程封路，追击和守椅都比较均衡。"},
    {18, "雕刻家", 7, 9, 7, 5, 6, {"long_range", "area_control", "camp"}, "雕像夹击和守椅强，能惩罚走位固定的目标。"},
    {19, "博士", 8, 4, 6, 4, 4, {"fast_chase", "no_chair", "anti_mobility"}, "追击直接，但淘汰节奏特殊，目标选择更看击倒效率。"},
    {20, "破轮", 9, 7, 7, 5, 7, {"anti_mobility", "fast_chase", "area_control"}, "高速形态压转点，适合抓依赖位移但落点固定的角色。"},
    {21, "渔女", 7, 8, 8, 5, 6, {"area_control", "anti_loop", "camp"}, "水渊封区强，能降低板窗区安全性。"},
    {22, "蜡像师", 8, 8, 6, 5, 6, {"anti_item", "long_range", "camp"}, "蜡油能限制交互和道具节奏，对依赖道具牵制位有效。"},
    {23, "噩梦", 7, 7, 6, 6, 5, {"teleport", "long_range", "info"}, "巡视与飞掠提供信息和突进，适合抓暴露位置的修机位。"},
    {24, "记录员", 6, 8, 9, 7, 6, {"area_control", "anti_decoder", "long_range"}, "记录能干扰修机和交互，适合压制高价值点位。"},
    {25, "隐士", 6, 8, 10, 8, 7, {"area_control", "anti_decoder", "split_damage"}, "电荷与连线让控场和压状态能力突出。"},
    {26, "守夜人", 9, 7, 6, 5, 7, {"fast_chase", "anti_mobility"}, "风域加速和吸拉强，首追效率高。"},
    {27, "歌剧演员", 10, 7, 8, 5, 8, {"fast_chase", "anti_mobility", "area_control"}, "暗影位移追击极强，能快速逼出强牵制位资源。"},
    {28, "愚人金", 8, 7, 7, 5, 6, {"area_control", "anti_loop", "long_range"}, "矿镐和不稳定区域能限制转点。"},
    {29, "时空之影", 8, 7, 8, 6, 7, {"teleport", "anti_mobility", "area_control"}, "空间压迫和快速位移使开局找人、换追更灵活。"},
    {30, "跛脚羊", 8, 8, 7, 5, 6, {"area_control", "fast_chase", "camp"}, "封路和追击强，适合限制固定板窗区。"},
    {31, "喧嚣", 8, 7, 7, 5, 6, {"anti_loop", "long_range", "fast_chase"}, "区域干扰和远程压迫让短板区更危险。"},
    {32, "杂货商", 7, 7, 8, 6, 6, {"area_control", "anti_item", "info"}, "道具和区域控制均衡，对依赖道具位有加成。"},
    {33, "台球手", 8, 7, 6, 5, 6, {"fast_chase", "long_range", "anti_mobility"}, "直线压迫和撞击能惩罚开阔区转点。"},
    {34, "女王蜂", 8, 8, 7, 6, 7, {"area_control", "anti_heal", "long_range"}, "蜂群压状态和封路能力强，对治疗体系更凶。"},
    {35, "牙医", 9, 8, 8, 7, 7, {"anti_mobility", "anti_heal", "long_range", "area_control"}, "高机动和范围压迫兼具，对飞行、治疗和转点角色威胁大。"}
};
