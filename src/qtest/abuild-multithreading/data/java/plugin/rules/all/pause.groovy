abuild.addTargetClosure('all') {
    Thread.currentThread().sleep(abuild.resolve('pause.duration'))
}
