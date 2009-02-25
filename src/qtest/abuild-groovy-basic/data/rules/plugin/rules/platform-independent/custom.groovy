abuild.configureTarget('all') {
    println "all from custom"
}

abuild.configureTarget('special') {
    println "special target: original closure"
}
