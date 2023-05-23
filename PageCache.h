#pragma once

#include "Common.h"
#include "ObjectPool.h"
#include "PageMap.h"

class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}

	//�����Ӷ���span*��ӳ��
	Span* MapObjectToSpan(void* obj);

	//��ȡһ��kҳ��PageCache
	Span* NewPage(size_t k);

	//��CentralCache�黹��span���ӵ�PageCache��Ӧ��Ͱ�У���������ǰ��ҳ�п��У��ϲ�֮
	void RealeaseSpanToPageCache(Span* span);

	//��CentralCache��ͬ���ǣ����˴�����Ͱ������Ƶ���ļ�����������PageCache����������һ����
	std::mutex _pageMtx;


private:
	PageCache()
	{}
	PageCache(const PageCache&) = delete;

	//���嶨����span�ڴ��������ʹ��new
	ObjectPool<Span> _spanPool;
	
	//std::unordered_map<PAGE_ID, Span*> _idSpanMap;
	TCMalloc_PageMap1<32 - PAGE_SHIFT> _idSpanMap;

	static PageCache _sInst;
	SpanList _spanlists[NPAGES];
};