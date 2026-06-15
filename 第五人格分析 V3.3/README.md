# Identity V First Chase Analyzer

> 第五人格监管者首抓分析器

## 项目简介

本项目是一个基于 **C++ / EasyX / Visual Studio** 的《第五人格》监管者首抓分析工具。程序会根据当前地图、4 名求生者的出生位置和角色、监管者出生位置和角色，综合计算监管者开局更可能首追哪一名求生者，并给出首追概率、寻路距离、计算系数、分析原因和行动建议。

程序采用图形化点击输入方式。启动后会打开 EasyX 窗口，用户通过鼠标选择地图、求生者位置、求生者角色、监管者位置和监管者角色，程序随后在右侧窗口显示完整分析结果，并将分析文本自动保存到 `result/` 文件夹。

本项目主要用于 C++ 面向对象程序设计、EasyX 图形界面练习、游戏数据建模、地图几何分析、最短路径计算和策略推荐展示。本项目不是《第五人格》官方工具，分析结果不代表官方结论，也不保证完全符合任何版本的真实对局环境。

---

## 项目信息

| 项目 | 内容 |
|---|---|
| 中文名称 | 第五人格监管者首抓分析器 |
| 英文名称 | Identity V First Chase Analyzer |
| 推荐仓库名 | `identity-v-first-chase-analyzer` |
| 当前版本 | `v3.3` |
| 代码写作日期 | `2026-06-15` |
| 日期依据 | 以当前压缩包版本、项目文件和结果文件时间为准 |
| 开发语言 | C++ |
| 图形库 | EasyX |
| 推荐 IDE | Visual Studio 2022 及以上 / 兼容版本 |
| 编译平台 | Windows x64 |
| 字符集 | 多字节字符集 |
| 源码编码 | 建议保持 GB2312 / GBK |
| 项目类型 | Windows 桌面图形化 C++ 程序 |
| 当前状态 | 学习与展示版本，核心功能已完成，仍保留继续扩展空间 |

---

## 当前版本说明

当前版本为 `v3.3`。本版本在上一版本地图几何分析的基础上，进一步把评分模型拆分为独立模块，并增加了外部参数配置文件，使模型权重可以在不重新编译代码的情况下调整。

当前版本主要特点如下：

1. 使用 EasyX 显示 `data/map` 中的 9 张地图图源。
2. 支持在地图原图上直接点击求生者和监管者位置。
3. 使用地图裁剪范围、白色像素识别、外墙外白底排除和墙体网格判断点击是否合法。
4. 使用墙体网格进行寻路距离计算，不再只依赖直线距离。
5. 使用求生者周边板窗资源修正地图追击收益。
6. 支持过山车、电车、荡绳、滑梯、船、地铁等特殊地图资源权重。
7. 将角色原始 1-10 分折算为百分制，提高评分表达清晰度。
8. 新增 `score_calculator` 模块，把各类评分计算从总分析器中拆分出来。
9. 新增 `config/model_params.txt`，支持直接修改模型参数。
10. 右侧输出会自动换行和滚动显示，避免中文、数字、百分比被挤出显示区域。
11. 每次结果会自动保存到 `result/YYYY-MM-DD_HH-MM-SS.txt`。
12. 附带 `data/identityv_scores.xlsx`，用于整理角色、地图、模型参数和资料来源。
13. 附带 `tools/generate_excel.py`、`tools/generate_map_geometry.py` 和 `tools/image_recognition/generate_map_geometry.py`，用于重新生成数据表和地图几何数据。

---

## 与上一版本的主要不同

相比上一版本，本版本的重点变化是：**把评分模型工程化、参数化，并补充特殊地图资源权重**。

| 对比项 | 上一版本 | 当前版本 `v3.3` |
|---|---|---|
| 评分代码结构 | 主要由分析器统一计算 | 新增 `score_calculator`，集中封装各项分数计算 |
| 参数调整方式 | 修改源码后重新编译 | 新增 `config/model_params.txt`，修改参数后下次分析自动读取 |
| 角色能力表达 | 主要使用原始 1-10 分 | 增加百分制折算，1-10 分可映射到 60-100 分 |
| 总分权重 | 权重相对固定 | 总公式权重、角色权重、距离权重、克制权重均可配置 |
| 地图资源 | 主要考虑区域、板窗和路径距离 | 增加过山车、电车、荡绳、滑梯、船、地铁等特殊地图资源 |
| 克制关系 | 标签克制与部分硬编码规则 | 克制权重集中参数化，强调严重克制影响最大 |
| 输出内容 | 输出概率、原因和建议 | 输出概率、寻路距离、角色系数、监管者风格、地图几何、特殊地图、距离、克制、板窗资源和综合分 |
| 结果显示 | 右侧文本可能较长 | 增加固定字符换行和滚动显示，降低显示溢出问题 |
| 工具脚本 | 地图几何生成脚本较基础 | 保留旧入口，并新增 `tools/image_recognition/` 目录便于后续扩展 |
| 项目可维护性 | 算法和流程耦合度更高 | 数据、参数、评分、界面、输出进一步分离 |

简单来说，上一版本更侧重“地图图源 + 几何寻路”，当前版本更侧重“**可配置评分模型 + 模块化算法结构 + 更完整的输出解释**”。

---

## 功能说明

本项目主要实现以下功能：

1. **地图选择**
   - 支持 9 张排位常见地图。
   - 程序左侧显示地图原图。
   - 程序内部根据地图几何数据判断点击位置、墙体、窗、板和距离。

2. **求生者信息输入**
   - 支持依次选择 4 名求生者的位置。
   - 支持依次选择 4 名求生者角色。
   - 地图上会标记 `S1`、`S2`、`S3`、`S4`。
   - 选中角色后，会在地图标记附近显示求生者名字。

