// Antibody class file
// Melissa Danforth
// UCD ECS271 W02

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "antibody.h"
#include "random.h"
using namespace std;

// Set the maximum bits used by the attribute values
void Antibody::setMax() {
        max[COMMAND] = 4; max[PROTOCOL] = 4; max[LENGTH] = 9; max[VARIABLES] = 3;
        max[PERCENT] = 5; max[APOS] = 3; max[PLUS] = 3; max[CDOT] = 4;
        max[BACKSLASH] = 3; max[OPAREN] = 3; max[CPAREN] = 3; max[FORWARD] = 3;
        max[LT] = 3; max[GT] = 3;
}

// Initialize the attribute values randomly
void Antibody::randomInit() {
        // Not all attributes are expressed, initialize which are
        for(int i = 0; i < ALEN; i++) {
                if(randomCoin() < 0.4) flags[i] = 1;
                else flags[i] = 0;
                if(!flags[i]) { // not expressed, set to 0
                        a[i] = 0; offset[i] = 0;
                        continue;
                }
                int maxnum = (int)pow(2.0, (double)max[i]);
                a[i] = randomInt(maxnum);   // Random value for attribute
                int omax = (int)pow(2.0, (max[i] - 1.0));  // offsets 1 bit less
                offset[i] = randomInt(omax);
                //    cout << "a[" << i << "] = " << a[i] << 
                //            " o[" << i << "] = " << offset[i] << endl;
        }
        offset[COMMAND] = 0; offset[PROTOCOL] = 0;  // no offsets, discrete values
}

// Create random NULL antibody
Antibody::Antibody() {
        setMax();
        randomInit();
        resetTests();
}

// Create antibody according to given values
Antibody::Antibody(int *in, int *off, int *flg) {
        setMax();
        for(int i = 0; i < ALEN; i++) {
                a[i] = in[i];
                offset[i] = off[i];
                flags[i] = flg[i];
        }
        resetTests();
}

Antibody::Antibody(Antibody &c) {  // Copy constructor
        for(int i = 0; i < ALEN; i++) {
                max[i] = c.max[i];
                a[i] = c.a[i];
                offset[i] = c.offset[i];
                flags[i] = c.flags[i];
        }
        tests = c.tests;
        pos = c.pos;
        false_pos = c.false_pos;
        neg = c.neg;
        false_neg = c.false_neg;
        for(int i = 0; i < CLASS_COUNT; i++) {
                cat[i] = c.cat[i];
                catPerc[i] = c.catPerc[i];
        }
}

// Set all statistic values to 0
void Antibody::resetTests() {
        tests = 0;
        pos = 0;
        false_pos = 0;
        neg = 0;
        false_neg = 0;
        for(int i = 0; i < CLASS_COUNT; i++) {
                cat[i] = 0;
                catPerc[i] = 0;
        }
}

int Antibody::queryClassification() {
        float max = catPerc[0];
        int maxi = 0;

        for(int i = 1; i < CLASS_COUNT; i++) {
                if(catPerc[i] > max) {
                        max = catPerc[i];
                        maxi = i;
                }
        }

        return maxi;
}

void Antibody::calcCategory(int c, int total) {
        if(c < 0 || c > CLASS_COUNT - 1) return;
        catPerc[c] = (float)cat[c] / (float)total;
}

