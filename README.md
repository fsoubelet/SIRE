# SIRE: Software for IBS and Radiation Effects

The `SIRE` code was inspired by `MOCAC` (MOnte CArlo Code).
After specifying the beam distribution and the optics along a lattice, `SIRE` iteratively computes Intra-Beam Scattering (IBS) collisions between pairs of macro-particles.
If requested, it also evaluates the effects of synchrotron radiation damping and quantum excitation.
The beam distribution is updated and the rms beam emittances are recomputed, giving finally as output the emittance evolution in time.

* Developed by A. Vivoli (2010)
* Revised by F. Antoniou (2014)
* Revised by Stefania Papadopoulou (2016)

<details>
  <summary><b>Note: Code Status</b></summary>

  The code is not maintained and has not been for a long time. This repository acts as a save and working point start for my own work on IBS, which will take inspiration from SIRE as well as use it for benchmarks.

</details>

## How to Compile and Run the Code

1. Compile the code with, for instance, `gcc`: `g++ sire.c -o sire`. The same would be done with a different compiler by changing the invoked executable (for instance `g++` -> `clang`).
2. Run the code, providing a TWISS file (e.g. from `MAD-X`), a parameters file and a target for the outputs:
```bash
./sire twiss.tfs params_file.dat outputfiles_naming
```

A fourth argument may be provided, in which case it is assumed to be the input distribution:
```bash
./sire twiss.tfs params_file.dat outputfiles_naming distribution.txt
```

The specifications for the various arguments are as follows:

