#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dberror.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"

#define initializeLkArray(arr,val,maxNum,dataType) memset(arr,val,maxNum*sizeof(dataType))
#define allocating(obj) malloc(sizeof(obj))
#define spaceAllocate() calloc(PAGE_SIZE, sizeof(SM_PageHandle))
#define freeData(det) free(det->pageFrames->headNode->data)
#define freeHeadNode(det) free(det->pageFrames->headNode)



// for creating a new node for page frame list
pgFrmDLLNode *newFrmNode(){
 pgFrmDLLNode *nd = allocating(pgFrmDLLNode);
 (*nd).frmNum = 0;    
 (*nd).fixCount = 0;
 (*nd).pgNum = NO_PAGE;    
 (*nd).dirtyFlag = 0;
 (*nd).refBit = 0;
 (*nd).data =  spaceAllocate();
 (*nd).prev = NULL;
 (*nd).nxt = NULL;   
 (*nd).pgFreq = 0;
 return nd;
}

pgFrmDLLNode *changingPointers(pgFrmDLLNode *updtNd){
 updtNd->prev->nxt = updtNd->nxt;
 updtNd->nxt->prev = updtNd->prev;
 return updtNd;
}

pgFrmDLLNode *setUpdtNdNext(pgFrmDLLNode *updtNd, pgFrmDLLNode *headNode){
updtNd->nxt = headNode;
return updtNd;
}

pgFrmDLLNode *setHeadNodePrev(pgFrmDLLNode *updtNd, pgFrmDLLNode *headNode){
headNode->prev = updtNd;
return headNode;
}

void updtFListPointers(frmList **fList, pgFrmDLLNode *updtNd){
 (*fList)->headNode = updtNd;
 (*fList)->headNode->prev = NULL;
 return;   
}

// finds the node with given pg num; first checked by the lookup arr & if available in memory, then to find the exact node
pgFrmDLLNode *findNdByPgNum(const PageNumber pgNum, frmList *list){
  if(list){
    pgFrmDLLNode *curr = list->headNode;
    for(int i=0; curr != NULL;curr = curr->nxt , i++){
      if(curr->pgNum == pgNum){
        return curr;
      }        
    }
  }
  return NULL;
}

// making the given node as the headNode of the list
void updtHeadNode(pgFrmDLLNode *updtNd, frmList **fList){
  if(updtNd && fList){
    pgFrmDLLNode *headNode = (*fList)->headNode;
    bool condition1 = (updtNd == (*fList)->tailNode);
    bool condition2 = (updtNd == (*fList)->headNode || headNode == NULL || updtNd == NULL);
    if(condition1){
      pgFrmDLLNode *tempNode = ((*fList)->tailNode)->prev;
      tempNode->nxt = NULL;
      (*fList)->tailNode = tempNode;
    }    
    else if(condition2){
      return;
    }
    else if(!condition1 && !condition2){
      updtNd = changingPointers(updtNd); 
    }
    
    updtNd = setUpdtNdNext(updtNd, headNode);
    headNode = setHeadNodePrev(updtNd,headNode);
    updtNd->prev = NULL;
    
    updtFListPointers(fList,updtNd);
  }
  return;
}

bufferManagerInfo *processingTailNodeNext(bufferManagerInfo *det){
 det->pageFrames->tailNode->nxt->prev = det->pageFrames->tailNode;
 return det;
}

bufferManagerInfo *processingTailNode(bufferManagerInfo *det){
 det->pageFrames->tailNode = det->pageFrames->tailNode->nxt;
 return det;
}

bufferManagerInfo *processingTailNodeFrmNum(bufferManagerInfo *det, int i){
 det->pageFrames->tailNode->frmNum = i;
 return det;
}

bufferManagerInfo *processing(bufferManagerInfo *det, const int numPages){
 int i=1;
  for(; i<numPages; i=i+1){
    det->pageFrames->tailNode->nxt = newFrmNode();
    det = processingTailNodeNext(det);
    det = processingTailNode(det);
    det = processingTailNodeFrmNum(det, i);
  }
  return det;
}
void setMgmtData(bufferManagerInfo *det, BM_BufferPool *const bm){
 bm->mgmtData = det;
}

