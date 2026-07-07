# 六种追加模式随机测试报告

- 用例数: 10 组随机 6x5 盘面，另配 10 组随机 7x6 盘面测试 7x6 模式
- 随机种子: `20260707`
- 盘面规则: 所有生成盘面都无初始横向或竖向 3 连
- 优先颜色: `R` + `G`，验证多颜色优先
- 对角线移动: 关闭
- 超时标准: 单项最大允许 10000 ms
- 路径校验: 成功结果全部按 route 重放，断言不越界；固定点模式额外断言手指不进入 blocked 格
- 求解目标: 先满足模式规则；在规则成立的候选里选择最终 combo 更高的结果，combo 相同再偏向步数更短
- Combo 统计: 对最终盘面运行标准消除模拟，记录 `combo/max_combo`

## 汇总

| 模式 | 成功率 | 平均 Combo | 平均耗时 ms | 最大耗时 ms |
|---|---:|---:|---:|---:|
| 1 红/绿优先 4 连 | 10/10 | 7.70 | 157.818 | 173.810 |
| 2 固定点不经过 | 10/10 | 2.20 | 170.683 | 303.413 |
| 3 7x6 combo | 10/10 | 12.00 | 762.844 | 842.375 |
| 4 十字 | 10/10 | 2.50 | 416.492 | 512.009 |
| 5 一行 | 10/10 | 3.00 | 19.192 | 26.572 |
| 6 一竖 | 10/10 | 2.90 | 30.840 | 35.823 |

## 明细

