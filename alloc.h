#ifndef _ALLOC_POOL_H
#define _ALLOC_POOL_H

#include <memory.h>

template<class T,int TypeSize,int ObjectSize = 20>
class AllocPool{
public: 
    AllocPool(){
        freelist  = nullptr;
        blocklist = nullptr;
    }
    ~AllocPool(){
        //delete block
        while(blocklist){
            auto block_next = blocklist->next;
            delete blocklist;
            blocklist = block_next;
        }
    }

    //alloc
    void* alloc();
    //free 
    void free(void *object);

private: 
    struct ObjectAlloc{
        ObjectAlloc* next;
        char data[TypeSize];
        ObjectAlloc():next(nullptr){}
    };

    struct BlockAlloc{
        BlockAlloc* next;
        ObjectAlloc object[ObjectSize];
        BlockAlloc():next(nullptr){}
    };
private: 
    //free node 
    ObjectAlloc* freelist;
    //malloc block
    BlockAlloc*  blocklist;
};

template<class T,int TypeSize,int ObjectSize>
void* AllocPool<T,TypeSize,ObjectSize>::alloc(){
    if(blocklist == nullptr){
        //malloc new block
        BlockAlloc* block = new BlockAlloc();
        blocklist->next = block;
        blocklist = block;
        //block all object connect to freelist , block[0] is freelist head node
        for(int i=1;i<ObjectSize;i++){
            block->object[i-1].next = block->object[i];
        }
        freelist = block->object[0];
        //head insert to blocklist
        {
            if(blocklist == nullptr){
                blocklist = block;
            }else{
                block->next = blocklist;
                blocklist = block;
            }
        }
    }
    //return objectlist head node
    auto alloc_mem = (void*)freelist;
    freelist = freelist->next;
    return alloc_mem;
}

template<class T,int TypeSize,int ObjectSize>
void AllocPool<T,TypeSize,ObjectSize>::free(void* object){
    auto free_ptr = (ObjectAlloc*)object;
    free_ptr->next = freelist;
    freelist = free_ptr;
}

#endif