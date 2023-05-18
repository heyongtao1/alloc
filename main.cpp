#include <iostream>
#include <memory>
#include "alloc_thread.h"
using namespace std;

class test{
public:
    test(){}
public:
    int a[100];
};

int main()
{
    alloc::alloc_thread at;
    test* t = (test*)at.allocate(sizeof(test));
    //test* t1 = (test*)at.allocate(sizeof(test));
    cout << "alloc" << endl;
    for(int i=0;i<100;i++){
        t->a[i] = 1000 + i;
        //t1->a[i] = 1000 + i;
    }
        
    for(int i=0;i<100;i++){
        cout << t->a[i] << " ";
        //cout << t1->a[i] << " ";
    }
        
    at.delete_node(t,sizeof(t));
}
