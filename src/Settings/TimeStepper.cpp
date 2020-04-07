/**
 * Construct a time stepper object.
 */

#include "DREAM/EquationSystem.hpp"
#include "DREAM/Settings/SimulationGenerator.hpp"
#include "DREAM/TimeStepper/TimeStepper.hpp"
#include "DREAM/TimeStepper/TimeStepperConstant.hpp"
#include "FVM/UnknownQuantityHandler.hpp"


using namespace DREAM;
using namespace std;


#define MODULENAME "timestep"


/**
 * Define options for the time stepper.
 * 
 * s: Settings object to define settings in.
 */
void SimulationGenerator::DefineOptions_TimeStepper(Settings *s) {
    s->DefineSetting(MODULENAME "/type", "Time step generator type", (int_t)TIMESTEPPER_TYPE_CONSTANT);
    s->DefineSetting(MODULENAME "/tmax", "Maximum simulation time", (real_t)0.0);
    s->DefineSetting(MODULENAME "/dt", "Length of each time step", (real_t)0.0);
    s->DefineSetting(MODULENAME "/nt", "Number of time steps to take", (int_t)0.0);
}

/**
 * Construct a TimeStepper object according to the settings.
 *
 * eqsys: Equation system object to assign TimeStepper object to.
 * s:     Settings object specifying how to construct the TimeStepper object.
 */
void SimulationGenerator::ConstructTimeStepper(EquationSystem *eqsys, Settings *s) {
    enum timestepper_type type = (enum timestepper_type)s->GetInteger(MODULENAME "/type");

    FVM::UnknownQuantityHandler *u = eqsys->GetUnknownHandler();
    TimeStepper *ts;
    switch (type) {
        case TIMESTEPPER_TYPE_CONSTANT:
            ts = ConstructTimeStepper_constant(s, u);
            break;

        default:
            throw SettingsException(
                "Unrecognized time stepper type: %d.", type
            );
    }

    eqsys->SetTimeStepper(ts);
}


/**
 * Construct a TimeStepperConstant object according to the
 * provided settings.
 *
 * s: Settings object specifying how to construct the
 *    TimeStepperConstant object.
 */
TimeStepperConstant *SimulationGenerator::ConstructTimeStepper_constant(Settings *s, FVM::UnknownQuantityHandler *u) {
    real_t tmax = s->GetReal(MODULENAME "/tmax");
    real_t dt   = s->GetReal(MODULENAME "/dt", false);
    int_t nt    = s->GetInteger(MODULENAME "/nt", false);

    bool dtset  = (dt > 0);
    bool ntset  = (nt > 0);

    if (dtset && ntset)
        throw SettingsException(
            "TimeStepper constant: "
            "Ambiguous time step specified. Only one of 'dt' and 'nt' "
            "may be set for the time stepper."
        );
    else if (!dtset && !ntset)
        throw SettingsException(
            "TimeStepper constant: "
            "No time step specified. Exactly one of 'dt' and 'nt' "
            "must be set for the time stepper."
        );

    // Generate object
    if (dtset) {
        s->MarkUsed(MODULENAME "/dt");
        return new TimeStepperConstant(tmax, dt, u);
    } else {
        s->MarkUsed(MODULENAME "/nt");
        return new TimeStepperConstant(tmax, (len_t)nt, u);
    }
}

