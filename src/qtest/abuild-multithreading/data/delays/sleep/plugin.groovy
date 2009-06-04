abuild.addTargetClosure('all') {
    def name = abuild.resolve('ABUILD_ITEM_NAME')
    println "[[$name]]:$name before"
    sleep(2000)
    println "[[$name]]:$name after"
}
