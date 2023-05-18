#ifndef _ALLOC_COMMON_H_
#define _ALLOC_COMMON_H_

#include <sys/types.h>
#include <assert.h>
#include <malloc.h>
#define next_node(node)     *(void**)node
#define MAX_BYTE            256 * 1024
#define PAGE_SHIFT          13
#define ALLOC_NODE_LIST_MAX 208

typedef void alloc_node;
typedef unsigned long long PAGE_ID;

namespace common{

static void* sys_alloc(size_t size){
	std::cout << "size = " << size << std::endl;
	return malloc(size);
}

//管理对齐和映射关系
class SizeClass
{
public:
	// 整体控制在最多10%左右的内碎片浪费
	// [1,128] 8byte对齐       freelist[0,16)
	// [128+1,1024] 16byte对齐   freelist[16,72)
	// [1024+1,8*1024] 128byte对齐   freelist[72,128)
	// [8*1024+1,64*1024] 1024byte对齐     freelist[128,184)
	// [64*1024+1,256*1024] 8*1024byte对齐   freelist[184,208)
    static inline size_t alignSize(size_t indx)
    {
        assert(indx >= 0 && indx < 208);
        int group_array[4] = { 16,56,56,56 };//打表，每个区间有多少个链
        if(indx >= 0 && indx < 16){
            return 8*(indx + 1);
        }
        else if(indx >= 16 && indx < 72){
            return 16*(indx - 16) + 128+1;
        }
        else if(indx >= 72 && indx < 128){
            return 128*(indx - 72) + 1024+1;
        }else if(indx >= 128 && indx < 184){
            return 1024*(indx - 128) + 8*1024+1;
        }else if(indx >= 184 && indx < 208){
            return 8*1024*(indx - 184) + 64*1024+1;
        }
        return 0;
    }

	static inline size_t _RoundUp(size_t bytes, size_t alignNum)
	{
		return (bytes + (alignNum - 1) & ~(alignNum - 1));//移位计算提高效率
	}

	//对齐大小的计算
	static inline size_t RoundUp(size_t size)//给定申请的内存大小，计算其对齐数，对齐规则如上
	{
		if (size <= 128)
		{
			return _RoundUp(size, 8);//按8byte对齐
		}
		else if(size <= 1024)
		{ 
			return _RoundUp(size, 16);//按16byte对齐
		}
		else if (size <= 8 * 1024)
		{
			return _RoundUp(size, 128);//按128byte对齐
		}
		else if(size <= 64 * 1024)
		{
			return _RoundUp(size, 1024);//按1024byte对齐
		}
		else if (size <= 256 * 1024)
		{
			return _RoundUp(size, 8 * 1024);//按8*1024byte对齐
		}
		else//大于256KB
		{
			//assert(false);
			return _RoundUp(size, 1 << PAGE_SHIFT);
		}
	}
	
	//计算在对应链桶中的下标
	static inline size_t _Index(size_t bytes, size_t align_shift)
	{
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	//计算映射到哪一个自由链表桶
	static inline size_t Index(size_t size)
	{
		assert(size <= MAX_BYTE);
		static int group_array[4] = { 16,56,56,56 };//打表，每个区间有多少个链
		if (size <= 128)
		{
			return _Index(size, 3);
		}
		else if (size <= 1024)
		{
			return _Index(size - 128, 4) + group_array[0];
		}
		else if(size <= 8 * 1024)
		{
			return _Index(size - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (size <= 64 * 1024)
		{
			return _Index(size - 8 * 1024, 10) + group_array[2] + group_array[1] + group_array[0];
		}
		else if (size <= 256 * 1024)
		{
			return _Index(size - 64 * 1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
		}
		else
		{
			assert(false);
		}
	}

	//计算向CentralCache一次获取批次的数量
	static size_t NumMoveSize(size_t size)
	{
		assert(size > 0);//获取的内存对象总大小必须大于0
		size_t num = MAX_BYTE / size;//计算合理的内存对象数量
		//根据批次计算获取的数量会在[1,32768]，范围过大，
		//因此控制获取的对象数量范围在区间[2, 512],较为合理
		//小对象获取的批次多，大对象获取的批次少
		if (num < 2)
			num = 2;
		if (num > 512)
			num = 512;
		return num;
	}

	//计算一次向PageCache申请多少页
	//size: 单个对象从8byte - 256kb
	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size);//当前size所需批次数量
		size_t nPage = num * size;
		nPage >>= PAGE_SHIFT;
		if (nPage == 0)
			nPage = 1;
		return nPage;
	}
};

};

#endif
