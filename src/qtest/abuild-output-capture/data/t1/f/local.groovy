abuild.addTargetClosure('all') {
    print 'oink'                // no newline
    System.out.flush()
    def t = new Thread(new ThreadGroup()) {
        // Print something that doesn't end in a newline and do it
        // from a separate thread group so abuild won't be able to
        // associate it with a job.
        print 'moo'
        System.out.flush()
    }
    t.start()
    t.join()
}
