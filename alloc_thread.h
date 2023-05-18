#ifndef _ALLOC_THREAD_H_
#define _ALLOC_THREAD_H_

#include <malloc.h>
#include "alloc_node_list.h"
#include "alloc_center.h"
namespace alloc{

class alloc_thread{
public: 
    typedef alloc_node_list anl;
    alloc_thread(){
        //pre_alloc(5);
    }
    ~alloc_thread(){
        
    }
public: 
    alloc_node* from_center_alloc(size_t node_align_size,size_t indx){
        assert(node_align_size > 0);
        alloc_node *start,*end;
        size_t alloc_size = alloc_center::instance()->alloc_node_range(start,end,node_align_size,10);
        std::cout << "instance alloc SIZE : " << alloc_size << std::endl;
        thread_free_list[indx].push_range(next_node(start),end,alloc_size-1);
        return start;
    }
    alloc_node* allocate(size_t size){
        assert(size > 0 && size <= MAX_BYTE);
        size_t alignsize = common::SizeClass::RoundUp(size);
        size_t indx = common::SizeClass::Index(size);
        if(!thread_free_list[indx].empty()){
            //return front node
            alloc_node* front = thread_free_list[indx].pop();
            thread_free_list[indx].get_node_list() = next_node(front);
            return front;
        }
        std::cout << "from_center_alloc" << std::endl;
        //malloc 
        //alloc_node* sys_alloc = malloc(alignsize);
        return from_center_alloc(alignsize,indx);
    }
    void  delete_node(alloc_node* node,size_t size){
        assert(size > 0 && size <= MAX_BYTE);
        size_t indx = common::SizeClass::Index(size);
        thread_free_list[indx].push(node);
    }
private:  
    void pre_alloc(size_t size){
        //thread_free_list[ALLOC_NODE_LIST_MAX] all alloc size
        for(int i=0;i<ALLOC_NODE_LIST_MAX;i++){
            //alloc_node* pre_node_arr = malloc(size*common::SizeClass::alignSize(i));
            alloc_node *start , *end;
            for(int j=0;j<size;j++){
                if(j==0){
                    start = malloc(common::SizeClass::alignSize(i));
                    end = start;
                    continue;
                }else{
                    alloc_node* new_node = malloc(common::SizeClass::alignSize(i));
                    next_node(new_node) = start;
                    start = new_node;
                }
            }
            thread_free_list[i].push_range(start,end,size);
        }
    }
private: 
    anl thread_free_list[ALLOC_NODE_LIST_MAX];
};

};

#endif