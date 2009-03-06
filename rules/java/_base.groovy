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
    abuild.default.dir.src = "${itemDir}/src/java"
    abuild.default.dir.resources = "${itemDir}/src/resources"
    abuild.default.dir.conf = "${itemDir}/src/conf"
    abuild.default.dir.metainf = "${itemDir}/src/conf/metainf"
    abuild.default.dir.web = "${itemDir}/src/web"
    abuild.default.dir.webcontent = "${itemDir}/src/web/content"

    abuild.default.dir.dist = "${buildDir}/dist"
    abuild.default.dir.classes = "${buildDir}/classes"
    abuild.default.dir.generatedDoc = "${buildDir}/doc"
    abuild.default.dir.generatedSrc = "${buildDir}/src/java"
    abuild.default.dir.generatedResources = "${buildDir}/src/resources"
    abuild.default.dir.generatedConf = "${buildDir}/src/conf"
    abuild.default.dir.generatedMetainf = "${buildDir}/src/conf/metainf"
    abuild.default.dir.generatedWeb = "${buildDir}/src/web"
    abuild.default.dir.generatedWebcontent = "${buildDir}/src/web/content"
}
