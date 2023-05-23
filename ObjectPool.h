#pragma once

#include "Common.h"

template <class T>//һ������T��С���ڴ�ռ�
class ObjectPool
{
public:
	T* New()
	{
		T* obj;
		if (_freeList != nullptr)
		{
			//ͷɾ
			obj = (T*)_freeList;
			_freeList = *(void**)_freeList;
		}
		else
		{
			if (_remainedBytes < sizeof(T))//��ǰ�ڴ����û�����Է�����ڴ棬��Ҫ����
			{
				_remainedBytes = 8 * 1024;	
				//_memory = (char*)malloc(_remainedBytes);//���붨����8Kb�����ڴ�ռ�
				_memory = (char*)SystemAlloc(_remainedBytes >> PAGE_SHIFT);//���붨����8Kb�����ڴ�ռ�

			}
			//��֤һ�η���Ŀռ乻����µ�ǰƽ̨��ָ��
			size_t unity = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			obj = (T*)_memory;
			_memory += unity;
			_remainedBytes -= unity;
		}
		//��λnew��ʽ����T���͹��캯��
		new(obj)T;
		return obj;
	}
	void Delete(T* obj)//��obj����ڴ����ӵ�_freeList��
	{
		//��ʽ����obj�������������,����ռ�
		obj->~T();
		//��obj�ڴ��ͷ��
		*(void**)obj = _freeList;
		_freeList = obj;
	}
private:
	char* _memory = nullptr;//�洢һ������һ�����ڴ�,char*���ͱ��ڷ����ڴ�
	void* _freeList = nullptr;//���ͷŵ��ڴ��������
	size_t _remainedBytes;//_memory��ʣ����ڴ�ռ�
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
//	// �����ͷŵ��ִ�
//	const size_t Rounds = 5;
//	// ÿ�������ͷŶ��ٴ�
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