| 序号 | 盘面 | 模式 | 结果 | ms | 步数 | 起点 | 颜色 | Combo | 备注 |
|---:|---|---|---|---:|---:|---:|---|---:|---|
| 1 | `HRRDGRRGDRLGBLGRBGBRDBGRRGHHDL` | 1 红/绿优先 4 连 | PASS | 157.657 | 33 | 3 | - | 8/9 | found exact R/G 4-connected erase, combo-scored |
| 1 | `HRRDGRRGDRLGBLGRBGBRDBGRRGHHDL` | 2 固定点不经过 | PASS | 209.244 | 22 | 0 | G | 2/9 | constructive color=G cost=4 combo-optimized |
| 1 | `LGRGRGGBRLDHBHLBBRDGDBRDLDLBDDRHRBHRGGLHHR` | 3 7x6 combo | PASS | 600.675 | 44 | 38 | - | 13/13 | 7x6 target_combo |
| 1 | `HRRDGRRGDRLGBLGRBGBRDBGRRGHHDL` | 4 十字 | PASS | 428.352 | 22 | 0 | G | 2/9 | constructive color=G cost=4 combo-optimized |
| 1 | `HRRDGRRGDRLGBLGRBGBRDBGRRGHHDL` | 5 一行 | PASS | 13.781 | 45 | 0 | G | 2/9 | constructive color=G cost=10 combo-optimized |
| 1 | `HRRDGRRGDRLGBLGRBGBRDBGRRGHHDL` | 6 一竖 | PASS | 35.823 | 54 | 0 | R | 3/9 | constructive color=R cost=8 combo-optimized |
| 2 | `HRRLRGRDRGLLGDGBDRGBRDRHBBRHGG` | 1 红/绿优先 4 连 | PASS | 173.810 | 24 | 23 | - | 8/9 | found exact R/G 4-connected erase, combo-scored |
| 2 | `HRRLRGRDRGLLGDGBDRGBRDRHBBRHGG` | 2 固定点不经过 | PASS | 142.776 | 66 | 0 | R | 2/9 | constructive color=R cost=6 combo-optimized |
| 2 | `LHHRDGRLRBRRLDRDRBLLGGGHDBDRGHDLBGBBHBGRDH` | 3 7x6 combo | PASS | 842.375 | 56 | 23 | - | 12/13 | 7x6 target_combo |
| 2 | `HRRLRGRDRGLLGDGBDRGBRDRHBBRHGG` | 4 十字 | PASS | 443.400 | 42 | 0 | G | 2/9 | constructive color=G cost=7 combo-optimized |
| 2 | `HRRLRGRDRGLLGDGBDRGBRDRHBBRHGG` | 5 一行 | PASS | 19.651 | 60 | 0 | G | 3/9 | constructive color=G cost=9 combo-optimized |
| 2 | `HRRLRGRDRGLLGDGBDRGBRDRHBBRHGG` | 6 一竖 | PASS | 27.575 | 59 | 0 | G | 3/9 | constructive color=G cost=7 combo-optimized |
| 3 | `GLHBRGRGGRRDHDBRDLLRGGRGBRDBRH` | 1 红/绿优先 4 连 | PASS | 158.673 | 34 | 26 | - | 6/9 | found exact R/G 4-connected erase, combo-scored |
| 3 | `GLHBRGRGGRRDHDBRDLLRGGRGBRDBRH` | 2 固定点不经过 | PASS | 143.731 | 55 | 0 | R | 2/9 | constructive color=R cost=4 combo-optimized |
| 3 | `BLDRBRDBLDLHBHGDHRGLGDRHBBHRRRGGRDGHGLRBDL` | 3 7x6 combo | PASS | 756.617 | 58 | 39 | - | 13/13 | 7x6 target_combo |
| 3 | `GLHBRGRGGRRDHDBRDLLRGGRGBRDBRH` | 4 十字 | PASS | 375.389 | 27 | 1 | G | 2/9 | constructive color=G cost=6 combo-optimized |
| 3 | `GLHBRGRGGRRDHDBRDLLRGGRGBRDBRH` | 5 一行 | PASS | 19.453 | 47 | 0 | R | 2/9 | constructive color=R cost=7 combo-optimized |
| 3 | `GLHBRGRGGRRDHDBRDLLRGGRGBRDBRH` | 6 一竖 | PASS | 30.495 | 25 | 0 | R | 3/9 | constructive color=R cost=6 combo-optimized |
| 4 | `GRGDHBLDDGLRBDBHGGRRGGRRLRHRBR` | 1 红/绿优先 4 连 | PASS | 163.902 | 33 | 10 | - | 8/9 | found exact R/G 4-connected erase, combo-scored |
| 4 | `GRGDHBLDDGLRBDBHGGRRGGRRLRHRBR` | 2 固定点不经过 | PASS | 101.143 | 60 | 0 | R | 3/9 | constructive color=R cost=6 combo-optimized |
| 4 | `DGRBLHGHDBBDBRLHBDHRRLDRLBBGHRGDGRGHGLDLRR` | 3 7x6 combo | PASS | 713.518 | 54 | 38 | - | 13/13 | 7x6 target_combo |
| 4 | `GRGDHBLDDGLRBDBHGGRRGGRRLRHRBR` | 4 十字 | PASS | 350.046 | 41 | 1 | G | 3/9 | constructive color=G cost=4 combo-optimized |
| 4 | `GRGDHBLDDGLRBDBHGGRRGGRRLRHRBR` | 5 一行 | PASS | 23.695 | 71 | 1 | G | 3/9 | constructive color=G cost=9 combo-optimized |
| 4 | `GRGDHBLDDGLRBDBHGGRRGGRRLRHRBR` | 6 一竖 | PASS | 30.563 | 49 | 1 | G | 3/9 | constructive color=G cost=13 combo-optimized |
| 5 | `HBLRGGRBGRGLDLRHRRGGDGDRBRHDRB` | 1 红/绿优先 4 连 | PASS | 142.479 | 33 | 2 | - | 8/9 | found exact R/G 4-connected erase, combo-scored |
| 5 | `HBLRGGRBGRGLDLRHRRGGDGDRBRHDRB` | 2 固定点不经过 | PASS | 133.877 | 39 | 0 | G | 2/9 | constructive color=G cost=8 combo-optimized |
| 5 | `RBLGDHHLGRRLDDRBGDBGRHDBHDLDGRLBRHGRLHRGBB` | 3 7x6 combo | PASS | 805.268 | 50 | 31 | - | 12/13 | 7x6 target_combo |
| 5 | `HBLRGGRBGRGLDLRHRRGGDGDRBRHDRB` | 4 十字 | PASS | 430.618 | 38 | 0 | R | 2/9 | constructive color=R cost=3 combo-optimized |
| 5 | `HBLRGGRBGRGLDLRHRRGGDGDRBRHDRB` | 5 一行 | PASS | 12.053 | 30 | 0 | R | 2/9 | constructive color=R cost=6 combo-optimized |
| 5 | `HBLRGGRBGRGLDLRHRRGGDGDRBRHDRB` | 6 一竖 | PASS | 33.406 | 48 | 0 | R | 3/9 | constructive color=R cost=6 combo-optimized |
| 6 | `RRBBRGBBDLGGHRGDRRRHLHDRRGGLGD` | 1 红/绿优先 4 连 | PASS | 164.940 | 34 | 5 | - | 8/9 | found exact R/G 4-connected erase, combo-scored |
| 6 | `RRBBRGBBDLGGHRGDRRRHLHDRRGGLGD` | 2 固定点不经过 | PASS | 224.223 | 32 | 2 | R | 3/9 | constructive color=R cost=7 combo-optimized |
| 6 | `BLGLRLHBHRGLHDRDBDRHRDGGLHDGHGBBRDDLRBBRGR` | 3 7x6 combo | PASS | 786.578 | 49 | 40 | - | 11/13 | 7x6 target_combo |
| 6 | `RRBBRGBBDLGGHRGDRRRHLHDRRGGLGD` | 4 十字 | PASS | 363.815 | 34 | 2 | R | 3/9 | constructive color=R cost=7 combo-optimized |
| 6 | `RRBBRGBBDLGGHRGDRRRHLHDRRGGLGD` | 5 一行 | PASS | 26.572 | 59 | 6 | G | 4/9 | constructive color=G cost=16 combo-optimized |
| 6 | `RRBBRGBBDLGGHRGDRRRHLHDRRGGLGD` | 6 一竖 | PASS | 31.533 | 45 | 0 | G | 3/9 | constructive color=G cost=7 combo-optimized |
| 7 | `GRRGRBDHGRLGRLRDHRRHGGDDLBBGRB` | 1 红/绿优先 4 连 | PASS | 150.182 | 31 | 3 | - | 8/9 | found exact R/G 4-connected erase, combo-scored |
| 7 | `GRRGRBDHGRLGRLRDHRRHGGDDLBBGRB` | 2 固定点不经过 | PASS | 88.678 | 34 | 0 | R | 2/9 | constructive color=R cost=5 combo-optimized |
| 7 | `GRGLDLLDBLRHDHDRHDGLHBHGRRBDRBRBRHRLGBGDGB` | 3 7x6 combo | PASS | 754.413 | 56 | 18 | - | 11/13 | 7x6 target_combo |
| 7 | `GRRGRBDHGRLGRLRDHRRHGGDDLBBGRB` | 4 十字 | PASS | 512.009 | 69 | 0 | R | 3/9 | constructive color=R cost=8 combo-optimized |
| 7 | `GRRGRBDHGRLGRLRDHRRHGGDDLBBGRB` | 5 一行 | PASS | 20.089 | 38 | 1 | G | 4/9 | constructive color=G cost=8 combo-optimized |
| 7 | `GRRGRBDHGRLGRLRDHRRHGGDDLBBGRB` | 6 一竖 | PASS | 32.380 | 63 | 0 | R | 3/9 | constructive color=R cost=8 combo-optimized |
| 8 | `DHRDGBBDGRRHRBGLLDRGHRRLGRGGBR` | 1 红/绿优先 4 连 | PASS | 151.733 | 34 | 20 | - | 7/9 | found exact R/G 4-connected erase, combo-scored |
| 8 | `DHRDGBBDGRRHRBGLLDRGHRRLGRGGBR` | 2 固定点不经过 | PASS | 230.822 | 48 | 0 | R | 2/9 | constructive color=R cost=5 combo-optimized |
| 8 | `HBRBDLHLDLRRDLGRBRBLDBRRGHGGLDHGDHRGRBBHDG` | 3 7x6 combo | PASS | 818.819 | 44 | 30 | - | 12/13 | 7x6 target_combo |
| 8 | `DHRDGBBDGRRHRBGLLDRGHRRLGRGGBR` | 4 十字 | PASS | 435.899 | 12 | 0 | G | 3/9 | constructive color=G cost=5 combo-optimized |
| 8 | `DHRDGBBDGRRHRBGLLDRGHRRLGRGGBR` | 5 一行 | PASS | 22.896 | 51 | 0 | G | 3/9 | constructive color=G cost=10 combo-optimized |
| 8 | `DHRDGBBDGRRHRBGLLDRGHRRLGRGGBR` | 6 一竖 | PASS | 29.341 | 20 | 0 | G | 2/9 | constructive color=G cost=5 combo-optimized |
| 9 | `GHHGLGGRRHRLDBBRRGGRBDBRDLRDGR` | 1 红/绿优先 4 连 | PASS | 161.638 | 30 | 4 | - | 8/9 | found exact R/G 4-connected erase, combo-scored |
| 9 | `GHHGLGGRRHRLDBBRRGGRBDBRDLRDGR` | 2 固定点不经过 | PASS | 303.413 | 47 | 1 | G | 2/9 | constructive color=G cost=10 combo-optimized |
| 9 | `DDRGGLHHBBGHDHDDLBRLBRGGLGRRBRRBBRLLGHDDHR` | 3 7x6 combo | PASS | 770.010 | 49 | 7 | - | 12/13 | 7x6 target_combo |
| 9 | `GHHGLGGRRHRLDBBRRGGRBDBRDLRDGR` | 4 十字 | PASS | 462.834 | 52 | 1 | G | 2/9 | constructive color=G cost=11 combo-optimized |
| 9 | `GHHGLGGRRHRLDBBRRGGRBDBRDLRDGR` | 5 一行 | PASS | 18.947 | 36 | 0 | R | 4/9 | constructive color=R cost=6 combo-optimized |
| 9 | `GHHGLGGRRHRLDBBRRGGRBDBRDLRDGR` | 6 一竖 | PASS | 29.084 | 58 | 1 | G | 3/9 | constructive color=G cost=9 combo-optimized |
| 10 | `LDBGGRLBDRGHBGDHRRGGRLRRRDGRHB` | 1 红/绿优先 4 连 | PASS | 153.166 | 33 | 8 | - | 8/9 | found exact R/G 4-connected erase, combo-scored |
| 10 | `LDBGGRLBDRGHBGDHRRGGRLRRRDGRHB` | 2 固定点不经过 | PASS | 128.927 | 82 | 0 | G | 2/9 | constructive color=G cost=9 combo-optimized |
| 10 | `HRGDRLHRRGGBBDBBRRLHGBHRBDDLDLHDBLRGLRHDGG` | 3 7x6 combo | PASS | 780.170 | 51 | 7 | - | 11/13 | 7x6 target_combo |
| 10 | `LDBGGRLBDRGHBGDHRRGGRLRRRDGRHB` | 4 十字 | PASS | 362.555 | 29 | 0 | G | 3/9 | constructive color=G cost=6 combo-optimized |
| 10 | `LDBGGRLBDRGHBGDHRRGGRLRRRDGRHB` | 5 一行 | PASS | 14.786 | 51 | 0 | R | 3/9 | constructive color=R cost=12 combo-optimized |
| 10 | `LDBGGRLBDRGHBGDHRRGGRLRRRDGRHB` | 6 一竖 | PASS | 28.199 | 47 | 0 | G | 3/9 | constructive color=G cost=6 combo-optimized |
