// This file is loaded before any rules in this target type.  It
// should never be loaded manually by the user.

// Set parameters for directory structure and other global information.
def itemDir = abuild.sourceDirectory.absolutePath
def buildDir = abuild.buildDirectory.absolutePath

// XXX not fully incorporated...not sure what all these things are
//
// src/
//   java/
//   resources/
//   definitions/
//     wsdl/
//       item.wsdl
//       itemtypes.xsd
//     nonwsdltypes.xsd
//   conf/
//     metainf/
//     services/
//     application.xml
//     web.xml
//     ejb-jar.xml
//     ra.xml
//     item.jnlp
// web/
//   content/
// docs/
//   overview.html
//
// Generated:
//
// build/
//   generated/
//   classes/
//   item.jar
//   item.jnlp

parameters {
    // Set defaults for the "java" rules.

    java.dir.src = "${itemDir}/src/java"
    java.dir.resources = "${itemDir}/src/resources"
    java.dir.conf = "${itemDir}/src/conf"
    java.dir.metainf = "${itemDir}/src/conf/metainf"
    java.dir.web = "${itemDir}/src/web"
    java.dir.webcontent = "${itemDir}/src/web/content"

    java.dir.dist = "${buildDir}/dist"
    java.dir.classes = "${buildDir}/classes"
    java.dir.generatedDoc = "${buildDir}/doc"
    java.dir.generatedSrc = "${buildDir}/src/java"
    java.dir.generatedResources = "${buildDir}/src/resources"
    java.dir.generatedConf = "${buildDir}/src/conf"
    java.dir.generatedMetainf = "${buildDir}/src/conf/metainf"
    java.dir.generatedWeb = "${buildDir}/src/web"
    java.dir.generatedWebcontent = "${buildDir}/src/web/content"

    // Set defaults for the "groovy" rules.  The groovy rules require
    // that the java rules are also being used and get most of their
    // defaults from java parameters.

    // XXX Should we change these to src/groovy?
    groovy.dir.src = "${itemDir}/src/java"
    groovy.dir.generatedSrc = "${itemDir}/src/java"
}
