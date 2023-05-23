#define _CRT_SECURE_NO_WARNINGS 1

#include "ObjectPool.h"
#include "ConcurrentAlloc.h"



void Alloc1()
{
	for (size_t i = 0; i < 5; i++)
	{
		ConcurrentAlloc(3);
		cout << std::this_thread::get_id() << ":" << pTLSThreadCache << endl;
	}
}

void Alloc2()
{
	for (size_t i = 0; i < 5; i++)
	{
		ConcurrentAlloc(6);
		cout << std::this_thread::get_id() << ":" << pTLSThreadCache << endl;
	}
}

void TLSTest()
{
	std::thread t1(Alloc1);
	t1.join();

	std::thread t2(Alloc2);
	t2.join();
}

void TestConcurrentAlloc1()
{
	void* p1 = ConcurrentAlloc(6);
	void* p2 = ConcurrentAlloc(8);
	void* p3 = ConcurrentAlloc(7);
	void* p4 = ConcurrentAlloc(1);
	void* p5 = ConcurrentAlloc(2);
	void* p6 = ConcurrentAlloc(3);
	cout << p1 << endl;
	cout << p2 << endl;
	cout << p3 << endl;
	cout << p4 << endl;
	cout << p5 << endl;
	cout << p6 << endl;
}

void TestConcurrentAlloc2()
{
	for (size_t i = 0; i < 1024; i++)
	{
		void* p1 = ConcurrentAlloc(8);
		cout << i << ":" << p1 << endl;
	}
	void* p2 = ConcurrentAlloc(8);
	cout << p2 << endl;
}

void TestConcurrentFree()
{
	void* p1 = ConcurrentAlloc(6);
	void* p2 = ConcurrentAlloc(8);
	void* p3 = ConcurrentAlloc(7);
	void* p4 = ConcurrentAlloc(1);
	void* p5 = ConcurrentAlloc(2);
	void* p6 = ConcurrentAlloc(3);
	void* p7 = ConcurrentAlloc(3);
	void* p8 = ConcurrentAlloc(3);
	cout << p1 << endl;
	cout << p2 << endl;
	cout << p3 << endl;
	cout << p4 << endl;
	cout << p5 << endl;
	cout << p6 << endl;
	ConcurrentFree(p1);
	ConcurrentFree(p2);
	ConcurrentFree(p3);
	ConcurrentFree(p4);
	ConcurrentFree(p5);
	ConcurrentFree(p6);
	ConcurrentFree(p7);
	ConcurrentFree(p8);
}


void ConcurrentFree1()
{
	std::vector<void*> v;
	for (size_t i = 0; i < 5; i++)
	{
		void* p1 = ConcurrentAlloc(3);
		v.push_back(p1);
		cout << std::this_thread::get_id() << ":" << pTLSThreadCache << endl;
	}
	for (auto& e : v)
	{
		ConcurrentFree(e);
	}
}

void ConcurrentFree2()
{
	std::vector<void*> v;

	for (size_t i = 0; i < 5; i++)
	{
		void* p2 = ConcurrentAlloc(6);
		v.push_back(p2);
		cout << std::this_thread::get_id() << ":" << pTLSThreadCache << endl;
	}
	for (auto& e : v)
	{
		ConcurrentFree(e);
	}
}

void MultiThreadConCurrentFreeTest()
{
	std::thread t1(ConcurrentFree1);
	std::thread t2(ConcurrentFree2);

	t1.join();
	t2.join();
}

void BigAlloc()
{
	void* p1 = ConcurrentAlloc(257 * 1024);
	ConcurrentFree(p1);

	void* p2 = ConcurrentAlloc(129 * 8 * 1024);
	ConcurrentFree(p2);
}
//
//int main()
//{
//	//TestObjectPool();
//	//TLSTest();
//	//TestConcurrentAlloc1();
//	//TestConcurrentAlloc2();
//	//TestConcurrentFree();
//	//MultiThreadConCurrentFreeTest();
//	//BigAlloc();
//	return 0;
//}