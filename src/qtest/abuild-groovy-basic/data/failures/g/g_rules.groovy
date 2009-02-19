abuild.configureTarget('all', 'deps': ['dep1', 'dep2'])
abuild.configureTarget('dep1', 'deps':'dep3') {
    abuild.runTarget('dep3')
}
abuild.configureTarget('dep2') {
    abuild.runTarget('all')
}
abuild.configureTarget('dep3')
