abuild.configureTarget('all') {
    println "hello from groovy"
    Thread.start {
        println "hello from groovy thread"
    }.join()
    if (abuild.resolve('SUPPORT_UTF8') == '1')
    {
        println "¿Would you like\na πiece of π?";
    }
    else
    {
        println "Would you like\na piece of pi?";
    }
}
