import org.abuild.groovy.Util

class JavaRules
{
    def abuild
    def ant
    def pathSep

    List<String> defaultCompileClassPath = []
    List<String> defaultManifestClassPath = []
    List<String> defaultPackageClassPath = []
    List<String> defaultWrapperClassPath = []

    JavaRules(abuild, ant)
    {
        this.abuild = abuild
        this.ant = ant
        this.pathSep = ant.project.properties['path.separator']
    }

    def getPathVariable(String var)
    {
        def result = abuild.resolveAsString("java.dir.${var}")
        if (! new File(result).isAbsolute())
        {
            result = "${itemDir}/${result}"
        }
        result
    }

    def initTarget()
    {
        // We have three classpath interface variables that we combine
        // in various ways to initialize our various classpath
        // variables here.  See java_help.txt for details.

        defaultCompileClassPath.addAll(
            abuild.resolve('abuild.classpath') ?: [])
        defaultCompileClassPath.addAll(
            abuild.resolve('abuild.classpath.external') ?: [])

        defaultManifestClassPath.addAll(
            abuild.resolve('abuild.classpath.manifest') ?: [])

        defaultPackageClassPath.addAll(
            abuild.resolve('abuild.classpath') ?: [])

        defaultWrapperClassPath.addAll(defaultCompileClassPath)

        // Filter out jars built by this build item from the compile
        // and manifest classpaths.
        def dist = getPathVariable('dist')
        defaultCompileClassPath = defaultCompileClassPath.grep {
            new File(it).parent != dist
        }
        defaultManifestClassPath = defaultManifestClassPath.grep {
            new File(it).parent != dist
        }
    }

    def compile(Map attributes)
    {
        def defaultAttrs = [
            'srcdirs': ['src', 'generatedSrc'].collect {getPathVariable(it) },
            'destdir': getPathVariable('classes'),
            'classpath': this.defaultCompileClassPath,
            // Would be nice to turn path warnings back on
            'compilerargs': ['-Xlint', '-Xlint:-path']
        ]

        Util.addDefaults(attributes, defaultAttrs)

        attributes['srcdirs'] = attributes['srcdirs'].grep {
            dir -> new File(dir).isDirectory()
        }
        if (! attributes['srcdirs'])
        {
            return
        }

        // Remove attributes that are handled specially
        def compileClassPath = attributes.remove('classpath')
        def includes = attributes.remove('includes')
        def excludes = attributes.remove('excludes')
        def compilerargs = attributes.remove('compilerargs')
        def srcdirs = attributes.remove('srcdirs')

        def javacArgs = attributes
        javacArgs['classpath'] = compileClassPath.join(pathSep)
        ant.mkdir('dir' : attributes['destdir'])
        ant.javac(javacArgs) {
            srcdirs.each { dir -> src('path' : dir) }
            compilerargs?.each { arg -> compilerarg('value' : arg) }
            includes?.each { include('name' : it) }
            excludes?.each { exclude('name' : it) }
        }
    }

    def compileTarget()
    {
        abuild.runActions('java.compile', this.&compile)
    }

