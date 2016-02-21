## 更新日志 - 2016-02.21

### Outline

- 增加了UNKNOW的运动状态
- 增加了Y方向运动状态的判定
- 增加了连续fail情况下运动状态的判定
- 增加了位置坐标的返回值

详细一点说:   
UNKNOW状态的判定，有三种情况
1. 第一秒积累初始数据时，返回UNKNOW
2. 连续fail超过15帧时，返回UNKNOW
3. 连续fail未超过15帧，但是检测或预测到人脸走出画面时，返回UNKNOW   

位置坐标的返回
- 返回的方法是统计前五帧的平均值，然后每次返回的都是前五帧的平均值
- 这里我觉得还是可以改进，在这个.md文档最后我想说一下可能的改进方法

### 接口函数

请使用这个接口函数：

```c++
MoveState TracingStreamed(int frameid, int x, int y, int width, int height, int mode = SMOOTH,
    int xdelay = 0, int ydelay = 0, int adelay = 25,
    // delay参数，用来调三个方向的精确度，当delay参数调整到最好的大小时，判定最准确
    int xdivisior = 8, int ydivisior = 12,int adivisior = 20,
    // 灵敏度参数，越大越灵敏
    // 但是过于灵敏，可能会降低算法的准确性和稳定性，所以前两列参数需要调到一个平衡的状态
    int len = 30, // 缓冲区大小，请保证 delay <= len - 5
    int interval = 15,  // SMOOTH模式的力度，越大越平滑
    int jpredict = 8);   // 预测平滑度的调整，则个参数越大，预测的结果越平滑
```

这个接口函数定义在TracingStreamed.cpp中

接口函数提供了两种模式，宏定义在head_streamed.h中

```c++
#define ACCURATE 0  // 精确模式
#define SMOOTH 1    // 平滑模式
```

这个接口返回一个MoveState类
```c++
class MoveState
{
public:
    int x;      // x方向状态
    int xloc;   // x方向坐标
    int y;      // y方向状态
    int yloc;   // y方向坐标
    int a;      // a方向状态 -- a是指面积的大小
    int aloc;   // a方向坐标
    MoveState(){}
    MoveState(int x, int y, int z);
    bool operator == (MoveState mv);
    bool operator != (MoveState mv);
};
```
这个类定义在Resource.cpp中

每个方向的状态，宏定义在head_streamed.h中
```c++
// 特殊状态
#define UNKNOW -1
// 三个方向都可能停止
#define STAY 0
// x方向
#define LEFT 1
#define RIGHT 2
// y方向
#define UP 3
#define DOWN 4
// a方向
#define FRONT 5
#define BACK 6
```

### 一个可能的改进方法 -- HMM隐马尔科夫模型

这也是我看了一个寒假的东西，感觉还是比较高端的，但是目前有很多细节我还没想明白，所以应该还需要一些时间才能给出代码。这儿先说一个大概？

隐马尔可夫模型是说，一个随机过程，有一个[观测到的状态链 O]，和隐藏在观测状态链后的[隐含状态链 S]

比如说，在这里，可以[观测到的状态链 O] = 无人机的坐标序列，隐藏在背后的[隐含状态链 S] = 无人机的真实运动状态，如STAY, LEFT, RIGHT等

先用观测到的坐标序列 O1 求无人机的运动状态序列 S1

对运动状态序列 S1，用聚类算法做平滑，得到平滑后的运动状态序列 S2

根据 S2 再返回平滑后的坐标序列 O2

O2 和 S2 应该就是我们想要的

我觉得这方法确实不怎么简单，我光是看书就看了一周多，发现是深坑，现在都还没看完。看书的序列是 实变函数测度部分 -- 概率论 -- 随机过程 -- 时间序列 -- HMM相关论文 -- 机器学习，都只是浅薄地过了一下，很多细节没有深究

再给一些时间应该搞得出来，要不要试试这个方法？
