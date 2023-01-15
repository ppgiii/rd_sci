/*  geom_dist
    Geometric distance program that converts GIS coordinates (latitude & longitude) 
    to radar coordinates (range & bearing) and vice versa.

    The distance formula is taken from a web site that uses the Haversine equation to calculate the 
    range and bearing.  There are some simplifications and assumptions made for this program as stated in
    the web site.  For the bearing, only current bearing is considered.

    The input is a file but the function can be modified or extended to add other methods of input.
    The input file only has 2 lines, header and data, comma separated (can be modified).  The header 
    identifies what the data structure looks like.  No other checks or error traps are used.
    For both types of file, the first two data are starting coordinates in latitude and longitude.  
    The next 2 depends on the coordinates.

    Two input files are created: one for GIS coordinates and the second for radar coordinates.
    For GIS coordinates, the data line format: xxxN, xxxN, xxxN, xxxN
        where N represents N, E, W, or S
        the first two for the starting coordinates, the last two for ending coordinates
    For radar coordinates, the data line format: xxxN, xxxN, yyy.yy, yyy.yy
        the first two for the starting coordinates, the last two:
            yyy.yy: range in km
            yyy.yy: bearing in degrees, 0-360

    The conversion assumes that the starting coordinates is always in GIS format.  This format only uses
    degrees but the function can be modified or extended to handle minutes/seconds.

    The user is asked to use options to determine which conversion to use:
        -G : conversion from GIS coordinates to radar coordinates
        -R : conversion from radar coordinates to GIS coordinates

    All variables using float since single precision is enough for this exercise.
    Parameter inputs set to call by value to avoid modifications of originals unless set to call by reference.
    Degrees measured from true north and coordinates are in whole degrees only.

    To use:
    geom_dist <option> <input file>

    Sample input file 1, using option -G, GIS (latitude, longitude) to radar (range, bearing) coordinates:
        Initial coordinates (latitude, longitude), Final coordinates (latitude, longitude)
        37N, 75W, 18N, 66W

    Sample output for the above:
        The range in decimal coordinates between the
                        starting coordinates  37 latitude and -75 longitude
        and             final coordinates  18 latitude and -66 longitude
        is              2288.66 kilometers
        with a          bearing of 154.96 degrees.

    Sample input file 2, using option -R, radar (range, bearing) coordinates to GIS (latitude, longitude):
        Initial coordinates (latitude, longitude), range (km), bearing (degrees)
        37N, 75W, 2288.66, 154.96

    Sample output for the above:
        From starting GIS coordinates of         37 latitude and -75 longitude
        with a range of                         2288.66 kilometers 
        and a bearing of                        154.96 degrees.
        The final coordinates are               18N and 66W.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
// this math.h header comes from GNU, using M_PI
#include <math.h>

#define CSV ", "                // define the format for file reading - must inspect file for delimeter useage
#define COORD 8                 // degrees only (0-360 value) plus direction (N, E, W, S)
                                // for radar, it combines range of yyyy.yy and bearing yyy.yy, max digits
                                // with null terminator
#define READ_BUF 1024           // read buffer size, default = 1024
#define FILE_NAME_LEN 256       // standard file name length
#define EARTH_RADIUS 6371       // mean Earth radius, simplified, in km
#define CONVR 1                 // conversion, here between km to meters, default is km

const char *clArgs = "G:R";     // command line options

struct geographic {
	float lat;
	float lon;
};

struct head {
	float bearing;
	float range;
};
// assigning unity or negation based on convention used for geographic coordinates
enum direction {
    N = 1, E = 1, W = -1, S = -1
};

/*  haversine
    This formula is taken from https://www.movable-type.co.uk/scripts/latlong.html
    Inputs are in radians and distance in kilometers.
    Calculating the range between 2 decimal GIS coordinates.

    Input:  lat_start, lon_start    - starting coordinates in latitude/longitude in radians
            lat_dest, lon_dest      - destination coordinates in latitude/longitude in radians
    Output: distance in km
*/
float haversine(float lat_start, float lon_start, float lat_dest, float lon_dest) {
    /* Haversine formula:
                        a = sin²(Δφ/2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ/2)
                        c = 2 ⋅ atan2( √a, √(1−a) )
                        d = R ⋅ c
                        where	φ is latitude, λ is longitude, R is earth’s radius (mean radius = 6,371km);
                        φ1 = lat_start // φ, λ in radians
                        φ2 = lat_dest
                        Δφ = lat_dest - lat_start
                        Δλ = lon_dest - lon_start
    */
    float R = EARTH_RADIUS * CONVR;              // allow placeholder to convert to other measurement
    float delta_theta = lat_dest - lat_start;
    float delta_lambda = lon_dest - lon_start;
    float a = pow(sin(delta_theta/2.), 2.);
    float b =  pow(sin(delta_lambda/2.), 2.);
    float c = a + cos(lat_start) * cos(lat_dest) * b;
    float d = 2.0 * atan2(sqrt(c), sqrt(1-c));
    return (R*d);
}

