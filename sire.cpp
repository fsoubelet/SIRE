// *********************************************************
// PROGRAMME SIRE.C
// In this version of the code only one turn tracking.
// Code of A. Vivoli
//
// Revival of the code by F.A.
// Start working on the code --> May 2014
// Changes:
//	1. Use input file to define parameters (June 2014)
//		Add the input file as an input argument when executing
//	2. One code for 1-turn or multi-turn with flag in the parameters file
// *********************************************************

#include <fstream>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>

#define BUFFSIZE1 5001
#define BUFFSIZE2 50000
#define INDEXIND 30
#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0 / IM1)
#define IMM1 (IM1 - 1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1 + IMM1 / NTAB)
#define EPS 1.2e-7
#define RNMX (1.0 - EPS)

#define sq2 1.4142135623730950
#define pi 3.1415926535897932
#define sqpi 3.544907701811032
#define cvel 299792458.0
#define twopicvel 1883651567.30885
#define re 2.8179409E-15
#define rp 1.534698E-18
#define mp 938.272046
#define me 0.510999
#define chargep 1
#define chargee 1

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::string;
using std::vector;

FILE *finput, *foutput, *foutput1, *finput1, *femittances, *fdist;

double *s, *len, *alphax, *alphaz, *betax, *betaz, *dispx, *disp1x, *dispz, *disp1z, *nux, *nuz, *lrep, *angle, *k1l;
int *nrep;
double *x, *xp, *z, *zp, *deltasp, *deltap;
double *ex, *ez, *es, *phix, *phiz, *phis;
double *ex1, *ez1, *es1, *phix1, *phiz1, *phis1;
double epsx, epsz, delta, deltas, numbunch, momentum, energy, massparticle, chargeparticle, TEMPO, dummy;
double invtune, gammap, T0, betarel, radius, realn, deltat, media = 0.0, deltatot = 0.0;
int n, comodo, cont, flag, flag2, ncollisions, continuation = 0, NIBSruns = 0;
int i, npoints, numpart = 0, nturns, oneturn, ncellx, ncellz, ncells, ncellt, ncelltot, ncell, flagangle = 0, flagk1l = 0;
float d;
char c;
char const *Gr_ratesnam   = "_Growth_Rates_";
char const *emittancesnam = "_EMITTANCE_";
char const *distribnam    = "_DISTRIB_";
char const *twissnam      = "_TWISS_";
char *dummystring, *dummyext, *distrfile;
long idum2 = 123456789, iy = 0, iv[NTAB], idum;
double *exm, *ezm, *esm, *exmt, *ezmt, *esmt, *grx, *grz, *grs, *temp, *grxp, *grzp, *grsp;
int NINJ, KINJ, damping, IBSflag, KINJ1, numpart1, q_ex, flag_rec, convsteadystate = 0, fastrun = 0, renormbinningtrans = 1, renormbinningall = 1,
                                                                   mppercell = 5; //,ninjruns=1;
int nturnstostudy, checktime = 0;
double TIMEINJ, DTIME, circumference, coupling;
double dtimex, dtimez, dtimes, eqx, eqz, eqs, eqdelta, eqdeltas, qex1, qex2, qez1, qez2, qes1, qes2, rsq, fac, ratio;
char path[] = "RES";
char ext[]  = ".txt";
char *grname, *emitname, *distname, *twissname; // Files produced
double diffx = 1, diffz = 1, diffs = 1, convlim = 1.e-6;
int particle = 0;
double deltacell, deltacellx, deltacellz, deltacells;

double ran2(void);
char *strlwr(char *str);
int read_madx(char *filemadx);
int read_input(char *fileinput);
int read_distrib(char *filedist);
int read_grates(char *filerate);
int check(int pos1, int pos2, double prec);
int recurrences(void);
int recurrences2(void);
int invtomom(int i);
int momtoinv(int i);
int IBS(void);
int scatter(int part1, int part2, double dimp, double dens);
int convkk = 0;
int flag_def, flag_renorm;

