#include "CentralCache.h"
#include "PageCache.h"

//��ȡһ���ǿյ�span
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	//�Ȳ��ҵ�ǰspanlist���Ƿ��зǿյ�span�����򷵻�֮
	Span* it = list.Begin();
	while (it != list.End())//�ڶ��˵������513���ڴ�ʱ��������ѭ����list�����һ��span ��prev��nextָ����ͬһ��ط�
	{
		if (it->_freeList != nullptr)
			return it;
		else
			it = it->_next;
	}

	//�ߵ��˴�˵���ڵ�ǰlist�в�δ�ҵ��ǿյ�span��������Ҫ��PageCache������
	//������PageCache����span�����������CentralCache�ٽ���Դ����˽�Ͱ�����
	list._mtx.unlock();

	//��PageCache�������������span����
	PageCache::GetInstance()->_pageMtx.lock();
	Span* span = PageCache::GetInstance()->NewPage(SizeClass::NumMovePage(size));
	span->_isUse = true;
	span->_objSize = size;//��¼��span�ж���Ĵ�С
	PageCache::GetInstance()->_pageMtx.unlock();

	//������õ�span���󻮷�
	char* start = (char*)(span->_pageId << PAGE_SHIFT);
	size_t bytes = span->_n << PAGE_SHIFT;
	char* end = start + bytes;
	//1.�Ƚ�һС���ڴ���뵽span��list��
	span->_freeList = start;
	start += size;
	//2.��ʣ���ڴ�β�嵽span�У�β�屣֤���λ�ò���
	void* tail = span->_freeList;
	//size_t i = 1;//��ʱspan���Ѿ���һ���ڴ�
	while (start < end)
	{
		//i++;//���㻮�ֵ��ڴ����
		NextObj(tail) = start;
		start += size;
		tail = NextObj(tail);
	}
	//ϸ�ڴ���β���Ҫ��tail��next�ÿ�,����Խ�����
	NextObj(tail) = nullptr;
	//3.��ʱ�������ʵ����span�е��ڴ���������ֻ����ÿһ���ڴ��ж���ʶ����һ����ڴ�λ��

	//�����span�����зֺ�ָ�Ͱ���Ա�֤�̰߳�ȫ
	list._mtx.lock();
	list.PushFront(span);//��span������뵽list��
	return span;
}

//��CentralCache��ȡһ���������ڴ�����ThreadCache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
	//����ThreadCache����ȡ���ڴ�������ĸ���ϣͰ��
	size_t index = SizeClass::Index(size);
	//������������ȡ���̰߳�ȫ(Ͱ��)
	_spanLists[index]._mtx.lock();

	//��spanList�л�ȡһ��span
	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span);
	assert(span->_freeList);

	start = span->_freeList;
	end = start;

	size_t i = 0;
	size_t actualNum = 1;
	while (i < batchNum - 1 && NextObj(end) != nullptr)
	{
		end = NextObj(end);
		i++;
		actualNum++;
	}

	span->_useCount += actualNum;

	span->_freeList = NextObj(end);
	NextObj(end) = nullptr;

	_spanLists[index]._mtx.unlock();

	return actualNum;
}

CentralCache CentralCache::_sInst;

void CentralCache::RealeaseListToSpan(void* start, size_t size)
{
	size_t index = SizeClass::Index(size);//��������
	_spanLists[index]._mtx.lock();//Ͱ����֤�ͷ�ʱ���̰߳�ȫ

	while (start)
	{
		void* next = NextObj(start);
		
		//Ѱ��start��ӳ����ǿ�span
		Span* span = PageCache::GetInstance()->MapObjectToSpan(start);
		//��startͷ���span��
		NextObj(start) = span->_freeList;
		span->_freeList = start;
		//�黹һ���кõ��ڴ��usecount--
		span->_useCount--;
		//span��useCount��Ϊ0��˵����һ��span�Ѿ����У�����黹��PageCache
		if (span->_useCount == 0)
		{
			//�˴������ڹ黹span��PageCacheʱδ��span��_spanLists��ɾ������������ѭ��
			_spanLists[index].Erase(span);
			span->_prev = nullptr;
			span->_next = nullptr;
			span->_freeList = nullptr;
			//��ʱ��span�黹��PageCache�Ĺ����У��̲߳��ٷ���CentralCache���ٽ���Դ
			//��˿��Խ�Ͱ��������������̷߳��ʲ������ڴ�
			//���ǹ黹��������Ҫ��֤Page Cache���̰߳�ȫ����Page Cache�����
			_spanLists[index]._mtx.unlock();
			PageCache::GetInstance()->_pageMtx.lock();
			PageCache::GetInstance()->RealeaseSpanToPageCache(span);
			PageCache::GetInstance()->_pageMtx.unlock();
			//��span�黹��Ҫ��Ͱ���ָ�����ֹ�����̰߳�ȫ����
			_spanLists[index]._mtx.lock();
		}
		start = next;
	}
	_spanLists[index]._mtx.unlock();
}
