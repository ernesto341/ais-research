/* thanks:
 *
 * PherricOxide
 * http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
 */


#include <iostream>
#include <sys/stat.h>

using namespace std;

struct stat buffer;

inline bool Exists (const char ** name)
{
        return (stat (*name, &buffer) == 0); 
}

int main(void)
{
        char * n = (char *)"./../ais/lifetime.25\0";
        if (Exists((const char **)&n))
                cout << "TRUE\t./../ais/lifetime.25 does exist!" << endl;
        else
                cout << "FALSE\t./../ais/lifetime.25 does not exist!" << endl;
        n = (char *)"./ais/lifetime.25\0";
        if (Exists((const char **)&n))
                cout << "TRUE\t./ais/lifetime.25 does exist!" << endl;
        else
                cout << "FALSE\t./ais/lifetime.25 does not exist!" << endl;
        return (0);
}