// BEGIN MAIN SUBROUTINE ///////////////////////////////////////////////////
int main(int narg, char *args[]) {
    clock_t start, end;
    start = clock();
    cout << "start = " << start << endl;

    ////////////////////////////////////////////
    // WE ENTER IN THE SETUP PART OF THE CODE //
    ////////////////////////////////////////////

    // BEGIN READING TWISS PARAMETERS FROM MADX FILE AND INPUT PARAMETERS FROM INPUT FILE
    if (narg < 3) {
        printf("Not enough arguments.\n");
        exit(1);
    }
    else {
        flag2 = read_input(args[2]); // returns 0 if succesful 1 if not
        if (flag2) {
            printf("Error reading INPUT file\n");
            exit(1);
        }

        flag = read_madx(args[1]); // returns 0 if succesful 1 if not
        cout << args[1] << endl;
        // Produce the names of the output files
        dummyext = (char *)malloc(strlen(args[3]) + 10);
        cout << "dummyext=" << dummyext << endl;
        strcpy(dummyext, args[3]);
        strcat(dummyext, ext);
        grname = (char *)malloc(strlen(path) + strlen(dummyext) + strlen(Gr_ratesnam) + 10);
        strcpy(grname, path);
        strcat(grname, Gr_ratesnam);
        strcat(grname, dummyext);
        twissname = (char *)malloc(strlen(path) + strlen(dummyext) + strlen(twissnam) + 10);

        strcpy(twissname, path);
        strcat(twissname, twissnam);
        strcat(twissname, dummyext);
        emitname = (char *)malloc(strlen(path) + strlen(dummyext) + strlen(emittancesnam) + 10);
        strcpy(emitname, path);
        strcat(emitname, emittancesnam);
        strcat(emitname, dummyext);
        distname = (char *)malloc(strlen(path) + strlen(dummyext) + strlen(distribnam) + 10);
        strcpy(distname, path);
        strcat(distname, distribnam);
        strcat(distname, dummyext);
    }

    if (flag) {
        printf("Error reading MADX file\n");
        exit(1);
    }
    if (narg > 4) // If narg>3 continue the simulations
    {
        flag         = read_distrib(args[4]); // Read the distribution file
        continuation = 1;                     // Flag to know if continuing or starting a simulation
    }
    if (flag) {
        printf("Error reading distribution file '%s', aborting...\n", distrfile);
        exit(1);
    }
    // Potentially shorten / simplify the lattice and create new twiss file
    else if (flag_rec) {
        flag = recurrences(); // First shortening of the lattice

        if (flag) {
            printf("Error recurrences1\n");
            exit(2);
        }
        else {
            flag = recurrences2(); //  Second shortening of the lattice

            if (flag) {
                printf("Error recurrences2\n");
                exit(3);
            }
            else {
                // Creates the new twiss file after the recurrences
                foutput = fopen(twissname, "w");
                fprintf(foutput, "s lrep len alphax betax nux alphaz betaz nuz dispx disp1x dispz disp1z nrep\n");
                for (i = 0; i < npoints; i++) {
                    fprintf(foutput, "%f %f %f %f %f %f %f %f %f %f %f %f %f %d\n", s[i], lrep[i], len[i], alphax[i], betax[i], nux[i], alphaz[i],
                            betaz[i], nuz[i], dispx[i], disp1x[i], dispz[i], disp1z[i], nrep[i]);
                }
                printf("Tutto ok 3\n");
                fclose(foutput);
            }
        }
    }

    // Compute the length of the new lattice
    dummy = 0;
    for (cont = 0; cont < npoints; cont++) { // Loop in all lattice elements
        dummy += lrep[cont]; // dummy containts the new lattice length
    }
    printf("Circumference= %.9e m, S= %.9e m\n", dummy, s[npoints - 1]);

    // Check if the length of the lattice after recurrences is the same and all the lattice points are correct
    if ((dummy - s[npoints - 1]) / s[npoints - 1] > 5e-2) {
        printf("Error, Circumference= %.9e m, S= %.9e\n", dummy, s[npoints - 1]);
        exit(4);
    }
    // END READING TWISS PARAMETERS FROM MADX AND INPUT PARAMETERS FROM INPUT FILE

    // COMPUTE AND DISPLAY SOME GLOBAL SIMULATION PARAMETERS
    energy = sqrt(momentum * momentum + massparticle * massparticle);
    gammap = energy / massparticle;
    cout << "gamma = " << gammap << endl;
    betarel          = sqrt(1 - 1 / gammap / gammap);
    T0            = s[npoints - 1] / betarel / cvel;  // Time for the particles to do a turn
    circumference = s[npoints - 1];
    cout << "circumference = " << circumference << endl;

    // If we're only doing one turn pass
    if (oneturn) {
        NINJ  = 1.;
        TEMPO = T0;  // TEMPO is the full time length of the simulation (here T0, time for 1 turn)
        DTIME = T0;  // DTIME is the time interval of print information
        nturns = 1;
    }
    // If we're doing multiple turns
    else {
        if (checktime)
            TEMPO = nturnstostudy * T0;  // TEMPO is the full time length of the simulation (nturnstostudy * turn_time)
        DTIME = TEMPO / NIBSruns;        // time interval of print information (should be much smaller than the IBS growth time)
        NINJ   = NIBSruns;               // Number of loops to run AND WRITE IN OUTPUT FILE
        nturns = (int)ceil(DTIME / T0);  // Number of turns per timestep
        TIMEINJ = nturns * T0;  // Timestep
    }

    // DISPLAY SOME PARAMETERS TO COMMANDLINE
    cout << "fastrun=" << fastrun << endl;
    cout << "checktime=" << checktime << endl;
    cout << "convsteadystate=" << convsteadystate << endl;
    cout << "Total run time (TEMPO) = " << TEMPO << endl;
    cout << "Time interval of print = " << DTIME << endl;
    cout << "NINJ = " << NINJ << endl;
    cout << "Number of turns per scan inetrval (nturns) = " << nturns << endl;
    cout << "TIMEINJ=" << TIMEINJ << endl;
    cout << "NIBSruns=" << NIBSruns << endl;
    cout << "Number of macroparticles=" << numpart << endl;
    cout << "T0 = " << T0 << endl;

    // If we continue from a previous job
    if (continuation) {
        numpart = numpart1;
    }
    else {
        KINJ1 = 0;  // Used later for current time step??
    }

    realn = (double)numbunch / numpart; // Number of real particles in one macroparticle

    // Compute longitudinal invariant 'invtune'
    if (eqdeltas == 0) {  // if no equilibrium bunch length was given
        cout << "Invtune = delta / deltas" << endl;  // for debugging
        cout << "With delta = " << delta << " and deltas = " << deltas << endl;  // for debugging
        invtune = delta / deltas; // starting energy spread / starting bunch length
    }
    else {
        cout << "Invtune = eqdelta / eqdeltas" << endl;  // for debugging
        cout << "With eqdelta = " << eqdelta << " and eqdeltas = " << eqdeltas << endl;  // for debugging
        invtune = eqdelta / eqdeltas; // equilibrium energy spread / equilibrium bunch length
    }
    cout << "INVTUNE = " << invtune << endl;

    idum = -time(0); // no idea?

    // BEGIN GENERATING DISTRIBUTION IF NOT A CONTINUATION FROM PREVIOUS SIMULATION
    if (!(continuation)) {
        ex1   = (double *)malloc(numpart * sizeof(double));
        ez1   = (double *)malloc(numpart * sizeof(double));
        es1   = (double *)malloc(numpart * sizeof(double));
        phix1 = (double *)malloc(numpart * sizeof(double));
        phiz1 = (double *)malloc(numpart * sizeof(double));
        phis1 = (double *)malloc(numpart * sizeof(double));

        // Generates distribution of macroparticles.
        // ran2(): Random number generation from 0-1 (uniform).
        // Generation of half distribution to get always positive numbers.
        for (cont = 0; cont < numpart; cont++) {
            ex1[cont] = -2 * epsx * log(1.0 - ran2()); // -2*exmean*log(1-ran2()) --> the factor of 2 is to go from emittance to action,
                                                       // log(1-ran2()) generates an exponential distribution
            ez1[cont]   = -2 * epsz * log(1.0 - ran2());
            es1[cont]   = -(delta * delta + invtune * invtune * deltas * deltas) * log(1 - ran2()); // this is alsready in action
            phix1[cont] = 2 * pi * ran2();
            phiz1[cont] = 2 * pi * ran2();
            phis1[cont] = 2 * pi * ran2();
        }
    }
    // END GENERATING DISTRIBUTION IF NOT A CONTINUATION

    printf("nturns=%d\n", nturns);
    printf("npoints=%d\n", npoints);
    printf("numpart=%d\n", numpart);

    // Allocate memory space for arrays that will be used in the main loop
    temp = (double *)calloc(NINJ + 1, sizeof(double));
    // Each here is an array with one entry per IBS interaction computed
    // These will store mean values of the distribution (emittances) at this step
    exm  = (double *)calloc(NINJ + 1, sizeof(double));
    ezm  = (double *)calloc(NINJ + 1, sizeof(double));
    esm  = (double *)calloc(NINJ + 1, sizeof(double));
    // Each here is an array with one entry per element in the lattice
    // These will store mean values of the distribution (emittances) at this element
    exmt = (double *)calloc(npoints + 1, sizeof(double));
    ezmt = (double *)calloc(npoints + 1, sizeof(double));
    esmt = (double *)calloc(npoints + 1, sizeof(double));
    // Each here is an array with one entry per IBS interaction computed
    // These will store growth rates at this step
    grx  = (double *)calloc(NINJ + 1, sizeof(double));
    grz  = (double *)calloc(NINJ + 1, sizeof(double));
    grs  = (double *)calloc(NINJ + 1, sizeof(double));

    // If CONTINUATION THEN READ GROWTH RATES FROM FILE
    if (continuation) {
        read_grates(grname);
    }
    // OTHERWISE JUST CREATE THE FILE FOR LATER
    else {
        foutput1 = fopen(grname, "w");
        fclose(foutput1);
    }

    // What the fuck?
    npoints--;
    if (s[npoints - 1] == s[npoints - 2]) {
        npoints--;
    }

    // LET THE USER KNOW READING INPUTS, COMPUTING PARAMETERS AND GENERATING (GETTING) DISTRIBUTION WENT WELL
    printf("Tutto ok 1\n");
    fflush(stdout);

    // WRITE DISTRIBUTION TO FILE IF IT WAS GENERATED
    if (!(continuation)) {
        fdist = fopen(distname, "w");
        if (fdist != NULL) {
            for (cont = 0; cont < numpart; cont++) {
                fprintf(fdist, "%.9e, %.9e, %.9e, %.9e, %.9e, %.9e\n", ex1[cont], ez1[cont], es1[cont], phix1[cont], phiz1[cont], phis1[cont]);
            }
            fclose(fdist);
        }
        else {
            printf("Can not write file %s. Aborting... \n", distname);
            exit(1);
        }
    }

    // Allocate memory space for arrays that will be used to store the distribution coordinates
    // Each of these is an array with one entry per particle, used for the invariants
    ex   = (double *)malloc(numpart * sizeof(double));
    ez   = (double *)malloc(numpart * sizeof(double));
    es   = (double *)malloc(numpart * sizeof(double));
    phix = (double *)malloc(numpart * sizeof(double));
    phiz = (double *)malloc(numpart * sizeof(double));
    phis = (double *)malloc(numpart * sizeof(double));
    // And populate these with the existing values from the distribution
    // we have just generated
    for (cont = 0; cont < numpart; cont++) {
        ex[cont]   = ex1[cont];
        ez[cont]   = ez1[cont];
        es[cont]   = es1[cont];
        phix[cont] = phix1[cont];
        phiz[cont] = phiz1[cont];
        phis[cont] = phis1[cont];
    }

    // If we interpolate every time step TIMEINJ
    if (fastrun) {
        // Allocate some more space, don't know who these are
        // They are literally never used again
        grxp = (double *)malloc(numpart * sizeof(double));
        grzp = (double *)malloc(numpart * sizeof(double));
        grsp = (double *)malloc(numpart * sizeof(double));
    }

    // LET THE USER KNOW FULL SIMULATION SETUP WENT WELL
    printf("Tutto ok pre-fin\n");
    fflush(stdout);

    /////////////////////////////////////////////////
    // WE ENTER IN THE SIMULATION PART OF THE CODE //
    /////////////////////////////////////////////////

    // WE LOOP OVER EVERY TIME STEP FOR WHICH THE IBS STUFF IS CALCULATED
    for (KINJ = KINJ1; KINJ < NINJ + 1; KINJ++) {
        cout << "KINJ=" << KINJ << endl;
        // Calculate the mean invariants of the distribution
        for (comodo = 0; comodo < numpart; comodo++) {  // comodo loops over particles (indices) in the distribution
            exm[KINJ] += ex[comodo];
            ezm[KINJ] += ez[comodo];
            esm[KINJ] += es[comodo];
            ex1[comodo] = ex[comodo];
            ez1[comodo] = ez[comodo];
            es1[comodo] = es[comodo];
        }

        // We apply a factor of 2 here is to compensate the fact that the distribution is generated in action space
        exm[KINJ] /= (2. * numpart);
        ezm[KINJ] /= (2. * numpart);
        esm[KINJ] /= (numpart);

        // Print out the mean invariants
        cout << "exm = " << exm[KINJ] << "  ezm = " << ezm[KINJ] << "  esm = " << esm[KINJ] << endl;

        if (KINJ == KINJ1) { // If we are at the first IBS effects application, compute a ratio?
            // TODO: need to figure out what this ratio is
            ratio = sqrt((betax[0] * exm[KINJ] + dispx[0] * dispx[0] * delta * delta) / (betaz[0] * ezm[KINJ] + dispz[0] * dispz[0] * delta * delta));
            cout << "ratio = " << ratio << endl;
        }

        // If not the first IBS computation make sure we're not at convergence to steady state
        // because if that was asked, we need to exit the program
        if (KINJ - KINJ1) {
            // If we track until convergence to steady-state, not up to full TEMPO time
            if (convsteadystate) {  
                diffx = (exm[KINJ] - exm[KINJ - 1]) / exm[KINJ - 1];
                diffz = (ezm[KINJ] - ezm[KINJ - 1]) / ezm[KINJ - 1];
                diffs = (esm[KINJ] - esm[KINJ - 1]) / esm[KINJ - 1];
                cout << "diffx = " << diffx << endl;
                cout << "diffz = " << diffz << endl;
                cout << "diffs = " << diffs << endl;

                if (diffx < convlim && diffz < convlim && diffs < convlim)
                    convkk = convkk + 1;

                if (convkk > 10) {
                    printf("Convergence to steady state has been reached.\n");
                    end = clock();
                    cout << "Time required for execution: " << (double)(end - start) / CLOCKS_PER_SEC << " seconds."
                         << "\n\n";
                    exit(1);
                }
            }
        }
        printf("Tutto ok 13\n");

        // IF NOT THE LAST IBS INTERACTION TIME, TRACK THE DISTRIBUTION
        // REMEMBER BETWEEN ELEMENTS WE ARE IN INVARIANTS (ACTION) SPACE: ACTION DOES NOT CHANGE
        // AND PHASES ARE REGENERATED RANDOMLY. TO APPLY EFFECTS WE CONVERT TO MOMENTUM
        // SPACE, APPLY AND THEN CONVERT BACK TO INVARIANTS (ACTION) SPACE
        if (KINJ - NINJ) { // NINJ is the number of timesteps you devide the TEMPO (full simulation time) to calculate IBS
            cout << "KINJ = " << KINJ << endl;

            // Loop through all turns in one timestep to calculate the emittance evolution
            // and growth rates. The results are written in file in every timestep
            for (n = 0; n < nturns; n++) {
                printf("Turn number=%d\n", n);
                // LOOP THROUGH EVERY POINT IN THE LATTICE
                for (i = 0; i < npoints; i++) {
                    // IF ASKED, APPLY IBS EFFECTS AT EACH POINT
                    if (IBSflag) {
                        // I don't know what the if branch below does
                        if (renormbinningall == 1 || renormbinningtrans == 1) {
                            if (KINJ == KINJ1 && i == 0) {
                                cout << "ratio = " << ratio << endl;
                                if (renormbinningall == 1)
                                    ncellx = ceil(sqrt((numpart * ratio / (ncells * mppercell))));

                                ncellz      = ceil(ncellx / ratio);
                                flag_def    = 1;
                                flag_renorm = 0;

                                // cout << "ncellx = " << ncellx << "  ncellz = " << ncellz << endl;
                            }

                            else if (KINJ > KINJ1 || i > 0) {
                                flag_def    = 0;
                                flag_renorm = 1;
                            }
                        }

                        flag = invtomom(i); // CONVERT DISTRIBUTION TO MOMENTUM SPACE
                        
                        // Compute a delta_t, used in the scattering routine
                        deltat = lrep[i] / s[npoints - 1] * T0;  
                        cout << "deltat = " << deltat << endl;
                        if (fastrun)
                            deltat = deltat * nturns;
                        cout << "deltat = " << deltat << endl;
                        
                        flag = IBS(); // APPLY IBS EFFECTS (PARTICLE KICKS)
                        flag = momtoinv(i); // CONVERT DISTRIBUTION BACK TO ACTION SPACE

                        // If we only do 1 turn, write emittances at the current lattice point
                        // (this will have written at all points when the loop is done)
                        if (oneturn) {
                            cout << "i = " << i << "/" << npoints << endl;

                            exmt[i] = 0;
                            ezmt[i] = 0;
                            esmt[i] = 0;

                            // Loop over particles below to get mean value 
                            for (comodo = 0; comodo < numpart; comodo++) {
                                exmt[i] += ex[comodo];
                                ezmt[i] += ez[comodo];
                                esmt[i] += es[comodo];
                            }

                            // factor 2 again to compensate for distribution being in action space
                            exmt[i] /= (2 * numpart);
                            ezmt[i] /= (2 * numpart);
                            esmt[i] /= (numpart);

                            // Write this to file
                            femittances = fopen(emitname, "a");
                            fprintf(femittances, "%.9e\t %.9e\t %.9e\t %.9e\t \n", s[i], exmt[i], ezmt[i], esmt[i]);
                            fflush(femittances);
                            fclose(femittances);
                        }
                    } // END OF IBS EFFECTS APPLICATION
                } // End of for loop at each point of the lattice

                //  NEEDS TO BE CHECKED! THE RESULTS ARE NOT VALID USING THIS.
                if (damping) { // To take damping into account
                // Loop over particles and change invariants according to damping
                    for (comodo = 0; comodo < numpart; comodo++) {
                        ex[comodo] = 2 * eqx + (ex[comodo] - 2 * eqx) * exp(-DTIME / dtimex);
                        ez[comodo] = 2 * eqz + (ez[comodo] - 2 * eqz) * exp(-DTIME / dtimez);
                        es[comodo] = 2 * eqs + (es[comodo] - 2 * eqs) * exp(-DTIME / dtimes);
                    } // End of for loop in each macroparticle
                } // End of if(damping)

                if (coupling != 0) { // Take coupling into account (Simplified way, only for weak coupling)
                    // Loop over particles and change transverse invariants according to coupling
                    for (comodo = 0; comodo < numpart; comodo++) {
                        ex[comodo] = ex[comodo] * (1 - coupling) + coupling * ez[comodo];
                        ez[comodo] = coupling * (ex[comodo] - coupling * ez[comodo]) / (1 - coupling) + (1 - coupling) * ez[comodo];
                    }
                }
                
                if (fastrun)
                    n = nturns;
            } // End of loop in each turn

            // Write results to file for this specific timestep (IBS effects application)
            cout << "Write to file" << endl;
            foutput1 = fopen(grname, "a");
            fprintf(foutput1, "%d\t %.9e\t %.9e\t %.9e\t %.9e\t %.9e\t %.9e\n", KINJ, exm[KINJ], ezm[KINJ], esm[KINJ], grx[KINJ], grz[KINJ], grs[KINJ]);
            fflush(foutput1);
            fclose(foutput1);
            printf("Tutto ok44! %d,%d,%d\n", n, i, flag);
        } // End of branch doing the IBS effects, distribution tracking etc for this time step
        printf("Tutto ok5! %d,%d,%d\n", n, i, flag);
    } // End of loop over all timesteps where the IBS is calculated

    // Let the user knob all is done and well, output info and exit
    printf("Tutto ok6! %d,%d,%d\n", n, i, flag);
    end = clock();
    cout << "Time required for execution: " << (double)(end - start) / CLOCKS_PER_SEC << " seconds." << "\n\n";
    return 0;
}
// END MAIN SUBROUTINE ////////////////////////////////////////////////////

