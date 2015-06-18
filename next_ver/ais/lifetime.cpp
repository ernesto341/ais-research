#include <iostream>
//#include <strstream>
//#include <sstream>
#include <fstream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <cstring>
#include <string>
#include <cstdlib>
#include "antibody.h"
#include "unknownweb.h"
#include "mass.h"
#include "import.hpp"
using namespace std;

#define VERBOSE_OUTPUT
//#undef VERBOSE_OUTPUT

#ifndef _MAX_RUNS
#define _MAX_RUNS    15    // Number of simultaneous populations to test
#endif

// Set defaults for command line parameters
int MAX_ROUNDS = 100;          // Maximum number of generations
float PR_XOVER = 0.6;         // Percent crossover
float PR_MUTATION = 0.3;      // Percent mutation
float BAD_THRESHOLD = 0.0002; // Threshold for bad antibodies in self-test
int TRAIN_AGREE = 3;          // Number that must agree to mark attack
float MAX_NEG = .4;	      // Maximum false negative rate for a class. Currently unused.
float MIN_ACC = .5;	      // Minimum accuracy requirement for a class
int MAX_RUNS = _MAX_RUNS;

float indepAccPerClass[CLASS_COUNT][MAX_ANTIBODIES][CLASS_COUNT];
Antibody *pop[CLASS_COUNT][MAX_ANTIBODIES];
Antibody champs[CLASS_COUNT][MAX_ANTIBODIES];
float avgFitness[CLASS_COUNT];
float falsePos[CLASS_COUNT];
float falseNeg[CLASS_COUNT];
int falsePosCnt[CLASS_COUNT];
int falseNegCnt[CLASS_COUNT];
Webdata attack((char *)"attack", 1);
Webdata normal((char *)"normal", 0);
unknownWeb unknown((char *)"unknown");
int logNeg;       // Flag to webdata to log false negatives

ofstream fout;    // Fitness
ofstream fout2;   // Webdata fitness statistics
ofstream fout3;   // Debug msgs
ofstream fout4;   // Bad antibodies
ofstream fout5;   // False positive percentage
ofstream fout6;   // False negative percentage
ofstream fout7;   // Trained classifications
ofstream fout8;   // Details about missed lines in training data
ofstream mystery; // Labels for unknown data

void initialGen();
void generateAntibody(int, int);
void testSelf();
void train();
void testUnknown(int);
void nextGen(float []);
void outputTrainedClasses(int);
void cleanup (void);

#define ALL 99

void Copy(const int & c = -1, const int & a = -1)
{
	if (c < 0)
	{
		cerr << "No antibodies to copy or bad input\n" << "c = " << c << endl << flush;
		return;
	}
	if (c == ALL)
	{
		for (int i = 0; i < CLASS_COUNT; i++)
		{
			for (int j = 0; j < MAX_ANTIBODIES; j++)
			{
				memcpy(&(champs[i][j]), pop[i][j], sizeof(Antibody) * 1);
			}
		}
		return;
	}
	if (a < 0)
	{
		for (int j = 0; j < MAX_ANTIBODIES; j++)
		{
			memcpy(&(champs[c][j]), pop[c][j], sizeof(Antibody) * 1);
		}
		return;
	}
	memcpy(&(champs[c][a]), pop[c][a], sizeof(Antibody) * 1);
	return;
}

