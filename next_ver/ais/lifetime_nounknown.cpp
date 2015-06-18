#include <iostream>
#include <strstream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include "antibody.h"
#include "unknownweb.h"
#include "mass.h"
using namespace std;

#undef VERBOSE_OUTPUT
#undef TEST_UNKNOWN
#undef OUTPUT_INFO

#ifndef _MAX_RUNS
#define _MAX_RUNS    5    // Number of simultaneous populations to test
#endif

#ifndef _MAX_ANTIBODIES
#define _MAX_ANTIBODIES   25  // Number of antibodies per population
#endif

// Set defaults for command line parameters
int MAX_ROUNDS = 10;          // Maximum number of generations
float PR_XOVER = 0.6;         // Percent crossover
float PR_MUTATION = 0.1;      // Percent mutation
float BAD_THRESHOLD = 0.0002; // Threshold for bad antibodies in self-test
int TRAIN_AGREE = 3;          // Number that must agree to mark attack
int MAX_ANTIBODIES = _MAX_ANTIBODIES;
int MAX_RUNS = _MAX_RUNS;

Antibody *pop[CLASS_COUNT][MAX_ANTIBODIES];
float avgFitness[CLASS_COUNT];
float falsePos[CLASS_COUNT];
float falseNeg[CLASS_COUNT];
int falsePosCnt[CLASS_COUNT];
int falseNegCnt[CLASS_COUNT];
Webdata attack("attack", 1);
Webdata normal("normal", 0);
#ifdef TEST_UNKNOWN
unknownWeb unknown("unknown");
#endif
int logNeg;       // Flag to webdata to log false negatives

#ifdef OUTPUT_INFO
ofstream fout;    // Fitness
ofstream fout2;   // Webdata fitness statistics
ofstream fout3;   // Debug msgs
ofstream fout4;   // Bad antibodies
ofstream fout5;   // False positive percentage
ofstream fout6;   // False negative percentage
ofstream fout7;   // Trained classifications
ofstream fout8;   // Details about missed lines in training data
#ifdef TEST_UNKNOWN
ofstream mystery; // Labels for unknown data
#endif
#endif

void initialGen();
void generateAntibody(int, int);
void testSelf();
void train();
#ifdef TEST_UNKNOWN
void testUnknown(int);
#endif
void nextGen(float []);
#ifdef OUTPUT_INFO
void outputTrainedClasses(int);
#endif

