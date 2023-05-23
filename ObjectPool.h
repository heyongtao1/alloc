#pragma once

#include "Common.h"

template <class T>//一次申请T大小的内存空间
class ObjectPool
{
public:
	T* New()
	{
		T* obj;
		if (_freeList != nullptr)
		{
			//头删
			obj = (T*)_freeList;
			_freeList = *(void**)_freeList;
		}
		else
		{
			if (_remainedBytes < sizeof(T))//当前内存池中没有足以分配的内存，需要申请
			{
				_remainedBytes = 8 * 1024;	
				//_memory = (char*)malloc(_remainedBytes);//申请定长（8Kb）的内存空间
				_memory = (char*)SystemAlloc(_remainedBytes >> PAGE_SHIFT);//申请定长（8Kb）的内存空间

			}
			//保证一次分配的空间够存放下当前平台的指针
			size_t unity = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			obj = (T*)_memory;
			_memory += unity;
			_remainedBytes -= unity;
		}
		//定位new显式调用T类型构造函数
		new(obj)T;
		return obj;
	}
	void Delete(T* obj)//将obj这块内存链接到_freeList中
	{
		//显式调用obj对象的析构函数,清理空间
		obj->~T();
		//将obj内存块头插
		*(void**)obj = _freeList;
		_freeList = obj;
	}
private:
	char* _memory = nullptr;//存储一次申请一大块的内存,char*类型便于分配内存
	void* _freeList = nullptr;//将释放的内存回收链接
	size_t _remainedBytes;//_memory中剩余的内存空间
};

//struct TreeNode
//{
//	int _val;
//	TreeNode* _left;
//	TreeNode* _right;
//	TreeNode()
//		:_val(0)
//		, _left(nullptr)
//		, _right(nullptr)
//	{}
//};
//void TestObjectPool()
//{
//	// 申请释放的轮次
//	const size_t Rounds = 5;
//	// 每轮申请释放多少次
//	const size_t N = 1000000;
//	size_t begin1 = clock();
//	std::vector<TreeNode*> v1;
//	v1.reserve(N);
//	for (size_t j = 0; j < Rounds; ++j)
//	{
//		for (int i = 0; i < N; ++i)
//		{
//			v1.push_back(new TreeNode);
//		}
//		for (int i = 0; i < N; ++i)
//		{
//			delete v1[i];
//		}
//		v1.clear();
//	}
//	size_t end1 = clock();
//	ObjectPool<TreeNode> TNPool;
//	size_t begin2 = clock();
//	std::vector<TreeNode*> v2;
//	v2.reserve(N);
//	for (size_t j = 0; j < Rounds; ++j)
//	{
//		for (int i = 0; i < N; ++i)
//		{
//			v2.push_back(TNPool.New());
//		}
//		for (int i = 0; i < 100000; ++i)
//		{
//			TNPool.Delete(v2[i]);
//		}
//		v2.clear();
//	}
//	size_t end2 = clock();
//	cout << "new cost time:" << end1 - begin1 << endl;
//	cout << "object pool cost time:" << end2 - begin2 << endl;
//}