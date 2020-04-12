#ifndef _DREAM_EQUATIONS_KINETIC_ENERGY_DIFFUSION_TERM_HPP
#define _DREAM_EQUATIONS_KINETIC_ENERGY_DIFFUSION_TERM_HPP



#include "DREAM/EquationSystem.hpp"
#include "DREAM/Settings/OptionConstants.hpp"
#include "DREAM/Equations/CollisionQuantityHandler.hpp"
#include "FVM/config.h"
#include "FVM/Equation/DiffusionTerm.hpp"
#include "FVM/Grid/Grid.hpp"
#include "FVM/UnknownQuantityHandler.hpp"


namespace DREAM {
    class EnergyDiffusionTerm
        : public FVM::DiffusionTerm {
    private:
        enum OptionConstants::momentumgrid_type gridtype;
        CollisionQuantityHandler *collQty;
        EquationSystem *eqSys;
        FVM::Grid *grid;
    public:
        EnergyDiffusionTerm(FVM::Grid*,CollisionQuantityHandler*,EquationSystem*,enum OptionConstants::momentumgrid_type);
        
        
        virtual void Rebuild(const real_t, const real_t, FVM::UnknownQuantityHandler*) override;
    };
}

#endif/*_DREAM_EQUATIONS_KINETIC_ENERGY_DIFFUSION_TERM_HPP*/


