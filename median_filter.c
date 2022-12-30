/*
    Program: median filtering on electron density and peak density.
             Using a simple one dimensional median filtering algorithm,
             each of electron density and peak density is plotted as an
             unfiltered data against its filtered data.

             Input file requires inspection to determine row of relevant data,
             set as NUM_ROWS.  Using GNU Plot with default settings.

    Input:   data file (header, blank row, data rows)
    Output:  plots of electron density & peak density
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NUM_ROWS 467            // inspect input file to determine num of rows of relevant data (way to detect w/o inspection???)
#define FILE_NAME_LEN 256       // standard file name length
#define FLOAT_DATA 11
#define CHAR_DATE 11            // includes null terminate
#define CHAR_TIME 9
#define INT_SYM 6               // 3 digit symbol with parentheses + null
#define READ_BUF 1024           // read buffer size, default = 1024
#define DAT0 0                  // index location first data requested from provided file
#define DAT1 5                  // index location second data requested from provided file
#define MEDIAN_WIN_GUESS 3      // guess for the window size for the median filter range (?????)
                                //  smallest value to start but algorithm has no adjustment for larger guess

struct temporal
{
    char date[CHAR_DATE];
    char dig[INT_SYM];
    char timestmp[CHAR_TIME];
    int x;
    float d[FLOAT_DATA];
};

/*
Taken from: https://en.cppreference.com/w/c/algorithm/qsort
            qsort for int

Function: compareVals
          to sort input types in floats

return: int
*/
int compareVals(const void* a, const void* b)
{
    float arg1 = *(const float*)a;
    float arg2 = *(const float*)b;
 
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
 
    // return (arg1 > arg2) - (arg1 < arg2); // possible shortcut
    // return arg1 - arg2; // erroneous shortcut (fails if INT_MIN is present)
}

/*
Function: sortDate
          to sort a struct on its member
          upon return, the struct is updated post sort

return: int , status of strcmp
*/
int sortDate(const void *a, const void *b) {
    struct temporal *aa = (struct temporal *)a;
    struct temporal *bb = (struct temporal *)b;
    return strcmp( aa->date, bb->date );            // low level equivalent: (*aa).timestmp, (*bb).timestmp
}

/*
Function: filterPlot
          using gnu plot to plot the data to show the filtered portion over original

return: int
*/
int filterPlot(float dat[], float ftr[], int end, char *title) {

    FILE *gnuplot = popen("gnuplot -persistent", "w");
    if (!gnuplot) {
        perror("popen");
        printf(" gnu launch error ");
    }
    // default options used -- data cannot use int
    fprintf(gnuplot, "set title '%s'\n", title);
    fprintf(gnuplot, "plot '-' u 1:2 t 'unfiltered' w lp lt 0, '' u 1:2 t 'filtered' w lines lt 2\n");
    for (int i = 0; i < end; ++i) {
        fprintf(gnuplot,"%f %f\n", (float)(i), dat[i]);
    }
    fprintf(gnuplot, "e\n");
    for (int i = 0; i < end; ++i) {
        fprintf(gnuplot,"%f %f\n", (float)(i), ftr[i]);
    }
    fprintf(gnuplot, "e\n");

    fflush(gnuplot);
    pclose(gnuplot);
    
    return 0;
}

/*
Function: medianFilter
          computes a median filter using one dimensional method
          plots the unfiltered data against the filtered data
          based on: https://en.wikipedia.org/wiki/Median_filter

          For the one dimensional filter, starting with an input array:
            A[] = {7,8,2,1,3,6,5,7,4}   : dat[] -corresponding var in function
            window size = 3             : window_width
                                        : standard min default
            X[] = med(7,8,2)            : outputPixelValue -corresponding var in function
            qsort(2,7,8)
            X[] = {7}                   : continue for all length of array

          Boundary conditions:
            first index                 : edge -corresponding var in function
                                        : min value
            total array length          : end -corresponding var in function

return: int
*/
int medianFilter(float dat[], int end, char *filter_name) {
    int window_width = MEDIAN_WIN_GUESS;                    // the guess width of the filter window
    float outputPixelValue[end];                            // new filtered data
    // transfer data - for filtering
    for (int k=0; k<end; k++) {
        outputPixelValue[k] = dat[k]; 
    }
    float window[window_width];                             // hold the values in the window in question
    int edge = floor(window_width / 2);                     // rounded down
    int k;
    for (int i=edge; i<(end-edge); i++) {
        k = 0;
        for (int j=0; j<window_width; j++) {
            window[k++] = dat[i + j - edge];
        }
        qsort(window, window_width, sizeof(float), compareVals);
        outputPixelValue[i] = window[edge];
    }

    // call the gnu plot
    filterPlot(dat, outputPixelValue, end, filter_name);

    return 0;
}

