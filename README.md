# rd_sci  

* iri_edp  
  * directory containing FORTRAN code and data for the iri2016 algorithm (see irimodel.org)  
  * GNU Plot required, see gnuplot.info  
  * FORTRAN compiler & C compiler required - see NOTES & pdf file within  
  
* Temporal relationship: median filters  
  * file: median_filter.c  
  * GNU Plot required, see gnuplot.info  
  * compile with any C compiler, compatible from C99 standards  
  * to execute in Windows:> .\median_filter.exe \<input filename\>
  * output plots:  
         filtered-hmf2 (png file)  
         filtered-foF2 (png file)  
  
* Cluster of data sets  
  * file: rd_sci_clustering.ipynb  
  * to implement, load in a Jupyter notebook  
  
* Coordinate Transformation  
  * file: geom_dist.c  
  * C program calculates GIS coordinates (latitude, longitude) to radar coordinates (range, bearing) and vice versa.  
    The program outputs to std out for simplicity.  
  * compile with any C compiler, compatible from C99 standards  
  * to execute (Windows format):>  .\geom_dist.exe \<options\> \<filename\>  
  * see the documentation in the code  
  * input files:  
          sample-gis.txt  
          sample-radar.txt  
          
