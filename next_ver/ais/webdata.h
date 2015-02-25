#ifndef __WEB_DATA_H__
#define __WEB_DATA_H__

#include <iostream>
#include <fstream>
#include <cstring>
#include "antibody.h"
using namespace std;

const int MAX_FILENAME = 500;

class Webdata {
  protected:
    char **files;
    int **input;   // Attributes for each file
    int *results;  // for attack data, results counts number of antibodies
                   //   that correctly labelled files[i] as an attack
                   // for normal data, results counts number of antibodies
                   //   that incorrectly labelled files[i] as an attack
    int *tested;   // flag to indicate that files[i] was tested for normal

    // Classification related parameters
    int *labels;   // The index number for this class label
    int labelCount[CLASS_COUNT];  // Count for each class in dataset

    int flag;      // 1 == attack data, 0 == normal data, 2 == unknown data
    int lines;     // count of files in dataset
    int threshold; // Number of antibodies required for a request to be
                   // labelled an attack

    unsigned int translateCommand(char *);
    unsigned int translateProtocol(float);
    int translateLabel(char *);
  public:
    Webdata(char *, int);
    ~Webdata();
    float labelAccuracy[CLASS_COUNT]; // Accuracy for each class in dataset

    void test(Antibody **a, int s, int f = 0, int cl = -1);
    void test(Antibody *a, int f = 0, int cl = -1);
    void setThreshold(int f) { threshold = f; }
    float queryMissed();
    int queryMissedCounts();
    void resetStats();
    void printMissed(ostream &);
    friend ostream &operator<<(ostream &, Webdata &);
};

#endif
