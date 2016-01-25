#include "head.h"

class CFaceLocation
{
// facelocation类，用于记录每一帧的位置数据
private:
    int frameid;     // 当前帧的序号

    // 时间
    // int minite;
    // int second;
    // int minisecond;

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
