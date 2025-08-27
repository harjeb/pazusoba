# MSBuild 编译指南

## 文件说明

- `pazusoba.sln` - Visual Studio 解决方案文件
- `pazusoba_v1.vcxproj` - V1版本项目文件（扩展功能版）
- `pazusoba_v2.vcxproj` - V2版本项目文件
- `build.bat` - 自动化构建脚本（推荐使用）
- `build_simple.bat` - 简化版构建脚本
- `test_features.bat` - 功能测试脚本

## 编译方法

### 方法1: 使用自动化脚本（推荐）
```batch
build.bat
```
这个脚本会：
- 自动检测并设置Visual Studio环境
- 创建输出目录
- 编译Debug和Release版本

### 方法2: 使用Visual Studio开发者命令提示符
1. 打开"Visual Studio Developer Command Prompt"
2. 导航到项目目录
3. 运行：
```batch
build_simple.bat
```

### 方法3: 直接使用MSBuild
在Visual Studio开发者命令提示符中：
```batch
msbuild pazusoba.sln /p:Configuration=Release /p:Platform=x64
```

## 输出文件
- Debug版本: `bin\Debug\`
- Release版本: `bin\Release\`

## 项目结构
- **V1版本**包含完整的solver实现和扩展功能（推荐用于功能开发）
  - 颜色优先级支持
  - 十字类型优先级
  - 9宫格类型优先级
  - 斜向移动控制
  - 显示选项控制
- V2版本是更新的核心实现（基础功能）

## 新增源文件
V1项目现在包含以下新文件：
- `core/v1/solver_config.h` - 配置系统头文件
- `core/v1/solver_config.cpp` - 配置系统实现

## 测试扩展功能

编译完成后，运行测试脚本验证扩展功能：
```batch
test_features.bat
```

测试新的多行棋盘显示：
```batch
test_multiline.bat
```

或手动测试：
```batch
# 基本功能测试（新的多行格式）
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG"

# 扩展功能测试
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG" --colors=RB --verbose
```

## 新显示格式

程序现在使用多行并排显示棋盘变化：
```
Initial Board:     Final Board:
RHGHDR       ->       RGLDHR
GGBBGG       ->       GBBGGG
DBLLHB       ->       DBLLHB
GGGRLH       ->       GGGRLH
GHDGLG       ->       GHDGLG
```

## 编译要求
- Visual Studio 2019或2022
- 目标平台为x64
- 使用C++17标准

## 常见编译问题

### 问题1: 缺少getScore或printFinalBoard方法
**错误消息:** `E0135 类 "Route" 没有成员 "getScore"`

**解决方案:**
1. 使用 `clean_rebuild.bat` 进行完全重建：
   ```batch
   clean_rebuild.bat
   ```

2. 或者在Visual Studio中：
   - 右键解决方案 → "清理解决方案"
   - 然后 "重新生成解决方案"

3. 如果仍有问题，关闭Visual Studio，删除以下文件夹后重新打开：
   - `bin/` 目录
   - `obj/` 目录
   - `.vs/` 目录（如果存在）

**原因:** 这通常是Visual Studio IntelliSense缓存问题，实际编译应该成功。

### 问题2: 找不到solver_config.h
确保项目文件包含了新的源文件，重新生成解决方案。

### 问题3: 链接错误
确保所有.cpp文件都被正确编译。

## 功能对比

| 功能 | V1版本 | V2版本 |
|------|--------|--------|
| 基础求解 | ✅ | ✅ |
| 颜色优先级 | ✅ | ❌ |
| 十字优先级 | ✅ | ❌ |
| 9宫格优先级 | ✅ | ❌ |
| 斜向移动控制 | ✅ | ❌ |
| 显示控制 | ✅ | ❌ |
| 详细参数 | ✅ | ❌ |

**推荐使用V1版本**获得完整的功能体验。