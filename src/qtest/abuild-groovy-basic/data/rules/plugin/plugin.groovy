abuild.configureTarget('all') {
    print "all from plugin top-level: "
    println abuild.resolve('pre.plugin.param').join(' ')
}
