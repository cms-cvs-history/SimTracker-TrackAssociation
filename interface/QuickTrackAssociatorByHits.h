#ifndef QuickTrackAssociatorByHits_h
#define QuickTrackAssociatorByHits_h

#include "SimTracker/TrackAssociation/interface/TrackAssociatorBase.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

// Forward declarations
class TrackerHitAssociator;

/** @brief TrackAssociator that associates by hits a bit quicker than the normal TrackAssociatorByHits class.
 *
 * NOTE - Doesn't implement the TrajectorySeed or TrackCandidate association methods (from TrackAssociatorBase) so will always
 * return empty associations for those.
 *
 * This track associator (mostly) does the same as TrackAssociatorByHits, but faster. I've tested it a fair bit and can't find
 * any differences between the results of this and the standard TrackAssociatorByHits.
 *
 * Configuration parameters:
 *
 * AbsoluteNumberOfHits - bool - if true, Quality_SimToReco and Cut_RecoToSim are the absolute number of shared hits required for
 * association, not the percentage.
 *
 * Quality_SimToReco - double - The minimum amount of shared hits required, as a percentage of either the reconstructed hits or
 * simulated hits (see SimToRecoDenominator), for the track to be considered associated during a call to associateSimToReco. See
 * also AbsoluteNumberOfHits.
 *
 * Purity_SimToReco - double - The minimum amount of shared hits required, as a percentage of the reconstructed hits, for the
 * track to be considered associated during a call to associateSimToReco. Has no effect if AbsoluteNumberOfHits is true.
 *
 * Cut_RecoToSim - double - The minimum amount of shared hits required, as a percentage of the reconstructed hits, for the track
 * to be considered associated during a call to associateRecoToSim. See also AbsoluteNumberOfHits.
 *
 * ThreeHitTracksAreSpecial - bool - If true, tracks with 3 hits must have all their hits associated.
 *
 * SimToRecoDenominator - string - Must be either "sim" or "reco". If "sim" Quality_SimToReco is the percentage of simulated hits
 * that need to be shared. If "reco" then it's the percentage of reconstructed hits (i.e. same as Purity_SimToReco).
 *
 * associatePixel - bool - Passed on to the hit associator.
 *
 * associateStrip - bool - Passed on to the hit associator.
 *
 *
 * Note that the TrackAssociatorByHits parameters UseGrouped and UseSplitting are not used.
 *
 * @author Mark Grimes (mark.grimes@cern.ch)
 * @date 09/Nov/2010
 * Significant changes to remove any differences to the standard TrackAssociatorByHits results 07/Jul/2011
 */
class QuickTrackAssociatorByHits : public TrackAssociatorBase
{
public:
	QuickTrackAssociatorByHits( const edm::ParameterSet& config );
	~QuickTrackAssociatorByHits();
	QuickTrackAssociatorByHits( const QuickTrackAssociatorByHits& otherAssociator );
	QuickTrackAssociatorByHits& operator=( const QuickTrackAssociatorByHits& otherAssociator );
	reco::RecoToSimCollection associateRecoToSim( edm::Handle<edm::View<reco::Track> >& trackCollectionHandle,
	                                              edm::Handle<TrackingParticleCollection>& trackingParticleCollectionHandle,
	                                              const edm::Event* pEvent=0,
	                                              const edm::EventSetup* pSetup=0 ) const;
	reco::SimToRecoCollection associateSimToReco( edm::Handle<edm::View<reco::Track> >& trackCollectionHandle,
	                                              edm::Handle<TrackingParticleCollection>& trackingParticleCollectionHandle,
	                                              const edm::Event* pEvent=0,
	                                              const edm::EventSetup* pSetup=0 ) const;
	reco::RecoToSimCollection associateRecoToSim( const edm::RefToBaseVector<reco::Track>& trackCollection,
												  const edm::RefVector<TrackingParticleCollection>& trackingParticleCollection,
												  const edm::Event* pEvent=0,
												  const edm::EventSetup* pSetup=0 ) const;
	reco::SimToRecoCollection associateSimToReco( const edm::RefToBaseVector<reco::Track>& trackCollection,
												  const edm::RefVector<TrackingParticleCollection>& trackingParticleCollection,
												  const edm::Event* pEvent=0,
												  const edm::EventSetup* pSetup=0 ) const;
private:
	typedef std::pair<uint32_t,EncodedEventId> SimTrackIdentifiers;
	enum SimToRecoDenomType {denomnone,denomsim,denomreco};

