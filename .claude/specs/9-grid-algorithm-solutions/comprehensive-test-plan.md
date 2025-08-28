# 9宫格算法综合比较测试

## 测试棋盘配置

### 棋盘1: 简单全G (30个G)
```
GGGGGG
GGGGGG
GGGGGG
GGGGGG
GGGGGG
```

### 棋盘2: 混合颜色RGBGGG模式 (20个G)
```
RGBGGG
RGBGGG
RGBGGG
RGBGGG
RGBGGG
```

### 棋盘3: 随机混合 (15个G)
```
GRRGBG
RGBGRG
BGGGRR
RGBGBG
GGRGBR
```

## 算法列表

1. **贪心填充算法** (Greedy Fill)
   - 策略: 为每个空位置找到最近的可用珠子
   - 优势: 简单直接，计算效率高
   - 劣势: 可能不是最优路径

2. **中心扩散算法** (Center Expansion)
   - 策略: 从9宫格中心开始，向外扩散填充
   - 优势: 保证中心位置优先，结构更稳定
   - 劣势: 可能需要更多移动步数

3. **分治算法** (Divide and Conquer)
   - 策略: 将3x3网格分为4个子区域，逐个解决
   - 优势: 问题分解，降低复杂度
   - 劣势: 子区域间可能存在依赖关系

4. **分布式聚集算法** (Distributed Clustering)
   - 策略: 两阶段算法，先聚集珠子再精确排列
   - 优势: 理论上更系统化
   - 劣势: 复杂，容易出现路径验证问题

## 测试结果总结

### 成功标准
- ✅ 在30步内完成9宫格形成
- ✅ 路径验证通过
- ✅ 算法成功执行并返回有效路径

### 测试执行
```bash
# 测试棋盘1
pazusoba_v1.exe "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGG" 3 30 1000 --nine-force=G --verbose

# 测试棋盘2  
pazusoba_v1.exe "RGBGGGRGBGGGRGBGGGRGBGGGRGBGGG" 3 30 1000 --nine-force=G --verbose

# 测试棋盘3
pazusoba_v1.exe "GRRGBGRGBGRGBGGGRRRGBGBGGGRGBR" 3 30 1000 --nine-force=G --verbose
```

## 预期结果分析

### 简单情况 (棋盘1)
- 所有算法都应该成功
- 贪心算法应该最快（0-1步）
- 其他算法可能需要少量移动

### 复杂情况 (棋盘2和3)
- 贪心算法: 可能成功，路径较短但验证可能失败
- 中心扩散: 中等成功率，结构化较好
- 分治算法: 中等成功率，需要更多步数
- 分布式算法: 当前验证有问题，可能失败

## 性能指标

1. **成功率**: 算法能找到有效解决方案的比例
2. **步数效率**: 完成所需的移动步数
3. **计算时间**: 算法执行时间
4. **验证通过率**: 生成路径通过验证的比例