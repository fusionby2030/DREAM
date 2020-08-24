/**
 * This file contains definitions of the enums that are used
 * to indicate options in DREAM. The file is included into the
 * definition of the 'SimulationGenerator' class.
 */

/////////////////////////////////////
///
/// INPUT DATA OPTIONS
///
/////////////////////////////////////
// Interpolation using our own 'Interpolator1D' object
enum prescribed_data_interp {
    // We start from 0 here to remain somewhat compatible
    // with the GSL interpolation interface
    PRESCRIBED_DATA_INTERP_NEAREST=0,
    PRESCRIBED_DATA_INTERP_LINEAR=1
};
enum prescribed_data_interp_gsl {
    PRESCRIBED_DATA_INTERP_GSL_LINEAR=1,
    PRESCRIBED_DATA_INTERP_GSL_POLYNOMIAL=2,
    PRESCRIBED_DATA_INTERP_GSL_CSPLINE=3,
    PRESCRIBED_DATA_INTERP_GSL_AKIMA=4,
    PRESCRIBED_DATA_INTERP_GSL_STEFFEN=5
};
enum prescribed_data_interp3d {
    PRESCRIBED_DATA_INTERP3D_NEAREST=0,
    PRESCRIBED_DATA_INTERP3D_LINEAR=1
};

enum ion_data_type {
    ION_DATA_PRESCRIBED=1,
    ION_DATA_EQUILIBRIUM=2,
    ION_DATA_TYPE_DYNAMIC=3
};

// Interpolation method for ADAS rate coefficients
enum adas_interp_type {
    ADAS_INTERP_BILINEAR=1,
    ADAS_INTERP_BICUBIC=2
};

/////////////////////////////////////
///
/// GRID OPTIONS
///
/////////////////////////////////////
// Type of radial grid
enum radialgrid_type {
    RADIALGRID_TYPE_CYLINDRICAL=1
};

// Type of momentum grid
enum momentumgrid_type {
    MOMENTUMGRID_TYPE_PXI=1,
    MOMENTUMGRID_TYPE_PPARPPERP=2
};

// Type of p grid
enum pxigrid_ptype {
    PXIGRID_PTYPE_UNIFORM=1,
    PXIGRID_PTYPE_BIUNIFORM=2
};

// Type of xi grid
enum pxigrid_xitype {
    PXIGRID_XITYPE_UNIFORM=1
};

/////////////////////////////////////
///
/// SOLVER OPTIONS
///
/////////////////////////////////////
enum solver_type {
    SOLVER_TYPE_LINEARLY_IMPLICIT=1,
    SOLVER_TYPE_NONLINEAR=2,
    SOLVER_TYPE_NONLINEAR_SNES=3
};
// Linear solver type (used by both the linear-implicit
// and nonlinear solvers)
enum linear_solver {
    LINEAR_SOLVER_LU=1,
    LINEAR_SOLVER_GMRES=2
};

/////////////////////////////////////
///
/// TIME STEPPER OPTIONS
///
/////////////////////////////////////
enum timestepper_type {
    TIMESTEPPER_TYPE_CONSTANT=1,
    TIMESTEPPER_TYPE_ADAPTIVE=2
};

/////////////////////////////////////
///
/// UNKNOWN QUANTITY OPTIONS
///
/////////////////////////////////////
enum uqty_E_field_eqn {
    UQTY_E_FIELD_EQN_PRESCRIBED=1,     // E_field is prescribed by the user
    UQTY_E_FIELD_EQN_SELFCONSISTENT=2, // E_field is prescribed by the user
};

enum uqty_V_loop_wall_eqn {
    UQTY_V_LOOP_WALL_EQN_PRESCRIBED=1,     // V_loop on wall (r=b) is prescribed by the user
    UQTY_V_LOOP_WALL_EQN_SELFCONSISTENT=2, // V_loop on wall is evolved self-consistently
};

enum uqty_n_cold_eqn {
    UQTY_N_COLD_EQN_PRESCRIBED=1,    // n_cold is calculated from ion species as sum_i n_i Z_i
    UQTY_N_COLD_EQN_SELFCONSISTENT=2 // n_cold is calculated from charge neutrality as sum_i n_i Z_i  - n_hot - n_RE
};

enum uqty_T_cold_eqn {
    UQTY_T_COLD_EQN_PRESCRIBED=1,   // T_cold prescribed by the user
    UQTY_T_COLD_SELF_CONSISTENT=2   // T_cold calculated self-consistently
};

