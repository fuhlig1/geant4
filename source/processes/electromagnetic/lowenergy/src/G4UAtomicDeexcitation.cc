//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                            *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: G4UAtomicDeexcitation.cc,v 1.11 
// GEANT4 tag $Name: geant4-09-04-patch-02 $
//
// -------------------------------------------------------------------
//
// Geant4 Class file
//  
// Authors: Alfonso Mantero (Alfonso.Mantero@ge.infn.it)
//
// Created 22 April 2010 from old G4UAtomicDeexcitation class 
//
// Modified:
// ---------
//  
//
// -------------------------------------------------------------------
//
// Class description:
// Implementation of atomic deexcitation 
//
// -------------------------------------------------------------------

#include "G4UAtomicDeexcitation.hh"
#include "Randomize.hh"
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4AtomicTransitionManager.hh"
#include "G4FluoTransition.hh"
#include "G4Proton.hh"

#include "G4teoCrossSection.hh"
#include "G4empCrossSection.hh"
#include "G4EmCorrections.hh"
#include "G4LossTableManager.hh"
#include "G4Material.hh"
#include "G4AtomicShells.hh"

using namespace std;

G4UAtomicDeexcitation::G4UAtomicDeexcitation():
  G4VAtomDeexcitation("UAtomDeexcitation"),
  minGammaEnergy(DBL_MAX), 
  minElectronEnergy(DBL_MAX)
{
  PIXEshellCS = 0;
  anaPIXEshellCS = new G4teoCrossSection("Analytical");
  emcorr = G4LossTableManager::Instance()->EmCorrections();
}

G4UAtomicDeexcitation::~G4UAtomicDeexcitation()
{
  delete PIXEshellCS;
  delete anaPIXEshellCS;
}

void G4UAtomicDeexcitation::InitialiseForNewRun()
{
  transitionManager = G4AtomicTransitionManager::Instance();

  // initializing PIXE x-section name
  // 
  if (PIXECrossSectionModel() == "" ||
      PIXECrossSectionModel() == "Empirical" ||
      PIXECrossSectionModel() == "empirical") 
    {
      SetPIXECrossSectionModel("Empirical");
    }
  else if (PIXECrossSectionModel() == "ECPSSR_Analytical" ||
	   PIXECrossSectionModel() == "Analytical" || 
	   PIXECrossSectionModel() == "analytical") 
    {
      SetPIXECrossSectionModel("Analytical");
    }
  else 
    {
      G4cout << "### G4UAtomicDeexcitation::InitialiseForNewRun WARNING "
	     << G4endl;
      G4cout << "    PIXE cross section name " << PIXECrossSectionModel()
	     << " is unknown, PIXE is disabled" << G4endl; 
      SetPIXEActive(false);
    }
    
  // Check if old model should be deleted 
  if(PIXEshellCS) 
    {
      if(PIXECrossSectionModel() != PIXEshellCS->GetName()) 
	{
	  delete PIXEshellCS;
          PIXEshellCS = 0;
	}
    }

  // Instantiate empirical model
  if(!PIXEshellCS && PIXECrossSectionModel() == "Empirical")
    {
      PIXEshellCS = new G4empCrossSection("Empirical");
    }
}

void G4UAtomicDeexcitation::InitialiseForExtraAtom(G4int /*Z*/)
{}

const G4AtomicShell* 
G4UAtomicDeexcitation::GetAtomicShell(G4int Z, G4AtomicShellEnumerator shell)
{
  return transitionManager->Shell(Z, size_t(shell));
}

void G4UAtomicDeexcitation::GenerateParticles(
		      std::vector<G4DynamicParticle*>* vectorOfParticles,  
		      const G4AtomicShell* atomicShell, 
		      G4int Z,
		      G4double gammaCut,
		      G4double eCut)
{
  // Defined initial conditions
  G4int givenShellId = atomicShell->ShellId();
  minGammaEnergy = gammaCut;
  minElectronEnergy = eCut;

  // generation secondaries
  G4DynamicParticle* aParticle;
  G4int provShellId = 0;
  G4int counter = 0;
  
  // The aim of this loop is to generate more than one fluorecence photon 
  // from the same ionizing event 
  do
    {
      if (counter == 0) 
	// First call to GenerateParticles(...):
	// givenShellId is given by the process
	{
	  provShellId = SelectTypeOfTransition(Z, givenShellId);
	  
	  if  ( provShellId >0) 
	    {
	      aParticle = GenerateFluorescence(Z,givenShellId,provShellId);  
	    }
	  else if ( provShellId == -1)
	    {
	      aParticle = GenerateAuger(Z, givenShellId);
	    }
	  else
	    {
	      G4Exception("G4UAtomicDeexcitation: starting shell uncorrect: check it");
	    }
	}
      else 
	// Following calls to GenerateParticles(...):
	// newShellId is given by GenerateFluorescence(...)
	{
	  provShellId = SelectTypeOfTransition(Z,newShellId);
	  if  (provShellId >0)
	    {
	      aParticle = GenerateFluorescence(Z,newShellId,provShellId);
	    }
	  else if ( provShellId == -1)
	    {
	      aParticle = GenerateAuger(Z, newShellId);
	    }
	  else
	    {
	      G4Exception("G4UAtomicDeexcitation: starting shell uncorrect: check it");
	    }
	}
      counter++;
      if (aParticle != 0) 
	{
	  vectorOfParticles->push_back(aParticle);
	  //	  G4cout << "FLUO!" << G4endl; //debug
	}
      else {provShellId = -2;}
    }
  
  // Look this in a particular way: only one auger emitted! // ????
  while (provShellId > -2); 
}