void setStrategy(BM_BufferPool *const bm, ReplacementStrategy strategy){
bm->strategy = strategy;
}


BM_BufferPool *processingBM(bufferManagerInfo *det, BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy){
  bm->pageFile = (char*) pageFileName;
  bm->numPages = numPages ?  numPages : 0;
  setMgmtData(det, bm);
  setStrategy(bm, strategy);
  
}

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData)
{
  SM_FileHandle fileH;
  
     printf("reached here");
  if(numPages <= 0){
    return RC_INVALID_BUFFER_M;
  }       
  if (openPageFile ((char *)pageFileName, &fileH) != RC_OK){
    return RC_FILE_NOT_FOUND;
  }
    
  // Initializing the mgmtInfo data
  bufferManagerInfo *det = allocating(bufferManagerInfo);
  det->numOfReadDone = 0;
  det->numOfWriteDone = 0;
  det->numOfFilledFrames = 0;
  det->countOfPinning = 0;    
  det->strategyData = stratData;
    
  // Initializing the lookup arrays with values equal to 0
  initializeLkArray(det->arrFrmToPg,NO_PAGE,MAX_FRMS,int);
   printf("reached here");
  initializeLkArray(det->arrPgToFrm,NO_PAGE,MAX_PGS,int);
  initializeLkArray(det->dirtyFlgs,NO_PAGE,MAX_FRMS,bool);
  initializeLkArray(det->fixedCountsOfFrames,NO_PAGE,MAX_FRMS,int);
  initializeLkArray(det->arrPgToFreq,0,MAX_PGS,int);
    memset(det->kHistory, -1, sizeof(&(det->kHistory)));
  // Creating the empty frames linked list
  det->pageFrames = allocating(frmList);    
  det->pageFrames->headNode = det->pageFrames->tailNode = newFrmNode();
  
  det = processing(det,numPages);    
  processingBM(det, bm,pageFileName, numPages,strategy);
  
  return closePageFile(&fileH);
}

bufferManagerInfo *setHeadTailNull(bufferManagerInfo *det){
 det->pageFrames->tailNode = det->pageFrames->headNode = NULL;
 return det;
}

bufferManagerInfo *processingDet(pgFrmDLLNode *curr, bufferManagerInfo *det){
 if(curr!=NULL){
   for(int i=0; curr != NULL; i++){
     curr = curr->nxt;
     freeData(det);
     freeHeadNode(det);
     (*det).pageFrames->headNode = curr;
   }
 }
 det=setHeadTailNull(det);
 free(det->pageFrames);
 return det;
}

RC shutdownBufferPool(BM_BufferPool *const bm)
{    
  if(bm && bm->numPages > 0){
    RC stat = forceFlushPool(bm);
    if(stat == RC_OK){
      bufferManagerInfo *det = (bufferManagerInfo *)bm->mgmtData;
      pgFrmDLLNode *curr = det->pageFrames->headNode;
      det = processingDet(curr,det);      
      free(det);
      bm->numPages = 0;
      return RC_OK;
    }
    else{
      return stat;
    }    
  }
  else{
    return RC_INVALID_BUFFER_M;
  }
}

bufferManagerInfo *moreProcessingKHistory(bufferManagerInfo *det, pgFrmDLLNode *fd, int i){
 det->kHistory[fd->pgNum][i] = det->kHistory[fd->pgNum][i-1];
 return det;
}

