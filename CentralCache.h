#pragma once

#include "Common.h"

class CentralCache
{
public:
	static CentralCache* GetInstance()
	{
		return &_sInst;
	}

	//��ȡһ���ǿյ�Span
	Span* GetOneSpan(SpanList& list, size_t byte_size);

	//��CentralCache��ȡһ���������ڴ�����ThreadCache
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);

	//��һ�������Ķ����ͷŵ�span���
	void RealeaseListToSpan(void* start, size_t size);

private:
	SpanList _spanLists[NFREELIST];//CentralCache��Span�ķָʽ��ThreadCache��ͬ
private:
	CentralCache()
	{}
	CentralCache(const CentralCache&) = delete;//������
	static CentralCache _sInst;//����ģʽ
};