G4double 
G4UAtomicDeexcitation::GetShellIonisationCrossSectionPerAtom(
			       const G4ParticleDefinition* pdef, 
			       G4int Z, 
			       G4AtomicShellEnumerator shellEnum,
			       G4double kineticEnergy,
			       const G4Material* mat)
{
  // check atomic number
  G4double xsec = 0.0;
  if(Z > 100) { return xsec; }
  G4int idx = G4int(shellEnum);
  if(idx >= G4AtomicShells::GetNumberOfShells(Z)) { return xsec; }

  // scaling to protons
  G4double mass = proton_mass_c2;
  G4double escaled = kineticEnergy*mass/(pdef->GetPDGMass());
  G4double q2 = 0.0;
  if(mat) {
    q2 = emcorr->EffectiveChargeSquareRatio(pdef,mat,kineticEnergy);
  } else {
    G4double q = pdef->GetPDGCharge()/eplus;
    q2 = q*q;
  }

  if(PIXEshellCS) { xsec = PIXEshellCS->CrossSection(Z,idx,escaled,mass); }
  if(0.0 == xsec) { xsec = anaPIXEshellCS->CrossSection(Z,idx,escaled,mass); }
  xsec *= q2;
  return xsec;
}

void G4UAtomicDeexcitation::SetCutForSecondaryPhotons(G4double cut)
{
  minGammaEnergy = cut;
}

void G4UAtomicDeexcitation::SetCutForAugerElectrons(G4double cut)
{
  minElectronEnergy = cut;
}

G4double 
G4UAtomicDeexcitation::ComputeShellIonisationCrossSectionPerAtom(
                               const G4ParticleDefinition* p, 
			       G4int Z, 
			       G4AtomicShellEnumerator shell,
			       G4double kinE,
			       const G4Material* mat)
{
  return GetShellIonisationCrossSectionPerAtom(p,Z,shell,kinE,mat);
}

G4int G4UAtomicDeexcitation::SelectTypeOfTransition(G4int Z, G4int shellId)
{
  if (shellId <=0 ) {
    G4Exception("G4UAtomicDeexcitation: zero or negative shellId");
    return 0;
  }
  G4bool fluoTransitionFoundFlag = false;
  
  G4int provShellId = -1;
  G4int shellNum = 0;
  G4int maxNumOfShells = transitionManager->NumberOfReachableShells(Z);  
  
  const G4FluoTransition* refShell = transitionManager->ReachableShell(Z,maxNumOfShells-1);

  // This loop gives shellNum the value of the index of shellId
  // in the vector storing the list of the shells reachable through
  // a radiative transition
  if ( shellId <= refShell->FinalShellId())
    {
      while (shellId != transitionManager->ReachableShell(Z,shellNum)->FinalShellId())
	{
	  if(shellNum ==maxNumOfShells-1)
	    {
	      break;
	    }
	  shellNum++;
	}
      G4int transProb = 0; //AM change 29/6/07 was 1
   
      G4double partialProb = G4UniformRand();      
      G4double partSum = 0;
      const G4FluoTransition* aShell = transitionManager->ReachableShell(Z,shellNum);      
      G4int trSize =  (aShell->TransitionProbabilities()).size();
    
      // Loop over the shells wich can provide an electron for a 
      // radiative transition towards shellId:
      // in every loop the partial sum of the first transProb shells
      // is calculated and compared with a random number [0,1].
      // If the partial sum is greater, the shell whose index is transProb
      // is chosen as the starting shell for a radiative transition
      // and its identity is returned
      // Else, terminateded the loop, -1 is returned
      while(transProb < trSize){
	
	 partSum += aShell->TransitionProbability(transProb);

	 if(partialProb <= partSum)
	   {
	     provShellId = aShell->OriginatingShellId(transProb);
	     fluoTransitionFoundFlag = true;

	     break;
	   }
	 transProb++;
      }

      // here provShellId is the right one or is -1.
      // if -1, the control is passed to the Auger generation part of the package 
    }
  else 
    {
      provShellId = -1;
    }
  //G4cout << "FlagTransition= " << provShellId << " ecut(MeV)= " << minElectronEnergy
  //	 << "  gcut(MeV)= " << minGammaEnergy << G4endl;
  return provShellId;
}

