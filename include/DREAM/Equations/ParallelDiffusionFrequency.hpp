#ifndef _DREAM_EQUATIONS_PARALLEL_DIFFUSION_FREQUENCY_HPP
#define _DREAM_EQUATIONS_PARALLEL_DIFFUSION_FREQUENCY_HPP

#include "SlowingDownFrequency.hpp"

namespace DREAM {
    class ParallelDiffusionFrequency : public CollisionQuantity{
    private:
        real_t **nonlinearMat = nullptr;
        real_t *trapzWeights = nullptr;
        real_t *Theta = nullptr;
        bool includeDiffusion;
        SlowingDownFrequency *nuS;
        CoulombLogarithm *lnLambdaEE;
        real_t rescaleFactor(len_t ir, real_t gamma);
        void calculateIsotropicNonlinearOperatorMatrix();
        void GetNonlinearPartialContribution(const real_t* lnLc, real_t *&partQty);
    protected:
        virtual void AllocatePartialQuantities() override;
        void DeallocatePartialQuantities();        
        virtual void RebuildPlasmaDependentTerms() override;
        virtual void RebuildConstantTerms() override;
        virtual void AssembleQuantity(real_t **&collisionQuantity, len_t nr, len_t np1, len_t np2, FVM::fluxGridType) override;

    public:
    
        ParallelDiffusionFrequency(FVM::Grid *g, FVM::UnknownQuantityHandler *u, IonHandler *ih,
            SlowingDownFrequency *nuS, CoulombLogarithm *lnLee,
                enum OptionConstants::momentumgrid_type mgtype,  struct collqty_settings *cqset);
        ~ParallelDiffusionFrequency();
        virtual real_t evaluateAtP(len_t ir, real_t p) override;
        virtual real_t evaluateAtP(len_t ir, real_t p, struct collqty_settings *inSettings) override; 

        void AddNonlinearContribution();


    };
}

#endif/*_DREAM_EQUATIONS_PARALLEL_DIFFUSION_FREQUENCY_HPP*/