// BEGIN RAN2 SUBROUTINE ////////////////////////////////////////////////////
//* Random number generator
double ran2(void) {
    int j;
    long k;
    float temp;

    if (idum <= 0) {
        if (-(idum) < 1) {
            idum = 1;
        }
        else {
            idum = -idum;
        }
        idum2 = idum;
        for (j = NTAB + 7; j >= 0; j--) {
            k    = idum / IQ1;
            idum = IA1 * (idum - k * IQ1) - k * IR1;
            if (idum < 0) {
                idum += IM1;
            }
            if (j < NTAB) {
                iv[j] = idum;
            }
        }
        iy = iv[0];
    }

    k    = idum / IQ1;
    idum = IA1 * (idum - k * IQ1) - k * IR1;
    if (idum < 0) {
        idum += IM1;
    }

    k     = idum2 / IQ2;
    idum2 = IA2 * (idum2 - k * IQ2) - k * IR2;
    if (idum2 < 0) {
        idum2 += IM2;
    }

    j     = iy / NDIV;
    iy    = iv[j] - idum2;
    iv[j] = idum;
    if (iy < 1) {
        iy += IMM1;
    }
    if ((temp = AM * iy) > RNMX) {
        return RNMX;
    }
    else {
        return temp;
    }
}
// END RAN2 SUBROUTINE ////////////////////////////////////////////////////

// BEGIN STRLWR SUBROUTINE //////////////////////////////////////////////////
// Converts a string to lowercase
char *strlwr(char *str) {
    int k;
    for (k = 0; str[k]; k++) {
        str[k] = tolower(str[k]);
    }
    return str;
}
// END STRLWR SUBROUTINE //////////////////////////////////////////////////

// BEGIN READ_MADX SUBROUTINE /////////////////////////////////////////////////////
//  Reads madx twiss file with a certain format. Columns has to follow the ordering:
//  name,s1,len1,betx1,alphax1,mux1,bety1,alphay1,muy1,dx1,dpx1,dy1,dpy1
int read_madx(char *filemadx) {
    cout << "READING MADX" << endl;
    char name[200], *buffer, ch = ' ', *keyword, prebuffer[BUFFSIZE1];
    char s1[200], len1[200], betx1[200], alphax1[200], mux1[200], bety1[200], alphay1[200], muy1[200], x[200], px[200], y[200], py[200], dx1[200],
        dpx1[200], dy1[200], dpy1[200], wx[200], phix[200], dmux[200], wy[200], phiy[200], dmuy[200], ddx[200], dppx[200], ddy[200], ddpy[200],
        ener[200];
    double flag1, s2[BUFFSIZE2], len2[BUFFSIZE2], betx2[BUFFSIZE2], alphax2[BUFFSIZE2], mux2[BUFFSIZE2], bety2[BUFFSIZE2], alphay2[BUFFSIZE2],
        muy2[BUFFSIZE2], dx2[BUFFSIZE2], dpx2[BUFFSIZE2], dy2[BUFFSIZE2], dpy2[BUFFSIZE2], k1l2[BUFFSIZE2], angle2[BUFFSIZE2];
    int cont, i, flag = 0, flagint = 0, flagstart = 0, linecount, linecount1 = 0, linecount2 = 0, linecount3 = 0, flagid, index[INDEXIND];
    int flagl = 0, flagmux = 0, flagmuy = 0, flagiden, flagok = 0, provatore;

    for (cont = 0; cont < INDEXIND; cont++) {
        index[cont] = 0;
    }
    cont = 0;

    if ((finput = fopen(filemadx, "r")) == NULL) {
        printf("Error opening file %s\n", filemadx);
        return 1;
    }

    while (!feof(finput)) { // feof is 1 when end-of-file, 0 when not end-of-file, WHILE IS TRUE WHEN DIFFERENT FROM 0
        fgets(prebuffer, BUFFSIZE1, finput);
        if (feof(finput)) {
            break;
        }

        buffer = strlwr(prebuffer);
        if (!(strstr(buffer, "s") == NULL)) {
            flagstart = 1;
            if ((flagstart) && (strstr(buffer, "betx") == NULL)) {
                flagstart = 0;
            }
            if ((flagstart) && (strstr(buffer, "bety") == NULL)) {
                flagstart = 0;
            }
            if ((flagstart) && (strstr(buffer, "alfx") == NULL)) {
                flagstart = 0;
            }
            if ((flagstart) && (strstr(buffer, "alfy") == NULL)) {
                flagstart = 0;
            }
            if ((flagstart) && (strstr(buffer, "dx") == NULL)) {
                flagstart = 0;
            }
            if ((flagstart) && (strstr(buffer, "dy") == NULL)) {
                flagstart = 0;
            }
            if ((flagstart) && (strstr(buffer, "dpx") == NULL)) {
                flagstart = 0;
            }
            if ((flagstart) && (strstr(buffer, "dpy") == NULL)) {
                flagstart = 0;
            }

            if (flagstart) {
                linecount = 0;
                flagid    = -1;
                while (linecount < strlen(buffer)) {
                    flagid   = flagid + 1;
                    flagiden = 0;
                    while ((linecount < strlen(buffer)) && (isspace(buffer[linecount]))) {

                        linecount  = linecount + 1;
                        linecount1 = linecount;
                        while ((linecount < strlen(buffer)) && (!(isspace(buffer[linecount])))) {
                            linecount = linecount + 1;
                        }

                        linecount2 = linecount;
                        keyword    = (char *)malloc(linecount2 - linecount1 + 1);
                        strncpy(keyword, buffer + linecount1, linecount2 - linecount1);
                        keyword[linecount2 - linecount1] = 0;
                        if (flagid) {
                            if (!(strcmp(keyword, "s"))) {
                                flagiden          = 1;
                                index[flagid - 1] = 1;
                            }
                            if (!(strcmp(keyword, "l"))) {
                                flagl             = 1;
                                flagiden          = 1;
                                index[flagid - 1] = 2;
                            }
                            if (!(strcmp(keyword, "betx"))) {
                                flagiden          = 1;
                                index[flagid - 1] = 3;
                            }
                            if (!(strcmp(keyword, "alfx"))) {
                                flagiden          = 1;
                                index[flagid - 1] = 4;
                            }
                            if (!(strcmp(keyword, "mux"))) {
                                flagmux           = 1;
                                flagiden          = 1;
                                index[flagid - 1] = 5;
                            }
                            if (!(strcmp(keyword, "bety"))) {
                                flagiden          = 1;
                                index[flagid - 1] = 6;
                            }
                            if (!(strcmp(keyword, "alfy"))) {
                                flagiden          = 1;
                                index[flagid - 1] = 7;
                            }
                            if (!(strcmp(keyword, "muy"))) {
                                flagmuy           = 1;
                                flagiden          = 1;
                                index[flagid - 1] = 8;
                            }
                            if (!(strcmp(keyword, "dx"))) {
                                flagiden          = 1;
                                index[flagid - 1] = 9;
                            }
                            if (!(strcmp(keyword, "dpx"))) {
                                flagiden          = 1;
                                index[flagid - 1] = 10;
                            }
                            if (!(strcmp(keyword, "dy"))) {
                                flagiden          = 1;
                                index[flagid - 1] = 11;
                            }
                            if (!(strcmp(keyword, "dpy"))) {
                                flagiden          = 1;
                                index[flagid - 1] = 12;
                            }
                            if (!(strcmp(keyword, "angle"))) {
                                flagangle         = 1;
                                flagiden          = 1;
                                index[flagid - 1] = 13;
                            }
                            if (!(strcmp(keyword, "k1l"))) {
                                flagk1l           = 1;
                                flagiden          = 1;
                                index[flagid - 1] = 14;
                            }
                            if ((!(flagiden)) && (flagid > 0)) {
                                index[flagid - 1] = 0;
                            }
                        }
                        free(keyword);
                        flagok = 1;
                    }
                }
            }

            if (flagok) {
                linecount = 0;
                flagid    = 0;
                while (linecount < strlen(buffer)) {
                    while ((linecount < strlen(buffer)) && (isspace(buffer[linecount]))) {
                        linecount = linecount + 1;
                    }

                    linecount1 = linecount;
                    while ((linecount < strlen(buffer)) && (!(isspace(buffer[linecount])))) {
                        linecount = linecount + 1;
                    }

                    linecount2 = linecount;
                    keyword    = (char *)malloc(linecount2 - linecount1 + 1);
                    strncpy(keyword, buffer + linecount1, linecount2 - linecount1);
                    keyword[linecount2 - linecount1] = 0;
                    switch (index[flagid]) { // if(!(index[flagid]==0)) //mettere switch
                    case 1: {
                        s2[cont] = atof(keyword);
                        break;
                    }
                    case 2: {
                        len2[cont] = atof(keyword);
                        break;
                    }
                    case 3: {
                        betx2[cont] = atof(keyword);
                        // printf("betx2=%.10f\n",betx2[cont]);
                        break;
                    }
                    case 4: {
                        alphax2[cont] = atof(keyword);
                        break;
                    }
                    case 5: {
                        mux2[cont] = atof(keyword);
                        break;
                    }
                    case 6: {
                        bety2[cont] = atof(keyword);
                        break;
                    }
                    case 7: {
                        alphay2[cont] = atof(keyword);
                        break;
                    }
                    case 8: {
                        muy2[cont] = atof(keyword);
                        break;
                    }
                    case 9: {
                        dx2[cont] = atof(keyword);
                        break;
                    }
                    case 10: {
                        dpx2[cont] = atof(keyword);
                        break;
                    }
                    case 11: {
                        dy2[cont] = atof(keyword);
                        break;
                    }
                    case 12: {
                        dpy2[cont] = atof(keyword);
                        break;
                    }
                    case 13: {
                        angle2[cont] = atof(keyword);
                        break;
                    }
                    case 14: {
                        k1l2[cont] = atof(keyword);
                        break;
                    }
                    default:
                        break;
                    }
                    free(keyword);
                    flagid = flagid + 1;
                }
                cont = cont + 1;
            }
        }
        fclose(finput);

        npoints = cont;
        printf("npoints=%d\n", npoints);
        for (cont = 0; cont < npoints; cont++) {
            if (!(flagl)) {
                if (cont == 0) {
                    len2[cont] = 0;
                }
                else {
                    len2[cont] = s2[cont] - s2[cont - 1];
                }
            }
            if (!(flagmux)) {
                if (cont == 0) {
                    mux2[cont] = 0;
                }
                else {
                    if (betx2[cont - 1] > 0) {
                        mux2[cont] = mux2[cont - 1] + len2[cont] / betx2[cont - 1];
                    }
                    else {
                        mux2[cont] = mux2[cont - 1];
                    }
                }
            }
            if (!(flagmuy)) {
                if (cont == 0) {
                    muy2[cont] = 0;
                }
                else {
                    if (bety2[cont - 1] > 0) {
                        muy2[cont] = muy2[cont - 1] + len2[cont] / bety2[cont - 1];
                    }
                    else {
                        muy2[cont] = muy2[cont - 1];
                    }
                }
            }
        }

        s      = (double *)malloc(npoints * sizeof(double));
        len    = (double *)malloc(npoints * sizeof(double));
        alphax = (double *)malloc(npoints * sizeof(double));
        alphaz = (double *)malloc(npoints * sizeof(double));
        betax  = (double *)malloc(npoints * sizeof(double));
        betaz  = (double *)malloc(npoints * sizeof(double));
        dispx  = (double *)malloc(npoints * sizeof(double));
        disp1x = (double *)malloc(npoints * sizeof(double));
        dispz  = (double *)malloc(npoints * sizeof(double));
        disp1z = (double *)malloc(npoints * sizeof(double));
        nux    = (double *)malloc(npoints * sizeof(double));
        nuz    = (double *)malloc(npoints * sizeof(double));
        nrep   = (int *)malloc(npoints * sizeof(int));
        lrep   = (double *)malloc(npoints * sizeof(double));
        if (flagangle) {
            angle = (double *)malloc(npoints * sizeof(double));
        }
        if (flagk1l) {
            k1l = (double *)malloc(npoints * sizeof(double));
        }
        cont = -1;
        for (i = 0; i < npoints; i++) {
            if (((cont < 0) && (betx2[i] > 0)) || (len2[i] > 0) || ((betx2[i] - betx2[i - 1]) * (betx2[i] - betx2[i - 1]) > 0) ||
                ((bety2[i] - bety2[i - 1]) * (bety2[i] - bety2[i - 1]) > 0) || ((alphax2[i] - alphax2[i - 1]) * (alphax2[i] - alphax2[i - 1]) > 0) ||
                ((alphay2[i] - alphay2[i - 1]) * (alphay2[i] - alphay2[i - 1]) > 0) || ((dy2[i] - dy2[i - 1]) * (dy2[i] - dy2[i - 1]) > 0) ||
                ((dx2[i] - dx2[i - 1]) * (dx2[i] - dx2[i - 1]) > 0) || ((dpy2[i] - dpy2[i - 1]) * (dpy2[i] - dpy2[i - 1]) > 0) ||
                ((dpx2[i] - dpx2[i - 1]) * (dpx2[i] - dpx2[i - 1]) > 0)) {
                cont += 1;
                s[cont]      = s2[i];
                len[cont]    = len2[i];
                alphax[cont] = alphax2[i];
                alphaz[cont] = alphay2[i];
                betax[cont]  = betx2[i];
                betaz[cont]  = bety2[i];
                nux[cont]    = mux2[i];
                nuz[cont]    = muy2[i];
                dispx[cont]  = dx2[i];
                disp1x[cont] = dpx2[i];
                dispz[cont]  = dy2[i];
                disp1z[cont] = dpy2[i];
                nrep[cont]   = 1;
                if (flagangle) {
                    angle[cont] = angle2[i];
                }
                if (flagk1l) {
                    k1l[cont] = k1l2[i];
                }
            }
        }

        npoints = cont + 1;
        for (i = 0; i < npoints; i++) {
            if (i < (npoints - 1)) {
                lrep[i] = len[i + 1];
            }
            else {
                lrep[i] = 0;
            }
        }
        cout << "END READING MADX" << endl;
        printf("npoints=%d\n", npoints);
        return 0;
    }
}
// END READ_MADX SUBROUTINE /////////////////////////////////////////////////////