G4DynamicParticle* 
G4UAtomicDeexcitation::GenerateFluorescence(G4int Z, G4int shellId,
					    G4int provShellId )
{ 
  //isotropic angular distribution for the outcoming photon
  G4double newcosTh = 1.-2.*G4UniformRand();
  G4double newsinTh = std::sqrt((1.-newcosTh)*(1. + newcosTh));
  G4double newPhi = twopi*G4UniformRand();
  
  G4double xDir = newsinTh*std::sin(newPhi);
  G4double yDir = newsinTh*std::cos(newPhi);
  G4double zDir = newcosTh;
  
  G4ThreeVector newGammaDirection(xDir,yDir,zDir);
  
  G4int shellNum = 0;
  G4int maxNumOfShells = transitionManager->NumberOfReachableShells(Z);
  
  // find the index of the shell named shellId
  while (shellId != transitionManager->
	 ReachableShell(Z,shellNum)->FinalShellId())
    {
      if(shellNum == maxNumOfShells-1)
	{
	  break;
	}
      shellNum++;
    }
  // number of shell from wich an electron can reach shellId
  size_t transitionSize = transitionManager->
    ReachableShell(Z,shellNum)->OriginatingShellIds().size();
  
  size_t index = 0;
  
  // find the index of the shell named provShellId in the vector
  // storing the shells from which shellId can be reached 
  while (provShellId != transitionManager->
	 ReachableShell(Z,shellNum)->OriginatingShellId(index))
    {
      if(index ==  transitionSize-1)
	{
	  break;
	}
      index++;
    }
  // energy of the gamma leaving provShellId for shellId
  G4double transitionEnergy = transitionManager->
    ReachableShell(Z,shellNum)->TransitionEnergy(index);
  
  if (transitionEnergy < minGammaEnergy) return 0;

  // This is the shell where the new vacancy is: it is the same
  // shell where the electron came from
  newShellId = transitionManager->
    ReachableShell(Z,shellNum)->OriginatingShellId(index);
  
  
  G4DynamicParticle* newPart = new G4DynamicParticle(G4Gamma::Gamma(), 
						     newGammaDirection,
						     transitionEnergy);
  return newPart;
}