3. **监管者信息输入**
   - 支持选择监管者出生点或当前所在位置。
   - 支持选择监管者角色。
   - 地图上会标记 `H` 表示监管者位置。

4. **返回与撤销**
   - 第 1 名求生者位置页可以返回地图选择。
   - 后续求生者位置页可以撤销上一名求生者。
   - 角色选择页可以撤销当前位置选择。
   - 避免选错后必须重启程序。

5. **首追概率计算**
   - 根据求生者角色价值、监管者追击风格、地图几何压力、双方寻路距离、特殊地图资源和角色克制关系进行综合评分。
   - 使用 softmax 将 4 名求生者的原始分转换为概率百分比。
   - 输出监管者对每名求生者的首追意愿。

6. **结果展示**
   - 右侧区域显示完整分析文本。
   - 输出每名求生者的能力指标、出生地、首追概率、寻路距离、计算原因和行动建议。
   - 输出监管者能力分析和对每名求生者的追击判断。

7. **结果保存**
   - 每次运行结束后自动生成分析结果文本。
   - 结果保存到 `result/` 文件夹。
   - 文件名按当前时间生成，不会覆盖上一次结果。

---

## 支持地图

当前版本支持 9 张地图：

| 编号 | 地图 | 区域数 |
|---:|---|---:|
| 1 | 军工厂 | 9 |
| 2 | 圣心医院 | 9 |
| 3 | 红教堂 | 9 |
| 4 | 永眠镇 | 12 |
| 5 | 唐人街 | 9 |
| 6 | 不归林 | 9 |
| 7 | 湖景村 | 12 |
| 8 | 月亮河公园 | 12 |
| 9 | 里奥的回忆 | 9 |

地图区域主要用于内部判断当前位置和生成分析文本。左侧显示的是 `data/map` 文件夹中的地图原图。

---

## 项目文件结构

```text
.
├── D5_firstcaught.sln                         # Visual Studio 解决方案文件
├── D5_firstcaught.vcxproj                     # Visual Studio C++ 工程文件
├── D5_firstcaught.vcxproj.filters             # Visual Studio 文件筛选器
├── config/
│   └── model_params.txt                       # 模型参数配置文件，修改后无需重新编译
├── data/
│   ├── identityv_scores.xlsx                  # 角色、地图、评分和资料来源数据整理表
│   └── map/                                   # 9 张地图图源
├── include/                                   # 头文件目录
│   ├── data_repository.h                      # 静态数据查找接口
│   ├── easyx_ui.h                             # EasyX 图形界面类声明
│   ├── first_chase_analyzer.h                 # 首追分析器类声明
│   ├── first_chase_app.h                      # 程序控制器类声明
│   ├── game_map.h                             # 地图类、区域结构、几何结构声明
│   ├── hunter.h                               # 监管者类和监管者数据结构声明
│   ├── hunter_data.h                          # 监管者静态数据
│   ├── map_data.h                             # 地图区域静态数据
│   ├── map_geometry_data.h                    # 地图几何数据，墙体网格、窗、板和真实比例
│   ├── model_params.h                         # 模型参数结构和读取类声明
│   ├── report_writer.h                        # 结果文本生成与保存类声明
│   ├── score_calculator.h                     # 评分计算类声明
│   ├── survivor.h                             # 求生者类和求生者数据结构声明
│   └── survivor_data.h                        # 求生者静态数据
├── src/                                       # 源文件目录
│   ├── data_repository.cpp                    # 静态数据查找实现
│   ├── easyx_ui.cpp                           # EasyX 绘图、点击输入和界面输出实现
│   ├── first_chase_analyzer.cpp               # 首追分析流程和 softmax 概率计算实现
│   ├── first_chase_app.cpp                    # 程序整体流程控制实现
│   ├── game_map.cpp                           # 地图区域压力、板窗资源、寻路距离实现
│   ├── hunter.cpp                             # 监管者类实现
│   ├── main.cpp                               # 程序入口
│   ├── model_params.cpp                       # 模型参数读取实现
│   ├── report_writer.cpp                      # 结果文本生成、换行和保存实现
│   ├── score_calculator.cpp                   # 角色分、监管者分、地图分、距离分、克制分计算实现
│   └── survivor.cpp                           # 求生者类实现
├── tools/
│   ├── generate_excel.py                      # 重新生成 Excel 数据表
│   ├── generate_map_geometry.py               # 旧地图几何生成入口
│   └── image_recognition/
│       └── generate_map_geometry.py           # 地图图像识别与几何数据生成脚本
├── result/                                    # 运行结果输出目录
└── README.md                                  # GitHub 项目说明文件
```

---

## 运行环境

推荐运行环境如下：

- 操作系统：Windows 10 / Windows 11
- IDE：Visual Studio 2022 及以上 / 兼容版本
- 编译平台：x64
- C++ 标准：C++17
- 字符集：多字节字符集
- 源码编码：建议保持 GB2312 / GBK
- 图形库：EasyX
- 依赖头文件：`graphics.h`
- 依赖库：`EasyXa.lib`
- Python：Python 3
- Python 依赖：`openpyxl`、`Pillow`

安装 Python 依赖：

```bash
pip install openpyxl pillow
```

---

## 编译与运行方法

1. 安装 Visual Studio，并确认已经安装 C++ 桌面开发组件。
2. 安装 EasyX 图形库。
3. 打开项目中的：

```text
D5_firstcaught.sln
```

4. 如果 Visual Studio 提示 `PlatformToolset v145` 不存在，可以右键项目，选择“重定向项目”，改成当前 Visual Studio 已安装的 C++ 工具集。
5. 确认项目平台为 `x64`。
6. 确认字符集为“多字节字符集”。
7. 编译并运行项目。

程序启动后会打开 EasyX 图形窗口，按照右侧提示依次点击即可完成输入。