int main(int argc, char *argv[]) {
  char f[500], f2[500], f3[500], f4[500], f5[500], f6[500], f7[500], f8[500];
  ostrstream fname(f, 500, ios::out);
  ostrstream fname2(f2, 500, ios::out);
  ostrstream fname3(f3, 500, ios::out);
  ostrstream fname4(f4, 500, ios::out);
  ostrstream fname5(f5, 500, ios::out);
  ostrstream fname6(f6, 500, ios::out);
  ostrstream fname7(f7, 500, ios::out);
  ostrstream fname8(f8, 500, ios::out);
  char mname[500];

  if(argc != 6) {
    cerr << "Usage: " << argv[0] 
         << " [max_generations] [mutation_percent] [agreement_threshold]"
         << " [crossover_percent] [self-test_threshold]" << endl;
    cerr << "Defaults: max_gen=" << MAX_ROUNDS << " self-test=" << BAD_THRESHOLD 
         << " agree=" << TRAIN_AGREE << " xover=" << PR_XOVER
         << " mutate=" << PR_MUTATION << endl;
  }

  if(argc > 1) MAX_ROUNDS = atoi(argv[1]);
  if(argc > 2) PR_MUTATION = atof(argv[2]);
  if(argc > 3) TRAIN_AGREE = atoi(argv[3]);
  if(argc > 4) PR_XOVER = atof(argv[4]);
  if(argc > 5) BAD_THRESHOLD = atof(argv[5]);

  cerr << "Vars: " << MAX_ROUNDS << " " << PR_MUTATION << endl;

  float classAccuracy[MAX_RUNS][MAX_ROUNDS][CLASS_COUNT];
  float averageAccuracy[MAX_ROUNDS][CLASS_COUNT];
  float bestAccuracy[MAX_ROUNDS][CLASS_COUNT];

  for(int i = 0; i < MAX_ROUNDS; i++) {
    for(int j = 0; j < CLASS_COUNT; j++) {
      averageAccuracy[i][j] = 0;
      bestAccuracy[i][j] = 0;
    }
  }

  attack.setThreshold(TRAIN_AGREE);
  normal.setThreshold(TRAIN_AGREE);
#ifdef TEST_UNKNOWN
  unknown.setThreshold((TRAIN_AGREE*2));
#endif
  logNeg = 0;

#ifdef TEST_UNKNOWN
  sprintf(mname, "res/unknown.%d_%d_%f_%f", MAX_ANTIBODIES, MAX_ROUNDS,
          PR_XOVER, PR_MUTATION);
  mystery.open(mname);
#endif

#ifdef OUTPUT_INFO
  fname << "res/fitness." << MAX_ANTIBODIES << "_" << MAX_ROUNDS << "_"
        << PR_XOVER << "_" << PR_MUTATION << ends;
  fout.open(f);

  cerr << f << endl;

  fname2 << "res/details." << MAX_ANTIBODIES << "_" << MAX_ROUNDS << "_"
         << PR_XOVER << "_" << PR_MUTATION << ends;
  fout2.open(f2);

#ifdef VERBOSE_OUTPUT
  fname3 << "res/full." << MAX_ANTIBODIES << "_" << MAX_ROUNDS << "_"
         << PR_XOVER << "_" << PR_MUTATION << ends;
  fout3.open(f3);
#endif

  fname4 << "res/bad." << MAX_ANTIBODIES << "_" << MAX_ROUNDS << "_"
         << PR_XOVER << "_" << PR_MUTATION << ends;
  fout4.open(f4);

  fname5 << "res/false_pos." << MAX_ANTIBODIES << "_" << MAX_ROUNDS << "_"
         << PR_XOVER << "_" << PR_MUTATION << ends;
  fout5.open(f5);

  fname6 << "res/false_neg." << MAX_ANTIBODIES << "_" << MAX_ROUNDS << "_"
         << PR_XOVER << "_" << PR_MUTATION << ends;
  fout6.open(f6);

  fname7 << "res/classifications." << MAX_ANTIBODIES << "_"<< MAX_ROUNDS << "_"
         << PR_XOVER << "_" << PR_MUTATION << ends;
  fout7.open(f7);

  fname8 << "res/missed." << MAX_ANTIBODIES << "_"<< MAX_ROUNDS << "_"
         << PR_XOVER << "_" << PR_MUTATION << ends;
  fout8.open(f8);

#ifdef VERBOSE_OUTPUT
  fout3 << "#Parameters: MAX_ROUNDS = " << MAX_ROUNDS << " MAX_ANTIBODIES = "
        << MAX_ANTIBODIES << endl 
        << "#            PR_XOVER = " << PR_XOVER << " PR_MUTATION = " 
        << PR_MUTATION << endl
        << "#            BAD_THRESHOLD = " << BAD_THRESHOLD
        << "  TRAIN_AGREE = " << TRAIN_AGREE << endl;
#endif
  fout << "#Parameters: MAX_ROUNDS = " << MAX_ROUNDS << " MAX_ANTIBODIES = "
       << MAX_ANTIBODIES << endl 
       << "#            PR_XOVER = " << PR_XOVER << " PR_MUTATION = " 
       << PR_MUTATION << endl
       << "#            BAD_THRESHOLD = " << BAD_THRESHOLD
       << "  TRAIN_AGREE = " << TRAIN_AGREE << endl;
#endif

  for(int i = 0; i < CLASS_COUNT; i++) {
    for(int j = 0; j < MAX_ANTIBODIES; j++) {
      pop[i][j] = NULL;
    }
  }

  for(int r = 0; r < MAX_RUNS; r++) {
    initialGen();
#ifdef OUTPUT_INFO
    fout << "Population " << setw(5) << r << endl;
#endif
    cerr << "Population " << setw(5) << r << endl;
    for(int i = 0; i < MAX_ROUNDS; i++) {
      if(i == MAX_ROUNDS - 1) logNeg = 1;

      cerr << "Generation " << i << endl;
      cerr << left << setw(15) << "Class" << setw(10) << "Percent";
      cerr << left << setw(7) << "Detect" << setw(7) << "Total" << endl;

#ifdef OUTPUT_INFO
#ifdef VERBOSE_OUTPUT
      fout3 << "BEGIN " << i << endl;
#endif
      fout << setw(5) << i << " |";
      fout4 << setw(3) << i;
      fout5 << setw(3) << i;
      fout6 << setw(3) << i;
#endif
      testSelf();
      train();
      for(int c = 0; c < CLASS_COUNT; c++) {
        classAccuracy[r][i][c] = attack.labelAccuracy[c];
        averageAccuracy[i][c] += attack.labelAccuracy[c];
        if(attack.labelAccuracy[c] > bestAccuracy[i][c])
          bestAccuracy[i][c] = attack.labelAccuracy[c];
      }
#ifdef OUTPUT_INFO
      outputTrainedClasses(i);
#endif
      if(i < MAX_ROUNDS - 1) nextGen(classAccuracy[r][i]);
#ifdef OUTPUT_INFO
      fout << endl;
      fout5 << endl;
      fout6 << endl;
#ifdef VERBOSE_OUTPUT
      fout3 << "END " << i << endl;
#endif
#endif
    }
#ifdef TEST_UNKNOWN
    testUnknown(r);
#endif
  }
  for(int r = 0; r < MAX_RUNS; r++) {
    cerr << "Overall classification stats for Population " << r << endl;
    cerr << left << setw(5) << "Gen";
    for(int c = 0; c < CLASS_COUNT; c++) {
      cerr << left << setw(10) << CLASS_LABELS[c];
    }
    cerr << endl;
    for(int i = 0; i < MAX_ROUNDS; i++) {
      cerr << setw(5) << i;
      for(int c = 0; c < CLASS_COUNT; c++) {
        cerr << setw(10) << classAccuracy[r][i][c];
      }
      cerr << endl;
    }
  }
  cerr << "Average accuracy over all populations.\n";
  cerr << left << setw(5) << "Gen";
  for(int c = 0; c < CLASS_COUNT; c++) {
    cerr << left << setw(10) << CLASS_LABELS[c];
  }
  cerr << endl;
  for(int i = 0; i < MAX_ROUNDS; i++) {
    cerr << setw(5) << i;
    for(int j = 0; j < CLASS_COUNT; j++) {
      cerr << setw(10) << averageAccuracy[i][j] / MAX_RUNS;
    }
    cerr << endl;
  }
  cerr << "Best accuracy over all populations.\n";
  cerr << left << setw(5) << "Gen";
  for(int c = 0; c < CLASS_COUNT; c++) {
    cerr << left << setw(10) << CLASS_LABELS[c];
  }
  cerr << endl;
  for(int i = 0; i < MAX_ROUNDS; i++) {
    cerr << setw(5) << i;
    for(int j = 0; j < CLASS_COUNT; j++) {
      cerr << setw(10) << bestAccuracy[i][j];
    }
    cerr << endl;
  }
#ifdef OUTPUT_INFO
  fout.close();
  fout2.close();
#ifdef VERBOSE_OUTPUT
  fout3.close();
#endif
  fout4.close();
  fout5.close();
  fout6.close();
#endif
  return 0;
}

