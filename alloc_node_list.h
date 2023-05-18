#ifndef _ALLOC_NODE_LIST_H_
#define _ALLOC_NODE_LIST_H_
#include <assert.h>
#include <sys/types.h>
#include "alloc_common.h"
namespace alloc{

class alloc_node_list{
private: 
    alloc_node* node_list;
    size_t list_size;
public: 
    alloc_node_list():node_list(nullptr),list_size(0){}
    //insert front
    void push(alloc_node* node){
        if(node_list == nullptr){
            node_list = node;
            list_size++;
            return ;
        }
        next_node(node) = node_list;
        node_list = node;
        list_size++;
    }
    //range [start,end] insert size node to front , [start,end] must sort
    void push_range(alloc_node* start,alloc_node* end,int size){
        assert(size >= 0);
        if(node_list == nullptr){
            node_list = start;
            list_size += size;
            return ;
        }
        next_node(end) = node_list;
        node_list = start;
        list_size += size;
    }
    //pop front
    alloc_node* pop(){
        assert(list_size > 0);
        alloc_node* front = node_list;
        node_list = next_node(node_list);
        list_size--;
        return front;
    }
    //get front node
    alloc_node* front(){
        assert(list_size > 0);
        return node_list;
    }
    //get list length
    int size(){
        return list_size;
    }
    //is empty
    bool empty(){
        return list_size == 0 ? true : false;
    }
    alloc_node*& get_node_list(){
        return node_list;
    }
};

};

#endif