void htmlReport (Antibody a[CLASS_COUNT][MAX_ANTIBODIES])
{
	ofstream o("Champions_Report.html", ios::trunc);

	o << fixed;

	o << "<html>";
	o << "<head>";

	o << "<style> tr {text-align:center;} td {text-align:center;} </style>";
	o << "</head>";
	o << "<body>";
	o << "<table width=\"100%\" border=\"1\" align=\"center\">";
	o << "<tr>";
	o << "<td>";
	o << "<strong>";
	o << "Class";
	o << "</strong>";
	o << "</td>";
	o << "<td>";
	o << "<strong>";
	o << "#";
	o << "</strong>";
	o << "</td>";
	o << "<td>";
	o << "<strong>";
	o << "Fitness";
	o << "</strong>";
	o << "</td>";
	o << "<td colspan=\"" << ALEN << "\">";
	o << "<strong>";
	o << "[ Attribute";
	o << " | ";
	o << "Offset ]";
	o << "</strong>";
	o << "</td>";
	o << "<td colspan=\"" << CLASS_COUNT << "\">";
	o << "<strong>";
	o << "Accuracy Per Class";
	o << "</strong>";
	o << "</td>";
	o << "</tr>";

	for (int i = 0; i < CLASS_COUNT; i++)
	{
		for (int j = 0; j < MAX_ANTIBODIES; j++)
		{
			if (j % 2 == 0)
			{
				o << "<tr bgcolor=\"#FFFF88\">";
			}
			else
			{
				o << "<tr bgcolor=\"#88FFFF\">";
			}
			o << "<td>";
			o << CLASS_LABELS[i];
			o << "</td>";
			o << "<td>";
			o << j + 1;
			o << "</td>";
			o << "<td>";
			o << setprecision(4) << setfill('0') << a[i][j].fitness();
			o << "</td>";
			o.unsetf(ios_base::floatfield);
			o << setfill('\0');
			for (int k = 0; k < ALEN; k++)
			{
				o << "<td>";
				o << " [ " << a[i][j].getAttr(k) << " | " << a[i][j].getOff(k) <<  " ] ";
				o << "</td>";
			}
			o << setfill('0');
			o << fixed;
			for (int k = 0; k < CLASS_COUNT; k++)
			{
				o << "<td>";
				o << setprecision(4) << setfill('0') << indepAccPerClass[i][j][k];
				o << "</td>";
			}
			o << "</tr>";
		}
		if (MAX_ANTIBODIES % 2 == 0)
		{
			o << "<tr bgcolor=\"#FFFF88\">";
		}
		else
		{
			o << "<tr bgcolor=\"88FFFF\">";
		}
		o << "<td><strong>Class</strong>";
		o << "</td>";
		o << "<td><strong>#</strong>";
		o << "</td>";
		o << "<td><strong>Fitness</strong>";
		o << "</td>";
		for (int j = 0; j < ALEN; j++)
		{
			o << "<td><strong>";
			switch(j)
			{
				case 0:
					o << "Cmd";
					break;
				case 1:
					o << "Proto";
					break;
				case 2:
					o << "Len";
					break;
				case 3:
					o << "Vars";
					break;
				case 4:
					o << "Percent";
					break;
				case 5:
					o << "Apos";
					break;
				case 6:
					o << "Plus";
					break;
				case 7:
					o << "Per";
					break;
				case 8:
					o << "BSlash";
					break;
				case 9:
					o << "OParen";
					break;
				case 10:
					o << "CParen";
					break;
				case 11:
					o << "FSlash";
					break;
				case 12:
					o << "LT";
					break;
				case 13:
					o << "GT";
					break;
				default:
					o << "<Unknown>";
					break;
			}
			o << "</td>";
		}
		for (int j = 0; j < CLASS_COUNT; j++)
		{
			o << "<td><strong>";
			o << CLASS_LABELS[j];
			o << "</strong>";
			o << "</td>";
		}
		o << "</tr>";
	}

	o << "</table>";
	o << "</body>";
	o << "</html>";

	o.close();
	return;
}

void Report(Antibody a[CLASS_COUNT][MAX_ANTIBODIES])
{
	ofstream rep("champions.rep", ios::trunc);
	rep << setw(9) << "Class";
	rep << setw(3) << "#";
	rep << setw(9) << "Fitness";
	rep << setw(9) << "Attribute";
	rep << setw(3) << " - ";
	rep << setw(6) << "Offset";
	rep << setw(20) << "Accuracy Per Class";
	rep << endl;
	rep << setw(9) << "-----";
	rep << setw(3) << "-";
	rep << setw(9) << "-------";
	rep << setw(9) << "-----------";
	rep << setw(3) << "---";
	rep << setw(8) << "--------";
	rep << setw(20) << "------------------";
	rep << endl;

	for (int i = 0; i < CLASS_COUNT; i++)
	{
		rep << setw(9) << CLASS_LABELS[i];
		for (int j = 0; j < MAX_ANTIBODIES; j++)
		{
			if (j != 0)
			{
				rep << setw(9) << " ";
			}
			rep << setw(3) << j + 1;
			rep << setw(9) << a[i][j].fitness();
			for (int k = 0; k < ALEN; k++)
			{
				rep << setw(21) << " ";
				rep << setw(9) << right << a[i][j].getAttr(k);
				rep << setw(3) << " ";
				rep << setw(6) << left << a[i][j].getOff(k);
				if (k == 0)
				{
					/* print next stuff */
				}
				rep << endl;
			}
			rep << endl;
		}
		rep << endl;
	}
	rep << endl;

	rep.close();

	return;
}

