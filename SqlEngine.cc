/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <climits>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  bool condOnKeyEquality = false;
  int keyMatch;
  bool condOnKeyRange = false;
  int keyMin = INT_MIN;
  int keyMax = INT_MAX;
  for (unsigned i = 0; i < cond.size(); i++) {
    if (cond[i].attr == 1) {
      int val;
      switch (cond[i].comp) {
      case SelCond::EQ:
        condOnKeyEquality = true;
        keyMatch = atoi(cond[i].value);
        break;
      case SelCond::GT:
        condOnKeyRange = true;
        val = atoi(cond[i].value);
        if (val+1 > keyMin) keyMin = val+1;
        break;
      case SelCond::LT: 
        condOnKeyRange = true;
        val = atoi(cond[i].value);
        if (val-1 < keyMax) keyMax = val-1;
        break;
      case SelCond::GE:
        condOnKeyRange = true;
        val = atoi(cond[i].value);
        if (val > keyMin) keyMin = val;
        break;
      case SelCond::LE:
        condOnKeyRange = true;
        val = atoi(cond[i].value);
        if (val < keyMax) keyMax = val;
        break;
      default:
        break;
      }
    }
  }

  BTreeIndex tree;
  bool tryTree = false;
  if (condOnKeyEquality || condOnKeyRange || (cond.size() == 0 && (attr == 1 || attr ==4))) {
    rc = tree.open(table + ".idx", 'r');
    tryTree = true;
  }
  // B+ tree opened successfully, use this index for searching
  if (tryTree && rc == 0) {
    tree.readRoot();
    IndexCursor entry;
    count = 0;
    if (condOnKeyEquality) {
      rc = tree.locate(keyMatch, entry);
      if (rc < 0 && rc != RC_NO_SUCH_RECORD) {
        fprintf(stderr, "Error locating searchKey in B+ tree\n");
        goto exit_index_select;
      }
      else if ((rc = tree.readForward(entry, key, rid)) < 0) {
        fprintf(stderr, "Error reading forward long B+ tree leaf\n");
        goto exit_index_select;
      }
      else {
        bool ridRead = false;
        for (unsigned i = 0; i < cond.size(); i++) {
          switch(cond[i].attr) {
          case 1:
            diff = key - atoi(cond[i].value);
            break;
          case 2:
            if (ridRead == false) {
              if ((rc = rf.read(rid, key, value)) < 0) {
                fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
                goto exit_index_select;
              }
              ridRead = true;
            }
            diff = strcmp(value.c_str(), cond[i].value);
            break;
          }

          switch (cond[i].comp) {
          case SelCond::EQ:
            if (diff != 0) goto section_end;
            break;
          case SelCond::NE:
            if (diff == 0) goto section_end;
            break;
          case SelCond::GT:
            if (diff <= 0) goto section_end;
            break;
          case SelCond::LT:
            if (diff >= 0) goto section_end;
            break;
          case SelCond::GE:
            if (diff < 0) goto section_end;
            break;
          case SelCond::LE:
            if (diff > 0) goto section_end;
            break;
          }
        }
        count++;
        switch (attr) {
        case 1:
          fprintf(stdout, "%d\n", key);
          break;
        case 2:
          if (ridRead == false) {
            if ((rc = rf.read(rid, key, value)) < 0) {
              fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
              goto exit_index_select;
            }
            ridRead = true;
          }
          fprintf(stdout, "%s\n", value.c_str());
          break;
        case 3:
          if (ridRead == false) {
            if ((rc = rf.read(rid, key, value)) < 0) {
              fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
              goto exit_index_select;
            }
            ridRead = true;
          }
          fprintf(stdout, "%d '%s'\n", key, value.c_str());
          break;
        }

        section_end: ;
      }
    }
    else {
      rc = tree.locate(keyMin, entry);
      if (rc < 0 && rc != RC_NO_SUCH_RECORD) {
        fprintf(stderr, "Error locating searchKey in B+ tree\n");
        goto exit_index_select;
      }
      if ((rc = tree.readForward(entry, key, rid)) < 0) {
        fprintf(stderr, "Error reading forward along B+ tree leaf\n");
        goto exit_index_select;
      }
      while (key <= keyMax) {
        bool ridRead = false;
        for (unsigned i = 0; i < cond.size(); i++) {
          switch(cond[i].attr) {
          case 1:
            diff = key - atoi(cond[i].value);
            break;
          case 2:
            if (ridRead == false) {
              if ((rc = rf.read(rid, key, value)) < 0) {
                fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
                goto exit_index_select;
              }
              ridRead = true;
            }
            diff = strcmp(value.c_str(), cond[i].value);
            break;
          }

         switch (cond[i].comp) {
          case SelCond::EQ:
            if (diff != 0) goto index_next_tuple;
            break;
          case SelCond::NE:
            if (diff == 0) goto index_next_tuple;
            break;
          case SelCond::GT:
            if (diff <= 0) goto index_next_tuple;
            break;
          case SelCond::LT:
            if (diff >= 0) goto index_next_tuple;
            break;
          case SelCond::GE:
            if (diff < 0) goto index_next_tuple;
            break;
          case SelCond::LE:
            if (diff > 0) goto index_next_tuple;
            break;
          }
        }
        count++;
        switch (attr) {
        case 1:
          fprintf(stdout, "%d\n", key);
          break;
        case 2:
          if (ridRead == false) {
            if ((rc = rf.read(rid, key, value)) < 0) {
              fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
              goto exit_index_select;
            }
            ridRead = true;
          }
          fprintf(stdout, "%s\n", value.c_str());
          break;
        case 3:
          if (ridRead == false) {
            if ((rc = rf.read(rid, key, value)) < 0) {
              fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
              goto exit_index_select;
            }
            ridRead = true;
          }
          fprintf(stdout, "%d '%s'\n", key, value.c_str());
          break;
        }

        index_next_tuple:
        rc = tree.readForward(entry, key, rid);
        if (rc == RC_END_OF_TREE) {
          break;
        }
        else if (rc < 0) {
          fprintf(stderr, "Error reading forward long B+ tree leaf\n");
          goto exit_index_select;
        }
      }
    }
    if (attr == 4) {
      fprintf(stdout, "%d\n", count);
    }
    rc = 0;

    exit_index_select:
    rf.close();
    tree.close();
    return rc;
  }
  // Otherwise, use default sequential scan
  else {
    // scan the table file from the beginning
    rid.pid = rid.sid = 0;
    count = 0;
    while (rid < rf.endRid()) {
      // read the tuple
      if ((rc = rf.read(rid, key, value)) < 0) {
        fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
        goto exit_select;
      }

      // check the conditions on the tuple
      for (unsigned i = 0; i < cond.size(); i++) {
        // compute the difference between the tuple value and the condition value
        switch (cond[i].attr) {
        case 1:
          diff = key - atoi(cond[i].value);
          break;
        case 2:
          diff = strcmp(value.c_str(), cond[i].value);
          break;
        }

        // skip the tuple if any condition is not met
        switch (cond[i].comp) {
        case SelCond::EQ:
          if (diff != 0) goto next_tuple;
          break;
        case SelCond::NE:
          if (diff == 0) goto next_tuple;
          break;
        case SelCond::GT:
          if (diff <= 0) goto next_tuple;
          break;
        case SelCond::LT:
          if (diff >= 0) goto next_tuple;
          break;
        case SelCond::GE:
          if (diff < 0) goto next_tuple;
          break;
        case SelCond::LE:
          if (diff > 0) goto next_tuple;
          break;
        }
      }

      // the condition is met for the tuple. 
      // increase matching tuple counter
      count++;

      // print the tuple 
      switch (attr) {
      case 1:  // SELECT key
        fprintf(stdout, "%d\n", key);
        break;
      case 2:  // SELECT value
        fprintf(stdout, "%s\n", value.c_str());
        break;
      case 3:  // SELECT *
        fprintf(stdout, "%d '%s'\n", key, value.c_str());
        break;
      }

      // move to the next tuple
      next_tuple:
      ++rid;
    }

    // print matching tuple count if "select count(*)"
    if (attr == 4) {
      fprintf(stdout, "%d\n", count);
    }
    rc = 0;

    // close the table file and return
    exit_select:
    rf.close();
    return rc;
  }
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
    ifstream file;
    file.open(loadfile.c_str());
    if (!file.is_open())
        exit(RC_FILE_OPEN_FAILED);
    string line;

    //Open target RecordFile
    RecordFile rf;
    const string recordName = table + ".tbl";
    rf.open(recordName, 'w');

    if (index) {
        // Open target index file
        BTreeIndex tree;
        const string treeName = table + ".idx";
        tree.open(treeName, 'w');
        tree.initializeTree();
        tree.readRoot();
        int inserted = 0;

        //For each file line extract value and key, insert into table
        while (getline(file, line))
        {
            int key;
            string value;
            //If parseLoadLine returns error
            if (parseLoadLine(line, key, value) < 0 ) {
                rf.close();
                tree.close();
                exit(RC_FILE_SEEK_FAILED);
            }
            
            //Insert each value and key into the RecordFile table
            RecordId rid;
            if (rf.append(key, value, rid) < 0) {
                rf.close();
                tree.close();
                exit(RC_FILE_WRITE_FAILED);
            }

            RC errorCode = tree.insert(key, rid);
            if (errorCode < 0) {
                rf.close();
                tree.close();
                exit(RC_FILE_WRITE_FAILED);
            }
            inserted++;
        }

        tree.close();
    }
    else {
        //For each file line extract value and key, insert into table
        while (getline(file, line))
        {
            int key;
            string value;
            //If parseLoadLine returns error
            if (parseLoadLine(line, key, value) < 0 ) {
                rf.close();
                exit(RC_FILE_SEEK_FAILED);
            }
            
            //Insert each value and key into the RecordFile table
            RecordId rid;
            if (rf.append(key, value, rid) < 0) {
                rf.close();
                exit(RC_FILE_WRITE_FAILED);
            }
        }
    }
    rf.close();
    return 0;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
