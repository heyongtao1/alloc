#include "PageCache.h"

PageCache PageCache::_sInst;

Span* PageCache::NewPage(size_t k)
{
	assert(k);

	if (k > NPAGES - 1)
	{
		void* ptr = SystemAlloc(k);
		//Span* span = new Span;
		Span* span = _spanPool.New();

		span->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
		span->_n = k;

		//����ӳ��,�������
		//_idSpanMap[span->_pageId] = span;
		_idSpanMap.Set(span->_pageId, span);
		return span;
	}

	//����ǰkҳPage���п��е�Span������֮
	if (!_spanlists[k].Empty())
	{
		//�ڷ���span֮ǰ����Ҫ�Ƚ���ӳ��
		Span* kSpan = _spanlists[k].PopFront();
		//error:���ڷ��ؽ���ʹ�õ�span������ÿҳ����Ҫ����ӳ�䣬������Ҳ������������޷�����
		/*_idSpanMap[kSpan->_pageId] = kSpan;
		_idSpanMap[kSpan->_pageId + kSpan->_n - 1] = kSpan;*/
		for (size_t i = 0; i < kSpan->_n; i++)
		{
			//kSpan��ÿһҳid��Ҫ��span����ӳ�䣬������������ʱ�ŷ������
			//_idSpanMap[kSpan->_pageId + i] = kSpan;
			_idSpanMap.Set(kSpan->_pageId + i, kSpan);
		}

		return kSpan;
		//return _spanlists[k].PopFront();
	}
	//����ǰkҳPage���޿��е�Span�������Ѱ�Ҹ���ҳ��Pageȥ�зֳ���Ҫ��Span��
	for (size_t i = k + 1; i < NPAGES; i++)
	{
		if (!_spanlists[i].Empty())//��iҳSpan�ǿգ������з�
		{
			//Span* kSpan = new Span;
			Span* kSpan = _spanPool.New();

			Span* nSpan = _spanlists[i].PopFront();

			//��nPage�зֳ�kҳSpan��n - kҳSpan
			kSpan->_pageId = nSpan->_pageId;
			kSpan->_n = k;

			nSpan->_pageId += k;//ϸ�ڴ���+=kд��-=k����Խ�����
			nSpan->_n -= k;
			//���зֺ��nPage�ҽӵ�n-kҳ��Span��
			_spanlists[i - k].PushFront(nSpan);

			//nSpan����βid��Ҫ��span����ӳ�䣬���ں����ϲ�Сpageʱ�Ĳ�ѯ
			/*_idSpanMap[nSpan->_pageId] = nSpan;
			_idSpanMap[nSpan->_pageId + nSpan->_n - 1] = nSpan;*/
			_idSpanMap.Set(nSpan->_pageId, nSpan);
			_idSpanMap.Set(nSpan->_pageId + nSpan->_n - 1, nSpan);
			//kSpan������id��Span��ӳ���ϵ������Central Cache����С���ڴ�ʱ���Ҷ�Ӧ��span
			for (size_t i = 0; i < kSpan->_n; i++)
			{
				//kSpan��ÿһҳid��Ҫ��span����ӳ�䣬������������ʱ�ŷ������
				//_idSpanMap[kSpan->_pageId + i] = kSpan;
				_idSpanMap.Set(kSpan->_pageId + i, kSpan);
			}

			//�����кõ�kSpan
			return kSpan;
		}
	}
	//�ߵ��˴�˵��PageCache��û�п���ʹ�õ�Span��
	//����Ҫ��ϵͳ���ѣ�����һ����ڴ�(128ҳ)�Ա���
	//Span* bigSpan = new Span;
	Span* bigSpan = _spanPool.New();

	void* ptr = SystemAlloc(NPAGES - 1);//����д�����Ȩ�޳�ͻ��ԭ��������kҳ�ڴ浼���þ���Խ�����
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigSpan->_n = NPAGES - 1;
	//��bigSpan�ҽӵ�NPAGES - 1��Ͱ��
	_spanlists[bigSpan->_n].PushFront(bigSpan);
	//������߼���Ϊ�з֣�����bigSpan�з�Ϊ��С������Span
	//��ֱ�ӵݹ鸴���������뼴��
	return NewPage(k);
}

