#ifndef __UNKNOWN_WEB_H__
#define __UNKNOWN_WEB_H__

#include <iostream>
#include "webdata.h"
using namespace std;

class unknownWeb : protected Webdata {
  private:
    int **guessedCount;

  public:
    unknownWeb(char *fn, int f = 2);
    ~unknownWeb();

    void reset();
    void setThreshold(int);
    void test(Antibody **a, int s, int f = 0, int cl = -1);

    friend ostream &operator<<(ostream &, const unknownWeb &);
};

#endif
