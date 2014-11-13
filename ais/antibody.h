#ifndef __ANTIBODY_H__
#define __ANTIBODY_H__

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

// HTTP request antibody.
// Some special notes on bit-maps:
//    For the command, the bitmap is as follows:
//       GET is bit 0, POST is bit 1, HEAD is bit 2, other is bit 3
//       A file that allowed only GET and POST would have value = 3
//    For protocol, bitmap is:
//       0.9 is bit 0, 1.0 is bit 1, 1.1 is bit 2, other is bit 3
//    All other attributes are integer ranges

const int ALEN = 14;

// Classifications of the attack. There are 6 categories. Each attack in the
// data file is labeled with one category. For every attack this antibody
// matches, it will track the associated categories.
const int CLASS_COUNT = 6;
const char CLASS_LABELS[CLASS_COUNT][15] = { "info", "traversal", "sql", "buffer", "script", "xss" };

class Antibody {
        private:
                static const int COMMAND = 0;     // HTTP command (GET, POST, HEAD, other)
                static const int PROTOCOL = 1;    // HTTP protocol (0.9, 1.0, 1.1, other)
                static const int LENGTH = 2;      // Length of request
                static const int VARIABLES = 3;   // Number of input variables
                static const int PERCENT = 4;     // Number of % chars in request
                static const int APOS = 5;        // Number of ' chars in request
                static const int PLUS = 6;        // Number of + chars in request
                static const int CDOT = 7;        // Number of .. chars in request
                static const int BACKSLASH = 8;   // Number of \ chars in request
                static const int OPAREN = 9;      // Number of ( chars in request
                static const int CPAREN = 10;     // Number of ) chars in request
                static const int FORWARD = 11;    // Number of // chars in request
                static const int LT = 12;         // Number of < chars in request
                static const int GT = 13;         // Number of > chars in request

                // Antibody characteristics
                int flags[ALEN];     // Is this attribute being expressed in this antibody?
                int a[ALEN];         // Attributes, indexed by const ints above
                int offset[ALEN];    // Offsets on attributes, to do ranges of values
                int max[ALEN];       // Maximum values for each attribute

                // Statistics for this antibody
                int tests;           // Total tests performed
                int pos;             // Number of attacks labeled as attacks
                int false_pos;       // Number of normal requests labeled as attacks
                int neg;             // Number of normal requests labeled as normal
                int false_neg;       // Number of attacks labeled as normal

                int cat[CLASS_COUNT];  // Count of matches in each classification category
                float catPerc[CLASS_COUNT];

                void setMax();       // Initialize the max values
                void randomInit();   // Randomize the attribute/offset values
        public:
                Antibody();                     // Create a random antibody
                Antibody(int *, int *, int *);  // Antibody w/ given attributes
                Antibody(Antibody &);

                int match(int *);    // Returns 1 = attack, 0 = normal
                float fitness(int cl = -1);
                void mate(Antibody *, Antibody **, Antibody **);
                void mutate();

                int queryTests()    { return tests; }
                int queryPos()      { return pos; }
                int queryFalsePos() { return false_pos; }
                int queryNeg()      { return neg; }
                int queryFalseNeg() { return false_neg; }
                int queryClassification();
                void resetTests();
                void adjPos()      { pos++; }
                void adjFalsePos() { false_pos++; }
                void adjNeg()      { neg++; }
                void adjFalseNeg() { false_neg++; }
                void adjCategory(int i) { if(i > -1) cat[i]++; }
                void calcCategory(int, int);

                friend ostream &operator<<(ostream &o, Antibody &a);

                void setCatPerc(const int &pos, const float &val)
                {
                        if (pos < 0 || pos >= CLASS_COUNT)
                        {
                                return;
                        }
                        catPerc[pos] = val;
                }

                void setCat(const int &pos, const int &val)
                {
                        if (pos < 0 || pos >= CLASS_COUNT)
                        {
                                return;
                        }
                        cat[pos] = val;
                }

                void setPos(const int &val)
                {
                        pos = val;
                }

                void setFalsePos(const int &val)
                {
                        false_pos = val;
                }

                void setNeg(const int &val)
                {
                        neg = val;
                }

                void setFalseNeg(const int &val)
                {
                        false_neg = val;
                }

                void setTests(const int &val)
                {
                        tests = val;
                }

                void setAttr(const int &pos, const int &val)
                {
                        if (pos < 0 || pos >= ALEN)
                        {
                                return;
                        }
                        a[pos] = val;
                }

                void setOffset(const int &pos, const int &val)
                {
                        if (pos < 0 || pos >= ALEN)
                        {
                                return;
                        }
                        offset[pos] = val;
                }

                void setMax(const int &pos, const int &val)
                {
                        if (pos < 0 || pos >= ALEN)
                        {
                                return;
                        }
                        max[pos] = val;
                }

                void setFlag(const int &pos, const int &val)
                {
                        if (pos < 0 || pos >= ALEN)
                        {
                                return;
                        }
                        flags[pos] = val;
                }
                string dump(void);
};

#endif