1. The `MAD-X` TWISS file needs columns ordered and named: `name,s1,len1,betx1,alphax1,mux1,bety1,alphay1,muy1,dx1,dpx1,dy1,dpy1`. I personally recommend using our [tfs-pandas](https://github.com/pylhc/tfs) package to work with your `TFS` file, should you need to in order to get the proper input.

2. The input parameters file needs the following fields:
```txt
TEMPO:           The full time length of the simulation.
nturnstostudy:   The total number of turns.
NIBSruns:        The number of timesteps you devide the TEMPO to calculate IBS.
oneturn:         A boolean integer, 1 for 1-turn calculation, 0 for multi-turn
fastrun:         A boolean integer, 1 if interpolation is used every time step TIMEINJ, and the NIBSruns parameter determines how many turns are skipped for the IBS calculation. So be careful to have a sufficient value for NIBSruns for a specific simulation time.
continuation:    A boolean integer, 1 so that if a job is killed it continues from where it stopped, 0 otherwise.
epsx:            The starting (geometrical!) horizontal emittance.
epsz:            The starting (geometrical!) vertical emittance.
delta:           The starting energy spread.
deltas:          The starting bunch length. It should be given in [m] for the 1 sigma, then for 1ns blength (4sigma) in the paramfile we should put blength=(1ns/4)*clight. Whenever changing blength -> change also en.spread
dtimex:          The horizontal damping time.
dtimez:          The vertical damping time.
dtimes:          The longitudinal damping time.
eqx:             The equilibrium horizontal emittance.
eqz:             The equilibrium vertical emittance.
eqdelta:         The equilibrium energy spread.
eqdeltas:        The equilibrium bunch length.
flag_rec:        A boolean integer, 0 if full lattice, 1 to use a smaller number of lattice points.
damping:         A boolean integer, 1 to take into account the radiation damping, 0 to ignore it.
IBSflag:         A boolean integer, 1 to perform the IBS calculations, 0 to ignore them.
q_ex:            A boolean integer, 1 to take into account the quantum excitation, 0 to ignore it.
momentum:        The beam momentum in [MeV].
massparticle:    The beam particle's mass in [MeV].
chargeparticle:  The charge of the particle (for instance 1 for e-).
numbunch:        The bunch population.
numpart:         The number of macro-particles.
ncellx:          The number of cells to use in the horizontal plane.
ncellz:          The number of cells to use in the vertical plane.
ncells:          The number of cells to use in the longitudinal plane.
ncollisions:     The number of collisions per particle (close encounters).
convsteadystate: Track until convergeance to steady-state not in full TEMPO time.
checktime:       A boolean integer, 0 if you want to see the TEMPO, 1 to see the turns instead of the TEMPO.
```

One may wish to modify the following in the `SIRE` code itself (`sire.c`):
```txt
precision:              Determines how the recurrences work: elements of the lattice with twiss functions differing of less than precision % are considered equal. The closer it is to 1 -> more recurrences -> shorter lattice -> less computation time but also less accuracy. The closer it is to 0 -> less recurrences -> higher accuracy.
renormbinningtrans(=1): A boolean integer. If set to 1, sees ncellx as f(ncells) using the mppercell value.
renormbinningall(=1):   A boolean integer. If set to 1, sees ncellz as f(ncells) using the mppercell value.
mppercell(=5):          The number of macro-particles per cell. Determines the number of created cells.
```

3. The output files are named based on the third provided argument:
```txt
RES_TWISS_arg3.txt:        The new twiss file after recurrences.
RES_EMITTANCE_arg3.txt:    The emittance at each point of the lattice after the IBS kicks (for 1-turn calculations only). This contains four columns: s, exm, ezm and esm.
RES_Growth_Rates_arg3.txt: A file with emittances for each time step. The growth rates are the zero-ed columns, so they are not saved. In this file, L{1}=timesteps (so the NIBSruns), L{2},L{3}=the emittances and energyspread=sqrt(L{4}/2).
RES_DISTRIB_arg3.txt:      The distribution file, that has a length=#mp. One can also ask for the output distribution.
```

## Some More Implementation Details

### Program Flow

The main program execution flow goes as follows:

1. Reads the `MAD-X` TWISS and input parameters files; and generates the names of the output files.
2. If `flag_rec` is set to 1, optimizes the number of points in the lattice to be used in order to speed up the simulation. This generates the new TWISS file!
3. Generates the distribution of macro-particles given the mean invariants and random phases.
4. If the `IBSflag` is set to 1, applies the transformation to the momentum frame, then applies the IBS routine in each point of the lattice and transforms back to the invariants frame.
5. If `oneturn` is set to 1, the above calculation of step 4 is done only once and the one-turn emittance evolution around the lattice is written in an output file. Otherwise, the tracking of the distribution evolution is followed for every element and for each turn for a time step of `TEMPO` or until convergence. The mean growth times and mean emittances are calculated and written in an output file every one `timestep` `TIMEINJ`.
6. If `damping` and `q_ex` are both set to 1, the effects of synchrotron radiation damping and quantum excitation are taken into account.

### The IBS Routine

The core of the performed calculations are the application of IBS kicks, which are done according to the steps below:

1. Finds the limits of the distributions.
2. The distributions are then split in cells (uniform split with respect to x, z and s coordinates).
3. Macro-particles are put in the determined cells and the program calculates the number of macro-particles and their density in each cell.
4. Scattering kicks are applied between macro-particles in the same cell, for each cell (binary collisions) -> redistribution of phase space.

### Application of fastrun

When setting `fastrun=1` and a value for `NIBSruns` in the parameter file one can speed up the calculation by not applying the IBS kicks at every turn.
When done so, the following modifications are made:

1. The IBS is applied in one turn and the growth rate per particle is calculated.
2. The emittance per particle is re-calculated based on the exponential IBS growth in a time step `TIMEINJ`, calculated from the `NIBSruns` and `TEMPO` values. 
3. Tracking continues with the interpolated distribution.

<details>
  <summary><b>Note: Use of fastrun</b></summary>

  The `fastrun` option is meant to speed up the simulation by simplifying the number of performed IBS kicks. It is left to the user to check that results are valid, which might not be the case based on your lattice, beam composition, included effects etc.

</details>
