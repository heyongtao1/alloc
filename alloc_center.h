#ifndef _ALLOC_CENTER_H
#define _ALLOC_CENTER_H
#include "alloc_common.h"
#include "alloc_span_list.h"
#include <unordered_map>
namespace alloc{

class alloc_center{
public:
    static alloc_center* instance(){
        static alloc_center center;
        return &center;
    }
private:
    span* sys_alloc_span(size_t node_align_size){
        assert(node_align_size > 0);
        span* new_span = new span();
        //sys_alloc 8k byte to node
        size_t alloc_node_size = 8*1024 / node_align_size;
        size_t bytes = alloc_node_size*node_align_size;
        void* ptr = common::sys_alloc(bytes);

        std::cout << "alloc addr : " << (PAGE_ID)ptr  << std::endl;
        new_span->pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
        new_span->page_n = bytes >> PAGE_SHIFT;

        char* start = (char*)(new_span->pageId << PAGE_SHIFT);
        char* end   = start + bytes;

        std::cout << "alloc_node_size = " << alloc_node_size << std::endl;
        std::cout << "start = " << (PAGE_ID)start << std::endl;
        new_span->node_list = start;
        if(next_node(new_span->node_list) == nullptr) std::cout << "nullptr" <<  std::endl;
        start += node_align_size;
        alloc_node* tail = new_span->node_list;
        while(start < end){
            next_node(tail) = start;
            start += node_align_size;
            tail = next_node(tail);
        }
        next_node(tail) = nullptr;
        
        new_span->obj_size = node_align_size;
        return new_span;
    }
public:
    //from alloc_page or center_span gain one span
    span* get_one_span(size_t node_align_size,alloc_span_list& slist){
        assert(node_align_size > 0);
        span* begin = slist.begin();
        while(begin != slist.end()){
            if(begin->node_list != nullptr){
                return begin;
            }
            begin = begin->next;
        }
        slist.unlock();
        //from alloc_page gain one span
        span* new_span = nullptr;
        {
            std::cout << "sys_alloc_span" << std::endl;
            new_span = sys_alloc_span(node_align_size);
            new_span->is_use = true;
            size_t alloc_bytes = new_span->page_n << PAGE_SHIFT;
            id_span_map[new_span->pageId] = new_span;
            slist.push_front(new_span);
            std::cout << "sys_alloc_span end" << std::endl;
        }
        slist.lock();
        
        return new_span;
    }
    //gain alloc_size node to thread
    size_t alloc_node_range(alloc_node*& start,alloc_node*& end,size_t node_align_size,int alloc_size){
        assert(alloc_size > 0 && node_align_size > 0);
        start = end = nullptr;
        //find free one span
        alloc_span_list& list = center_span[common::SizeClass::Index(node_align_size)];
        //lock
        list.lock();
        std::cout << "get_one_span" << std::endl;
        span* span = get_one_span(node_align_size,list);
        start = span->node_list;
        end = start;
        size_t used_alloc_size = 0;
        while(used_alloc_size < alloc_size-1 && next_node(end) != nullptr){
            end = next_node(end);
            used_alloc_size++;
        }
        span->node_list = next_node(end);
        next_node(end) = nullptr;
        span->use_count += used_alloc_size;
        std::cout << "get_one_span end : " << used_alloc_size<< std::endl;
        list.unlock();
        return used_alloc_size;
    }
private:
    alloc_center(const alloc_center& ) = delete;
    alloc_center(){}
    ~alloc_center(){
        for(auto it=id_span_map.begin();it != id_span_map.end(); it++){
            span* span = it->second;
            std::cout << "alloc addr : " << (PAGE_ID)(span->pageId << PAGE_SHIFT)  << std::endl;
            void* ptr = (void*)(span->pageId << PAGE_SHIFT);
            free(ptr);
            delete span;
            span = nullptr;
        }
        id_span_map.clear();
    }
private: 
    alloc_span_list center_span[ALLOC_NODE_LIST_MAX];
    std::unordered_map<unsigned long long,span*> id_span_map;
};

}
#endif