	/** @brief The method that does the work for both overloads of associateRecoToSim.
	 */
	reco::RecoToSimCollection associateRecoToSimImplementation() const;

	/** @brief The method that does the work for both overloads of associateSimToReco.
	 */
	reco::SimToRecoCollection associateSimToRecoImplementation() const;

	/** @brief Returns the TrackingParticle that has the most associated hits to the given track.
	 *
	 * Return value is a vector of pairs, where first is an edm::Ref to the associated TrackingParticle, and second is
	 * the number of associated hits.
	 */
	std::vector< std::pair<edm::Ref<TrackingParticleCollection>,size_t> > associateTrack( const reco::Track* pTrack ) const;

	/** @brief Returns true if the supplied TrackingParticle has the supplied g4 track identifiers. */
	bool trackingParticleContainsIdentifier( const TrackingParticle* pTrackingParticle, const SimTrackIdentifiers& identifier ) const;

	/** @brief This method was copied almost verbatim from the standard TrackAssociatorByHits. */
	int getDoubleCount( trackingRecHit_iterator begin, trackingRecHit_iterator end, const TrackingParticle& associatedTrackingParticle ) const;

	/** @brief Returns a vector of pairs where first is a SimTrackIdentifiers (see typedef above) and second is the number of hits that came from that sim track.
	 *
	 * This is used so that the TrackingParticle collection only has to be looped over once to search for each sim track, rather than once per hit.
	 * E.g. If all the hits in the reco track come from the same sim track, then there will only be one entry with second as the number of hits in
	 * the track.
	 */
	std::vector< std::pair<SimTrackIdentifiers,size_t> > getAllSimTrackIdentifiers( const reco::Track* pTrack ) const;

	//
	// Members. Note that there are custom copy constructor and assignment operators, so if any members are added
	// those methods will need to be updated.
	//
	mutable TrackerHitAssociator* pHitAssociator_;
	const mutable edm::Event* pEventForWhichAssociatorIsValid_;
	void initialiseHitAssociator( const edm::Event* event ) const;

	edm::ParameterSet hitAssociatorParameters_;

	bool absoluteNumberOfHits_;
	double qualitySimToReco_;
	double puritySimToReco_;
	double cutRecoToSim_;
	bool threeHitTracksAreSpecial_;
	SimToRecoDenomType simToRecoDenominator_;

	/** @brief Pointer to the handle to the track collection.
	 *
	 * Only one of pTrackCollectionHandle_ or pTrackCollection_ will ever be non Null. This is so that both flavours of the
	 * associateRecoToSim (one takes a Handle, the other a RefToBaseVector) can use the same associateRecoToSimImplementation
	 * method and keep the logic for both in one place.  The old implementation for the handle flavour copied everything into
	 * a new RefToBaseVector, wasting memory.  I tried to do something clever with templates but couldn't get it to work, so
	 * the associateRecoToSimImplementation method checks which is non Null and uses that to get the tracks.
	 */
	mutable edm::Handle<edm::View<reco::Track> >* pTrackCollectionHandle_;

	/** @brief Pointer to the track collection.
	 *
	 * Either this or pTrackCollectionHandle_ will be set, the other will be Null. See the comment on pTrackCollectionHandle_
	 * for reasons why.
	 */
	mutable const edm::RefToBaseVector<reco::Track>* pTrackCollection_;

	/** @brief Pointer to the TrackingParticle collection handle
	 *
	 * Either this or pTrackingParticleCollection_ will be set, the other will be Null. See the comment on pTrackCollectionHandle_
	 * for reasons why.
	 */
	mutable edm::Handle<TrackingParticleCollection>* pTrackingParticleCollectionHandle_;

	/** @brief Pointer to the TrackingParticle collection handle
	 *
	 * Either this or pTrackingParticleCollectionHandle_ will be set, the other will be Null. See the comment on pTrackCollectionHandle_
	 * for reasons why.
	 */
	mutable const edm::RefVector<TrackingParticleCollection>* pTrackingParticleCollection_;
}; // end of the QuickTrackAssociatorByHits class

#endif // end of ifndef QuickTrackAssociatorByHits_h