RC forceFlushPool(BM_BufferPool *const bm)
{
  if(bm && bm->numPages > 0){  
    SM_FileHandle fileH;
    int oStat=openPageFile ((char *)(bm->pageFile), &fileH);
    if (oStat == RC_OK){
      bufferManagerInfo *det = (bufferManagerInfo *)bm->mgmtData;
      pgFrmDLLNode *curr = det->pageFrames->headNode;  
        
      for(int i=0; curr != NULL; i++){
        if(curr->dirtyFlag == 1){
          int stat = writeBlock(curr->pgNum, &fileH, curr->data);
          if(stat != RC_OK){
            return RC_WRITE_FAILED;
          }
          curr->dirtyFlag = 0;
          (det->numOfWriteDone)++;
        }
        curr = curr->nxt;
      }  
    return closePageFile(&fileH);
   }
   else{
    return RC_FILE_NOT_FOUND;
   }    
  }
  else{
    return RC_INVALID_BUFFER_M;
  }
}

RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
  if(bm && bm->numPages > 0){  
    pgFrmDLLNode *fd;
    bufferManagerInfo *det = (bufferManagerInfo *)bm->mgmtData;
        
    // Locating the pg to mark dirty
    fd = findNdByPgNum(page->pageNum, det->pageFrames);
    
    //found the page
    if(fd != NULL){
      fd->dirtyFlag = 1; // Marking as dirty pg 
      return RC_OK;
    }
    else{
      return RC_NON_EXISTING_PG_IN_FRM;
    }    
  }
  else{
    return RC_INVALID_BUFFER_M;
  }
}

pgFrmDLLNode *findPageInMemory(BM_PageHandle *const page, BM_BufferPool *const bm, const PageNumber pageNum){

  bufferManagerInfo *det = (bufferManagerInfo *)(*bm).mgmtData;
  pgFrmDLLNode *fd; 
  if((det->arrPgToFrm)[pageNum] == NO_PAGE)
    return NULL;
  else{
    if((fd = findNdByPgNum(pageNum, det->pageFrames)) != NULL){
      /* pinned, so increase the fix count and the read-count*/
      fd->refBit = 1;
      fd->fixCount++;
      
      /* provide the client with the data and details of page*/
      page->data = fd->data;
      page->pageNum = pageNum;     
      return fd;
    }
    else{
      return NULL;
    }   
  }    
}

bufferManagerInfo *processingKHistory(bufferManagerInfo *det, pgFrmDLLNode *fd){
 (*det).kHistory[fd->pgNum][0] = det->countOfPinning;
 return det;
}

