# Special implementation for 'E_field'

import numpy as np

from . FluidQuantity import FluidQuantity
from . OutputException import OutputException


class ElectricField(FluidQuantity):
    

    def __init__(self, name, data, grid, output, attr=list()):
        """
        Constructor.
        """
        super().__init__(name=name, data=data, attr=attr, grid=grid, output=output)

    
    def getNormEfield(self, field, r=None, t=None):
        """
        Returns an electric field from the other quantities by name.
        This routine is intended as a uniform interface for fetching
        quantities such as Ec, Eceff, ED etc.
        """
        # List of supported quantities (to avoid user error)
        nrm = ['Eceff', 'Ecfree', 'Ectot', 'Ec', 'ED', 'EDreic']
        if field == 'Ec': field = 'Ectot'
        elif field == 'ED': field = 'EDreic'

        if 'fluid' not in self.output.other:
            raise OutputException('No "other" fluid quantities saved in output. Normalizing electric fields are thus not available.')
        if field not in nrm:
            raise OutputException("Cannot normalize to '{}': This seems to not make sense.".format(field))
        if field not in self.output.other.fluid:
            raise OutputException("Cannot normalize to '{}': quantity not saved to output after simulation.".format(field))

        return self.output.other.fluid[field].get(r=r, t=t)


    def norm(self, to='Ec'):
        """
        Return the value of this quantity normalized to the
        quantity specified by 'to'.

        to: Name of quantity to normalize electric field to.
            Possible values:
               Eceff   Effective critical electric field (as defined by Hesslow et al)
               Ecfree  Connor-Hastie threshold field (calculated with n=n_free)
               Ectot   Connor-Hastie threshold field (calculated with n=n_tot)
               Ec      (alias for 'Ectot')
               ED      Dreicer field
            Note that the quantity with which to normalize to
            must be saved as an 'OtherQuantity'.
        """
        return self.normalize(to=to)


    def normalize(self, to='Ec'):
        """
        Return the value of this quantity normalized to the
        quantity specified by 'to'.

        to: Name of quantity to normalize electric field to.
            Possible values:
               Eceff   Effective critical electric field (as defined by Hesslow et al)
               Ecfree  Connor-Hastie threshold field (calculated with n=n_free)
               Ectot   Connor-Hastie threshold field (calculated with n=n_tot)
               Ec      (alias for 'Ectot')
               EDreic  Dreicer field
               ED      (alias for 'EDreic')
            Note that the quantity with which to normalize to
            must be saved as an 'OtherQuantity'.
        """
        # OtherQuantity's are not defined at t=0, so we extend them
        # arbitrarily here (in order for the resulting FluidQuantity to
        # be plottable on the regular time grid)
        Enorm = np.zeros(self.data.shape)
        Enorm[1:,:] = self.getNormEfield(field=to)
        Enorm[0,:]  = Enorm[1,:]

        data = self.data[:] / Enorm
        return FluidQuantity(name='E / {}'.format(to), data=data, grid=self.grid, output=self.output)


    def plot(self, norm=None, **kwargs):
        """
        Wrapper for FluidQuantity.plot(), adding the 'norm' argument
        (for directly plotting a normalized electric field)
        """
        if norm is None:
            return super().plot(**kwargs)
        else:
            _E = self.normalize(to=norm)
            return _E.plot(**kwargs)


    def plotRadialProfile(self, norm=None, **kwargs):
        """
        Wrapper for FluidQuantity.plotRadialProfile(), adding the 'norm'
        argument (for directly plotting a normalized electric field)
        """
        if norm is None:
            return super().plotRadialProfile(**kwargs)
        else:
            _E = self.normalize(to=norm)
            return _E.plotRadialProfile(**kwargs)


    def plotTimeProfile(self, norm=None, **kwargs):
        """
        Wrapper for FluidQuantity.plotTimeProfile(), adding the 'norm'
        argument (for directly plotting a normalized electric field)
        """
        if norm is None:
            return super().plotTimeProfile(**kwargs)
        else:
            _E = self.normalize(to=norm)
            return _E.plotTimeProfile(**kwargs)


    def plotIntegral(self, norm=None, **kwargs):
        """
        Wrapper for FluidQuantity.plotTimeProfile(), adding the 'norm'
        argument (for directly plotting a normalized electric field)
        """
        if norm is None:
            return super().plotIntegral(**kwargs)
        else:
            _E = self.normalize(to=norm)
            return _E.plotIntegral(**kwargs)


