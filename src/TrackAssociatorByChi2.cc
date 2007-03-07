#include "SimTracker/TrackAssociation/interface/TrackAssociatorByChi2.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"

// #include "TrackingTools/GeomPropagators/interface/HelixExtrapolatorToLine2Order.h"
#include "DataFormats/GeometrySurface/interface/Line.h"
#include "DataFormats/GeometryVector/interface/Pi.h"

using namespace edm;
using namespace reco;
using namespace std;

double TrackAssociatorByChi2::compareTracksParam ( TrackCollection::const_iterator rt, 
						   SimTrackContainer::const_iterator st, 
						   const HepLorentzVector vertexPosition, 
						   GlobalVector magField,
						   TrackBase::CovarianceMatrix  
						   invertedCovariance  ) const{
  
  Basic3DVector<double> momAtVtx(st->momentum().x(),st->momentum().y(),st->momentum().z());
  Basic3DVector<double> vert = (Basic3DVector<double>) vertexPosition;
      
  TrackBase::ParameterVector sParameters=parametersAtClosestApproachGeom(vert, momAtVtx, st->charge());
  TrackBase::ParameterVector rParameters = rt->parameters();
  
  TrackBase::ParameterVector diffParameters = rParameters - sParameters;
  double chi2 = ROOT::Math::Dot(diffParameters * invertedCovariance, diffParameters);
  
  return chi2;
}


TrackAssociatorByChi2::RecoToSimPairAssociation 
TrackAssociatorByChi2::compareTracksParam(const TrackCollection& rtColl,
					  const SimTrackContainer& stColl,
					  const SimVertexContainer& svColl) const{
  
  RecoToSimPairAssociation outputVec;

  for (TrackCollection::const_iterator track=rtColl.begin(); track!=rtColl.end(); track++){
     Chi2SimMap outMap;

    TrackBase::ParameterVector rParameters = track->parameters();
    TrackBase::CovarianceMatrix recoTrackCovMatrix = track->covariance();
    if (onlyDiagonal){
      for (unsigned int i=0;i<5;i++){
	for (unsigned int j=0;j<5;j++){
	  if (i!=j) recoTrackCovMatrix(i,j)=0;
	}
      }
    }
    recoTrackCovMatrix.Invert();

    for (SimTrackContainer::const_iterator st=stColl.begin(); st!=stColl.end(); st++){

      Basic3DVector<double> momAtVtx(st->momentum().x(),st->momentum().y(),st->momentum().z());
      Basic3DVector<double> vert = (Basic3DVector<double>)  svColl[st->vertIndex()].position();

      TrackBase::ParameterVector sParameters=parametersAtClosestApproachGeom(vert, momAtVtx, st->charge());
      
      TrackBase::ParameterVector diffParameters = rParameters - sParameters;
      double chi2 = ROOT::Math::Dot(diffParameters * recoTrackCovMatrix, diffParameters);
      chi2/=5;
      if (chi2<chi2cut) outMap[chi2]=*st;
    }
    outputVec.push_back(RecoToSimPair(*track,outMap));
  }
  return outputVec;
}


