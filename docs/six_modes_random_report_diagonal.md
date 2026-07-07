# 六种追加模式随机测试报告

- 用例数: 10 组随机 6x5 盘面，另配 10 组随机 7x6 盘面测试 7x6 模式
- 随机种子: `20260707`
- 盘面规则: 所有生成盘面都无初始横向或竖向 3 连
- 优先颜色: `R` + `G`，验证多颜色优先
- 对角线移动: 开启
- 超时标准: 单项最大允许 10000 ms
- 路径校验: 成功结果全部按 route 重放，断言不越界；固定点模式额外断言手指不进入 blocked 格
- 求解目标: 先满足模式规则；在规则成立的候选里选择最终 combo 更高的结果，combo 相同再偏向步数更短
- Combo 统计: 对最终盘面运行标准消除模拟，记录 `combo/max_combo`

## 汇总

| 模式 | 成功率 | 平均 Combo | 平均耗时 ms | 最大耗时 ms |
|---|---:|---:|---:|---:|
| 1 红/绿优先 4 连 | 10/10 | 8.50 | 300.291 | 385.697 |
| 2 固定点不经过 | 10/10 | 3.00 | 334.181 | 599.746 |
| 3 7x6 combo | 10/10 | 12.20 | 1492.535 | 1920.272 |
| 4 十字 | 10/10 | 3.40 | 572.813 | 772.423 |
| 5 一行 | 10/10 | 2.90 | 38.347 | 56.207 |
| 6 一竖 | 10/10 | 2.30 | 71.668 | 84.954 |

## 明细

