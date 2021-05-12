# Settings for the runaway electron density

import numpy as np
from scipy.special import kn
from scipy import integrate
from . EquationException import EquationException
from . UnknownQuantity import UnknownQuantity
import DREAM.Settings.Equations.IonSpecies as Ions



VELOCITY_MODE_NONE=1
VELOCITY_MODE_PRESCRIBED=2

ABLATION_MODE_NEGLECT=1
ABLATION_MODE_FLUID_NGS=2
ABLATION_MODE_KINETIC_NGS=3

DEPOSITION_MODE_NEGLECT=1
DEPOSITION_MODE_LOCAL=2
DEPOSITION_MODE_LOCAL_LAST_FLUX_TUBE=3
DEPOSITION_MODE_LOCAL_GAUSSIAN=4

HEAT_ABSORBTION_MODE_NEGLECT=1
HEAT_ABSORBTION_MODE_LOCAL_FLUID_NGS=2
HEAT_ABSORBTION_MODE_LOCAL_FLUID_NGS_GAUSSIAN=3

CLOUD_RADIUS_MODE_NEGLECT=1
CLOUD_RADIUS_MODE_PRESCRIBED_CONSTANT=2
CLOUD_RADIUS_MODE_SELFCONSISTENT=3

ZMolarMassList=[1,1,10]
isotopesMolarMassList=[2,0,0]# 0 means naturally occuring mix
molarMassList=[0.0020141,0.001008,0.020183]# kg/mol

ZSolidDensityList=[1,1,10]
isotopesSolidDensityList=[2,0,0]
solidDensityList=[205.9,86,1444]# kg/m^3

N_A=6.02214076e23

