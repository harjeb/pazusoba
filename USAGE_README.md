# 运行说明

## 问题解决

如果运行exe文件没有反应，原因如下：

### V1版本 (pazusoba_v1.exe)
- 程序正常运行但原始版本没有足够的输出信息
- 已更新添加了进度输出
- 程序会处理默认的棋盘字符串并找到最优路径

### V2版本 (pazusoba_v2.exe)
- V2实现不完整，原本只返回0
- 已更新添加了说明性输出

## 使用方法

### 基本运行
```batch
# 使用默认设置运行
pazusoba_v1.exe

# 使用自定义棋盘字符串
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG"

# 设置最小消除数
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGHDGLG" 3

# 设置最大步数
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGHDGLG" 3 50

# 设置搜索大小
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGHDGLG" 3 50 20000
```

### 从文件读取棋盘
```batch
pazusoba_v1.exe "assets/sample_board_65.txt"
```

## 程序输出
程序现在会显示：
- 启动信息
- 求解器初始化状态
- 求解完成信息和找到的路径数量
- 程序完成确认

## 棋盘格式
- 字符串格式：每个字符代表一个珠子类型
- R=红, G=绿, B=蓝, L=光, D=暗, H=回复
- 标准6x5棋盘需要30个字符

## 故障排除
1. **没有输出** - 现在已修复，会显示进度信息
2. **程序卡死** - 正常，AI求解需要时间（几秒到几分钟）
3. **找不到路径** - 检查棋盘字符串格式是否正确
4. **内存不足** - 减少搜索大小参数（第4个参数）