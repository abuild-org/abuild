parameters {
    abuild.rules = 'java'
    java.jarName = 'potato-test.jar'
    java.wrapperName = 'test_potato'
    java.mainClass = 'com.example.PotatoTest'
    java.junitTestsuite = 'com.example.PotatoTest'
    java.junitBatchIncludes = '**/*Test.class'
    java.junitBatchExcludes = '**/PotatoTest.class'
}
