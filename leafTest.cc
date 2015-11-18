#include <list>
#include <iostream>
#include "BTreeNode.h"
#include "PageFile.h"
#include "RecordFile.h"

int main() {
    PageFile leafPage;
    leafPage.open("testIndex.txt", 'w');
    int rootid = leafPage.endPid();
    BTLeafNode leaf1(-1, rootid, -1);
    RecordId r;
    r.pid = 10; r.sid = 22;
    leaf1.insert(90, r);
    r.pid = 20; r.sid = 4;
    leaf1.insert(79, r);
    leaf1.insert(99, r);
    leaf1.write(rootid, leafPage);
    leaf1.print();

    BTLeafNode leaf2(-1, rootid, -3);
    leaf2.read(rootid, leafPage);
    leaf2.print();

    leafPage.close();

    int eid;
    int result = leaf2.locate(79, eid);
    std::cout << "eid: " << eid << " code: " << result << std::endl;
    result = leaf2.locate(1024, eid);
    std::cout << "eid: " << eid << " code: " << result << std::endl;
}
