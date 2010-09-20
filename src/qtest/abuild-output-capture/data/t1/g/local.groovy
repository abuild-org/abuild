abuild.addTargetClosure('all') {
    if (abuild.resolve('MISBEHAVE'))
    {
        System.exit(1)
    }
}
