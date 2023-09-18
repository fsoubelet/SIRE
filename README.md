# SIRE: Software for IBS and Radiation Effects

The SIRE code was inspired by MOCAC (MOnte CArlo Code).
After specifying the beam distribution and the optics along a lattice, SIRE iteratively computes intra-beam collisions between pairs of macro-particles.
If requested, it also evaluates the effects of synchrotron radiation damping and quantum excitation.
The beam distribution is updated and the RMS beam emittances are recomputed, giving finally as output the emittance evolution in time. 

* Developed by A. Vivoli (2010)
* Revised by F. Antoniou (2014)
* Revised by Stefania Papadopoulou (2016)


**NOTE**: This code is not maintained and has not been for a long time. This repository acts as a save and working point start for my own work on IBS, which will take inspiration from SIRE and use it for benchmarks.

## HOW TO COMPILE AND RUN THE CODE

1. Compile the code with, for instance, `gcc`: `g++ sire.c -o sire`
2. Run the code, providing a TWISS file (e.g. from `MAD-X`), a parameters file and a target for the outputs:
```bash
./sire twiss.tfs params_file.dat outputfile_name
```

A fourth argument may be provided, in which case it is assumed to be the input distribution:
```bash
./sire twiss.tfs params_file.dat outputfile_name distribution.txt
```

The specifications for the various arguments are as follows:

1. The `MAD-X` TWISS file needs columns ordered and named: `name,s1,len1,betx1,alphax1,mux1,bety1,alphay1,muy1,dx1,dpx1,dy1,dpy1`. I personally heavily recommend using our [tfs-pandas](https://github.com/pylhc/tfs) package to work with your `TFS` file should you need to.

2. The input parameters file needs the following fields:
```txt
TEMPO: The full time length of the simulation.
nturnstostudy: The total number of turns.
NIBSruns: The number of timesteps you devide the TEMPO to calculate IBS.
oneturn: A boolean integer, 1 for 1-turn calculation, 0 for multi-turn
fastrun: A boolean integer, 1 if interpolation is used every TIMEINJ, NIBSruns determines how many turns are skipped for the IBS calculation. So be careful to have a sufficient NIBSruns for a specific time.
continuation: A boolean integer, 1 so that if a job is killed it continues from where it stopped, 0 otherwise.
epsx: The starting (geometrical!) horizontal emittance.
epsz: The starting (geometrical!) vertical emittance.
delta: The starting energy spread.
deltas: The starting bunch length. It should be given in [m] for the 1 sigma, then for 1ns blength (4sigma) in the paramfile we should put blength=(1ns/4)*clight. Whenever changing blength -> change also en.spread
dtimex: The horizontal damping time.
dtimez: The vertical damping time.
dtimes: The longitudinal damping time.
eqx: The equilibrium horizontal emittance.
eqz: The equilibrium vertical emittance.
eqdelta: The equilibrium energy spread.
eqdeltas: The equilibrium bunch length.
flag_rec: A boolean integer, 0 if full lattice, 1 to use a smaller number of lattice points.
damping: A boolean integer, 1 to take into account the radiation damping, 0 to ignore it.
IBSflag: A boolean integer, 1 to perform the IBS calculations, 0 to ignore them.
q_ex: A boolean integer, 1 to take into account the quantum excitation, 0 to ignore it.
momentum: The beam momentum in [MeV].
massparticle: The beam particle's mass in [MeV].
chargeparticle: The charge of the particle (for instance 1 for e-).
numbunch: The bunch population.
numpart: The number of macroparticles.
ncellx: The number of cells to use in the horizontal plane.
ncellz: The number of cells to use in the vertical plane.
ncells: The number of cells to use in the longitudinal plane.
ncollisions: The number of collisions per particle (close encounters).
convsteadystate: Track until convergeance to steady-state not in full TEMPO time.
checktime: A boolean integer, 0 if you want to see the TEMPO, 1 to see the turns instead of the TEMPO.
```



in sire code:
	precision: tells you how tha reccurences work. Elements of the lattice with twiss functions differing of less than prec.% are considered equal.
	The closer it is to 1->more reccurences->shorter lattice->less time-> but also less accurancy. The closer it is to 0->less recurences->higher accurancy.
	
	renormbinningtrans=1,renormbinningall=1 sees ncellx and ncellz as f(ncells) using the mppercell
	mppercell=5 is the #mp/cell


1. Output files using the arg3 for the naming of the output files:
  RES_TWISS_arg3.txt --> The new twiss file after recurrences
  RES_EMITTANCE_arg3.txt --> The emittance at each point of the lattice after the IBS kicks (for 1-turn calculations only). Four columns (s, exm, ezm, esm)
  RES_Growth_Rates_arg3.txt --> File with emittances for each time step! The Growth rates are the zeroed columns, so they are not saved. 
				    In this file, L{1}=timesteps (so the NIBSruns), L{2},L{3}=the emittances and energyspread=sqrt(L{4}/2).
  RES_DISTRIB_arg3.txt --> The distribution file, that has a length=#mp.
  we can also ask for the output distribution.
  

*****************************
*                           *
* MORE COMMENTS AND DETAILS *
*                           *
*****************************

1. Structure of main: 
  a. Reads madx and input parameters files and generates the names of the output files
  b. if flag_rec then optimizes the number of points in the lattice to be used in order to speed up the simulation --> Generates the new twiss file
  c. Generates the distribution of macroparticles given the mean invariants values and random phases
  d. if the IBSflag=1 then apply the transformation to the momentum frame, then apply the IBS routine in each point of the lattice, transform back to the invariants frame
  e. If oneturn=1 the above calculation is done only once and the one-turn emittance evolution around the lattice is writen in an output file
  e. If oneturn=0 the tracking of the distribution evolution is followed for every element anf for each turn for a timestep TEMPO or until convergance. The mean growth times and mean emittances are calculated and written in an output file every one timestep TIMEINJ
  f. If damping=1 and q_ex=1 then the effects of synchrotron radiation damping and quantum excitation are taken into account.
  
2.  How the IBS routine works:
  a. Finds the limits of the distributions
  b. The distributions are splitted in cells (uniform split with respect to x, z, s)
  c. Puts the macroparticles in cells and calculates the number of macroparticles and the density of mp in each cell
  d. Applies the scattering kicks between the macroparticles in each cell (binary collisions) --> redestribution of phase space
  
3. How the fastrun is applied: 
  a. The IBS is applied in oneturn and the growth rate per particle is calculated
  b. The emittance per particle is recalculated based on the exponential IBS growth in a timestem TIMEINJ. 
  c. contunue tracking with the interpolated distribution

  Comment: The fastrun is applied in order to speed up the simulation. The user has to check if it gives valid results   
  
  
