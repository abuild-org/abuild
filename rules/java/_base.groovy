// This file is loaded before any rules in this target type.  It
// should never be loaded manually by the user.

def itemDir = abuild.sourceDirectory.absolutePath
def buildDir = abuild.buildDirectory.absolutePath

// Set parameters for directory structure and other global information.
parameters {
    java.dir.src = "${itemDir}/src/java"
    java.dir.resources = "${itemDir}/src/resources"
    java.dir.metainf = "${itemDir}/src/conf/META-INF"
    java.dir.webContent = "${itemDir}/src/web/content"
    java.dir.webinf = "${itemDir}/src/web/WEB-INF"

    java.dir.dist = "${buildDir}/dist"
    java.dir.classes = "${buildDir}/classes"
    java.dir.signedJars = "${buildDir}/signed-jars"

    java.dir.generatedDoc = "${buildDir}/doc"
    java.dir.generatedSrc = "${buildDir}/src/java"
    java.dir.generatedResources = "${buildDir}/src/resources"
    java.dir.generatedMetainf = "${buildDir}/src/conf/META-INF"
    java.dir.generatedWebContent = "${buildDir}/src/web/content"
    java.dir.generatedWebinf = "${buildDir}/src/web/WEB-INF"

    java.dir.extraSrc = []
    java.dir.extraResources
    java.dir.extraMetainf = []
    java.dir.extraWebContent = []
    java.dir.extraWebinf = []

    // Set defaults for the "groovy" rules.  The groovy rules require
    // that the java rules are also being used and get most of their
    // defaults from java parameters.
    groovy.dir.src = "${itemDir}/src/groovy"
    groovy.dir.generatedSrc = "${itemDir}/src/groovy"
}
