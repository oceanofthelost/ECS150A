 #include <stdio.h>
#include <stdlib.h>

int main(void)
{
	FILE *f1 = fopen("./sss.txt", "w");
	int i;

	for (i = 0; i < 1000; i++)
	{
		fseek(f1, rand(), SEEK_SET);
		fprintf(f1, "%d%d%d%d", rand(), rand(),rand(), rand());
		if (i % 100 == 0) 
		{
			sleep(1);
		}
	}
	fflush(f1);
}
