abuild.setTarget("all") {
    abuild.runTarget("unknown")
}
abuild.setTarget("all") {
    ant.echo('other all')
}
abuild.setTarget("all") {
    ant.fail('ant failure')
}
abuild.setTarget("all") {
    abuild.fail('abuild failure')
}
abuild.setTarget("all") {
    throw new Exception('other failure')
}
