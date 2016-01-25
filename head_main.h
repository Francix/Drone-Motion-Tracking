// 类名，函数名不同的单词大写

#include <iostream>
#include <fstream>
#include <cmath>


// 以下的这些宏，定义了当前帧的运动状态
#define STAY 0
#define LEFT 1
#define RIGHT 2
#define FRONT 3
#define BACK 4
#define LEFTFRONT 5
#define LEFTBACK 6
#define RIGHTFRONT 7
#define RIGHTBACK 8

// 这个宏定义了本函数实时分析的模式
#define ACCURATE 0
#define SMOOTH 1

// 条件编译
//#define FRAMEWISE
#define TEST

using namespace std;

class CFaceLocation
{
// facelocation类，用于记录每一帧的位置数据
private:
    int frameid;     // 当前帧的序号

    // 各个方向的参数
    int xlabel;
    int ylabel;
    int width;
    int height;
    int area;

    // 当前状态参数
    int state;
    bool ispredicted;   // 是否坏点 -- 如果是坏点，则预测

public:
    CFaceLocation(){};    // 默认构造函数
    friend class CQueueLocation;
};

// 临时变量，用于辅助读取数据
struct node
{
    int tmpx, tmpy, tmpw, tmph;
    int tmpa;
    float tmpf;
};

class CQueueLocation
{
    //  位置队列 -- 多少帧？acnt + adelay帧 -- 这里需要一个构造函数
    //  这个队列需要完成的事情:
    //  1.新位置入队
    //      队列没满怎么办？先入队，然后直接返回stay
    //  2.根据以往状态，判定新位置状态
    //      如何判定？指数加权
    //  3.是直接输出新位置状态，还是进行二次平滑？
    //      应该写一个进行二次平滑与不进行二次平滑的版本
    //  4.每次新位置入队时，还需要对上一个错误点 or 以往的错误点？进行更正
    //      我觉得，这里应该写一个更正的版本，与不更正的版本
private:
    // 队列内容
    CFaceLocation *pfacloc;

    // 到目前为止总的帧数
    int allframe;

    // 队列指针参数
    int pnow;
    int len;
    // len >= adelay + acnt
    bool isfull;

    // 算法灵敏度参数·一阶平滑
    int xcnt;
    int xdelay;
    int acnt;
    int adelay;
    // 算法灵敏度参数·二阶平滑
    int interval;
    // 预测范围参数
    int jpredict;

public:
    CQueueLocation(){} // 注意，这里从语法上是不需要写分号的
    ~CQueueLocation();

    // 初始化
    void init(int xcnt = 6, int acnt = 8, int xdelay = 0, int adelay = 20,
        int len = 30, int interval = 15, int jpredict = 5);
    // 返回下一个index
    inline int nextindex(int i);
    // 返回前一个index
    inline int previndex(int i);
    // 入队函数，控制自动出队，返回队尾指针
    int insert(node tmpnd);
    // 计算当前状态并返回
    int findstate(int tmpindex, int mode = ACCURATE);
    // 根据现在的状态坐标，预测下一个状态，返回一个node结构
    node pridict(int tmpindex);
    // 纠正错误点
    void correct();
    // 声明友元函数
    friend int TracingStreamed(int frameid, int x, int y, int width, int height,
        int xcnt, int acnt, int xdelay, int adelay, int len, int interval, int jpredict);
};

int TracingStreamed(int frameid, int x, int y, int width, int height,
    int xcnt = 6, int acnt = 6, int xdelay = 0, int adelay = 20,
    int len = 30, int interval = 15, int jpredict = 5);
