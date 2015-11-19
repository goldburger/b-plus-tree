/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <string.h>
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

#define ROOT_STORAGE_BLOCK 0
#define NO_NEXT_LEAF -1

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
}

RC BTreeIndex::writeRoot()
{
    char buffer[PageFile::PAGE_SIZE];
    memset(buffer, 0, sizeof(char) * PageFile::PAGE_SIZE);
    memcpy(buffer, &rootPid, sizeof(PageId));
    return pf.write(ROOT_STORAGE_BLOCK, buffer);
}

RC BTreeIndex::readRoot()
{
    char buffer[PageFile::PAGE_SIZE];
    memset(buffer, 0, sizeof(char) * PageFile::PAGE_SIZE);
    RC errorCode = pf.read(ROOT_STORAGE_BLOCK, buffer);
    if (errorCode < 0)
        return errorCode;
    memcpy(&rootPid, buffer, sizeof(PageId));
    return 0;
}

// Used when first creating the index file after LOAD command
RC BTreeIndex::initializeTree()
{
    writeRoot(); // Used to fill 0th block of index file
    BTLeafNode rootLeaf(pf.endPid());
    rootPid = rootLeaf.getPageId();
    writeRoot();
    return rootLeaf.write(rootLeaf.getPageId(), pf);
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
    return pf.open(indexname, mode);
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    return pf.close();
}

RC BTreeIndex::insertRecursive(BTNonLeafNode& node, int key, const RecordId& rid)
{

}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    char buffer[PageFile::PAGE_SIZE];
    memset(buffer, 0, sizeof(char) * PageFile::PAGE_SIZE);
    RC errorCode = pf.read(rootPid, buffer);
    if (errorCode < 0)
        return errorCode;
    int isLeaf;
    memcpy(&isLeaf, buffer, sizeof(int));
    if (isLeaf) {
        BTLeafNode leaf(rootPid);
        leaf.read(rootPid, pf);
        // Attempt direct insertion
        errorCode = leaf.insert(key, rid);
        // If insertion fails, do insertAndSplit, then create a new root
        if (errorCode == RC_NODE_FULL) {
            BTLeafNode sibling(pf.endPid());
            int siblingKey;
            errorCode = insertAndSplit(key, rid, sibling, siblingKey);
            if (errorCode != 0)
                return errorCode;
            leaf.write(leaf.getPageId(), pf);
            sibling.write(sibling.getPageId(), pf);
            BTNonLeafNode newRoot(pf.endPid());
            newRoot.initializeRoot(leaf.getPageId(), siblingKey, sibling.getPageId());
            rootPid = newRoot.getPageId();
            newRoot.write(rootPid, pf);
            writeRoot();
            return 0;
        }
        else
            return errorCode;
    }
    else {
        BTNonLeafNode nonLeaf(rootPid);
        nonLeaf.read(rootPid, pf);
        /* Call recursive function; case for new root here */ 
    }
}

/**
 * Run the standard B+Tree key search algorithm and identify the
 * leaf node where searchKey may exist. If an index entry with
 * searchKey exists in the leaf node, set IndexCursor to its location
 * (i.e., IndexCursor.pid = PageId of the leaf node, and
 * IndexCursor.eid = the searchKey index entry number.) and return 0.
 * If not, set IndexCursor.pid = PageId of the leaf node and
 * IndexCursor.eid = the index entry immediately after the largest
 * index key that is smaller than searchKey, and return the error
 * code RC_NO_SUCH_RECORD.
 * Using the returned "IndexCursor", you will have to call readForward()
 * to retrieve the actual (key, rid) pair from the index.
 * @param key[IN] the key to find
 * @param cursor[OUT] the cursor pointing to the index entry with
 *                    searchKey or immediately behind the largest key
 *                    smaller than searchKey.
 * @return 0 if searchKey is found. Othewise an error code
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
    return 0;
}
