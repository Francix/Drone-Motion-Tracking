#include "head_streamed.h"
#include "head_test.h"

/*
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
    void init(int xcnt, int acnt, int xdelay, int adelay, int l = 30, int interval = 15, int jpredict = 5);
    // 返回下一个index
    inline int nextindex(int i);
    // 返回前一个index
    inline int previndex(int i);
    // 入队函数，控制自动出队，返回队尾指针
    int insert(LocationNode tmpnd);
    // 计算当前状态并返回
    int findstate(int tmpindex, int mode = SMOOTH);
    // 根据现在的状态坐标，预测下一个状态，返回一个LocationNode结构
    LocationNode pridict(int tmpindex);
    // 纠正错误点
    void correct();
    // 声明友元函数
    friend int TracingStreamed(int frameid, int x, int y, int width, int height,
        int xcnt, int acnt, int xdelay, int adelay, int l, int interval, int jpredict)
}
*/

// 初始化函数 -- 这个算是写好了吧
void CQueueLocation::init(int mode, int xdelay, int ydelay, int adelay,
    int xdivisior, int ydivisior,int adivisior, int len, int interval, int jpredict)
{
    pnow = -1;
    isfull = false;
    allframe = 0;
    pfacloc = new CFaceLocation[len];
    failcnt = 0;
    xlimit = 1920;
    ylimit = 1080;

    this->mode = mode;
    this->xdelay = xdelay;
    this->ydelay = ydelay;
    this->adelay = adelay;
    this->xdivisior = 8;
    this->ydivisior = 12;
    this->adivisior = 20;
    this->len = len; // 使用了this指针，避免名称歧义
    this->interval = interval;
    this->jpredict = jpredict;
}

// 析构函数 -- 这个应该算是写好了
CQueueLocation::~CQueueLocation()
{
    delete []pfacloc;
}

// 求前一个坐标 -- 写好了
inline int CQueueLocation::previndex(int i)
{
    return (i - 1 + len) % len;
}

// 求下一个坐标 -- 写好了
inline int CQueueLocation::nextindex(int i)
{
    return (i + 1) % len;
}

// 新节点入队，返回新节点指针 -- 要求新节点必须处理好 -- 写好了
int CQueueLocation::insert(LocationNode tmpnd)
{
    pnow = nextindex(pnow);
    pfacloc[pnow].xlabel = tmpnd.tmpx;
    pfacloc[pnow].ylabel = tmpnd.tmpy;
    pfacloc[pnow].width = tmpnd.tmpw;
    pfacloc[pnow].height = tmpnd.tmph;
    pfacloc[pnow].area = tmpnd.tmpa;
    pfacloc[pnow].isfail = tmpnd.isfail;
    if(tmpnd.isfail == true)
    {
        failcnt++;
        if(failcnt > 15) failstate = true;
    }
    else
    {
        failcnt = 0;
        failstate = false;
    }
    allframe++;
    if(allframe == len)
    {
        isfull = true;
    }
    return pnow;
}