// Check if expressed attributes match given input
// Returns 1 if matches, 0 if not matches
int Antibody::match(int *test, const int & debug)
{
        tests++;
        if(flags[COMMAND] && !(a[COMMAND] & test[COMMAND]))
        {
                if (debug == 1)
                {
                        cerr << "\n\tantibody::match returning 0 ( - NORMAL - )\n\t\tflags[COMMAND] && !(a[COMMAND] & test[COMMAND])\n" << flush;
                        cout << "\n\tantibody::match returning 0 ( - NORMAL - )\n\t\tflags[COMMAND] && !(a[COMMAND] & test[COMMAND])\n" << flush;
                }
                return (0);
        }
        if(flags[PROTOCOL] && !(a[PROTOCOL] & test[PROTOCOL]))
        {
                if (debug == 1)
                {
                        cerr << "\n\tantibody::match returning 0 ( - NORMAL - )\n\t\tflags[PROTOCOL] && !(a[PROTOCOL] & test[PROTOCOL])\n" << flush;
                        cout << "\n\tantibody::match returning 0 ( - NORMAL - )\n\t\tflags[PROTOCOL] && !(a[PROTOCOL] & test[PROTOCOL])\n" << flush;
                }
                return (0);
        }
        for(int i = LENGTH; i < ALEN; i++)
        {
                if(!flags[i])
                {
                        continue;
                }
                int overf = (int)pow(2.0, (double)max[i]) + (int)pow(2.0, max[i] - 1.0) - 2;
                if (test[i] > overf)
                {
                        if (debug == 1)
                        {
                                cerr << "\n\tantibody::match returning 1 ( - ATTACK - )\n\t\ttest[i] > overf\n" << flush;
                                cout << "\n\tantibody::match returning 1 ( - ATTACK - )\n\t\ttest[i] > overf\n" << flush;
                        }
                        return (1);  // Over max antibody match, assume is attack
                }
                if (test[i] < (a[i] - offset[i]) || test[i] > (a[i] + offset[i]))
                {
                        if (debug == 1)
                        {
                                cerr << "\n\tantibody::match returning 0 ( - NORMAL - )\n\t\t(test[i] < (a[i] - offset[i]) || test[i] > (a[i] + offset[i]))\n" << flush;
                                cout << "\n\tantibody::match returning 0 ( - NORMAL - )\n\t\t(test[i] < (a[i] - offset[i]) || test[i] > (a[i] + offset[i]))\n" << flush;
                        }
                        return (0);
                }
        }
        if (debug == 1)
        {
                cerr << "\n\tantibody::match returning 1 ( - ATTACK - )\n\t\tDefault Case\n" << flush;
                cout << "\n\tantibody::match returning 1 ( - ATTACK - )\n\t\tDefault Case\n" << flush;
        }
        return (1);
}

// Fitness is a combination of correct positives (match == 1),
// correct negatives (match == 0) and false positives and false negatives.
// Scale is -2 to 2
//    -2 means everything is false positives and false negatives
//    +2 means absolutely no false positives or false negatives
float Antibody::fitness(int cl) {
        float p_fpos, p_fneg, p_pos, p_neg;

        // Classification fitness returns how accurately this antibody can 
        // classify the given classification cl
        if(cl >= 0) {
                int c = queryClassification();
                return (c == cl) ? catPerc[c] : 0;
        }

        // total normal tests = neg + false_pos
        if((neg + false_pos) == 0) {  // no normal tests done
                p_fpos = 0.0;
                p_neg = 0.0;
        }
        else {  // determine percentage that are labeled correctly and incorrectly
                p_fpos = (float)false_pos / (float)(neg + false_pos);
                p_neg = (float)neg / (float)(neg + false_pos);
        }

        // total attack tests = pos + false_neg
        if((pos + false_neg) == 0) { // no attack tests done
                p_fneg = 0.0;
                p_pos = 0.0;
        }
        else {  // determine percentage that are labeled correctly and incorrectly
                p_fneg = (float)false_neg / (float)(pos + false_neg);
                p_pos = (float)pos / (float)(pos + false_neg);
        }

        return p_pos + p_neg - p_fpos - p_fneg;

        // test: since no false_pos, try ranking on pure # detected attacks
        //  return (float)pos;
}

