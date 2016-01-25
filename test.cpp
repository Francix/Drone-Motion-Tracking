#include "head_main.h"
#include "head_test.h"

void CountTime(int frameid)
{
    int second, minite, minisecond;
    second = ((frameid << 1) / 60) % 60;
    minite = ((frameid << 1) / 60) / 60;
    minisecond = (((frameid << 1) % 60) * 1000) / 60;
    printf("time: %.2d:%.3d\t", second ,minisecond);
}

parameters ParameterGenerator()
{
    parameters para;
    para.xcnt = 6;
    para.acnt = 4;
    para.xdelay = 0;
    para.adelay = 25;
    para.len = 30;
    // len >= adelay + acnt
    para.interval = 15;
    para.jpredict = 5;
    return para;
}
