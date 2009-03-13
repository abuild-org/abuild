parameters {
    abuild.rules = 'java'
    java.warName = 'client-war-example.war'
    java.jarsToCopy = resolve(abuild.classpath)
    java.webxml = 'war-example.xml'
}