---

## 使用方法

程序所有输入均通过鼠标点击完成，不需要在终端输入。

输入顺序如下：

1. 在右侧点击本局地图。
2. 在左侧地图原图上点击第 1 名求生者所在位置。
3. 在右侧角色列表中点击第 1 名求生者角色。
4. 重复完成 4 名求生者的位置和角色选择。
5. 在左侧地图原图上点击监管者出生点或当前位置。
6. 在右侧角色列表中点击监管者角色。
7. 查看右侧完整分析结果。
8. 在 `result/` 文件夹中查看自动保存的结果文本。

撤销规则如下：

- 第 1 名求生者位置页点击“返回选地图”，可以重新选择地图。
- 后续求生者位置页点击“撤销上一名”，会删除上一名已确认的求生者。
- 角色选择页点击“撤销位置”，会回到当前角色的位置选择。
- 只有有效地图范围内、外墙内部的白色空地可以点选；点到墙体、建筑、窗、板、门图标、文字、图例、外墙外白底或图片空白边时，程序会要求重新点击。

结果文件命名格式示例：

```text
result/2026-06-15_20-52-10.txt
```

每次输出都会新建一个按当前时间命名的文件，不会覆盖历史结果。

---

## 输出格式说明

右侧“完整分析”区域会按以下格式输出。

求生者部分每人固定三行：

```text
求生者编号 + 求生者名字 + 出生地
能力参数：破译、牵制、辅助、救援的等级、原始分和百分制分
现在应该怎么做
```

监管者部分先输出：

```text
监管者编号 + 监管者名字
监管者追击、守椅、控场、信息能力和强度分析
```

然后对每名求生者固定输出三行：

```text
求生者名称 + 追击意愿概率 + 寻路距离
计算系数原理：角色系数、监管者风格、地图几何、特殊地图、距离、克制、板窗资源、综合分
说明：文字解释和区域建议
```

---

## 类与对象关系说明

本项目整体采用面向对象结构设计，核心思路是将“数据对象”“地图对象”“参数配置”“评分模型”“图形界面”“输出模块”和“程序控制器”分开。

### 1. 程序入口与控制器

```text
main()
└── first_chase_app
    ├── easyx_ui
    ├── data_repository
    ├── first_chase_analyzer
    │   └── score_calculator
    └── report_writer
```

`main.cpp` 中的 `main()` 函数只负责设置控制台编码并创建 `first_chase_app` 对象运行程序。具体业务流程交给 `first_chase_app` 处理。

`first_chase_app` 是程序控制器，负责串联完整流程：

1. 初始化 EasyX 界面。
2. 读取地图选择。
3. 读取 4 名求生者的位置和角色。
4. 读取监管者位置和角色。
5. 调用首追分析器计算结果。
6. 调用结果生成器生成文本。
7. 调用界面类展示最终结果。

---

### 2. 求生者数据结构与求生者类

```text
survivor_static_data
└── survivor
```

`survivor_static_data` 是求生者静态数据结构，用于保存求生者的基础评分和标签。

主要字段包括：

| 字段名 | 含义 |
|---|---|
| `id` | 求生者编号 |
| `name` | 求生者名字 |
| `decode` | 破译能力 |
| `kite` | 牵制能力 |
| `assist` | 辅助能力 |
| `rescue` | 救援能力 |
| `target_value` | 首抓价值 |
| `tags` | 角色标签 |
| `feature` | 角色特点说明 |

`survivor` 类是对求生者静态数据的封装，同时增加当前位置和地图坐标信息，表示该求生者在当前地图中的位置。

主要方法包括：

| 方法名 | 功能 |
|---|---|
| `id()` | 获取求生者编号 |
| `name()` | 获取求生者名字 |
| `decode_score()` | 获取破译能力 |
| `kite_score()` | 获取牵制能力 |
| `assist_score()` | 获取辅助能力 |
| `rescue_score()` | 获取救援能力 |
| `target_value()` | 获取首抓价值 |
| `region_id()` | 获取所在区域编号 |
| `position_x()` | 获取地图横坐标 |
| `position_y()` | 获取地图纵坐标 |
| `has_tag()` | 判断是否具有某个角色标签 |
| `metric_line()` | 生成求生者能力描述 |
| `short_profile()` | 生成简短角色信息 |
| `advice()` | 生成行动建议 |

---

### 3. 监管者数据结构与监管者类

```text
hunter_static_data
└── hunter
```

`hunter_static_data` 是监管者静态数据结构，用于保存监管者的能力评分和标签。

主要字段包括：

| 字段名 | 含义 |
|---|---|
| `id` | 监管者编号 |
| `name` | 监管者名字 |
| `chase` | 追击能力 |
| `camp` | 守椅能力 |
| `control` | 控场能力 |
| `info` | 信息能力 |
| `extra_power` | 版本强势度 / 综合修正值 |
| `tags` | 监管者标签 |
| `feature` | 监管者特点说明 |

`hunter` 类是对监管者静态数据的封装，用于提供监管者能力读取、标签判断和强度描述。

主要方法包括：

| 方法名 | 功能 |
|---|---|
| `id()` | 获取监管者编号 |
| `name()` | 获取监管者名字 |
| `chase_score()` | 获取追击能力 |
| `camp_score()` | 获取守椅能力 |
| `control_score()` | 获取控场能力 |
| `info_score()` | 获取信息能力 |
| `extra_power()` | 获取综合修正值 |
| `has_tag()` | 判断是否具有某个监管者标签 |
| `strength_line()` | 生成监管者强度分析文本 |

---

### 4. 地图数据结构与地图类

```text
map_region_data
└── map_static_data
    └── game_map
```

`map_region_data` 表示地图中的一个区域。

主要字段包括：

