#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include "antibody.h"
#include "webdata.h"
using namespace std;

Webdata::Webdata(char *inf, int f) {
        char buf[2000];
        ifstream fin(inf);
        float tmp = 0.0;
        int blah;

        flag = f;
        lines = 0;
        while(fin.getline(buf, 2000)) {
                lines++;
        }
        fin.close();
        fin.clear();
        fin.open(inf, ios::in);

        files = new char*[lines];
        input = new int*[lines];
        results = new int[lines];
        tested = new int[lines];
        labels = new int[lines];
        resetStats();

        for(int i = 0; i < CLASS_COUNT; i++) {
                labelCount[i] = 0;
        }

        for(int i = 0; i < lines; i++) {
                results[i] = 0;
                tested[i] = 0;
                files[i] = new char[MAX_FILENAME];
                input[i] = new int[ALEN];
                fin >> files[i] >> buf >> tmp;
                input[i][0] = translateCommand(buf);
                input[i][1] = translateProtocol(tmp);
                for(int j = 2; j < ALEN; j++) {
                        fin >> input[i][j];
                }
                // Read in the class label and convert it to an index number
                // Only do this for the attack data
                // Also record how much of each label there is in the dataset
                if(flag == 1) {
                        fin >> buf;
                        labels[i] = translateLabel(buf);
                        labelCount[labels[i]]++;
                }
                else { // Set normal/unknown category to -1
                        labels[i] = -1;
                }
        }
}

Webdata::~Webdata() {
        for(int i = 0; i < lines; i++) {
                delete [] files[i];
                delete [] input[i];
        }
        delete [] files;
        delete [] input;
        delete [] results;
        delete [] tested;
        delete [] labels;
}

unsigned int Webdata::translateCommand(char *str) {
        unsigned int ret = 0;

        if(strcmp(str, "GET") == 0) ret += 1;
        else if(strcmp(str, "POST") == 0) ret += 2;
        else if(strcmp(str, "HEAD") == 0) ret += 4;
        else ret += 8;
        return ret;
}

unsigned int Webdata::translateProtocol(float prot) {
        unsigned int ret = 0;

        if(prot == 0.9) ret += 1;
        else if(prot == 1.0) ret += 2;
        else if(prot == 1.1) ret += 4;
        else ret += 8;
        return ret;
}

int Webdata::translateLabel(char *str) {
        for(int i = 0; i < CLASS_COUNT; i++) {
                if(strcmp(str, CLASS_LABELS[i]) == 0) return i;
        }
        return -1; /* Unknown class of attack */
}

// Test a single antibody. Self-test mode only when replacing an
// antibody that matched on self.
void Webdata::test(Antibody *a, int f, int cl) {
        for(int i = 0; i < lines; i++) {
                if(flag == 0) {            // normal data
                        if(f) {                  // training mode
                                if(random() % 5) {     // Probability 0.8 to skip
                                        continue;
                                }
                        }
                        else {                   // self-test mode
                                if(!(random() % 5)) {  // Probability 0.2 to skip
                                        continue;
                                }
                        }
                }
                int ret = a->match(input[i]);
                if(ret == -1) continue;
                if(ret == 1) {  // labeled as attack
                        if(flag == 1) {  // and it actually was an attack, good antibody
                                a->adjPos();
                                a->adjCategory(labels[i]); // Add one to this line's category
                        }
                        else { // actually was normal
                                a->adjFalsePos();
                        }
                }
                else {  // labeled as normal
                        if(flag == 1) {  // actually was an attack, wrong label
                                a->adjFalseNeg();
                        }
                        else {
                                a->adjNeg();
                        }
                }
        }

        // Now normalize the classification counts from a count to a percentage
        for(int i = 0; i < CLASS_COUNT; i++) {
                a->calcCategory(i, labelCount[i]);
                a->setCatTotal(i, labelCount[i]);
        }
}

