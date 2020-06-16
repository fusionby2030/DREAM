/**
 * An external boundary condition which represents a physically
 * motivated boundary condition for the poloidal flux psi_p at
 * the plasma boundary r=a, which is related to the total plasma 
 * current I_p.
 */

#include "DREAM/Equations/PoloidalFlux/AmperesLawZeroFluxAtBoundary.hpp"
#include "DREAM/Constants.hpp"


using namespace DREAM::FVM::BC;




/**
 * Constructor.
 */
AmperesLawZeroFluxAtBoundary::AmperesLawZeroFluxAtBoundary(Grid *g, Grid *targetGrid, 
    const Operator *eqn, real_t scaleFactor)
    : BoundaryCondition(g), equation(eqn), targetGrid(targetGrid), scaleFactor(scaleFactor) { }

/**
 * Destructor.
 */
AmperesLawZeroFluxAtBoundary::~AmperesLawZeroFluxAtBoundary(){}

/**
 * Rebuilds the single non-zero matrix element that contributes, which 
 * represents the external flux at the upper r boundary in the 
 * AmperesLawDiffusionTerm induced by the total plasma current I_p.
 */
bool AmperesLawZeroFluxAtBoundary::Rebuild(const real_t, UnknownQuantityHandler*){
    const len_t nr = this->grid->GetNr();    
    RadialGrid *rGrid = grid->GetRadialGrid();
    
    /**
     * dr_f is set to the difference between max r 
     * on distribution grid and max r on r flux grid,
     * where the quantity is assumed to be zero.
     */
    real_t
        dr    = rGrid->GetDr()[nr-1], 
        dr_f  = rGrid->GetR_f(nr) - rGrid->GetR(nr-1), 
        Vp    = this->grid->GetVp(nr-1)[0],
        Vp_fr = this->grid->GetVp_fr(nr)[0],
        Drr = equation->GetDiffusionCoeffRR(nr)[0],
        dd = 0;

    /**
     * Note that the Vp_fr cancels between the two coefficients,
     * but keeping it in for clarity
     */
    real_t diffusionTermCoeff = -(1+dd)*Drr*Vp_fr/(Vp*dr);

    /**
     *  dpsi/dr(r=a) = [psi(a)-psi(rmax)]/(a-rmax) = -psi(r_max)/dr_f.
     */
    real_t dPsiDrCoeff = scaleFactor/dr_f;

    this->coefficient = diffusionTermCoeff * dPsiDrCoeff;

    return true;
}

/**
 * Add flux to jacobian block.
 */
void AmperesLawZeroFluxAtBoundary::AddToJacobianBlock(
    const len_t derivId, const len_t qtyId, Matrix * jac, const real_t* /*x*/
) {
    if (derivId == qtyId)
        this->AddToMatrixElements(jac, nullptr);

    // TODO handle derivatives of coefficients
}

/**
 * Add flux to linearized operator matrix.
 *
 * mat: Matrix to add boundary conditions to.
 * rhs: Right-hand-side vector (not used).
 */
void AmperesLawZeroFluxAtBoundary::AddToMatrixElements(
    Matrix *mat, real_t*
) {
    const len_t nr = grid->GetNr(); 
    const len_t N_target = targetGrid->GetNCells();   
    mat->SetElement(nr-1, N_target-1, coefficient);
}
/**
 * Add flux to function vector.
 *
 * vec: Function vector to add boundary conditions to.
 * f:   Current value of distribution function.
 */
void AmperesLawZeroFluxAtBoundary::AddToVectorElements(
    real_t *vec, const real_t *f
) {
    const len_t nr = this->grid->GetNr();
    const len_t N_target = targetGrid->GetNCells();   

    vec[nr-1] += coefficient * f[N_target-1];
}