void Dump (Antibody c[CLASS_COUNT][MAX_ANTIBODIES])
{
	ofstream o("champions.abs", ios::trunc);
	ofstream p("Champions.xml", ios::trunc);
	for (int i = 0; i < CLASS_COUNT; i++)
	{
		for (int j = 0; j < MAX_ANTIBODIES; j++)
		{
			o << champs[i][j].dump();
			p << champs[i][j].dumpXml();
		}
	}
	o.close();
	p.close();

	htmlReport(c);
	return;
}

char * use_existing = 000;

int main(int argc, char *argv[]) {
	srandom(time(NULL));
	string fname = "", fname2 = "", fname3 = "", fname4 = "", fname5 = "", fname6 = "", fname7 = "", fname8 = "";
	char mname[500];
	bool cont = false;
	int MA = MAX_ANTIBODIES;

	if(argc != 6) {
		cout << endl << "Usage: " << endl << "\t" << argv[0] 
			<< " [max_generations] [mutation_percent] [agreement_threshold]"
			<< " [crossover_percent] [self-test_threshold]" << endl << endl
			<< "\t\t\t--- OR ---" << endl << endl
			<< "\t" << argv[0] << " --existing <path_to_file>" << endl << endl
			<< "\t\t\t--- OR ---" << endl << endl
			<< "\t" << argv[0] << " -e <path_to_file>" << endl << endl;
		cout << "Defaults: max_gen=" << MAX_ROUNDS << " self-test=" << BAD_THRESHOLD 
			<< " agree=" << TRAIN_AGREE << " xover=" << PR_XOVER
			<< " mutate=" << PR_MUTATION << endl;
	}

	if (argc == 3 && ((strcmp(argv[1], (const char *)"--existing") == 0) || strcmp(argv[1], (const char *)"-e") == 0))
	{
		size_t flen = strlen(argv[2]);
		use_existing = new char [flen + 1];
		strncpy(use_existing, argv[2], flen);
		use_existing[flen] = '\0';
	}
	else
	{
		if(argc > 1) MAX_ROUNDS = atoi(argv[1]);
		if(argc > 2) PR_MUTATION = atof(argv[2]);
		if(argc > 3) TRAIN_AGREE = atoi(argv[3]);
		if(argc > 4) PR_XOVER = atof(argv[4]);
		if(argc > 5) BAD_THRESHOLD = atof(argv[5]);
	}

	cerr << "Vars: " << MAX_ROUNDS << " " << PR_MUTATION << endl;

	float classAccuracy[MAX_RUNS][MAX_ROUNDS][CLASS_COUNT];
	float averageAccuracy[MAX_ROUNDS][CLASS_COUNT];
	float bestAccuracyGeneration[MAX_ROUNDS][CLASS_COUNT];
	float bestAccuracy[CLASS_COUNT];
	float tmp_acc = 0.f;

	attack.setThreshold(TRAIN_AGREE);
	normal.setThreshold(TRAIN_AGREE);
	unknown.setThreshold((TRAIN_AGREE*2));
	logNeg = 0;

	sprintf(mname, "res/unknown.%d_%d_%f_%f", MA, MAX_ROUNDS,
			PR_XOVER, PR_MUTATION);
	mystery.open(mname);
	fname += "res/fitness.";
	fname += MA;
	fname += "_";
	fname += MAX_ROUNDS;
	fname += "_";
	fname += PR_XOVER;
	fname += "_";
	fname += PR_MUTATION;
	fname += "\0";
	fout.open(fname.c_str());

	cerr << fname << endl;

	fname2 += "res/details.";
	fname2 += MA;
	fname2 += "_";
	fname2 += MAX_ROUNDS;
	fname2 += "_";
	fname2 += PR_XOVER;
	fname2 += "_";
	fname2 += PR_MUTATION;
	fname2 += "\0";
	fout2.open(fname2.c_str());

#ifdef VERBOSE_OUTPUT
	fname3 += "res/full.";
	fname3 += MA;
	fname3 += "_";
	fname3 += MAX_ROUNDS;
	fname3 += "_";
	fname3 += PR_XOVER;
	fname3 += "_";
	fname3 += PR_MUTATION;
	fname3 += "\0";
	fout3.open(fname3.c_str());
#endif

	fname4 += "res/bad.";
	fname4 += MA;
	fname4 += "_";
	fname4 += MAX_ROUNDS;
	fname4 += "_";
	fname4 += PR_XOVER;
	fname4 += "_";
	fname4 += PR_MUTATION;
	fname4 += "\0";
	fout4.open(fname4.c_str());

	fname5 += "res/false_pos.";
	fname5 += MA;
	fname5 += "_";
	fname5 += MAX_ROUNDS;
	fname5 += "_";
	fname5 += PR_XOVER;
	fname5 += "_";
	fname5 += PR_MUTATION;
	fname5 += "\0";
	fout5.open(fname5.c_str());

	fname6 += "res/false_neg.";
	fname6 += MA;
	fname6 += "_";
	fname6 += MAX_ROUNDS;
	fname6 += "_";
	fname6 += PR_XOVER;
	fname6 += "_";
	fname6 += PR_MUTATION;
	fname6 += "\0";
	fout6.open(fname6.c_str());

	fname7 += "res/classifications.";
	fname7 += MA;
	fname7 += "_";
	fname7 += MAX_ROUNDS;
	fname7 += "_";
	fname7 += PR_XOVER;
	fname7 += "_";
	fname7 += PR_MUTATION;
	fname7 += "\0";
	fout7.open(fname7.c_str());

	fname8 += "res/missed.";
	fname8 += MA;
	fname8 += "_";
	fname8 += MAX_ROUNDS;
	fname8 += "_";
	fname8 += PR_XOVER;
	fname8 += "_";
	fname8 += PR_MUTATION;
	fname8 += "\0";
	fout8.open(fname8.c_str());

#ifdef VERBOSE_OUTPUT
	fout3 << "#Parameters: MAX_ROUNDS = " << MAX_ROUNDS << " MA = "
		<< MA << endl 
		<< "#            PR_XOVER = " << PR_XOVER << " PR_MUTATION = " 
		<< PR_MUTATION << endl
		<< "#            BAD_THRESHOLD = " << BAD_THRESHOLD
		<< "  TRAIN_AGREE = " << TRAIN_AGREE << endl;
#endif
	fout << "#Parameters: MAX_ROUNDS = " << MAX_ROUNDS << " MA = "
		<< MA << endl 
		<< "#            PR_XOVER = " << PR_XOVER << " PR_MUTATION = " 
		<< PR_MUTATION << endl
		<< "#            BAD_THRESHOLD = " << BAD_THRESHOLD
		<< "  TRAIN_AGREE = " << TRAIN_AGREE << endl;

	for(int i = 0; i < CLASS_COUNT; i++) {
		for(int j = 0; j < MA; j++) {
			pop[i][j] = NULL;
			for(int k = 0; k < CLASS_COUNT; k++) {
				indepAccPerClass[i][j][k] = 0.f;
			}
		}
		bestAccuracy[i] = 0.f;
	}

	for(int i = 0; i < MAX_ROUNDS; i++) {
		for(int j = 0; j < CLASS_COUNT; j++) {
			averageAccuracy[i][j] = 0.f;
			bestAccuracyGeneration[i][j] = 0.f;
		}
	}

	if (use_existing != 000)
	{
		if (importChamps(use_existing) == 000)
		{
			cerr << "Import operation failed for an unknown reason, starting fresh" << endl << flush;
			use_existing = 000;
		}
		/*
		   Antibody ** t_pop = importChamps(use_existing);
		   for (int i = 0; i < CLASS_COUNT; i++)
		   {
		   for (int j = 0; j < MA; j++)
		   {
		   memcpy(&(champs[i][j]), &(t_pop[i][j]), sizeof(Antibody) * 1);
		   }
		   }
		   */
	}
	else
	{
	}
	do
	{
		cont = false;
		for(int r = 0; r < MAX_RUNS; r++) {
			if (use_existing == 000)
			{
				initialGen();
			}
			fout << "Population " << setw(5) << r << endl;
			//cerr << "Population " << setw(5) << r << endl;
			for(int i = 0; i < MAX_ROUNDS; i++) {
				if(i == MAX_ROUNDS - 1) logNeg = 1;

				//cerr << "Generation " << i << endl;
				//cerr << left << setw(15) << "Class" << setw(10) << "Percent";
				//cerr << left << setw(7) << "Detect" << setw(7) << "Total" << endl;

#ifdef VERBOSE_OUTPUT
				fout3 << "BEGIN " << i << endl;
#endif
				fout << setw(5) << i << " |";
				fout4 << setw(3) << i;
				fout5 << setw(3) << i;
				fout6 << setw(3) << i;
				/*  moved to train()
				//if (use_existing == 000)
				{
				testSelf();
				}
				*/
				train();
				if (r == 0)
				{
					Copy(ALL);
				}
				for(int c = 0; c < CLASS_COUNT; c++) {
					classAccuracy[r][i][c] = attack.labelAccuracy[c];
					averageAccuracy[i][c] += attack.labelAccuracy[c];
					if(attack.labelAccuracy[c] > bestAccuracyGeneration[i][c])
					{
						bestAccuracyGeneration[i][c] = attack.labelAccuracy[c];
					}
					if (bestAccuracyGeneration[i][c] > bestAccuracy[c])
					{
						bestAccuracy[c] = bestAccuracyGeneration[i][c];
					}
					for (int j = 0; j < MA; j++)
					{
						for (int k = 0; k < CLASS_COUNT; k++)
						{
							tmp_acc = pop[c][j]->getCatCount(k) / pop[c][j]->getCatTotal(k);
							//tmp_acc = pop[c][j]->getCatPerc(k);
							/* and uniqueness measure? */
							if (indepAccPerClass[c][j][k] < tmp_acc)
							{
								indepAccPerClass[c][j][k] = tmp_acc;
								Copy(c, j);
							}
						}
					}
				}
				outputTrainedClasses(i);
				if(i < MAX_ROUNDS - 1) nextGen(classAccuracy[r][i]);
				fout << endl;
				fout5 << endl;
				fout6 << endl;
#ifdef VERBOSE_OUTPUT
				fout3 << "END " << i << endl;
#endif
			}
			testUnknown(r);
		}
		cout << "\nTraining complete. Best Accuracy per class:\n";
		for (int i = 0; i < CLASS_COUNT; i++)
		{
			cout << "\t" << setw(15) << setfill(' ') << CLASS_LABELS[i] << " - %" << setprecision(2) << fixed << setfill('0') << bestAccuracy[i] * 100.f << endl << endl << flush;
			cout << CLASS_LABELS[i] << " Antibody Traits:\n";
			for (int j = 0; j < MA; j++)
			{
				cout << " [" << j << "] : " << champs[i][j] << endl;
			}
			cout << endl << flush;
			if (bestAccuracy[i] < MIN_ACC)
			{
				cont = true;
				for (int j = 0; j < MA; j++)
				{
					generateAntibody(i, j);
				}
			}
		}
	}
	while (cont);
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
			cerr << setw(10) << bestAccuracyGeneration[i][j];
		}
		cerr << endl;
	}
	cerr << endl;

	Dump(champs);

	fout.close();
	fout2.close();