/////////////////////////////////////
///
/// COLLISION QUANTITY HANDLER SETTINGS
///
/////////////////////////////////////
enum collqty_lnLambda_type {             // The Coulomb logarithm is... 
    COLLQTY_LNLAMBDA_CONSTANT=1,         // the relativistic lnLambda, lnL = lnLc
    COLLQTY_LNLAMBDA_ENERGY_DEPENDENT=2, // energy dependent, separate for collisions with electrons and ions
    COLLQTY_LNLAMBDA_THERMAL=3           // the thermal lnLambda, lnL = lnLT
};

enum collqty_collfreq_mode {
    COLLQTY_COLLISION_FREQUENCY_MODE_SUPERTHERMAL=1,      // Collision frequencies given in limit T->0 (except in Coulomb logarithm where T_cold enters)
    COLLQTY_COLLISION_FREQUENCY_MODE_FULL=2,              // Full collision frequencies (with Chandrasekhar and error functions etc in the non-relativistic case)
    COLLQTY_COLLISION_FREQUENCY_MODE_ULTRA_RELATIVISTIC=3 // Collision frequencies given in the limit p>>mc, i.e. ignoring the 1/v^2 behavior but keeping the logarithmic increase with gamma
};

enum collqty_collfreq_type {
    COLLQTY_COLLISION_FREQUENCY_TYPE_COMPLETELY_SCREENED=1, // only free electrons contribute 
    COLLQTY_COLLISION_FREQUENCY_TYPE_NON_SCREENED=2,        // free and bound electrons contribute equally
    COLLQTY_COLLISION_FREQUENCY_TYPE_PARTIALLY_SCREENED=3   // bound electrons contribute via mean excitation energies etc
};

enum collqty_pstar_mode {                // Runaway growth rates are determined from dynamics that are
    COLLQTY_PSTAR_MODE_COLLISIONAL = 1,  // collisional (no trapping correction)
    COLLQTY_PSTAR_MODE_COLLISIONLESS = 2 // collisionless (with trapping correction)
};

enum collqty_Eceff_mode {
    COLLQTY_ECEFF_MODE_CYLINDRICAL = 1, // Sets Eceff using the Hesslow formula ignoring trapping effects.
    COLLQTY_ECEFF_MODE_SIMPLE = 2,      // An approximate numerical calculation with a simplified account of trapping effects
    COLLQTY_ECEFF_MODE_FULL = 3         // Full 'Lehtinen theory' expression.
};

/////////////////////////////////////
///
/// EQUATION TERM OPTIONS
///
/////////////////////////////////////

enum eqterm_avalanche_mode {            // Avalanche generation is...
    EQTERM_AVALANCHE_MODE_NEGLECT = 1,  // neglect
    EQTERM_AVALANCHE_MODE_FLUID = 2,    // modelled with fluid growth rate formula
    EQTERM_AVALANCHE_MODE_KINETIC = 3   // modelled kinetically with RP avalanche source
};

enum eqterm_nonlinear_mode {                     // Non-linear self-collisions are...
    EQTERM_NONLINEAR_MODE_NEGLECT = 1,           // neglected
    EQTERM_NONLINEAR_MODE_NON_REL_ISOTROPIC = 2, // accounted for with isotropic Landau-Fokker-Planck operator 
    EQTERM_NONLINEAR_MODE_NORSEPP = 3            // included with full NORSE++ formalism
};

enum eqterm_bremsstrahlung_mode {                // Bremsstrahlung radiation reaction is...
    EQTERM_BREMSSTRAHLUNG_MODE_NEGLECT=1,        // neglected
    EQTERM_BREMSSTRAHLUNG_MODE_STOPPING_POWER=2, // accounted for with an effective force F_br(p)
    EQTERM_BREMSSTRAHLUNG_MODE_BOLTZMANN=3       // accounted for with a linear (Boltzmann) integral operator
};

enum eqterm_synchrotron_mode {         // Synchrotron radiation reaction is...
    EQTERM_SYNCHROTRON_MODE_NEGLECT=1, // neglected 
    EQTERM_SYNCHROTRON_MODE_INCLUDE=2  // included
};

enum eqterm_dreicer_mode {
    EQTERM_DREICER_MODE_NONE=1,                 // Disable Dreicer generation
    EQTERM_DREICER_MODE_CONNOR_HASTIE_NOCORR=2, // Dreicer based on Connor-Hastie formula (without corrections)
    EQTERM_DREICER_MODE_CONNOR_HASTIE=3,        // Dreicer based on Connor-Hastie formula
    EQTERM_DREICER_MODE_NEURAL_NETWORK=4        // Dreicer using neural network by Hesslow et al
};

enum eqterm_compton_mode {
    EQTERM_COMPTON_MODE_NEGLECT=1,                   // No compton source
    EQTERM_COMPTON_MODE_ITER_DMS=2                   // Use the compton source for ITER suggested by the ITER DMS task force
};