| 字段名 | 含义 |
|---|---|
| `id` | 区域编号 |
| `name` | 区域名称 |
| `x` | 区域横坐标 |
| `y` | 区域纵坐标 |
| `width` | 区域宽度 |
| `height` | 区域高度 |
| `kite_score` | 区域牵制舒适度 |
| `openness` | 区域开阔程度 |
| `decode_value` | 区域电机 / 转点价值 |
| `note` | 区域说明 |

`game_map` 类是对地图静态数据和地图几何数据的封装，用于提供区域查找、区域名称获取、地图压力计算、白点判定、墙体判定、资源点计算和寻路距离计算。

主要方法包括：

| 方法名 | 功能 |
|---|---|
| `find_region()` | 按编号查找区域 |
| `find_region_at()` | 根据坐标查找区域 |
| `region_id_at()` | 根据坐标返回区域编号 |
| `region_name()` | 获取区域名称 |
| `region_pressure()` | 计算区域压力 |
| `position_pressure()` | 计算具体点击点压力 |
| `path_distance_m()` | 计算地图路径距离 |
| `nearby_resource_detail()` | 计算附近板窗资源详情 |
| `nearby_resource_score()` | 计算附近板窗资源分 |
| `is_blocked_point()` | 判断点是否为墙体或障碍 |
| `region_advice()` | 生成区域行动建议 |

---

### 5. 地图几何数据结构

```text
map_geometry_static_data
├── wall_cells
├── windows
└── pallets
```

`map_geometry_static_data` 来自 `include/map_geometry_data.h`，由 `tools/generate_map_geometry.py` 或 `tools/image_recognition/generate_map_geometry.py` 根据 `data/map` 中的地图图源生成。

主要内容包括：

| 字段 | 含义 |
|---|---|
| `map_id` | 地图编号 |
| `source_file` | 地图图源文件名 |
| `real_width_m` | 地图估算真实宽度 |
| `real_height_m` | 地图估算真实高度 |
| `grid_width` | 寻路网格宽度 |
| `grid_height` | 寻路网格高度 |
| `wall_cells` | 墙体 / 障碍网格 |
| `windows` | 窗位置线段 |
| `pallets` | 板位置线段 |

当前版本地图几何数据采用 90 × 90 网格。墙体用于点击限制和寻路避障，窗和板用于计算当前位置附近的牵制资源。

---

### 6. 模型参数类

```text
model_params
└── model_params_loader
```

`model_params` 用于保存模型中所有可调参数，包括总公式权重、百分制折算区间、角色指标权重、监管者风格权重、地图几何权重、特殊地图资源权重和克制关系权重。

`model_params_loader` 负责读取：

```text
config/model_params.txt
```

该文件采用 `key=value` 格式，`#` 后内容为注释。程序每次分析前都会读取一次配置，因此修改参数后不需要重新编译。

---

### 7. 评分计算类

```text
score_calculator
└── score_breakdown
```

`score_calculator` 集中管理所有和首追分数有关的计算，避免 `first_chase_analyzer` 过于臃肿。

`score_breakdown` 保存单名求生者的评分拆解结果。

主要字段包括：

| 字段名 | 含义 |
|---|---|
| `character_score` | 角色指标分 |
| `hunter_score` | 监管者风格分 |
| `map_score` | 地图几何分 |
| `distance_score` | 寻路距离分 |
| `counter_score` | 克制关系分 |
| `raw_score` | 原始总分 |
| `probability` | softmax 后的首追概率 |
| `path_distance_m` | 寻路距离，单位为米 |
| `nearby_window_count` | 附近窗数量 |
| `nearby_pallet_count` | 附近板数量 |
| `nearby_resource_score` | 附近板窗资源分 |
| `special_map_score` | 特殊地图资源分 |
| `reason` | 计算原因 |
| `best_action` | 行动建议 |

---

### 8. 首追分析器

```text
first_chase_analyzer
├── score_calculator
└── analysis_result
```

`first_chase_analyzer` 是首追分析流程类，只负责遍历四名求生者、调用 `score_calculator` 计算每个人的原始分、使用 softmax 做概率化，并确定最可能被首追的目标。

`analysis_result` 保存一次完整分析的结果，包括地图、监管者、监管者位置、四名求生者、评分结果和最推荐首追目标。

---

### 9. EasyX 图形界面类

```text
easyx_ui
├── map_marker
├── click_area
├── number_input_result
└── position_input_result
```

`easyx_ui` 负责图形界面、地图绘制、编号列表绘制、鼠标点击输入、撤销按钮、结果页滚动和最终结果显示。

`map_marker` 用于在地图上标记求生者或监管者位置。

主要字段包括：

| 字段名 | 含义 |
|---|---|
| `region_id` | 标记所在区域编号 |
| `label` | 标记文本，例如 `S1`、`S2`、`H` |
| `display_name` | 角色显示名称 |
| `color` | 标记颜色 |
| `x` | 标记地图横坐标 |
| `y` | 标记地图纵坐标 |
| `has_position` | 是否使用精确坐标显示 |

---

### 10. 数据仓库与结果输出

`data_repository` 提供静态数据查找接口，负责根据编号查找求生者、监管者和地图数据。

`report_writer` 负责生成输出文本、固定字符换行和保存结果文件。

---

## 采用的算法说明

本项目不是简单地用固定规则判断首追目标，而是综合使用了多种算法和计算方法。

### 1. 加权评分算法

程序会对每名求生者分别计算多个维度的分数，然后按权重合成原始分。

主要评分项包括：

| 分值 | 作用 |
|---|---|
| `character_score` | 衡量求生者自身是否值得首追 |
| `hunter_score` | 衡量监管者风格是否适合追该求生者 |
| `map_score` | 衡量当前位置是否容易被追 |
| `special_map_score` | 衡量特殊地图资源对首追收益的影响 |
| `distance_score` | 衡量监管者到该求生者的寻路距离 |
| `counter_score` | 衡量监管者和求生者之间的克制关系 |