// BEGIN READ_INPUT SUBROUTINE /////////////////////////////////////////////////////
int read_input(char *fileinput) {
    char *arr;
    unsigned int length;

    string mystring;

    vector<string> v;
    vector<string> paramname;
    vector<string> paramvalue;
    vector<double> params;

    ifstream input(fileinput);
    // Obtaining the length of file
    input.seekg(0, ios::end);
    length = input.tellg();
    input.seekg(0, ios::beg);

    arr = new char[length];

    if (!input) {
        cout << "Unable to open parameter file" << endl;
        return 1;
    }
    else
        cout << "Open parameter file successfully!" << endl;
    cout << "" << endl;

    while (!input.eof()) {
        if (input) {
            input >> arr;
            mystring = arr;
            v.push_back(mystring);
        }
    }

    unsigned int vsize = v.size();
    for (unsigned int jj = 0; jj < vsize; jj++) {
        if (jj == 0 || jj % 2 == 0)
            paramname.push_back(v[jj]);
        if (jj == 1 || jj % 2 != 0)
            paramvalue.push_back(v[jj]);
    }
    paramname.resize(vsize / 2);
    paramvalue.resize(vsize / 2);

    if (paramname.size() < 30) {
        cout << paramname.size() << endl;
        cout << "Not enough input arguments" << endl;
        return 1;
    }
    else {
        for (int ii = 0; ii < paramname.size(); ii++)
            params.push_back(atof(paramvalue[ii].c_str()));

        for (int ii = 0; ii < paramname.size(); ii++) {
            if (paramname[ii] == "TEMPO")
                TEMPO = params[ii];
            if (paramname[ii] == "NIBSruns")
                NIBSruns = params[ii];
            if (paramname[ii] == "oneturn")
                oneturn = params[ii];
            if (paramname[ii] == "fastrun")
                fastrun = params[ii];
            if (paramname[ii] == "renormbinningtrans")
                renormbinningtrans = params[ii];
            if (paramname[ii] == "renormbinningall")
                renormbinningall = params[ii];
            if (paramname[ii] == "nturnstostudy")
                nturnstostudy = params[ii];
            if (paramname[ii] == "checktime")
                checktime = params[ii];
            if (paramname[ii] == "epsx")
                epsx = params[ii];
            if (paramname[ii] == "epsz")
                epsz = params[ii];
            if (paramname[ii] == "delta")
                delta = params[ii];
            if (paramname[ii] == "deltas")
                deltas = params[ii];
            if (paramname[ii] == "dtimex")
                dtimex = params[ii];
            if (paramname[ii] == "dtimez")
                dtimez = params[ii];
            if (paramname[ii] == "dtimes")
                dtimes = params[ii];
            if (paramname[ii] == "eqx")
                eqx = params[ii];
            if (paramname[ii] == "eqz")
                eqz = params[ii];
            if (paramname[ii] == "eqdelta")
                eqdelta = params[ii];
            if (paramname[ii] == "eqdeltas")
                eqdeltas = params[ii];
            if (paramname[ii] == "flag_rec")
                flag_rec = params[ii];
            if (paramname[ii] == "damping")
                damping = params[ii];
            if (paramname[ii] == "IBSflag")
                IBSflag = params[ii];
            if (paramname[ii] == "q_ex")
                q_ex = params[ii];
            if (paramname[ii] == "momentum")
                momentum = params[ii];
            if (paramname[ii] == "numbunch")
                numbunch = params[ii];
            if (paramname[ii] == "numpart")
                numpart = params[ii];
            if (paramname[ii] == "ncellx")
                ncellx = params[ii];
            if (paramname[ii] == "ncellz")
                ncellz = params[ii];
            if (paramname[ii] == "ncells")
                ncells = params[ii];
            if (paramname[ii] == "ncollisions")
                ncollisions = params[ii];
            if (paramname[ii] == "convsteadystate")
                convsteadystate = params[ii];
            if (paramname[ii] == "particle")
                particle = params[ii];
        }

        if (particle == 1) {
            massparticle   = mp;
            chargeparticle = chargep;
            radius         = rp;
            cout << "particle = proton" << endl;
        }
        if (particle == 0) {
            massparticle   = me;
            chargeparticle = chargee;
            radius         = re;
            cout << "particle = electron" << endl;
        }
        cout << "massparticle = " << massparticle << endl;
        cout << "chargeparticle = " << chargeparticle << endl;
    }
    return 0;
}
// END READ_INPUT SUBROUTINE /////////////////////////////////////////////////////

// BEGIN  READ_DISTRIB SUBROUTINE /////////////////////////////////////////////////////
int read_distrib(char *filedist) {
    FILE *fdist;
    char name[200], buffer[5000], ch = ',', *str;
    char emitx1[300], emitz1[300], emits1[300], phasex1[300], phasez1[300], phases1[300];
    int cont, i, flag, flagint = 0, npart = 0, lim1 = 0, lim2 = 0, dati = 0;

    fdist = fopen(filedist, "r");

    if (fdist) {
        while (!feof(fdist)) {
            fgets(buffer, sizeof(buffer) / sizeof(char), fdist);

            if (feof(fdist)) {
                break;
            }
            else {
                npart += 1;
            }
        }
        ex1   = (double *)malloc(npart * sizeof(double));
        ez1   = (double *)malloc(npart * sizeof(double));
        es1   = (double *)malloc(npart * sizeof(double));
        phix1 = (double *)malloc(npart * sizeof(double));
        phiz1 = (double *)malloc(npart * sizeof(double));
        phis1 = (double *)malloc(npart * sizeof(double));

        rewind(fdist);
        for (cont = 0; cont < npart; cont++) {
            fgets(buffer, sizeof(buffer) / sizeof(char), fdist);
            dati = 0;
            while (dati < 6) {
                switch (dati) { // if(!(index[flagid]==0)) //mettere switch
                case 0: {
                    str = strchr(buffer, ch);
                    if (str == NULL) {
                        printf("Error reading distribution file %s\n", filedist);
                        return 1;
                    }
                    else {
                        lim1 = 0;
                        lim2 = str - buffer;
                        strncpy(emitx1, buffer + lim1, lim2 - lim1);
                        ex1[cont] = atof(emitx1);
                        lim1      = lim2 + 1;
                        dati += 1;
                    }
                    break;
                }
                case 1: {
                    str = strchr(buffer + lim1, ch);
                    if (str == NULL) {
                        printf("Error reading distribution file %s\n", filedist);
                        return 1;
                    }
                    else {
                        lim2 = str - buffer;
                        strncpy(emitz1, buffer + lim1, lim2 - lim1);
                        ez1[cont] = atof(emitz1);
                        lim1      = lim2 + 1;
                        dati += 1;
                    }
                    break;
                }
                case 2: {
                    str = strchr(buffer + lim1, ch);
                    if (str == NULL) {
                        printf("Error reading distribution file %s\n", filedist);
                        return 1;
                    }
                    else {
                        lim2 = str - buffer;
                        strncpy(emits1, buffer + lim1, lim2 - lim1);
                        es1[cont] = atof(emits1);
                        lim1      = lim2 + 1;
                        dati += 1;
                    }
                    break;
                }
                case 3: {
                    str = strchr(buffer + lim1, ch);
                    if (str == NULL) {
                        printf("Error reading distribution file %s\n", filedist);
                        return 1;
                    }
                    else {
                        lim2 = str - buffer;
                        strncpy(phasex1, buffer + lim1, lim2 - lim1);
                        phix1[cont] = atof(phasex1);
                        lim1        = lim2 + 1;
                        dati += 1;
                    }
                    break;
                }
                case 4: {
                    str = strchr(buffer + lim1, ch);
                    if (str == NULL) {
                        printf("Error reading distribution file %s\n", filedist);
                        return 1;
                    }
                    else {
                        lim2 = str - buffer;
                        strncpy(phasez1, buffer + lim1, lim2 - lim1);
                        phiz1[cont] = atof(phasez1);
                        lim1        = lim2 + 1;
                        dati += 1;
                    }
                    break;
                }
                default: {
                    while (isspace(buffer[lim1])) {
                        lim1++;
                    }
                    lim2 = lim1;
                    while (!(isspace(buffer[lim2]))) {
                        lim2++;
                    }
                    strncpy(phases1, buffer + lim1, lim2 - lim1);
                    phis1[cont] = atof(phases1);
                    dati += 1;
                }
                }
            }
        }
        fclose(fdist);
    }
    else {
        printf("Error opening file %s\n", filedist);
        return 1;
    }
    numpart1 = npart;
    return 0;
}
// END READ_DISTRIB SUBROUTINE /////////////////////////////////////////////////////

