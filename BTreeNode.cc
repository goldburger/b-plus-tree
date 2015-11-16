#include <list>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include "BTreeNode.h"

using namespace std;

#define MAX_KEYS 75

void reportErrorExit(RC error) {
    printf("Error! Received RC code%d\n", error);
    exit(error);
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{
    RC errorCode = pf.read(pid, buffer);
    if (errorCode < 0)
        reportErrorExit(errorCode);

    int bufferIndex = 0;
    memcpy(&isLeaf, buffer + bufferIndex, sizeof(int));
    bufferIndex += sizeof(int);
    memcpy(&length, buffer + bufferIndex, sizeof(int));
    bufferIndex += sizeof(int);
    for (int i = 0; i < length; i++) {
        RecordId nextRecord;
        memcpy(&nextRecord, buffer + bufferIndex, sizeof(RecordId));
        bufferIndex += sizeof(RecordId);
        records.push_back(nextRecord);
        int nextKey;
        memcpy(&nextKey, buffer + bufferIndex, sizeof(int));
        bufferIndex += sizeof(int);
        keys.push_back(nextKey);
    }
    memcpy(&parent, buffer + bufferIndex, sizeof(PageId));
    bufferIndex += sizeof(PageId);
    memcpy(&nextLeaf, buffer + bufferIndex, sizeof(PageId));
    return 0;
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{
    memset(buffer, 0, sizeof(char) * PageFile::PAGE_SIZE);
    int bufferIndex = 0;
    memcpy(buffer + bufferIndex, &isLeaf, sizeof(int));
    bufferIndex += sizeof(int);
    memcpy(buffer + bufferIndex, &length, sizeof(int));
    bufferIndex += sizeof(int);
    std::list<RecordId>::iterator recIt = records.begin();
    std::list<int>::iterator keyIt = keys.begin();
    for (int i = 0; i < length; i++) {
        memcpy(buffer + bufferIndex, &*recIt, sizeof(RecordId));
        bufferIndex += sizeof(RecordId);
        memcpy(buffer + bufferIndex, &*keyIt, sizeof(int));
        bufferIndex += sizeof(int);
        recIt++;
        keyIt++;
    }
    memcpy(buffer + bufferIndex, &parent, sizeof(PageId));
    bufferIndex += sizeof(PageId);
    memcpy(buffer + bufferIndex, &nextLeaf, sizeof(PageId));
    bufferIndex += sizeof(PageId);
    RC errorCode = pf.write(pid, buffer);
    if (errorCode < 0)
        reportErrorExit(errorCode);
    return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
    return length;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
    if (length >= MAX_KEYS)
        return RC_NODE_FULL;
    else
    {
        int index = 0;
        std::list<int>::iterator it;
        for(it = keys.begin(); it != keys.end(); it++) {
            if (*it < key) {
                index++;
            }
            else
                break;
        }
        keys.insert(it, key);
        std::list<RecordId>::iterator recIt;
        for (int i = 0; i < index; i++) {
            recIt++;
        }
        RecordId newRec;
        newRec.pid = rid.pid;
        newRec.sid = rid.sid;
        records.insert(recIt, newRec);
        length++;
    }
    return 0;
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ return 0; }

/**
 * If searchKey exists in the node, set eid to the index entry
 * with searchKey and return 0. If not, set eid to the index entry
 * immediately after the largest index key that is smaller than searchKey,
 * and return the error code RC_NO_SUCH_RECORD.
 * Remember that keys inside a B+tree node are always kept sorted.
 * @param searchKey[IN] the key to search for.
 * @param eid[OUT] the index entry number with searchKey or immediately
                   behind the largest key smaller than searchKey.
 * @return 0 if searchKey is found. Otherwise return an error code.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ return 0; }

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
    // Note: node entries are indexed starting from zero, length starts from 1
    if (eid >= length)
        return RC_NO_SUCH_RECORD;
    
    std::list<int>::iterator keyIt = keys.begin();
    std::list<RecordId>::iterator recIt = records.begin();
    for (int i = 0; i < eid; i++) {
        keyIt++;
        recIt++;
    }
    key = *keyIt;
    rid = *recIt;
    return 0;
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{
    return nextLeaf;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
    nextLeaf = pid;
    return 0;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
    // Assumes pf is opened before & closed after by caller, as this needs file name
    // Is this assumption valid?
    RC errorCode = pf.read(pid, buffer);
    if (errorCode < 0)
        reportErrorExit(errorCode);

    int bufferIndex = 0;
    memcpy(&isLeaf, buffer + bufferIndex, sizeof(int));
    bufferIndex += sizeof(int);
    memcpy(&length, buffer + bufferIndex, sizeof(int));
    bufferIndex += sizeof(int);
    memcpy(&leftMostPageId, buffer + bufferIndex, sizeof(PageId));
    bufferIndex += sizeof(PageId);
    for (int i = 0; i < length; i++) {
        PageId nextPage;
        memcpy(&nextPage, buffer + bufferIndex, sizeof(PageId));
        bufferIndex += sizeof(PageId);
        pages.push_back(nextPage);
        int nextKey;
        memcpy(&nextKey, buffer + bufferIndex, sizeof(int));
        bufferIndex += sizeof(int);
        keys.push_back(nextKey);
    }
    memcpy(&parent, buffer + bufferIndex, sizeof(PageId));
    bufferIndex += sizeof(PageId);
    return 0;
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
    memset(buffer, 0, sizeof(PageFile::PAGE_SIZE * sizeof(char)));    
    int bufferIndex = 0;
    memcpy(buffer + bufferIndex, &isLeaf, sizeof(int));
    bufferIndex += sizeof(int);
    memcpy(buffer + bufferIndex, &length, sizeof(int));
    bufferIndex += sizeof(int);
    memcpy(buffer + bufferIndex, &leftMostPageId, sizeof(PageId));
    bufferIndex += sizeof(PageId);
    std::list<PageId>::iterator pageIt = pages.begin();
    std::list<int>::iterator keyIt = keys.begin();
    for (int i = 0; i < length; i++) {
        memcpy(buffer + bufferIndex, &*pageIt, sizeof(PageId));
        bufferIndex += sizeof(PageId);
        memcpy(buffer + bufferIndex, &*keyIt, sizeof(int));
        bufferIndex += sizeof(int);
        pageIt++;
        keyIt++;
    }
    memcpy(buffer + bufferIndex, &parent, sizeof(PageId));
    bufferIndex += sizeof(PageId);
    RC errorCode = pf.write(pid, buffer);
    if (errorCode < 0)
        reportErrorExit(errorCode);
    return 0;

}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
    return length;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ return 0; }

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ return 0; }

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ 
    int keyIndex = 0;
    std::list<int>::iterator keyIt = keys;
    std::list<PageId>::iterator pageIt = pages;

    if (searchKey < *keyIt)
        pid = leftMostPageId;
    else
    {
        while (keyIt != keys.end() && searchKey >= *keyIt)
        {
            keyIt++;
            pageIt++;
        }
        pageIt--; //since leftmostPid is not in the vector the page iterator will always be one greater. 
        pid = pageIt;
    }
    return 0;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ 
    //Clear buffer
    memset(buffer, 0, sizeof(PageFile::PAGE_SIZE * sizeof(char)));
    pages.clear();
    keys.clear();

    isLeaf = 0;
    length = 1;
    leftMostPageId = pid1;
    keys.insert(key);
    pages.insert(pid2);
    
    //Where do we write this to?     
    return 0;
}