#ifdef VERBOSE_OUTPUT
	fout3.close();
#endif
	fout4.close();
	fout5.close();
	fout6.close();

	delete [] (use_existing);
	cleanup();

	cout << "Done generating antibodies." << endl << endl << flush;

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
#ifdef VERBOSE_OUTPUT
				fout3 << "Bad antibody " << i << " " << j  << " " << *pop[i][j];
#endif
				generateAntibody(i, j);
				normal.test(pop[i][j], 0, i);
				cnt++;
			}
			pop[i][j]->resetTests();  // reset for training round
		}
		normal.resetStats(); // reset for training round
		fout4 << setw(5) << cnt;
	}
	fout4 << endl;
}

void train() {
	testSelf();
	for(int i = 0; i < CLASS_COUNT; i++) {
		normal.test(pop[i], MAX_ANTIBODIES, 1, i);
		falsePos[i] = normal.queryMissed();
		falsePosCnt[i] = normal.queryMissedCounts();
		attack.test(pop[i], MAX_ANTIBODIES, 0, i);
		falseNeg[i] = attack.queryMissed();
		//cout << "train. Class " << i << ". False Neagtive: " << falseNeg[i] << endl << flush;
		falseNegCnt[i] = attack.queryMissedCounts();
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
		normal.resetStats();  // reset for next run
		attack.resetStats();  // reset for next run
#ifdef VERBOSE_OUTPUT
		for(int j = 0; j < MAX_ANTIBODIES; j++) {
			fout3 << "Trained " << i << " " << j  << " " << *pop[i][j];
		}
#endif
	}
}

