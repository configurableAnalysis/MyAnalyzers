import FWCore.ParameterSet.Config as cms

# Bit Plotting
triggerFilter = cms.EDFilter("TriggerFilter",
                             #directory = cms.untracked.string('myDirectory'),
                             #histLabel = cms.untracked.string('myHistLabel'),
                             #label = cms.string('myLabel'),
                             #out = cms.untracked.string('dqm.root'),
                             HLTPaths = cms.vstring('HLT_PFHT350_v[0-9]'),
                             filterTypes = cms.untracked.vstring( "HLTPrescaler" ),
                             #denominatorWild = cms.untracked.string(''),
                             #denominator = cms.untracked.string(''),
                             #eventSetupPathsKey = cms.untracked.string(''),
                             TriggerResultsTag = cms.InputTag('TriggerResults','','HLT')
)
