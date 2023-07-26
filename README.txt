Project Modules
C source files: storage_mgr.c, dberror.c, test_assign1_1.c
Header files: storage_mgr.h, dberror.h, test_helper.h

Team members:
1) Aditi Tanwar 
2) Elisha Maria Krista Orlina 
3) Naga Mohan Reddy Karri

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