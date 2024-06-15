# stm32
2024.6.11 - 2024.6.14

嵌入式实验 交通信号灯 stm32 keil 

## 使用器材：

- stm32主板

- LED灯 

- 点阵模块  8*8

- 按键

- 数码管

- 连接线，下载线

  

## 实现功能：

- 8个LED灯显示其中一个方向的左转 直行 右转的信号灯， 点阵模块用于显示另一个方向的信号灯，仅有直行和禁止通行。
- 绿灯8秒，黄灯2秒。
- LED灯：左转和直行分别使用红绿黄灯为一组来显示，最后两个红和绿灯来显示右转。从右转绿灯开始亮为起始点循环。
- 点阵模块表示的方向只有在LED中右转为绿灯时可以直行显示直行标识，其他时间为禁止通行
- 数码管显示LED灯路口信号剩余时间
- 按键 0 ：左转绿灯 ，按键 1 ：直行绿灯 ，按键 2 ：右转绿灯

## 实现效果展示：

![嵌入式](http://wyq-index.oss-cn-beijing.aliyuncs.com/c3b1a43f-24f1-4c8e-ad4e-19be07e41e86.jpg)
