# Base class for fluid (radius + time) quantities
#

import matplotlib.pyplot as plt
import numpy as np

from DREAM.Output.UnknownQuantity import UnknownQuantity


class FluidQuantity(UnknownQuantity):
    

    def __init__(self, name, data, grid, output):
        """
        Constructor.
        """
        super(FluidQuantity, self).__init__(name=name, data=data, grid=grid, output=output)


    def __repr__(self):
        """
        Convert this object to an "official" string.
        """
        s = self.__str__() + "\n"
        s += self.dumps()
        return s


    def __str__(self):
        """
        Convert this object to a string.
        """
        return '({}) Fluid quantity of size NT x NR = {} x {}'.format(self.name, self.data.shape[0], self.data.shape[1])


    def get(self, r=None, t=None):
        """
        Returns the data in the specified time or radial
        point. If neither 'r' nor 't' are given, returns
        the full spatiotemporal evolution of the profile.
        """
        if (r is None) and (t is None):
            return self.data
        elif (r is not None) and (t is None):
            return self.data[:,r]
        elif (r is None) and (t is not None):
            return self.data[t,:]
        else:
            return self.data[t,r]

        
    def plot(self, ax=None, show=None):
        """
        Generate a contour plot of the spatiotemporal evolution
        of this quantity.

        ax:   Matplotlib axes object to use for plotting.
        show: If 'True', shows the plot immediately via a call to
              'matplotlib.pyplot.show()' with 'block=False'. If
              'None', this is interpreted as 'True' if 'ax' is
              also 'None'.

        RETURNS a matplotlib axis object and a colorbar object
        (which may be 'None' if not used).
        """
        genax = ax is None

        if genax:
            ax = plt.axes()

            if show is None:
                show = True
        
        cp = ax.contourf(self.grid.r, self.grid.t, self.data, cmap='GeriMap')
        ax.set_xlabel(r'Radius $r/a$')
        ax.set_ylabel(r'Time $t$')

        cb = None
        if genax:
            cb = plt.colorbar(mappable=cp, ax=ax)

        if show:
            plt.show(block=False)

        return ax, cb


    def plotRadialProfile(self, t=-1, ax=None, show=None):
        """
        Plot the radial profile of this quantity at the specified
        time slice.

        t: Time index to plot.
        ax:   Matplotlib axes object to use for plotting.
        show: If 'True', shows the plot immediately via a call to
              'matplotlib.pyplot.show()' with 'block=False'. If
              'None', this is interpreted as 'True' if 'ax' is
              also 'None'.

        RETURNS a matplotlib axis object.
        """
        if ax is None:
            ax = plt.axes()

            if show is None:
                show = True

        ax.plot(self.grid.r, self.data[t,:])
        ax.set_xlabel(r'Radius $r/a$')
        ax.set_ylabel('{}'.format(self.getTeXName()))

        if show:
            plt.show(block=False)

        return ax


    def plotTimeProfile(self, r=0, ax=None, show=None):
        """
        Plot the temporal profile of this quantity at the specified
        radius.

        r: Radial index to plot evolution for.
        ax:   Matplotlib axes object to use for plotting.
        show: If 'True', shows the plot immediately via a call to
              'matplotlib.pyplot.show()' with 'block=False'. If
              'None', this is interpreted as 'True' if 'ax' is
              also 'None'.

        RETURNS a matplotlib axis object.
        """
        if ax is None:
            ax = plt.axes()

            if show is None:
                show = True

        ax.plot(self.grid.t, self.data[:,r])
        ax.set_xlabel(r'Time $t$')
        ax.set_ylabel('{}'.format(self.getTeXName()))

        if show:
            plt.show(block=False)

        return ax


    def dumps(self, r=None, t=None):
        return self.get(r=r, t=t).__str__()


    def print(self, r=None, t=None):
        """
        Print the data in this quantity.
        """
        print(self.dumps(r,t))
        

