parameters {
    abuild.rules = 'java'
    java.warName = 'client-war-example.war'
    java.jarsToSign = resolve(abuild.classpath)
    java.webxml = 'war-example.xml'
}
