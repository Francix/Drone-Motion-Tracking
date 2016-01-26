#include "head_streamed.h"
#include "head_test.h"

/*
Class MoveState
{
private:
    int x;
    int y;
    int a;
public:
    MoveState(){}
    MoveState(int x, int y, int a);
    bool operator == (MoveState mv);
    bool operator != (MoveState mv);
};
*/

MoveState::MoveState(int x, int y, int a)
{
    this->x = x;
    this->y = y;
    this->a = a;
}

bool MoveState::operator == (MoveState mv)
{
    if((this->x == mv.x) && (this->y == mv.y) && (this->a == mv.a)) return true;
    else return false;
}

bool MoveState::operator != (MoveState mv)
{
    if((this->x == mv.x) && (this->y == mv.y) && (this->a == mv.a)) return false;
    else return true;
}
