/**
 * Implementation of the radial grid.
 */

#include <algorithm>
#include <vector>
#include "FVM/Grid/RadialGrid.hpp"
#include "FVM/Grid/RadialGridGenerator.hpp"


using namespace std;
using namespace TQS::FVM;

/***********************
 * Constructors        *
 ***********************/
/**
 * Initialize an empty grid by only specifying the
 * grid size.
 *
 * rg: Object to use for (re-)generating the radial grid.
 * t0: Time to initialize grid at.
 */
RadialGrid::RadialGrid(RadialGridGenerator *rg, const real_t t0)
    : nr(rg->GetNr()), generator(rg) {

    this->momentumGrids = new MomentumGrid*[rg->GetNr()];

    // Build radial grid for the first time using
    // the given RadialGridGenerator
    rg->Rebuild(t0, this);
}

/**
 * Initialize the grid by specifying the number of
 * radial grid points and the momentum grid to use
 * in all radial grid points (same in all).
 *
 * rg: Object to use for (re-)generating the radial grid.
 * m:  Momentum grid to use in all radial grid points.
 * t0: Time to initialize grid at.
 */
RadialGrid::RadialGrid(RadialGridGenerator *rg, MomentumGrid *m, const real_t t0)
    : RadialGrid(rg, t0) {
    
    this->SetAllMomentumGrids(m);
}

/**
 * Destructor.
 */
RadialGrid::~RadialGrid() {
    // Destroy momentum grids
    //   Since several, or even all, radii may share
    //   a single momentum grid, we should be careful
    //   not to try to double-free any momentum grid.
    vector<MomentumGrid*> deletedPtrs(this->nr);
    for (len_t i = 0; i < this->nr; i++) {
        MomentumGrid *p = this->momentumGrids[i];

        // Has the MomentumGrid been deleted already?
        if (find(deletedPtrs.begin(), deletedPtrs.end(), p) != deletedPtrs.end())
            continue;

        deletedPtrs.push_back(p);
        delete p;
    }

    // Delete radial grid quantities as usual
    delete [] this->avGradr2_R2;
    delete [] this->avGradr2;
    delete [] this->volumes;
    delete [] this->dr_f;
    delete [] this->dr;
    delete [] this->r_f;
    delete [] this->r;

    delete [] this->momentumGrids;
    delete this->generator;
}

/***************************
 * PUBLIC METHODS          *
 ***************************/
/**
 * Get the total number of cells on this grid,
 * including on the momentum grids at each radius.
 */
len_t RadialGrid::GetNCells() const {
    len_t Nr = this->GetNr();
    len_t Nm = 0;

    for (len_t i = 0; i < Nr; i++)
        Nm += this->momentumGrids[i]->GetNCells();

    return (Nr*Nm);
}

/**
 * Rebuilds any non-static (i.e. time dependent) grids
 * used. This can be used if, for example, a dynamically
 * evolving magnetic equilibrium is used, or if some
 * grids are adaptive.
 *
 * t: Time to which re-build the grids for.
 */
bool RadialGrid::Rebuild(const real_t t) {
    bool rgridUpdated = false, updated = false;

    // Re-build radial grid
    if (this->generator->NeedsRebuild(t))
        rgridUpdated = this->generator->Rebuild(t, this);

    updated = rgridUpdated;

    // Re-build momentum grids
    for (len_t i = 0; i < this->nr; i++) {
        if (this->momentumGrids[i]->NeedsRebuild(t, rgridUpdated))
            updated |= this->momentumGrids[i]->Rebuild(t, i, this);
    }

    return updated;
}

/**
 * Set the momentum grid for the specified radius.
 *
 * i: Index of radial coordinate to set momentum grid for.
 * m: Momentum grid to set.
 */
void RadialGrid::SetMomentumGrid(const len_t i, MomentumGrid *m, const real_t t0) {
    this->momentumGrids[i] = m;
    m->Rebuild(t0, i, this);
}

/**
 * Set all momentum grids to be the same object.
 *
 * m: Pointer to momentum grid to use at all radii.
 */
void RadialGrid::SetAllMomentumGrids(MomentumGrid *m, const real_t t0) {
    for (len_t i = 0; i < this->nr; i++)
        this->momentumGrids[i] = m;

    m->Rebuild(t0, 0, this);
}

