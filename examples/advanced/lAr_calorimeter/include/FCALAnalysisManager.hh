//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
// $Id: FCALAnalysisManager.hh
// GEANT4 tag $Name:
//
// Author: Alex Howard (a.s.howard@ic.ac.uk)
//
// History:
// -----------
//  16 Jan 2002  Alex Howard   Created
//
// -------------------------------------------------------------------



#ifdef G4ANALYSIS_USE
#ifndef FCALAnalysisManager_h
#define FCALAnalysisManager_h 1

#include "globals.hh"

// Histogramming from AIDA 

#include "AIDA/IAnalysisFactory.h"

#include "AIDA/ITreeFactory.h"
#include "AIDA/ITree.h"

#include "AIDA/IHistogramFactory.h"
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IHistogram3D.h"

#include "AIDA/IPlotterFactory.h"
#include "AIDA/IPlotter.h"

#include "AIDA/ITupleFactory.h"
#include "AIDA/ITuple.h"

#include "AIDA/IManagedObject.h"


class IAnalysisFactory;
class ITree;
class IHistogramFactory;
class ITupleFactory;
class ITuple;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

class FCALAnalysisManager
{
public:
 
  virtual ~FCALAnalysisManager();
  
  //  void book();
  void book();
  
  void finish();
  
  //fill histograms with OutOfWorld
  void NumOutOfWorld(G4double, G4int, G4int);

  //fill histograms with Number of Secondaries
  void Secondaries(G4double, G4int, G4int);
  
  //fill histograms with Energy Deposit
  void Edep(G4double, G4double);

  //method to call to create an instance of this class
  static FCALAnalysisManager* getInstance();
 

private:
  
  //private constructor in order to create a singleton
  FCALAnalysisManager();
 
  static FCALAnalysisManager* instance;
  
  // Quantities for the ntuple

  G4double OutOfWorld; G4int i; G4int j;

  G4double Secondary;
  
  G4double EmEdep; G4double HadEdep;

  AIDA::IAnalysisFactory  *af;
  AIDA::ITree             *tree;
  AIDA::IHistogramFactory *hf;
  AIDA::ITupleFactory     *tpf;
  AIDA::IPlotterFactory   *pf;
  AIDA::IPlotter          *plotter;

};
#endif
#endif


