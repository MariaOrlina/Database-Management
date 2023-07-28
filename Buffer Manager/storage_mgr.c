#include<stdlib.h>
#include<stdio.h>
#include<sys/stat.h>
#include"storage_mgr.h"
#include<string.h>

#define openDataFile(fileName,mode) fopen(fileName, mode)
#define closeDataFile(openFile) fclose(openFile)
#define allocate() (char *) calloc(PAGE_SIZE, sizeof(char))
#define writePageFile(pagePointer, fileStream) fwrite(pagePointer, sizeof(char), PAGE_SIZE, fileStream)
#define seekMethod(fHandle, pageNum) fseek(fHandle->mgmtInfo, (pageNum+1)*PAGE_SIZE*sizeof(char), SEEK_SET)
#define readPageFile(memPage,fHandle) fread(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo)


void initStorageManager (void){
 printf("Group3 Storage Manager Initialized!\n");
}

//Method for creating a page file
RC createPageFile (char *fileName){
 FILE *createFile = openDataFile(fileName, "w"); //to open a file using macro defined above
 if(fileName == NULL){
   if(closeDataFile(createFile)!=0) //to close a file using macro defined above
     {  
       return RC_FILE_CLOSE_FAILURE;
     }
   return RC_FILE_NOT_FOUND;
  }
  else{
    char *totalPage_ptr, *firstPage_ptr;

    totalPage_ptr = allocate();  // allocating "first" page to store information about total number of pages and filling page with ’\0’ bytes.
    firstPage_ptr = allocate();  // actual first page and filling page with ’\0’ bytes.

    strcat(totalPage_ptr,"1\n");

    writePageFile(totalPage_ptr, createFile);
    writePageFile(firstPage_ptr, createFile);

    free(totalPage_ptr);
    free(firstPage_ptr);

    if(closeDataFile(createFile)!=0)  
    {
      return RC_FILE_CLOSE_FAILURE;
    }

    return RC_OK;
  }
}

//Method for opening a page file
RC openPageFile (char *fileName, SM_FileHandle *fHandle){
 FILE *openFile;
 openFile = openDataFile(fileName, "r+");
 if (openFile!=NULL) //found the file
   {
     char *t;
     t = allocate();
     fgets(t, PAGE_SIZE, openFile);
     t = strtok(t, "\n"); // removing trailing newline character
     
     //structure variables initialization 
     fHandle->mgmtInfo = openFile;  
     fHandle->curPagePos = 0;
     fHandle->totalNumPages = atoi(t) ? atoi(t) : 0;   
     fHandle->fileName = fileName;
     free(t);
     return RC_OK;
   }
   else{
     return RC_FILE_NOT_FOUND;
   }
}

//Method for closing a page file
RC closePageFile (SM_FileHandle *fHandle){
 if(fHandle!=NULL){
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
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
 if (fHandle != NULL)  //checking file handle initialization
 {
   FILE *openFile = openDataFile(fHandle->fileName,"r+");
   if(openFile == NULL) //checking for presence of the file
     return RC_FILE_NOT_FOUND;
      
   size_t readSize;
   int totalPages=fHandle->totalNumPages;
   if (pageNum > totalPages || pageNum < 0)//returning error if its an invalid page number
   {
     return RC_READ_NON_EXISTING_PAGE;
   }
   char *fName =fHandle->mgmtInfo;
   if (fName == NULL) //checking for presence of the file
     return RC_FILE_NOT_FOUND;
    
   int positionSetSuccess = seekMethod(fHandle, pageNum); //seeking to a block as per the page number
   if (positionSetSuccess == 0){
     readSize = readPageFile(memPage,fHandle); //reading the block
     fHandle->curPagePos = pageNum ? pageNum : 0;
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
int getBlockPos (SM_FileHandle *fHandle){
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
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
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
   int last_pos=fHandle->totalNumPages;
   return readBlock(last_pos,fHandle, memPage); //reading last block
 }
 return RC_FILE_NOT_FOUND; 
}

// Writing a page to the disk
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
 if(fHandle!=NULL){
   size_t writeSize;
   
   // Checking whether totalPages is equal to fHandle->totalNumPages using Conditional Expression  
   int totalPages = fHandle->totalNumPages ? fHandle->totalNumPages : 0;

   // Checking whether totalPages less than pageNum or 0 using or operator
   if (pageNum > (totalPages) || (pageNum < 0)){
     return RC_WRITE_FAILED;
   }
   
   // Checking for fseek value
   int positionSetSuccess = fseek(fHandle->mgmtInfo, (pageNum+1)*PAGE_SIZE*sizeof(char), SEEK_SET);

   // If fseek equals to 0 then writing the page to the disk
   if (positionSetSuccess == 0){
     writeSize = writePageFile(memPage, fHandle->mgmtInfo); //writes data to the file
     fHandle->curPagePos = pageNum ? pageNum : 0;
     return RC_OK;
   }
   else{
     return RC_WRITE_FAILED;
   }
 }
 return RC_FILE_HANDLE_NOT_INIT; 
}

// Writing a page to the disk at the current position
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
 FILE *openFile = openDataFile(fHandle->fileName,"r+");
 if(openFile != NULL){
   // Assigning the current page position to a variable
   int presentPage=fHandle->curPagePos;
   fHandle->totalNumPages=fHandle->totalNumPages+1;
   
   // Calling writeBlock function after updating the total number of pages   
   return writeBlock(presentPage, fHandle, memPage);
 }
 // If file not found returns RC_FILE_NOT_FOUND
 return RC_FILE_NOT_FOUND;
}

// Increasing the number of pages in the file by one
RC appendEmptyBlock (SM_FileHandle *fHandle){
 if(fHandle!=NULL){
   SM_PageHandle newBlock;
   size_t writeSize;

   newBlock = allocate(); // allocating memory and returning a pointer

   int totalPages=fHandle->totalNumPages;
   int positionSetSuccess = fseek(fHandle->mgmtInfo,(totalPages + 1)*PAGE_SIZE*sizeof(char) , SEEK_END); //seeking file write ptr to the last pg

   if (positionSetSuccess == 0){
     writeSize = writePageFile(newBlock, fHandle->mgmtInfo); 
     fHandle->totalNumPages = fHandle->totalNumPages + 1;
     fHandle->curPagePos = fHandle->totalNumPages ? fHandle->totalNumPages : 0;
     rewind(fHandle->mgmtInfo);
     fprintf(fHandle->mgmtInfo, "%d\n" , fHandle->totalNumPages); //updating total number of pages info
     free(newBlock);
     fseek(fHandle->mgmtInfo, (fHandle->totalNumPages + 1)*PAGE_SIZE*sizeof(char), SEEK_SET);        
     return RC_OK;
   }
   else{
     free(newBlock);
     return RC_WRITE_FAILED;
   }
 }
 return RC_FILE_HANDLE_NOT_INIT; 
}

// Ensuring the capacity of the file. If number of pages are less than numberOfPages increase the size to numberOfPages
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
 if(fHandle!=NULL){
   if (fHandle->totalNumPages < numberOfPages){
     int pagesToAdd = numberOfPages - fHandle->totalNumPages;
     int i=0;
     
     // Using for loop to call appendEmptyBlock function and increasing the size
     for(; i < pagesToAdd; i=i+1)
     {
       appendEmptyBlock(fHandle); 
     }
   }
   return RC_OK;    
 }
 return RC_FILE_HANDLE_NOT_INIT; 
}