| 序号 | 盘面 | 模式 | 结果 | ms | 步数 | 起点 | 颜色 | Combo | 备注 |
|---:|---|---|---|---:|---:|---:|---|---:|---|
| 1 | `HRRDGRRGDRLGBLGRBGBRDBGRRGHHDL` | 1 红/绿优先 4 连 | PASS | 207.986 | 31 | 13 | - | 9/9 | found exact R/G 4-connected erase, combo-scored |
| 1 | `HRRDGRRGDRLGBLGRBGBRDBGRRGHHDL` | 2 固定点不经过 | PASS | 206.612 | 30 | 0 | G | 3/9 | constructive color=G cost=8 combo-optimized |
| 1 | `LGRGRGGBRLDHBHLBBRDGDBRDLDLBDDRHRBHRGGLHHR` | 3 7x6 combo | PASS | 1014.065 | 47 | 41 | - | 13/13 | 7x6 target_combo with diagonal movement |
| 1 | `HRRDGRRGDRLGBLGRBGBRDBGRRGHHDL` | 4 十字 | PASS | 412.307 | 30 | 0 | G | 3/9 | constructive color=G cost=8 combo-optimized |
| 1 | `HRRDGRRGDRLGBLGRBGBRDBGRRGHHDL` | 5 一行 | PASS | 17.593 | 38 | 0 | G | 3/9 | constructive color=G cost=14 combo-optimized |
| 1 | `HRRDGRRGDRLGBLGRBGBRDBGRRGHHDL` | 6 一竖 | PASS | 51.606 | 20 | 0 | G | 2/9 | constructive color=G cost=6 combo-optimized |
| 2 | `HRRLRGRDRGLLGDGBDRGBRDRHBBRHGG` | 1 红/绿优先 4 连 | PASS | 273.721 | 34 | 27 | - | 8/9 | found exact R/G 4-connected erase, combo-scored |
| 2 | `HRRLRGRDRGLLGDGBDRGBRDRHBBRHGG` | 2 固定点不经过 | PASS | 106.241 | 29 | 0 | R | 3/9 | constructive color=R cost=6 combo-optimized |
| 2 | `LHHRDGRLRBRRLDRDRBLLGGGHDBDRGHDLBGBBHBGRDH` | 3 7x6 combo | PASS | 1600.750 | 30 | 14 | - | 12/13 | 7x6 target_combo with diagonal movement |
| 2 | `HRRLRGRDRGLLGDGBDRGBRDRHBBRHGG` | 4 十字 | PASS | 704.253 | 29 | 0 | G | 4/9 | constructive color=G cost=9 combo-optimized |
| 2 | `HRRLRGRDRGLLGDGBDRGBRDRHBBRHGG` | 5 一行 | PASS | 40.824 | 38 | 0 | G | 3/9 | constructive color=G cost=9 combo-optimized |
| 2 | `HRRLRGRDRGLLGDGBDRGBRDRHBBRHGG` | 6 一竖 | PASS | 69.898 | 28 | 3 | R | 2/9 | constructive color=R cost=8 combo-optimized |
| 3 | `GLHBRGRGGRRDHDBRDLLRGGRGBRDBRH` | 1 红/绿优先 4 连 | PASS | 346.326 | 32 | 26 | - | 7/9 | found exact R/G 4-connected erase, combo-scored |
| 3 | `GLHBRGRGGRRDHDBRDLLRGGRGBRDBRH` | 2 固定点不经过 | PASS | 311.844 | 38 | 1 | G | 3/9 | constructive color=G cost=10 combo-optimized |
| 3 | `BLDRBRDBLDLHBHGDHRGLGDRHBBHRRRGGRDGHGLRBDL` | 3 7x6 combo | PASS | 1357.775 | 45 | 19 | - | 13/13 | 7x6 target_combo with diagonal movement |
| 3 | `GLHBRGRGGRRDHDBRDLLRGGRGBRDBRH` | 4 十字 | PASS | 332.641 | 25 | 1 | G | 4/9 | constructive color=G cost=5 combo-optimized |
| 3 | `GLHBRGRGGRRDHDBRDLLRGGRGBRDBRH` | 5 一行 | PASS | 50.097 | 24 | 0 | R | 2/9 | constructive color=R cost=6 combo-optimized |
| 3 | `GLHBRGRGGRRDHDBRDLLRGGRGBRDBRH` | 6 一竖 | PASS | 84.954 | 21 | 2 | G | 2/9 | constructive color=G cost=7 combo-optimized |
| 4 | `GRGDHBLDDGLRBDBHGGRRGGRRLRHRBR` | 1 红/绿优先 4 连 | PASS | 385.697 | 28 | 1 | - | 8/9 | found exact R/G 4-connected erase, combo-scored |
| 4 | `GRGDHBLDDGLRBDBHGGRRGGRRLRHRBR` | 2 固定点不经过 | PASS | 219.190 | 24 | 1 | G | 3/9 | constructive color=G cost=10 combo-optimized |
| 4 | `DGRBLHGHDBBDBRLHBDHRRLDRLBBGHRGDGRGHGLDLRR` | 3 7x6 combo | PASS | 442.187 | 18 | 15 | - | 13/13 | 7x6 target_combo with diagonal movement |
| 4 | `GRGDHBLDDGLRBDBHGGRRGGRRLRHRBR` | 4 十字 | PASS | 526.839 | 21 | 1 | G | 4/9 | constructive color=G cost=4 combo-optimized |
| 4 | `GRGDHBLDDGLRBDBHGGRRGGRRLRHRBR` | 5 一行 | PASS | 32.876 | 13 | 1 | G | 3/9 | constructive color=G cost=7 combo-optimized |
| 4 | `GRGDHBLDDGLRBDBHGGRRGGRRLRHRBR` | 6 一竖 | PASS | 65.941 | 21 | 1 | G | 3/9 | constructive color=G cost=4 combo-optimized |
| 5 | `HBLRGGRBGRGLDLRHRRGGDGDRBRHDRB` | 1 红/绿优先 4 连 | PASS | 300.036 | 32 | 20 | - | 9/9 | found exact R/G 4-connected erase, combo-scored |
| 5 | `HBLRGGRBGRGLDLRHRRGGDGDRBRHDRB` | 2 固定点不经过 | PASS | 325.544 | 27 | 0 | G | 3/9 | constructive color=G cost=7 combo-optimized |
| 5 | `RBLGDHHLGRRLDDRBGDBGRHDBHDLDGRLBRHGRLHRGBB` | 3 7x6 combo | PASS | 1920.272 | 46 | 33 | - | 12/13 | 7x6 target_combo with diagonal movement |
| 5 | `HBLRGGRBGRGLDLRHRRGGDGDRBRHDRB` | 4 十字 | PASS | 535.944 | 27 | 0 | G | 3/9 | constructive color=G cost=7 combo-optimized |
| 5 | `HBLRGGRBGRGLDLRHRRGGDGDRBRHDRB` | 5 一行 | PASS | 34.477 | 20 | 0 | R | 3/9 | constructive color=R cost=7 combo-optimized |
| 5 | `HBLRGGRBGRGLDLRHRRGGDGDRBRHDRB` | 6 一竖 | PASS | 72.997 | 15 | 0 | R | 2/9 | constructive color=R cost=8 combo-optimized |
| 6 | `RRBBRGBBDLGGHRGDRRRHLHDRRGGLGD` | 1 红/绿优先 4 连 | PASS | 154.647 | 18 | 17 | - | 9/9 | found exact R/G 4-connected erase, combo-scored |
| 6 | `RRBBRGBBDLGGHRGDRRRHLHDRRGGLGD` | 2 固定点不经过 | PASS | 435.071 | 29 | 1 | G | 3/9 | constructive color=G cost=8 combo-optimized |
| 6 | `BLGLRLHBHRGLHDRDBDRHRDGGLHDGHGBBRDDLRBBRGR` | 3 7x6 combo | PASS | 1577.058 | 28 | 40 | - | 11/13 | 7x6 target_combo with diagonal movement |
| 6 | `RRBBRGBBDLGGHRGDRRRHLHDRRGGLGD` | 4 十字 | PASS | 578.518 | 36 | 0 | G | 5/9 | constructive color=G cost=15 combo-optimized |
| 6 | `RRBBRGBBDLGGHRGDRRRHLHDRRGGLGD` | 5 一行 | PASS | 32.898 | 22 | 0 | G | 2/9 | constructive color=G cost=11 combo-optimized |
| 6 | `RRBBRGBBDLGGHRGDRRRHLHDRRGGLGD` | 6 一竖 | PASS | 73.743 | 17 | 0 | G | 2/9 | constructive color=G cost=6 combo-optimized |
| 7 | `GRRGRBDHGRLGRLRDHRRHGGDDLBBGRB` | 1 红/绿优先 4 连 | PASS | 361.943 | 34 | 10 | - | 9/9 | found exact R/G 4-connected erase, combo-scored |
| 7 | `GRRGRBDHGRLGRLRDHRRHGGDDLBBGRB` | 2 固定点不经过 | PASS | 313.542 | 20 | 1 | G | 3/9 | constructive color=G cost=8 combo-optimized |
| 7 | `GRGLDLLDBLRHDHDRHDGLHBHGRRBDRBRBRHRLGBGDGB` | 3 7x6 combo | PASS | 1742.591 | 46 | 27 | - | 12/13 | 7x6 target_combo with diagonal movement |
| 7 | `GRRGRBDHGRLGRLRDHRRHGGDDLBBGRB` | 4 十字 | PASS | 772.423 | 21 | 1 | G | 3/9 | constructive color=G cost=5 combo-optimized |
| 7 | `GRRGRBDHGRLGRLRDHRRHGGDDLBBGRB` | 5 一行 | PASS | 42.842 | 19 | 1 | G | 3/9 | constructive color=G cost=10 combo-optimized |
| 7 | `GRRGRBDHGRLGRLRDHRRHGGDDLBBGRB` | 6 一竖 | PASS | 80.975 | 32 | 1 | G | 3/9 | constructive color=G cost=11 combo-optimized |
| 8 | `DHRDGBBDGRRHRBGLLDRGHRRLGRGGBR` | 1 红/绿优先 4 连 | PASS | 311.645 | 33 | 28 | - | 9/9 | found exact R/G 4-connected erase, combo-scored |
| 8 | `DHRDGBBDGRRHRBGLLDRGHRRLGRGGBR` | 2 固定点不经过 | PASS | 456.849 | 22 | 0 | R | 3/9 | constructive color=R cost=5 combo-optimized |
| 8 | `HBRBDLHLDLRRDLGRBRBLDBRRGHGGLDHGDHRGRBBHDG` | 3 7x6 combo | PASS | 1729.140 | 50 | 32 | - | 12/13 | 7x6 target_combo with diagonal movement |
| 8 | `DHRDGBBDGRRHRBGLLDRGHRRLGRGGBR` | 4 十字 | PASS | 709.138 | 22 | 0 | R | 3/9 | constructive color=R cost=5 combo-optimized |
| 8 | `DHRDGBBDGRRHRBGLLDRGHRRLGRGGBR` | 5 一行 | PASS | 56.207 | 39 | 0 | G | 4/9 | constructive color=G cost=13 combo-optimized |
| 8 | `DHRDGBBDGRRHRBGLLDRGHRRLGRGGBR` | 6 一竖 | PASS | 69.796 | 44 | 0 | G | 3/9 | constructive color=G cost=13 combo-optimized |
| 9 | `GHHGLGGRRHRLDBBRRGGRBDBRDLRDGR` | 1 红/绿优先 4 连 | PASS | 323.065 | 31 | 10 | - | 9/9 | found exact R/G 4-connected erase, combo-scored |
| 9 | `GHHGLGGRRHRLDBBRRGGRBDBRDLRDGR` | 2 固定点不经过 | PASS | 599.746 | 30 | 1 | G | 3/9 | constructive color=G cost=13 combo-optimized |
| 9 | `DDRGGLHHBBGHDHDDLBRLBRGGLGRRBRRBBRLLGHDDHR` | 3 7x6 combo | PASS | 1650.825 | 39 | 29 | - | 12/13 | 7x6 target_combo with diagonal movement |
| 9 | `GHHGLGGRRHRLDBBRRGGRBDBRDLRDGR` | 4 十字 | PASS | 586.036 | 19 | 0 | R | 3/9 | constructive color=R cost=6 combo-optimized |
| 9 | `GHHGLGGRRHRLDBBRRGGRBDBRDLRDGR` | 5 一行 | PASS | 36.859 | 21 | 0 | R | 4/9 | constructive color=R cost=6 combo-optimized |
| 9 | `GHHGLGGRRHRLDBBRRGGRBDBRDLRDGR` | 6 一竖 | PASS | 74.126 | 22 | 0 | R | 2/9 | constructive color=R cost=6 combo-optimized |
| 10 | `LDBGGRLBDRGHBGDHRRGGRLRRRDGRHB` | 1 红/绿优先 4 连 | PASS | 337.841 | 30 | 14 | - | 8/9 | found exact R/G 4-connected erase, combo-scored |
| 10 | `LDBGGRLBDRGHBGDHRRGGRLRRRDGRHB` | 2 固定点不经过 | PASS | 367.166 | 36 | 0 | G | 3/9 | constructive color=G cost=9 combo-optimized |
| 10 | `HRGDRLHRRGGBBDBBRRLHGBHRBDDLDLHDBLRGLRHDGG` | 3 7x6 combo | PASS | 1890.690 | 39 | 7 | - | 12/13 | 7x6 target_combo with diagonal movement |
| 10 | `LDBGGRLBDRGHBGDHRRGGRLRRRDGRHB` | 4 十字 | PASS | 570.034 | 23 | 0 | G | 2/9 | constructive color=G cost=6 combo-optimized |
| 10 | `LDBGGRLBDRGHBGDHRRGGRLRRRDGRHB` | 5 一行 | PASS | 38.794 | 19 | 0 | G | 2/9 | constructive color=G cost=9 combo-optimized |
| 10 | `LDBGGRLBDRGHBGDHRRGGRLRRRDGRHB` | 6 一竖 | PASS | 72.643 | 15 | 0 | G | 2/9 | constructive color=G cost=5 combo-optimized |
