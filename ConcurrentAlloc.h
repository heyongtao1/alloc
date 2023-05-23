#pragma once

#include "Common.h"
#include "ThreadCache.h"
#include "PageCache.h"
#include "ObjectPool.h"

static void* ConcurrentAlloc(size_t size)
{
	//��������ڴ����256KB����ֱ������CentralCacheֱ����Page Cache�����ڴ�
	if (size > MAX_BYTE)
	{
		//�ȼ���Ҫ������ڴ�������Ĵ�С
		size_t alignSize = SizeClass::RoundUp(size);//����alignSize��size�Ķ��������϶���
		size_t kPage = alignSize >> PAGE_SHIFT;
		//��PageCacheȥ�����ڴ�
		PageCache::GetInstance()->_pageMtx.lock();
		Span* span = PageCache::GetInstance()->NewPage(kPage);
		span->_isUse = true;
		span->_objSize = size;
		PageCache::GetInstance()->_pageMtx.unlock();
		void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
		return ptr;
	}
	
	else
	{
		//ÿ���߳�ͨ��TLS���Դ����Լ�˽�е�ThreadCache����
		if (pTLSThreadCache == nullptr)
		{
			//��̬��Ա����֤ȫ��ֻ��һ������
			static ObjectPool<ThreadCache> tcPool;
			//pTLSThreadCache = new ThreadCache;
			pTLSThreadCache = tcPool.New();
		}
		return pTLSThreadCache->Allocate(size);
	}
}

static void ConcurrentFree(void* ptr)
{
	Span* span = PageCache::GetInstance()->MapObjectToSpan(ptr);
	size_t size = span->_objSize;
	//���ͷŵ��ڴ����256KB���黹��Page Cache
	if (size > MAX_BYTE)
	{
		//�ҵ�ptr��Ӧ���ǿ�span
		PageCache::GetInstance()->_pageMtx.lock();
		PageCache::GetInstance()->RealeaseSpanToPageCache(span);
		PageCache::GetInstance()->_pageMtx.unlock();
	}
	else
	{
		assert(pTLSThreadCache);
		pTLSThreadCache->DeAllocate(ptr, size);
	}
}