RecoToSimCollection TrackAssociatorByChi2::associateRecoToSim(edm::Handle<reco::TrackCollection>& tCH, 
							      edm::Handle<TrackingParticleCollection>& tPCH,
							      const edm::Event * e ) const{

  RecoToSimCollection  outputCollection;
  double chi2;

  const TrackCollection tC = *(tCH.product());
  const TrackingParticleCollection tPC= *(tPCH.product());

  int tindex=0;
  for (TrackCollection::const_iterator rt=tC.begin(); rt!=tC.end(); rt++, tindex++){

    LogDebug("TrackAssociator") << "=========LOOKING FOR ASSOCIATION===========" << "\n"
				<< "rec::Track #"<<tindex<<" with pt=" << rt->pt() <<  "\n"
				<< "===========================================" << "\n";
 
    TrackBase::ParameterVector rParameters = rt->parameters();
    TrackBase::CovarianceMatrix recoTrackCovMatrix = rt->covariance();
    if (onlyDiagonal){
      for (unsigned int i=0;i<5;i++){
	for (unsigned int j=0;j<5;j++){
	  if (i!=j) recoTrackCovMatrix(i,j)=0;
	}
      }
    } 

    recoTrackCovMatrix.Invert();

    int tpindex =0;
    for (TrackingParticleCollection::const_iterator tp=tPC.begin(); tp!=tPC.end(); tp++, ++tpindex){
	
      //skip tps with a very small pt
      if (sqrt(tp->momentum().perp2())<0.5) continue;
	
      Basic3DVector<double> momAtVtx(tp->momentum().x(),tp->momentum().y(),tp->momentum().z());
      Basic3DVector<double> vert=(Basic3DVector<double>) tp->vertex();

      TrackBase::ParameterVector sParameters=parametersAtClosestApproachGeom(vert, momAtVtx, tp->charge());

      TrackBase::ParameterVector diffParameters = rParameters - sParameters;

      chi2 = ROOT::Math::Similarity(diffParameters, recoTrackCovMatrix);
      chi2 /= 5;

      LogDebug("TrackAssociator") << "====RECO TRACK WITH PT=" << rt->pt() << "====\n" 
				  << "qoverp simG: " << sParameters[0] << "\n" 
				  << "lambda simG: " << sParameters[1] << "\n" 
				  << "phi    simG: " << sParameters[2] << "\n" 
				  << "dxy    simG: " << sParameters[3] << "\n" 
				  << "dsz    simG: " << sParameters[4] << "\n" 
				  << ": " /*<< */ << "\n" 
				  << "qoverp rec: " << rt->qoverp()/*rParameters[0]*/ << "\n" 
				  << "lambda rec: " << rt->lambda()/*rParameters[1]*/ << "\n" 
				  << "phi    rec: " << rt->phi()/*rParameters[2]*/ << "\n" 
				  << "dxy    rec: " << rt->dxy()/*rParameters[3]*/ << "\n" 
				  << "dsz    rec: " << rt->dsz()/*rParameters[4]*/ << "\n" 
				  << ": " /*<< */ << "\n" 
				  << "chi2: " << chi2 << "\n";

      if (chi2<chi2cut) {
	outputCollection.insert(reco::TrackRef(tCH,tindex), 
				std::make_pair(edm::Ref<TrackingParticleCollection>(tPCH, tpindex),
					       -chi2));//-chi2 because the Association Map is ordered using std::greater
      }
    }
  }
  return outputCollection;
}



SimToRecoCollection TrackAssociatorByChi2::associateSimToReco(edm::Handle<reco::TrackCollection>& tCH, 
							      edm::Handle<TrackingParticleCollection>& tPCH,
							      const edm::Event * e ) const {
  SimToRecoCollection  outputCollection;
  double chi2;

  const TrackCollection tC = *(tCH.product());
  const TrackingParticleCollection tPC= *(tPCH.product());

  int tpindex =0;
  for (TrackingParticleCollection::const_iterator tp=tPC.begin(); tp!=tPC.end(); tp++, ++tpindex){
    
    //skip tps with a very small pt
    if (sqrt(tp->momentum().perp2())<0.5) continue;
    
    LogDebug("TrackAssociator") << "=========LOOKING FOR ASSOCIATION===========" << "\n"
				<< "TrackingParticle #"<<tpindex<<" with pt=" << sqrt(tp->momentum().perp2()) << "\n"
				<< "===========================================" << "\n";
    
    Basic3DVector<double> momAtVtx(tp->momentum().x(),tp->momentum().y(),tp->momentum().z());
    Basic3DVector<double> vert(tp->vertex().x(),tp->vertex().y(),tp->vertex().z());
    
    TrackBase::ParameterVector sParameters=parametersAtClosestApproachGeom(vert, momAtVtx, tp->charge());
    
    int tindex=0;
    for (TrackCollection::const_iterator rt=tC.begin(); rt!=tC.end(); rt++, tindex++){
      
      TrackBase::ParameterVector rParameters = rt->parameters();
      TrackBase::CovarianceMatrix recoTrackCovMatrix = rt->covariance();
      if (onlyDiagonal) {
	for (unsigned int i=0;i<5;i++){
	  for (unsigned int j=0;j<5;j++){
	    if (i!=j) recoTrackCovMatrix(i,j)=0;
	  }
	}
      }
      
      recoTrackCovMatrix.Invert();
      
      TrackBase::ParameterVector diffParameters = rParameters - sParameters;
      
      chi2 = ROOT::Math::Similarity(recoTrackCovMatrix, diffParameters);
      chi2 /= 5;

      LogDebug("TrackAssociator") << "====RECO TRACK WITH PT=" << rt->pt() << "====\n" 
				  << "qoverp simG: " << sParameters[0] << "\n" 
				  << "lambda simG: " << sParameters[1] << "\n" 
				  << "phi    simG: " << sParameters[2] << "\n" 
				  << "dxy    simG: " << sParameters[3] << "\n" 
				  << "dsz    simG: " << sParameters[4] << "\n" 
				  << ": " /*<< */ << "\n" 
				  << "qoverp rec: " << rt->qoverp()/*rParameters[0]*/ << "\n" 
				  << "lambda rec: " << rt->lambda()/*rParameters[1]*/ << "\n" 
				  << "phi    rec: " << rt->phi()/*rParameters[2]*/ << "\n" 
				  << "dxy    rec: " << rt->dxy()/*rParameters[3]*/ << "\n" 
				  << "dsz    rec: " << rt->dsz()/*rParameters[4]*/ << "\n" 
				  << ": " /*<< */ << "\n" 
				  << "chi2: " << chi2 << "\n";
      
      if (chi2<chi2cut) {
	outputCollection.insert(edm::Ref<TrackingParticleCollection>(tPCH, tpindex),
				std::make_pair(reco::TrackRef(tCH,tindex),
					       -chi2));//-chi2 because the Association Map is ordered using std::greater
      }
    }
  }
  return outputCollection;
}

