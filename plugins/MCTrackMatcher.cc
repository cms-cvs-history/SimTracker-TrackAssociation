/** \class MCTrackMatcher 
 *
 * \author Luca Lista, INFN
 *
 * \version $Id: MCTrackMatcher.cc,v 1.2 2007/11/12 15:13:05 llista Exp $
 *
 */
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/ParameterSet/interface/InputTag.h"
#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

namespace edm { class ParameterSet; }

class MCTrackMatcher : public edm::EDProducer {
 public:
  /// constructor
  MCTrackMatcher( const edm::ParameterSet & );

 private:
  void produce( edm::Event& evt, const edm::EventSetup& es );
  std::string associator_;
  edm::InputTag tracks_, genParticles_;
  typedef edm::Association<reco::GenParticleCollection> GenParticleMatch;
};

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Utilities/interface/EDMException.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SimTracker/Records/interface/TrackAssociatorRecord.h"
#include "SimTracker/TrackAssociation/interface/TrackAssociatorBase.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"
using namespace edm;
using namespace std;
using namespace reco;

MCTrackMatcher::MCTrackMatcher(const ParameterSet & p) :
  associator_(p.getParameter<string>("associator")),
  tracks_(p.getParameter<InputTag>("tracks")),
  genParticles_( p.getParameter<InputTag>("genParticles")) {
  produces<GenParticleMatch>();
}

void MCTrackMatcher::produce(Event& evt, const EventSetup& es) {
  ESHandle<TrackAssociatorBase> assoc;  
  es.get<TrackAssociatorRecord>().get(associator_,assoc);
  const TrackAssociatorBase * associator = assoc.product();
  Handle<TrackCollection> tracks;
  evt.getByLabel(tracks_, tracks);
  Handle<TrackingParticleCollection> trackingParticles;
  evt.getByType(trackingParticles);
  Handle<vector<int> > barCodes;
  evt.getByLabel(genParticles_,barCodes );
  Handle<GenParticleCollection> genParticles;
  evt.getByLabel(genParticles_, genParticles );
  RecoToSimCollection associations = associator->associateRecoToSim ( tracks, trackingParticles, & evt ); 
  auto_ptr<GenParticleMatch> match(new GenParticleMatch(GenParticleRefProd(genParticles)));
  GenParticleMatch::Filler filler(*match);
  size_t n = tracks->size();
  vector<int> indices(n,-1);
  for (size_t i = 0; i < n; ++ i ) {
    TrackRef track(tracks, i);
    RecoToSimCollection::const_iterator f = associations.find(track);
    if ( f != associations.end() ) {
      TrackingParticleRef tp = f->val.front().first;
      const HepMC::GenParticle * particle = 0;
      TrackingParticle::genp_iterator j, b = tp->genParticle_begin(), e = tp->genParticle_end();
      for( j = b; j != e; ++ j ) {
	const HepMC::GenParticle * p = j->get();
	if (p->status() == 1) {
	  particle = p; break;
	}
      }
      if( particle != 0 ) {
	int barCode = particle->barcode();
	vector<int>::const_iterator 
	  b = barCodes->begin(), e = barCodes->end(), f = find( b, e, barCode );
	if(f == e) throw edm::Exception(errors::InvalidReference)
	  << "found matching particle with barcode" << *f
	  << " which has not been found in " << genParticles_;
	indices[i] = *f;
      }
    }
  }
  filler.insert(tracks, indices.begin(), indices.end());
  filler.fill();
  evt.put(match);
}

#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_FWK_MODULE( MCTrackMatcher );