class SPI(UnknownQuantity):
    

    def __init__(self, settings, rp=None, vp=None, xp=None , VpVolNormFactor=1, rclPrescribedConstant=0.01, velocity=VELOCITY_MODE_NONE, ablation=ABLATION_MODE_NEGLECT, deposition=DEPOSITION_MODE_NEGLECT, heatAbsorbtion=HEAT_ABSORBTION_MODE_NEGLECT, cloudRadiusMode=CLOUD_RADIUS_MODE_NEGLECT):
        """
        Constructor.
        """
        super().__init__(settings=settings)

        self.velocity           = velocity
        self.ablation           = ablation
        self.deposition         = deposition
        self.heatAbsorbtion     = heatAbsorbtion
        self.cloudRadiusMode    = cloudRadiusMode
        self.VpVolNormFactor    = VpVolNormFactor
        self.rclPrescribedConstant = rclPrescribedConstant

        self.rp       = None
        self.vp       = None
        self.xp       = None
        #self.setInitialData(rp=rp, vp=vp, xp=xp)


    def setInitialData(self, rp, vp, xp):

        if np.isscalar(rp):
            self.rp = np.asarray([rp])
        else: self.rp = np.asarray(rp)

        if np.isscalar(vp):
            self.vp = np.asarray([vp])
        else: self.vp = np.asarray(vp)

        if np.isscalar(xp):
            self.xp = np.asarray([xp])
        else: self.xp = np.asarray(xp)
        
    def rpDistrParksStatistical(self,rp,kp):
        """
        Evaluates the shard size distribution function referred to as the 
        'statistical model' in P. Parks 2016 GA report (DOI:10.2172/1344852)
        """
        return kn(0,rp*kp)*kp**2*rp
        
    def sampleRpDistrParksStatistical(self,N,kp):
        """
        Samples N shard radii according to the distribution function 
        given by rpDistrParksStatistical()
        """
        # First we calculate the cdf, and then interpolate the cdf-values 
        # back to the corresponding radii at N randomly chosen points between 0 and 1
        rp_integrate=np.linspace(1e-10/kp,10/kp,5000)
        cdf=integrate.cumtrapz(y=self.rpDistrParksStatistical(rp_integrate,kp),x=rp_integrate)
        return np.interp(np.random.uniform(size=N),np.hstack((0.0,cdf)),rp_integrate)
        
    def setRpParksStatistical(self, nShard, Ninj, Zs, isotopes, molarFractions, ionNames, n_i, add=True, **kwargs):
        """
        sets (or adds) nShard shards with radii distributed accordin to 
        rpDistrParksStatistical(), with the characteristic inverse shard size kp 
        calculated from the given pellet and shattering parameters. Also updates the ion 
        settings with the appropriate molar fractions contributing to each ion species
        
        nShard: Number of shards into which the pellet is shattered
        Ninj: Numbr of particles contained in the pellet
        Zs: List of charge numbers for every ion species the pellet consists of
        isotopes: List of isotopes for every ion species the pellet consists of
        molarFractions: Molar fraction with which each ion species contribute
        ionNames: List of names for the ion species to be added and connected 
                  to the ablation of this pellet
        n_i: Ion settings object to be updated
        add: If 'True', add the new pellet shards to the existing ones, otherwise 
             existing shards are cleared
             
        RETURNS the inverse characteristic shard size kp
        """
        
        
        # Calculate solid particle density of the pellet (needed to calculate the 
        # inverse characteristic shard size)
        molarVolume=0
        for iZ in range(len(Zs)):
            for iList in range(len(solidDensityList)):
                if Zs[iZ]==ZSolidDensityList[iList] and isotopes[iZ]==isotopesSolidDensityList[iList]:
                    solidDensityIZ=solidDensityList[iList]
                if Zs[iZ]==ZMolarMassList[iList] and isotopes[iZ]==isotopesMolarMassList[iList]:
                    molarMassIZ=molarMassList[iList]
            
            molarVolume+=molarFractions[iZ]*molarMassIZ/solidDensityIZ
            
        solidParticleDensity=N_A/molarVolume
       
       
        # Calculate inverse characteristic shard size
        kp=(6*np.pi**2*solidParticleDensity*nShard/Ninj)**(1/3)
        
        # Sample the shard sizes and rescale to get exactly the 
        # specified number of particles in the pellet
        rp_init=self.sampleRpDistrParksStatistical(nShard,kp)
        Ninj_obtained=np.sum(4*np.pi*rp_init**(3)/3/molarVolume*N_A)
        rp_init*=(Ninj/Ninj_obtained)**(1/3)       
       
        if add and self.rp is not None:
            self.rp=np.concatenate((self.rp,rp_init))
        else:
            self.rp=rp_init
            
        # Add an ion species connected to this pellet to the ion settings
        for iZ in range(len(Zs)):
            
            # SPIMolarFraction must have the smae length as all pellet shard, 
            # not only the pellet which is initiated here, so set the molar fraction 
            # to zero for previously set shards (apparently somethin does not like having
            # SPIMolarFraction at exactly 0, so set it to something very small...)
            SPIMolarFraction=1e-10*np.ones(len(self.rp))
            SPIMolarFraction[-nShard:]=molarFractions[iZ]*np.ones(nShard)
            print(SPIMolarFraction[SPIMolarFraction!=1])
            n_i.addIon(name=ionNames[iZ], n=1e0, Z=Zs[iZ], isotope=isotopes[iZ], iontype=Ions.IONS_DYNAMIC_NEUTRAL, SPIMolarFraction=SPIMolarFraction,**kwargs)

        # Add zeros to the end of SPIMolarFraction for all ion species connected to a pellet
        for ion in n_i.ions:
            SPIMolarFractionPrevious=ion.getSPIMolarFraction()
            if SPIMolarFractionPrevious[0]!=-1:
                ion.setSPIMolarFraction(np.concatenate((SPIMolarFractionPrevious,np.zeros(nShard))))
            
        return kp
        
    def setShardPositionSinglePoint(self, nShard,shatterPoint,add=True):
        """
        Sets self.xp to a vector of the (x,y,z)-coordinates of nShard initial
        pellet shard positions starting from the single point shatterPoint
        
        nShard: Number of shards 
        shatterPoint: (x,y,z)-coordinates for the starting point of the shards to be set
        add: If 'True', add the new pellet shard positions to the existing ones, otherwise 
             existing shards are cleared
        """
        if add and self.xp is not None:
            self.xp=np.concatenate((self.xp,np.tile(shatterPoint,nShard)))
        else:
            self.xp=np.tile(shatterPoint,nShard)
            
    def setShardVelocitiesUniform(self, nShard,abs_vp_mean,abs_vp_diff,alpha_max,nDim=2,add=True):
        """
        Sets self.vp to a vector storing the (x,y,z)-components of nShard shard velosities,
        assuming a uniform velocity distribution over a nDim-dimensional cone whose axis
        is anti-parallell to the x-axis. TODO: implement support for an arbitrary axis?
        
        nShard: Number of shards
        abs_vp_mean: Mean of the magnitude of the shard velocities
        abs_vp_diff: width of the uniform distribution of the magnitude of the shard velocities
        alpha_max: Span of divergence angle (ie twice the opening angle of the cone)
        nDim: number of dimensions into which the shards should be spread
        add: If 'True', add the new pellet shard velocities to the existing ones, otherwise 
             existing shards are cleared
        """
        
        # Sample magnitude of velocities
        abs_vp_init=(abs_vp_mean+abs_vp_diff*(-1+2*np.random.uniform(size=nShard)))
        
        # Sample directions uniformly over a nDim-dimensional cone and set the velocity vectors
        vp_init=np.zeros(3*nShard)
        if nDim==1:
            # in 1D, the "cone" simply becomes a straight line
            vp_init[0::3]=-abs_vp_init
            
        elif nDim==2:
            # in 2D, the cone becomes and arc
            alpha=alpha_max*(-1+2*np.random.uniform(size=nShard))
            vp_init[0::3]=-abs_vp_init*np.cos(alpha)
            vp_init[1::3]=abs_vp_init*np.sin(alpha)
            
        elif nDim==3:
            # The solid angle covered by the part of the cone between alpa and d(alpha) 
            # is proportional to sin(alpha), and the normalised probability distribution 
            # becomes f(alpha)=sin(alpha)/(1-cos(alpha_max/2)). We sample from this
            # distribution by applying the inverse cdf to uniformly drawn numbers
            # between 0 and 1
            alpha=np.arcsin(np.random.uniform(size=nShard)*(1-np.cos(alpha_max/2)))
            
            # The angle in the yz-plane is simply drawn randomly
            phi=2*np.pi*np.random.uniform(size=nShard)
            
            # Finally calculate the velocity vectors
            vp_init[0::3]=-abs_vp_init*np.cos(alpha)
            vp_init[1::3]=abs_vp_init*np.sin(alpha)*np.cos(phi)
            vp_init[2::3]=abs_vp_init*np.sin(alpha)*np.sin(phi)
            
        else:
            raise EquationException("spi: Invalid number of dimensions into which the pellet shards are spread")
            
        if add and self.vp is not None:
            self.vp=np.concatenate((self.vp,vp_init))
        else:
            self.vp=vp_init
            
    def setParamsVallhagenMSc(self, nShard, Ninj, Zs, isotopes, molarFractions, ionNames, n_i, shatterPoint, abs_vp_mean,abs_vp_diff,alpha_max,nDim=2, add=True, **kwargs):
        """
        Wrapper for setRpParksStatistical(), setShardPositionSinglePoint() and setShardVelocitiesUniform(),
        which combined are used to set up an SPI-scenario similar to those in Oskar Vallhagens MSc thesis
        (available at https://ft.nephy.chalmers.se/files/publications/606ddcbc08804.pdf)
        """
        
        kp=self.setRpParksStatistical(nShard, Ninj, Zs, isotopes, molarFractions, ionNames, n_i, add, **kwargs)
        self.setShardPositionSinglePoint(nShard,shatterPoint,add)
        self.setShardVelocitiesUniform(nShard,abs_vp_mean,abs_vp_diff,alpha_max,nDim,add)
        return kp
        
    def setVpVolNormFactor(self,VpVolNormFactor):
        self.VpVolNormFactor=VpVolNormFactor

    def setRclPrescribedConstant(self,rclPrescribedConstant):
        self.rclPrescribedConstant=rclPrescribedConstant


    def setVelocity(self, velocity):
        """
        Specifies mode to calculate shard velocities.
        """
        self.velocity = int(velocity)


    def setAblation(self, ablation):
        """
        Specifies which model to use for calculating the
        ablation rate.
        """
        self.ablation = int(ablation)

    def setDeposition(self, deposition):
        """
        Specifies which model to use for calculating the
        deposition of ablated material.
        """
        self.deposition = int(deposition)

    def setHeatAbsorbtion(self, heatAbsorbtion):
        """
        Specifies which model to use for calculating the
        heat absorbtion in the neutral pellet cloud
        """
        self.heatAbsorbtion = int(heatAbsorbtion)

    def setCloudRadiusMode(self, cloudRadiusMode):
        """
        Specifies which model to use for calculating the
        radius of the the neutral pellet cloud
        """
        self.cloudRadiusMode = int(cloudRadiusMode)


    def fromdict(self, data):
        """
        Set all options from a dictionary.
        """
        self.velocity       = data['velocity']
        self.ablation       = data['ablation']
        self.deposition     = data['deposition']
        self.heatAbsorbtion = data['heatAbsorbtion']
        self.cloudRadiusMode = data['cloudRadiusMode']

        self.VpVolNormFactor = data['VpVolNormFactor']
        self.rclPrescribedConstant = data['rclPrescribedConstant']
        self.rp              = data['init']['rp']
        self.vp              = data['init']['vp']
        self.xp              = data['init']['xp']


    def todict(self):
        """
        Returns a Python dictionary containing all settings of
        this SPI object.
        """
        data = {
            'velocity': self.velocity,
            'ablation': self.ablation,
            'deposition': self.deposition,
            'heatAbsorbtion': self.heatAbsorbtion,
            'cloudRadiusMode': self.cloudRadiusMode,
            'VpVolNormFactor': self.VpVolNormFactor,
            'rclPrescribedConstant': self.rclPrescribedConstant
        }
        data['init'] = {
                'rp': self.rp,
                'vp': self.vp,
                'xp': self.xp
        }

        return data


    def verifySettings(self):
        """
        Verify that the settings of this unknown are correctly set.
        """
        if type(self.velocity) != int:
            raise EquationException("spi: Invalid value assigned to 'velocity'. Expected integer.")
        if type(self.ablation) != int:
            raise EquationException("spi: Invalid value assigned to 'ablation'. Expected integer.")
        if type(self.deposition) != int:
            raise EquationException("spi: Invalid value assigned to 'deposition'. Expected integer.")
        if type(self.heatAbsorbtion) != int:
            raise EquationException("spi: Invalid value assigned to 'heatAbsorbtion'. Expected integer.")



    def verifySettingsPrescribedInitialData(self):
        if vp.size!=3*rp.size:
            raise EquationException("Missmatch in size of initial data arrays for rp and vp. Expected vp to have a size 3 times the size of rp")
        if xp.size!=3*rp.size:
            raise EquationException("Missmatch in size of initial data arrays for rp and xp. Expected xp to have a size 3 times the size of rp")
        
