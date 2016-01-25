#include "head_main.h"
#include "head_test.h"

static CQueueLocation queloc; // 静态全局变量，每一帧的位置队列

int TracingStreamed(int frameid, int x, int y, int width, int height,
    int xcnt, int acnt, int xdelay, int adelay, int l, int interval, int jpredict)
{
    // 这里需要维持一个队列，这个队列需要手动写...可不可以通过下标访问？
    // 完成的事情：读入一个坐标位置，返回一个状态
    // 静态判别符
    static bool isfirst = true;
    // 队列尾指针
    int now;
    // 返回状态
    int state;
    // 辅助节点
    node tmpnd;
    // 如果第一个节点
    if(isfirst)
    {
        // 初始化queloc
        isfirst = false;
        queloc.init(xcnt, acnt, xdelay, adelay);
    }

    tmpnd.tmpx = x;
    tmpnd.tmpy = y;
    tmpnd.tmpw = width;
    tmpnd.tmph = height;
    tmpnd.tmpa = width * height;


    if((tmpnd.tmpx == -1) && (tmpnd.tmpy == -1))
    {
        // 如果坐标为fail，则预测
        tmpnd = queloc.pridict(queloc.pnow);
    }

    else
    {
        // 预处理
        tmpnd.tmpx = tmpnd.tmpx + (tmpnd.tmpw >> 1);
        tmpnd.tmpy = tmpnd.tmpy + (tmpnd.tmph >> 1);
        tmpnd.tmpa = tmpnd.tmpw * tmpnd.tmph;
    }
    // 入队
    now = queloc.insert(tmpnd);
    // printf("now = %d\n", now);
    // 求状态
    state = queloc.findstate(now, SMOOTH);
    // 纠错 -- 先不管
    if((queloc.allframe % 10 == 1) && (queloc.isfull)) queloc.correct();// -- 这个有问题
    return state;
}
