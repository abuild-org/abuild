abuild.configureTarget("all") {
    abuild.runTarget("unknown")
}
abuild.configureTarget("all") {
    ant.echo('other all')
}
abuild.configureTarget("all") {
    ant.fail('ant failure')
}
abuild.configureTarget("all") {
    abuild.fail('abuild failure')
}
abuild.configureTarget("all") {
    throw new Exception('other failure')
}
