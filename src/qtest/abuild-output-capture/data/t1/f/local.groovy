abuild.addTargetClosure('all') {
    print 'oink'                // no newline
    System.out.flush()
    if (abuild.resolve('MISBEHAVE'))
    {
        // Passing the closure by putting after the method call rather
        // than explicitly passing it no longer works in groovy >= 1.7
        // for Thread because there's no way to distinguish the
        // closure from an anonymous inner class.
        def t = new Thread(new ThreadGroup(), {
            // Print something that doesn't end in a newline and do it
            // from a separate thread group so abuild won't be able to
            // associate it with a job.
            print 'moo'
            System.out.flush()
        })
        t.start()
        t.join()
    }
}
