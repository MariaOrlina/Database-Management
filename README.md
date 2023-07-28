# Database-Management

# For Storage Manager :
Aim:
The goal of this assignment is to implement a storage manager - a module that is capable of reading blocks from a file on disk into memory and writing blocks from memory to a file on disk. The storage manager deals with pages (blocks) of fixed size (PAGE_SIZE). We have written functions to create, open and close files and read, write,append pages in the file. Read functions can also read current block ,next block, previous block etc.

Instructions to run the code
Go to the path where the extracted files are present.
Run the command: make
Run command for testing test_assign1.c: ./test_assign1.c

Functions:
Function used to implement storage manager are described below:

initStorageManager()
Initializes the program.

createPageFile()
Creates a new page file fileName. The initial file size is one page. This method fills the single page with ’\0’ bytes.

openPageFile()
Opens an existing page file. It returns RC FILE NOT FOUND if the file does not exist. The second parameter is an existing file handle. If opening the file is successful, then the fields of this file handle are initialized with the information about the opened file.

closePageFile()
Closes the file. If the file doesn't exist or unable to open in 'read' mode ,then returns the error code accordingly.

destroyPageFile()
Removes the existing file and returns success message. If file does not exist returns the error code.

readBlock()
Reads the block at position pageNum from a file and stores its content in the memory pointed to by the memPage page handle. If the file has less than pageNum pages, it returns RC READ NON EXISTING PAGE.

getBlockPos()
Return the current page position in a file.

readFirstBlock ()
Reads the first page in a file

readPreviousBlock ()
Read the previous page relative to the curPagePos of the file.

readCurrentBlock ()
Reads the current relative to the curPagePos of the file.

readNextBlock ()
Reads the next page relative to the curPagePos of the file.

readLastBlock ()
Reads the last page in a file.

writeBlock()
Writes a page to disk using an absolute position.

writeCurrentBlock()
Writes a page to disk using either the current position.

appendEmptyBlock()
Increases the number of pages in the file by one. The new last page is filled with zero bytes.

ensureCapacity()
If the file has less than numberOfPages pages then increase the size to numberOfPages

# For Buffer Manager :
How to Run the script:

With default test cases-
 Compile: 
   make assign2_1
   make assign2_2
 Run:
   ./assign2_1
   ./assign2_2
 For memory leaks:
   valgrind --leak-check=full --track-origins=yes ./assign2_1
   valgrind --leak-check=full --track-origins=yes ./assign2_2

Logic:

Data Structures and Design-

Each node of the frame is set as pgFrmDLLNode and it contains the following;
1.pgNum- The number of pages present in the pagefile
2.frmNum- The number of frames present in the frame list
3.dirtyFlag- This is the dirty bit of the frame, it is used to perform any modification if required before sending the file to the disk from the buffer pool.
4.fixCount- It is used in during pinning/unpinning request
5.refBit- Reference bit per node used in any page replacement algorithm
6.data- actual data pointed by the framenode

At the BufferPool level, just a few attributes are declared. We created a structure named bufferManagerInfo that has these attributes and have allocated it to the BM BufferPool->mgmtData. 

1.numOfFilledFrames- count of the filled number of frames in the list
2.numOfReadDone- total number of read performed on the buffer pool
3.numOfWriteDone- total number of write performed on the buffer pool
4.countOfPinning- total number of pinning done for the buffer pool
5.strategyData- value of BM_BufferPool->StartData
6.arrPgToFrm- It is an array drawn from pagenumber to framenumber. 
7.dirtyFlgs- An array of dirtyflags of all the frames
8.fixedCountsOfFrames- an array of fixed count of all the frames

BufferPool Functions-
1.initBufferPool- uses the page replacement approach to construct a new buffer pool with numPages page frames. Pages from the page file with the name pageFileName are cached in the pool. All page frames ought to start out empty. In other words, this procedure shouldn't create a new page file; the page file should already be there. The page replacement strategy's parameters can be given using stratData. For instance, the parameter k for LRU-k may be this.
2.shutDownBufferPool- eliminates a buffer pool. This procedure needs to release all buffer pool-related resources. For instance, it needs to release the memory designated for page frames. Before destructing the buffer pool, any dirty pages should be written back to disk if there are any. 
3.forceFlushPool- causes every dirty page from the buffer pool with a fix count of 0 to be written to disk.

PageManagement Functions-
1.PinPage- pageNum is used to pin the page. The page handle given to the method's buffer manager must have the pageNum field set. The data field should similarly refer to the page frame that the page is saved in (the area in memory storing the content of the page).
2.unpinPage- removes the page's pins. To determine which page to unpin, utilize the page's pageNum field.
3.markDirty- marks a page dirty
4.forcePage- should update the page's content and write it back to the disk's page file.

