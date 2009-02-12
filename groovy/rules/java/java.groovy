def buildDir = ant.project.properties['basedir']
def itemDir = new File(buildDir).parentFile.absolutePath

// XXX Figure out what we should allow the user to override.  Then use
// getVariable with these as default values.
def srcDir = "${itemDir}/src/java"
def classesDir = "${buildDir}/classes"
def distDir = "${buildDir}/dist"

def srcJavaDir = "${itemDir}/src/java"

abuild.setTarget('all', 'deps' : ['package-jar', 'wrapper'])

abuild.setTarget('generate')

abuild.setTarget('compile', 'deps' : ['generate']) {
    // XXX debug=..., deprecation=...
    def classPath = abuild.ifc['abuild.classpath'].join(';')
    ant.mkdir('dir' : classesDir)
    ant.javac('destdir' : classesDir, 'classpath' : classPath) {
        src('path' : srcDir)
        compilerarg('value' : '-Xlint')
        compilerarg('value' : '-Xlint:-path')
    }
}

abuild.setTarget('package-jar', 'deps' : ['compile']) {
    def jarName = abuild.getVariable('abuild.jar.jar-name')
    if (! jarName)
    {
        return
    }
    def mainClass = abuild.getVariable('abuild.jar.main-class')
    def classPath = abuild.ifc['abuild.classpath'].collect {
        new File(it).name }.join(':')
    ant.mkdir('dir' : distDir)
    ant.jar('destfile' : "${distDir}/${jarName}") {
        fileset('dir' : classesDir) {
            include('name' : '**/*.class')
        }
        manifest() {
            attribute('name' : 'Class-Path', 'value' : classPath)
            if (mainClass)
            {
                attribute('name' : 'Main-Class', 'value' : mainClass)
            }
        }
    }
}

abuild.setTarget('wrapper') {
    def wrapperName = abuild.getVariable('abuild.jar.wrapper-name')
    def mainClass = abuild.getVariable('abuild.jar.main-class')
    def jarName = abuild.getVariable('abuild.jar.jar-name')
    if (! (wrapperName && mainClass && jarName))
    {
        return
    }
    // XXX fix path separators everywhere in this file with a top-level def
    def classPathList = [*abuild.ifc['abuild.classpath'],
        "${distDir}/${jarName}"]
    def classPath = classPathList.join(':')

//  <if>
//   <os family="windows"/>
//   <then>
//    <echo file="${abuild.wrapper-name}.bat">@echo off
//java -classpath ${wrapper.classpath} ${abuild.main-class} %1 %2 %3 %4 %5 %6 %7 %8 %9
//</echo>
//    <!-- in case we're in cygwin... -->
//    <echo file="${abuild.wrapper-name}">#!/bin/sh
//exec `dirname $0`/`basename $0`.bat ${1+"$@"}
//</echo>
//    <chmod file="${abuild.wrapper-name}" perm="a+x"/>
//   </then>
//   <else>
    ant.echo('file' : "${buildDir}/${wrapperName}",
             """#!/bin/sh
exec java -classpath ${classPath} ${mainClass} ${1+"\$@"}
""")
    ant.chmod('file' : "${buildDir}/${wrapperName}", 'perm' : 'a+x');
}
