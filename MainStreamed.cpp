#include "head_main.h"
#include "head_test.h"

int main(int argc, char **argv)
{
    //  这里需要建立学习机制 -- 怎么学？ -- 选定量描述最平衡的呗
    //  读文件 -- 设参数 -- 跑程序 -- 定量记录效果 -- 输出
    //  如何定量：
    //      1.精确度 -- 越精确，越平滑，延时应该越高
    //      2.灵敏度 -- 越灵敏，延时越低，越不平滑，出错率应该越高
    //      3.稳定性 -- 越稳定，“抖动”的点越少
    //      3.预测准确率
    //      4.纠错成功率 -- 3和4是相关的
    //  哪几个参数需要被设置？
    //  xcnt, xdelay, acnt, adelay
    int tmpstate;
    char filename[50];
    char readbuffer[512];
    int i, j;
    int frameid, tmpx, tmpy, tmpw, tmph;
    float tmpf;
    int totalloc, totalline;
    ifstream infile;

    strcpy(filename, argv[1]);
    printf("file:%s\n", filename);
    infile.open(filename, ios::in);
    if(!infile)
    {
        printf("fail to open file!\n");
        exit(1);
    }
    while(infile.getline(readbuffer, sizeof(readbuffer)))
    {
        totalline++;
    }
    totalloc = totalline - 4;
    infile.close();

    infile.open(filename, ios::in);
    if(!infile)
    {
        printf("fail to open file!\n");
        exit(1);
    }

    i = 0;
    while(infile.getline(readbuffer, sizeof(readbuffer)))
    {
        sscanf(readbuffer, "%d %f %d %d %d %d", &frameid, &tmpf,
        &tmpx, &tmpy, &tmpw, &tmph);

        parameters para = ParameterGenerator();

        tmpstate = TracingStreamed(frameid, tmpx, tmpy, tmpw, tmph, para.xcnt, para.acnt,
        para.xdelay, para.adelay, para.len, para.interval, para.jpredict);

        #ifdef FRAMEWISE
        printf("id: %d\t", frameid);
        CountTime(frameid);
        if(tmpstate == STAY) printf("STAY \tSTAY \t");
        else if(tmpstate == LEFT) printf("STAY \tLEFT \t");
        else if(tmpstate == RIGHT) printf("STAY \tRIGHT\t");
        else if(tmpstate == FRONT) printf("FRONT\tSTAY \t");
        else if(tmpstate == BACK) printf("BACK \tSTAY \t");
        else if(tmpstate == LEFTFRONT) printf("FRONT\tLEFT \t");
        else if(tmpstate == LEFTBACK) printf("BACK \tLEFT \t");
        else if(tmpstate == RIGHTFRONT) printf("FRONT\tRIGHT\t");
        else if(tmpstate == RIGHTBACK) printf("BACK \tRIGHT\t");
        if(tmpx == -1) printf("pridicted!");
        printf("\n");
        #endif
        i++;
        if(i == totalloc) break;
    }
    infile.close();

    return 0;
}