RC updtNewFrm(pgFrmDLLNode *fd, BM_PageHandle *const pg, BM_BufferPool *const bm, const PageNumber pageNum){
  bufferManagerInfo *det = (bufferManagerInfo *)bm->mgmtData;
  SM_FileHandle fileH;
  RC stat;
  stat = openPageFile ((char *)(bm->pageFile), &fileH);
  if (stat == RC_OK){
    // If dirty frame is to be replaced, write to the disk
    if(fd->dirtyFlag != 0){
      stat = ensureCapacity(pageNum, &fileH);
      if(stat == RC_OK){
        stat = writeBlock(fd->pgNum,&fileH, fd->data);
        if(stat == RC_OK){
          (det->numOfWriteDone)++;
        }
        else{
          return stat;
        }        
      }
      else{
        return stat;
      }  
    }    
    // Updating the arrPgToFrm lookup, setting the replaceable pg's value as NO_PAGE
    (det->arrPgToFrm)[fd->pgNum] = NO_PAGE;
    
    // Reading the data into new frm    
    if(((stat = ensureCapacity(pageNum, &fileH)) != RC_OK) ||((stat = readBlock(pageNum, &fileH, fd->data)) != RC_OK) ){
        return stat;
    }
    (det->numOfReadDone)+=1;
    // providing client with the data and info of pg
    pg->data = fd->data;
    pg->pageNum = pageNum ? pageNum : 0;   
    
    // Setting all the params of the new frm, and updating the lookup arrs    
    fd->fixCount = fd->refBit = 1;
    fd->dirtyFlag = 0;
    fd->pgNum = pageNum ? pageNum : 0;
    
    (det->arrFrmToPg)[fd->frmNum] = fd->pgNum ? fd->pgNum : 0;
    (det->arrPgToFrm)[fd->pgNum] = fd->frmNum ? fd->frmNum : 0;
    return closePageFile(&fileH);
  }
  else{
    return stat;
  }   
}


   
//LRU page replacement strategy
RC pinPageStrat_LRU (BM_PageHandle *const pg, BM_BufferPool *const bm, const PageNumber pageNum)
{
  if(bm){
    bufferManagerInfo *det = (bufferManagerInfo *)(*bm).mgmtData;
    pgFrmDLLNode *fd;
    
    fd = findPageInMemory(pg, bm, pageNum);
    // Checking if pg is present in memory. The arrPgToFrm gives the fast look up
    if(fd){
        // Putting this frm as the headNode of the frm list, as it is the latest used frm
        updtHeadNode(fd, &(det->pageFrames));
        return RC_OK;
    }
    
    int filledFramesNum = det->numOfFilledFrames;

    if(filledFramesNum >= bm->numPages){
    
    // if all the frms are filled, for new pg try to find a frm with fixedCount = 0 . Start from the tailNode
        fd = det->pageFrames->tailNode;
        int i=0;
        for(;fd != NULL && fd->fixCount != 0;i++){
            fd = fd->prev;
        }
        
        // If reached headNode, then no frms were dound with fixedCount = 0
        if (!fd){
            return RC_NO_SPACE_IN_BP;
        }   
    }
    else{
        // find the first free frm from the headNode, if the frms in the memory are less than the total number of available frms    
        fd = det->pageFrames->headNode;
        
        int i = 0;
        for(;i < det->numOfFilledFrames;i=i+1){
            fd = fd->nxt;
        }
        // increasing the frame count
        (det->numOfFilledFrames)+=1;
    }
    
    RC stat = updtNewFrm(fd, pg, bm, pageNum);
    
    if(stat == RC_OK){
      // Since it is the latest used frm, putting this frm to the head of the frame list
      updtHeadNode(fd, &(det->pageFrames));
      return RC_OK;
    }
    else{
      return stat;
    }   
  }
  else{
     return RC_INVALID_BUFFER_M;
  }
}

RC pinPageStrat_LRU_K (BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum)
{
if(bm){
    bufferManagerInfo *det = (bufferManagerInfo *)bm->mgmtData;
    pgFrmDLLNode *fd;
    int K_Val = (int)((*det).strategyData);
    int i;
    ((*det).countOfPinning)+=1;
    fd = findPageInMemory(page, bm, pageNum);  // Checking if pg is present in memory. The arrPgToFrm gives the fast look up
    if(fd){
        i = K_Val-1;
        for(; i>0; i=i-1){
           det = moreProcessingKHistory(det,fd,i);
        }
        
        det = processingKHistory(det,fd);
        
        return RC_OK;
    }
        
    if(((*det).numOfFilledFrames) >= bm->numPages){
    // if all the frms are filled, for new pg try to find a frm with fixedCount = 0 . Start from the headNode
        
        int dst, max_dst = -1;
        pgFrmDLLNode *curr;
        curr = (*det).pageFrames->headNode;
        
        for(int j=0; curr != NULL;j=j+1,curr = curr->nxt){
            if(curr->fixCount == 0 && det->kHistory[curr->pgNum][K_Val] != -1){
                
                dst = (*det).countOfPinning - (*det).kHistory[curr->pgNum][K_Val];
                
                if(!(dst < max_dst) && !(dst==max_dst)){
                    max_dst = dst;
                    fd = curr;
                }
            }
            
        }
        
        // no frames with fixed count=0, if reached to headNode
        if(max_dst == -1){
            curr = det->pageFrames->headNode;
            
            for(int j=0; curr->fixCount != 0 && curr != NULL;j++, curr = curr->nxt){
                dst = (*det).countOfPinning - (*det).kHistory[curr->pgNum][0];
                if(!(dst < max_dst) && !(dst==max_dst)){
                    max_dst = dst;
                    fd = curr;
                }                
            }
            
            // no frames with fixed count=0, if reached to headNode
            if (max_dst == -1){
                return RC_NO_SPACE_IN_BP;
            }
        }    
    }
    else{
        // find the first free frm from the headNode, if the frms in the memory are less than the total number of available frms   
        fd = det->pageFrames->headNode;
        
        int i = 0;
        for(;i < det->numOfFilledFrames;i=i+1){
            fd = fd->nxt;
        }
        (det->numOfFilledFrames)++; //increasing frame count by 1
    }
    
    RC stat;
    stat = updtNewFrm(fd, page, bm, pageNum);
    if(stat == RC_OK){
      for(i = K_Val-1; i>0; i=i-1){
        det = moreProcessingKHistory(det,fd,i);
      }
      det->kHistory[fd->pgNum][0] = det->countOfPinning;
      return RC_OK;
    }
    return stat;
  }
  else{
    return RC_INVALID_BUFFER_M;
  }
}

