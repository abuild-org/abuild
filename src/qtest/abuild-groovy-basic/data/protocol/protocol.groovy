abuild.configureTarget('all') {
    println "hello from groovy"
    Thread.start {
        println "hello from groovy thread"
    }.join()
    println "¿Would you like\na πiece of π?";
}
