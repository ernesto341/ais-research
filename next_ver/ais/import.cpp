#include "import.hpp"

using namespace std;

struct stat buf;

volatile sig_atomic_t alen = 0;
volatile sig_atomic_t class_count = 0;
volatile sig_atomic_t ab_count = 0;

unsigned int arr_size = 100;
static bool checked = false;

int fib(int n)
{
	if (n < 2) return n;
	return (fib(n-1) + fib(n-2));
}

char * Contains (char * str, char * pattern)
{
	if (str == NULL || pattern == NULL)
	{
		return (NULL);
	}
	uint32_t str_len = strlen(str);
	uint32_t pattern_len = strlen(pattern);
	if (&(str[0]) == &(pattern[0]) && str_len == pattern_len)
	{
		return (str);
	}
	if (pattern_len < 1)
	{
		return (NULL);
	}
	if (str_len < pattern_len)
	{
		return (NULL);
	}
	uint32_t i = 0, j = 0, k = 1;
	uint8_t good = 0;
	if (pattern_len == 1)
	{
		while (i < str_len)
		{
			if (str[i] == pattern[0])
			{
				/* found */
				return (&(str[i]));
			}
			i++;
		}
		return (NULL);
	}
	/* only check up to max len */
	while (i < str_len-pattern_len+1)
	{
		/* check first and last chars for match */
		if (((str[i]) ^ (pattern[0])) == 0)
		{
			if (((str[i + pattern_len-1]) ^ (pattern[pattern_len-1])) == 0)
			{
				/* check digrams for equality, exit on false.
				 * possible for even number of chars, perfect only check inner letters.
				 * possible for odd number of chars, last check should check second to last + last */
				j = i+1;
				k = 1;
				good = 0;
				while (j-i < pattern_len-2 && good == 0)
				{
					if (!((((str[j]) | (str[j+1])) ^ ((pattern[k]) | (pattern[k+1]))) == 0))
					{
						good = 1;
					}
					j+=2;
					k+=2;
				}
				if (good == 0)
				{
					return (&(str[i]));
				}
			}
		}
		i++;
	}
	return (NULL);
}

inline static void resizeChar(char ** a = 0, unsigned int * s = 0)
{
	if (a == 0)
	{
		return;
	}
	int32_t size = strlen(*a);
	char * tmp = (char *)malloc(sizeof(char) * size * 2);
	memcpy(tmp, *a, size);
	free(*a);
	*a = tmp;
	if (s != 0)
	{
		*s = (size * 2);
	}
}

inline static bool Exists (const char * n)
{
	return((bool)(stat(n, &buf) == 0));
}