void Webdata::test(Antibody **a, int s, int f, int cl) {
        for(int i = 0; i < lines; i++) {
                if(flag == 0) {            // normal data
                        if(f) {                  // training mode
                                if(random() % 5) {     // Probability 0.8 to skip
                                        continue;
                                }
                        }
                        else {                   // self-test mode
                                if(!(random() % 5)) {  // Probability 0.2 to skip
                                        continue;
                                }
                        }
                }
                tested[i] = 1;

                // Now test the antibody population on this line
                for(int j = 0; j < s; j++) {
                        int ret = a[j]->match(input[i]);
                        if(ret == -1) continue;
                        if(ret == 1) {  // labeled as attack
                                results[i] += 1;
                                if(flag == 1) {  
                                        a[j]->adjPos();
                                        a[j]->adjCategory(labels[i]); // Add one to this line's category
                                }
                                else {
                                        a[j]->adjFalsePos();
                                }
                        }
                        else {
                                if(flag == 1) {
                                        a[j]->adjFalseNeg();
                                }
                                else {
                                        a[j]->adjNeg();
                                }
                        }
                }
        }
        if(flag == 0) return; // no need to do classifications for normal data

        // Now normalize the classification counts from a count to a percentage
        for(int j = 0; j < s; j++) {
                for(int k = 0; k < CLASS_COUNT; k++) {
                        a[j]->calcCategory(k, labelCount[k]);
                        a[j]->setCatTotal(k, labelCount[k]);
                }
        }

        int classTotal = 0;
        int classDetected = 0;

        // Test to see how many antibodies in population can correctly label class
        for(int i = 0; i < lines; i++) {
                if(labels[i] < 0) continue;  // line has no category, skip it
                if(labels[i] != cl) continue; // Not the class we're interested in
                int localCat = 0;   // classifications of antibodies matching line
                int localMatch = 0; // number of antibodies that match this line

                classTotal++;

                for(int j = 0; j < s; j++) {
                        int ret = a[j]->match(input[i]);
                        if(ret != 1) continue;  // antibody did not match
                        localMatch++; 
                        int c = a[j]->queryClassification();
                        if(c == cl) localCat++;  // increment class that antibody detects
                }
                if(localCat > 0) classDetected++;
        }

        labelAccuracy[cl] = (classTotal > 0) ? ((float)classDetected/classTotal) : 0;

        cerr << left << setw(15) << CLASS_LABELS[cl] << setw(15) << labelAccuracy[cl];
        cerr << setw(7) << classDetected << setw(7) << classTotal << endl;
}

// This function returns the percentage of mislabelled data.
// Data is considered missed if:
//   For attack data, < 'threshold' antibodies labeled it attack.
//   For normal data, >= 'threshold' antibodies labeled it attack.
float Webdata::queryMissed() {
        float cnt1 = 0.0, cnt2 = 0.0;

        for(int i = 0; i < lines; i++) {
                if(!tested[i]) continue;
                cnt1++;
                if(results[i] < threshold) cnt2++;
        }
        return (flag ? cnt2/cnt1 : 1.0 - cnt2/cnt1);
}

int Webdata::queryMissedCounts() {
        int cnt1 = 0, cnt2 = 0;

        for(int i = 0; i < lines; i++) {
                if(!tested[i]) continue;
                cnt1++;
                if(results[i] < threshold) cnt2++;
        }
        return (flag ? cnt2 : cnt1 - cnt2);
}

void Webdata::resetStats() {
        for(int i = 0; i < lines; i++) {
                results[i] = 0;
                tested[i] = 0;
        }
}

void Webdata::printMissed(ostream &o) {
        for(int i = 0; i < lines; i++) {
                if(!tested[i]) continue;
                if((flag && results[i] < threshold) || (!flag && results[i] >= threshold)) {
                        o << files[i] << " ";
                        for(int j = 0; j < ALEN; j++) {
                                o << input[i][j] << " ";
                        }
                        o << "=> " << setw(3) << tested[i] << setw(5) << results[i] << endl;
                }
        }
}

ostream &operator<<(ostream &o, Webdata &h) {
        o << "Flag = " << h.flag << endl;
        for(int i = 0; i < h.lines; i++) {
                o << h.files[i];
                for(int j = 0; j < ALEN; j++) {
                        o << h.input[i][j] << " ";
                }
                o << " => " << setw(3) << h.tested[i] << setw(5) << h.results[i] << endl;
        }
        return o;
}
