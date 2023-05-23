#pragma once

#include "Common.h"

class ThreadCache
{
public:
	//������ͷ��ڴ����
	void* Allocate(size_t size);//��Thread Cache������size��Bytes�Ŀռ�
	void DeAllocate(void* ptr, size_t size);

	void* FetchFromCentralCache(size_t index, size_t size);//��CentralCache������ռ�
	void ListTooLong(FreeList& list, size_t size);//�ͷŶ������������ʱ���黹�ڴ浽CentralCache
private:
	FreeList _freeLists[NFREELIST];//ThreadCache����������
};

//TLS: thread local storage
static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;