// BEGIN  READ_GRATES SUBROUTINE /////////////////////////////////////////////////////
int read_grates(char *filerate) {
    FILE *frates;
    char name[200], buffer[5000], ch = '\t', *str;
    char temp1[300], exm1[300], ezm1[300], esm1[300], grx1[300], grz1[300], grs1[300];
    int cont = 0, i, flag, flagint = 0, nturn, npart, dati, lim1 = 0, lim2 = 0;
    frates = fopen(filerate, "r");

    if (frates) {
        while (!feof(frates)) {
            fgets(buffer, sizeof(buffer) / sizeof(char), frates);
            if (feof(frates)) {
                break;
            }
            else {
                KINJ1 += 1;
            }
        }

        rewind(frates);

        for (cont = 0; cont < KINJ1; cont++) {
            fgets(buffer, sizeof(buffer) / sizeof(char), frates);
            dati = 0;
            while (dati < 6) {
                switch (dati) { // if(!(index[flagid]==0)) //mettere switch
                case 0: {
                    str = strchr(buffer, ch);
                    if (str == NULL) {
                        printf("Error reading growth rates file %s\n", filerate);
                        return 1;
                    }
                    else {
                        lim1 = 0;
                        lim2 = str - buffer;
                        strncpy(temp1, buffer + lim1, lim2 - lim1);
                        temp[cont] = atof(temp1);
                        lim1       = lim2 + 1;
                        dati += 1;
                    }
                    break;
                }
                case 1:
                case 2:
                case 3: {
                    str = strchr(buffer + lim1, ch);
                    if (str == NULL) {
                        printf("Error reading distribution file %s\n", filerate);
                        return 1;
                    }
                    else {
                        lim2 = str - buffer;
                        lim1 = lim2 + 1;
                        dati += 1;
                    }
                    break;
                }
                case 4: {
                    str = strchr(buffer + lim1, ch);
                    if (str == NULL) {
                        printf("Error reading distribution file %s\n", filerate);
                        return 1;
                    }
                    else {
                        lim2 = str - buffer;
                        strncpy(grx1, buffer + lim1, lim2 - lim1);
                        grx[cont] = atof(grx1);
                        lim1      = lim2 + 1;
                        dati += 1;
                    }
                    break;
                }
                case 5: {
                    str = strchr(buffer + lim1, ch);
                    if (str == NULL) {
                        printf("Error reading distribution file %s\n", filerate);
                        return 1;
                    }
                    else {
                        lim2 = str - buffer;
                        strncpy(grz1, buffer + lim1, lim2 - lim1);
                        grz[cont] = atof(grz1);
                        lim1      = lim2 + 1;
                        dati += 1;
                    }
                    break;
                }
                default: {
                    while (isspace(buffer[lim1])) {
                        lim1++;
                    }
                    lim2 = lim1;
                    while (!(isspace(buffer[lim2]))) {
                        lim2++;
                    }
                    strncpy(grs1, buffer + lim1, lim2 - lim1);
                    grs[cont] = atof(grs1);
                    dati += 1;
                }
                }
            }
        }
        fclose(frates);
    }
    else {
        printf("Error opening file %s\n", filerate);
        return 1;
    }
    return 0;
}
// END READ_GRATES SUBROUTINE /////////////////////////////////////////////////////

// BEGIN CHECK SUBROUTINE /////////////////////////////////////////////////////
int check(int pos1, int pos2, double prec) {
    if (len[pos2] != 0) {
        if (fabs(len[pos1] / len[pos2] - 1.0) > prec) {
            printf("Error 1\n");
            return 0;
        }
    }

    if ((fabs(alphax[pos2]) > 0.1) && (fabs(alphax[pos1]) > 0.1)) {
        if (fabs(alphax[pos1] / alphax[pos2] - 1.0) > prec) {
            printf("Error 2, alphax1=%f,alphax2=%f\n", alphax[pos1], alphax[pos2]);
            return 0;
        }
    }
    if ((fabs(alphaz[pos2]) > 0.1) && (fabs(alphaz[pos1]) > 0.1)) {
        if (fabs(alphaz[pos1] / alphaz[pos2] - 1.0) > prec) {
            printf("Error 3\n");
            return 0;
        }
    }
    if (betaz[pos2] != 0) {
        if (fabs(betaz[pos1] / betaz[pos2] - 1.0) > prec) {
            printf("Error 4\n");
            return 0;
        }
    }
    if (betax[pos2] != 0) {
        if (fabs(betax[pos1] / betax[pos2] - 1.0) > prec) {
            printf("Error 5\n");
            return 0;
        }
    }
    if ((fabs(dispx[pos2]) > 0.001) && (fabs(dispx[pos1]) > 0.001)) {
        if (fabs(dispx[pos1] / dispx[pos2] - 1.0) > prec) {
            printf("Error 6\n");
            return 0;
        }
    }

    if ((nuz[pos2] - nuz[pos2 - 1]) != 0) {
        if (fabs((nuz[pos2] - nuz[pos2 - 1]) / (nuz[pos2] - nuz[pos2 - 1]) - 1.0) > prec) {
            printf("Error 10\n");
            return 0;
        }
    }
    if ((nux[pos2] - nux[pos2 - 1]) != 0) {
        if (fabs((nux[pos2] - nux[pos2 - 1]) / (nux[pos2] - nux[pos2 - 1]) - 1.0) > prec) {
            printf("Error 9\n");
            return 0;
        }
    }

    return 1;
}
// END CHECK SUBROUTINE /////////////////////////////////////////////////////

// BEGIN RECURRENCES SUBROUTINE /////////////////////////////////////////////////////
int recurrences(void) {
    double *s3, *len3, *alphax3, *alphaz3, *betax3, *betaz3, *dispx3, *disp1x3, *dispz3, *disp1z3, *nux3, *nuz3, *lrep3;
    int *nrep3;
    int start = 1, start1, current = 2, current1, start2, flag = 1, flag1, flag2, cont, cont1, current3 = 1, current4;
    double precision = 0.3;

    s3      = (double *)malloc(npoints * sizeof(double));
    len3    = (double *)malloc(npoints * sizeof(double));
    alphax3 = (double *)malloc(npoints * sizeof(double));
    alphaz3 = (double *)malloc(npoints * sizeof(double));
    betax3  = (double *)malloc(npoints * sizeof(double));
    betaz3  = (double *)malloc(npoints * sizeof(double));
    dispx3  = (double *)malloc(npoints * sizeof(double));
    disp1x3 = (double *)malloc(npoints * sizeof(double));
    dispz3  = (double *)malloc(npoints * sizeof(double));
    disp1z3 = (double *)malloc(npoints * sizeof(double));
    nux3    = (double *)malloc(npoints * sizeof(double));
    nuz3    = (double *)malloc(npoints * sizeof(double));
    nrep3   = (int *)malloc(npoints * sizeof(int));
    lrep3   = (double *)malloc(npoints * sizeof(double));

    s3[0]      = s[0];
    len3[0]    = len[0];
    alphax3[0] = alphax[0];
    alphaz3[0] = alphaz[0];
    betaz3[0]  = betaz[0];
    betax3[0]  = betax[0];
    dispx3[0]  = dispx[0];
    disp1x3[0] = disp1x[0];
    dispz3[0]  = dispz[0];
    disp1z3[0] = disp1z[0];
    nux3[0]    = nux[0];
    nuz3[0]    = nuz[0];
    nrep3[0]   = 1;
    lrep3[0]   = lrep[0];

    while (flag) {
        start1 = start;
        while (flag) {
            if ((fabs(alphax[start1]) > 0.1) && (fabs(alphax[current]) > 0.1)) {
                if (fabs(alphax[current] / alphax[start1] - 1.0) > precision) {
                    flag = 0;
                }
            }
            if ((fabs(alphaz[start1]) > 0.1) && (fabs(alphaz[current]) > 0.1)) {
                if (fabs(alphaz[current] / alphaz[start1] - 1.0) > precision) {
                    flag = 0;
                }
            }
            if (betaz[start1] != 0) {
                if (fabs(betaz[current] / betaz[start1] - 1.0) > precision) {
                    flag = 0;
                }
            }
            if (betax[start1] != 0) {
                if (fabs(betax[current] / betax[start1] - 1.0) > precision) {
                    flag = 0;
                }
            }
            if ((fabs(dispx[start1]) > 0.001) && (fabs(dispx[current]) > 0.001)) {
                if (fabs(dispx[current] / dispx[start1] - 1.0) > precision) {
                    flag = 0;
                }
            }

            if ((nuz[start1] - nuz[start1 - 1]) != 0) {
                if (fabs((nuz[current] - nuz[current - 1]) / (nuz[start1] - nuz[start1 - 1]) - 1.0) > precision) {
                    flag = 0;
                }
            }
            if ((nux[start1] - nux[start1 - 1]) != 0) {
                if (fabs((nux[current] - nux[current - 1]) / (nux[start1] - nux[start1 - 1]) - 1.0) > precision) {
                    flag = 0;
                }
            }

            if (flag) {
                flag1    = 1;
                current1 = current;
                start2   = start1;
                flag2    = 1;

                while (flag1 * flag2) {
                    start2++;
                    current1++;
                    flag1 = check(start2, current1, precision);
                    if (start2 >= (current - 1)) {
                        flag2 = 0;
                    }
                }
                if (flag1) {
                    for (cont = 0; cont < (start1 - start); cont++) {
                        s3[current3 + cont]      = s[start + cont];
                        len3[current3 + cont]    = len[start + cont];
                        alphax3[current3 + cont] = alphax[start + cont];
                        alphaz3[current3 + cont] = alphaz[start + cont];
                        betaz3[current3 + cont]  = betaz[start + cont];
                        betax3[current3 + cont]  = betax[start + cont];
                        dispx3[current3 + cont]  = dispx[start + cont];
                        disp1x3[current3 + cont] = disp1x[start + cont];
                        dispz3[current3 + cont]  = dispz[start + cont];
                        disp1z3[current3 + cont] = disp1z[start + cont];
                        nux3[current3 + cont]    = nux[start + cont];
                        nuz3[current3 + cont]    = nuz[start + cont];
                        nrep3[current3 + cont]   = 1;
                        lrep3[current3 + cont]   = lrep[start + cont];
                    }
                    current3 = current3 + start1 - start;

                    printf("Bene1\n");

                    for (cont = 0; cont < (current - start1); cont++) {
                        s3[current3 + cont]      = s[start1 + cont];
                        len3[current3 + cont]    = len[start1 + cont];
                        alphax3[current3 + cont] = alphax[start1 + cont];
                        alphaz3[current3 + cont] = alphaz[start1 + cont];
                        betaz3[current3 + cont]  = betaz[start1 + cont];
                        betax3[current3 + cont]  = betax[start1 + cont];
                        dispx3[current3 + cont]  = dispx[start1 + cont];
                        disp1x3[current3 + cont] = disp1x[start1 + cont];
                        dispz3[current3 + cont]  = dispz[start1 + cont];
                        disp1z3[current3 + cont] = disp1z[start1 + cont];
                        nux3[current3 + cont]    = nux[start1 + cont];
                        nuz3[current3 + cont]    = nuz[start1 + cont];
                        nrep3[current3 + cont]   = 2;
                        lrep3[current3 + cont]   = lrep[start1 + cont] + lrep[current + cont];
                    }
                    flag2 = 1;
                    cont1 = 2;

                    printf("Bene2\n");

                    while (flag1) {
                        flag2 = 1;
                        cont  = 0;
                        current1++;
                        while (flag1 * flag2) {
                            flag1 = check(current1 + cont, start1 + cont, precision);
                            cont++;
                            if (cont >= (current - start1)) {
                                flag2 = 0;
                            }
                        }

                        if (flag1) {
                            current4 = current1;
                            current1 = current1 + cont - 1;
                            cont1++;
                            for (cont = 0; cont < (current - start1); cont++) {
                                nrep3[current3 + cont] = nrep3[current3 + cont] + 1;
                                lrep3[current3 + cont] = lrep3[current3 + cont] + lrep[current4 + cont];
                            }
                            printf("Bene3\n");
                        }
                        else {
                            current3 = current3 + current - start1;
                            start    = start1 + cont1 * (current - start1);
                            current  = start + 1;
                            start1   = start;
                        }
                    }
                }
            }

            if (start1 >= (current - 1)) {
                flag = 0;
            }
            else {
                flag = 1;
                start1++;
            }
        }

        if (current >= (npoints - 3)) {
            for (cont = 0; cont < (npoints - start); cont++) {
                s3[current3 + cont]      = s[start + cont];
                len3[current3 + cont]    = len[start + cont];
                alphax3[current3 + cont] = alphax[start + cont];
                alphaz3[current3 + cont] = alphaz[start + cont];
                betaz3[current3 + cont]  = betaz[start + cont];
                betax3[current3 + cont]  = betax[start + cont];
                dispx3[current3 + cont]  = dispx[start + cont];
                disp1x3[current3 + cont] = disp1x[start + cont];
                dispz3[current3 + cont]  = dispz[start + cont];
                disp1z3[current3 + cont] = disp1z[start + cont];
                nux3[current3 + cont]    = nux[start + cont];
                nuz3[current3 + cont]    = nuz[start + cont];
                nrep3[current3 + cont]   = 1;
                lrep3[current3 + cont]   = lrep[start + cont];
            }
            npoints = current3 + npoints - start;
            flag    = 0;
        }
        else {
            flag = 1;
            current++;
        }
    }

    free(s);
    free(len);
    free(alphax);
    free(alphaz);
    free(betax);
    free(betaz);
    free(dispx);
    free(disp1x);
    free(dispz);
    free(disp1z);
    free(nux);
    free(nuz);
    free(nrep);
    free(lrep);

    s      = (double *)malloc(npoints * sizeof(double));
    len    = (double *)malloc(npoints * sizeof(double));
    alphax = (double *)malloc(npoints * sizeof(double));
    alphaz = (double *)malloc(npoints * sizeof(double));
    betax  = (double *)malloc(npoints * sizeof(double));
    betaz  = (double *)malloc(npoints * sizeof(double));
    dispx  = (double *)malloc(npoints * sizeof(double));
    disp1x = (double *)malloc(npoints * sizeof(double));
    dispz  = (double *)malloc(npoints * sizeof(double));
    disp1z = (double *)malloc(npoints * sizeof(double));
    nux    = (double *)malloc(npoints * sizeof(double));
    nuz    = (double *)malloc(npoints * sizeof(double));
    nrep   = (int *)malloc(npoints * sizeof(int));
    lrep   = (double *)malloc(npoints * sizeof(double));

    for (cont = 0; cont < npoints; cont++) {
        s[cont]      = s3[cont];
        len[cont]    = len3[cont];
        alphax[cont] = alphax3[cont];
        alphaz[cont] = alphaz3[cont];
        betaz[cont]  = betaz3[cont];
        betax[cont]  = betax3[cont];
        dispx[cont]  = dispx3[cont];
        disp1x[cont] = disp1x3[cont];
        dispz[cont]  = dispz3[cont];
        disp1z[cont] = disp1z3[cont];
        nux[cont]    = nux3[cont];
        nuz[cont]    = nuz3[cont];
        nrep[cont]   = nrep3[cont];
        lrep[cont]   = lrep3[cont];
    }

    free(s3);
    free(len3);
    free(alphax3);
    free(alphaz3);
    free(betax3);
    free(betaz3);
    free(dispx3);
    free(disp1x3);
    free(dispz3);
    free(disp1z3);
    free(nux3);
    free(nuz3);
    free(nrep3);
    free(lrep3);

    return 0;
}
// END RECURRENCES SUBROUTINE /////////////////////////////////////////////////////