// Mating process. Given args are mate and pointers to two children
// Mating option: Children express all attributes that both parents express
//     and randomly express attributes that only one parent expresses
// Crossover occurs only on genes that are expressed
void Antibody::mate(Antibody *m, Antibody **c1, Antibody **c2) {
        int a1[ALEN], o1[ALEN], a2[ALEN], o2[ALEN], f[ALEN], bits, last = -1;
        int cnt = 0, fc = 0, mc = 0;
        unsigned int tmp1, tmp2;

        for(int i = 0; i < ALEN; i++) {
                if(flags[i]) fc++;    // Count of expressed attributes
                if(m->flags[i]) mc++; // Count of expressed attributes
                if(flags[i] && m->flags[i]) {
                        f[i] = 1;
                }
                else if(flags[i] || m->flags[i]) {
                        if(randomInt(4)) {  // 1 in 4 chance of expressing attribute
                                f[i] = 0;
                        }
                        else {
                                f[i] = 1;
                        }
                }
                else {
                        f[i] = 0;
                }
                if(f[i]) {
                        last = i;
                        cnt++;
                }
        }
        // Not enough attributes expressed. Select 1 randomly from each parent.
        while(cnt < 2) { 
                int tmp = randomInt(ALEN);
                while(f[tmp] == 1 || !(flags[tmp] == 1 || m->flags[tmp] == 1) ) {
                        tmp = randomInt(ALEN);
                }
                f[tmp] = 1;
                last = (tmp > last) ? tmp : last;
                cnt++;
        }

        int gene = randomInt(ALEN);
        while(f[gene] == 0) gene = randomInt(ALEN); // xover on expressed gene
        if(gene == COMMAND || gene == PROTOCOL) {  // these two have 0 offsets
                bits = randomInt(max[gene]);
        }
        else {
                bits = randomInt(((max[gene]*2) - 1));
        }
        if(gene == last) {    // On the last gene, we do not want the xover bits
                while(!bits) {      // to be 0, because no xover will occur then
                        bits = randomInt(((max[gene]*2) - 1));
                }
        }
        for(int i = 0; i < ALEN; i++) {
                if(i == gene) {  // crossover on this gene, (attribute, offset) pair
                        if(bits >= max[gene] - 1) {  // crossover in attribute value
                                bits -= max[gene] - 1;
                                tmp1 = 0xFFFFFFFF << bits;
                                tmp2 = (unsigned int)pow(2.0, (double)bits) - 1;
                                a1[i] = (a[i] & tmp1) + (m->a[i] & tmp2);
                                a2[i] = (a[i] & tmp2) + (m->a[i] & tmp1);
                                o1[i] = m->offset[i];
                                o2[i] = offset[i];
                        }
                        else {  // crossover in offset value
                                tmp1 = 0xFFFFFFFF << bits;
                                tmp2 = (unsigned int)pow(2.0, (double)bits) - 1;
                                a1[i] = a[i];
                                a2[i] = m->a[i];
                                o1[i] = (offset[i] & tmp1) + (m->offset[i] & tmp2);
                                o2[i] = (offset[i] & tmp2) + (m->offset[i] & tmp1);
                        }
                }
                else if(i < gene) {  // (a1,o1) = father, (a2,o2) = mother
                        a1[i] = a[i];
                        a2[i] = m->a[i];
                        o1[i] = offset[i];
                        o2[i] = m->offset[i];
                }
                else {  // (a1,o1) = mother, (a2,o2) = father
                        a1[i] = m->a[i];
                        a2[i] = a[i];
                        o1[i] = m->offset[i];
                        o2[i] = offset[i];
                }
        }

        *c1 = new Antibody(a1, o1, f);
        *c2 = new Antibody(a2, o2, f);
}

/* This does bit-wise mutations
   void Antibody::mutate() {
   int bit, gene = random() % ALEN;
   while(flags[gene] == 0) gene = random() % ALEN; // mutate expressed gene
   if(gene == COMMAND || gene == PROTOCOL) {  // these two have 0 offsets
   bit = random() % max[gene];
   }
   else {
   bit = random() % ((max[gene]*2) - 1);
   }
   if(bit > max[gene] - 2) { // flip attribute bit
   bit -= max[gene] - 1;
   int setVal = 1 << bit;
   if(a[gene] & setVal) a[gene] -= setVal;
   else a[gene] += setVal;
   }
   else {
   int setVal = 1 << bit;
   if(offset[gene] & setVal) offset[gene] -= setVal;
   else offset[gene] += setVal;
   }
   }
   */