Antibody ** import (char *fin)
{
	if (fin == (char *)0)
	{
		fin = (char *)"./champions.abs\0";
	}
	if (DEBUG)
	{
		cerr << "importChamps: filename = " << fin << endl << flush;
	}
	ifstream in(fin);
	if (in.fail())
	{
		strcmp(fin, (const char *)"./champions.abs\0") == 0 ? fin = (char *)"./ais/champions.abs\0" : fin = (char *)"./champions.abs\0";
		if (DEBUG)
		{
			cerr << "importChamps: filename = " << fin << endl << flush;
		}
		ifstream in(fin);
		if (in.fail())
		{
			if (DEBUG)
			{
				cerr << "Unable to open champs file\nAttempting to generate...\n" << flush;
			}
			if (!checked)
			{
				//system(Exists((const char *)("./../ais/lifetime.25\0")) ? (char *)("cd ./../ais/ && ./lifetime.25 && cd ../retdir/\0") : (Exists((const char *)("./ais/lifetime.25\0")) ? (char *)("cd ./ais/ && ./lifetime.25 && cd ../\0") : (char *)("")));
				checked = true;

				return(importChamps((char *)(0)));
			}
			else
			{
				if (DEBUG)
				{
					cerr << "Unable to generate champs file. Aborting.\n" << flush;
				}
				return (0);
			}
		}
	}
	char * data = 0;
	data = (char *)malloc(sizeof(char) * arr_size);
	if (!data)
	{
		if (DEBUG)
		{
			cerr << "Unable to allocate memory\n" << flush;
		}
		return (0);
	}
	int i = 0;
	ab_count = 0;
	alen = 0;
	class_count = 0;
	while (in.get(data[i]))
	{
		if (DEBUG)
		{
			cerr << data[i] << flush;
		}
		if (ab_count == 0)
		{
			if (data[i] == ',')
			{
				alen++;
			}
			else if (data[i] == ';')
			{
				class_count++;
			}
		}
		if (data[i] == '\n')
		{
			ab_count++;
		}
		i++;
		if ((unsigned int)i == arr_size - 1)
		{
			resizeChar(&data, &arr_size);
		}
	}
	if (DEBUG)
	{
		cerr << endl << flush;
	}
	in.close();
	if (DEBUG)
	{
		cerr << "Got alen: " << alen << endl;
		cerr << "Got class_count: " << class_count << endl;
	}
	ab_count /= class_count;
	int data_size = strlen(data);
	if (DEBUG)
	{
		cerr << "Got ab_count: " << ab_count << endl;
		cerr << "Got data_size: " << data_size << endl << flush;
	}

	Antibody ** tmp_pop = 0;
	tmp_pop = new Antibody *[class_count];
	for (int k = 0; k < class_count; k++)
	{
		tmp_pop[k] = new Antibody [ab_count];
	}

	int j = 0, k = 0, l = 0, tmp = 0, begin = 0;
	i = 0;
	while (i < class_count && l < data_size)
	{
		j = 0;
		while (j < ab_count && l < data_size)
		{
			k = 0;
			/* first val:   flag[i] (i < alen) */
			/* second val:  a[i] - attribute */
			/* third val:   offset[i] */
			/* fourth val:  max[i] */
			/* comma separates value set, iterate iterator */
			while (k < alen && l < data_size)
			{
				begin = l;
				while (data[l] != 32)
				{
					l++;
				}
				tmp = atoi(&data[begin]);
				tmp_pop[i][j].setFlag(k, tmp);
				begin = l;
				l++;
				while (data[l] != 32)
				{
					l++;
				}
				tmp = atoi(&data[begin]);
				tmp_pop[i][j].setAttr(k, tmp);
				begin = l;
				l++;
				while (data[l] != 32)
				{
					l++;
				}
				tmp = atoi(&data[begin]);
				tmp_pop[i][j].setOffset(k, tmp);
				begin = l;
				l++;
				while (data[l] != 32)
				{
					l++;
				}
				tmp = atoi(&data[begin]);
				tmp_pop[i][j].setMax(k, tmp);
				while (data[l] != 32)
				{
					l++;
				}
				l++;
				k++;
			}
			/* tab to stats */
			while (data[l] != 9)
			{
				l++;
			}
			begin = l;
			l++;
			while (data[l] != 32)
			{
				l++;
			}
			tmp = atoi(&data[begin]);
			tmp_pop[i][j].setTests(tmp);
			begin = l;
			l++;
			while (data[l] != 32)
			{
				l++;
			}
			tmp = atoi(&data[begin]);
			tmp_pop[i][j].setPos(tmp);
			begin = l;
			l++;
			while (data[l] != 32)
			{
				l++;
			}
			tmp = atoi(&data[begin]);
			tmp_pop[i][j].setFalsePos(tmp);
			begin = l;
			l++;
			while (data[l] != 32)
			{
				l++;
			}
			tmp = atoi(&data[begin]);
			tmp_pop[i][j].setNeg(tmp);
			begin = l;
			tmp = atoi(&data[begin]);
			tmp_pop[i][j].setFalseNeg(tmp);
			while (data[l] != 9)
			{
				l++;
			}
			k = 0;
			while (k < class_count)
			{
				begin = l;
				tmp = atoi(&data[begin]);
				l++;
				while (data[l] != 32)
				{
					l++;
				}
				tmp_pop[i][j].setCat(k, tmp);
				k++;
			}
			/* use a tab to delimit totals */
			while (data[l] != 9)
			{
				l++;
			}
			k = 0;
			while (k < class_count)
			{
				begin = l;
				tmp = atoi(&data[begin]);
				l++;
				/* spaces between values */
				while (data[l] != 32)
				{
					l++;
				}
				if (DEBUG)
				{
					cerr << "Precalc percent: " << tmp_pop[i][j].getCatPerc(k) << endl << flush;
					cerr << "Got total count: " << tmp << endl << flush;
				}
				tmp_pop[i][j].setCatTotal(k, tmp);
				/* figure percentages */
				tmp_pop[i][j].calcCategory(k, tmp);
				if (DEBUG)
				{
					cerr << "Postcalc percent: " << tmp_pop[i][j].getCatPerc(k) << endl << endl << flush;
				}
				k++;
			}
			/* newline, end of line */
			while (data[l] != 10)
			{
				l++;
			}
			l++;
			j++;
		}
		i++;
	}

	if (DEBUG)
	{
		cerr << endl << flush;
		/* Excessive output */
		fprintf(stderr, "\n\nDone importing. Got the following:\n");
		for (int i = 0; i < class_count; i++)
		{
			fprintf(stderr, "Class %d :\n", i + 1);
			for (int j = 0; j < ab_count; j++)
			{
				cerr << "\t" << tmp_pop[i][j] << "\n";
			}
			cerr << endl;
		}
		cerr << endl << endl << flush;
	}

	/* copy temporary population to pop */
	for (i = 0; i < CLASS_COUNT; i++)
	{
		for (j = 0; j < MAX_ANTIBODIES; j++)
		{
			if (pop[i][j] != 000)
			{
				pop[i][j] = 000;
			}
			pop[i][j] = new Antibody();
			memcpy(pop[i][j], &(tmp_pop[i][i]), sizeof(Antibody) * 1);
		}
	}

	return (tmp_pop);
}

