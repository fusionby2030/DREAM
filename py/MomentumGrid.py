# Settings object for the hot-tail grid.

import numpy as np
from DREAMException import DREAMException


from PGrid import PGrid
from XiGrid import XiGrid


MOMENTUMGRID_TYPE_PXI = 1
MOMENTUMGRID_TYPE_PPARPPERP = 2


class MomentumGrid:

    
    def __init__(self, name, enabled=True, ttype=1, np=100, nxi=1, pmax=None):
        """
        Constructor.

        name:    Name, indicating what type of grid this is (hot-tail or runaway).
        enabled: If 'True', enables the hot-tail grid in the simulation.
        ttype:   Type of momentum grid (p/xi or ppar/pperp).
        np:      Number of momentum grid points.
        nxi:     Number of pitch grid points.
        pmax:    Maximum momentum on grid.
        """
        self.name = name

        self.set(enabled=enabled, ttype=ttype, np=np, nxi=nxi, pmax=pmax)


    def set(self, enabled=True, ttype=1, np=100, nxi=1, pmax=None):
        """
        Set all settings for this hot-tail grid.
        """
        self.enabled = enabled

        self.type = ttype
        if self.type == MOMENTUMGRID_TYPE_PXI:
            self.pgrid = PGrid(self.name, np=np, pmax=pmax)
            self.xigrid = XiGrid(self.name, nxi=nxi)
        elif self.type == MOMENTUMGRID_TYPE_PPARPPERP:
            raise DREAMException("No support implemented yet for 'ppar/pperp' grids.")
        else:
            raise DREAMException("Unrecognized momentum grid type specified: {}.".format(ttype))

    ##################
    # SETTERS
    ##################
    def setEnabled(self, enabled=True):
        self.enabled = (enabled == True)


    def setNp(self, np):
        if np <= 0:
            raise DREAMException("{}: Invalid value assigned to 'np': {}. Must be > 0.".format(self.name, np))
        elif np == 1:
            print("WARNING: {}: np = 1. Consider disabling the hot-tail grid altogether.".format(self.name))

        self.pgrid.setNp(np)


    def setNxi(self, nxi):
        if nxi <= 0:
            raise DREAMException("{}: Invalid value assigned to 'nxi': {}. Must be > 0.".format(self.name, nxi))

        self.xigrid.setNxi(nxi)


    def setPmax(self, pmax):
        if pmax <= 0:
            raise DREAMException("{}: Invalid value assigned to 'pmax': {}. Must be > 0.".format(self.name, pmax))

        self.pgrid.setPmax(pmax)


    def todict(self, verify=True):
        """
        Returns a Python dictionary containing all settings of
        this MomentumGrid object.
        """
        if verify:
            self.verifySettings()

        data = {
            'enabled': self.enabled,
            'type':    self.type
        }

        if not self.enabled: return data

        if self.type == MOMENTUMGRID_TYPE_PXI:
            data = {**data, **(self.pgrid.todict()), **(self.xigrid.todict())}
        elif self.type == MOMENTUMGRID_TYPE_PPARPPERP:
            raise DREAMException("No support implemented yet for 'ppar/pperp' grids.")
        else:
            raise DREAMException("Unrecognized momentum grid type specified: {}.".format(ttype))

        return data


    def verifySettings(self):
        """
        Verify that all (mandatory) settings are set and consistent.
        """
        if self.type == MOMENTUMGRID_TYPE_PXI:
            if self.enabled:
                self.pgrid.verifySettings()
                self.xigrid.verifySettings()
        elif self.type == MOMENTUMGRID_TYPE_PPARPPERP:
            raise DREAMException("{}: No support implemented yet for 'ppar/pperp' grids.".format(self.name))
        else:
            raise DREAMException("{}: Unrecognized momentum grid type specified: {}.".format(self.name, ttype))