总分公式如下：

```cpp
raw_score = raw_base
    + character_score * character_weight
    + hunter_score * hunter_style_weight
    + (map_score + special_map_score) * map_weight
    + distance_score * distance_weight
    + counter_score * counter_weight;
```

默认参数中，克制关系权重最高，角色指标和监管者风格次之，地图和距离作为综合修正。具体数值可以在 `config/model_params.txt` 中修改。

---

### 2. 百分制折算算法

求生者和监管者的能力原始数据使用 1-10 分。为了让不同角色之间的差异更明显，程序会把 1-10 分折算为百分制区间。

默认折算规则：

```text
1 分  -> 60 分
10 分 -> 100 分
```

折算公式为：

```text
percent_score = percent_min + (raw_score - 1) × (percent_max - percent_min) / 9
```

其中：

```text
percent_min = 60
percent_max = 100
percent_middle = 80
```

`percent_middle` 作为模型判断“中等水平”的基准线。

---

### 3. 角色指标评分算法

角色指标分主要考虑：

- 破译能力
- 牵制能力
- 辅助能力
- 救援能力
- 首抓价值
- 弱牵制标签

大致逻辑是：破译越高、牵制越弱、首抓价值越高，越容易被监管者优先追击。

示意公式：

```text
角色指标分 =
    破译压力
  + 首抓价值压力
  + 弱牵制压力
  + 辅助位压力
  + 救援位压力
  + 特殊标签修正
```

同时，救援位如果本身牵制强，开局硬追通常不划算，因此会额外扣分；弱牵制角色会额外加分。

---

### 4. 监管者风格匹配算法

监管者风格分用于判断监管者的能力类型是否适合追某个求生者。

主要考虑：

- 监管者追击能力和求生者牵制能力的差值
- 监管者综合强度修正
- 监管者控场 / 信息能力对破译位的压制
- 监管者守椅能力对救援位的影响
- 监管者控场能力对辅助位的影响
- 慢追击监管者追强牵制角色的风险
- 快追击监管者追弱牵制角色的收益

例如，追击能力强的监管者更愿意抓弱牵制角色；控场和信息能力强的监管者更重视高破译角色。

---

### 5. 角色克制标签算法

程序为监管者和求生者设置了标签，例如：

```text
anti_mobility
anti_item
anti_heal
anti_decoder
area_control
long_range
teleport
anti_loop
decoder
mobility
item_dependent
healer
stunner
kiter
weak_kiter
portal
stealth
rescue
assist
```

当监管者标签和求生者标签形成克制关系时，会增加追击分数。

例如：

- `anti_mobility` 克制 `mobility`
- `anti_item` 克制 `item_dependent`
- `anti_heal` 克制 `healer`
- `anti_decoder` 克制 `decoder`
- `anti_loop` 克制 `kiter`
- `teleport` 对 `stealth` 有一定压制
- `camp` 对 `rescue` 有一定压制

此外，部分特殊监管者和求生者类型还设置了额外修正，例如摄影师、疯眼、隐士对破译位的压制，或强牵制角色面对慢追击监管者时降低首追收益。

---

### 6. 地图区域压力算法

地图区域压力主要来自 `map_data.h` 中的区域评分。

每个区域都有：

- `kite_score`：牵制舒适度
- `openness`：开阔程度
- `decode_value`：电机 / 转点价值

基础逻辑是：

```text
区域牵制越弱，首追分越高；
区域越开阔，首追分越高；
区域电机价值越高，压制收益越高。
```

示意公式：

```text
region_pressure =
    (10 - kite_score) × 1.8
  + (openness - 5) × 1.0
  + (decode_value - 5) × 0.9
```

---

### 7. 附近板窗资源修正算法

当前版本不只看区域，还会看点击点附近是否有窗和板。

程序从 `map_geometry_data.h` 中读取 `windows` 和 `pallets`，将窗和板看作地图线段，然后计算点击点到这些线段的距离。

主要逻辑是：

- 离窗越近，说明求生者附近有可用牵制资源，追击风险降低。
- 离板越近，说明求生者附近有可用板区资源，追击风险降低。
- 窗和板的资源分会降低当前位置的首追收益。
- 资源分有上限，避免一个点附近资源过多导致评分失真。

---

### 8. 特殊地图资源评分算法

当前版本增加了特殊地图资源对首追判断的影响。

主要资源包括：

| 地图 | 特殊资源 |
|---|---|
| 月亮河公园 | 过山车、滑梯、桥区 |
| 永眠镇 | 电车站 |
| 湖景村 | 大船 / 小船区域 |
| 不归林 | 树屋、荡绳、河道等转点资源 |

这些资源通常帮助求生者快速转点，因此默认会降低监管者首追意愿。如果监管者具有强机动、传送或反位移能力，则会抵消部分特殊地形收益。

---

### 9. 地图图像识别与几何数据生成算法

当前版本使用 `tools/image_recognition/generate_map_geometry.py` 从地图图源中提取几何信息，同时保留 `tools/generate_map_geometry.py` 作为旧入口。

主要步骤包括：

1. 对地图图片设置实际对局区域裁剪框。
2. 将裁剪后的地图划分为 90 × 90 网格。
3. 根据像素颜色判断墙体区域。
4. 根据蓝色像素识别窗。
5. 根据绿色像素识别板。
6. 使用连通块算法合并同一段窗或板。
7. 将窗、板和墙体网格转换为 0-1000 相对地图坐标。
8. 生成 `include/map_geometry_data.h`，供 C++ 程序使用。

其中，窗和板识别使用的是颜色阈值和连通块统计；墙体识别使用灰度、颜色差和网格内墙体像素占比判断。

