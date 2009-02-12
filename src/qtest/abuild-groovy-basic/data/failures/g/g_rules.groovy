abuild.setTarget('all', 'deps': ['dep1', 'dep2'])
abuild.setTarget('dep1', 'deps':'dep3') {
    abuild.runTarget('dep3')
}
abuild.setTarget('dep2') {
    abuild.runTarget('all')
}
abuild.setTarget('dep3')
