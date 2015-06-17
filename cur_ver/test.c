#include <stdio.h>
#include <lib/base64.h>

int main()
{
	char buf[256];
	memset(&(buf[0]), '\0', 256 * sizeof(char));

	while (getline(&(buf[0]), 255, stdin) < 1)
	{
		fprintf(stderr, "Insufficient input, try again\n");
		fflush(stderr);
		memset(&(buf[0]), '\0', 256 * sizeof(char));
	}

	printf("Got: %s\n", buf);

}
