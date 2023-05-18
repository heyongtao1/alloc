#ifndef _ALLOC_SPAN_LIST_H
#define _ALLOC_SPAN_LIST_H
#include "alloc_common.h"
#include <mutex>
namespace alloc{
struct span{
	span*   prev = nullptr;//双向链表结构
	span*   next = nullptr;

	PAGE_ID pageId = 0;//大块内存起始页的id
	size_t  page_n = 0;//页的数量
	size_t  obj_size = 0;//记录Span中所切的小块内存的大小
	bool    is_use = false;//标识这块span是否正在被使用
	size_t  use_count = 0;//当前Span已经被分配给Thread Cache的内存数量
	alloc_node*  node_list = nullptr;
};

class alloc_span_list{
public:
    alloc_span_list():size(0),span_list(nullptr){
        span_list = new span();
        span_list->next = span_list;
        span_list->prev = span_list;
    }
    ~alloc_span_list(){}

private:
    void push_span(span* pos,span* new_span){
        assert(new_span && pos);
        span* pre = pos->prev;
        pre->next = new_span;
        pos->prev = new_span;
        new_span->next = pos;
        new_span->prev = pre;
        size++;
    }
    void pop_span(span* erase_span){
        assert(size > 0 && erase_span && erase_span != span_list);
        erase_span->prev->next = erase_span->next;
        erase_span->next->prev = erase_span->prev->next;
        size--;
    }
public:
    void push_front(span* span){
        push_span(span_list->next,span);
    }
    span* pop_front(){
        span* front = begin();
        pop_span(front);
        return front;
    }
    span* begin(){
        return span_list->next;
    }
    span* end(){
        return span_list;
    }
    bool empty(){
        return span_list->next == span_list;
    }
    size_t length(){
        return size;
    }
public:
    void lock(){
        mtx.lock();
    }
    void unlock(){
        mtx.unlock();
    }
private:
    span*  span_list;
    size_t size;
	std::mutex mtx;//对每一个哈希桶都加一个锁以保证线程安全
};
};

#endif 