G4DynamicParticle* G4UAtomicDeexcitation::GenerateAuger(G4int Z, G4int shellId)
{
  if(!IsAugerActive()) { return 0; }

  if (shellId <=0 ) {
    {G4Exception("G4UAtomicDeexcitation: zero or negative shellId");}
  }
  // G4int provShellId = -1;
  G4int maxNumOfShells = transitionManager->NumberOfReachableAugerShells(Z);  
  
  const G4AugerTransition* refAugerTransition = 
        transitionManager->ReachableAugerShell(Z,maxNumOfShells-1);

  // This loop gives to shellNum the value of the index of shellId
  // in the vector storing the list of the vacancies in the variuos shells 
  // that can originate a NON-radiative transition
  
  // ---- MGP ---- Next line commented out to remove compilation warning
  // G4int p = refAugerTransition->FinalShellId();

  G4int shellNum = 0;

  if ( shellId <= refAugerTransition->FinalShellId() ) 
    //"FinalShellId" is final from the point of view of the elctron who makes the transition, 
    // being the Id of the shell in which there is a vacancy
    {
      G4int pippo = transitionManager->ReachableAugerShell(Z,shellNum)->FinalShellId();
      if (shellId  != pippo ) {
	do { 
	  shellNum++;
 	  if(shellNum == maxNumOfShells)
 	    {
 	      //G4Exception("G4UAtomicDeexcitation: No Auger transition found");
	      return 0;
 	    }
	}
 	while (shellId != (transitionManager->ReachableAugerShell(Z,shellNum)->FinalShellId()) ) ;
      }


      // Now we have that shellnum is the shellIndex of the shell named ShellId

      //      G4cout << " the index of the shell is: "<<shellNum<<G4endl;

      // But we have now to select two shells: one for the transition, 
      // and another for the auger emission.

      G4int transitionLoopShellIndex = 0;      
      G4double partSum = 0;
      const G4AugerTransition* anAugerTransition = 
            transitionManager->ReachableAugerShell(Z,shellNum);

      //      G4cout << " corresponding to the ID: "<< anAugerTransition->FinalShellId() << G4endl;


      G4int transitionSize = 
            (anAugerTransition->TransitionOriginatingShellIds())->size();
      while (transitionLoopShellIndex < transitionSize) {

        std::vector<G4int>::const_iterator pos = 
               anAugerTransition->TransitionOriginatingShellIds()->begin();

        G4int transitionLoopShellId = *(pos+transitionLoopShellIndex);
        G4int numberOfPossibleAuger = 
              (anAugerTransition->AugerTransitionProbabilities(transitionLoopShellId))->size();
        G4int augerIndex = 0;
        //      G4int partSum2 = 0;


	if (augerIndex < numberOfPossibleAuger) {
	  
	  do 
	    {
	      G4double thisProb = anAugerTransition->AugerTransitionProbability(augerIndex, 
										transitionLoopShellId);
	      partSum += thisProb;
	      augerIndex++;
	      
	    } while (augerIndex < numberOfPossibleAuger);
		}
        transitionLoopShellIndex++;
      }
      


      // Now we have the entire probability of an auger transition for the vacancy 
      // located in shellNum (index of shellId) 

      // AM *********************** F I X E D **************************** AM
      // Here we duplicate the previous loop, this time looking to the sum of the probabilities 
      // to be under the random number shoot by G4 UniformRdandom. This could have been done in the 
      // previuos loop, while integrating the probabilities. There is a bug that will be fixed 
      // 5 minutes from now: a line:
      // G4int numberOfPossibleAuger = (anAugerTransition->
      // AugerTransitionProbabilities(transitionLoopShellId))->size();
      // to be inserted.
      // AM *********************** F I X E D **************************** AM

      // Remains to get the same result with a single loop.

      // AM *********************** F I X E D **************************** AM
      // Another Bug: in EADL Auger Transition are normalized to all the transitions deriving from 
      // a vacancy in one shell, but not all of these are present in data tables. So if a transition 
      // doesn't occur in the main one a local energy deposition must occur, instead of (like now) 
      // generating the last transition present in EADL data.
      // AM *********************** F I X E D **************************** AM


      G4double totalVacancyAugerProbability = partSum;


      //And now we start to select the right auger transition and emission
      G4int transitionRandomShellIndex = 0;
      G4int transitionRandomShellId = 1;
      G4int augerIndex = 0;
      partSum = 0; 
      G4double partialProb = G4UniformRand();
      // G4int augerOriginatingShellId = 0;
      
      G4int numberOfPossibleAuger = 0;
      
      G4bool foundFlag = false;

      while (transitionRandomShellIndex < transitionSize) {

        std::vector<G4int>::const_iterator pos = 
               anAugerTransition->TransitionOriginatingShellIds()->begin();

        transitionRandomShellId = *(pos+transitionRandomShellIndex);
        
	augerIndex = 0;
	numberOfPossibleAuger = (anAugerTransition-> 
				 AugerTransitionProbabilities(transitionRandomShellId))->size();

        while (augerIndex < numberOfPossibleAuger) {
	  G4double thisProb =anAugerTransition->AugerTransitionProbability(augerIndex, 
									   transitionRandomShellId);

          partSum += thisProb;
          
          if (partSum >= (partialProb*totalVacancyAugerProbability) ) { // was /
	    foundFlag = true;
	    break;
	  }
          augerIndex++;
        }
        if (partSum >= (partialProb*totalVacancyAugerProbability) ) {break;} // was /
        transitionRandomShellIndex++;
      }

      // Now we have the index of the shell from wich comes the auger electron (augerIndex), 
      // and the id of the shell, from which the transition e- come (transitionRandomShellid)
      // If no Transition has been found, 0 is returned.  

      if (!foundFlag) {return 0;} 
      
      // Isotropic angular distribution for the outcoming e-
      G4double newcosTh = 1.-2.*G4UniformRand();
      G4double  newsinTh = std::sqrt(1.-newcosTh*newcosTh);
      G4double newPhi = twopi*G4UniformRand();
      
      G4double xDir =  newsinTh*std::sin(newPhi);
      G4double yDir = newsinTh*std::cos(newPhi);
      G4double zDir = newcosTh;
      
      G4ThreeVector newElectronDirection(xDir,yDir,zDir);
      
      // energy of the auger electron emitted
      
      
      G4double transitionEnergy = anAugerTransition->AugerTransitionEnergy(augerIndex, transitionRandomShellId);
      /*
	G4cout << "AUger TransitionId " << anAugerTransition->FinalShellId() << G4endl;
	G4cout << "augerIndex: " << augerIndex << G4endl;
	G4cout << "transitionShellId: " << transitionRandomShellId << G4endl;
      */
      
      if (transitionEnergy < minElectronEnergy) return 0;

      // This is the shell where the new vacancy is: it is the same
      // shell where the electron came from
      newShellId = transitionRandomShellId;
      
      return new G4DynamicParticle(G4Electron::Electron(), 
				   newElectronDirection,
				   transitionEnergy);
    }
  else 
    {
      //G4Exception("G4UAtomicDeexcitation: no auger transition found");
      return 0;
    }
}