abuild.configureTarget('all') {
    println "all from local"
}
abuild.configureTarget('special', 'replaceClosures': true) {
    println "new special closure"
}