// BEGIN RECURRENCES2 SUBROUTINE /////////////////////////////////////////////////////
int recurrences2(void) {
    double *s3, *len3, *alphax3, *alphaz3, *betax3, *betaz3, *dispx3, *disp1x3, *dispz3, *disp1z3, *nux3, *nuz3, *lrep3;
    int *nrep3;
    int start = 1, start1, current = 2, current1, start2, flag = 1, flag1, flag2, cont, cont1, current3 = 1, current4;
    double precision = 0.3;

    s3      = (double *)malloc(npoints * sizeof(double));
    len3    = (double *)malloc(npoints * sizeof(double));
    alphax3 = (double *)malloc(npoints * sizeof(double));
    alphaz3 = (double *)malloc(npoints * sizeof(double));
    betax3  = (double *)malloc(npoints * sizeof(double));
    betaz3  = (double *)malloc(npoints * sizeof(double));
    dispx3  = (double *)malloc(npoints * sizeof(double));
    disp1x3 = (double *)malloc(npoints * sizeof(double));
    dispz3  = (double *)malloc(npoints * sizeof(double));
    disp1z3 = (double *)malloc(npoints * sizeof(double));
    nux3    = (double *)malloc(npoints * sizeof(double));
    nuz3    = (double *)malloc(npoints * sizeof(double));
    nrep3   = (int *)malloc(npoints * sizeof(int));
    lrep3   = (double *)malloc(npoints * sizeof(double));

    s3[0]      = s[0];
    len3[0]    = len[0];
    alphax3[0] = alphax[0];
    alphaz3[0] = alphaz[0];
    betaz3[0]  = betaz[0];
    betax3[0]  = betax[0];
    dispx3[0]  = dispx[0];
    disp1x3[0] = disp1x[0];
    dispz3[0]  = dispz[0];
    disp1z3[0] = disp1z[0];
    nux3[0]    = nux[0];
    nuz3[0]    = nuz[0];
    nrep3[0]   = 1;
    lrep3[0]   = lrep[0];

    while (flag) {
        start1 = start;
        while (flag) {
            if ((fabs(alphax[start1]) > 0.1) && (fabs(alphax[current]) > 0.1)) {
                if (fabs(alphax[current] / alphax[start1] - 1.0) > precision) {
                    flag = 0;
                }
            }
            if ((fabs(alphaz[start1]) > 0.1) && (fabs(alphaz[current]) > 0.1)) {
                if (fabs(alphaz[current] / alphaz[start1] - 1.0) > precision) {
                    flag = 0;
                }
            }
            if (betaz[start1] != 0) {
                if (fabs(betaz[current] / betaz[start1] - 1.0) > precision) {
                    flag = 0;
                }
            }
            if (betax[start1] != 0) {
                if (fabs(betax[current] / betax[start1] - 1.0) > precision) {
                    flag = 0;
                }
            }
            if ((fabs(dispx[start1]) > 0.001) && (fabs(dispx[current]) > 0.001)) {
                if (fabs(dispx[current] / dispx[start1] - 1.0) > precision) {
                    flag = 0;
                }
            }

            if ((nuz[start1] - nuz[start1 - 1]) != 0) {
                if (fabs((nuz[current] - nuz[current - 1]) / (nuz[start1] - nuz[start1 - 1]) - 1.0) > precision) {
                    flag = 0;
                }
            }
            if ((nux[start1] - nux[start1 - 1]) != 0) {
                if (fabs((nux[current] - nux[current - 1]) / (nux[start1] - nux[start1 - 1]) - 1.0) > precision) {
                    flag = 0;
                }
            }

            if (flag) {
                //	      printf("trovato %d,%d\n",start1,current);
                flag1    = 1;
                current1 = current;
                start2   = start1;
                flag2    = 1;

                while (flag1 * flag2) {
                    start2++;
                    current1++;
                    flag1 = check(start2, current1, precision);
                    if (start2 >= (current - 1)) {
                        flag2 = 0;
                    }
                }
                if (flag1) {
                    for (cont = 0; cont < (start1 - start); cont++) {
                        s3[current3 + cont]      = s[start + cont];
                        len3[current3 + cont]    = len[start + cont];
                        alphax3[current3 + cont] = alphax[start + cont];
                        alphaz3[current3 + cont] = alphaz[start + cont];
                        betaz3[current3 + cont]  = betaz[start + cont];
                        betax3[current3 + cont]  = betax[start + cont];
                        dispx3[current3 + cont]  = dispx[start + cont];
                        disp1x3[current3 + cont] = disp1x[start + cont];
                        dispz3[current3 + cont]  = dispz[start + cont];
                        disp1z3[current3 + cont] = disp1z[start + cont];
                        nux3[current3 + cont]    = nux[start + cont];
                        nuz3[current3 + cont]    = nuz[start + cont];
                        nrep3[current3 + cont]   = nrep[start + cont];
                        lrep3[current3 + cont]   = lrep[start + cont];
                    }
                    current3 = current3 + start1 - start;

                    printf("Bene1\n");

                    for (cont = 0; cont < (current - start1); cont++) {
                        s3[current3 + cont]      = s[start1 + cont];
                        len3[current3 + cont]    = len[start1 + cont];
                        alphax3[current3 + cont] = alphax[start1 + cont];
                        alphaz3[current3 + cont] = alphaz[start1 + cont];
                        betaz3[current3 + cont]  = betaz[start1 + cont];
                        betax3[current3 + cont]  = betax[start1 + cont];
                        dispx3[current3 + cont]  = dispx[start1 + cont];
                        disp1x3[current3 + cont] = disp1x[start1 + cont];
                        dispz3[current3 + cont]  = dispz[start1 + cont];
                        disp1z3[current3 + cont] = disp1z[start1 + cont];
                        nux3[current3 + cont]    = nux[start1 + cont];
                        nuz3[current3 + cont]    = nuz[start1 + cont];
                        nrep3[current3 + cont]   = nrep[start1 + cont] + nrep[current + cont];
                        lrep3[current3 + cont]   = lrep[start1 + cont] + lrep[current + cont];
                    }
                    flag2 = 1;
                    cont1 = 2;

                    printf("Bene2\n");

                    while (flag1) {
                        flag2 = 1;
                        cont  = 0;
                        current1++;
                        while (flag1 * flag2) {
                            flag1 = check(current1 + cont, start1 + cont, precision);
                            cont++;
                            if (cont >= (current - start1)) {
                                flag2 = 0;
                            }
                        }

                        if (flag1) {
                            current4 = current1;
                            current1 = current1 + cont - 1;
                            cont1++;
                            for (cont = 0; cont < (current - start1); cont++) {
                                nrep3[current3 + cont] = nrep3[current3 + cont] + nrep[current4 + cont];
                                lrep3[current3 + cont] = lrep3[current3 + cont] + lrep[current4 + cont];
                            }
                            printf("Bene3\n");
                        }
                        else {
                            current3 = current3 + current - start1;
                            start    = start1 + cont1 * (current - start1);
                            current  = start + 1;
                            start1   = start;
                        }
                    }
                }
            }

            if (start1 >= (current - 1)) {
                flag = 0;
            }
            else {
                flag = 1;
                start1++;
            }
        }

        if (current >= (npoints - 3)) {
            for (cont = 0; cont < (npoints - start); cont++) {
                s3[current3 + cont]      = s[start + cont];
                len3[current3 + cont]    = len[start + cont];
                alphax3[current3 + cont] = alphax[start + cont];
                alphaz3[current3 + cont] = alphaz[start + cont];
                betaz3[current3 + cont]  = betaz[start + cont];
                betax3[current3 + cont]  = betax[start + cont];
                dispx3[current3 + cont]  = dispx[start + cont];
                disp1x3[current3 + cont] = disp1x[start + cont];
                dispz3[current3 + cont]  = dispz[start + cont];
                disp1z3[current3 + cont] = disp1z[start + cont];
                nux3[current3 + cont]    = nux[start + cont];
                nuz3[current3 + cont]    = nuz[start + cont];
                nrep3[current3 + cont]   = nrep[start + cont];
                lrep3[current3 + cont]   = lrep[start + cont];
            }
            npoints = current3 + npoints - start;
            flag    = 0;
        }
        else {
            flag = 1;
            current++;
        }
    }

    free(s);
    free(len);
    free(alphax);
    free(alphaz);
    free(betax);
    free(betaz);
    free(dispx);
    free(disp1x);
    free(dispz);
    free(disp1z);
    free(nux);
    free(nuz);
    free(nrep);
    free(lrep);

    s      = (double *)malloc(npoints * sizeof(double));
    len    = (double *)malloc(npoints * sizeof(double));
    alphax = (double *)malloc(npoints * sizeof(double));
    alphaz = (double *)malloc(npoints * sizeof(double));
    betax  = (double *)malloc(npoints * sizeof(double));
    betaz  = (double *)malloc(npoints * sizeof(double));
    dispx  = (double *)malloc(npoints * sizeof(double));
    disp1x = (double *)malloc(npoints * sizeof(double));
    dispz  = (double *)malloc(npoints * sizeof(double));
    disp1z = (double *)malloc(npoints * sizeof(double));
    nux    = (double *)malloc(npoints * sizeof(double));
    nuz    = (double *)malloc(npoints * sizeof(double));
    nrep   = (int *)malloc(npoints * sizeof(int));
    lrep   = (double *)malloc(npoints * sizeof(double));

    for (cont = 0; cont < npoints; cont++) {
        s[cont]      = s3[cont];
        len[cont]    = len3[cont];
        alphax[cont] = alphax3[cont];
        alphaz[cont] = alphaz3[cont];
        betaz[cont]  = betaz3[cont];
        betax[cont]  = betax3[cont];
        dispx[cont]  = dispx3[cont];
        disp1x[cont] = disp1x3[cont];
        dispz[cont]  = dispz3[cont];
        disp1z[cont] = disp1z3[cont];
        nux[cont]    = nux3[cont];
        nuz[cont]    = nuz3[cont];
        nrep[cont]   = nrep3[cont];
        lrep[cont]   = lrep3[cont];
    }

    free(s3);
    free(len3);
    free(alphax3);
    free(alphaz3);
    free(betax3);
    free(betaz3);
    free(dispx3);
    free(disp1x3);
    free(dispz3);
    free(disp1z3);
    free(nux3);
    free(nuz3);
    free(nrep3);
    free(lrep3);

    return 0;
}
// END RECURRENCES2 SUBROUTINE /////////////////////////////////////////////////////

