#include "head.h"

/*
class CFaceLocation
{
public:
    int total;
    int number;

    int minite;
    int second;
    int minisecond;

    int xlabel;
    int ylabel;
    int width;
    int height;
    int area;

    int state;

    CFaceLocation();
    CFaceLocation(int num, int x, int y, int wid, int hei);

    void ShowFaceLoc(int x);
    void CountTime();
    void FindState_1(CFaceLocation *pfacloc, int i);
};
*/

CFaceLocation::CFaceLocation()
{
    state = STAY;
    ispredicted = false;

    aave = -1;
    athe = -1;
    adiff = -1;
}

CFaceLocation::CFaceLocation(int num, int x, int y, int wid, int hei)
{
    state = STAY;
    ispredicted = false;
    number = num;
    xlabel = x;
    ylabel = y;
    width = wid;
    height = hei;

    aave = -1;
    athe = -1;
    adiff = -1;
    // x轴与y轴的数据待完善
}

void CFaceLocation::ShowFaceLoc(int x = 0, int mode)
{
    if(x & (1 << 1))printf("num:\t%.4d\ttime:\t%.2d:%.2d:%.3d\n", number, minite, second, minisecond);
    printf("x:%d\ty:%d\tarea:%d\t", xlabel, ylabel, area);
    if(ispredicted) printf("PRIDICTED");
    printf("\n");
    if(x & 1)
    {
        if(state == STAY)
        {
            printf("STAY \t");
            // if(mode  == FRONTBACK) printf("aave = %d\tathe = %d\tadiff = %d\t", aave, athe, adiff);
            // 上面注释掉的那一句用来对参数做更细粒度的分析
        }
        if(mode & LEFTRIGHT)
        {
            if((state & LEFT) || (state & RIGHT))
            {
                if(state & LEFT) printf("LEFT \t");
                if(state & RIGHT) printf("RIGHT\t");
            }
            else printf("\t\t");
        }
        /*
        if(mode & UPDOWN)
        {
            if((state & UP) || (state & DOWN))
            {
                if(state & UP) printf("UP   \t");
                if(state & DOWN) printf("DOWN \t");
            }
            else printf("\t\t");
        }
        */
        if(mode & FRONTBACK)
        {
            if(state & FRONT) printf("FRONT\t");
            else if(state & BACK) printf("BACK \t");
        }
        printf("\n");
    }
}

void CFaceLocation::CountTime()
{
    second = ((number << 1) / 60) % 60;
    minite = ((number << 1) / 60) / 60;
    minisecond = (((number << 1) % 60) * 1000) / 60;
}

void CFaceLocation::FindState_1(CFaceLocation *pfacloc)
{
    int xaverage, yaverage, aaverage;
    int xsum, ysum, asum;
    int wsum, hsum;
    int athreshold, xthreshold, ythreshold;
    int num = number;
    int i , xcnt = 6, ycnt = 6, acnt = 8, adelay = 20;
    // 动态threshold，取前cnt个数据的平均值
    if(num < xcnt);
    else
    {
        xsum = 0;
        wsum = 0;
        for(i = num - 1; i >= num - xcnt; i--)
        {
            xsum += pfacloc[i].xlabel;
            wsum += pfacloc[i].width;
        }
        xaverage = xsum / xcnt;
        xthreshold = wsum / (8 * xcnt);
        // 这个可能有问题，如果只是左右移动，则效果比较好(对near-timelog.txt效果不错)
        // 但是在较远的时候，并且加上前后移动的话，就过于灵敏了
        // 没有考虑框框变大的情况
        if((xlabel > xaverage) && (xlabel - xaverage > xthreshold)) state = state | RIGHT;
        else if((xlabel < xaverage) && (xaverage - xlabel > xthreshold)) state = state | LEFT;
    }
    /* 这里没有实现y轴方向的运动状态分析
    if(num < ycnt);
    else
    {
        ysum = 0;
        hsum = 0;
        for(i = num - 1; i >= num - ycnt; i--)
        {
            ysum += pfacloc[i].ylabel;
            hsum += pfacloc[i].height;
        }
        yaverage = ysum / ycnt;
        ythreshold = hsum / (8 * ycnt);
        if((ylabel > yaverage) && (ylabel - yaverage > ythreshold)) state = state | UP;
        else if((ylabel < yaverage) && (yaverage - ylabel > ythreshold)) state = state | DOWN;
    }
    */
    if(num < acnt + adelay);
    else
    {
        asum = 0;
        // 对于面积的比较，添加了一个delay参数，调到20帧，效果到目前为止比较好
        for(i = num - adelay - 1; i >= num - acnt - adelay; i--)
        {
            asum += pfacloc[i].area;
        }
        aaverage = asum / acnt;
        athreshold = asum / (12 * acnt);
        neararea = (4 * area + 2 * pfacloc[num - 1].area + 2 * pfacloc[num - 2].area) >> 3;
        if((area > aaverage) && (area - aaverage > athreshold)) state = state | FRONT;
        else if((area < aaverage) && (aaverage - area > athreshold)) state = state | BACK;
        else
        {
            aave = aaverage;
            athe = athreshold;
            adiff = abs(aave - area);
        }
    }
}
