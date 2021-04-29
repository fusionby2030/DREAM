#include "DREAM/Equations/Fluid/RadiatedPowerTerm.hpp"


/**
 * Implementation of a class which represents the 
 * radiated power as calculated with rate coefficients
 * from the ADAS database (PLT corresponds to line
 * and PRB to brems and recombination radiated power).
 * The term is of the form n_e * sum_i n_i L_i, summed over all
 * ion species i. In the semi-implicit solver, n_e is the "unknown"
 * evaluated at the next time step and n_i L_i coefficients.
 * We ignore the Jacobian with respect to L_i(n,T) and capture only the
 * n_e and n_i contributions.
 */


using namespace DREAM;


RadiatedPowerTerm::RadiatedPowerTerm(FVM::Grid* g, FVM::UnknownQuantityHandler *u, IonHandler *ionHandler, ADAS *adas, NIST *nist, bool includePRB) 
    : FVM::DiagonalComplexTerm(g,u), includePRB(includePRB) 
{
    SetName("RadiatedPowerTerm");

    this->adas = adas;
    this->nist = nist;
    this->ionHandler = ionHandler;

    this->id_ncold = unknowns->GetUnknownID(OptionConstants::UQTY_N_COLD);
    this->id_Tcold = unknowns->GetUnknownID(OptionConstants::UQTY_T_COLD);
    this->id_ni    = unknowns->GetUnknownID(OptionConstants::UQTY_ION_SPECIES);

    AddUnknownForJacobian(unknowns, id_ncold);
    AddUnknownForJacobian(unknowns, id_ni);
    AddUnknownForJacobian(unknowns, id_Tcold);

    // constants appearing in bremsstrahlung formula
    real_t c = Constants::c;
    this->bremsPrefactor = (32.0/3.0)*Constants::alpha*Constants::r0*Constants::r0*c
        * sqrt(Constants::me*c*c*Constants::ec*2.0/M_PI);
    this->bremsRel1 = 19.0/24.0; // relativistic-maxwellian correction
    this->bremsRel2 = 5.0/(8.0*M_SQRT2)*(44.0-3.0*M_PI*M_PI); // e-e brems correction
}


void RadiatedPowerTerm::SetWeights(){
    len_t NCells = grid->GetNCells();
    len_t nZ = ionHandler->GetNZ();
    const len_t *Zs = ionHandler->GetZs();
    
    real_t *n_cold = unknowns->GetUnknownData(id_ncold);
    real_t *T_cold = unknowns->GetUnknownData(id_Tcold);
    real_t *n_i    = unknowns->GetUnknownData(id_ni);
    
    for (len_t i = 0; i < NCells; i++)
            weights[i] = 0;

    for(len_t iz = 0; iz<nZ; iz++){
        ADASRateInterpolator *PLT_interper = adas->GetPLT(Zs[iz]);
        ADASRateInterpolator *PRB_interper = adas->GetPRB(Zs[iz]);
        ADASRateInterpolator *ACD_interper = adas->GetACD(Zs[iz]);
        ADASRateInterpolator *SCD_interper = adas->GetSCD(Zs[iz]);
        real_t dWi = 0;
        for(len_t Z0 = 0; Z0<=Zs[iz]; Z0++){
            len_t indZ = ionHandler->GetIndex(iz,Z0);
            for (len_t i = 0; i < NCells; i++){
                // Radiated power term
                real_t Li =  PLT_interper->Eval(Z0, n_cold[i], T_cold[i]);
                if (includePRB) 
                    Li += PRB_interper->Eval(Z0, n_cold[i], T_cold[i]);
                real_t Bi = 0;
                // Binding energy rate term
                if(Z0>0 && includePRB) // Recombination gain
                    Bi -= dWi * ACD_interper->Eval(Z0, n_cold[i], T_cold[i]);
                if(Z0<Zs[iz]){         // Ionization loss
                    dWi = Constants::ec * nist->GetIonizationEnergy(Zs[iz],Z0);
                    Bi += dWi * SCD_interper->Eval(Z0, n_cold[i], T_cold[i]);
                }
                weights[i] += n_i[indZ*NCells + i]*(Li+Bi);
            }
        }
    }

    /**
     * If neglecting the recombination radiation, explicitly add the
     * bremsstrahlung loss which is otherwise included in 'PRB'.
     * Using the R J Gould, The Astrophysical Journal 238 (1980)
     * formula with a relativistic correction. The 'bremsPrefactor'
     * is the factor ~1.69e-38 appearing in the NRL formula (implemented in GO).
     * The 'ionTerm' equals sum_ij n_i^(j) Z_0j^2 
     */
    if(!includePRB) 
        for(len_t i=0; i<NCells; i++){
            real_t ionTerm = ionHandler->GetZeff(i)*ionHandler->GetFreeElectronDensityFromQuasiNeutrality(i);
            real_t relativisticCorrection = bremsRel1*ionTerm + bremsRel2*n_cold[i];
            weights[i] += bremsPrefactor*T_cold[i]*(ionTerm + relativisticCorrection*T_cold[i]/Constants::mc2inEV);
        }
}

