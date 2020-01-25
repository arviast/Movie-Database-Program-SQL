/*main.cpp*/

//
// myDB project using AVL trees
//
// <<Arvinbayar Jamsranjav>>
// U. of Illinois, Chicago
// CS 251: Fall 2019
// Project #04
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cassert>

#include "avl.h"
#include "util.h"

using namespace std;

// read data from the file, insert to my tree if column is indexed
// ...
void treeBuild(avltree<string, streamoff> &avlTree, string tablename, int keyColumn, int numOfColumns, int offset) {
    string filename = tablename + ".data";
    ifstream data(filename, ios::in | ios:: binary);
    if(!data.good()) {
        cout << "**Error: couldn't open data file '" << filename << "'." << endl;
        return;        
    }
    
    data.seekg(0, data.end);  // move to the end to get length of file:
    streamoff length = data.tellg();
    streamoff pos = 0;
    string value;
    
    while(pos < length) {
        data.seekg(pos, data.beg);
        for(int i = 0; i < numOfColumns; i++) {
            data >> value;
            if(i == keyColumn ){
                avlTree.insert(value, pos);
            }
        }
        pos += offset;
    } 
}

// tokenize
//
// Given a string, breaks the string into individual tokens (words) based
// on spaces between the tokens.  Returns the tokens back in a vector.
//
// Example: "select * from students" would be tokenized into a vector
// containing 4 strings:  "select", "*", "from", "students".
//
vector<string> tokenize(string line)
{
  vector<string> tokens;
  stringstream  stream(line);
  string token;

  while (getline(stream, token, ' '))
  {
    tokens.push_back(token);
  }

  return tokens;
}


