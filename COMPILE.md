# 编译指南

## 编译 pazusoba.cpp

使用以下命令编译主程序：

```bash
C:/msys64/ucrt64/bin/g++.exe" -std=c++14 -O2 -Iinclude -pthread support/main.cpp src/pazusoba.cpp -o pazusoba.exe"
```

## 编译参数说明

- `-std=c++14`: 使用 C++14 标准
- `-O2`: 优化级别 2
- `-Iinclude`: 包含 include 目录中的头文件
- `-pthread`: 启用多线程支持
- `support/main.cpp`: 主程序入口文件
- `src/pazusoba.cpp`: 核心算法实现文件
- `-o pazusoba.exe`: 输出可执行文件名

## 注意事项

- 必须使用 `cmd /c` 方式执行，直接调用编译器可能会出现权限问题
- 确保 MSYS2 已正确安装在 `C:/msys64/` 路径下
- 编译器路径为 `C:/msys64/ucrt64/bin/g++.exe`