void RadiatedPowerTerm::SetDiffWeights(len_t derivId, len_t /*indZs*/){
    len_t NCells = grid->GetNCells();
    len_t nZ = ionHandler->GetNZ();
    const len_t *Zs = ionHandler->GetZs();

    real_t *n_cold = unknowns->GetUnknownData(id_ncold);
    real_t *T_cold = unknowns->GetUnknownData(id_Tcold);
    real_t *n_i    = unknowns->GetUnknownData(id_ni);

    if(derivId == id_ni){
        for(len_t iz = 0; iz<nZ; iz++){
            ADASRateInterpolator *PLT_interper = adas->GetPLT(Zs[iz]);
            ADASRateInterpolator *PRB_interper = adas->GetPRB(Zs[iz]);
            ADASRateInterpolator *ACD_interper = adas->GetACD(Zs[iz]);
            ADASRateInterpolator *SCD_interper = adas->GetSCD(Zs[iz]);
            real_t dWi = 0;
            for(len_t Z0 = 0; Z0<=Zs[iz]; Z0++){
                len_t indZ = ionHandler->GetIndex(iz,Z0);
                for (len_t i = 0; i < NCells; i++){
                    real_t Li =  PLT_interper->Eval(Z0, n_cold[i], T_cold[i]);
                    if (includePRB)
                        Li += PRB_interper->Eval(Z0, n_cold[i], T_cold[i]);
                    real_t Bi = 0;
                    if(Z0>0 && includePRB)
                        Bi -= dWi * ACD_interper->Eval(Z0, n_cold[i], T_cold[i]);
                    if(Z0<Zs[iz]){
                        dWi = Constants::ec * nist->GetIonizationEnergy(Zs[iz],Z0);
                        Bi += dWi * SCD_interper->Eval(Z0, n_cold[i], T_cold[i]);
                    }
                    diffWeights[NCells*indZ + i] = Li+Bi;
                }
            }
        }
        if(!includePRB){ //bremsstrahlung contribution
            for(len_t i=0; i<NCells; i++){
                for(len_t iz = 0; iz<nZ; iz++)
                    for(len_t Z0=0; Z0<=Zs[iz]; Z0++){
                        len_t indZ = ionHandler->GetIndex(iz,Z0);
                        real_t dIonTerm = Z0*Z0;
                        real_t dRelativisticCorrection = bremsRel1*dIonTerm;
                        diffWeights[NCells*indZ + i] += bremsPrefactor*T_cold[i]*(dIonTerm + dRelativisticCorrection*T_cold[i]/Constants::mc2inEV);
                    }
            }
        }
    } else if(derivId == id_ncold){
        for(len_t iz = 0; iz<nZ; iz++){
            ADASRateInterpolator *PLT_interper = adas->GetPLT(Zs[iz]);
            ADASRateInterpolator *PRB_interper = adas->GetPRB(Zs[iz]);
            ADASRateInterpolator *ACD_interper = adas->GetACD(Zs[iz]);
            ADASRateInterpolator *SCD_interper = adas->GetSCD(Zs[iz]);
            real_t dWi = 0;
            for(len_t Z0 = 0; Z0<=Zs[iz]; Z0++){
                len_t indZ = ionHandler->GetIndex(iz,Z0);
                for (len_t i = 0; i < NCells; i++){
                    real_t dLi = PLT_interper->Eval_deriv_n(Z0, n_cold[i], T_cold[i]);
                    if (includePRB)
                        dLi += PRB_interper->Eval_deriv_n(Z0, n_cold[i], T_cold[i]);
                    real_t dBi = 0;
                    if(Z0>0 && includePRB)
                        dBi -= dWi * ACD_interper->Eval_deriv_n(Z0, n_cold[i], T_cold[i]);
                    if(Z0<Zs[iz]){
                        dWi = Constants::ec * nist->GetIonizationEnergy(Zs[iz],Z0);
                        dBi += dWi * SCD_interper->Eval_deriv_n(Z0, n_cold[i], T_cold[i]);
                    }
                    diffWeights[i] += n_i[indZ*NCells + i]*(dLi+dBi);
                }
            }
        }
        if(!includePRB) //bremsstrahlung contribution
            for(len_t i=0; i<NCells; i++)
                diffWeights[i] += bremsPrefactor*T_cold[i]*bremsRel2*T_cold[i]/Constants::mc2inEV;
    } else if (derivId == id_Tcold){
        for(len_t iz = 0; iz<nZ; iz++){
            ADASRateInterpolator *PLT_interper = adas->GetPLT(Zs[iz]);
            ADASRateInterpolator *PRB_interper = adas->GetPRB(Zs[iz]);
            ADASRateInterpolator *ACD_interper = adas->GetACD(Zs[iz]);
            ADASRateInterpolator *SCD_interper = adas->GetSCD(Zs[iz]);
            real_t dWi = 0;
            for(len_t Z0 = 0; Z0<=Zs[iz]; Z0++){
                len_t indZ = ionHandler->GetIndex(iz,Z0);
                for (len_t i = 0; i < NCells; i++){
                    real_t dLi = PLT_interper->Eval_deriv_T(Z0, n_cold[i], T_cold[i]);
                    if (includePRB)
                        dLi += PRB_interper->Eval_deriv_T(Z0, n_cold[i], T_cold[i]);
                    real_t dBi = 0;
                    if(Z0>0 && includePRB)
                        dBi -= dWi * ACD_interper->Eval_deriv_T(Z0, n_cold[i], T_cold[i]);
                    if(Z0<Zs[iz]){
                        dWi = Constants::ec * nist->GetIonizationEnergy(Zs[iz],Z0);
                        dBi += dWi * SCD_interper->Eval_deriv_T(Z0, n_cold[i], T_cold[i]);
                    }
                    diffWeights[i] += n_i[indZ*NCells + i]*(dLi+dBi);
                }
            }
        }
        if(!includePRB){ //bremsstrahlung contribution
            for(len_t i=0; i<NCells; i++){
                real_t ionTerm = ionHandler->GetZeff(i)*ionHandler->GetFreeElectronDensityFromQuasiNeutrality(i);
                real_t relativisticCorrection = bremsRel1*ionTerm + bremsRel2*n_cold[i]; 
                diffWeights[i] += bremsPrefactor*(ionTerm + 2.0*relativisticCorrection*T_cold[i]/Constants::mc2inEV);
            }
        }
    }
}

