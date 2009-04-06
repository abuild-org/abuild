parameters {
    abuild.rules = 'java'
    java.warName = 'war-example.war'
    java.warLibJars = resolve(abuild.classpath)
    java.webxml = 'war-example.xml'
    java.packageWar << ['filestopackage' : ['filex']]
}
