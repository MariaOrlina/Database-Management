#include<stdlib.h>
#include<stdio.h>
#include<sys/stat.h>
#include"storage_mgr.h"
#include<string.h>

#define openDataFile(fileName,mode) fopen(fileName, mode)
#define closeDataFile(openFile) fclose(openFile)

/*******************************************************************************************************************************************************************
 * Authors:
 *
 * NAME		               EMAIL			IMPLEMENTATION
 * --------------------------  -----------------------	------------------------------------------------------------------------------------------------------------
 * Aditi Tanwar                <atanwar@hawk.iit.edu>	initStorageManager(), createPageFile(), openPageFile(), closePageFile(), destroyPageFile(), readBlock();
 * Elisha Maria Krista Orlina  <eorlina@hawk.iit.edu>	getBlockPos(), readFirstBlock(), readPreviousBlock(), readCurrentBlock(), readNextBlock(), readLastBlock();
 * Naga Mohan Reddy Karri      <nkarri1@hawk.iit.edu>   writeBlock(), writeCurrentBlock(), appendEmptyBlock(), ensureCapacity();
 *******************************************************************************************************************************************************************/

void initStorageManager(void){
  printf("Group3 Storage Manager Initialized!\n");
}

//Method for creating a page file
RC createPageFile(char *fileName){
  FILE *createFile = openDataFile(fileName,"w");  //to open a file using macro defined above
  if(createFile == NULL){
    if(closeDataFile(createFile)!=0) //to close a file using macro defined above
    {  
      return RC_FILE_CLOSE_FAILURE;
    }
      return RC_FILE_NOT_FOUND;
  }
  else{
    fseek(createFile,PAGE_SIZE,SEEK_SET); //setting file position at the beginning of the file
    putc('\0',createFile); //filling single page with ’\0’ bytes.
    if(closeDataFile(createFile)!=0)  
    {
      return RC_FILE_CLOSE_FAILURE;
    }
    return RC_OK;
  }
}

//Method for opening a page file
RC openPageFile(char *fileName, SM_FileHandle *fHandle){
  FILE *openFile;
  openFile = openDataFile(fileName, "r+");

  if(openFile != NULL)  //found the file
  {
   struct stat openFlStat; //to calculate file size
   stat(fileName,&openFlStat);
   
   //structure variables initialization
   fHandle->mgmtInfo = openFile;
   fHandle->curPagePos = ftell(openFile)/PAGE_SIZE;
   fHandle->totalNumPages = (int)(openFlStat.st_size/PAGE_SIZE) ? (int)(openFlStat.st_size/PAGE_SIZE) : 0;
   fHandle->fileName = fileName;
   if(closeDataFile(openFile)!=0){
     return RC_FILE_CLOSE_FAILURE;
   } 
   return RC_OK;
  }
  else{
    return RC_FILE_NOT_FOUND;
  }	
}

//Method for closing a page file
RC closePageFile(SM_FileHandle *fHandle){
  if(fHandle != NULL){
    FILE *openFile = openDataFile(fHandle->fileName,"r");
    if(openFile == NULL){
      return RC_FILE_NOT_FOUND; 
    }		
    if(closeDataFile(fHandle->mgmtInfo)!=0) //closing the file
    {
      return RC_FILE_CLOSE_FAILURE;
    } 
    return RC_OK;
  }
  return RC_FILE_HANDLE_NOT_INIT;  //returning error in case file handle is not initialized
}

//Method to destroy a page file
RC destroyPageFile (char *fileName){
  FILE *destroyFile;
  destroyFile = openDataFile(fileName, "r");  //opening the page file which needs to be destroyed
  if(destroyFile != NULL){	
    if(closeDataFile(destroyFile)!=0){
      return RC_FILE_CLOSE_FAILURE;
    }
    int delStat = remove(fileName); //removing the page file 
    if(delStat == 0){
      return RC_OK;  // returning success code
    }	
  }
  return RC_FILE_NOT_FOUND; // returning error code incase the file to be destroyed doesn't exist
}

//Method to read a block in a page file
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
  if (fHandle != NULL)  //checking file handle initialization
  {  
    FILE *openFile = openDataFile(fHandle->fileName,"r+");
    if(openFile == NULL) //checking for presence of the file
      return RC_FILE_NOT_FOUND;
  
    int totalPages = fHandle->totalNumPages ? fHandle->totalNumPages : 0;
    if(totalPages < pageNum)  //returning error if its an invalid page number
      return RC_READ_NON_EXISTING_PAGE;

    int positionSetSuccess = fseek(fHandle->mgmtInfo , (pageNum*PAGE_SIZE) , SEEK_SET); //seeking to a block as per the page number
    if(positionSetSuccess == 0){
      size_t readSize = fread(memPage, 1, PAGE_SIZE, fHandle->mgmtInfo);  //reading the block
      if(readSize != PAGE_SIZE)
        return RC_READ_FILE_ERROR;
      fHandle->curPagePos = (ftell(fHandle->mgmtInfo)/PAGE_SIZE) ? (ftell(fHandle->mgmtInfo)/PAGE_SIZE) : 0;	//setting the positon of the curent page
    
      if(closeDataFile(openFile) != 0){ //closing the file
        return RC_FILE_CLOSE_FAILURE;
      }
      return RC_OK;
    }
    else{
      return RC_READ_NON_EXISTING_PAGE; //returning error if seeking to non existent block
    }
  }
  return RC_FILE_HANDLE_NOT_INIT;
}