double TrackAssociatorByChi2::associateRecoToSim( TrackCollection::const_iterator rt, 
						  TrackingParticleCollection::const_iterator tp ) const{
  
  double chi2;
  
  TrackBase::ParameterVector rParameters = rt->parameters();
  TrackBase::CovarianceMatrix recoTrackCovMatrix = rt->covariance();
      if (onlyDiagonal) {
	for (unsigned int i=0;i<5;i++){
	  for (unsigned int j=0;j<5;j++){
	    if (i!=j) recoTrackCovMatrix(i,j)=0;
	  }
	}
      }
  recoTrackCovMatrix.Invert();
  
  Basic3DVector<double> momAtVtx(tp->momentum().x(),tp->momentum().y(),tp->momentum().z());
  Basic3DVector<double> vert(tp->vertex().x(),tp->vertex().y(),tp->vertex().z());
  TrackBase::ParameterVector sParameters=parametersAtClosestApproachGeom(vert, momAtVtx, tp->charge());
    
  TrackBase::ParameterVector diffParameters = rParameters - sParameters;
  chi2 = ROOT::Math::Dot(diffParameters * recoTrackCovMatrix, diffParameters);
  chi2 /= 5;

  LogDebug("TrackAssociator") << "====NEW RECO TRACK WITH PT=" << rt->pt() << "====\n" 
			      << "qoverp simG: " << sParameters[0] << "\n" 
			      << "lambda simG: " << sParameters[1] << "\n" 
			      << "phi    simG: " << sParameters[2] << "\n" 
			      << "dxy    simG: " << sParameters[3] << "\n" 
			      << "dsz    simG: " << sParameters[4] << "\n" 
			      << ": " /*<< */ << "\n" 
			      << "qoverp rec: " << rt->qoverp()/*rParameters[0]*/ << "\n" 
			      << "lambda rec: " << rt->lambda()/*rParameters[1]*/ << "\n" 
			      << "phi    rec: " << rt->phi()/*rParameters[2]*/ << "\n" 
			      << "dxy    rec: " << rt->dxy()/*rParameters[3]*/ << "\n" 
			      << "dsz    rec: " << rt->dsz()/*rParameters[4]*/ << "\n" 
			      << ": " /*<< */ << "\n" 
			      << "chi2: " << chi2 << "\n";
  
  return chi2;
}

