import FWCore.ParameterSet.Config as cms

trackMCMatch = cms.EDFilter("MCTrackMatcher",
    trackingParticles = cms.InputTag("mergedtruth","MergedTrackTruth"),
    tracks = cms.InputTag("generalTracks"),
    genParticles = cms.InputTag("genParticles"),
    associator = cms.string('TrackAssociatorByHits')
)