/*
Function: readInputFile

reads the input file with the following assumptions:
input file inspected for header structure and number of rows for data to be read
header and blank line known to exist as first 2 rows so discarded
the rest of rows are data rows

the data is stored in a struct

return: int , value of last row
*/
int readInputFile(struct temporal *temporals, char *argv[]) {
    // read input file
    char filename[FILE_NAME_LEN];
    filename[0] = '\0';
    strcpy( filename, argv[1] );

    FILE *p;
    p = fopen(filename, "r");
    if (!p) {
        fprintf(stderr,"Cannot read file");
        perror(NULL);
        exit(1);
    }

    // upon inspection, input file has header then blank row
    // discard both
    // does not account missing header so it will treat a line as any other
    printf("\n-------------------------------------------------------------------\n");
    printf("Initiate read file: discard header and blank line (from inspection)");
    printf("\n-------------------------------------------------------------------\n");
    char header[READ_BUF]; header[0] = '\0';
    fgets(header, sizeof(header), p);
    // blank line
    fgets(header, sizeof(header), p);

    // read the file until EOF, i tracks the number of lines
    // read the file and transfer data to memory then close file
    char buf[READ_BUF];
    int i=0;
    while (fgets(buf,sizeof(buf),p) != NULL ) {
        strcpy(temporals[i].date, strtok(buf, " "));
        strcpy(temporals[i].dig, strtok(NULL, " "));
        strcpy(temporals[i].timestmp, strtok(NULL, " "));
        temporals[i].x = atoi(strtok(NULL, " "));
        temporals[i].d[0] = atof(strtok(NULL, " \n"));
        temporals[i].d[1] = atof(strtok(NULL, " \n"));
        temporals[i].d[2] = atof(strtok(NULL, " \n"));
        temporals[i].d[3] = atof(strtok(NULL, " \n"));
        temporals[i].d[4] = atof(strtok(NULL, " \n"));
        temporals[i].d[5] = atof(strtok(NULL, " \n"));
        temporals[i].d[6] = atof(strtok(NULL, " \n"));
        temporals[i].d[7] = atof(strtok(NULL, " \n"));
        temporals[i].d[8] = atof(strtok(NULL, " \n"));
        temporals[i].d[9] = atof(strtok(NULL, " \n"));
        temporals[i].d[10] = atof(strtok(NULL, " \n"));
        i++;
    }
    fclose(p);
    int end = i;          // holding the last index with data, i holds total after the post incr
    
    return end;
}

/*
Function: main
          to read the data, sort by date/time, compute median filter on fof2 & hmf2

return: int
*/
int main(int argc, char *argv[]) {

    struct temporal temporals[NUM_ROWS];

    int end = readInputFile(temporals, argv);

    // ***  sort data, compute data ***

    // the date already starts at the largest number (year) to lower number
    // time starts at largest number(hour) to lower numbers
    // taking advantage of this format, combine date/time to make it continuous char for qsort
    // store in new struct
    struct new_temporal {
        char date_t[CHAR_DATE+CHAR_TIME];
        float fof2; float hmf2;
    } date_time[end];                           // num of rows

    // storing date/time for other uses, i.e. debugging, inspection, verification
    // main data: fof2 & hmf2 columns
    for (int k=0; k<end; k++) {
        strcpy(date_time[k].date_t,temporals[k].date);
        strcat(date_time[k].date_t,".");
        strcat(date_time[k].date_t,temporals[k].timestmp);
        date_time[k].fof2 = temporals[k].d[DAT0];
        date_time[k].hmf2 = temporals[k].d[DAT1];
    }

    // using built in C , stdlib.h, qsort 
    qsort(date_time, end, sizeof(struct new_temporal), sortDate);

    // compute: median filter for fof2, hmf2
    float fof2[end]; float hmf2[end];
    // prep for the generic median filter function, pass only intended data for filtering
    for (int k=0; k<end; k++) {
        fof2[k] = date_time[k].fof2;
        hmf2[k] = date_time[k].hmf2; 
    }
    // compute median filter
    medianFilter(fof2, end, "foF2");
    medianFilter(hmf2, end, "hmF2");

    printf("\n");

    return 0;
}