#include "SynchronizedExperimentDataQueue.hpp"

namespace fi {

void SynchronizedExperimentDataQueue::addData(ExperimentData* exp){
      //
      m_sema_full.wait();
      ExperimentDataQueue::addData(exp);
      m_sema_empty.post();
      //	
}


ExperimentData* SynchronizedExperimentDataQueue::getData(){
      //
      m_sema_empty.wait();
      return ExperimentDataQueue::getData();
      m_sema_full.post();
      //
}


};