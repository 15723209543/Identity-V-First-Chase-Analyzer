#pragma once

#include "auxiliary_trait.h"

#include <vector>

// 辅助特质参考第五人格 BWIKI“辅助特质”页面整理。
// 系数含义：
// opening_cooldown_seconds：开局冷却时间；若目标在冷却结束前已经被击打，则该辅助特质不修正追击时间。
// first_hit_delta_seconds：冷却结束后对首刀时间的直接修正，负数表示更容易更快拿首刀。
// down_delta_seconds：对完整击倒时间的直接修正，负数表示更容易更快击倒。
// tags：给 score_calculator 做二次判断，例如闪现更适合高板窗区，兴奋更克制控制位。
const std::vector<auxiliary_trait_static_data> k_auxiliary_trait_data = {
    { 1, "聆听", 0.0, 0.0, 0.0, { "info" },
        "暴露正在跑动或交互的求生者，主要提升开局确认方向效率。" },
    { 2, "失常", 40.0, 0.0, 0.0, { "control_cipher" },
        "破坏密码机、狂欢之椅等进度；本模型中不直接修正首追击打时间。" },
    { 3, "兴奋", 40.0, -1.0, -3.0, { "anti_control" },
        "解除并短时间免疫控制，对前锋、击球手、勘探员等控制位收益更高。" },
    { 4, "巡视者", 30.0, -5.0, -8.0, { "lock_target" },
        "放出巡视者咬住求生者，降低转点和博弈空间。" },
    { 5, "传送", 45.0, -2.0, -5.0, { "far_reposition" },
        "快速转移到远处密码机、椅子或门等目标，远距离开局收益更高。" },
    { 6, "窥视者", 10.0, -2.0, -5.0, { "slow_reveal" },
        "放置窥视者暴露附近求生者，并降低移动和板窗交互速度。" },
    { 7, "闪现", 60.0, -12.0, -15.0, { "blink", "anti_resource" },
        "瞬间位移补刀，最常用于跨过板窗或贴近目标拿关键一刀。" },
    { 8, "移形", 50.0, -4.0, -7.0, { "far_reposition", "multi_reposition" },
        "建立装置后可进行位置交换，帮助改追击路线或切入远点。" }
};