---

### 10. 点击合法性判定算法

为了避免用户点到墙体、文字、图例、外墙外空白或不可走区域，本项目采用多层点击判定：

1. 判断鼠标是否在地图画布范围内。
2. 将屏幕坐标转换为 0-1000 地图相对坐标。
3. 判断是否在地图有效裁剪范围内。
4. 判断原图对应像素是否接近白色可走区域。
5. 判断是否属于外墙外侧的大块白底。
6. 判断该点对应的几何网格是否为墙体或障碍。
7. 只有通过全部判定，才允许作为求生者或监管者位置。

这样可以尽量避免点到不可走区域，提升分析输入的合理性。

---

### 11. 最短路径 / 寻路距离算法

当前版本的距离不使用区域中心点直线距离，而是使用地图网格进行寻路。

主要步骤：

1. 将监管者位置和求生者位置转换为 90 × 90 网格下的起点和终点。
2. 如果起点或终点落在墙体网格上，就用 BFS 思想寻找最近的可走网格。
3. 使用带优先队列的 Dijkstra 最短路径算法计算两点之间的路径距离。
4. 支持 8 个方向移动，包括上下左右和斜向移动。
5. 斜向移动时检查两侧相邻格，避免从墙角斜穿过去。
6. 每一步距离根据地图估算真实宽高换算为米制距离。
7. 如果寻路失败，则退回到欧氏距离并乘以修正系数。

距离越远，监管者开局首追该求生者的意愿越低。

距离修正逻辑：

```text
distance_score = - path_distance_m / distance_divisor
```

---

### 12. Softmax 概率归一化算法

4 名求生者都会得到一个原始分 `raw_score`。为了将原始分转换成首追概率，程序使用 softmax 算法。

公式为：

```text
probability_i = exp((score_i - best_score) / softmax_divisor)
              / sum(exp((score_j - best_score) / softmax_divisor))
```

其中减去 `best_score` 是为了避免指数计算过大；`softmax_divisor` 用于控制概率差距，不让小分差被过度放大。

最终输出为百分比，表示监管者更可能首追哪名求生者。

---

### 13. 参数文件解析算法

`model_params_loader` 会读取 `config/model_params.txt`，按行解析参数。

解析规则如下：

1. 去掉 `#` 后面的注释。
2. 判断当前行是否包含 `=`。
3. 提取 `key=value`。
4. 去掉首尾空白字符。
5. 将数值转成 `double`。
6. 根据参数名写入 `model_params`。
7. 未知参数会被忽略，便于后续扩展。

这个设计使得用户修改模型权重时不需要重新编译代码。

---

### 14. 文本自动换行与滚动显示算法

右侧结果区内容较多，程序会对中文文本进行自动换行处理。

实现思路：

1. 按 GBK 中文字符宽度拆分文本。
2. 使用 EasyX 的文字宽度判断当前行能否继续加入字符。
3. 超出宽度时自动换行。
4. 结果页支持滚动查看，避免长文本显示不完整。
5. 输出文本还会限制每行最多约 50 个字符，避免中文和数字被挤出显示区域。

---

## 参数配置说明

模型参数集中放在：

```text
config/model_params.txt
```

这个文件里每一项都有中文注释，可以直接修改权重，不需要重新编译。程序每次分析都会重新读取一次配置文件。

常用参数包括：

| 参数 | 作用 |
|---|---|
| `raw_base` | 原始基础分 |
| `character_weight` | 角色指标权重 |
| `hunter_style_weight` | 监管者风格匹配权重 |
| `map_weight` | 地图区域、板窗、特殊资源权重 |
| `distance_weight` | 寻路距离权重 |
| `counter_weight` | 克制关系权重 |
| `softmax_divisor` | 控制最终概率差距大小 |
| `percent_min` / `percent_max` | 控制 1-10 分折算到百分制的区间 |
| `distance_divisor` | 控制距离惩罚强度 |
| `resource_cap` | 附近板窗资源分上限 |
| `coaster_score` / `tram_score` / `rope_score` | 特殊地图资源分 |
| `counter_tag_...` | 标签克制权重 |

---

## 数据口径说明

项目中的求生者、监管者和地图评分主要为程序建模使用的主观量化数据，不是官方强度表，也不代表固定版本答案。

数据来源主要包括：

- 游戏公开资料整理
- 角色机制理解
- 排位地图区域理解
- 用户提供的 `data/map` 几何地图图源
- 程序建模需要
- 手工整理的评分表 `data/identityv_scores.xlsx`

可修改的数据文件包括：

```text
include/survivor_data.h
include/hunter_data.h
include/map_data.h
include/map_geometry_data.h
config/model_params.txt
data/identityv_scores.xlsx
```

可修改的权重和公式主要位于：

```text
config/model_params.txt
src/score_calculator.cpp
src/first_chase_analyzer.cpp
```

修改数据后，可运行：

```bash
python tools/generate_excel.py
```

重新生成 `data/identityv_scores.xlsx`。

如果更换了 `data/map` 中的地图图片，应先运行：

```bash
python tools/generate_map_geometry.py
```

再运行 Excel 生成脚本。

---

## 变量命名说明

本项目变量命名以“含义清楚、模块明确、便于维护”为主要原则。

### 1. 类名命名

类名主要采用小写英文和下划线组合：

| 类名 | 含义 |
|---|---|
| `survivor` | 求生者类 |
| `hunter` | 监管者类 |
| `game_map` | 地图类 |
| `model_params_loader` | 模型参数读取类 |
| `score_calculator` | 评分计算类 |
| `first_chase_analyzer` | 首追分析器类 |
| `first_chase_app` | 程序控制器类 |
| `easyx_ui` | EasyX 图形界面类 |
| `report_writer` | 结果文本输出类 |

