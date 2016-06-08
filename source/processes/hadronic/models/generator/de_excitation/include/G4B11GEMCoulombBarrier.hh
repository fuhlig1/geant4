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
// $Id: G4B11GEMCoulombBarrier.hh,v 1.2 2002/12/12 19:17:01 gunter Exp $
// GEANT4 tag $Name: geant4-05-00 $
//
// Hadronic Process: Nuclear De-excitations
// by V. Lara (Dec 1999)

#ifndef G4B11GEMCoulombBarrier_h
#define G4B11GEMCoulombBarrier_h 1

#include "G4GEMCoulombBarrierHE.hh"
#include "globals.hh"

class G4B11GEMCoulombBarrier : public G4GEMCoulombBarrierHE
{
public:
  G4B11GEMCoulombBarrier() : G4GEMCoulombBarrierHE(11,5) {};
  ~G4B11GEMCoulombBarrier() {};

private:
  G4B11GEMCoulombBarrier(const G4B11GEMCoulombBarrier & right);

  const G4B11GEMCoulombBarrier & operator=(const G4B11GEMCoulombBarrier & right);
  G4bool operator==(const G4B11GEMCoulombBarrier & right) const;
  G4bool operator!=(const G4B11GEMCoulombBarrier & right) const;
  

};

#endif