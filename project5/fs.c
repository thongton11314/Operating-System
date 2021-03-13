// ============================================================================
// fs.c - user FileSytem API
// ============================================================================

#include "bfs.h"
#include "fs.h"

// ============================================================================
// Close the file currently open on file descriptor 'fd'.
// ============================================================================
i32 fsClose(i32 fd)
{
  i32 inum = bfsFdToInum(fd);
  bfsDerefOFT(inum);
  return 0;
}

// ============================================================================
// Create the file called 'fname'.  Overwrite, if it already exsists.
// On success, return its file descriptor.  On failure, EFNF
// ============================================================================
i32 fsCreate(str fname)
{
  i32 inum = bfsCreateFile(fname);
  if (inum == EFNF)
    return EFNF;
  return bfsInumToFd(inum);
}

// ============================================================================
// Format the BFS disk by initializing the SuperBlock, Inodes, Directory and
// Freelist.  On succes, return 0.  On failure, abort
// ============================================================================
i32 fsFormat()
{
  FILE *fp = fopen(BFSDISK, "w+b");
  if (fp == NULL)
    FATAL(EDISKCREATE);

  i32 ret = bfsInitSuper(fp); // initialize Super block
  if (ret != 0)
  {
    fclose(fp);
    FATAL(ret);
  }

  ret = bfsInitInodes(fp); // initialize Inodes block
  if (ret != 0)
  {
    fclose(fp);
    FATAL(ret);
  }

  ret = bfsInitDir(fp); // initialize Dir block
  if (ret != 0)
  {
    fclose(fp);
    FATAL(ret);
  }

  ret = bfsInitFreeList(); // initialize Freelist
  if (ret != 0)
  {
    fclose(fp);
    FATAL(ret);
  }

  fclose(fp);
  return 0;
}

// ============================================================================
// Mount the BFS disk.  It must already exist
// ============================================================================
i32 fsMount()
{
  FILE *fp = fopen(BFSDISK, "rb");
  if (fp == NULL)
    FATAL(ENODISK); // BFSDISK not found
  fclose(fp);
  return 0;
}

// ============================================================================
// Open the existing file called 'fname'.  On success, return its file
// descriptor.  On failure, return EFNF
// ======================================================================== ====
i32 fsOpen(str fname)
{
  i32 inum = bfsLookupFile(fname); // lookup 'fname' in Directory
  if (inum == EFNF)
    return EFNF;
  return bfsInumToFd(inum);
}

// ============================================================================
// Read 'numb' bytes of data from the cursor in the file currently fsOpen'd on
// File Descriptor 'fd' into 'buf'.  On success, return actual number of bytes
// read (may be less than 'numb' if we hit EOF).  On failure, abort
// ============================================================================
i32 fsRead(i32 fd, i32 numb, void *buf)
{

  // Find the inum
  i32 inum = bfsFdToInum(fd);
  i32 count = 0; // count total size;

  // Find the current cursor
  i32 cursor = bfsTell(fd);
  i32 totalSize = numb;

  // If the size that's being read is larger than the iNode size, read only Inode remaining
  if (cursor + numb > bfsGetSize(inum)) totalSize = bfsGetSize(inum) - cursor; 
  i32 currFbn = cursor / BYTESPERBLOCK; // Get fileblock number, floored
  do
    {

    // 1 block = max of 512 bytes, get a max of 512 or the remainder of total bytes
    // Read max 512 bytes pertime
    if (totalSize > BYTESPERBLOCK) 
    {
      numb = BYTESPERBLOCK;
      totalSize -= BYTESPERBLOCK;
    }
    else
    {
      numb = totalSize;
      totalSize = 0;
    }

    // Used to temporary store buffer and put it back in later
    i8 tempBuf[BUFSIZ]; 

    // Read 
    bfsRead(inum, currFbn, tempBuf);    // Read
    memmove(buf + count, tempBuf, numb);// Move teporary buff into buffer
    memset(tempBuf, 0, BYTESPERBLOCK);  // Set temporary buff = 0


    // Set new cursor
    bfsSetCursor(inum, cursor += numb); // Move cursor up
    count += numb;                      // Increment total size read
    currFbn++;                          // Increment File Block number
    } while (totalSize > 0);
  return count;
}