/*  bearing
    This formula is taken from https://www.movable-type.co.uk/scripts/latlong.html
    Simplified computation using current heading to find the bearing in degrees.
    The initial GIS coordinates in decimal form is used to convert to radians.

    Input:  lat_start, lon_start    - starting coordinates in latitude/longitude in radians
            lat_dest, lon_dest      - destination coordinates in latitude/longitude in radians
    Output: bearing in degrees
*/
float bearing(float lat_start, float lon_start, float lat_dest, float lon_dest) {
    /*  Formula:	
                θ = atan2( sin Δλ ⋅ cos φ2 , cos φ1 ⋅ sin φ2 − sin φ1 ⋅ cos φ2 ⋅ cos Δλ )
                where	φ1,λ1 is the start point, φ2,λ2 the end point (Δλ is the difference in longitude)
                (all angles in radians)
    */
    float a = sin(lon_dest - lon_start);
    float b = cos(lon_dest - lon_start);
    float y = a * cos(lat_dest);
    float x = cos(lat_start)*sin(lat_dest) - sin(lat_start)*cos(lat_dest)*b;
    float theta = atan2(y,x);
    return fmod((theta*180./M_PI) + 360., 360.);     // degrees - floating point modulu for extra sig figs
}

/*  GIS2Radar
    {For this exercise, this utility will print the converted values to std out.}
    This function converts the decimal input initial GIS coordinates & final GIS coordinates
    into range & bearing.

    Bearing is simplified to only use the current heading.

    Input:  decimal form
            init_gis: struct type- initial GIS coordinates
            final_gis: struct type- final GIS coordinates
    Output: None
*/
void GIS2Radar(struct geographic init_gis, struct geographic final_gis) {
    // convert to radians
    float lat = init_gis.lat * (M_PI/180.);
    float lon = init_gis.lon * (M_PI/180.);
    float lat_dest = final_gis.lat * (M_PI/180.);
    float lon_dest = final_gis.lon * (M_PI/180.);

    printf("\n");
    // default output to std out, can be modified per user requirements
    printf("The range in decimal coordinates between the \n\t\tstarting coordinates %3.0f latitude and %3.0f longitude\n", init_gis.lat, init_gis.lon);
    printf("and \t\tfinal coordinates %3.0f latitude and %3.0f longitude\n", final_gis.lat, final_gis.lon);

    // get range
    printf("is \t\t%.2f kilometers \n", haversine(lat, lon, lat_dest, lon_dest) );

    // get bearing
    printf("with a \t\tbearing of %.2f degrees.\n", bearing(lat, lon, lat_dest, lon_dest) );

    printf("\n");
}

/*  strCoord
    Changing the value into a string for std out.

    Input:  coord - coordinates in degrees
    Output: str - coordinates in string format, modifying negative into south or west
*/
int strCoord(float coord, char *str) {
    // for latitude, negative value is south (S)
    // for longitude, negative value is west (W)

    // temp variable
    char temp[COORD];
    coord = round(coord);           // for degrees only, change for other formats
    // determine if negative value
    if (0 > coord) { 
        coord = -1.*coord;          // remove '-'
        sprintf(temp, "%g", coord); // cast float to str
        strcpy(str, temp);
        return -1;
    }
    else {
        sprintf(temp, "%g", coord);
        strcpy(str, temp);
        return 1;
    }
    return 0;
}

/*  displayCoord
    Using a different method of std out.  This module only displays the output and
    can be modified for a different output type.

    Input:  init_gis - struct type for the initial GIS coordinates
            radar    - struct type for the range & bearing
            lat_fin, lon_fin - final GIS coordinates
    Output: None
*/
void displayCoord(struct geographic init_gis, struct head radar, float lat_fin, float lon_fin) {
    // convert decimal direction to str
    char str[] = "\0";

    printf("\n");
    printf("From starting GIS coordinates of \t%3.0f latitude and %3.0f longitude\n", init_gis.lat, init_gis.lon);
    printf("with a range of \t\t\t%.2f kilometers \n", radar.range);
    printf("and a bearing of \t\t\t%.2f degrees.\n", radar.bearing);

    if (strCoord(lat_fin, str)>0) { strcat(str, "N"); }
    else                          { strcat(str, "S"); }
    printf("The final coordinates are \t\t%s ", str);
    if (strCoord(lon_fin, str)>0) { strcat(str, "E"); }
    else                          { strcat(str, "W"); }
    printf("and %s.\n", str);
    printf("\n");
}