// This does real-value mutations
void Antibody::mutate() {
        int gene = randomInt(ALEN);

        if(flags[gene] == 0) flags[gene] = 1;  // Turn on unexpressed gene
        // These two variables are binary variables with 3 bits, so flip a bit
        if(gene == COMMAND || gene == PROTOCOL) {
                int bit = randomInt(max[gene]);
                int setVal = 1 << bit;
                if(a[gene] & setVal) a[gene] -= setVal; // bit is set, turn it off
                else a[gene] += setVal;                 // bit is not set, turn it on
        }
        // The remainder are real-value variables, so add/subtract a delta
        // from the base value and/or the offset
        else {
                double coin = randomCoin();
                int shiftBase = randomInt(2);
                int shiftOffset = randomInt(4);
                if(randomCoin() < 0.5) shiftBase = -shiftBase;
                if(randomCoin() < 0.5) shiftOffset = -shiftOffset;
                if(coin > 0.8) { // alter both
                        a[gene] += shiftBase;
                        offset[gene] += shiftOffset;
                }
                else if(coin > 0.2) { // alter just the base
                        a[gene] += shiftBase;
                }
                else {  // alter just the offset
                        offset[gene] += shiftOffset;
                }
                if(a[gene] < 0) a[gene] = 0;
                if(offset[gene] < 0) offset[gene] = 0;
        }
}

string Antibody::dump (void)
{
        ostringstream s;
        for(int i = 0; i < ALEN; i++)
        {
                /* flags - is the attribute expressed */
                s << (int)(this->flags[i]);
                s << " ";
                /* attributes */
                s << (int)(this->a[i]);
                s << " ";
                /* offsets for ranges of values */
                s << (int)(this->offset[i]);
                s << " ";
                /* max values for each attrubute */
                s << (int)(this->max[i]);
                /* commas to indicate ALEN */
                s << ", ";
        }
        // Statistics for this antibody
        s << "\t";
        s << (int)(this->tests);           // Total tests performed
        s << " ";
        s << (int)(this->pos);             // Number of attacks labeled as attacks
        s << " ";
        s << (int)(this->false_pos);       // Number of normal requests labeled as attacks
        s << " ";
        s << (int)(this->neg);             // Number of normal requests labeled as normal
        s << " ";
        s << (int)(this->false_neg);       // Number of attacks labeled as normal
        s << "\t";
        for(int i = 0; i < CLASS_COUNT; i++)
        {
                s << (int)(this->cat[i]);  // Count of matches in each classification category
                /* semicolons to indicate CLASS_COUNT */
                s << "; ";
        }
        s << "\n";

        return (s.str());
}

// Output the value of the antibody
ostream &operator<<(ostream &o, Antibody &a) {
        o << "  Fit: " << a.fitness() << " (" << a.tests << " " 
                << a.pos << " " << a.false_pos << " " << a.neg << " " 
                << a.false_neg << ")";
        o << "  Cat: ";
        for(int i = 0; i < CLASS_COUNT; i++) {
                o << "(" << a.cat[i] << " " << a.catPerc[i] << ") ";
        }
        /* This prints out the binary form of each attribute. Too long
           for(int i = 0; i < ALEN; i++) {
           if(!(i % (ALEN/2))) o << endl << " ";
           if(a.flags[i]) o << "+";
           int mask = 1 << (a.max[i] - 1);
           for(int j = 0; j < a.max[i]; j++) {
           o << (a.a[i] & mask ? '1' : '0');
           mask >>= 1;
           }
           if(i < 2) {
           o << " ";
           continue;
           }
           o << "-";
           mask = 1 << (a.max[i] - 2);
           for(int j = 0; j < (a.max[i] - 1); j++) {
           o << (a.offset[i] & mask ? '1' : '0');
           mask >>= 1;
           }
           o << " ";
           }
           */
        // Print out the decimal form of each attribute
        o << endl << "     ";
        for(int i = 0; i < ALEN; i++) {
                if(a.flags[i]) o << "+";
                o << a.a[i] << "(" << a.offset[i] << ") ";
        }
        o << endl;
        return o;
}
