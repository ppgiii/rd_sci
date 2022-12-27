/*
C-code assessment 1 module
Links with the iritest.for module and a shared library .a of FORTRAN
codes for the IRI algorithms.

Data extracted from iritest.for are the electron density (Ne), m-3
and converted to plasma frequency (MHz) then plotted using GNU PLOT.

Several initial conditions (JF switch) have been defined but not used
in the hopes of using a different interface, irisub.for.

Due to time limitations for lack of understanding the options used in iritest.for,
the step size of the heights have been predetermined to be 54 using 
    min: 60 m -absolute minimum in the algorithm
    max: 600 m - from the sample plot
    step: 10 m - random choice of value

*/

#include <stdio.h>
#include <math.h>

// const defn here instead of .h const style since very few constants
#define JF_SWITCH 50
#define OARR_SIZE 100
#define OARR_LEN 1000
#define OUTF_SIZE 20
#define OUTF_LEN 1000
#define TEMP_LOOP 54    // hardcoded loop count for the height step

extern void iritest_(float [], float []);
// used Unix style for boolean initial conditions
typedef enum { 
    B_FALSE, 
    B_TRUE 
} boolean_t;

/*
assessgnu: use GNU PLOT to visualize data
           no array checking involve but will be added for future improvements

freq: 1-D array of plasma frequency values
hgt: 1-D array of heights
return: 0
*/
int assessgnu(float freq[], float hgt[]) {

    FILE *gnuplot = popen("gnuplot", "w");
    if (!gnuplot) {
        perror("popen");
        printf(" gnu launch error ");
    }
    // default options used
    fprintf(gnuplot, "plot '-' u 1:2 t 'Frequency Mar 3, 2021:1100' w lp\n");
    for (int i = 0; i < TEMP_LOOP; ++i) {
        fprintf(gnuplot,"%f %f\n", freq[i], hgt[i]);
    }
    fprintf(gnuplot, "e\n");

    fprintf(stdout, "Click Ctrl+d to quit...\n");
    fflush(gnuplot);
    getchar();

    pclose(gnuplot);
    
    return 0;
}

/*
To link with the IRI FORTRAN interface and extract the electron density per height.

JF switches as initial conditions identified and set.  These have assumptions that
were followed from iritest.for but not verified.  Not used with iritest.for but planned
for use with irisub.for.

External Function:
assessgnu()

*/
int main (void) {

    // ICs declared as boolean
    boolean_t compute_Ne = B_TRUE;
    boolean_t compute_Te_Ti = B_TRUE;
    boolean_t compute_Ni = B_TRUE;
    boolean_t f107 = B_FALSE;
    boolean_t f107a = B_FALSE;
    boolean_t f1_layer = B_TRUE;
    boolean_t bil2000 = B_FALSE;
    boolean_t NmF2 = B_FALSE;
    boolean_t hmF2 = B_FALSE;
        
    int jf[JF_SWITCH ];
    for (int i=0; i<JF_SWITCH ; i++)
        jf[i] = 1;

    // set IC for JS switches
    // taken from iritest.for listing of JF IC switches
    // FORTRAN vs C index: change to 0-based for C, FORTRAN equivalent will be 1 step higher
    jf[3] = 0;
    jf[4] = 0;
    jf[5] = 0;
    jf[20] = 0;
    jf[22] = 0;
    jf[27] = 0;
    jf[28] = 0;
    jf[29] = 0;
    jf[32] = 0;
    jf[34] = 0;
    jf[38] = 0;
    jf[39] = 0;
    jf[46] = 0;

    // output array
    float oarr[OARR_LEN][OARR_SIZE];
    for (int j=0; j<OARR_LEN ; j++)
        for (int i=0; i<OARR_SIZE ; i++)
            oarr[j][i] = 0.0;

    float outf[OUTF_LEN][OUTF_SIZE];

    // required input parameters
    // these are hard coded due to time, this assessment has been on trial for awhile
    // so deliberately setting the values as is to compare
    // against the test case ran versus iritest.for
    // description taken from irisub.for
    int jmag = 0;           // geographic
    float alati = 50.;      // LATITUDE NORTH AND LONGITUDE EAST IN DEGREES
    float along = 40.;
    float iyyyy = 2000.;
    float mmdd = 0101;      // data
    float dhour = 1.5;      // in UT, LOCAL TIME (OR UNIVERSAL TIME + 25) IN DECIMAL HOURS
                            // hard coded as Time_type = Universal
    float heibeg = 100.;        // HEIGHT RANGE IN KM; maximal 100 heights
    float heiend = 2000.;
    float heistp = 50.;

    printf("\nProducing results from iritest.for\n\n");
    // link to the FORTRAN interface
    float freq[TEMP_LOOP]; float hgt[TEMP_LOOP];
    iritest_(freq, hgt);

    for ( int i=0; i<TEMP_LOOP; i++) {
        freq[i] = sqrt(freq[i]*8.0640e-5*1.0e6f)/1.0e6f;
    }

    assessgnu(freq, hgt);

    return 0;
}