/*  finalCoord
    This formula is taken from https://www.movable-type.co.uk/scripts/latlong.html
    Converting initial GIS coordinates with a range & bearing to a final GIS coordinates.

    All input angles in radians then final output in degrees.

    Input:  lat, lon - initial GIS coordinates
            range, bearing - given radar coordinates
    Output: lat_fin, lon_fin
*/
void finalCoord(float lat, float lon, float range, float bearing, float *lat_fin, float *lon_fin) {
    /*  Formula:	φ2 = asin( sin φ1 ⋅ cos δ + cos φ1 ⋅ sin δ ⋅ cos θ )
                    λ2 = λ1 + atan2( sin θ ⋅ sin δ ⋅ cos φ1, cos δ − sin φ1 ⋅ sin φ2 )
                    where	φ is latitude, λ is longitude, θ is the bearing (clockwise from north), 
                    δ is the angular distance d/R; d being the distance travelled, R the earth’s radius

        longitude normalised: (lon+540)%360-180
        All angles in radians.
    */
    float R = EARTH_RADIUS * CONVR;              // allow placeholder to convert to other measurement
    float delta = range/R;
    *lat_fin = asin(sin(lat)*cos(delta) + cos(lat)*sin(delta)*cos(bearing));
    float x = sin(bearing)*sin(delta)*cos(lat);
    float y = cos(delta)-sin(lat)*sin(*lat_fin);
    *lon_fin = lon + atan2(x,y);
    // convert to degrees for final output
    *lat_fin = *lat_fin * 180./M_PI;
    *lon_fin = fmod((*lon_fin * 180./M_PI)+540, 360.) - 180.;
}

/*  RtoG
    Radar coordinates to GIS coordinates conversion module.
    This module interfaces the input parameters for computation into 
    final GIS coordinates.

    Angle inputs as degrees converted to radians.

    Input:  init_gis - struct type for initial GIS coordinates
            radar    - struct type for range & bearing coordinates
    Output: None
*/
void RtoG(struct geographic init_gis, struct head radar) {
    // convert to radians
    float lat = init_gis.lat * (M_PI/180.);
    float lon = init_gis.lon * (M_PI/180.);
    float bearing = radar.bearing * (M_PI/180.);

    // local variables for passing to functions
    float lat_final; float lon_final;
    finalCoord(lat, lon, radar.range, bearing, &lat_final, &lon_final);
    displayCoord(init_gis, radar, lat_final, lon_final);
}

/*  news
    Extracts the last char of the input data, determines which geographic direction
    to identify a negation, and converts the char type into int.
    This assumes a simplified coordinates in degrees as whole value.

    By convention, south (S) and west (W) are negative in decimal notation.

    Input:  geographic coordinates
    Output: value - negative for coordinates in south (S) or west (W)
                  - positive for coordinates in north (N) or east (E)
*/
float news(char *coord) {
    // set to default: no sign
    enum direction d;
    d = N;
    float value = d;        // type casting

    // get the heading letter
    int len = strlen(coord);
    char c = coord[len-1];
    // store values only, in char type
    char newc[len]; size_t i;
    for (i=0; i<len-1; i++) {
        newc[i] = coord[i];
    }
    newc[i] = '\0';     // null terminator

    // convert to values plus based on direction
    switch (c) {
        case 'S':
        case 'W':   d = W;
                    value = d*atoi(newc);       // degrees in whole number
                    break;
        default:    value = d*atoi(newc);       // positive direction
                    break;
    }

    return value;
}

