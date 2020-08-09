#ifndef _DREAM_TIME_STEPPER_ADAPTIVE_HPP
#define _DREAM_TIME_STEPPER_ADAPTIVE_HPP

#include <vector>
#include "DREAM/ConvergenceChecker.hpp"
#include "DREAM/TimeStepper/TimeStepper.hpp"

namespace DREAM {
    class TimeStepperAdaptive : public TimeStepper {
    private:
        real_t tMax, dt;
        real_t currentTime=0, initTime=0;

        // List of non-trivial unknowns
        std::vector<len_t> nontrivials;
        // Object used to evaluate norms of solution vectors
        ConvergenceChecker *convChecker;
        // Number of steps to take after checking for convergence
        len_t checkEvery = 0;
        // Number of time steps taken since last tolerance check
        len_t stepsSinceCheck;

        // If true, generates excessive output to stdout
        bool verbose = false;

        // Flags for keeping track of where in the adaptive
        // stepping we are
        enum ts_stage {
            STAGE_NORMAL,        // Normal time stepping
            STAGE_FIRST_HALF,    // Taking first half-step
            STAGE_SECOND_HALF,   // Taking second half-step
            STAGE_FULL           // Taking full step (for later error comparison)
        };

        ts_stage currentStage = STAGE_NORMAL;

        // Total length of progress bar (including percentage)
        const len_t PROGRESSBAR_LENGTH = 80;

        len_t sol_size=0, sol_init_size=0;
        // Solution obtained by taking two half steps, each of size dt/2
        real_t *sol_half=nullptr;
        // Initial solution (before taking the first half step)
        real_t *sol_init=nullptr;
        // Solution obtained by taking a single step of size dt
        real_t *sol_full=nullptr;

        // Maximum error in previous time step
        real_t oldMaxErr = 1;

        // PRIVATE METHODS
        ts_stage AdvanceStage();
        void AllocateSolutionFull(const len_t);
        void AllocateSolutions(const len_t);
        void CopySolution(real_t**);
        void CopySolutionFull(real_t**);
        void DeallocateSolutionFull();
        void DeallocateSolutions();
        void RestoreInitialSolution();
        bool ShouldCheckError();
        bool UpdateStep();

        const char *GetStageName(ts_stage);

    public:
        TimeStepperAdaptive(
            const real_t tMax, const real_t dt0, FVM::UnknownQuantityHandler*,
            std::vector<len_t>&, const real_t reltol=1e-6, int_t checkEvery=0,
            bool verbose=false
        );
        ~TimeStepperAdaptive();

        virtual real_t CurrentTime() const override;
        virtual bool IsFinished() override;
        virtual bool IsSaveStep() override;
        virtual real_t NextTime() override;

        virtual void PrintProgress() override;
    };
}

#endif/*_DREAM_TIME_STEPPER_ADAPTIVE_HPP*/
