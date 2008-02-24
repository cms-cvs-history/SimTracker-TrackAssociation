#ifndef TrackAssociatorByHits_h
#define TrackAssociatorByHits_h

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/EDProduct.h"
#include "DataFormats/Common/interface/Ref.h"
#include "SimTracker/TrackerHitAssociation/interface/TrackerHitAssociator.h"

//reco track
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
//TrackingParticle
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"
#include "SimDataFormats/EncodedEventId/interface/EncodedEventId.h"
#include "SimTracker/TrackAssociation/interface/TrackAssociatorBase.h"

class TrackAssociatorByHits : public TrackAssociatorBase {
  
 public:
  explicit TrackAssociatorByHits( const edm::ParameterSet& );  
  ~TrackAssociatorByHits();
  
  /* Associate SimTracks to RecoTracks By Hits */
 
  /// Association Reco To Sim with Collections
  reco::RecoToSimCollection associateRecoToSim(edm::RefToBaseVector<reco::Track>&,
					       edm::RefVector<TrackingParticleCollection>&,
					       const edm::Event * event = 0 ) const ;
  /// Association Sim To Reco with Collections
  reco::SimToRecoCollection associateSimToReco(edm::RefToBaseVector<reco::Track>&,
					       edm::RefVector<TrackingParticleCollection>&,
					       const edm::Event * event = 0 ) const ;
  
  /// compare reco to sim the handle of reco::Track and TrackingParticle collections
  reco::RecoToSimCollection associateRecoToSim(edm::Handle<edm::View<reco::Track> >& tCH, 
					       edm::Handle<TrackingParticleCollection>& tPCH, 
					       const edm::Event * event = 0) const {
    return TrackAssociatorBase::associateRecoToSim(tCH,tPCH,event);
  }
  
  /// compare reco to sim the handle of reco::Track and TrackingParticle collections
  reco::SimToRecoCollection associateSimToReco(edm::Handle<edm::View<reco::Track> >& tCH, 
					       edm::Handle<TrackingParticleCollection>& tPCH,
					       const edm::Event * event = 0) const {
    return TrackAssociatorBase::associateSimToReco(tCH,tPCH,event);
  }  

  //seed
  reco::RecoToSimCollectionSeed associateRecoToSim(edm::Handle<TrajectorySeedCollection >&, 
						   edm::Handle<TrackingParticleCollection>&, 
						   const edm::Event * event = 0) const;
  
  reco::SimToRecoCollectionSeed associateSimToReco(edm::Handle<TrajectorySeedCollection >&, 
						   edm::Handle<TrackingParticleCollection>&, 
						   const edm::Event * event = 0) const;
  template<typename iter>
  void getMatchedIds(std::vector<SimHitIdpr>&, 
		     std::vector<SimHitIdpr>&, 
		     int&, 
		     iter,
		     iter,
		     TrackerHitAssociator*) const;
  
  int getShared(std::vector<SimHitIdpr>&, 
		std::vector<SimHitIdpr>&,
		TrackingParticleCollection::const_iterator) const;

 private:
  // ----- member data
  const edm::ParameterSet& conf_;
  const bool AbsoluteNumberOfHits;
  const std::string SimToRecoDenominator;
  const double theMinHitCut;    
  const bool UsePixels;
  const bool UseGrouped;
  const bool UseSplitting;
  const bool ThreeHitTracksAreSpecial;
  int LayerFromDetid(const DetId&) const;

  const TrackingRecHit* getHitPtr(edm::OwnVector<TrackingRecHit>::const_iterator iter) const {return &*iter;}
  const TrackingRecHit* getHitPtr(trackingRecHit_iterator iter) const {return &**iter;}
};

#endif