### 2. 结构体命名

结构体主要用于保存静态数据和分析结果：

| 结构体名 | 含义 |
|---|---|
| `survivor_static_data` | 求生者静态数据 |
| `hunter_static_data` | 监管者静态数据 |
| `map_region_data` | 地图区域数据 |
| `map_static_data` | 地图静态数据 |
| `map_geometry_static_data` | 地图几何数据 |
| `model_params` | 模型参数 |
| `score_breakdown` | 单名求生者评分拆解 |
| `analysis_result` | 一次完整分析结果 |
| `map_marker` | 地图标记 |
| `click_area` | 鼠标点击区域 |
| `number_input_result` | 编号输入结果 |
| `position_input_result` | 位置输入结果 |
| `map_resource_detail` | 附近板窗资源详情 |

### 3. 成员变量命名

类中的私有成员变量大多使用结尾下划线，表示这是类内部保存的状态。

| 变量名 | 含义 |
|---|---|
| `data_` | 当前对象持有的静态数据 |
| `region_id_` | 当前求生者所在区域编号 |
| `position_x_` | 当前求生者地图横坐标 |
| `position_y_` | 当前求生者地图纵坐标 |
| `click_areas_` | 当前界面所有可点击区域 |
| `result_cache_` | 当前结果缓存 |
| `result_scroll_line_` | 结果页滚动行号 |
| `result_max_scroll_line_` | 结果页最大滚动行号 |
| `calculator_` | 首追分析器内部的评分计算对象 |

### 4. 评分变量命名

| 变量名 | 含义 |
|---|---|
| `character_score` | 角色指标分 |
| `hunter_score` | 监管者风格分 |
| `map_score` | 地图几何分 |
| `special_map_score` | 特殊地图资源分 |
| `distance_score` | 寻路距离分 |
| `counter_score` | 克制关系分 |
| `raw_score` | 原始总分 |
| `probability` | 首追概率 |
| `best_index` | 最优首追目标下标 |
| `path_distance_m` | 寻路距离，单位为米 |
| `nearby_resource_score` | 附近板窗资源分 |

### 5. 区域与对象变量命名

| 变量名 | 含义 |
|---|---|
| `map_id` | 地图编号 |
| `hunter_id` | 监管者编号 |
| `survivor_id` | 求生者编号 |
| `hunter_region_id` | 监管者所在区域编号 |
| `survivor_region_id` | 求生者所在区域编号 |
| `hunter_position` | 监管者地图坐标 |
| `survivor_position` | 求生者地图坐标 |
| `game_map_obj` | 当前地图对象 |
| `hunter_role` | 当前监管者对象 |
| `survivor_role` | 当前求生者对象 |
| `survivors` | 四名求生者对象数组 |
| `markers` | 地图标记数组 |

### 6. 静态数据常量命名

静态数据表使用 `k_` 前缀，表示常量数据集合。

| 常量名 | 含义 |
|---|---|
| `k_survivor_data` | 求生者静态数据表 |
| `k_hunter_data` | 监管者静态数据表 |
| `k_map_data` | 地图区域数据表 |
| `k_map_geometry_data` | 地图几何数据表 |
| `k_map_play_crops` | 地图有效裁剪区域数据 |

---

## 命名风格说明

本项目命名风格主要遵循以下规则：

1. **类名和结构体名使用小写英文与下划线**
   - 例如：`first_chase_analyzer`、`score_breakdown`、`score_calculator`

2. **函数名尽量体现动作或返回内容**
   - `find_survivor_data()` 表示查找求生者数据
   - `draw_result()` 表示绘制结果
   - `build_output_text()` 表示生成输出文本
   - `distance_penalty()` 表示计算距离惩罚
   - `path_distance_m()` 表示计算地图路径距离
   - `nearby_resource_score()` 表示计算附近资源保护分
   - `load()` 表示读取模型参数

3. **成员变量使用下划线结尾**
   - 例如：`data_`、`region_id_`、`click_areas_`、`calculator_`

4. **评分变量直接使用含义命名**
   - 例如：`character_score`、`counter_score`、`raw_score`

5. **静态数据常量使用 `k_` 前缀**
   - 例如：`k_survivor_data`、`k_hunter_data`、`k_map_data`

---

## 设计特点

### 1. 主函数较简洁

`main.cpp` 只负责设置编码并启动 `first_chase_app`，避免所有逻辑堆在主函数中。

### 2. 输入输出和算法分离

`easyx_ui` 只负责界面，`first_chase_analyzer` 和 `score_calculator` 负责算法，`report_writer` 负责结果文本，便于后续修改和维护。

### 3. 静态数据集中管理

求生者、监管者、地图区域和地图几何数据分别放在独立头文件中，便于后续更新角色、地图和评分。

### 4. 模型参数外置

模型权重集中放在 `config/model_params.txt` 中，修改参数后无需重新编译程序。

### 5. 支持图形化点击输入

用户不需要在控制台输入编号，直接通过 EasyX 界面点击位置和角色即可完成输入。

### 6. 支持地图几何路径计算

当前版本能够根据墙体网格计算路径距离，而不是只根据直线距离判断远近。

### 7. 输出结果可保存

每次分析结果都会保存为独立文本文件，便于复盘和对比。

---

## 当前不足与未完成内容

当前版本核心功能已经完成，但仍有一些没有完成或可以继续完善的地方：

1. **没有自动读取 Excel 数据**
   - `data/identityv_scores.xlsx` 目前主要作为整理表。
   - 程序实际使用的数据仍写在 `include/survivor_data.h`、`include/hunter_data.h`、`include/map_data.h` 和 `include/map_geometry_data.h` 中。
   - 后续可以增加从 Excel、CSV 或 JSON 自动读取数据的功能。

