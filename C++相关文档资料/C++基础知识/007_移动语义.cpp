#include <iostream>
#include <stdio.h>


#define LOG(...)   { \
    do {                                                              \
        printf("__FILE__ = %s,__LINE__ = %d\t", __FILE__, __LINE__);  \
        printf(__VA_ARGS__);                                          \
    while (0);                                                        \
}

class moveClass
{
public:
    moveClass(int value = 0) 
        : _move(new int(value))    
    {
        LOG("_nptr = %d", ++_nptr)
    }

    moveClass(const moveClass &moveclass){
        LOG("_cptr = %d", ++_cptr);
    }
    // 右值引用实现移动拷贝
    moveClass(moveClass &&moveclass) 
        : _move(moveClass._move)  // 当前对象指向赋值对象堆内存
    {
        moveClass._move = nullptr; // 将赋值对象指向空
        LOG("_mptr = %d", ++_mptr);
    }

    ~moveClass() {
        delete _move;
        LOG("_dptr = %d", ++_dptr);
    }

//private:
    int          *_move;
    static  int   _nptr;
    static  int   _cptr;
    static  int   _dptr;
    static  int   _mptr;
};

int moveClass::_nptr = 0;
int moveClass::_cptr = 0;
int moveClass::_dptr = 0;
int moveClass::_mptr = 0;


moveClass getMoveClassObject()
{
    moveClass moveclass(10);
    std::cout << "Resource from " << __func__ << ":" << hex << moveclass._move << std::endl;
    return moveclass;
}



int main(int argc, char *argv[])
{

    moveClass move = getMoveClassObject();


    return 0;
}