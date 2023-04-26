//	compile command
// 		gcc -Wall -O3 -o parseTimes parseTimes.c -lm

//	run command
// 		./parseTimes [time_1] [time_2] [time_3] [time_4] [time_5]

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Simple mean and standard deviation times parser
int main(int argc, char *argv[])
{
	if (argc != 6)
	{
		printf("Invalid number of arguments\n");
		return 1;
	}
	
	double meanTime = 0.0;
	double standardDeviationTime = 0.0;
	
	for (int i = 0; i < 5; i++)
		meanTime += atof(argv[i + 1]);
	meanTime /= 5;
	
	for (int i = 0; i < 5; i++)
		standardDeviationTime += (atof(argv[i + 1]) - meanTime) * (atof(argv[i + 1]) - meanTime);
	standardDeviationTime /= 5;
	standardDeviationTime = sqrt(standardDeviationTime);
	
	printf("\nMean time = %.6f s\n", meanTime);
	printf("Standard deviation time = %.6f s\n", standardDeviationTime);
	
	return 0;
}