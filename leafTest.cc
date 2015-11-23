#include <list>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "BTreeNode.h"
#include "BTreeIndex.h"
#include "PageFile.h"
#include "RecordFile.h"

int main() {
    srand(time(NULL));

    // Test for creating
    /*BTreeIndex tree;
    tree.open("indextest.txt", 'w');
    tree.initializeTree();
    tree.readRoot();
    RecordId rec;
    for(int i = 0; i < 27; i++) {
        rec.pid = rand() % 1000;
        rec.sid = rand() % 1000;
        tree.insert(rand() % 10000, rec);
    }
    tree.print();
    tree.close();*/

    // Test for loading
    BTreeIndex tree;
    tree.open("testing.idx", 'r');
    tree.readRoot();
    tree.print();
    tree.close();
	
    /*PageFile leafPage;
    leafPage.open("testIndex.txt", 'w');
    int rootid = leafPage.endPid();
    BTLeafNode leaf1(rootid);
    RecordId r;
    r.pid = 10; r.sid = 22;
    leaf1.insert(90, r);
    r.pid = 20; r.sid = 4;
    leaf1.insert(79, r);
    leaf1.insert(99, r);
    leaf1.write(rootid, leafPage);
    leaf1.print();

    BTLeafNode leaf2(rootid);
    leaf2.read(rootid, leafPage);
    leaf2.print();

    BTNonLeafNode nl1(leafPage.endPid());
    nl1.initializeRoot(leaf1.getPageId(), 10, leaf2.getPageId());
    nl1.insert(11, 55);
    nl1.insert(54, 82);
    nl1.insert(202, 41);
    nl1.insert(73, 29);
    nl1.print();
    std::cout << "\n" << std::endl;
    BTNonLeafNode nl2(leafPage.endPid());
    int midKey;
    nl1.insertAndSplit(85, 12, nl2, midKey);
    nl1.print();
    nl2.print();
    std::cout << "Midkey: " << midKey << std::endl;

    r.pid = 7; r.sid = 13;
    leaf1.insert(9, r);
    r.pid = 133; r.sid = 90;
    leaf1.insert(11, r);
    BTLeafNode leaf3(leafPage.endPid());
    r.pid = 37; r.sid = 30;
    leaf1.print();
    std::cout << "\n" << std::endl;
    int siblingKey;
    leaf1.insertAndSplit(12, r, leaf3, siblingKey);
    leaf1.print();
    leaf3.print();

    leafPage.close();*/

    /*int eid;
    int result = leaf2.locate(79, eid);
    std::cout << "eid: " << eid << " code: " << result << std::endl;
    result = leaf2.locate(1024, eid);
    std::cout << "eid: " << eid << " code: " << result << std::endl;*/
}