RC pinPageStrat_FIFO (BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum)
{
   if(bm){
    pgFrmDLLNode *fd;
    bufferManagerInfo *info = (bufferManagerInfo *)(*bm).mgmtData;
    fd = findPageInMemory(page, bm, pageNum);//Checking if pg is present in memory. The pgToFrm array gives the fast lookup
    
    if(!fd ){
      int filledFrames = info->numOfFilledFrames;
      fd = info->pageFrames->tailNode;
    
      if(filledFrames >= bm->numPages){
        //if all frames are filled, For new page , finding the oldest frame with fixCount = 0
        for(int i=0; fd != NULL && fd->fixCount != 0;i++){
          fd = fd->prev;
        }
        if (fd != NULL){
          updtHeadNode(fd, &(info->pageFrames));
        }
        else{
          return RC_NO_SPACE_IN_BP;
        }        
      }
      else{//first free frm from the headNode if the frms in the memory are less than the total available frms
        fd = info->pageFrames->headNode;
        int i = 0;
        
        for(;i < info->numOfFilledFrames;i=i+1){
            fd = fd->nxt;
        }
        (info->numOfFilledFrames)+=1; // increasing the frameCount by 1
        updtHeadNode(fd, &(info->pageFrames));
      }
    
      RC stat = updtNewFrm(fd, page, bm, pageNum);
    
      if(stat == RC_OK){
        return RC_OK;
      }
      return stat;
    }
    else{
      return RC_OK;
    }
  }
  else
    return RC_INVALID_BUFFER_M;    
}

RC pinPageStrat_CLOCK (BM_PageHandle *const pg, BM_BufferPool *const bm, const PageNumber pageNum)
{
  bufferManagerInfo *det = (bufferManagerInfo *)bm->mgmtData;
  pgFrmDLLNode *fd= findPageInMemory(pg, bm, pageNum);
  //Pg not in memory
  if(!fd){
    pgFrmDLLNode *SD = det->pageFrames->headNode;
    // retrieving first frm with refBit as 0 and setting all the bits to 0 along the way
    for (int i=0; SD != NULL && SD->refBit == 1;i++, SD = SD->nxt){
      SD->refBit = 0;      
    }
    if (!SD){
      return RC_NO_SPACE_IN_BP;
    }
    fd = SD;
    //calling updtNewFrm with the new val of fd
    RC stat = updtNewFrm(fd, pg, bm, pageNum);    
    return stat;
  }
  // Pg is there in the memory
  else{
    return RC_OK;
  }
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    if (bm && bm->numPages > 0){
      pgFrmDLLNode *fd;
      bufferManagerInfo *det = (bufferManagerInfo *)bm->mgmtData;
      fd = findNdByPgNum(page->pageNum, det->pageFrames);// Locating the pg to be unpinned      
      if(fd != NULL){       
       if(fd->fixCount <= 0){
         return RC_NON_EXISTING_PG_IN_FRM;        
       }
       else{      
         fd->fixCount-=1; //decreasing the fixCount by 1
       }    
       return RC_OK;
      }
      else{
       return RC_NON_EXISTING_PG_IN_FRM;
      } 
    }
    else{
      return RC_INVALID_BUFFER_M;
    }    
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
  if (bm && bm->numPages > 0){
    if(pageNum >= 0){
     if((*bm).strategy == RS_CLOCK)
        return pinPageStrat_CLOCK(page,bm,pageNum);      
      else if((*bm).strategy == RS_LRU)
        return pinPageStrat_LRU(page,bm,pageNum);      
      else if((*bm).strategy == RS_FIFO)
        return pinPageStrat_FIFO(bm,page,pageNum);
      else if((*bm).strategy == RS_LRU_K)
        return pinPageStrat_LRU_K(bm, page, pageNum);        
      return RC_OK;
    }
    else{
      return RC_NON_EXISTING_PG_IN_FRM;
    }
  }
  else{
    return RC_INVALID_BUFFER_M;
  }    
}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)