TrackBase::ParameterVector TrackAssociatorByChi2::parametersAtClosestApproachGeom (Basic3DVector<double> vertex,
										   Basic3DVector<double> momAtVtx,
										   float charge) const{
  GlobalVector magField = theMF->inTesla( (GlobalPoint) vertex );
  double simTrCurv = -charge*2.99792458e-3 * magField.z()/momAtVtx.perp();

  double rho = fabs(1/simTrCurv);

  double phiAtVtx = momAtVtx.phi();
  
  double d0sim1,s,dzsim1,beta,phisim ;

  d0sim1 = rho-sqrt(rho*rho+vertex.x()*vertex.x()+vertex.y()*vertex.y()
		    +2*rho*(-sin(phiAtVtx)*vertex.x()+cos(phiAtVtx)*vertex.y()));
  
  s = rho*(atan2(cos(phiAtVtx)*vertex.x()+sin(phiAtVtx)*vertex.y(),
		 rho-sin(phiAtVtx)*vertex.x()+cos(phiAtVtx)*vertex.y()));
  
  dzsim1 = vertex.z() - s*momAtVtx.z()/momAtVtx.perp();
  
  beta = atan2(rho*cos(phiAtVtx)+vertex.y(),
	       rho*sin(phiAtVtx)-vertex.x() );
  
  phisim = +beta-Geom::halfPi();
  
  if (beta<0) {
    phisim = +beta+3*Geom::halfPi();
  }
  
  //FIXME??
  phisim=-phisim;
  
  if (phisim<-Geom::pi()) {
    phisim+=2*Geom::pi();
  }

  double qoverp = charge/momAtVtx.mag();

  TrackBase::ParameterVector sParameters;
  //sParameters[0] = simTrCurv;
  //sParameters[1] = momAtVtx.theta();
  sParameters[0] = qoverp;
  sParameters[1] = Geom::halfPi() - momAtVtx.theta();
  sParameters[2] = phisim;
  sParameters[3] = -d0sim1;
  sParameters[4] = dzsim1*momAtVtx.perp()/momAtVtx.mag();

#if 0
  GlobalVector pca(d0sim1*sin(phisim),d0sim1*cos(phisim),dzsim1);
  GlobalVector momAtPca(momAtVtx.perp()*cos(phisim),momAtVtx.perp()*sin(phisim),momAtVtx.z());
  double helixCenterX = (rho-d0sim1)*sin(phisim);
  double helixCenterY = (rho-d0sim1)*cos(phisim);
  double centerToPcaX = (helixCenterX-pca.x());
  double centerToPcaY = (helixCenterY-pca.y());
  LogDebug("TrackAssociator") << "+++++++++++++++parametersAtClosestApproachGEOM++++++++++++++" << "\n"
    //<< "alpha: " << atan2(cos(phiAtVtx)*vertex.x()+sin(phiAtVtx)*vertex.y(),
    //rho-sin(phiAtVtx)*vertex.x()+cos(phiAtVtx)*vertex.y()) << "\n"
    //<< "alph1: " << Geom::pi()+atan2(cos(phiAtVtx)*vertex.x()+sin(phiAtVtx)*vertex.y(),
    //rho-sin(phiAtVtx)*vertex.x()+cos(phiAtVtx)*vertex.y()) << "\n"
    //<< "alph2: " << atan2(rho-sin(phiAtVtx)*vertex.x()+cos(phiAtVtx)*vertex.y(),
    //cos(phiAtVtx)*vertex.x()+sin(phiAtVtx)*vertex.y()) << "\n"
    //<< "beta: " << beta << "\n"    
			      << "vertex.x(): " << vertex.x() << "\n"
			      << "vertex.y(): " << vertex.y() << "\n"
			      << "vertex.z(): " << vertex.z() << "\n"
			      << "pca.x(): " << pca.x() << "\n"
			      << "pca.y(): " << pca.y() << "\n"
			      << "pca.z(): " << pca.z() << "\n"
			      << "helixCenterX: " << helixCenterX << "\n"
			      << "helixCenterY: " << helixCenterY << "\n"
			      << "centerToPcaX: " << centerToPcaX << "\n"
			      << "centerToPcaY: " << centerToPcaY << "\n"
			      << "CO^2: " << helixCenterX*helixCenterX+helixCenterY*helixCenterY << "\n"
			      << "CA^2: " << centerToPcaX*centerToPcaX+centerToPcaY*centerToPcaY << "\n"
			      << "R: " << sqrt(centerToPcaX*centerToPcaX+centerToPcaY*centerToPcaY) << "\n"
			      << "1/R: " << 1/sqrt(centerToPcaX*centerToPcaX+centerToPcaY*centerToPcaY) << "\n"
			      << "path: " << s << "\n"
			      << "momAtPca.x(): " << momAtPca.x() << "\n"
			      << "momAtPca.y(): " << momAtPca.y() << "\n"
			      << "momAtPca.z(): " << momAtPca.z() << "\n"
			      << "momAtPca.mag(): " << momAtPca.mag() << "\n"
			      << "momAtPca.perp(): " << momAtPca.perp() << "\n"
			      << "momAtVtx.x(): " << momAtVtx.x() << "\n"
			      << "momAtVtx.y(): " << momAtVtx.y() << "\n"
			      << "momAtVtx.z(): " << momAtVtx.z() << "\n"
			      << "momAtVtx.mag(): " << momAtVtx.mag() << "\n"
			      << "momAtVtx.perp(): " << momAtVtx.perp() << "\n"
			      << "magField.z()   : " << magField.z() << "\n"
			      << "magField.perp(): " << magField.perp() << "\n"
			      << " " /*<< */ << "\n"
			      << "qoverp: " << sParameters[0] << "\n"
			      << "lambda: " << sParameters[1] << "\n"
			      << "phi   : " << sParameters[2] << "\n"
			      << "dxy   : " << sParameters[3] << "\n"
			      << "dsz   : " << sParameters[4] << "\n"
			      << " " /*<< */ << "\n"
			      << " " /*<< */ << "\n";
#endif
  return sParameters;
}
	