Antibody ** importXml(char * h)
{
	if (DEBUG)
	{
		cerr << endl << "importxml begin, got " << h << endl;
	}
	ifstream in;
	if (!h)
	{
		in.open("Champions.xml");
	}
	else
	{
		in.open(h);
	}
	if (in.fail())
	{
		if (DEBUG)
		{
			cerr << "unable to open file" << endl;
		}
		return (0);
	}
	string buf = "";
	/* xml open */
	getline(in, buf, '>'); // end of opening tag
	getline(in, buf, '>'); // end of first data opening tag
	ab_count = class_count = alen = 0;
	{
		getline(in, buf, '<'); // end of data tag
		if (DEBUG)
		{
			cerr << "very begining, got data: " << buf << endl;
		}
		ab_count = atoi(buf.c_str());
		/* end tag */
		getline(in, buf, '>'); // end of closing tag
	}
	{
		getline(in, buf, '>'); // end of open tag
		getline(in, buf, '<'); // end of data tag
		if (DEBUG)
		{
			cerr << "very begining, got data: " << buf << endl;
		}
		class_count = atoi(buf.c_str());
		/* end tag */
		getline(in, buf, '>'); // end of closing tag
	}
	{
		getline(in, buf, '>'); // end of closing tag
		getline(in, buf, '<'); // end of data tag
		if (DEBUG)
		{
			cerr << "very begining, got data: " << buf << endl;
		}
		alen = atoi(buf.c_str());
		/* end tag */
		getline(in, buf, '>'); // end of closing tag
	}

	if (DEBUG)
	{
		cerr << "got ab_count: " << ab_count << endl;
		cerr << "got class_count: " << class_count << endl;
		cerr << "got alen: " << alen << endl;
		cerr << "buf: " << buf << endl;
	}
	Antibody ** tmp_pop = 000;
	tmp_pop = new Antibody * [class_count];
	size_t i = 0, j = 0, pos = 0, iter = 0;
	for (i = 0; i < ab_count; i++)
	{
		tmp_pop[i] = new Antibody();
	}
	i = 0;
	string category = "";
	if (DEBUG)
	{
		cerr << "buf: " << buf << endl;
	}
	while (in.good() && i < class_count)
	{
		// get next opening tag
		getline(in, buf, '>'); // end of opening tag
		if (DEBUG)
		{
			cerr << "pos: " << pos << endl;
		}
		//if (pos == 0)
		{
			if (Contains((char *)(buf.c_str()), (char *)"class_count"))
			{
				if (DEBUG)
				{
					cerr << "skipping: " << buf << endl;
				}
				getline(in, buf, '<'); // end of data tag
				if (DEBUG)
				{
					cerr << "skipping: " << buf << endl;
				}
				getline(in, buf, '>'); // end of closing tag
				if (DEBUG)
				{
					cerr << "skipping: " << buf << endl;
				}
				pos = 0;
				continue;
			}
			if (Contains((char *)(buf.c_str()), (char *)"alen"))
			{
				if (DEBUG)
				{
					cerr << "skipping: " << buf << endl;
				}
				getline(in, buf, '<'); // end of data tag
				if (DEBUG)
				{
					cerr << "skipping: " << buf << endl;
				}
				getline(in, buf, '>'); // end of closing tag
				if (DEBUG)
				{
					cerr << "skipping: " << buf << endl;
				}
				pos = 0;
				continue;
			}
			if (Contains((char *)(buf.c_str()), (char *)"antibody_count"))
			{
				if (DEBUG)
				{
					cerr << "skipping: " << buf << endl;
				}
				getline(in, buf, '<'); // end of data tag
				if (DEBUG)
				{
					cerr << "skipping: " << buf << endl;
				}
				getline(in, buf, '>'); // end of closing tag
				if (DEBUG)
				{
					cerr << "skipping: " << buf << endl;
				}
				pos = 0;
				continue;
			}
		}
		if (DEBUG)
		{
			cerr << "category: " << buf;
		}
		string tmp = buf;
		getline(in, buf, '<'); // end of data tag
		if (DEBUG)
		{
			cerr << ", data: " << buf << endl;
		}

		/* HERE */

		if (Contains((char *)(tmp.c_str()), (char *)"/xml"))
		{
			// closing antibody tag
			pos ^= pos;
			++j;
			if (j == ab_count)
			{
				++i;
				j ^= 0;
			}
			++iter;
			continue;
		}
		if (Contains((char *)(tmp.c_str()), (char *)"xml"))
		{
			// opening antibody tag
			pos ^= pos;
			continue;
		}
		if (Contains((char *)(tmp.c_str()), (char *)"/"))
		{
			// closing tag
			continue;
		}
		if (Contains((char *)(tmp.c_str()), (char *)"flag"))
		{
			if (DEBUG)
			{
				cerr << "Got a flag" << endl;
				cerr << "pos / 4 = " << pos/4 << endl;
			}
			tmp_pop[i][j].setFlag(pos/4, atoi(buf.c_str()));
			++pos;
			continue;
		}
		else if (Contains((char *)(tmp.c_str()), (char *)"attribute"))
		{
			if (DEBUG)
			{
				cerr << "Got an attribute" << endl;
				cerr << "pos / 4 = " << pos/4 << endl;
			}
			tmp_pop[i][j].setAttr(pos/4, atoi(buf.c_str()));
			++pos;
			continue;
		}
		else if (Contains((char *)(tmp.c_str()), (char *)"offset"))
		{
			if (DEBUG)
			{
				cerr << "Got an offset" << endl;
				cerr << "pos / 4 = " << pos/4 << endl;
			}
			tmp_pop[i][j].setOffset(pos/4, atoi(buf.c_str()));
			++pos;
			continue;
		}
		else if (Contains((char *)(tmp.c_str()), (char *)"max"))
		{
			if (DEBUG)
			{
				cerr << "Got a max" << endl;
				cerr << "pos / 4 = " << pos/4 << endl;
			}
			tmp_pop[i][j].setMax(pos/4, atoi(buf.c_str()));
			++pos;
			continue;
		}
		else if (Contains((char *)(tmp.c_str()), (char *)"negative") && !(Contains((char *)tmp.c_str(), (char *)"false")))
		{
			if (DEBUG)
			{
				cerr << "Got a negative" << endl;
			}
			tmp_pop[i][j].setNeg(atoi(buf.c_str()));
			continue;
		}
		else if (Contains((char *)(tmp.c_str()), (char *)"positive") && !(Contains((char *)tmp.c_str(), (char *)"false")))
		{
			if (DEBUG)
			{
				cerr << "Got a positive" << endl;
			}
			tmp_pop[i][j].setPos(atoi(buf.c_str()));
			continue;
		}
		else if (Contains((char *)(tmp.c_str()), (char *)"positive"))
		{
			if (DEBUG)
			{
				cerr << "Got a false positive" << endl;
			}
			tmp_pop[i][j].setFalsePos(atoi(buf.c_str()));
			continue;
		}
		else if (Contains((char *)(tmp.c_str()), (char *)"negative"))
		{
			if (DEBUG)
			{
				cerr << "Got a false negative" << endl;
			}
			tmp_pop[i][j].setFalseNeg(atoi(buf.c_str()));
			pos ^= pos;
			continue;
		}
		else if (Contains((char *)(tmp.c_str()), (char *)"tmp") && !(Contains((char *)tmp.c_str(), (char *)"total")))
		{
			if (DEBUG)
			{
				cerr << "Got a category" << endl;
				cerr << "pos/2 = " << pos/2 << endl;
			}
			tmp_pop[i][j].setCatTotal(pos/2, atoi(buf.c_str()));
			++pos;
			continue;
		}
		else if (Contains((char *)(tmp.c_str()), (char *)"tmp"))
		{
			if (DEBUG)
			{
				cerr << "Got a category total" << endl;
				cerr << "pos/2 = " << pos/2 << endl;
			}
			tmp_pop[i][j].setCat(pos/2, atoi(buf.c_str()));
			tmp_pop[i][j].setCatPerc(pos/2, (float)(tmp_pop[i][j].getCatCount(pos/2))/(float)(tmp_pop[i][j].getCatTotal(pos/2)));
			++pos;
			continue;
		}

		/*
		   if (pos < alen)
		   {
		   switch(pos%4)
		   {
		   case 0:
		   tmp_pop[i][j].setFlag(pos, atoi(buf.c_str()));
		   ++pos;
		   break;
		   case 1:
		   tmp_pop[i][j].setAttr(pos, atoi(buf.c_str()));
		   ++pos;
		   break;
		   case 2:
		   tmp_pop[i][j].setOffset(pos, atoi(buf.c_str()));
		   ++pos;
		   break;
		   case 3:
		   tmp_pop[i][j].setMax(pos, atoi(buf.c_str()));
		   ++pos;
		   break;
		   }
		   }
		   else
		   {
		   cerr << "pos - alen = " << pos - alen << endl;
		   switch(pos - alen)
		   {
		   case 0:
		   tmp_pop[i][j].setTests(atoi(buf.c_str()));
		   ++pos;
		   break;
		   case 1:
		   tmp_pop[i][j].setPos(atoi(buf.c_str()));
		   ++pos;
		   break;
		   case 2:
		   tmp_pop[i][j].setFalsePos(atoi(buf.c_str()));
		   ++pos;
		   break;
		   case 3:
		   tmp_pop[i][j].setNeg(atoi(buf.c_str()));
		   ++pos;
		   break;
		   case 4:
		   tmp_pop[i][j].setFalseNeg(atoi(buf.c_str()));
		   ++pos;
		   break;
		   case 5:
		   tmp_pop[i][j].setTests(atoi(buf.c_str()));
		   ++pos;
		   break;
		   default:
		   cerr << "default case" << endl;
		   if (pos - alen - 6 < class_count)
		   {
		   if (((pos - alen) % 2) == 1)
		   {
		   if (!(tmp_pop[i][j].setCat(pos - alen - 6, atoi(buf.c_str()))))
		   {
		   cerr << "pos - alen - 6 is out of class count range\n";
		   }
		   }
		   else
		   {
		   if (!(tmp_pop[i][j].setCatTotal(pos - alen - 6, atoi(buf.c_str()))))
		   {
		   cerr << "pos - alen - 6 is out of class count range\n";
		   }
		   }
		   ++pos;
	}
		   else if (pos - alen - 6 == class_count + 1)
		   {
			   pos = 0;
		   }
		   else
		   {
			   ++pos;
		   }
		   break;
	}
	}

	getline(in, category, '>'); // end of closing tag
	if (in.good())
	{
		getline(in, category, '>'); // end of open tag
	}
	iter++;
	*/
	}

	if (DEBUG)
	{
		cerr << endl << "importxml end, got " << iter << " antibodies" << endl;
	}

	return (tmp_pop);
}

Antibody ** importChamps (char * fin)
{
	//string tmp(fin);
	//if (Contains((char *)(tmp.c_str()), (char *)".xml"))
	if (Contains(fin, (char *)".xml"))
	{
		return (importXml(fin));
	}
	return (import(fin));
}

