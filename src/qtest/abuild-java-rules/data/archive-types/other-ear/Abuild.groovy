parameters {
    abuild.rules = 'java'
    java.earName = 'other-ear-example.ear'
    java.appxml = 'application.xml'
    // Prevent jar-example.jar from being included in the ear.
    def archives = resolve('abuild.classpath').grep {
        new File(it).name != 'jar-example.jar' }
    java.packageEar = ['filestopackage': archives]
}
