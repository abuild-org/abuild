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
            'extrasrcdirs' : [],
            'destdir': getPathVariable('classes'),
            'classpath': this.defaultCompileClassPath,
            // Would be nice to turn path warnings back on
            'compilerargs': ['-Xlint', '-Xlint:-path'],
            'debug': 'true',
            'deprecation': 'on'
        ]

        Util.addDefaults(attributes, defaultAttrs)

        def srcdirs = attributes.remove('srcdirs')
        srcdirs.addAll(attributes.remove('extrasrcdirs'))

        srcdirs = srcdirs.grep {dir -> new File(dir).isDirectory()}
        if (! srcdirs)
        {
            return
        }

        // Remove attributes that are handled specially
        def compileClassPath = attributes.remove('classpath')
        def includes = attributes.remove('includes')
        def excludes = attributes.remove('excludes')
        def compilerargs = attributes.remove('compilerargs')

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
        // The packageJarTask method will already have initialized the
        // jarname attribute based on the java.jarName parameter if
        // appropriate, so we don't have to check that here.
        def jarname = attributes.remove('jarname')
        if (! jarname)
        {
            return
        }

        // XXX need a way to associate filesets with directories

        def defaultAttrs = [
            'distdir': getPathVariable('dist'),
            'classesdir': getPathVariable('classes'),
            'resourcesdirs': [getPathVariable('resources'),
                              getPathVariable('generatedResources')],
            'extraresourcesdirs' : [],
            'confdirs': [getPathVariable('conf'),
                         getPathVariable('generatedConf')],
            'extraconfdirs' : [],
            'metainfdirs' : [getPathVariable('metainf'),
                             getPathVariable('generatedMetainf')],
            'extrametainfdirs' : [],
            'mainclass' : abuild.resolveAsString('java.mainClass'),
            'manifestclasspath' : defaultManifestClassPath,
            'extramanifestkeys' : [:]
        ]

        Util.addDefaults(attributes, defaultAttrs)

        // XXX What am I supposed to do with confdirs for a simple jar
        // task?

        // Remove keys that we will handle expicitly
        def distdir = attributes.remove('distdir')
        def classesdir = attributes.remove('classesdir')
        def resourcesdirs = attributes.remove('resourcesdirs')
        resourcesdirs.addAll(attributes.remove('extraresourcesdirs'))
        def confdirs = attributes.remove('confdirs')
        confdirs.addAll(attributes.remove('extraconfdirs'))
        def metainfdirs = attributes.remove('metainfdirs')
        metainfdirs.addAll(attributes.remove('extrametainfdirs'))
        def mainclass = attributes.remove('mainclass')
        def manifestClassPath = attributes.remove('manifestclasspath')
        def extramanifestkeys = attributes.remove('extramanifestkeys')

        if ((! manifestClassPath) && defaultCompileClassPath)
        {
            ant.echo('level':'warning',
                     "manifest class path is not set;" +
                     " using compile classpath as manifest classpath")
            manifestClassPath.addAll(defaultCompileClassPath)
        }

        // Take only last path element for each manifest class path
        manifestClassPath = manifestClassPath.each { new File(it).name }

        // Filter out non-existent directories
        def filesets = [classesdir, resourcesdirs].flatten().grep {
            new File(it).isDirectory()
        }
        metainfdirs = metainfdirs.grep {
            new File(it).isDirectory()
        }

        ant.mkdir('dir' : distdir)
        ant.jar('destfile' : "${distdir}/${jarname}") {
            metainfdirs.each { metainf('dir': it) }
            filesets.each { fileset('dir': it) }
            manifest() {
                if (manifestClassPath)
                {
                    attribute('name' : 'Class-Path',
                              'value' : manifestClassPath.join(pathSep))
                }
                if (mainclass)
                {
                    attribute('name' : 'Main-Class', 'value' : mainclass)
                }
                extramanifestkeys.each() {
                    key, value -> attribute('name' : key, 'value' : value)
                }
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
        // XXX REDO
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
        // XXX?
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
