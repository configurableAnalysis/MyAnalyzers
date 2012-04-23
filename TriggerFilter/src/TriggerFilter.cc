// -*- C++ -*-
//
// Package:    TriggerFilter
// Class:      TriggerFilter
// 
/**\class TriggerFilter TriggerFilter.cc MyAnalyzers/TriggerFilter/src/TriggerFilter.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Wee Teo
//         Created:  Fri Mar 23 16:49:19 CDT 2012
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/Run.h"


#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"


#include "TPRegexp.h" 
#include "TString.h"

//
// class declaration
//

class TriggerFilter : public edm::EDFilter {
public:
  explicit TriggerFilter(const edm::ParameterSet&);
  ~TriggerFilter();
  
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
private:
  virtual void beginJob() ;
  virtual bool filter(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;
  
  virtual bool beginRun(edm::Run&, edm::EventSetup const&);
  virtual bool endRun(edm::Run&, edm::EventSetup const&);
  virtual bool beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);
  virtual bool endLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);
  
  virtual void configSelector(std::vector<std::string> selectTriggers, std::vector<std::string > & theSelectTriggers);

  
  // ----------member data ---------------------------
  edm::InputTag inputTag_;
  HLTConfigProvider hltConfig_;
  bool hltchange_;

  std::vector <std::vector <std::string> > triggerFilters_;
  std::vector <std::vector <uint> > triggerFilterIndices_;
  std::vector<std::string > filterTypes_;
  unsigned int HLTPathByIndex_;
  std::vector<std::string > dummyFilters_;

  std::vector<std::string > HLTPathNamesConfig_;
  std::vector<std::string > HLTPathNamesConfigPreVal_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
TriggerFilter::TriggerFilter(const edm::ParameterSet& iConfig):
  inputTag_ (iConfig.getParameter<edm::InputTag> ("TriggerResultsTag")),
  HLTPathNamesConfigPreVal_(iConfig.getParameter<std::vector<std::string > >("HLTPaths"))
{
   //now do what ever initialization is needed
  hltchange_=false;

  dummyFilters_.clear();
  filterTypes_ = iConfig.getUntrackedParameter<std::vector<std::string > >("filterTypes",dummyFilters_);

}


TriggerFilter::~TriggerFilter()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called on each new Event  ------------
bool
TriggerFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  
  // get hold of TriggerResults Object
  edm::Handle<edm::TriggerResults> trh;
  iEvent.getByLabel(inputTag_,trh);
  
  if (trh.failedToGet()) {
    edm::LogError("HLTMonBitSummary")<<" could not get: "<<inputTag_;
    return false;
  }

  //std::cout << "hey there, how's it going?" << std::endl;

  
  // get hold of trigger names - based on TriggerResults object!
  const edm::TriggerNames & triggerNames = iEvent.triggerNames(*trh);

  unsigned int lastModule = 0;
  
  if(HLTPathNamesConfig_.size()==1){
    
    HLTPathByIndex_ = triggerNames.triggerIndex( HLTPathNamesConfig_[0] );
    
    lastModule = trh->index(HLTPathByIndex_);
    
    ////go through the list of filters
    //for(unsigned int filt = 0; filt < triggerFilters_[0].size()-1; filt++){
    //  std::cout << "triggerFilters_["<<0<<"]["<<filt+1<<"] = " << triggerFilters_[0][filt+1] 
    //		<< " , triggerFilterIndices = " << triggerFilterIndices_[0][filt+1]
    //		<< " , lastModule = " << lastModule << std::endl;
    //}



    if(lastModule > triggerFilterIndices_[0][1]){
      //std::cout << "trigger passed prescaler" << std::endl;
      
      //if(trh->accept(HLTPathByIndex_)){
      //	std::cout << "\t trigger passed!!" << std::endl;
      return true;
      //}
      //else{
      //	std::cout << "\t trigger failed!" << std::endl;
      //	return false;
      //}

    }
    else{
      //std::cout << "trigger didn't pass prescaler" << std::endl;
      return false;
    }

  }

  return false;

}

// ------------ method called once each job just before starting event loop  ------------
void 
TriggerFilter::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
TriggerFilter::endJob() {
}

// ------------ method called when starting to processes a run  ------------
bool 
TriggerFilter::beginRun(edm::Run &r, edm::EventSetup const &iSetup)
{ 
  
  std::string processName = inputTag_.process();
  bool initOK = false;
  initOK = hltConfig_.init(r, iSetup, processName, hltchange_);

  HLTPathNamesConfig_.clear();
  if(HLTPathNamesConfigPreVal_.size()){
    //run trigger config selection
    if (initOK) {
      configSelector(HLTPathNamesConfigPreVal_, HLTPathNamesConfig_);
    }
    else{
      edm::LogError("HLTMonBitSummary") << "HLTConfigProvider initialization with process name " 
					<< processName << " failed." << std::endl;
    }
  }

  //for(uint i = 0; i< HLTPathNamesConfig_.size(); ++i){
  //  std::cout << "HLTPathNamesConfig_[" << i << "] = " << HLTPathNamesConfig_[i] << std::endl;
  //}

  
  //get all the filters -
  //only if filterTypes_ is nonempty and only on HLTPathNamesConfig_ paths
  if(initOK){
    //std::cout << "initok!" << std::endl; 
    if(!filterTypes_.empty()){
      triggerFilters_.clear();
      triggerFilterIndices_.clear();
      //for( size_t i = 0; i < nValidConfigTriggers_; i++) {
      if(HLTPathNamesConfig_.size()==1){
	// create a row [triggername,filter1name, filter2name, etc.] 
	triggerFilters_.push_back(std::vector <std::string>());  
	// create a row [0, filter1index, filter2index, etc.]
	triggerFilterIndices_.push_back(std::vector <uint>()); 
      
	std::vector<std::string> moduleNames = hltConfig_.moduleLabels( HLTPathNamesConfig_[0] );
      
	triggerFilters_[0].push_back( HLTPathNamesConfig_[0] );//first entry is trigger name      
	triggerFilterIndices_[0].push_back(0);
      
	int numModule = 0, numFilters = 0;
	std::string moduleName, moduleType;
	unsigned int moduleIndex;
      
	//print module name
	std::vector<std::string>::const_iterator iDumpModName;
	for (iDumpModName = moduleNames.begin();iDumpModName != moduleNames.end();iDumpModName++) {
	  moduleName = *iDumpModName;
	  moduleType = hltConfig_.moduleType(moduleName);
	  //moduleIndex = hltConfig_.moduleIndex(HLTPathNamesConfig_[i], moduleName);
	  moduleIndex = hltConfig_.moduleIndex( HLTPathNamesConfig_[0] , moduleName);
	  //LogDebug ("HLTMonBitSummary") 
	  //std::cout << "Module"      << numModule
	  //	    << " is called " << moduleName
	  //	    << " , type = "  << moduleType
	  //	    << " , index = " << moduleIndex
	  //	    << std::endl;
	  numModule++;
	  for(size_t k = 0; k < filterTypes_.size(); k++) {
	    if(moduleType == filterTypes_[k]) {
	      numFilters++;
	      triggerFilters_[0].push_back(moduleName);
	      triggerFilterIndices_[0].push_back(moduleIndex);
	    }
	  }
	}//end for modulesName
      }//end for nValidConfigTriggers_
    }
  }
  else{
    edm::LogError("HLTMonBitSummary") << "HLTConfigProvider initialization has failed."
				      << " Could not get filter names." << std::endl;
    //filterFlag_ = true;
  }
  
  return true;
}

// ------------ method called when ending the processing of a run  ------------
bool 
TriggerFilter::endRun(edm::Run&, edm::EventSetup const&)
{
  return true;
}

// ------------ method called when starting to processes a luminosity block  ------------
bool 
TriggerFilter::beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&)
{
  return true;
}

// ------------ method called when ending the processing of a luminosity block  ------------
bool 
TriggerFilter::endLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&)
{
  return true;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
TriggerFilter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}


void TriggerFilter::configSelector(std::vector<std::string> selectTriggers, std::vector<std::string > & theSelectTriggers){

  //get the configuration
  std::vector<std::string> validTriggerNames = hltConfig_.triggerNames(); 
  
  bool goodToGo = false;
  //remove all path names that are not valid
  while(!goodToGo && selectTriggers.size()!=0){
    goodToGo=true;
    for (std::vector<std::string>::iterator j=selectTriggers.begin();j!=selectTriggers.end();++j){
      //bool goodOne = false;
      //check if trigger name is valid
      //use of wildcard
      TPRegexp wildCard(*j);
      //std::cout << "wildCard.GetPattern() = " << wildCard.GetPattern() << std::endl;
      for (unsigned int i = 0; i != validTriggerNames.size(); ++i){
	if (TString(validTriggerNames[i]).Contains(wildCard)){ 
	  //goodOne = true;
	  if (find(theSelectTriggers.begin(),
		   theSelectTriggers.end(), 
		   validTriggerNames[i])==theSelectTriggers.end()){
	    //std::cout << "wildcard trigger = " << validTriggerNames[i] << std::endl;
	    theSelectTriggers.push_back( validTriggerNames[i] ); //add it after duplicate check.
	  }
	}
      }
    }
  }//while
}

//define this as a plug-in
DEFINE_FWK_MODULE(TriggerFilter);