/*  coordUtility
    This interface converts the input data into computed data.  Based on the user selection,
    each utility is called accordingly.

    Inputs: command_stat: call by value to avoid modifying original, user desired conversion option
            i_lat, i_lon: starting latitude/longitude
            coord_a, coord_b: next 2 inputs based on the type of conversion
                    for GIS to radar: the final latitude/longitude coordinates
                    for radar to GIS: range (coord_a) & bearing (coord_b) data
    Output: None
*/
void coordUtility(int command_stat, char *i_lat, char *i_lon, char *coord_a, char *coord_b) {

    switch (command_stat) {
        case 1:
                struct geographic init_gis, final_gis;
                init_gis.lat = news(i_lat);
                init_gis.lon = news(i_lon);
                final_gis.lat = news(coord_a);
                final_gis.lon = news(coord_b);
                // call converter
                GIS2Radar(init_gis, final_gis);
                break;
        case 2:
                struct geographic start_gis;
                struct head radar;
                start_gis.lat = news(i_lat);
                start_gis.lon = news(i_lon);
                radar.range = atof(coord_a);
                radar.bearing = atof(coord_b);
                // call converter
                RtoG(start_gis, radar);
                break;
        default:
                break;
    }

}

/*  readInputFile
    Read the input file.  This function is specific to this type of implementation.
    This can be kept or modified if other types of input are needed.

    The input file structure is as follows:
    header
    data

    "data" is split into 4 sections, separated by a delimeter as defined above.  This is
    file specific so requires inspection prior to executing this program.
    The 4 sections are describe as output variables below.

    Input:  file
    Output: i_lat, i_lon: starting latitude/longitude
            coord_a, coord_b: next 2 inputs based on the type of conversion
                    for GIS to radar: the final latitude/longitude coordinates
                    for radar to GIS: range/bearing data
*/
void readInputFile(char *argv[], char i_lat[], char i_lon[], char coord_a[], char coord_b[]) {
    // read input file
    // read the file and transfer data to memory then close file
    char filename[FILE_NAME_LEN];
    filename[0] = '\0';
    strcpy( filename, argv[2] );

    FILE *p;
    p = fopen(filename, "r");
    if (!p) {
        printf("Cannot read file");
        perror(NULL);
        exit(1);
    }

    // input file format has header then data, discard header
    // does not account missing header or any other error checks
    char header[READ_BUF]; header[0] = '\0';
    fgets(header, sizeof(header), p);
    // read data, only 1 line for this exercise
    char buf[READ_BUF];
    while (fgets(buf,sizeof(buf),p) != NULL ) {
        strcpy(i_lat, strtok(buf, CSV));
        strcpy(i_lon, strtok(NULL, CSV));
        strcpy(coord_a, strtok(NULL, CSV));
        strcpy(coord_b, strtok(NULL, CSV));
    }

    fclose(p);
}

/*  getComOptions
    Check for correct user options and determine which option to do the conversion.

    Input:  user option and file input
    Output: -1 -for incomplete command use
             0 -function executed, mainly for status check
             1 -GIS to radar conversion selected
             2 -radar to GIS conversion selected
*/
int getComOptions(int argc, char *argv[]) {
    // check options: geometric coordinates or range/bearing navigation
    if( argc < 3 )
    {
        printf("Usage: geom_dist <options> <filename>\n");
        printf("\tthere are 2 options, -G or -R\n");
        printf("\t-G to convert (GIS coordinates) gemoetric latitude and longitude coordinates to range and bearing (radar coordinates)\n");
        printf("\t-R to convert (radar coordinates) range and bearing to latitude & longitude (GIS coordinates)\n");
        return -1;
    }

    int c;
    while( ( c = getopt( argc, argv, clArgs ) ) != EOF )
    {
        switch ( c ) 
        {
            case 'G':       // int value for this option
            {
                return 1;   break;
            }
            case 'R':       // int value for this option
            {
                return 2;   break;
            }
            default:
            {   
                printf("Option not found.\n");
                printf("Usage: geom_dist <options> <filename>\n");
                printf("\t2 options allowed: -G or -R\n");
                break;
            }
        }
    }

    return 0;
}

/*  main
    Start of program.  Handles the input and pass the user data for computation.
    Error handling and checking not added but placeholder for addition.

    Input:  user options and file input
    Output: None
*/
int main(int argc, char *argv[]) {
    // local data to store:
    //      i_lat, i_lon: starting latitude/longitude
    //      coord_a, coord_b: next 2 inputs based on the type of conversion
    //                for GIS to radar: the final latitude/longitude coordinates
    //                for radar to GIS: range/bearing data
    char i_lat[COORD]; char i_lon[COORD];
    char coord_a[COORD]; char coord_b[COORD];
    // get options and use status commmand to determine conversion
    int command_stat = getComOptions(argc, argv);
    if (-1 != command_stat) {
        readInputFile(argv, i_lat, i_lon, coord_a, coord_b);
        coordUtility(command_stat, i_lat, i_lon, coord_a, coord_b);
    }
    // else: any error check or message

    return 0;
}