//Method to return the position of the current page
int getBlockPos (SM_FileHandle *fHandle)
{		
  if(fHandle!=NULL){
    FILE *openFile = openDataFile(fHandle->fileName,"r");
    if(openFile != NULL)
      return fHandle->curPagePos; //returning the position of the current page
    return RC_FILE_NOT_FOUND;  //if file is not found return the error
  }
  return RC_FILE_HANDLE_NOT_INIT;
}

//Method to read the first block in a page file
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  int PageNum = fHandle->curPagePos;
  fseek(fHandle->mgmtInfo, 0, SEEK_SET);
  PageNum=0;
  return readBlock(PageNum, fHandle, memPage); //reading first block
}

//Method to read the previous block
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
  FILE *openFile = openDataFile(fHandle->fileName,"r");
  if(openFile != NULL) //if file exists
  {
    int cur_pos=fHandle->curPagePos;
    return readBlock(cur_pos-1,fHandle, memPage); //reading previous block
  }
  return RC_FILE_NOT_FOUND;
}

//Method to read the current block
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  FILE *openFile = openDataFile(fHandle->fileName,"r");
  if(openFile != NULL) //if file exists
  {
    int cur_pos=fHandle->curPagePos;
    return readBlock(cur_pos,fHandle, memPage); //reading current block
  }
  return RC_FILE_NOT_FOUND;
}

//Method to read next block
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  for(int i=1 ; i<= fHandle->totalNumPages; i++)
    fseek(fHandle->mgmtInfo, i, SEEK_CUR);
  int cur_pos=fHandle->curPagePos;
  return readBlock(cur_pos+1,fHandle, memPage); //reading next block
}

//Method to read last block
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  FILE *openFile = openDataFile(fHandle->fileName,"r");
  if(openFile != NULL){
    int last_pos=fHandle->totalNumPages - 1;
    return readBlock(last_pos,fHandle, memPage); //reading last block
  }
  return RC_FILE_NOT_FOUND;       	 
}

// Writing a page to the disk
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
  if (fHandle != NULL){  
    FILE *openFile = openDataFile(fHandle->fileName,"r+");
    
    // If the file is not found it returns RC_FILE_NOT_FOUND
    if(openFile == NULL)
      return RC_FILE_NOT_FOUND;

    // Checking whether totalPages is equal to fHandle->totalNumPages using Conditional Expression  
    int totalPages = fHandle->totalNumPages ? fHandle->totalNumPages : 0;

    // Checking whether totalPages less than pageNum or 0 using or operator
    if(totalPages < pageNum || pageNum < 0)
      return RC_WRITE_FAILED;
    
    // Checking for fseek value
    int positionSetSuccess = fseek(openFile , pageNum*PAGE_SIZE , SEEK_SET);

    // If fseek equals to 0 then writing the page to the disk
    if(positionSetSuccess == 0){
      size_t writeSize = fwrite(memPage,PAGE_SIZE,1,openFile) ;    
      fseek(openFile , (pageNum+1)*PAGE_SIZE , SEEK_SET);   
      fHandle->curPagePos = (ftell(fHandle->mgmtInfo)/PAGE_SIZE);
      fHandle->totalNumPages = (ftell(openFile)/PAGE_SIZE);
      
      // Closing the file
      if(closeDataFile(openFile) != 0){
       return RC_FILE_CLOSE_FAILURE;
      }
      return RC_OK;
    }
    else{
      return RC_WRITE_FAILED;
    } 
  }
  return RC_FILE_HANDLE_NOT_INIT;
}

// Writing a page to the disk at the current position of cursor
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
  FILE *openFile = openDataFile(fHandle->fileName,"r+");
  if(openFile != NULL){
    // Assigning the current page position to a variable
    int presentPage=fHandle->curPagePos/PAGE_SIZE;
    fHandle->totalNumPages=fHandle->totalNumPages+1;
    // Calling writeBlock function after updating the total number of pages   
    return writeBlock(presentPage, fHandle, memPage);
  }

  // If file not found returns RC_FILE_NOT_FOUND
  return RC_FILE_NOT_FOUND;
}

// Increasing the number of pages in the file by one
RC appendEmptyBlock (SM_FileHandle *fHandle) {    
  FILE *openFile=openDataFile(fHandle->fileName,"r+");
  if(openFile!=NULL){
    fseek(openFile, 0, SEEK_END);
    int i=0;
    for(; i < PAGE_SIZE; i=i+1){
      // Filling the empty block with '\0' bytes
      fwrite("\0",1, 1,openFile);
      fseek(openFile,0,SEEK_END);
    } 
    fHandle->totalNumPages = (ftell(openFile)/PAGE_SIZE) ? (ftell(openFile)/PAGE_SIZE) : 0;
    fHandle->mgmtInfo = openFile;
    return RC_OK;
  }
  else{
    return RC_FILE_NOT_FOUND;
  }
}

// Ensuring the capacity of the file. If number of pages are less than numberOfPages increase the size to numberOfPages
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
  if(fHandle!=NULL){
    if(openDataFile(fHandle->fileName,"r+") != NULL){
      if (numberOfPages > fHandle->totalNumPages){

        // Finding number of pages to be added
        int pagesToAdd = numberOfPages - fHandle->totalNumPages;
        int i=0;

        // Using for loop to call appendEmptyBlock function and increasing the size
        for(; i < pagesToAdd; i=i+1)
        {
          appendEmptyBlock(fHandle); 
        }
        
        return RC_OK;
      }

      // Returns RC_READ_NON_EXISTING_PAGE if numberOfPages are les than totalNumPages
      else{
        return RC_READ_NON_EXISTING_PAGE;
      }
    }
    return RC_FILE_NOT_FOUND; 
  }
  return RC_FILE_HANDLE_NOT_INIT;   
}