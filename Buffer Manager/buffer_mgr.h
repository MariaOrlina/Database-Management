#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

// Include return codes and methods for logging errors
#include "dberror.h"

// Include bool DT
#include "dt.h"

// Replacement Strategies
typedef enum ReplacementStrategy {
	RS_FIFO = 0,
	RS_LRU = 1,
	RS_CLOCK = 2,
	RS_LFU = 3,
	RS_LRU_K = 4
} ReplacementStrategy;

// Data Types and Structures
typedef int PageNumber;
#define NO_PAGE -1

typedef struct BM_BufferPool {
	char *pageFile;
	int numPages;
	ReplacementStrategy strategy;
	void *mgmtData; // use this one to store the bookkeeping info your buffer
	// manager needs for a buffer pool
} BM_BufferPool;

typedef struct BM_PageHandle {
	PageNumber pageNum;
	char *data;
} BM_PageHandle;


// Doubly Linked List frame node consisting one page/frame of buffer pool
typedef struct pgFrmDLLNode{
  char *data;     // actual data pointed by the frame
  int pgFreq;     // LFU freq of the pg per client req
  int frmNum;     // # of the frame in frame list
  int fixCount;   // based on the pin/un-pin req fixCount of the pg 
  int dirtyFlag;  // dirtyFlag = 1 if dirty, dirtyFlag = 0 if not dirty
  int pgNum;      // the page number of pg in the pgFile
  int refBit;     // clock reference bit per node 
  struct pgFrmDLLNode *nxt;
  struct pgFrmDLLNode *prev;
    
}pgFrmDLLNode;

#define MAX_PGS 20000
#define MAX_FRMS 200
#define MAXIMUM_K_VAL 10

// A list of frames with a ptr to head node and tail node of type pgFrmDLLNode
typedef struct frmList{    
  pgFrmDLLNode *headNode;    // Frame list head node. It should add new node/updated node/recently used node to the head
  pgFrmDLLNode *tailNode;    // Frame list tail node. It should be the first/start for removal as per strategy
   
}frmList;

// BM_BufferPool->mgmtData will have following info
typedef struct bufferManagerInfo{
    frmList *pageFrames;      // a buffer pool pointer to the frm list
    int numOfFilledFrames;    // num of frms filled in the frm list 
    int countOfPinning;       // for the buffer manager, count of the total num of pinning done
    int numOfReadDone;        // on the buffer pool, count of reads done 
    int numOfWriteDone;       // on the buffer pool, count of writes done
    bool dirtyFlgs[MAX_FRMS]; // dirtyflags array of all the frms
    void *strategyData;
    int arrPgToFrm[MAX_PGS];  // array from pg number to frm number. size of array = size of the pageFile
    int kHistory[MAX_PGS][MAXIMUM_K_VAL]; 
    int arrPgToFreq[MAX_PGS]; // array mapping the page number to page freq. size of array = size of the pageFile
    int arrFrmToPg[MAX_FRMS]; // array from frm number to pg number. size of array = size of the frame list
    int fixedCountsOfFrames[MAX_FRMS]; // Of all the frames, fixed count  
}bufferManagerInfo;

// convenience macros
#define MAKE_POOL()					\
		((BM_BufferPool *) malloc (sizeof(BM_BufferPool)))

#define MAKE_PAGE_HANDLE()				\
		((BM_PageHandle *) malloc (sizeof(BM_PageHandle)))

// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		const int numPages, ReplacementStrategy strategy,
		void *stratData);
RC shutdownBufferPool(BM_BufferPool *const bm);
RC forceFlushPool(BM_BufferPool *const bm);

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page);
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page);
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page);
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
		const PageNumber pageNum);

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm);
bool *getDirtyFlags (BM_BufferPool *const bm);
int *getFixCounts (BM_BufferPool *const bm);
int getNumReadIO (BM_BufferPool *const bm);
int getNumWriteIO (BM_BufferPool *const bm);

#endif