//�����з�spanΪС���ڴ�󣬽��зֺõ�ÿһ���ڴ��id��ӳ�䵽�˱��зֵ�span��
//��ˣ�ͨ���ú������԰������������ڴ�黹����ԭ�����ڵ�span��
Span* PageCache::MapObjectToSpan(void* ptr)
{
	//ʹ�û������������ڵ��øú���ʱ������ɶ�д��ͻ�����������
	PAGE_ID id = (PAGE_ID)ptr >> PAGE_SHIFT;
	Span* ret = (Span*)_idSpanMap.Get(id);
	assert(ret != nullptr);
	return ret;
	//PAGE_ID id = (PAGE_ID)ptr >> PAGE_SHIFT;
	////������ֹ�����̰߳�ȫ����
	//std::unique_lock<std::mutex> lock(_pageMtx);
	//if (_idSpanMap.find(id) != _idSpanMap.end())
	//{
	//	return _idSpanMap[id];
	//}
	//else
	//{
	//	assert(false);
	//	return nullptr;
	//}
}

void PageCache::RealeaseSpanToPageCache(Span* span)
{
	//���ͷŵ�span����256KB����span��ҳ������NPAGES - 1����ֱ�ӽ��ڴ��ͷŵ���
	if (span->_n > NPAGES - 1)
	{
		void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
		SystemFree(ptr);
		//delete span;
		_spanPool.Delete(span);
		return;
	}
	//��spanǰ���ҳ���л��գ������ڴ���Ƭ����
	//1.����spanǰ��ҳ
	while (1)
	{
		PAGE_ID prevId = span->_pageId - 1;
		//auto ret = _idSpanMap.find(prevId);
		Span* ret = (Span*)_idSpanMap.Get(prevId);

		//���spanǰû��ҳ����ô��������ǰ����
		//if (ret == _idSpanMap.end())
		if(ret == nullptr)
		{
			break;
		}

		//Span* prevSpan = ret->second;
		Span* prevSpan = ret;
		//���prevSpan���ڱ�ʹ�ã���ô��������ǰ����
		if (prevSpan->_isUse == true)
		{
			break;
		}

		//����ϲ����spanҳ������NPAGES - 1������������ǰ����
		if (prevSpan->_n + span->_n > NPAGES - 1)
		{
			break;
		}

		//�ϲ�spanǰ��ҳprevSpan
		span->_pageId = prevSpan->_pageId;
		span->_n += prevSpan->_n;
		//��prevSpan�������ҽӵ�Ͱ��ȥ��
		_spanlists[prevSpan->_n].Erase(prevSpan);
		//delete prevSpan;
		_spanPool.Delete(prevSpan);
	}
	//�����span���ҳ
	while (1)
	{
		PAGE_ID nextId = span->_pageId + span->_n;
		//auto ret = _idSpanMap.find(nextId);
		Span* ret = (Span*)_idSpanMap.Get(nextId);

		//�������ҳ�����ڣ���������������
		//if (ret == _idSpanMap.end())
		if(ret == nullptr)
		{
			break;
		}

		//Span* nextSpan = ret->second;
		Span* nextSpan = ret;
		//�������ҳ���ڱ�ʹ�ã���������������
		if (nextSpan->_isUse == true)
		{
			break;
		}

		//�������ҳ�ϲ���ҳ������NAPGES - 1��������������
		if (nextSpan->_n + span->_n > NPAGES - 1)
		{
			break;
		}
		
		//�ϲ������ҳ
		span->_n += nextSpan->_n;
		//��nextSpan�������ڵ�Ͱ��ȡ����
		_spanlists[nextSpan->_n].Erase(nextSpan);
		//delete nextSpan;
		_spanPool.Delete(nextSpan);
	}
	//��span�ҽӵ�������Ӧ��Page��
	_spanlists[span->_n].PushFront(span);
	//��ʶspanΪδ��ʹ�õ�״̬
	span->_isUse = false;
	//����span����βPAGE_ID��span*��ӳ��
	/*_idSpanMap[span->_pageId] = span;
	_idSpanMap[span->_pageId + span->_n - 1] = span;*/
	_idSpanMap.Set(span->_pageId, span);
	_idSpanMap.Set(span->_pageId + span->_n - 1, span);
}