{
    pgFrmDLLNode *fd;
    bufferManagerInfo *det = (bufferManagerInfo *)bm->mgmtData;
    SM_FileHandle fileH;
    if (bm && bm->numPages > 0){   
    
      if (openPageFile ((char *)(bm->pageFile), &fileH) == RC_OK){
        fd = findNdByPgNum(page->pageNum, det->pageFrames);/* Locate the page to be forced on the disk */
        if(fd){
          int stat = writeBlock(fd->pgNum, &fileH, fd->data); /* write the current content of the page back to the page file on disk */
        if(stat == RC_OK){
         (det->numOfWriteDone)+=1; 
          return  closePageFile(&fileH); 
         
        }
        else{
          closePageFile(&fileH);
          return RC_WRITE_FAILED;
        }        
      }
      else{
         closePageFile(&fileH);
          return RC_NON_EXISTING_PG_IN_FRM;
        }
    
       
      }
      else{
      return RC_FILE_NOT_FOUND;
      }
      
    }
    else{
      return RC_INVALID_BUFFER_M;
    }   
}

PageNumber *getFrameContents (BM_BufferPool *const bm)
{
  if(bm){
    int arr[MAX_FRMS];
    initializeLkArray(arr,NO_PAGE,MAX_FRMS,int);
    //return the val of arrFrmToPg
    return ((bufferManagerInfo *)bm->mgmtData)->arrFrmToPg;
  }
  else
    return 0;  
}

bool *getDirtyFlags (BM_BufferPool *const bm)
{
    //going through the list of frms and updates the val of dirtyFlags
    bufferManagerInfo *det = (bufferManagerInfo *)bm->mgmtData;
    pgFrmDLLNode *curr = (*det).pageFrames->headNode;
    
    for (int i=0; curr != NULL; i++,curr = (*curr).nxt){
        (det->dirtyFlgs)[curr->frmNum] = (*curr).dirtyFlag;        
    }    
    return (*det).dirtyFlgs;
}

int *getFixCounts (BM_BufferPool *const bm)
{
   //iterating through the entire list of frames and modifying the val of fixedCount of frms
    bufferManagerInfo *det = (bufferManagerInfo *)bm->mgmtData;
    pgFrmDLLNode *curr = (*det).pageFrames->headNode;
    
    for (int i=0; curr != NULL; i++,curr = curr->nxt){
        (det->fixedCountsOfFrames)[curr->frmNum] = curr->fixCount;        
    }     
    return (*det).fixedCountsOfFrames;
}

int getNumReadIO (BM_BufferPool *const bm)
{
  int reads = 0;  
  reads = ((bufferManagerInfo *)bm->mgmtData)->numOfReadDone;  
  return reads; //returning the value of numRead
}

int getNumWriteIO (BM_BufferPool *const bm)
{
  int writes = 0;
  writes = ((bufferManagerInfo *)bm->mgmtData)->numOfWriteDone;
  return writes;// returning the val of numWrite
}
