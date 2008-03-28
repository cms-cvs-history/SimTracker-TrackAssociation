import FWCore.ParameterSet.Config as cms

TrackAssociatorByHitsESProducer = cms.ESProducer("TrackAssociatorByHitsESProducer",
    associateRecoTracks = cms.bool(True),
    UseGrouped = cms.bool(True),
    associatePixel = cms.bool(True),
    ROUList = cms.vstring('TrackerHitsTIBLowTof', 'TrackerHitsTIBHighTof', 'TrackerHitsTIDLowTof', 'TrackerHitsTIDHighTof', 'TrackerHitsTOBLowTof', 'TrackerHitsTOBHighTof', 'TrackerHitsTECLowTof', 'TrackerHitsTECHighTof', 'TrackerHitsPixelBarrelLowTof', 'TrackerHitsPixelBarrelHighTof', 'TrackerHitsPixelEndcapLowTof', 'TrackerHitsPixelEndcapHighTof'),
    UseSplitting = cms.bool(False),
    UsePixels = cms.bool(True),
    ThreeHitTracksAreSpecial = cms.bool(True),
    AbsoluteNumberOfHits = cms.bool(False),
    associateStrip = cms.bool(True),
    MinHitCut = cms.double(0.5),
    SimToRecoDenominator = cms.string('sim') ##"reco"

)


