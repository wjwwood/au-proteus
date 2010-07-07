/*
	Code that reads the formatted output of our java
	GCS. Data in that output is of the form:
	"Data 1: LAT LON ALT"
	Where "1" is the plane number.

	Data is then output to separate files for each plane
	for convenient exporting into a	kml file that will
	show the plane's route graphically in Google Earth. 
	The format for GPS coordinates in the	kml file is:
	"LON LAT ALT"

	The file output for each plane can then simply be
	copied and pasted into the portion of the kml file
	"<coordinates>
			GPS_DATA_HERE
	<\coordinates>"
Contributers: Chester Hamilton
							Varun Sampath
*/



#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main(int argc, char *argv[])
{
	if(argc < 2) {
		printf("Correct usage:\n\t\t%s [GCS output filename] [Number of planes] \nt\tOutput will create an 'out' file for each plane.\n",argv[0]);
		return -1;
	}

	char str [100];																			
	char str1 [11];																				
	char plane [5];
	int lon,lat,alt;
	FILE* GCS = fopen(argv[1],"r");
	int compare,count,i;
	if(GCS == NULL)	{
		return -1;
	}

	//Read in the number of planes involved
	//and create a pointer to a file pointer array
	//that has as many elements as the count
	sscanf(argv[2],"%d",&count);
	FILE **files = malloc (count * sizeof(FILE *));

	//Use "out" for the beginning of each filename
	//then append the plane number and open the file
	//for writing
	for (i=1;i <= count;i++) {
		strcpy(str1,"out");
		sprintf(plane,"%d",i);
		strcat(str1,plane);
		printf("Opening %s for writing\n",str1);
		files[i-1] = fopen(str1,"w");
		if(files[i-1] == NULL)	{
			return -1;
		}
	}
	
	//Main read-in loop
	while(!feof(GCS))	{
		fscanf(GCS, "%s",str);													//Read in a string
		compare = strncmp(str,"Data",4);								//check if it's "Data"
		if(compare==0) {																//If so:
			fscanf(GCS,"%s",str);													//read in the next string
			for(i=1;i<=count;i++)	{												//and check which plane's data
				sprintf(plane,"%d",i);											//follows
				strcat(plane,":");
				compare = strncmp(str,plane,6);	
				if(compare==0)	{														//Read in lat,lon,alt and then
					fscanf(GCS,"%d",&lat);										//write them to the appropriate file
					fscanf(GCS,"%d",&lon);										//in the appropriate format
					fscanf(GCS,"%d",&alt);
					fprintf(files[i-1],"%8f,%8f,%d\n",lon/1000000.0,lat/1000000.0,alt); 
				}
			}
		}
	}

 	//Close all files, release all used memory
	fclose(GCS);
	printf("GCS file closed\n");

	for (i=0;i<count;i++) {
		printf("attempting to close\n");
		fclose(files[i]);
		printf("Closed file out%d\n",i+1);
	}
	free(files);
	return 0;
}

