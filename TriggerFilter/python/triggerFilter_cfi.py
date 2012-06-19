import FWCore.ParameterSet.Config as cms

# Bit Plotting
pfht350PassPrescaleFilter = cms.EDFilter("TriggerFilter",
                             HLTPaths = cms.vstring('HLT_PFHT350_v[0-9]'),
                             filterTypes = cms.untracked.vstring( "HLTPrescaler" ),
                             TriggerResultsTag = cms.InputTag('TriggerResults','','HLT')
)
