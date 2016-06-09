//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
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
// The code was written by :
//	^Claudio Andenna claudio.andenna@iss.infn.it, claudio.andenna@ispesl.it
//      *Barbara Caccia barbara.caccia@iss.it
//      with the support of Pablo Cirrone (LNS, INFN Catania Italy)
//
// ^ISPESL and INFN Roma, gruppo collegato Sanità, Italy
// *Istituto Superiore di Sanità and INFN Roma, gruppo collegato Sanità, Italy
//  Viale Regina Elena 299, 00161 Roma (Italy)
//  tel (39) 06 49902246
//  fax (39) 06 49387075
//
// more information:
// http://g4advancedexamples.lngs.infn.it/Examples/medical-linac
//
//*******************************************************//


#include "ML2Ph_FullWater.hh"

CML2Ph_FullWater::CML2Ph_FullWater()
{}

CML2Ph_FullWater::~CML2Ph_FullWater(void)
{
}
bool CML2Ph_FullWater::Construct(G4VPhysicalVolume *PVWorld, G4int saving_in_ROG_Voxels_every_events, G4int seed, G4String ROGOutFile, G4bool bSaveROG)
{
	this->PVWorld=PVWorld;

	bool bCreated=false;
	G4ThreeVector halfSize;
	halfSize.set(300.*mm,300.*mm,300.*mm);
	G4Material *WATER=G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER");
	G4Box *fullWaterPhantomBox = new G4Box("fullWaterPhantomBox", halfSize.getX(), halfSize.getY(), halfSize.getZ());
	G4LogicalVolume *fullWaterPhantomLV = new G4LogicalVolume(fullWaterPhantomBox, WATER, "fullWaterPhantomLV", 0, 0, 0);
	G4VPhysicalVolume *fullWaterPhantomPV=0;
	fullWaterPhantomPV = new G4PVPlacement(0, G4ThreeVector(0.,0.,0.), "fullWaterPhantomPV", fullWaterPhantomLV, this->PVWorld, false, 0);

	// Region for cuts
	G4Region *regVol= new G4Region("fullWaterPhantomR");
	G4ProductionCuts* cuts = new G4ProductionCuts;
	cuts->SetProductionCut(0.1*mm);
	regVol->SetProductionCuts(cuts);

	fullWaterPhantomLV->SetRegion(regVol);
	regVol->AddRootLogicalVolume(fullWaterPhantomLV);

	// Visibility
	G4VisAttributes* simpleAlSVisAtt= new G4VisAttributes(G4Colour::Red());
	simpleAlSVisAtt->SetVisibility(true);
	simpleAlSVisAtt->SetForceSolid(true);
	fullWaterPhantomLV->SetVisAttributes(simpleAlSVisAtt);

	// Sensitive detector
	this->sensDet=new CML2SDWithVoxels("Water phantom", saving_in_ROG_Voxels_every_events, seed, ROGOutFile, bSaveROG, halfSize, 100, 100, 100);
	G4SDManager *SDManager=G4SDManager::GetSDMpointer();
	SDManager->AddNewDetector(this->sensDet);
	
	// Read Out Geometry
	CML2ReadOutGeometry *ROG = new CML2ReadOutGeometry(this->PVWorld->GetFrameRotation(), &this->PVWorld->GetFrameTranslation());
	ROG->setBuildData(G4ThreeVector(0.,0.,0.), halfSize, 100, 100, 100);
	ROG->BuildROGeometry();
	this->sensDet->SetROgeometry(ROG);
	fullWaterPhantomLV->SetSensitiveDetector(this->sensDet);

	bCreated=true;
	return bCreated;
}

