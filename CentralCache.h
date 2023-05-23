#pragma once

#include "Common.h"

class CentralCache
{
public:
	static CentralCache* GetInstance()
	{
		return &_sInst;
	}

	//获取一个非空的Span
	Span* GetOneSpan(SpanList& list, size_t byte_size);

	//从CentralCache获取一定数量的内存对象给ThreadCache
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);

	//将一定数量的对象释放到span跨度
	void RealeaseListToSpan(void* start, size_t size);

private:
	SpanList _spanLists[NFREELIST];//CentralCache的Span的分割方式与ThreadCache相同
private:
	CentralCache()
	{}
	CentralCache(const CentralCache&) = delete;//防拷贝
	static CentralCache _sInst;//单例模式
};