void initialGen() {
  for(int i = 0; i < CLASS_COUNT; i++) {
    for(int j = 0; j < MAX_ANTIBODIES; j++) {
      generateAntibody(i, j);
    }
  }
}

void generateAntibody(int i, int j) {
  if(pop[i][j] != NULL) {
    delete pop[i][j];
    pop[i][j] = NULL;
  }
  try {
    pop[i][j] = new Antibody();
  } catch(bad_alloc) {
    pop[i][j] = NULL;
  }
  if(pop[i][j] == NULL) {
    cerr << "Allocation failure. Try with a smaller population size.\n";
    exit(1);
  }
}

void testSelf() {
  for(int i = 0; i < CLASS_COUNT; i++) {
    int cnt = 0;
    normal.test(pop[i], MAX_ANTIBODIES, 0, i);
    for(int j = 0; j < MAX_ANTIBODIES; j++) {
      int tests = pop[i][j]->queryTests();
      int max = (int)(tests * BAD_THRESHOLD);
      if(max == 0) max = 1;  // Allow at least 1 mis-label
      while(pop[i][j]->queryFalsePos() > max) { // labelled self data as attack
#ifdef OUTPUT_INFO
#ifdef VERBOSE_OUTPUT
        fout3 << "Bad antibody " << i << " " << j  << " " << *pop[i][j];
#endif
#endif
        generateAntibody(i, j);
        normal.test(pop[i][j], 0, i);
        cnt++;
      }
      pop[i][j]->resetTests();  // reset for training round
    }
    normal.resetStats(); // reset for training round
#ifdef OUTPUT_INFO
    fout4 << setw(5) << cnt;
#endif
  }
#ifdef OUTPUT_INFO
  fout4 << endl;
#endif
}