void testUnknown(int r) {
	unknown.reset();
	mystery << "Population " << r << endl;
	for(int i = 0; i < CLASS_COUNT; i++) {
		unknown.test(pop[i], MAX_ANTIBODIES, 2, i);
	}
	mystery << unknown;
}

void nextGen(float accuracy[]) {
	for(int i = 0; i < CLASS_COUNT; i++) {
		//cout << "nextGen. accuracy[" << i << "] = " << accuracy[i] << endl << flush;
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
		fout << setiosflags(ios::fixed | ios::showpoint)
			<< setw(5) << setprecision(2) << avgFitness[i];

		fout5 << setiosflags(ios::fixed | ios::showpoint)
			<< setw(7) << setprecision(4) << falsePos[i] 
			<< setw(4) << falsePosCnt[i];
		fout6 << setiosflags(ios::fixed | ios::showpoint)
			<< setw(7) << setprecision(4) << falseNeg[i]
			<< setw(4) << falseNegCnt[i];

		int k = 0;
		int ccnt = (int)(MAX_ANTIBODIES * PR_XOVER); // number of parents/children
		for(int j = 0; j < ccnt/2; j++) {
			Antibody *tmp1 = parents.deleteRandom(ccnt - k);      // parent 1
			Antibody *tmp2 = parents.deleteRandom(ccnt - k - 1);  // parent 2
#ifdef VERBOSE_OUTPUT
			fout3 << "Parent 1 " << *tmp1 << "Parent 2 " << *tmp2;
#endif
			tmp1->mate(tmp2, &pop[i][k], &pop[i][k+1]);          // mate
#ifdef VERBOSE_OUTPUT
			fout3 << "Next gen (child) " << k << ": " << *pop[i][k];
			fout3 << "Next gen (child) " << k+1 << ": " << *pop[i][k+1];
#endif
			k += 2;
			delete tmp1;
			tmp1 = 000;
			delete tmp2;
			tmp2 = 000;
		}

		// Now put in the survivors. 
		// The number of survivors is MAX_ANTIBODIES - ccnt

		// Allow a fraction of the survivors to be anyone from the mass
		// to increase genetic diversity

		for(int j = 0; j < (MAX_ANTIBODIES - ccnt) / 10; j++) {
			pop[i][k++] = survivors.deleteRandom(MAX_ANTIBODIES - j);
#ifdef VERBOSE_OUTPUT
			fout3 << "Next gen (survivor) " << k-1 << ": " << *pop[i][k-1];
#endif
		}

		while(k < MAX_ANTIBODIES) {
			pop[i][k] = survivors.deleteRandom(MAX_ANTIBODIES - k);
#ifdef VERBOSE_OUTPUT
			fout3 << "Next gen (survivor) " << k << ": " << *pop[i][k];
			k++;
#endif
		}
		parents.killHeap();
		survivors.killHeap();
		for(int j = 0; j < MAX_ANTIBODIES; j++) {
			if(random() % 100 < 100 * PR_MUTATION) {
				pop[i][j]->mutate();
#ifdef VERBOSE_OUTPUT
				fout3 << "Mutated " << j << " to " << *pop[i][j];
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

void cleanup (void)
{
	for (int i = 0; i < CLASS_COUNT; i++)
	{
		for (int j = 0; j < MAX_ANTIBODIES; j++)
		{
			if(pop[i][j] != NULL)
			{
				delete pop[i][j];
				pop[i][j] = 000;
			}
		}
	}
}

