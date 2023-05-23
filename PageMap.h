#pragma once

#include "Common.h"

//���������
template <size_t BITS>
class TCMalloc_PageMap1
{
private:
	static const int LENGTH = 1 << BITS;
	void** _array;

public:
	typedef uintptr_t Number;//�洢ָ���һ���޷�����������
	explicit TCMalloc_PageMap1()//һ�ν���������ռ俪��
	{
		//�������鿪�ٿռ�����Ĵ�С
		size_t size = sizeof(void*) << BITS;
		size_t alignSize = SizeClass::_RoundUp(size, 1 << PAGE_SHIFT);
		//����Ҫ���ٵĿռ���2M���Ѿ��ܴ��ˣ���ֱ���������
		_array = (void**)SystemAlloc(alignSize >> PAGE_SHIFT);
		memset(_array, 0, size);
	}
	void Set(Number key, void* v)//key�ķ�Χ��[0, 2^BITS - 1],_pageId
	{
		_array[key] = v;
	}
	void* Get(Number key) const
	{
		if ((key >> BITS) > 0)//��key������Χ��δ�����ã��򷵻ؿ�
		{
			return nullptr;
		}
		return _array[key];
	}
};