void train() {
  for(int i = 0; i < CLASS_COUNT; i++) {
    normal.test(pop[i], MAX_ANTIBODIES, 1, i);
    falsePos[i] = normal.queryMissed();
    falsePosCnt[i] = normal.queryMissedCounts();
    attack.test(pop[i], MAX_ANTIBODIES, 0, i);
    falseNeg[i] = attack.queryMissed();
    falseNegCnt[i] = attack.queryMissedCounts();
#ifdef OUTPUT_INFO
    if(logNeg) {
      fout2 << "Population " << i << " antibodies:" << endl;
      for(int j = 0; j < MAX_ANTIBODIES; j++) {
        fout2 << setw(2) << j << " " << *pop[i][j];
      }
      fout2 << "Population " << i << " attack data performance:" << endl;
      fout2 << attack;
      fout8 << "Population " << i << " false positives:" << endl;
      normal.printMissed(fout8);
    }
#endif
    normal.resetStats();  // reset for next run
    attack.resetStats();  // reset for next run
#ifdef OUTPUT_INFO
#ifdef VERBOSE_OUTPUT
    for(int j = 0; j < MAX_ANTIBODIES; j++) {
      fout3 << "Trained " << i << " " << j  << " " << *pop[i][j];
    }
#endif
#endif
  }
}

#ifdef TEST_UNKNOWN
void testUnknown(int r) {
  unknown.reset();
  mystery << "Population " << r << endl;
  for(int i = 0; i < CLASS_COUNT; i++) {
    unknown.test(pop[i], MAX_ANTIBODIES, 2, i);
  }
  mystery << unknown;
}
#endif

