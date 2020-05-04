#
# This class represents a single ion species (with multiple
# charge states). Each ion species provides a number of settings
# for DREAM:
#   
#   name     -- Label allowing user to identify ion species
#   Z        -- Ion charge number
#   type     -- Method to use for evolving the ion density
#               Is either 'prescribed' (time evolution explicitly
#               given in settings), 'equilibrium' (time evolution
#               governed by enforcing coronal equilibrium), or
#               'dynamic' (solve the ionization rate equation)
#   data
#     t      -- Time grid on which the density is given
#     r      -- Radial grid on which the density is given
#     n      -- Depending on 'type', this is either the prescribed
#               density (size (Z+1) x nt x nr) or the initial
#               density (size (Z+1) x nr), where 'nt' is the number
#               of time points that the density is given, and 'nr'
#               is the number of radial locations.
#

import matplotlib.pyplot as plt
import numpy as np
from Equations.EquationException import EquationException
from Equations.IonSpecies import IonSpecies


# Types in DREAM
IONS_PRESCRIBED = 1
IONS_EQUILIBRIUM = 2
IONS_DYNAMIC = 3

# Types which are extensions implemented in this interface
# (which are special cases of the DREAM types above)
IONS_DYNAMIC_NEUTRAL = -1
IONS_DYNAMIC_FULLY_IONIZED = -2
IONS_PRESCRIBED_NEUTRAL = -3
IONS_PRESCRIBED_FULLY_IONIZED = -4


