#include "head_main.h"
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
    int insert(node tmpnd);
    // 计算当前状态并返回
    int findstate(int tmpindex, int mode = SMOOTH);
    // 根据现在的状态坐标，预测下一个状态，返回一个node结构
    node pridict(int tmpindex);
    // 纠正错误点
    void correct();
    // 声明友元函数
    friend int TracingStreamed(int frameid, int x, int y, int width, int height,
        int xcnt, int acnt, int xdelay, int adelay, int l, int interval, int jpredict)
}
*/

// 初始化函数 -- 这个算是写好了吧
void CQueueLocation::init(int xcnt, int acnt, int xdelay,
    int adelay, int len, int interval, int jpredict)
{
    printf("Initializing...\n");
    pnow = -1;
    isfull = false;
    allframe = 0;
    pfacloc = new CFaceLocation[len];
    this->len = len; // 使用了this指针，避免名称歧义
    this->xcnt = xcnt;
    this->acnt = acnt;
    this->xdelay = xdelay;
    this->adelay = adelay;
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
int CQueueLocation::insert(node tmpnd)
{
    pnow = nextindex(pnow);
    pfacloc[pnow].xlabel = tmpnd.tmpx;
    pfacloc[pnow].ylabel = tmpnd.tmpy;
    pfacloc[pnow].width = tmpnd.tmpw;
    pfacloc[pnow].height = tmpnd.tmph;
    pfacloc[pnow].area = tmpnd.tmpa;
    allframe++;
    if(allframe == len)
    {
        isfull = true;
        //printf("queue full! \n");
    }
    return pnow;
}

// 求当前节点的状态 -- 写好了
int CQueueLocation::findstate(int tmpindex, int mode)
{
    // 位置均值变量，与平滑后的当前位置变量做比较，来判定当前状态
    int xaverage, aaverage;
    // 平滑后的当前位置变量
    int anear, xnear;
    // 辅助变量
    int xsum, asum;
    int wsum, hsum;
    // 阈值
    int athreshold, xthreshold;
    // 二次平滑模式，辅助计数变量
    int xstaycnt, leftcnt, rightcnt;
    int astaycnt, frontcnt, backcnt;
    // 辅助状态记录变量
    int LRstate;
    int FBstate;
    // 遍历时的下标
    int i, j;

    // 如果队列未满，直接返回stay
    if(!isfull)
    {
        // 这里待填补 -- 要填啊不要偷懒啊少年 -- 一百五十多行呢想到都怕 -- 哎呀还是想想具体应该怎么填
        return STAY;
    }
    else
    {
        // 求x方向average与threshold
        xsum = 0;
        wsum = 0;
        i = tmpindex;
        j = 0;
        for(j = 0; j <= xdelay; j++) i = previndex(i);
        for(j = 0; j < xcnt; i = previndex(i), j++)
        {
            xsum += pfacloc[i].xlabel;
            wsum += pfacloc[i].width;
        }
        xaverage = xsum / xcnt;
        xthreshold = wsum / (8 * xcnt);

        // x方向平滑，使用线性衰减
        xnear = 0;
        for(i = tmpindex, j = 5; j >=1; i = previndex(i), j--)
        {
            xnear += (j * pfacloc[i].xlabel);
        }
        xnear = xnear / 15;

        // 求x方向的状态
        if((xnear > xaverage) && (xnear - xaverage) > xthreshold) LRstate = LEFT;
        else if((xnear < xaverage) && (xaverage - xnear > xthreshold)) LRstate = RIGHT;
        else LRstate = STAY;

        // 求a方向的average与threshold
        asum = 0;
        i = tmpindex;
        j = 0;
        // len >= adelay + acnt
        for(j = 0; j <= adelay; j++) i = previndex(i);
        for(j = 0; j < acnt; i = previndex(i), j++) asum += pfacloc[i].area;
        aaverage = asum / acnt;
        athreshold = aaverage / 20;

        // a方向平滑
        anear = 0;
        for(i = tmpindex, j = 5; j >=1; i = previndex(i), j--)
        {
            // 使用线性衰减模型
            anear += (j * pfacloc[i].area);
        }
        anear = anear / 15;

        // 求a方向状态
        if((anear > aaverage) && (anear - aaverage) > athreshold) FBstate = FRONT;
        else if((anear < aaverage) && (aaverage - anear > athreshold)) FBstate = BACK;
        else FBstate = STAY;

        // 状态整合
        if(LRstate == STAY && FBstate == STAY) pfacloc[tmpindex].state = STAY;
        else if(LRstate == LEFT && FBstate == STAY) pfacloc[tmpindex].state = LEFT;
        else if(LRstate == RIGHT && FBstate == STAY) pfacloc[tmpindex].state = RIGHT;
        else if(LRstate == STAY && FBstate == FRONT) pfacloc[tmpindex].state = FRONT;
        else if(LRstate == STAY && FBstate == BACK) pfacloc[tmpindex].state = BACK;
        else if(LRstate == LEFT && FBstate == FRONT) pfacloc[tmpindex].state = LEFTFRONT;
        else if(LRstate == LEFT && FBstate == BACK) pfacloc[tmpindex].state = LEFTBACK;
        else if(LRstate == RIGHT && FBstate == FRONT) pfacloc[tmpindex].state = RIGHTFRONT;
        else if(LRstate == RIGHT && FBstate == BACK) pfacloc[tmpindex].state = RIGHTBACK;

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
            astaycnt = frontcnt = backcnt = 0;

            // 暴力计数平滑，权值相等
            for(i = tmpindex, j = 0; j < interval; j++, i = previndex(i))
            {
                if(pfacloc[i].state == STAY)
                {
                    xstaycnt++;
                    astaycnt++;
                }
                else if(pfacloc[i].state == LEFT)
                {
                    leftcnt++;
                    astaycnt++;
                }
                else if(pfacloc[i].state == RIGHT)
                {
                    rightcnt++;
                    astaycnt++;
                }
                else if(pfacloc[i].state == FRONT)
                {
                    xstaycnt++;
                    frontcnt++;
                }
                else if(pfacloc[i].state == BACK)
                {
                    xstaycnt++;
                    backcnt++;
                }
                else if(pfacloc[i].state == LEFTFRONT)
                {
                    leftcnt++;
                    frontcnt++;
                }
                else if(pfacloc[i].state == RIGHTFRONT)
                {
                    rightcnt++;
                    frontcnt++;
                }
                else if(pfacloc[i].state == LEFTBACK)
                {
                    leftcnt++;
                    backcnt++;
                }
                else if(pfacloc[i].state == RIGHTBACK)
                {
                    rightcnt++;
                    backcnt++;
                }
            }

            // 求x方向状态
            if(leftcnt > (interval / 2)) LRstate = LEFT;
            else if(rightcnt > (interval / 2)) LRstate = RIGHT;
            else LRstate = STAY;

            // 求a方向状态
            if(frontcnt > (interval / 2)) FBstate = FRONT;
            else if(backcnt > (interval / 2)) FBstate = BACK;
            else FBstate = STAY;

            // 状态整合，返回状态
            if(LRstate == STAY && FBstate == STAY) return STAY;
            else if(LRstate == LEFT && FBstate == STAY) return LEFT;
            else if(LRstate == RIGHT && FBstate == STAY) return RIGHT;
            else if(LRstate == STAY && FBstate == FRONT) return FRONT;
            else if(LRstate == STAY && FBstate == BACK) return BACK;
            else if(LRstate == LEFT && FBstate == FRONT) return LEFTFRONT;
            else if(LRstate == LEFT && FBstate == BACK) return LEFTBACK;
            else if(LRstate == RIGHT && FBstate == FRONT) return RIGHTFRONT;
            else if(LRstate == RIGHT && FBstate == BACK) return RIGHTBACK;
            else return STAY;// 这句应该没用
        }
    }
}

