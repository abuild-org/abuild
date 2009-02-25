abuild.configureTarget("all", 'deps':['other1', 'other2']) {
    println "all"
}
abuild.configureTarget("other1") {
    abuild.fail("other1 failure")
}
abuild.configureTarget("other2") {
    println "other2"
}
abuild.configureTarget("other2") {
    println "other2 with failure"
    abuild.configureTarget('other2', 'deps':'other1')
}