class IonSpecies:
    
    def __init__(self, name, Z, ttype=0, n=None, r=None, t=None, interpr=None, interpt=None):
        """
        Constructor.

        name:  Name by which the ion species will be referred to.
        Z:     Ion charge number.
        ttype: Method to use for evolving ions in time.
        n:     Ion density (can be either a scalar, 1D array or
               2D array, depending on the other input parameters)
        r:     Radial grid on which the input density is defined.
        t:     Time grid on which the input density is defined.
        """
        self.name = name
        self.Z    = Z

        self.n = None
        self.r = None
        self.t = None

        if ttype == IONS_PRESCRIBED:
            self.initialize_prescribed(n=n, r=r, t=t)
        elif ttype == IONS_DYNAMIC:
            self.initialize_dynamic(n=n, r=r)
        elif ttype == IONS_EQUILIBRIUM:
            self.initialize_equilibrium(n=n, r=r)
        
        # TYPES AVAILABLE ONLY IN THIS INTERFACE
        elif ttype == IONS_DYNAMIC_NEUTRAL:
            self.initialize_dynamic_neutral(n=n, r=r, interpr=interpr)
        elif ttype == IONS_DYNAMIC_FULLY_IONIZED:
            self.initialize_dynamic_fully_ionized(n=n, r=r, interpr=interpr)
        elif ttype == IONS_PRESCRIBED_NEUTRAL:
            self.initialize_prescribed_neutral(n=n, r=r, t=t, interpr=interpr, interpt=interpt)
        elif ttype == IONS_PRESCRIBED_FULLY_IONIZED:
            self.initialize_prescribed_fully_ionized(n=n, r=r, t=t, interpr=interpr, interpt=interpt)
        else:
            raise EquationException("ion_species: Unrecognized ion type: {}.".format(ttype))


    def getDensity(self): return self.n


    def getName(self): return self.name


    def getR(self): return self.r


    def getT(self): return self.t


    def getType(self): return self.type


    def getZ(self): return self.Z


    def initialize_prescribed(self, n=None, r=None, t=None):
        """
        Prescribes the evolution for this ion species.
        """
        self.ttype = IONS_PRESCRIBED

        if n is None:
            raise EquationException("ion_species: Input density must not be 'None'.")

        # Convert lists to NumPy arrays
        if type(n) == list:
            n = np.array(n)

        # Scalar (assume density constant in spacetime)
        if type(n) == float or (type(n) == np.ndarray and n.size == 1):
            self.t = np.array([0])
            self.r = np.array([0,1])
            self.n = np.ones((self.Z+1,1,2)) * n
            return

        if r is None:
            raise EquationException("ion_species: Non-scalar density prescribed, but no radial coordinates given.")

        # Radial profile (assume fully ionized)
        if len(n.shape) == 1:
            raise EquationException("ion_species: Prescribed density data has only one dimension.")
        # Radial profiles of charge states
        elif len(n.shape) == 2:
            raise EquationException("ion_species: Prescribed density data has only two dimensions.")
        # Full time evolution of radial profiles of charge states
        elif len(n.shape) == 3:
            if t is None:
                raise EquationException("ion_species: 3D ion density prescribed, but no time coordinates given.")

            if self.Z+1 != n.shape[0] or t.size != n.shape[1] or r.size != n.shape[2]:
                raise EquationException("ion_species: Invalid dimensions of prescribed density: {}x{}x{}. Expected {}x{}x{}"
                    .format(n.shape[0], n.shape[1], n.shape[2], self.Z+1, t.size, r.size)

            self.t = t
            self.r = r
            self.n = n
        else:
            raise EquationException("ion_species: Unrecognized shape of prescribed density: {}.".format(n.shape))


    def initialize_dynamic(self, n=None, r=None):
        """
        Evolve ions according to the ion rate equation in DREAM.
        """
        self.ttype = IONS_DYNAMIC

        if n is None:
            raise EquationException("ion_species: Input density must not be 'None'.")

        # Convert lists to NumPy arrays
        if type(n) == list:
            n = np.array(n)

        # Scalar (assume density constant in spacetime)
        if type(n) == float or (type(n) == np.ndarray and n.size == 1):
            raise EquationException("ion_species: Initial density must be two dimensional (charge states x radius).")

        if r is None:
            raise EquationException("ion_species: Non-scalar initial ion density prescribed, but no radial coordinates given.")

        # Radial profiles for all charge states 
        if len(n.shape) == 2:
            if t is None:
                raise EquationException("ion_species: 3D ion initial ion density prescribed, but no time coordinates given.")

            if self.Z+1 != n.shape[0] or r.size != n.shape[1]:
                raise EquationException("ion_species: Invalid dimensions of initial ion density: {}x{}. Expected {}x{}."
                    .format(n.shape[0], n.shape[1], self.Z+1, r.size)

            self.t = None
            self.r = r
            self.n = n
        else:
            raise EquationException("ion_species: Unrecognized shape of initial density: {}.".format(n.shape))


    def initialize_equilibrium(self, n=None, r=None, interpr=None):
        """
        Evolve ions according to the equilibrium equation in DREAM.
        """
        self.ttype = IONS_EQUILIBRIUM

        if n is None:
            raise EquationException("ion_species: Input density must not be 'None'.")

        # Convert lists to NumPy arrays
        if type(n) == list:
            n = np.array(n)

        # Scalar (assume density constant in spacetime)
        if type(n) == float or (type(n) == np.ndarray and n.size == 1):
            raise EquationException("ion_species: Initial density must be two dimensional (charge states x radius).")

        if r is None:
            raise EquationException("ion_species: Non-scalar initial ion density prescribed, but no radial coordinates given.")

        # Radial profiles for all charge states 
        if len(n.shape) == 2:
            if t is None:
                raise EquationException("ion_species: 3D ion initial ion density prescribed, but no time coordinates given.")

            if self.Z+1 != n.shape[0] or r.size != n.shape[1]:
                raise EquationException("ion_species: Invalid dimensions of initial ion density: {}x{}. Expected {}x{}."
                    .format(n.shape[0], n.shape[1], self.Z+1, r.size)

            self.t = None
            self.r = r
            self.n = n
        else:
            raise EquationException("ion_species: Unrecognized shape of initial density: {}.".format(n.shape))


    def initialize_dynamic_neutral(self, n=None, r=None, interpr=None):
        """
        Evolve the ions dynamically, initializing them all as neutrals.
        """
        self.initialize_dynamic_charge_state(0, n=n, r=r, interpr=interpr)


    def initialize_dynamic_fully_ionized(self, n=None, r=None, interpr=None):
        """
        Evolve the ions dynamically, initializing them all as fully ionized.
        """
        self.initialize_dynamic_charge_state(self.Z, n=n, r=r, interpr=interpr)


    def initialize_dynamic_charge_state(self, Z0, n=None, r=None, interpr=None):
        """
        Evolve the ions dynamically, initializing them all to reside in the specified charge state Z0.
        """
        if Z0 > self.Z or Z0 < 0:
            raise EquationException("ion_species: Invalid charge state specified: {}. Ion has charge Z = {}.".format(Z0, self.Z))

        if n is None:
            raise EquationException("ion_species: Input density must not be 'None'.")

        # Convert lists to NumPy arrays
        if type(n) == list:
            n = np.array(n)

        # Scalar (assume density constant in spacetime)
        if type(n) == float or (type(n) == np.ndarray and n.size == 1):
            r = interpr if interpr is not None else np.array([0,1])
            N = np.ones((self.Z+1,r.size))
            N[Z0,0,:] = n

            self.initialize_dynamic(n=N, t=t, r=r, interpr=interpr)
            return

        if r is None:
            raise EquationException("ion_species: Non-scalar density prescribed, but no radial coordinates given.")

        # Radial profile
        if len(n.shape) == 1:
            if r.size != n.size:
                raise EquationException("ion_species: Invalid dimensions of prescribed density: {}. Expected {}."
                    .format(n.shape[0], r.size))
                
            N = np.zeros((self.Z+1, r.size))
            N[Z0,:] = n
            self.initialize_dynamic(n=n, t=t, r=r, interpr=interpr)
        else:
            raise EquationException("ion_species: Unrecognized shape of prescribed density: {}.".format(n.shape))


    def initialize_prescribed_neutral(self, n=None, r=None, t=None, interpr=None, interpt=None):
        """
        Prescribe the ions to be neutral.
        """
        self.initialize_prescribed_charge_state(0, n=n, r=r, t=t, interpr=interpr, interpt=interpt)


    def initialize_prescribed_fully_ionized(self, n=None, r=None, t=None, interpr=None, interpt=None):
        """
        Prescribe the ions to be fully ionized.
        """
        self.initialize_prescribed_charge_state(self.Z, n=n, r=r, t=t, interpr=interpr, interpt=interpt)


    def initialize_prescribed_charge_state(self, Z0, n=None, r=None, t=None, interpr=None, interpt=None):
        """
        Prescribe the ions to all be situated in the specified charge state Z0.
        """
        if Z0 > self.Z or Z0 < 0:
            raise EquationException("ion_species: Invalid charge state specified: {}. Ion has charge Z = {}.".format(Z0, self.Z))

        if n is None:
            raise EquationException("ion_species: Input density must not be 'None'.")

        # Convert lists to NumPy arrays
        if type(n) == list:
            n = np.array(n)

        # Scalar (assume density constant in spacetime)
        if type(n) == float or (type(n) == np.ndarray and n.size == 1):
            t = interpt if interpt is not None else np.array([0])
            r = interpr if interpr is not None else np.array([0,1])
            N = np.ones((self.Z+1,t.size,r.size))
            N[Z0,0,:] = n

            self.initialize_prescribed(n=N, t=t, r=r, interpr=interpr, interpt=interpt)
            return

        if r is None:
            raise EquationException("ion_species: Non-scalar density prescribed, but no radial coordinates given.")

        # Radial profile
        if len(n.shape) == 1:
            if r.size != n.size:
                raise EquationException("ion_species: Invalid dimensions of prescribed density: {}. Expected {}."
                    .format(n.shape[0], r.size))
                
            t = interpt if interpt is not None else np.array([0])
            n = np.reshape(n, (t.size,r.size))

        # Radial + temporal profile
        if len(n.shape) == 2:
            if t is None:
                raise EquationException("ion_species: 2D ion density prescribed, but no time coordinates given.")

            if t.size != n.shape[0] or r.size != n.shape[1]:
                raise EquationException("ion_species: Invalid dimensions of prescribed density: {}x{}. Expected {}x{}."
                    .format(n.shape[0], n.shape[1], t.size, r.size))

            N = np.zeros((self.Z+1, t.size, r.size))
            N[Z0,:,:] = n

            self.initialize_prescribed(n=n, t=t, r=r, interpr=interpr, interpt=interpt)
        else:
            raise EquationException("ion_species: Unrecognized shape of prescribed density: {}.".format(n.shape))


