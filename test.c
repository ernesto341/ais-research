#include <stdio.h>
#include <signal.h>

int main()
{
        printf("sizeof(int) = %d\r\n", sizeof(int));
        printf("sizeof(sig_atomic_t) = %d\r\n", sizeof(sig_atomic_t));
        return (0);
}