Statistics Functions-
These operations give statistics regarding a buffer pool's contents. These functions are used internally by the print debug procedures described below to collect data about a pool.
1.getFrameContents- the ith value of the array of PageNumbers returned by the method is the number of the page stored in the ith page frame. The array has a size of numPages. The constant NO PAGE is used to indicate an empty page frame.
2.getDirtyFlags- If a page stored in an ith page frame is dirty, the function returns an array of bools (of size numPages) with the ith member being TRUE. Page frames that are empty are viewed as clean.
3.getFixCounts- The function's output is an array of ints with a size of numPages, where the ith element represents the fixed count of the page that is kept in the ith page frame. For empty page frames, return 0.
4.getNumReadIO- When a buffer pool is started, the function returns the total number of pages that have been read from the disk. This statistic must be initialized by the code at the moment the pool is created and must be updated each time a page is read into a page frame from the page file.
5.getNumWritesIO- returns how many pages have been written to the page file since the buffer pool was set up.


The Page Replacement Strategies:

pinPageStrat_FIFO-

1.It first determines whether the page is in memory. If the page is located, the method returns RC_OK and calls the findPageInMemory function as detailed in "Helper Functions".
2.The first free frame is found starting from the head if the page needs to be loaded into memory. The page is loaded and the page details are set if an empty frame is discovered. Also, the found frame is updated to be the head of the linked list.
3.If all of the frames on a new page are filled, the method begins iterating from the list's trail to find the oldest frame with a fixed count of 0. The linked list's head is updated to point to the detected frame.
4.The updtNewFrm method is discussed in "Helper Functions" later if the frame is located using the aforementioned strategy; otherwise, the function returns no more space in buffer error.

pinPageStrat_LRU_K:

1.It first determines whether the page is in memory. If so, it instantly returns with RC_OK and calls the findPageInMemory function discussed in Helper Functions.
2.The reference number (current count of pinning) is changed in the history array (bufferManagemenIinfo->kHistory) each time a frame is referred.
3.The distance is calculated as the difference between the current count of pinning and the page's kth reference if the page is not in memory, with all pages in memory having a fixCount of 0. If the page is in memory, iteration begins at the top of the list.
4.The maximum distance page has been replaced. It operates exactly like the LRU and looks for the least recently used page if no page is called k times (kth reference is -1 for all pages).
5.In either instance, if a frame matching that description is located, the updtNewFrm function is called; otherwise, it returns no more space in buffer error.

pinPageStrat_LRU- 
The LRU replacement strategy is put into practice using this function, as explained in the lecture.
1.It first determines whether the page is in memory. If so, it instantly returns with RC OK and calls the findPageInMemory function discussed in Helper Functions.
2.A frame gets advanced to the front of the frmList each time it is referred to. The head will therefore always be the frame that has been used most recently, while the tail will always be the frame that has been used the least.
3.It begins iterating from the tail of the list to look for a frame with fixCount 0 if the page is not in memory.
4.No more space in buffer error is returned if such a frame is found; otherwise, it performs the updtNewFrm method mentioned in the section below on helper functions.

Helper Functions:

updateHead- 
1.This function makes the given node the top of the given list and accepts a frmList and a pgFrmDLLNode as parameters.
2.Different page replacement techniques use this method to maintain the logical order of the frames in the frame list.

findNdByPgNum-
1.This function looks for the page in the frmList using a frmList and a pgNum as parameters.
2.It returns the pgFrmDLLNodeif the page was discovered; else, it returns NULL.
3.Various page replacement techniques use this function to search the frame list for the needed frame.

findPageInMemory -
1.This function searches the frmList for the specified page using the parameters BM BufferPool, BM PageHandle, and pageNumber.
2.It sets the BM PageHandle to refer to this page in memory if the page is discovered and It raises the page fixcount.
3.Different page replacement algorithms use this function when a page is already present in the frame list.

updtNewFrm -
1.The inputs for this function are a BM BufferPool, a BM PageHandle, the target frame, and a pageNumber.
2.If a dirty page exists in the destination frame, it is written back to the disk and the associated attributes are updated.
3.The destination page is then read into memory from the disk and stored in the designated frame.
4.It modifies the destination frame's pgNum, numOfReadDone, dirtyFlag, fixCount, and refBit characteristics.
5.In the event that the page is not in the memory, many page replacement schemes employ this function. The function is called along with the frame that has to be changed and the newly loaded page.

Additional Page replacement strategies-


pinPageStrat_CLOCK:

1.This function puts into practice the lecture's explanation of the clock replacement policy.
2.It first determines whether the page is in memory. If it is, it immediately responds with RC_OK.
3.It looks for the first frame with a reference bit equal to zero if the page is not present in memory. All reference bits are set to zero along the way. pgFrmDLLNode->refBit sets the reference bit (refBit).
4.In updtNewFrm, the updated value of discovered is utilized.


