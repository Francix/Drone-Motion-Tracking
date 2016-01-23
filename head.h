// 类名，函数名不同的单词大写

#include <iostream>
#include <fstream>
#include <cmath>

// 以下的这些宏，定义为当前帧的运动状态
#define STAY 0
#define LEFT 1
#define RIGHT (1 << 1)
#define UP (1 << 2)
#define DOWN (1 << 3)
#define FRONT (1 << 4)
#define BACK (1 << 5)

// 以下这宏定义了本函数的模式
#define LEFTRIGHT (1 | (1 << 1))        // 分析左右运动
#define UPDOWN ((1 << 2) | (1 << 3))    // 分析上下运动 -- 这个模式暂时没有实现
#define FRONTBACK ((1 << 4) | (1 << 5)) // 分析前后运动
#define VERBOSE (1 << 7)                // 是否打印详细信息
#define BADCOUNT (1 << 8)               // 坏点分析模式
#define ACCURATE (1 << 9)               // 灵敏模式
#define VAGUE_L0 (1 << 10)              // 零级模糊模式
#define VAGUE_L1 (1 << 11)              // 一级模糊模式
#define VAGUE_L2 (1 << 12)              // 二级模糊模式

using namespace std;

class CFaceLocation
{
// facelocation类，用于记录每一帧的位置数据
public:
    // 帧的计数变量
    int total;      // 所有的点计数
    int totalfail;  // 所有的坏点计数
    int number;     // 当前帧的序号
    // 时间
    int minite;
    int second;
    int minisecond;
    // 各个方向的参数
    int xlabel;
    int ylabel;
    int width;
    int height;
    int area;
    // 前后方向的测试参数
    int neararea;
    int aave;   // average平均值
    int athe;   // threshold阈值
    int adiff;  // difference差值
    // 当前状态参数
    int state;
    bool ispredicted;   // 是否坏点 -- 如果是坏点，则预测

    CFaceLocation();    // 默认构造函数
    CFaceLocation(int num, int x, int y, int wid, int hei); // 二号构造函数 -- 这个函数实际上并没有被用到

    void ShowFaceLoc(int x, int mode = LEFTRIGHT | UPDOWN | FRONTBACK); // 打印每一帧的坐标
    void CountTime();   // 计算当前时间
    void FindState(CFaceLocation *pfacloc);     // 求当前点的状态 -- 这个函数已经被淘汰，请使用下一个
    void FindState_1(CFaceLocation *pfacloc);   // 这个函数应该比上一个函数要好！
};

//int CFaceLocation::total = 0;

// 临时变量，用于辅助读取数据
struct node
{
    int tmpx, tmpy, tmpw, tmph;
    int tmpa;
    float tmpf;
};

void pridict(node *nd, CFaceLocation *pfacloc, int i);  // 当遇到fail的点时，调用此函数进行预测
void test(CFaceLocation *pfacloc, int mode = LEFTRIGHT | UPDOWN | FRONTBACK | VERBOSE); // 测试函数，所有函数的入口
void ParseState(CFaceLocation *pfacloc);    // 分析当前状态
void ParseState_1(CFaceLocation *pfacloc, int mode = LEFTRIGHT | UPDOWN | FRONTBACK);   // 细致模式的状态分析函数
void ParseState_2(CFaceLocation *pfacloc, int mode = LEFTRIGHT | UPDOWN | FRONTBACK);   // 平滑模式的状态分析函数
void BadCount(CFaceLocation *pfacloc, int mode, int rightstate, int timestart, int timeend);    // 错误点计算函数，-b模式下使用
