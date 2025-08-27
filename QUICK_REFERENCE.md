# Pazusoba Solver V1 - 快速参考

## 🚀 快速开始

```bash
# 基本使用
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG"

# 查看所有选项
pazusoba_v1.exe --help
```

## 📊 新的显示格式

**默认输出：**
```
Pazusoba Solver V1 (Extended) starting...
Solver initialized, starting solve...
Combo: 8
Path: (2, 3) ULLDR [5 steps]

Initial Board:     Final Board:
RHGHDR       ->       RGLDHR
GGBBGG       ->       GBBGGG
DBLLHB       ->       DBLLHB
GGGRLH       ->       GGGRLH
GHDGLG       ->       GHDGLG
Solve completed, found 3 routes.
Program finished successfully.
```

**显示说明：**
1. **路径信息** - 除非使用 `--no-path`
2. **棋盘变化** - 多行并排显示初始和最终棋盘 (默认显示)
3. **初始棋盘** - 仅在 `--verbose` 模式下在开始显示
4. **详细最终棋盘** - 仅在 `--verbose + --final-board` 时显示

**显示控制：**
- `--no-transform` - 不显示棋盘变化对比
- `--no-board` - 不显示详细最终棋盘状态
- `--no-path` - 不显示路径信息  
- `--no-score` - 不显示分数（需配合--verbose）
- `--verbose` - 显示详细信息（初始棋盘、配置、分数等）

## 📋 参数速查

| 参数 | 说明 | 示例 |
|------|------|------|
| `--colors=COLORS` | 颜色优先级 | `--colors=RB` |
| `--plus=COLORS` | 十字优先级 | `--plus=L` |
| `--nine=COLORS` | 9宫格优先级 | `--nine=G` |
| `--no-diagonal` | 禁用斜向移动 | `--no-diagonal` |
| `--no-transform` | 不显示棋盘变化 | `--no-transform` |
| `--no-board` | 不显示详细最终棋盘 | `--no-board` |
| `--no-path` | 不显示路径 | `--no-path` |
| `--no-score` | 不显示分数 | `--no-score` |
| `--verbose` | 详细输出 | `--verbose` |

## 🎨 颜色代码

| 代码 | 颜色 | 代码 | 颜色 |
|------|------|------|------|
| `R` | 红/火 | `L` | 光 |
| `B` | 蓝/水 | `D` | 暗 |
| `G` | 绿/木 | `H` | 回复 |

## ⭐ 常用组合

```bash
# 默认模式 (新格式: 显示路径 + 多行棋盘对比)
pazusoba_v1.exe "棋盘字符串"

# 颜色优先 + 详细输出 (显示初始棋盘)
pazusoba_v1.exe "棋盘字符串" --colors=RBG --verbose

# 十字优先 + 禁用斜向
pazusoba_v1.exe "棋盘字符串" --plus=L --no-diagonal

# 全功能组合 + 详细信息
pazusoba_v1.exe "棋盘字符串" --colors=RB --plus=L --nine=G --verbose

# 极简模式（只显示基本进度信息）
pazusoba_v1.exe "棋盘字符串" --no-path --no-transform --no-board

# 只要棋盘变化结果（多行对比）
pazusoba_v1.exe "棋盘字符串" --no-path
```

## 🛠️ 编译

```batch
# Windows (推荐)
build.bat

# 或使用VS开发者命令提示符
build_simple.bat
```

## 🧪 测试

```batch
# 运行功能测试
test_features.bat
```

---
详细文档：`EXTENDED_USAGE_GUIDE.md`