// 求当前节点的状态 -- 写好了
MoveState CQueueLocation::findstate(int tmpindex)
{
    // 位置均值变量，与平滑后的当前位置变量做比较，来判定当前状态
    if(failstate == true)
    {
        MoveState state(UNKNOW, UNKNOW, UNKNOW);
        return state;
    }
    int xaverage, aaverage, yaverage;
    // 平滑后的当前位置变量
    int anear, xnear, ynear;
    // 辅助变量
    int xsum, asum, ysum;
    int wsum, hsum;
    // 阈值
    int athreshold, xthreshold, ythreshold;
    // 二次平滑模式，辅助计数变量
    int xstaycnt, leftcnt, rightcnt;
    int ystaycnt, upcnt, downcnt;
    int astaycnt, frontcnt, backcnt;
    // 辅助状态记录变量
    MoveState state;
    // 遍历时的下标
    int i, j;

    // 如果队列未满，直接返回stay
    if(!isfull)
    {
        // 这里待填补 -- 不填了
        state.x = UNKNOW;
        state.y = UNKNOW;
        state.a = UNKNOW;
        return state;
    }
    else
    {
        // 求x方向average与threshold
        xsum = 0;
        wsum = 0;
        i = tmpindex;
        j = 0;
        for(j = 0; j <= xdelay; j++) i = previndex(i);
        for(j = 0; j < 6; i = previndex(i), j++)
        {
            xsum += pfacloc[i].xlabel;
            wsum += pfacloc[i].width;
        }
        xaverage = xsum / 6;
        xthreshold = wsum / (xdivisior * 6);

        // x方向平滑，使用线性衰减
        xnear = 0;
        for(i = tmpindex, j = 5; j >=1; i = previndex(i), j--)
        {
            xnear += (j * pfacloc[i].xlabel);
        }
        xnear = xnear / 15;

        // 求x方向的状态
        if((xnear > xaverage) && (xnear - xaverage) > xthreshold) state.x = RIGHT;
        else if((xnear < xaverage) && (xaverage - xnear > xthreshold)) state.x = LEFT;
        else state.x = STAY;

        // 求x方向average与threshold
        ysum = 0;
        hsum = 0;
        i = tmpindex;
        j = 0;
        for(j = 0; j <= ydelay; j++) i = previndex(i);
        for(j = 0; j < 6; i = previndex(i), j++)
        {
            ysum += pfacloc[i].ylabel;
            hsum += pfacloc[i].height;
        }
        yaverage = ysum / 6;
        ythreshold = hsum / (ydivisior * 6);

        // x方向平滑，使用线性衰减
        ynear = 0;
        for(i = tmpindex, j = 3; j >=1; i = previndex(i), j--)
        {
            ynear += (j * pfacloc[i].ylabel);
        }
        ynear = ynear / 6;

        // 求y方向的状态
        if((ynear > yaverage) && (ynear - yaverage) > ythreshold) state.y = UP;
        else if((ynear < yaverage) && (yaverage - ynear > ythreshold)) state.y = DOWN;
        else state.y = STAY;

        // 求a方向的average与threshold
        asum = 0;
        i = tmpindex;
        j = 0;
        // len >= adelay + acnt
        for(j = 0; j <= adelay; j++) i = previndex(i);
        for(j = 0; j < 4; i = previndex(i), j++) asum += pfacloc[i].area;
        aaverage = asum / 4;
        athreshold = aaverage / adivisior;

        // a方向平滑
        anear = 0;
        for(i = tmpindex, j = 5; j >=1; i = previndex(i), j--)
        {
            // 使用线性衰减模型
            anear += (j * pfacloc[i].area);
        }
        anear = anear / 15;

        // 求a方向状态
        if((anear > aaverage) && (anear - aaverage) > athreshold) state.a = FRONT;
        else if((anear < aaverage) && (aaverage - anear > athreshold)) state.a = BACK;
        else state.a = STAY;

        // 状态整合
        pfacloc[tmpindex].state = state;

        // 根据不同模式，返回当前状态
        if(mode == ACCURATE)
        {
            // 精确模式
            return pfacloc[tmpindex].state;
        }
        else
        {
            // 二次平滑模式
            xstaycnt = leftcnt = rightcnt = 0;
            ystaycnt = upcnt = downcnt = 0;
            astaycnt = frontcnt = backcnt = 0;

            // 暴力计数平滑，权值相等
            for(i = tmpindex, j = 0; j < interval; j++, i = previndex(i))
            {
                switch(pfacloc[i].state.x)
                {
                    case LEFT:
                    leftcnt++;
                    break;
                    case RIGHT:
                    rightcnt++;
                    break;
                    case STAY:
                    xstaycnt++;
                    break;
                    default:
                    xstaycnt++;
                    break;
                }
                switch(pfacloc[i].state.y)
                {
                    case UP:
                    upcnt++;
                    break;
                    case DOWN:
                    downcnt++;
                    break;
                    case STAY:
                    ystaycnt++;
                    break;
                    default:
                    ystaycnt++;
                    break;
                }
                switch(pfacloc[i].state.a)
                {
                    case FRONT:
                    frontcnt++;
                    break;
                    case BACK:
                    backcnt++;
                    break;
                    case STAY:
                    astaycnt++;
                    break;
                    default:
                    astaycnt++;
                    break;
                }
            }

            // 求x方向状态
            if(leftcnt > (interval / 2)) state.x = LEFT;
            else if(rightcnt > (interval / 2)) state.x = RIGHT;
            else state.x = STAY;

            // 求y方向状态
            if(upcnt > (interval / 2)) state.y = UP;
            else if(downcnt > (interval / 2)) state.y = DOWN;
            else state.y = STAY;

            // 求a方向状态
            if(frontcnt > (interval / 2)) state.a = FRONT;
            else if(backcnt > (interval / 2)) state.a = BACK;
            else state.a = STAY;

            // 状态整合，返回状态
            return state;
        }
    }
}

