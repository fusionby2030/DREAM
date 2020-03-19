/**
 * Implementation of a general advection term.
 */

#include "FVM/Equation/AdvectionTerm.hpp"
#include "FVM/Grid/RadialGrid.hpp"


using namespace TQS::FVM;

/**
 * Constructor.
 */
AdvectionTerm::AdvectionTerm(RadialGrid *rg)
    : EquationTerm(rg) {
    
    this->AllocateCoefficients();
}

/**
 * Destructor.
 */
AdvectionTerm::~AdvectionTerm() {
    if (!this->coefficientsShared)
        DeallocateCoefficients();
}

/**
 * Allocate new memory for the advection coefficients
 * based on the current grid sizes.
 */
void AdvectionTerm::AllocateCoefficients() {
    if (!this->coefficientsShared)
        DeallocateCoefficients();

    this->nr = this->grid->GetNr();

    this->n1 = new len_t[nr];
    this->n2 = new len_t[nr];

    this->fr = new real_t*[nr];
    this->f1 = new real_t*[nr];
    this->f2 = new real_t*[nr];

    for (len_t i = 0; i < nr; i++) {
        len_t N = this->grid->GetMomentumGrid(i)->GetNCells();

        this->n1[i] = this->grid->GetMomentumGrid(i)->GetNp1();
        this->n2[i] = this->grid->GetMomentumGrid(i)->GetNp2();

        this->fr[i] = new real_t[N];
        this->f1[i] = new real_t[N];
        this->f2[i] = new real_t[N];
    }

    this->coefficientsShared = false;
}

/**
 * Deallocates the memory used by the advection coefficients.
 */
void AdvectionTerm::DeallocateCoefficients() {
    if (f2 != nullptr) {
        for (len_t i = 0; i < grid->GetNr(); i++)
            delete [] f2[i];

        delete [] f2;
    }
    if (f1 != nullptr) {
        for (len_t i = 0; i < grid->GetNr(); i++)
            delete [] f1[i];

        delete [] f1;
    }
    if (fr != nullptr) {
        for (len_t i = 0; i < grid->GetNr(); i++)
            delete [] fr[i];

        delete [] fr;
    }

    if (n2 != nullptr)
        delete [] n2;
    if (n1 != nullptr)
        delete [] n1;
}

/**
 * Assign the memory regions to store the coefficients
 * of this term. This means that we will assume that the
 * memory region is 'shared', and will leave it to someone
 * else to 'free' the memory later on.
 *
 * fr: List of radial advection coefficients.
 * f1: List of first momentum advection coefficients.
 * f2: List of second momentum advection coefficients.
 */
void AdvectionTerm::SetCoefficients(real_t **fr, real_t **f1, real_t **f2) {
    this->fr = fr;
    this->f1 = f1;
    this->f2 = f2;

    this->coefficientsShared = true;
}

/**
 * This function is called whenever the momentum grid is
 * re-built, in case the grid has been re-sized (in which case we might
 * need to re-allocate memory for the advection coefficients)
 */
bool AdvectionTerm::GridRebuilt() {
    this->EquationTerm::GridRebuilt();

    // Do not re-build if our coefficients are owned by someone else
    if (this->coefficientsShared)
        return false;

    this->AllocateCoefficients();
    
    return true;
}

/**
 * Build the matrix elements for this operator.
 *
 * mat: Matrix to build elements of.
 */
void AdvectionTerm::SetMatrixElements(Matrix *mat) {

    const len_t nr = grid->GetNr();
    len_t offset = 0;
    len_t np1np2_prev = grid->GetMomentumGrid(0)->GetNCells();

    // Iterate over interior radial grid points
    for (len_t ir = 0; ir < nr; ir++) {
        const MomentumGrid *mg = grid->GetMomentumGrid(ir);

        const len_t
            np1 = mg->GetNp1(),
            np2 = mg->GetNp2();

        const real_t
            *h2_f1 = mg->GetH2_f1(),
            *h3_f1 = mg->GetH3_f1(),
            *h1_f2 = mg->GetH1_f2(),
            *h3_f2 = mg->GetH3_f2(),
            *dp1   = mg->GetDp1(),
            *dp2   = mg->GetDp2();

        const real_t
            *F1 = f1[ir],
            *F2 = f2[ir];

        for (len_t j = 0; j < np2; j++) {
            // Evaluate flux in first point
            real_t S1 = F1[j*np1 + 1] * h2_f1[j*np1 + 1] * h3_f1[j*np1 + 1] / dp1[1];

            len_t idx  = j*np1 + 1;
            for (len_t i = 0; i < np1; i++) {

                /////////////////////////
                // RADIUS
                /////////////////////////
                #define f(I,V) mat->SetElement(offset + idx, (I) + idx, (V))

                if (ir > 0 && ir < nr-1) {
                    // Phi^(r)_{ir-1/2,i,j}
                    
                    // Phi^(r)_{ir+1/2,i,j}
                    
                    // TODO
                    // This term is pretty difficult, since we need to evaluate the
                    // coefficients in different radial grid points, which in general
                    // have different momentum grids and thus require interpolation
                    // across grids. For this application, we should be able to assume
                    // that momentum grids at all radii use the same coordinate systems.
                    // In general, it is much more difficult, though (so it would require
                    // a bit more thinking if we wanted to interpolate generally between
                    // two different momentum grids)
                }
                
                #undef f
                
                /////////////////////////
                // MOMENTUM 1
                /////////////////////////
                #define f(I,J,V) mat->SetElement(offset + idx, offset + ((J)*np1) + (I), (V))

                // Phi^(1)_{i-1/2,j}
                if (i > 0) {
                    f(i-1, j, S1 * (1-delta1[ir][idx]));
                    f(i,   j, S1 * delta1[ir][idx]);

                    idx += 1;
                    S1 = F1[idx] * h2_f1[idx] * h3_f1[idx] / dp1[i];
                }

                // Phi^(1)_{i+1/2,j}
                if (i < np1-1) {
                    f(i,   j, S1 * (1-delta1[ir][idx]));
                    f(i+1, j, S1 * delta1[ir][idx]);
                }

                /////////////////////////
                // MOMENTUM 2
                /////////////////////////
                // Phi^(2)_{i,j-1/2}
                if (j > 0) {
                    real_t S2m = F2[idx] * h1_f2[idx] * h3_f2[idx] / dp2[j];
                    f(i, j-1, S2m * (1-delta2[ir][idx]));
                    f(i, j,   S2m * delta2[ir][idx]);
                }

                // Phi^(2)_{i,j+1/2}
                if (j < np2-1) {
                    real_t S2p = F2[idx+np1] * h1_f2[idx+np1] * h3_f2[idx+np1] / dp2[j+1];
                    f(i, j,   S2p * (1-delta2[ir][idx+np1]));
                    f(i, j+1, S2p * delta2[ir][idx+np1]);
                }

                #undef f
            }
        }

        offset += np1*np2;
    }
}

