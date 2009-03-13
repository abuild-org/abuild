parameters {
    abuild.rules = 'java'
    java.warName = 'client-war-example.war'
    java.jarsToCopy = abuild.classpath
    java.webxml = 'war-example.xml'
}
