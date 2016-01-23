class CTimeLine
// 这个类用来做二阶平滑
{
public:
    int startnum, endnum;
    static int interval;
    int state;
    CTimeLine();
}

CTimeLine::CTimeLine(int start, int end, int stat)
{
    startnum = start;
    endnum = end;
    state = stat;
}

int CTimeLine::interval = 0;