    def packageJar(Map attributes)
    {
        def defaultAttrs = [
            'distdir': getPathVariable('dist')
        ]

        Util.addDefaults(attributes, defaultAttrs)

        // XXX REDO based on new paradigm
        def jarName = attributes.remove('jarname')
        def mainClass = abuild.resolveAsString('java.mainClass')
        if (! jarName)
        {
            return
        }

        def distDir = attributes.remove('distdir')
        def classesDir = getPathVariable('classes')

        def manifestClassPath =
            abuild.resolveAsList('abuild.classpath.manifest', [])
        if (! manifestClassPath)
        {
            // XXX What do we want to do here when
            // abuild.classpath.manifest is not defined?

            // XXX duplicated
            manifestClassPath.addAll(
                abuild.resolve('abuild.classpath') ?: [])
            // XXX include external?
            manifestClassPath.addAll(
                abuild.resolve('abuild.classpath.external') ?: [])
        }

        ant.mkdir('dir' : distDir)
        ant.jar('destfile' : "${distDir}/${jarName}") {

            // call metainf('dir' : dir) for each metainf directory
            if (new File(classesDir).isDirectory())
            {
                fileset('dir' : classesDir)
            }
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
/*
                otherManifestAttributes.each() {
                    key, value -> attribute('name' : key, 'value' : value)
                }
*/
            }
        }
    }

    def packageJarTarget()
    {
        // XXX coverage
        def packageJarActions = abuild.resolveAsList('java.packageJar')
        def jarName = abuild.resolve('java.jarName')
        if (jarName)
        {
            if (packageJarActions)
            {
                if ((packageJarActions.size() == 1) &&
                    (packageJarActions[0] instanceof Map) &&
                    (! packageJarActions[0].containsKey('jarname')))
                {
                    packageJarActions[0]['jarname'] = jarName
                }
                else
                {
                    fail('java.jarName and java.packageJar may not both be ' +
                         ' set unless java.packageJar is a single map with' +
                         ' no jarname key')
                }
            }
            else
            {
                packageJarActions = [['jarname': jarName]]
            }
        }
        abuild.runActions(packageJarActions, this.&packageJar)
    }

    def javadocTarget()
    {
        if (! javadocTitle)
        {
            return
        }
        def srcDirs = [srcDir, generatedSrcDir].grep {
            dir -> new File(dir).isDirectory()
        }
        if (! srcDirs)
        {
            return
        }
        def sourcePath = srcDirs.join(pathSep)
        def access = abuild.resolveAsString('javadoc.accessLevel', 'protected')
        ant.javadoc(['sourcepath' : sourcePath, 'destdir' : docDir,
                     'access' : access, 'Windowtitle' : javadocTitle,
                     'Doctitle' : javadocTitle])
        {
            constructLinkPaths().each() {
                linkPath ->
                link('href' : linkPath)
            }
        }
    }

    private List<String> constructLinkPaths()
    {
        def javadocPaths = []
        abuild.itemPaths.each() {
            item, path ->
            File javadocPath = new File("$path/abuild-java/doc")
            if (javadocPath.isDirectory())
            {
                ant.echo("Build item has javadocs:" +
                         " ($item, ${javadocPath.path})")

                javadocPaths.add(Util.absToRel(destinationDir, javadocPath))
            }
            else
            {
                ant.echo("Build item has no javadocs:" +
                         " ($item, $path")
            }
        }
        return javadocPaths
    }

    def wrapperTarget()
    {
        // XXX REDO
        def wrapperName = abuild.resolveAsString('java.wrapperName')
        def mainClass = abuild.resolveAsString('java.mainClass')
        def jarName = abuild.resolveAsString('java.jarName')
        if (! (wrapperName && mainClass && jarName))
        {
            return
        }
        def buildDir = abuild.buildDirectory.absolutePath
        def wrapperPath = "${buildDir}/${wrapperName}"
        def distDir = getPathVariable('dist')

        def wrapperClassPath = []
        // XXX duplicated
        wrapperClassPath.addAll(
            abuild.resolve('abuild.classpath') ?: [])
        // XXX include external?
        wrapperClassPath.addAll(
            abuild.resolve('abuild.classpath.external') ?: [])
        wrapperClassPath << "${distDir}/${jarName}"
        wrapperClassPath = wrapperClassPath.join(pathSep)
        if (Util.inWindows)
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

def javaRules = new JavaRules(abuild, ant)

abuild.addTargetClosure('init', javaRules.&initTarget)
abuild.addTargetDependencies('all', ['package', 'wrapper'])
abuild.addTargetDependencies('package', ['init', 'package-jar'])
abuild.addTargetDependencies('generate', ['init'])
abuild.addTargetDependencies('doc', ['javadoc'])
abuild.configureTarget('compile', 'deps' : ['generate'],
                       javaRules.&compileTarget)
abuild.configureTarget('package-jar', 'deps' : ['compile'],
                       javaRules.&packageJarTarget)
abuild.configureTarget('javadoc', 'deps' : ['compile'],
                       javaRules.&javadocTarget)
abuild.configureTarget('wrapper', 'deps' : ['package-jar'],
                       javaRules.&wrapperTarget)
