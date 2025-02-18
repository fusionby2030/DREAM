#ifndef _DREAM_FVM_GRID_CYLINDRICAL_RADIAL_GRID_GENERATOR_HPP
#define _DREAM_FVM_GRID_CYLINDRICAL_RADIAL_GRID_GENERATOR_HPP

//#include "FVM/Grid/Grid.hpp"
//#include "FVM/Grid/MomentumGrid.hpp"
#include "FVM/Grid/RadialGrid.hpp"
#include "FVM/Grid/RadialGridGenerator.hpp"
#include <functional>

namespace DREAM::FVM {
    class CylindricalRadialGridGenerator : public RadialGridGenerator {
    private:
        real_t xMin=0, xMax=1;
        real_t B0=0;
        real_t *xf_provided=nullptr;
        real_t *x, *x_f=nullptr;
		len_t ntheta_out;

        // Set to true when the grid is constructed for the first time
        bool isBuilt = false;

    public:
        CylindricalRadialGridGenerator(const len_t nx, const real_t B0, const real_t x0=0, const real_t xa=1, const len_t ntheta_out=120);
        CylindricalRadialGridGenerator(const real_t *x_f, const len_t nx, const real_t B0, const len_t ntheta_out=120);

        virtual bool NeedsRebuild(const real_t) const override { return (!isBuilt); }
        virtual bool Rebuild(const real_t, RadialGrid*) override;

        virtual void GetRThetaPhiFromCartesian(real_t*, real_t*, real_t*, real_t, real_t, real_t, real_t, real_t) override;
        virtual void GetGradRCartesian(real_t*, real_t, real_t, real_t);
        virtual real_t FindClosestApproach(real_t, real_t, real_t, real_t, real_t, real_t) override;

        virtual real_t JacobianAtTheta(const len_t ir, const real_t) override
            {return x[ir];}
        virtual real_t ROverR0AtTheta(const len_t, const real_t) override 
            {return 1.0;}
        virtual real_t NablaR2AtTheta(const len_t, const real_t) override 
            {return 1.0;}
        virtual real_t JacobianAtTheta_f(const len_t ir, const real_t) override
            {return x_f[ir];}
        virtual real_t ROverR0AtTheta_f(const len_t, const real_t) override 
            {return 1.0;}
        virtual real_t NablaR2AtTheta_f(const len_t, const real_t) override 
            {return 1.0;}
        virtual void EvaluateGeometricQuantities(const len_t ir, const real_t, real_t &B, real_t &Jacobian, real_t &ROverR0, real_t &NablaR2) override
            {Jacobian=x[ir]; B=B0; NablaR2 = 1; ROverR0 = 1;}
        virtual void EvaluateGeometricQuantities_fr(const len_t ir, const real_t, real_t &B, real_t &Jacobian, real_t &ROverR0, real_t &NablaR2) override
            {Jacobian=x_f[ir]; B=B0; NablaR2 = 1; ROverR0 = 1;}

		virtual const real_t GetZ0() override { return 0; }
		virtual const len_t GetNPsi() override { return this->GetNr(); }
		virtual const len_t GetNTheta() override { return this->ntheta_out; }
		virtual const real_t *GetFluxSurfaceRMinusR0() override;
		virtual const real_t *GetFluxSurfaceRMinusR0_f() override;
		virtual const real_t *GetFluxSurfaceZMinusZ0() override;
		virtual const real_t *GetFluxSurfaceZMinusZ0_f() override;
		virtual const real_t *GetPoloidalAngle() override;
    };
}

#endif/*_DREAM_FVM_GRID_CYLINDRICAL_RADIAL_GRID_GENERATOR_HPP*/
