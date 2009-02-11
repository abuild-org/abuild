abuild.setTarget("all", 'deps':['other1', 'other2']) {
    println "all"
}
abuild.setTarget("other1") {
    abuild.fail("other1 failure")
}
abuild.setTarget("other2") {
    println "other2"
}