2. **权重已有配置文件，但仍没有图形化设置界面**
   - 当前可以通过 `config/model_params.txt` 修改权重。
   - 后续可以增加“权重设置界面”，允许用户在程序中直接调整模型参数。

3. **地图几何识别仍有主观修正**
   - 当前地图几何数据来自图片颜色识别和手动裁剪框。
   - 对部分地图的特殊道路、白底、图标和复杂建筑区域仍可能需要人工修正。

4. **暂未支持对局录像或截图自动识别**
   - 当前需要用户手动选择角色和位置。
   - 后续可以扩展为读取截图、录像或 OCR 识别，但当前版本未实现。

5. **结果输出格式仍可扩展**
   - 当前主要输出 `.txt` 文本文件。
   - 后续可以增加 Markdown、HTML、CSV 或图片报告导出。

6. **跨平台能力有限**
   - 当前依赖 Windows 和 EasyX。
   - Linux、macOS 暂不支持直接运行。

7. **数据版本需要持续维护**
   - 《第五人格》角色、地图和版本环境会更新。
   - 本项目评分数据需要根据实际版本继续调整。

8. **模型仍属于主观策略模型**
   - 当前模型根据角色机制、地图理解和手工权重进行估计。
   - 后续可以结合真实对局数据进行统计校准。

---

## 预留开发接口说明

本项目已经预留了较清晰的模块结构，后续可以基于以下位置继续开发：

| 预留位置 | 可扩展方向 |
|---|---|
| `data_repository` | 改造成从 CSV、JSON、Excel 或数据库读取角色与地图数据 |
| `model_params` | 增加更多可配置参数，例如版本环境、段位环境、阵容修正 |
| `score_calculator` | 调整评分公式、增加新权重、加入角色组合和出生点联动 |
| `first_chase_analyzer` | 增加换追建议、控场路线推荐、多目标追击收益对比 |
| `score_breakdown` | 增加更多评分项，例如开局刷点强度、出生点联动、求生者阵容配合 |
| `analysis_result` | 增加推荐追击路线、换追建议、控场建议 |
| `game_map` | 优化地图坐标、增加区域连通关系、转点路线计算 |
| `map_geometry_data` | 增加更多地图障碍、门、地下室、电机点和地窖点数据 |
| `easyx_ui` | 增加设置界面、结果导出按钮、重新分析按钮 |
| `report_writer` | 扩展为 Markdown、HTML、CSV、图片报告输出 |
| `survivor_data.h` | 增加新求生者、调整角色标签和评分 |
| `hunter_data.h` | 增加新监管者、调整监管者标签和评分 |
| `map_data.h` | 增加新地图、优化地图区域数据 |
| `tools/generate_map_geometry.py` | 继续优化墙体、窗、板识别规则 |
| `tools/generate_excel.py` | 继续完善数据表字段和资料来源表 |
| `first_chase_app` | 增加完整流程管理，例如重新开始、返回上一步、保存配置 |

建议后续优先补充“外部数据读取”和“图形化权重配置”功能，使项目不需要每次修改源码就能调整角色数据和模型参数。

---

## 适用场景

本项目适合用于：

- C++ 面向对象程序设计练习
- EasyX 图形界面练习
- 游戏数据分析模型展示
- 简单策略推荐系统练习
- 地图网格寻路算法练习
- 参数化模型设计练习
- 课程作业或个人项目展示
- GitHub 项目作品集整理

---

## 注意事项

1. 本项目为学习和练习性质，不是《第五人格》官方工具。
2. 项目中的角色评分、地图评分和权重为主观建模数据，不代表官方强度。
3. 输出结果只能作为娱乐、学习和分析参考，不应作为真实对局中的绝对判断。
4. 项目没有进行完整的异常输入、跨平台兼容和长期稳定性测试。
5. 如果需要用于正式展示、比赛或二次开发，请自行进行充分修改和测试。
6. 作者不对任何使用、修改、传播、运行本项目代码后产生的后果负责。

---

## 版权与使用声明

### 项目版权说明

本项目代码和文档仅用于学习、交流和参考。项目中涉及的《第五人格》名称、角色名称、地图名称、游戏机制、图像资料和相关游戏内容，其权利归原权利方所有。本项目不是官方项目，也不代表官方立场。

### 使用许可

本项目无需额外授权即可查看、学习、参考、修改和使用，但必须遵守以下要求：

1. **不得将本项目占为己有。**
2. **不得删除或故意隐藏项目来源说明。**
3. **如在作业、报告、课程设计、博客、仓库或其他公开内容中使用本项目内容，必须明确标注出处。**
4. **不得将本项目直接作为自己的原创课程作业或原创项目提交。**
5. **不得利用本项目冒充官方工具、官方数据或官方结论。**
6. **涉及《第五人格》的名称、角色、地图、图片和相关内容时，应尊重原权利方权益。**
7. **使用者因复制、修改、运行、提交或传播本项目所产生的一切后果，由使用者自行承担。**
8. **作者不对任何使用后果负责，包括但不限于课程查重、成绩评定、代码错误、运行异常、数据丢失、版权争议等问题。**

建议引用格式：

```text
参考项目：Identity V First Chase Analyzer / 第五人格监管者首抓分析器
项目性质：C++ / EasyX 学习项目
来源：GitHub 仓库 README 或原项目地址
```

---

## 联系方式

如需交流或反馈问题，可通过以下邮箱联系：

```text
15723209543@163.com
```

---

## 免责声明

本项目仅作为 C++ 程序设计学习资料、EasyX 图形界面练习和游戏数据建模参考，不保证代码在所有环境下均可正常运行，也不保证分析结果符合真实游戏环境。

任何人使用、复制、修改、运行或传播本项目，即表示已理解并接受本 README 中的版权声明、使用限制和免责声明。
