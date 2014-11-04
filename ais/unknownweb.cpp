
#include "unknownweb.h"
#include <iomanip>
using namespace std;

unknownWeb::unknownWeb(char *fn, int f) : Webdata(fn, f) {
        guessedCount = new int*[lines];
        for(int i = 0; i < lines; i++) {
                guessedCount[i] = new int[CLASS_COUNT];
        }
}

unknownWeb::~unknownWeb() {
        for(int i = 0; i < lines; i++) {
                delete [] guessedCount[i];
        }
        delete [] guessedCount;
}

void unknownWeb::reset() {
        Webdata::resetStats();
        for(int i = 0; i < lines; i++) {
                for(int c = 0; c < CLASS_COUNT; c++) {
                        guessedCount[i][c] = 0;
                }
        }
}

/* HERE make this a live data test function that takes an int * and tests against that, returning/logging/printing result 
 * what is 'tested'? 'results'? testing live traffic, may overflow an integer */
void unknownWeb::test(Antibody **a, int s, int f, int cl, const int * sig) {
        if (sig)
        {
                for(int i = 0; i < lines; i++) {
                        for(int j = 0; j < s; j++) { // Run it against good antibodies in this class
                                // fitness() with no argument computes how reliably this antibody can
                                // distinguish normal from attack. Skip over unreliable antibodies.
                                if(a[j]->fitness() <= 0.5) continue;
                                tested[i]++;
                                int ret = a[j]->match(&sig[i]);
                                int c = a[j]->queryClassification();
                                if(ret == -1) continue;
                                if(ret == 1) { // labeled as attack
                                        results[i]++;
                                        if(c == cl) guessedCount[i][c]++; // in correct class for population
                                }
                        }
                }
        }
}

void unknownWeb::test(Antibody **a, int s, int f, int cl) {
        for(int i = 0; i < lines; i++) {
                for(int j = 0; j < s; j++) { // Run it against good antibodies in this class
                        // fitness() with no argument computes how reliably this antibody can
                        // distinguish normal from attack. Skip over unreliable antibodies.
                        if(a[j]->fitness() <= 0.5) continue;
                        tested[i]++;
                        int ret = a[j]->match(input[i]);
                        int c = a[j]->queryClassification();
                        if(ret == -1) continue;
                        if(ret == 1) { // labeled as attack
                                results[i]++;
                                if(c == cl) guessedCount[i][c]++; // in correct class for population
                        }
                }
        }
}

void unknownWeb::setThreshold(int t) {
        Webdata::setThreshold(t);
}

ostream &operator<<(ostream &o, const unknownWeb &data) {
        for(int i = 0; i < data.lines; i++) {
                int localMax = data.guessedCount[i][0];
                int localWon = 0;
                for(int j = 1; j < CLASS_COUNT; j++) {
                        if(data.guessedCount[i][j] > localMax) {
                                localMax = data.guessedCount[i][j];
                                localWon = j;
                        }
                }
                if(data.guessedCount[i][localWon] <= data.threshold) {
                        o << "NORMAL (" << setw(5) << data.tested[i] << setw(5) 
                                << data.results[i] << ") ";
                }
                else {
                        o << "ATTACK (" << setw(5) << data.tested[i] << setw(5) 
                                << data.results[i] << ") Label: " << CLASS_LABELS[localWon] << " (";
                        for(int j = 0; j < CLASS_COUNT; j++) {
                                o << setw(5) << data.guessedCount[i][j];
                        }
                        o << ") ";
                }
                o << data.files[i];
                for(int j = 0; j < ALEN; j++) {
                        o << " " << data.input[i][j];
                }
                o << endl;
        }

        return o;
}
