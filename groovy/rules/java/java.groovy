import org.apache.tools.ant.taskdefs.condition.Os

// Initialize variables whose values will be available at
// initialization.  Use the init target to initialize things that we
// don't want to look at until we are actually building.

def buildDir = abuild.buildDirectory.absolutePath
def itemDir = abuild.sourceDirectory.absolutePath
def pathSep = ant.project.properties['path.separator']

// These are overridable defaults.
def defaultDistDir = "${buildDir}/dist"
def defaultClassesDir = "${buildDir}/classes"
def defaultSrcDir = "${itemDir}/src/java"
def defaultGeneratedSrcDir = "${buildDir}/src/java"
def defaultResourcesDir = "${itemDir}/src/resources"
def defaultGeneratedResourcesDir = "${buildDir}/src/resources"
def defaultConfDir = "${itemDir}/src/conf"
def defaultGeneratedConfDir = "${buildDir}/src/conf"

// These are initialized by the init target.
def distDir
def classesDir
def srcDir
def generatedSrcDir
def resourcesDir
def generatedResourcesDir
def confDir
def generatedConfDir

def compileClassPath
def jarName
def mainClass
def wrapperName

ant.taskdef('name': 'groovyc',
            'classname': 'org.codehaus.groovy.ant.Groovyc')

abuild.addTargetDependencies('all', ['package', 'wrapper'])

abuild.addTargetDependencies('package', ['init', 'package-jar'])

abuild.addTargetDependencies('generate', ['init'])

def getPathVariable(String var, defaultValue)
{
    def result = abuild.getVariable("java.dir.${var}", defaultValue)
    if (! new File(result).isAbsolute())
    {
        result = "${itemDir}/${result}"
    }
    result
}

abuild.addTargetClosure('init') {
    distDir =
        getPathVariable('dist', defaultDistDir)
    classesDir =
        getPathVariable('classes', defaultClassesDir)
    srcDir =
        getPathVariable('src', defaultSrcDir)
    generatedSrcDir =
        getPathVariable('generated-src', defaultGeneratedSrcDir)
    resourcesDir =
        getPathVariable('resources', defaultResourcesDir)
    generatedResourcesDir =
        getPathVariable('generated-resource', defaultGeneratedSrcDir)
    confDir =
        getPathVariable('conf', defaultConfDir)
    generatedConfDir =
        getPathVariable('generated-conf', defaultGeneratedConfDir)

    compileClassPath = abuild.getVariable('abuild.classpath', [])
    jarName = abuild.getVariable('java.jar-name')
    mainClass = abuild.getVariable('java.main-class')
    wrapperName = abuild.getVariable('java.wrapper-name')
}

abuild.configureTarget('compile', 'deps' : ['generate']) {
    def srcDirs = [srcDir, generatedSrcDir].grep {
        dir -> new File(dir).isDirectory()
    }
    if (! srcDirs)
    {
        return
    }
    def javacArgs = abuild.getVariable('java.javac-args', [:])
    javacArgs['destdir'] = classesDir
    javacArgs['classpath'] = compileClassPath.join(pathSep)
    ant.mkdir('dir' : classesDir)
    ant.javac(javacArgs) {
        srcDirs.each { dir -> src('path' : dir) }
        compilerarg('value' : '-Xlint')
        compilerarg('value' : '-Xlint:-path')
    }
    // Other args to groovyc?
    ant.groovyc('destdir' : classesDir,
                'classpath' : compileClassPath.join(pathSep)) {
        srcDirs.each { dir -> src('path' : dir) }
    }
}

abuild.configureTarget('package-jar', 'deps' : ['compile']) {
    if (! jarName)
    {
        return
    }
    def manifestClassPath = compileClassPath.collect { new File(it).name }
    ant.mkdir('dir' : distDir)
    ant.jar('destfile' : "${distDir}/${jarName}") {
        fileset('dir' : classesDir) {
            include('name' : '**/*.class')
        }
        // We could easily control how the manifest is constructed
        // using additional parameters.
        manifest() {
            if (manifestClassPath)
            {
                attribute('name' : 'Class-Path',
                          'value' : manifestClassPath.join(pathSep))
            }
            if (mainClass)
            {
                attribute('name' : 'Main-Class', 'value' : mainClass)
            }
        }
    }
}

abuild.configureTarget('wrapper', 'deps' : ['package-jar']) {
    if (! (wrapperName && mainClass && jarName))
    {
        return
    }
    def wrapperPath = "${buildDir}/${wrapperName}"

    def wrapperClassPath = [*compileClassPath, "${distDir}/${jarName}"].join(pathSep)
    if (Os.isFamily('windows'))
    {
        ant.echo('file' : "${wrapperPath}.bat", """@echo off
java -classpath ${wrapperClassPath} ${mainClass} %1 %2 %3 %4 %5 %6 %7 %8 %9
""")
        // In case we're in Cygwin...
        ant.echo('file' : wrapperPath, '''#!/bin/sh
exec `dirname $0`/`basename $0`.bat ${1+"$@"}
''')
    }
    else
    {
        ant.echo('file' : wrapperPath,
                 """#!/bin/sh
exec java -classpath ${wrapperClassPath} ${mainClass} ${1+"\$@"}
""")
    }
    ant.chmod('file' : wrapperPath, 'perm' : 'a+x');
}
