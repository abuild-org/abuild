import org.apache.tools.ant.taskdefs.condition.Os

class JavaRules
{
    // Initialized by the constructor from the script's binding
    def abuild
    def ant

    // Initialized by the constructor
    String buildDir
    String itemDir
    String pathSep

    String defaultDistDir
    String defaultClassesDir
    String defaultSrcDir
    String defaultGeneratedSrcDir
    String defaultResourcesDir
    String defaultGeneratedResourcesDir
    String defaultConfDir
    String defaultGeneratedConfDir

    // Initialized by the init target
    String distDir
    String classesDir
    String srcDir
    String generatedSrcDir
    String resourcesDir
    String generatedResourcesDir
    String confDir
    String generatedConfDir

    List compileClassPath
    String jarName
    String mainClass
    String wrapperName

    JavaRules(abuild, ant)
    {
        this.abuild = abuild
        this.ant = ant

        // Initialize variables whose values will be available during
        // abuild's initialization phase.  Use the init target to
        // initialize things that we don't want to look at until we
        // are actually building.

        this.buildDir = abuild.buildDirectory.absolutePath
        this.itemDir = abuild.sourceDirectory.absolutePath
        this.pathSep = ant.project.properties['path.separator']

        // These are overridable defaults.
        this.defaultDistDir = "${buildDir}/dist"
        this.defaultClassesDir = "${buildDir}/classes"
        this.defaultSrcDir = "${itemDir}/src/java"
        this.defaultGeneratedSrcDir = "${buildDir}/src/java"
        this.defaultResourcesDir = "${itemDir}/src/resources"
        this.defaultGeneratedResourcesDir = "${buildDir}/src/resources"
        this.defaultConfDir = "${itemDir}/src/conf"
        this.defaultGeneratedConfDir = "${buildDir}/src/conf"
    }

    def getPathVariable(String var, defaultValue)
    {
        def result = abuild.resolveAsString("java.dir.${var}", defaultValue)
        if (! new File(result).isAbsolute())
        {
            result = "${itemDir}/${result}"
        }
        result
    }

    def initTarget()
    {
        distDir =
            getPathVariable('dist', defaultDistDir)
        classesDir =
            getPathVariable('classes', defaultClassesDir)
        srcDir =
            getPathVariable('src', defaultSrcDir)
        generatedSrcDir =
            getPathVariable('generatedSrc', defaultGeneratedSrcDir)
        resourcesDir =
            getPathVariable('resources', defaultResourcesDir)
        generatedResourcesDir =
            getPathVariable('generatedResource', defaultGeneratedSrcDir)
        confDir =
            getPathVariable('conf', defaultConfDir)
        generatedConfDir =
            getPathVariable('generatedConf', defaultGeneratedConfDir)

        compileClassPath = abuild.resolveAsList('abuild.classpath', [])
        jarName = abuild.resolveAsString('java.jarName')
        mainClass = abuild.resolveAsString('java.mainClass')
        wrapperName = abuild.resolveAsString('java.wrapperName')
    }

    def compileTarget()
    {
        def srcDirs = [srcDir, generatedSrcDir].grep {
            dir -> new File(dir).isDirectory()
        }
        if (! srcDirs)
        {
            return
        }
        def javacArgs = abuild.resolve('java.javacArgs', [:])
        if (! (javacArgs instanceof Map))
        {
            fail("java.javacArgs must be a map")
        }
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

    def packageJarTarget()
    {
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

    def wrapperTarget()
    {
        if (! (wrapperName && mainClass && jarName))
        {
            return
        }
        def wrapperPath = "${buildDir}/${wrapperName}"

        def wrapperClassPath =
            [*compileClassPath, "${distDir}/${jarName}"].join(pathSep)
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
        ant.chmod('file' : wrapperPath, 'perm' : 'a+x')
    }
}

ant.taskdef('name': 'groovyc',
            'classname': 'org.codehaus.groovy.ant.Groovyc')

def java_rules = new JavaRules(abuild, ant)

abuild.addTargetDependencies('all', ['package', 'wrapper'])
abuild.addTargetDependencies('package', ['init', 'package-jar'])
abuild.addTargetDependencies('generate', ['init'])
abuild.addTargetClosure('init', java_rules.&initTarget)
abuild.configureTarget('compile', 'deps' : ['generate'],
                       java_rules.&compileTarget)
abuild.configureTarget('package-jar', 'deps' : ['compile'],
                       java_rules.&packageJarTarget)
abuild.configureTarget('wrapper', 'deps' : ['package-jar'],
                       java_rules.&wrapperTarget)
