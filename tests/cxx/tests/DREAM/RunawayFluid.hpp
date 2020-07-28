#ifndef _DREAMTESTS_DREAM_RUNAWAY_FLUID_HPP
#define _DREAMTESTS_DREAM_RUNAWAY_FLUID_HPP

#include <string>
#include "FVM/Grid/Grid.hpp"
#include "FVM/UnknownQuantityHandler.hpp"
#include "DREAM/IonHandler.hpp"
#include "DREAM/Equations/ConnorHastie.hpp"
#include "DREAM/Equations/RunawayFluid.hpp"
#include "DREAM/Settings/OptionConstants.hpp"
#include "UnitTest.hpp"

namespace DREAMTESTS::_DREAM {
    class RunawayFluid : public UnitTest {
    private:
        len_t id_ions;

    public:
        RunawayFluid(const std::string& s) : UnitTest(s) {}

        DREAM::IonHandler *GetIonHandler(DREAM::FVM::Grid*, DREAM::FVM::UnknownQuantityHandler*, const len_t, const len_t*);
        DREAM::FVM::UnknownQuantityHandler *GetUnknownHandler(DREAM::FVM::Grid*,
            const len_t, const len_t*, const real_t, const real_t);
        DREAM::RunawayFluid *GetRunawayFluid(
            DREAM::CollisionQuantity::collqty_settings *cq, const len_t,
            const len_t*, const real_t, const real_t, const real_t,
            const len_t nr, enum DREAM::OptionConstants::eqterm_dreicer_mode dm=DREAM::OptionConstants::EQTERM_DREICER_MODE_NONE
        );
        bool CompareEceffWithTabulated();
        bool CompareGammaAvaWithTabulated();
        bool CompareConnorHastieRateWithTabulated();

        real_t _ConnorHastieFormula(
            const real_t, const real_t, const real_t,
            const real_t, const real_t,
            bool
        );

//        bool CheckConservativity();
        virtual bool Run(bool) override;
    };
}

#endif/*_DREAMTESTS_DREAM_RUNAWAY_FLUID_HPP*/
