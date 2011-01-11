abuild.addTargetClosure('all') {
    print 'oink'                // no newline
    System.out.flush()
    if (abuild.resolve('MISBEHAVE'))
    {
        // For some reason, passing the closure by putting after the
        // method call rather than explicit passing it works in groovy
        // 1.6.7 but not in 1.7.6.
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
