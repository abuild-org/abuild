#ifndef __DEPENDENCYRUNNER_HH__
#define __DEPENDENCYRUNNER_HH__

// This classes uses a worker pool to evaluate all the items in the
// given dependency graph.  It does so by using a DependencyEvaluator
// object and a worker pool to process as many items in parallel as
// possible (subject to the maximum number of workers) such that no
// item is processed unless all of its dependencies have completed.

#include <DependencyEvaluator.hh>
#include <WorkerPool.hh>

class DependencyRunner
{
  public:
    DependencyRunner(DependencyGraph const& g,
		     int num_workers,
		     boost::function<bool (std::string)> const& method);

    // Returns false iff there were any errors.  If
    // stop_on_first_error is true, if any item fails, this method
    // returns after all currently pending jobs are completed.
    bool run(bool stop_on_first_error = false);

    // Return the internally stored evaluator and graph.
    DependencyEvaluator const& getEvaluator() const;
    DependencyGraph const& getGraph() const;
    // Set the evaluator's change callback
    void setChangeCallback(DependencyEvaluator::ChangeCallback,
			   bool call_now);

  private:
    DependencyRunner(DependencyRunner const&);
    DependencyRunner& operator=(DependencyRunner const&);

    DependencyEvaluator evaluator;
    WorkerPool<std::string, bool> pool;
};

#endif // __DEPENDENCYRUNNER_HH__