// 预测函数，如果当前fail，则（离散化）线性预测 -- 也算写好了，不过这个和最小二乘的结果是不是相等还有待证明
LocationNode CQueueLocation::pridict(int tmpindex)
{
    LocationNode nd;
    int i, j;
    int xgrad, xgradsum = 0, xaverage, xsum = 0;
    int ygrad, ygradsum = 0, yaverage, ysum = 0;
    int wgrad, wgradsum = 0, waverage, wsum = 0;
    int hgrad, hgradsum = 0, haverage, hsum = 0;
    i = tmpindex;
    if(allframe < jpredict + 3)
    {
        nd.tmpx = pfacloc[i].xlabel;
        nd.tmpy = pfacloc[i].ylabel;
        nd.tmpw = pfacloc[i].width;
        nd.tmph = pfacloc[i].height;
        nd.tmpa = pfacloc[i].area;
    }
    else
    {
        // 求平均值与平均梯度
        for(i = tmpindex, j = 0; j < jpredict; i = previndex(i), j++)
        {
            xgradsum += pfacloc[i].xlabel - pfacloc[previndex(i)].xlabel;
            ygradsum += pfacloc[i].ylabel - pfacloc[previndex(i)].ylabel;
            wgradsum += pfacloc[i].width - pfacloc[previndex(i)].width;
            hgradsum += pfacloc[i].height - pfacloc[previndex(i)].height;
            xsum += pfacloc[i].xlabel;
            ysum += pfacloc[i].ylabel;
            wsum += pfacloc[i].width;
            hsum += pfacloc[i].height;
        }
        xgrad = xgradsum / jpredict;
        ygrad = ygradsum / jpredict;
        wgrad = wgradsum / jpredict;
        hgrad = hgradsum / jpredict;
        xaverage = xsum / jpredict;
        yaverage = ysum / jpredict;
        waverage = wsum / jpredict;
        haverage = hsum / jpredict;
        nd.tmpx = xaverage + ((jpredict + 1)/ 2) * xgrad;
        nd.tmpy = yaverage + ((jpredict + 1)/ 2) * ygrad;
        nd.tmpw = waverage + ((jpredict + 1)/ 2) * wgrad;
        nd.tmph = haverage + ((jpredict + 1)/ 2) * hgrad;
        nd.tmpa = nd.tmpw * nd.tmph;
        if((nd.tmpx > xlimit) || (nd.tmpy > ylimit) || (nd.tmpx < 0) || (nd.tmpy < 0)) failstate = true;
    }
    return nd;
}

// 纠错函数,每十帧判定当前状态 - 找到错误点 - 预测 -- 这个有问题
void CQueueLocation::correct()
{
    // 循环变量
    int i, j;
    // 辅助计数变量
    int xstaycnt = 0, leftcnt = 0, rightcnt = 0;
    int ystaycnt = 0, upcnt = 0, downcnt = 0;
    int astaycnt = 0, frontcnt = 0, backcnt = 0;
    // 辅助状态记录变量
    MoveState tmpstate;
    // 辅助节点
    LocationNode tmpnd;

    // 开始求前10帧的状态
    i = previndex(pnow);
    for(j = 0; j < 10; j++, i = previndex(i))
    {
        switch(pfacloc[i].state.x)
        {
            case LEFT:
            leftcnt++;
            break;
            case RIGHT:
            rightcnt++;
            break;
            case STAY:
            xstaycnt++;
            break;
            default:
            xstaycnt++;
            break;
        }
        switch(pfacloc[i].state.y)
        {
            case UP:
            upcnt++;
            break;
            case DOWN:
            downcnt++;
            break;
            case STAY:
            ystaycnt++;
            break;
            default:
            ystaycnt++;
            break;
        }
        switch(pfacloc[i].state.a)
        {
            case FRONT:
            frontcnt++;
            break;
            case BACK:
            backcnt++;
            break;
            case STAY:
            astaycnt++;
            break;
            default:
            astaycnt++;
            break;
        }
    }

    // 求x方向状态
    if(leftcnt > (interval / 2)) tmpstate.x = LEFT;
    else if(rightcnt > (interval / 2)) tmpstate.x = RIGHT;
    else tmpstate.x = STAY;

    // 求y方向状态
    if(upcnt > (interval / 2)) tmpstate.y = UP;
    else if(downcnt > (interval / 2)) tmpstate.y = DOWN;
    else tmpstate.y = STAY;

    // 求a方向状态
    if(frontcnt > (interval / 2)) tmpstate.a = FRONT;
    else if(backcnt > (interval / 2)) tmpstate.a = BACK;
    else tmpstate.a = STAY;

    // 开始纠错

    #ifdef TEST
    CountTime(allframe - 1);
    CountTime(allframe - 11);
    switch(tmpstate.x)
    {
        case LEFT:
        printf("LEFT \t");
        break;
        case RIGHT:
        printf("RIGHT\t");
        break;
        case STAY:
        printf("STAY \t");
        break;
    }
    switch(tmpstate.y)
    {
        case UP:
        printf("UP   \t");
        break;
        case RIGHT:
        printf("DOWN \t");
        break;
        case STAY:
        printf("STAY \t");
        break;
    }
    switch(tmpstate.a)
    {
        case FRONT:
        printf("FRONT\t");
        break;
        case BACK:
        printf("BACK \t");
        break;
        case STAY:
        printf("STAY \t");
        break;
    }
    printf("\n");
    #endif

    i = previndex(pnow);
    for(j = 0; j < 10; j++) i = previndex(i);
    for(j = 0; j < 10; j++, i = nextindex(i))
    {
        if(pfacloc[i].state != tmpstate)
        {
            //printf("find a wrong frame\n");
            tmpnd = pridict(previndex(i));
            pfacloc[i].xlabel = tmpnd.tmpx;
            pfacloc[i].ylabel = tmpnd.tmpy;
            pfacloc[i].width = tmpnd.tmpw;
            pfacloc[i].height = tmpnd.tmph;
            pfacloc[i].area = tmpnd.tmpa;
        }
    }
}