int main()
{
  vector<string> metaFile;                  // will store all data from .meta
  vector<string> metaFileNum;               // will store all words from .meta
  vector<string> metaFileIndex;             // will store all indexes (0 or 1 ) from .meta  (for students file: 10010)
  vector<string> metaFileIndexColumn;       // will store only indexed column names (for students file: uin netid)
  vector<string> metaFileIndexLinearCol;    // will store not indexed column names (for students file: firstname lastname email)
  int treeNum = 0;                          // counter for trees
    
  string tablename; // = "students";

  cout << "Welcome to myDB, please enter tablename> ";
  getline(cin, tablename);

  cout << "Reading meta-data..." << endl;

  string filename = tablename + ".meta";
  ifstream meta(filename, ios::in | ios::binary);
  if (!meta.good())
  {
    cout << "**Error: couldn't open data file '" << filename << "'." << endl;
    return 0;
  }
  //  reading record one by one
  int recordSize;             // distance between positions
  int numOfColumns;           
  string insideMeta;          
  int pos = 0;
  meta.seekg(pos, meta.beg);  // move to start of record:
  meta >> recordSize;
  meta >> numOfColumns;
  
  meta >> insideMeta;  
  while (!meta.eof())
  {
        metaFile.push_back(insideMeta);
        meta >> insideMeta;
  }
     
  // organizing my vector into 4 pieces to compare with the tokens later
  // ...
  for(unsigned int i = 0; i < metaFile.size(); i++) {
      if(metaFile.at(i).compare("0") == 0 || metaFile.at(i).compare("1") == 0 ) {
         metaFileNum.push_back(metaFile.at(i)); 
      }
      if (metaFile.at(i).compare("1") == 0) {
          metaFileIndexColumn.push_back(metaFile.at(i-1));
      }
      if (metaFile.at(i).compare("0") == 0) {
          metaFileIndexLinearCol.push_back(metaFile.at(i-1));
      }
      else if(metaFile.at(i).compare("0") != 0 && metaFile.at(i).compare("1") != 0 )  {
          metaFileIndex.push_back(metaFile.at(i)); 
      }
  }
        
  cout << "Building index tree(s)..." << endl;    
  vector<avltree<string, streamoff>> avl(numOfColumns);  // declaring my vector of avl trees    
  // this is where trees built
  // if you find index 1 in meta
  // ...
  for(unsigned int i = 0; i < metaFileNum.size(); i++) {
      if(metaFileNum[i] == "1") {
          treeBuild(avl[treeNum], tablename, i, numOfColumns, recordSize); 
          treeNum++;
      }
  }
  
 // printing key, height, size of each tree 
 // ...  
 treeNum = 0;  // didn't want to call my tree on meta index so started new counter
 for (unsigned int i = 0; i < avl.size(); i++) {
     if (metaFileNum[i] == "1"){
         cout << "Index column: " << metaFileIndex[i] << endl;
         cout << "  Tree size: " << avl[treeNum].size() << endl;
         cout << "  Tree height: " << avl[treeNum].height() << endl;
         treeNum++;
     }
 }
   
  // start of query check  
  // ... 
  string query;
  int savedColumn = 0;                 // saved column # that we should print if there is no *  
  cout << endl;
  cout << "Enter query> ";
  getline(cin, query);
  
  while (query != "exit")
  {
    int countingCorrectColumn = 0;    // counting if token[1] and token[5] are both correct for searching
    vector<string> tokens = tokenize(query);
    // basic test case where size is wrong  
    if((tokens[0] == "select" && tokens.size() != 8)) {
        cout << "Invalid select query, ignored...\n\n";
        cout << "Enter query> ";
        getline(cin, query);
        continue;
    }
    // basic tests   
    if((tokens[0] != "select" && tokens[0] != "exit") || (tokens[2] != "from") 
          || (tokens[4] != "where") || (tokens[6] != "=") || tokens.size() != 8 ) {
        cout << "Unknown query, ignored...\n\n";
        cout << "Enter query> ";
        getline(cin, query);
        continue;
    }
    
    // if our token[1] is not equal to "*" then check if the 2 columns are correct
    // also gives error messages if token[1] and token[5] are incorrect  
    else if (tokens[1] != "*") {
        bool found = false;
        // for token[1]
        for (unsigned int i = 0; i < metaFileIndex.size(); i++) {
            if(tokens[1] == metaFileIndex[i]) {
                savedColumn = i;
                countingCorrectColumn++;
                found = true;
                break;
            }
        }
        if(!found){
            cout << "Invalid select column, ignored...\n\n";
            cout << "Enter query> ";
            getline(cin, query);
            continue;
        }
        // for token[5]
        found = false;
        for (unsigned int i = 0; i < metaFileIndex.size(); i++) {
            if(tokens[5] == metaFileIndex[i]) {
                countingCorrectColumn++; 
                found = true;
                break;
            }
        }
        if(!found){
            cout << "Invalid where column, ignored...\n\n";
            cout << "Enter query> ";
            getline(cin, query);
            continue;
        }  
    }
    // check tablename in query is correct as choosen one in the beginning of the while loop            
    if (tokens[3] != tablename) {
        cout << "Invalid table name, ignored...\n\n";
        cout << "Enter query> ";
        getline(cin, query);
        continue;
    }   
    
    // printing part 1
    // if we are not using "*"  
    if (countingCorrectColumn == 2 && tokens[1] != "*") {
        // using tree search
        // ...
        for(unsigned int i = 0; i < metaFileIndexColumn.size(); i++) {
            if(tokens[5] == metaFileIndexColumn[i]) {
               streamoff *pos = avl[i].search(tokens[7]);
               // if position doesnt exist
               // ...
               if(pos == nullptr) {
                   cout << "Not found...\n";
                   break;
                   
               }  
               vector<string> tempVector;
               tempVector = GetRecord(tablename, *pos, numOfColumns);
               cout << tokens[1] << ": " << tempVector[savedColumn] << endl;
            }
        }
        
        // using linear search
        // ...
        for(unsigned int i = 0; i < metaFileIndexLinearCol.size(); i++){
            if(tokens[5] == metaFileIndexLinearCol[i]) {
                for (unsigned int i = 0; i < metaFileIndex.size(); i++) {
                    if(tokens[5] == metaFileIndex[i]) {
                        vector<streamoff> tempPositions;
                        vector<string> storeValues;
                        tempPositions = LinearSearch(tablename, recordSize, numOfColumns, tokens[7], (i+1));
                        // some data's has multiple same keys
                        // displaying all of them
                        for (unsigned int i = 0; i < tempPositions.size(); i++){
                            storeValues = GetRecord(tablename, tempPositions[i], numOfColumns);
                            cout << tokens[1] << ": " << storeValues[savedColumn] << endl; 
                        }
                        // if my vector empty then data not found
                        if(tempPositions.size() == 0) {
                           cout << "Not found...\n";
                           break;
                        }                    
                    }
                }
            }
        } 
    }  
    
    // printing part 2
    // if we are using "*"  
    // ...
    if (tokens[1] == "*") { 
        bool found1 = false;
        // using tree search
        for(unsigned int i = 0; i < metaFileIndexColumn.size(); i++) {
            if(tokens[5] == metaFileIndexColumn[i]) {
                streamoff *pos = avl[i].search(tokens[7]);
                found1 = true;
                // if position pointer null it means we didnt find anything
                if(pos == NULL) {
                    cout << "Not found...\n";
                    break;
                }
                vector<string> tempValue;
                tempValue = GetRecord(tablename, *pos, numOfColumns);
                // for * display all the column info
                for(unsigned int i = 0; i < metaFileIndex.size(); i++) {
                    cout << metaFileIndex[i] << ": " << tempValue[i] << endl;
                }
            }
        }
        
        // using linear search
        for(unsigned int i = 0; i < metaFileIndexLinearCol.size(); i++) {
            if(tokens[5] == metaFileIndexLinearCol[i]) {
                    for(unsigned int i = 0; i < metaFileIndex.size(); i++) {
                        if (tokens[5] == metaFileIndex[i]) {
                           found1 = true;
                           vector<streamoff> tempPositions2;
                           tempPositions2 = LinearSearch(tablename, recordSize, numOfColumns, tokens[7], (i+1));
                           vector<string> storeValues2;
                           // get all the data 
                           for(unsigned int i = 0; i < tempPositions2.size(); i++) {
                               storeValues2 = GetRecord(tablename, tempPositions2[i], numOfColumns);
                               // print all the data
                               for(unsigned int i = 0; i < metaFileIndex.size(); i++) {
                                   cout << metaFileIndex[i] << ": " << storeValues2[i] << endl;
                               }
                           }
                           // if vector of positions is empty then not found
                           if(tempPositions2.size() == 0) {
                               cout << "Not found...\n";
                               break;
                           } 
                        }
                    }
            }
        }
        // if all above skipped means you didnt find correct tokens[5]
        // and displays error
        if(!found1) {
            cout << "Invalid where column, ignored...\n\n";
            cout << "Enter query> ";
            getline(cin, query);
            continue;
        }
    }       
    cout << endl;
    cout << "Enter query> ";
    getline(cin, query);
  }

  //
  // done:
  //
  return 0;
}
