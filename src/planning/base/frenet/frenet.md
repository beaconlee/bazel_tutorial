# Frenet

使用三次样条插值实际规划的实际上只是一个简单的路径，而非最终能够执行的路径。

## Jerk 加速度的变化率
也叫做 加加速度。


## 为什么使用 Frenet 坐标系
在 Frenet 坐标系中，使用参考线的切线向量 t 和法线向量 n 建立一个坐标系，它以车辆自身投影到参考线上的点为坐标原点（也就是说 Frenet 坐标系是实时变化的），坐标轴互相垂直，分为 s 方向（即沿着参考线方向，通常被称为纵向，Longitudinal）和 d 方向（即参考线方向，被称为横向，Lateral）。这样会比笛卡尔坐标系简化一些问题：容易描述车辆的位置、两个方向的速度也容易计算（分别为 s、d 的导数）。

在 Frenet 坐标系下，动作规划有三个维度：（s, d, t），t 是我们规划出来的每一个动作的时间点，轨迹和路径的本质区别就是轨迹考虑了时间这一维度。

Werling 的动作规划方法一个很关键的理念就是将动作规划这一高维度的优化问题分割成横向和纵向两个方向上的彼此独立的优化问题：

假设上层（行为规划层）要求当前车辆在 t' 时刻越过虚线完成一次变道，即车辆在横向上需要完成一个 Δd，以及纵向上完成一个 Δs 的移动，则可以将 s 和 d 表示成 t 的函数：s(t)、d(t)。这样就将原来复杂的动作规划问题分割成了两个独立的优化问题：对于 横向 和 纵向 的轨迹优化，我们使用一个损失函数 C，将使得 C 最小的轨迹作为最终规划的动作序列。

## Jerk 最小化 和 5次轨迹多项式求解
Takahashi的文章——Local path planning and motion control for AGV in positioning中已经证明，任何Jerk最优化问题中的解都可以使用一个5次多项式来表示。