// ============================================================================
// Move the cursor for the file currently open on File Descriptor 'fd' to the
// byte-offset 'offset'.  'whence' can be any of:
//
//  SEEK_SET : set cursor to 'offset'
//  SEEK_CUR : add 'offset' to the current cursor
//  SEEK_END : add 'offset' to the size of the file
//
// On success, return 0.  On failure, abort
// ============================================================================
i32 fsSeek(i32 fd, i32 offset, i32 whence)
{

  if (offset < 0)
    FATAL(EBADCURS);

  i32 inum = bfsFdToInum(fd);
  i32 ofte = bfsFindOFTE(inum);

  switch (whence)
  {
  case SEEK_SET:
    g_oft[ofte].curs = offset;
    break;
  case SEEK_CUR:
    g_oft[ofte].curs += offset;
    break;
  case SEEK_END:
  {
    i32 end = fsSize(fd);
    g_oft[ofte].curs = end + offset;
    break;
  }
  default:
    FATAL(EBADWHENCE);
  }
  return 0;
}

// ============================================================================
// Return the cursor position for the file open on File Descriptor 'fd'
// ============================================================================
i32 fsTell(i32 fd)
{
  return bfsTell(fd);
}

// ============================================================================
// Retrieve the current file size in bytes.  This depends on the highest offset
// written to the file, or the highest offset set with the fsSeek function.  On
// success, return the file size.  On failure, abort
// ============================================================================
i32 fsSize(i32 fd)
{
  i32 inum = bfsFdToInum(fd);
  return bfsGetSize(inum);
}

// ============================================================================
// Write 'numb' bytes of data from 'buf' into the file currently fsOpen'd on
// filedescriptor 'fd'.  The write starts at the current file offset for the
// destination file.  On success, return 0.  On failure, abort
// ============================================================================
i32 fsWrite(i32 fd, i32 numb, void *buf)
{
  i32 inum = bfsFdToInum(fd);
  i32 cursor = bfsTell(fd);

  // Read the first block value
  i8 firstBlock[BYTESPERBLOCK];
  i8 lastBlock[BYTESPERBLOCK];
  // Build the buffer that will be written using bioWrite

  // FDN
  i32 firstBlock_FBN = cursor / BYTESPERBLOCK;
  i32 lastBlock_FBN = (cursor + numb) / BYTESPERBLOCK;
  i32 curr_FBN = cursor / BYTESPERBLOCK;

  // Read 
  bfsRead(inum, cursor / BYTESPERBLOCK, firstBlock);
  if (lastBlock_FBN * BYTESPERBLOCK  < bfsGetSize(inum))
  bfsRead(inum, lastBlock_FBN, lastBlock);

  // if the written data is only taking a fraction of a block
  i32 s_dataBefore = cursor % BYTESPERBLOCK;
  i32 s_totalSize = (lastBlock_FBN - firstBlock_FBN + 1) * BYTESPERBLOCK;
  i8 finalBuf[s_totalSize];
  i32 index = 0;
  // Used to add to size

  // Move the original data from 1st block, because write might not cover all of 1st block
  memmove(finalBuf, firstBlock, s_dataBefore);

  // Move the original data from last block to lastBLock_FBN * 512
  if (lastBlock_FBN * BYTESPERBLOCK  < bfsGetSize(inum))
  memmove(finalBuf + (lastBlock_FBN - firstBlock_FBN) * BYTESPERBLOCK, lastBlock, BYTESPERBLOCK);

  // Move data to be written
  memmove(finalBuf + s_dataBefore, buf, numb);
  // Move data to be written
  while (s_totalSize > 0) {
    // Check if extension is needed (offset by 1 because size is not 0 based)
     if (curr_FBN * BYTESPERBLOCK > bfsGetSize(inum) - 1) {
       bfsExtend(inum, curr_FBN);
     }
     // convert FBN to DBN
    i32 curr_DBN = bfsFbnToDbn(inum, curr_FBN);
    // Write to DBN
    bioWrite(curr_DBN, finalBuf + index);

    // decrement total size
    s_totalSize -= BYTESPERBLOCK;
    curr_FBN++;
    index += BYTESPERBLOCK;
  }
  bfsSetCursor(inum, cursor + numb);
  // only if the final cursor has been changed
  if (cursor + numb > bfsGetSize(inum)) {
    bfsSetSize(inum, cursor + numb);
  }
  return 0;
}