// 预测函数，如果当前fail，则（离散化）线性预测 -- 也算写好了，不过这个和最小二乘的结果是不是相等还有待证明
node CQueueLocation::pridict(int tmpindex)
{
    node nd;
    int i, j;
    int xgrad, xgradsum = 0, xaverage, xsum = 0;
    int wgrad, wgradsum = 0, waverage, wsum = 0;
    int hgrad, hgradsum = 0, haverage, hsum = 0;
    i = tmpindex;
    if(allframe < jpredict + 3)
    {
        nd.tmpx = pfacloc[i].xlabel;
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
            wgradsum += pfacloc[i].width - pfacloc[previndex(i)].width;
            hgradsum += pfacloc[i].height - pfacloc[previndex(i)].height;
            xsum += pfacloc[i].xlabel;
            wsum += pfacloc[i].width;
            hsum += pfacloc[i].height;
        }
        xgrad = xgradsum / jpredict;
        wgrad = wgradsum / jpredict;
        hgrad = hgradsum / jpredict;
        xaverage = xsum / jpredict;
        waverage = wsum / jpredict;
        haverage = hsum / jpredict;
        nd.tmpx = xaverage + ((jpredict + 1)/ 2) * xgrad;
        nd.tmpw = waverage + ((jpredict + 1)/ 2) * wgrad;
        nd.tmph = haverage + ((jpredict + 1)/ 2) * hgrad;
        nd.tmpa = nd.tmpw * nd.tmph;
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
    int astaycnt = 0, frontcnt = 0, backcnt = 0;
    // 辅助状态记录变量
    int LRstate;
    int FBstate;
    int tmpstate;
    // 辅助节点
    node tmpnd;

    // 开始求前10帧的状态
    i = previndex(pnow);
    for(j = 0; j < 10; j++, i = previndex(i))
    {
        if(pfacloc[i].state == STAY)
        {
            xstaycnt++;
            astaycnt++;
        }
        else if(pfacloc[i].state == LEFT)
        {
            leftcnt++;
            astaycnt++;
        }
        else if(pfacloc[i].state == RIGHT)
        {
            rightcnt++;
            astaycnt++;
        }
        else if(pfacloc[i].state == FRONT)
        {
            xstaycnt++;
            frontcnt++;
        }
        else if(pfacloc[i].state == BACK)
        {
            xstaycnt++;
            backcnt++;
        }
        else if(pfacloc[i].state == LEFTFRONT)
        {
            leftcnt++;
            frontcnt++;
        }
        else if(pfacloc[i].state == RIGHTFRONT)
        {
            rightcnt++;
            frontcnt++;
        }
        else if(pfacloc[i].state == LEFTBACK)
        {
            leftcnt++;
            backcnt++;
        }
        else if(pfacloc[i].state == RIGHTBACK)
        {
            rightcnt++;
            backcnt++;
        }
    }

    // 求x方向状态
    if(leftcnt > (interval / 2)) LRstate = LEFT;
    else if(rightcnt > (interval / 2)) LRstate = RIGHT;
    else LRstate = STAY;

    // 求a方向状态
    if(frontcnt > (interval / 2)) FBstate = FRONT;
    else if(backcnt > (interval / 2)) FBstate = BACK;
    else FBstate = STAY;

    // 状态整合，返回状态
    if(LRstate == STAY && FBstate == STAY)
    {
        tmpstate = STAY;
    }
    else if(LRstate == LEFT && FBstate == STAY)
    {
        tmpstate = LEFT;
    }
    else if(LRstate == RIGHT && FBstate == STAY)
    {
        tmpstate = RIGHT;
    }
    else if(LRstate == STAY && FBstate == FRONT)
    {
        tmpstate = FRONT;
    }
    else if(LRstate == STAY && FBstate == BACK)
    {
        tmpstate = BACK;
    }
    else if(LRstate == LEFT && FBstate == FRONT)
    {
        tmpstate = LEFTFRONT;
    }
    else if(LRstate == LEFT && FBstate == BACK)
    {
        tmpstate = LEFTBACK;
    }
    else if(LRstate == RIGHT && FBstate == FRONT)
    {
        tmpstate = RIGHTFRONT;
    }
    else if(LRstate == RIGHT && FBstate == BACK)
    {
        tmpstate = RIGHTBACK;
    }
    // 开始纠错

    #ifdef TEST
    CountTime(allframe - 1);
    CountTime(allframe - 11);
    if(tmpstate == STAY) printf("STAY \tSTAY \t");
    else if(tmpstate == LEFT) printf("STAY \tLEFT \t");
    else if(tmpstate == RIGHT) printf("STAY \tRIGHT\t");
    else if(tmpstate == FRONT) printf("FRONT\tSTAY \t");
    else if(tmpstate == BACK) printf("BACK \tSTAY \t");
    else if(tmpstate == LEFTFRONT) printf("FRONT\tLEFT \t");
    else if(tmpstate == LEFTBACK) printf("BACK \tLEFT \t");
    else if(tmpstate == RIGHTFRONT) printf("FRONT\tRIGHT\t");
    else if(tmpstate == RIGHTBACK) printf("BACK \tRIGHT\t");
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
            findstate(ACCURATE, i);
        }
    }
}
