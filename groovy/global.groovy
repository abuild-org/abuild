// Configure targets that abuild guarantees will exist

abuild.addTarget('all')

// Make test call test-only after building "all".
abuild.addTarget('test-only')
abuild.configureTarget('test', 'deps':'all') {
    abuild.runTarget('test-only')
}

// Make check an alias for test
abuild.addTargetDependencies('check', 'test')

abuild.addTarget('doc')