// BEGIN INVTOMOM SUBROUTINE /////////////////////////////////////////////////////
// Convert from invariants to momenta
// Provided i argument is the index of the element in the lattice at which to do the conversion
// and is used to get the optics functions at this element which are needed for the conversion
int invtomom(int i) {
    int cont;
    double prova;
    FILE *fcoord;
    char filecoord[] = "coordin_A.txt";

    x       = (double *)malloc(numpart * sizeof(double));
    xp      = (double *)malloc(numpart * sizeof(double));
    z       = (double *)malloc(numpart * sizeof(double));
    zp      = (double *)malloc(numpart * sizeof(double));
    deltasp = (double *)malloc(numpart * sizeof(double));
    deltap  = (double *)malloc(numpart * sizeof(double));

    prova = ran2();

    for (cont = 0; cont < numpart; cont++) {
        phix[cont]    = 2.0 * pi * ran2();
        phiz[cont]    = 2.0 * pi * ran2();
        phis[cont]    = 2.0 * pi * ran2();
        deltap[cont]  = sqrt(es[cont]) * cos(phis[cont]);
        deltasp[cont] = sqrt(es[cont]) * sin(phis[cont]) / invtune;
        x[cont]       = sqrt(ex[cont] * betax[i]) * cos(phix[cont]) + dispx[i] * deltap[cont];
        xp[cont]      = -sqrt(ex[cont] / betax[i]) * (alphax[i] * cos(phix[cont]) + sin(phix[cont])) + disp1x[i] * deltap[cont];
        z[cont]       = sqrt(ez[cont] * betaz[i]) * cos(phiz[cont]) + dispz[i] * deltap[cont];
        zp[cont]      = -sqrt(ez[cont] / betaz[i]) * (alphaz[i] * cos(phiz[cont]) + sin(phiz[cont])) + disp1z[i] * deltap[cont];
    }

    // My own addition: outputting the converted distribution to file
    foutput = fopen("mom_dist.txt", "w");
    fprintf(foutput, "x, xp, y, yp, deltap, deltasp\n");
    for (cont=0; cont<numpart; cont++) {
        fprintf(foutput, "%.9e, %.9e, %.9e, %.9e, %.9e, %.9e\n", x[cont], xp[cont], z[cont], zp[cont], deltap[cont], deltasp[cont]);
    }
    fclose(foutput);

    free(ex);
    free(ez);
    free(es);
    free(phix);
    free(phiz);
    free(phis);

    return 0;
}
// END INVTOMOM SUBROUTINE /////////////////////////////////////////////////////

// BEGIN MOMTOINV SUBROUTINE /////////////////////////////////////////////////////
// Convert from momenta to invariants
// Provided i argument is the index of the element in the lattice at which to do the conversion
// and is used to get the optics functions at this element which are needed for the conversion
int momtoinv(int i) {
    int cont;

    ex   = (double *)malloc(numpart * sizeof(double));
    ez   = (double *)malloc(numpart * sizeof(double));
    es   = (double *)malloc(numpart * sizeof(double));
    phix = (double *)malloc(numpart * sizeof(double));
    phiz = (double *)malloc(numpart * sizeof(double));
    phis = (double *)malloc(numpart * sizeof(double));

    for (cont = 0; cont < numpart; cont++) {
        ex[cont] = betax[i] * (xp[cont] - disp1x[i] * deltap[cont]) * (xp[cont] - disp1x[i] * deltap[cont]) +
                   2.0 * alphax[i] * (xp[cont] - disp1x[i] * deltap[cont]) * (x[cont] - dispx[i] * deltap[cont]) +
                   (1 + alphax[i] * alphax[i]) / betax[i] * (x[cont] - dispx[i] * deltap[cont]) * (x[cont] - dispx[i] * deltap[cont]);
        ez[cont] = betaz[i] * (zp[cont] - disp1z[i] * deltap[cont]) * (zp[cont] - disp1z[i] * deltap[cont]) +
                   2.0 * alphaz[i] * (zp[cont] - disp1z[i] * deltap[cont]) * (z[cont] - dispz[i] * deltap[cont]) +
                   (1 + alphaz[i] * alphaz[i]) / betaz[i] * (z[cont] - dispz[i] * deltap[cont]) * (z[cont] - dispz[i] * deltap[cont]);
        es[cont]   = deltap[cont] * deltap[cont] + invtune * invtune * deltasp[cont] * deltasp[cont];
        phis[cont] = atan(invtune * deltasp[cont] / deltap[cont]);
        if (deltap[cont] < 0) {
            phis[cont] = phis[cont] + pi;
        }
        phix[cont] = atan(-(xp[cont] - disp1x[i] * deltap[cont]) / (x[cont] - dispx[i] * deltap[cont]) * betax[i] - alphax[i]);
        if (x[cont] - dispx[i] * deltap[cont] < 0) {
            phix[cont] = phix[cont] + pi;
        }
        phiz[cont] = atan(-(zp[cont] - disp1z[i] * deltap[cont]) / (z[cont] - dispz[i] * deltap[cont]) * betaz[i] - alphaz[i]);
        if (z[cont] - dispz[i] * deltap[cont] < 0) {
            phiz[cont] = phiz[cont] + pi;
        }
    }

    free(x);
    free(z);
    free(deltasp);
    free(xp);
    free(zp);
    free(deltap);

    return 0;
}
// END MOMTOINV SUBROUTINE /////////////////////////////////////////////////////

