/**
 * Definition of equations relating to the cold electron temperature.
 */

#include "DREAM/EquationSystem.hpp"
#include "DREAM/NIST.hpp"
#include "DREAM/Settings/OptionConstants.hpp"
#include "DREAM/Settings/Settings.hpp"
#include "DREAM/Settings/SimulationGenerator.hpp"
#include "FVM/Equation/IdentityTerm.hpp"
#include "FVM/Equation/TransientTerm.hpp"
#include "DREAM/Equations/Fluid/CollisionalEnergyTransferKineticTerm.hpp"
#include "DREAM/Equations/Fluid/IonisationHeatingTerm.hpp"
#include "DREAM/Equations/Fluid/MaxwellianCollisionalEnergyTransferTerm.hpp"
#include "DREAM/Equations/Fluid/OhmicHeatingTerm.hpp"
#include "DREAM/Equations/Fluid/RadiatedPowerTerm.hpp"
#include "DREAM/Equations/Fluid/SPIHeatAbsorbtionTerm.hpp"
#include "DREAM/Equations/Fluid/IonSPIIonizLossTerm.hpp"
#include "DREAM/Equations/Fluid/ElectronHeatTerm.hpp"
#include "FVM/Equation/PrescribedParameter.hpp"
#include "FVM/Grid/Grid.hpp"


using namespace DREAM;


#define MODULENAME "eqsys/T_cold"
#define MODULENAME_SPI "eqsys/spi"
#define MODULENAME_ION "eqsys/n_i"


/**
 * Define options for the electron temperature module.
 */
void SimulationGenerator::DefineOptions_T_abl(Settings *s){
    s->DefineSetting(MODULENAME "/type_abl", "Type of equation to use for determining the ablation electron temperature evolution", (int_t)OptionConstants::UQTY_T_ABL_EQN_PRESCRIBED);
    s->DefineSetting(MODULENAME "/recombination_abl", "Whether to include recombination radiation (true) or ionization energy loss (false)", (bool)true);

    // Prescribed data (in radius+time)
    DefineDataRT(MODULENAME, s, "data_abl");

    // Prescribed initial profile (when evolving T self-consistently)
    DefineDataR(MODULENAME, s, "init_abl");
    
    // Transport settings
    DefineOptions_Transport(MODULENAME, s, false);
}


/**
 * Construct the equation for the electric field.
 */
void SimulationGenerator::ConstructEquation_T_abl(
    EquationSystem *eqsys, Settings *s, ADAS *adas, NIST *nist, AMJUEL *amjuel,
    struct OtherQuantityHandler::eqn_terms *oqty_terms
) {
    enum OptionConstants::uqty_T_abl_eqn type = (enum OptionConstants::uqty_T_abl_eqn)s->GetInteger(MODULENAME "/type_abl");

    switch (type) {
        case OptionConstants::UQTY_T_ABL_EQN_PRESCRIBED:
            ConstructEquation_T_abl_prescribed(eqsys, s);
            break;

        default:
            throw SettingsException(
                "Unrecognized equation type for '%s': %d.",
                OptionConstants::UQTY_T_ABL, type
            );
    }
}

/**
 * Construct the equation for a prescribed temperature.
 */
void SimulationGenerator::ConstructEquation_T_abl_prescribed(
    EquationSystem *eqsys, Settings *s
) {
    FVM::Grid *fluidGrid = eqsys->GetFluidGrid();
    FVM::Operator *eqn = new FVM::Operator(fluidGrid);

    FVM::Interpolator1D *interp = LoadDataRT_intp(MODULENAME,fluidGrid->GetRadialGrid(), s, "data_abl");
    eqn->AddTerm(new FVM::PrescribedParameter(fluidGrid, interp));

    eqsys->SetOperator(OptionConstants::UQTY_T_ABL, OptionConstants::UQTY_T_ABL, eqn, "Prescribed");

    // Initialization
    eqsys->initializer->AddRule(
        OptionConstants::UQTY_T_ABL,
        EqsysInitializer::INITRULE_EVAL_EQUATION
    );
    
    //eqsys->SetUnknown(OptionConstants::UQTY_W_COLD, OptionConstants::UQTY_W_COLD_DESC, fluidGrid);
    ConstructEquation_W_abl(eqsys, s);
}


/**
 * Construct the equation for electron energy content:
 *    W_cold = 3n_cold*T_cold/2
*/
void SimulationGenerator::ConstructEquation_W_abl(
    EquationSystem *eqsys, Settings* /*s*/
) {
    FVM::Grid *fluidGrid = eqsys->GetFluidGrid();
    
    FVM::Operator *Op1 = new FVM::Operator(fluidGrid);
    FVM::Operator *Op2 = new FVM::Operator(fluidGrid);

    len_t id_W_cold = eqsys->GetUnknownID(OptionConstants::UQTY_W_ABL);
    len_t id_T_cold = eqsys->GetUnknownID(OptionConstants::UQTY_T_ABL);
    len_t id_n_cold = eqsys->GetUnknownID(OptionConstants::UQTY_N_ABL);
    
    Op1->AddTerm(new FVM::IdentityTerm(fluidGrid,-1.0) );
    Op2->AddTerm(new ElectronHeatTerm(fluidGrid,id_n_cold,eqsys->GetUnknownHandler()) );

    eqsys->SetOperator(id_W_cold, id_W_cold, Op1, "W_cold = (3/2)*n_cold*T_cold");
    eqsys->SetOperator(id_W_cold, id_T_cold, Op2);    

    eqsys->initializer->AddRule(
        id_W_cold,
        EqsysInitializer::INITRULE_EVAL_EQUATION,
        nullptr,
        id_T_cold,
        id_n_cold
    );


}