void nextGen(float accuracy[]) {
  for(int i = 0; i < CLASS_COUNT; i++) {
    if(accuracy[i] >= 1.0) continue;  // No need to breed, 100% accuracy for class
    AntibodyMass parents(pop[i], MAX_ANTIBODIES, i);
    AntibodyMass survivors(parents);

    avgFitness[i] = 0;
    for(int j = 0; j < MAX_ANTIBODIES; j++) {
      avgFitness[i] += pop[i][j]->fitness(i);
      delete pop[i][j];  // free space for children
      pop[i][j] = NULL;
    }
    avgFitness[i] /= MAX_ANTIBODIES;
#ifdef OUTPUT_INFO
    fout << setiosflags(ios::fixed | ios::showpoint)
         << setw(5) << setprecision(2) << avgFitness[i];

    fout5 << setiosflags(ios::fixed | ios::showpoint)
          << setw(7) << setprecision(4) << falsePos[i] 
          << setw(4) << falsePosCnt[i];
    fout6 << setiosflags(ios::fixed | ios::showpoint)
          << setw(7) << setprecision(4) << falseNeg[i]
          << setw(4) << falseNegCnt[i];
#endif
    
    int k = 0;
    int ccnt = (int)(MAX_ANTIBODIES * PR_XOVER); // number of parents/children
    for(int j = 0; j < ccnt/2; j++) {
      Antibody *tmp1 = parents.deleteRandom(ccnt - k);      // parent 1
      Antibody *tmp2 = parents.deleteRandom(ccnt - k - 1);  // parent 2
#ifdef OUTPUT_INFO
#ifdef VERBOSE_OUTPUT
      fout3 << "Parent 1 " << *tmp1 << "Parent 2 " << *tmp2;
#endif
#endif
      tmp1->mate(tmp2, &pop[i][k], &pop[i][k+1]);          // mate
#ifdef OUTPUT_INFO
#ifdef VERBOSE_OUTPUT
      fout3 << "Next gen (child) " << k << ": " << *pop[i][k];
      fout3 << "Next gen (child) " << k+1 << ": " << *pop[i][k+1];
#endif
#endif
      k += 2;
      delete tmp1;
      delete tmp2;
    }

    // Now put in the survivors. 
    // The number of survivors is MAX_ANTIBODIES - ccnt

    // Allow a fraction of the survivors to be anyone from the mass
    // to increase genetic diversity

    for(int j = 0; j < (MAX_ANTIBODIES - ccnt) / 10; j++) {
      pop[i][k++] = survivors.deleteRandom(MAX_ANTIBODIES - j);
#ifdef OUTPUT_INFO
#ifdef VERBOSE_OUTPUT
      fout3 << "Next gen (survivor) " << k-1 << ": " << *pop[i][k-1];
#endif
#endif
    }

    while(k < MAX_ANTIBODIES) {
      pop[i][k++] = survivors.deleteRandom(MAX_ANTIBODIES - k);
#ifdef OUTPUT_INFO
#ifdef VERBOSE_OUTPUT
      fout3 << "Next gen (survivor) " << k-1 << ": " << *pop[i][k-1];
#endif
#endif
    }
    parents.killHeap();
    survivors.killHeap();
    for(int j = 0; j < MAX_ANTIBODIES; j++) {
      if(random() % 100 < 100 * PR_MUTATION) {
        pop[i][j]->mutate();
#ifdef OUTPUT_INFO
#ifdef VERBOSE_OUTPUT
        fout3 << "Mutated " << j << " to " << *pop[i][j];
#endif
#endif
      }
    }
  }

  // Reset population for next generation
  for(int i = 0; i < CLASS_COUNT; i++) {
    for(int j = 0; j < MAX_ANTIBODIES; j++) {
      pop[i][j]->resetTests();  // reset for next generation
    }
  }
}

#ifdef OUTPUT_INFO
void outputTrainedClasses(int i) {
  int overallClasses[CLASS_COUNT];

  fout7 << "Gen " << setw(5) << i << setw(5) << "Cls";
  for(int i = 0; i < CLASS_COUNT; i++) {
    fout7 << setw(10) << CLASS_LABELS[i];
  }
  fout7 << endl;

  for(int j = 0; j < CLASS_COUNT; j++) {
    overallClasses[j] = 0;
  }

  for(int i = 0; i < CLASS_COUNT; i++) {
    fout7 << setw(5) << i;
    for(int j = 0; j < MAX_ANTIBODIES; j++) {
      int c = pop[i][j]->queryClassification();
      if(c == -1) continue;
      if(c == i) overallClasses[c]++; // matches desired class for sub-pop
    }
  }

  for(int j = 0; j < CLASS_COUNT; j++) {
    fout7 << setw(10) << overallClasses[j];
  }

  fout7 << endl;
}
#endif