// BEGIN IBS SUBROUTINE /////////////////////////////////////////////////////
int IBS(void) {
    FILE *fprova;
    double maxx, minx, maxz, minz, maxs, mins;
    double density, totx, totz, tots;
    int cont, cont2, nump, scatterflag, dummy, dummy1, dummy2;
    int *cell, *npart, **part, *ncol;
    int flag, comodino, j1, j2, conteur = 0, limit1, limit2, ncolcel;

    // Start putting 'any' value from these (they are read from the MAD-X Twiss)
    maxx = x[0];
    minx = x[0];
    maxz = z[0];
    minz = z[0];
    maxs = deltasp[0];
    mins = deltasp[0];

    // Finds the limits of the distributions: max and min in each dimension
    for (cont = 1; cont < numpart; cont++) {
        if (x[cont] < minx) {
            minx = x[cont];
        }
        else {
            if (x[cont] > maxx) {
                maxx = x[cont];
            }
        }
        if (z[cont] < minz) {
            minz = z[cont];
        }
        else {
            if (z[cont] > maxz) {
                maxz = z[cont];
            }
        }
        if (deltasp[cont] < mins) {
            mins = deltasp[cont];
        }
        else {
            if (deltasp[cont] > maxs) {
                maxs = deltasp[cont];
            }
        }
    }

    // Sets a cut on the minimum and maximum of the distribution of macroparticles (??)
    maxx = maxx * 1.0001;
    minx = minx * 1.0001;
    maxz = maxz * 1.0001;
    minz = minz * 1.0001;
    maxs = maxs * 1.0001;
    mins = mins * 1.0001;
    totx = (maxx - minx);
    totz = (maxz - minz);
    tots = (maxs - mins);

    // Outputting the determined limits of the mesh
    cout << "Min x = " << minx << endl;
    cout << "Max x = " << maxx << endl;
    cout << "Min y = " << minz << endl;
    cout << "Max y = " << maxz << endl;
    cout << "Min z = " << mins << endl;
    cout << "Max z = " << maxs << endl;

    // Define the size of each cell.
    // The cells are equal with respect to their size in space (not current)
    // Each cell has to contain not too few but also not too many macroparticles to take into account of interactions with close encounters.
    // We are interested only in binary collisions due to charge. The further distance encounters will contribute to space charge, not the IBS
    cout << "flag_def = " << flag_def << endl;
    cout << "flag_renorm = " << flag_renorm << endl;
    if (flag_def == 1) {  // true when (KINJ == KINJ1 && i == 0) in the main loop (i.e. not first IBS effect, not first element)
        deltacellx = totx / ncellx;
        deltacellz = totz / ncellz;
        deltacells = tots / ncells;
    }
    if (flag_renorm == 1) {  // true when (KINJ > KINJ1 || i > 0) in the main loop (i.e. )
        ncellx = ceil(totx / deltacellx);
        ncellz = ceil(totz / deltacellz);
        ncells = ceil(tots / deltacells);
    }

    // We cap the number of cells to 999 in every direction and find the
    // 'deltacell' which is the size of the cell in a given direction
    if (ncellx >= 1000) {
        ncellx     = 999;
        deltacellx = totx / ncellx;
    }
    if (ncellz >= 999) {
        ncellz     = 999;
        deltacellz = totz / ncellz;
    }
    if (ncells >= 1000) {
        ncells     = 999;
        deltacells = tots / ncells;
    }

    // Print this out to the command line
    cout << "deltacellx = " << deltacellx << endl;
    cout << "ncellx = " << ncellx << endl;
    cout << "deltacellz = " << deltacellz << endl;
    cout << "ncellz = " << ncellz << endl;
    cout << "deltacells = " << deltacells << endl;
    cout << "ncells = " << ncells << endl;

    ncellt   = ncellx * ncellz; // Number of cells in the transverse plane
    ncelltot = ncellt * ncells; // Total number of cells

    ///////////////////////////////////////
    // BEGIN GROUPING PARTICLES IN CELLS //
    ///////////////////////////////////////
    cell  = (int *)malloc(numpart * sizeof(int));  // characteristic integer (see below) for each particle
    npart = (int *)calloc(ncelltot, sizeof(int));  // number of particles in the cell of the index (at idx 75 is number of parts in cell 75)
    part  = (int **)malloc(ncelltot * sizeof(int *));  // array with for each cell ID and array of all the particle IDS in that cell

    // We loop over all particles and place them in cells by assigning a characteristic
    // integer: all macroparticles in the same cell will have the same integer
    for (cont = 0; cont < numpart; cont++) {
        cell[cont] = ((int)floor((x[cont] - minx) / deltacellx))
                     + ((int)floor((z[cont] - minz) / deltacellz)) * ncellx
                     + ((int)floor((deltasp[cont] - mins) / deltacells)) * ncellt ; // calculation of the integer for this particle
        npart[cell[cont]]++; // counts the number of macroparticles with the same integer
    }
    cout << "Particle Cells Attribution: " << cell << endl;  // for reproduction / debugging

    // loop over the cells to do???
    for (cont2 = 0; cont2 < ncelltot; cont2++) {
        // for each cell (cont2) we allocate in part[cont2] an array (of ints)
        // which will hold the indices of every particle in that given cell
        part[cont2] = (int *)malloc(npart[cont2] * sizeof(int));
    }

    free(npart);
    npart = (int *)calloc(ncelltot, sizeof(int)); // calloc is used to set memory to zero
    fflush(stdout);

    // We look over all particles
    for (cont = 0; cont < numpart; cont++) {
        part[cell[cont]][npart[cell[cont]]] = cont; // store the ID of this particle in part[cell[cont]] which stores all particles in that cell
        npart[cell[cont]]++;                        // increment the known number of particles in that cell (cell[cont])
    }
    /////////////////////////////////////
    // END GROUPING PARTICLES IN CELLS //
    /////////////////////////////////////

    ////////////////////////////////////////////////////////////
    // BEGIN SHUFFLING & SCATTERING OF PARTICLES IN EACH CELL //
    ////////////////////////////////////////////////////////////
    // We loop over the cells
    for (cont2 = 0; cont2 < ncelltot; cont2++) {
        limit1 = npart[cont2] - 1; // number of particles in this cell - 1 - determines maybe ncollisions done

        cout << endl;  // for reproduction / debugging
        cout << "CELL NUMBER: " << cont2 << endl;  // for reproduction / debugging
        cout << "NUMBER OF PARTICLES = " << npart[cont2] << endl;  // for reproduction / debugging

        // Determine the number of collisions to do for this cell
        if (ncollisions >= npart[cont2]) { // if number of collisions per macroparticle (in inputs file) > number of parts in this cell
            ncolcel = limit1;
        }
        else {
            ncolcel = ncollisions;  // number of collisions to do for this cell
        }

        ncol    = (int *)calloc(npart[cont2], sizeof(int)); // number of collisions for a given part or cell??
        density = npart[cont2] * realn / deltacellx / deltacellz / deltacells / ncolcel; // cell density number
        // density = n_macroparticles_in_cell * n_real_particles_per_macroparticle / (dx * dy * dz / number_of_collisions)
        limit2  = limit1;  // starts as nparticles in cell - 1

        cout << "dimp parameter (totz) = " << totz << endl;  // for reproduction / debugging
        cout << "Cell density = " << density << endl;  // for reproduction / debugging
        cout << endl;  // for reproduction / debugging

        // Loop as long as the number of ??? is not 0 (collisions to do in the cell?)
        // Here limit1 starts as the number of particles in the cell - 1
        while (limit1 > 0) {
            if (limit1 == 0) {  // literally we can't enter that condition lol
                printf("Errore limit1=%d!\n", limit1);
                exit(1);
            }

            nump   = ncolcel - ncol[limit1];  // number of collisions left for this cell - starts as the total number to do minus those done by the currently selected particle
            dummy1 = part[cont2][limit1];  // the ID of a particle - starts as the "last" in the cell

            // Loop over the number of collisions left
            for (cont = 0; cont < nump; cont++) {
                dummy  = (int)floor(ran2() * limit2);  // random integer to be used to choose the paired particle
                dummy2 = part[cont2][dummy];  // the ID of the second particle in the pair

                // Call scatter function that takes care of computing the Coulomb
                // kick and changing momenta for these two particles
                scatterflag = scatter(dummy1, dummy2, totz, density);

                // Here I have no clue...
                // As evidenced by debugging output, logic is flawed as some particles collide with themselves
                limit2--;
                comodino            = ncol[dummy] + 1;
                ncol[dummy]         = ncol[limit2];
                ncol[limit2]        = comodino;
                part[cont2][dummy]  = part[cont2][limit2];
                part[cont2][limit2] = dummy2;

                // Also no clue
                if (ncol[limit2] == ncolcel) {
                    limit1--;
                    comodino            = ncol[limit1];
                    ncol[limit1]        = ncol[limit2];
                    ncol[limit2]        = comodino;
                    part[cont2][limit2] = part[cont2][limit1];
                    part[cont2][limit1] = dummy2;
                }
                if (limit2 == 0) {
                    limit2 = limit1;
                }
            }
            limit1--;
        }
        free(ncol);
        free(part[cont2]);
    }
    //////////////////////////////////////////////////////////
    // END SHUFFLING & SCATTERING OF PARTICLES IN EACH CELL //
    //////////////////////////////////////////////////////////

    free(npart);
    free(cell);
    free(part);
    return 0;
}
// END IBS SUBROUTINE /////////////////////////////////////////////////////

// BEGIN SCATTER SUBROUTINE /////////////////////////////////////////////////////
// Applies the scattering between two provided particles. This function relies on a FUCKTON of global variables.
// part1 should be the ID of particle 1 or something
// part2 should be the ID of particle 1 or something
// dimp is given as 'totz' at call (in IBS func) which is the (maxz - minz) - total space in Y (remember in SIRE vertical is Z)
// dens is the computed density (in IBS func)
int scatter(int part1, int part2, double dimp, double dens) {
    double Deltapcmx, Deltapcmz, Deltapcms;
    double Deltapcmt, Deltapcmn;
    double Deltap1cmx, Deltap1cmz, Deltap1cms;
    double Psi, Phi, cosphi, sinphi, cospsi, sinpsi;
    double betatilda, coulomb;
    double oneminuscospsi;

    cout << "Particle 1 momenta: " << xp[part1] << " " << zp[part1] << " " << deltap[part1] << endl;  // for reproduction / debugging
    cout << "Particle 2 momenta: " << xp[part2] << " " << zp[part2] << " " << deltap[part2] << endl;  // for reproduction / debugging

    // Computing the delta of relevant coordinates (momenta) between the two particles
    // Think this is computed in the center of mass (pcm)
    Deltapcmx = xp[part1] - xp[part2];  // in horizontal
    Deltapcmz = zp[part1] - zp[part2];  // in vertical
    Deltapcms = (deltap[part1] - deltap[part2]) / gammap;  // in longitudinal, gammap is relativistic gamma
    cout << "Deltapcmx = " << Deltapcmx << endl;  // for reproduction / debugging
    cout << "Deltapcmy = " << Deltapcmz << endl;  // for reproduction / debugging
    cout << "Deltapcmz = " << Deltapcms << endl;  // for reproduction / debugging

    // Total momentum change in transverse and then in total?
    Deltapcmt = sqrt(Deltapcmx * Deltapcmx + Deltapcmz * Deltapcmz);
    Deltapcmn = sqrt(Deltapcmt * Deltapcmt + Deltapcms * Deltapcms);
    cout << "Deltapcmt = " << Deltapcmt << endl;
    cout << "Detlapcmn = " << Deltapcmn << endl;

    // The angle change of colliding particles
    Phi       = 2 * pi * ran2(); // The azimuthal angle, which is chosen randomly
    cout << "Azimuthal angle (Phi) = " << Phi << endl;  // for reproduction / debugging
    cosphi    = cos(Phi);
    sinphi    = sin(Phi);

    cout << "cosphi = " << cosphi << endl;  // for reproduction / debugging
    cout << "sinphi = " << sinphi << endl;  // for reproduction / debugging
    cout << "beta = " << betarel << endl;  // for reproduction / debugging
    cout << "gammap = " << gammap << endl;  // for reproduction / debugging

    betatilda = betarel * gammap / 2.0 * Deltapcmn;
    coulomb   = dimp * betatilda * betatilda / radius;  // to be used for the coulomb logarithm?
    cout << "betatilda = " << betatilda << endl;  // for reproduction / debugging
    cout << "Coulomb = " << coulomb << endl;  // for reproduction / debugging

    if (coulomb > 1) {
        coulomb        = log(coulomb);
	    cout << "Coulog = " << coulomb << endl;  // for reproduction / debugging
        oneminuscospsi = twopicvel * dens * radius * radius * deltat * coulomb / gammap / gammap / betatilda / betatilda / betatilda;
        cout << "oneminuscospsi = " << oneminuscospsi << endl;  // for reproduction / debugging
        sinpsi         = sq2 * sqrt(oneminuscospsi); // The azimuthal collision angle

        // Assuming Rutherford scattering, an effective scattering angle is computed (statistical effect)
        // The change in momentum will then be calculated based on this effective angle
        if (Deltapcmt) { // If transverse momentum change, it's taken into account
            Deltap1cmx = (-Deltapcmx * oneminuscospsi + (sinpsi * cosphi * Deltapcmx * Deltapcms - sinpsi * sinphi * Deltapcmn * Deltapcmz) / Deltapcmt) / 2.0;
            Deltap1cmz = (-Deltapcmz * oneminuscospsi + (sinpsi * cosphi * Deltapcmz * Deltapcms - sinpsi * sinphi * Deltapcmn * Deltapcmx) / Deltapcmt) / 2.0;
            Deltap1cms = (-Deltapcms * oneminuscospsi - sinpsi * cosphi * Deltapcmt) / 2.0 * gammap;
        }
        else {  // If transverse momentum conservation, only use
            Deltap1cmx = Deltapcmn * sinpsi * cosphi / 2.0;
            Deltap1cmz = Deltapcmn * sinpsi * sinphi / 2.0;
            Deltap1cms = -Deltapcmn * oneminuscospsi / 2.0 * gammap;
        }

        cout << "Deltas part 1 (+): " << Deltap1cmx << " " << Deltap1cmz << " " << Deltap1cms << endl;  // for reproduction / debugging
        cout << endl;  // for reproduction / debugging

        // We modify the coordinates of the 2 particles directly from this computed delta
        xp[part1]     = xp[part1] + Deltap1cmx;
        xp[part2]     = xp[part2] - Deltap1cmx;
        zp[part1]     = zp[part1] + Deltap1cmz;
        zp[part2]     = zp[part2] - Deltap1cmz;
        deltap[part1] = deltap[part1] + Deltap1cms;
        deltap[part2] = deltap[part2] - Deltap1cms;
    }
    return 0;
}
// END SCATTER SUBROUTINE /////////////////////////////////////////////////////
