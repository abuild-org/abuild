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
            result = new File(abuild.sourceDirectory, result)
        }
        // Wrap this in a file object and call absolutePath so
        // paths are formatted appropriately for the operating
        // system.
        new File(result).absolutePath
    }

    def getPathListVariable(String var)
    {
        abuild.resolveAsList("java.dir.${var}").collect {
            if (new File(it).isAbsolute())
            {
                new File(it).absolutePath
            }
            else
            {
                new File(abuild.sourceDirectory, it).absolutePath
            }
        }
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
        def srcdirs = attributes.remove('srcdirs')
        srcdirs.addAll(attributes.remove('extrasrcdirs'))

        srcdirs = srcdirs.grep { dir -> new File(dir).isDirectory() }
        if (! srcdirs)
        {
            return
        }

        // Remove attributes that are handled specially
        def compileClassPath = attributes.remove('classpath')
        def includes = attributes.remove('includes')
        def excludes = attributes.remove('excludes')
        def compilerargs = attributes.remove('compilerargs')

        def javacAttrs = attributes
        javacAttrs['classpath'] = compileClassPath.join(pathSep)
        ant.mkdir('dir' : attributes['destdir'])
        ant.javac(javacAttrs) {
            srcdirs.each { dir -> src('path' : dir) }
            compilerargs?.each { arg -> compilerarg('value' : arg) }
            includes?.each { include('name' : it) }
            excludes?.each { exclude('name' : it) }
        }
    }

    def compileTarget()
    {
        def defaultAttrs = [
            'srcdirs': ['src', 'generatedSrc'].collect {getPathVariable(it) },
            'extrasrcdirs' : getPathListVariable('extraSrc'),
            'destdir': getPathVariable('classes'),
            'classpath': this.defaultCompileClassPath,
            // Would be nice to turn path warnings back on
            'compilerargs': ['-Xlint', '-Xlint:-path'],
            'debug': 'true',
            'deprecation': 'on'
        ]
        abuild.runActions('java.compile', this.&compile, defaultAttrs)
    }

    def packageJar(Map attributes)
    {
        // Remove keys that we will handle expicitly
        def jarname = attributes.remove('jarname')
        if (! jarname)
        {
            return
        }

        def distdir = attributes.remove('distdir')
        def classesdir = attributes.remove('classesdir')
        def resourcesdirs = attributes.remove('resourcesdirs')
        resourcesdirs.addAll(attributes.remove('extraresourcesdirs'))
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
        manifestClassPath = manifestClassPath.collect { new File(it).name }

        // Filter out non-existent directories
        def filesets = [classesdir, resourcesdirs].flatten().grep {
            new File(it).isDirectory()
        }
        metainfdirs = metainfdirs.grep {
            new File(it).isDirectory()
        }

        ant.mkdir('dir' : distdir)
        def jarAttrs = attributes
        jarAttrs['destfile'] = "${distdir}/${jarname}"
        ant.jar(jarAttrs) {
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
        def defaultAttrs = [
            'jarname': abuild.resolveAsString('java.jarName'),
            'distdir': getPathVariable('dist'),
            'classesdir': getPathVariable('classes'),
            'resourcesdirs': [getPathVariable('resources'),
                              getPathVariable('generatedResources')],
            'extraresourcesdirs' : getPathListVariable('extraResources'),
            'metainfdirs' : [getPathVariable('metainf'),
                             getPathVariable('generatedMetainf')],
            'extrametainfdirs' : getPathListVariable('extraMetainf'),
            'mainclass' : abuild.resolveAsString('java.mainClass'),
            'manifestclasspath' : defaultManifestClassPath,
            'extramanifestkeys' : [:]
        ]

        abuild.runActions('java.packageJar', this.&packageJar, defaultAttrs)
    }

    def packageWar(Map attributes)
    {
        // Remove keys that we will handle expicitly
        def warname = attributes.remove('warname')
        def webxml = attributes.remove('webxml')
        def wartype = attributes.remove('wartype')
        if (! (warname && webxml && wartype))
        {
            return
        }
        if (! ((wartype == 'client') || (wartype == 'server')))
        {
            fail('war type must be either "client" or "server"')
        }
        boolean clientWar = (wartype == 'client')

        if (! new File(webxml).isAbsolute())
        {
            webxml = new File(abuild.sourceDirectory, webxml).absolutePath
        }

        def distdir = attributes.remove('distdir')
        def classesdir = attributes.remove('classesdir')
        def webdirs = attributes.remove('webdirs')
        webdirs.addAll(attributes.remove('extrawebdirs'))
        def metainfdirs = attributes.remove('metainfdirs')
        metainfdirs.addAll(attributes.remove('extrametainfdirs'))
        def webinfdirs = attributes.remove('webinfdirs')
        webinfdirs.addAll(attributes.remove('extrawebinfdirs'))
        def mainclass = attributes.remove('mainclass')
        def manifestClassPath = attributes.remove('manifestclasspath')
        def archives = attributes.remove('archives')

        // Filter out non-existent directories
        webdirs = webdirs.grep { new File(it).isDirectory() }
        metainfdirs = metainfdirs.grep { new File(it).isDirectory() }
        webinfdirs = webinfdirs.grep { new File(it).isDirectory() }

        ant.mkdir('dir' : distdir)
        def warAttrs = attributes
        warAttrs['destfile'] = "${distdir}/${warname}"
        warAttrs['webxml'] = webxml
        ant.war(warAttrs) {
            webinfdirs.each { webinf('dir': it) }
            metainfdirs.each { metainf('dir': it) }
            webdirs.each { fileset('dir': it) }
            if (new File(classesdir).isDirectory())
            {
                classes('dir': classesdir)
            }
            archives.each {
                File f = new File(it)
                if (f.absolutePath !=
                    new File("${distdir}/${warname}").absolutePath)
                {
                    if (clientWar)
                    {
                        fileset('dir': f.parent, 'includes': f.name)
                    }
                    else
                    {
                        lib('dir': f.parent, 'includes': f.name)
                    }
                }
            }
        }
    }

    def packageWarTarget()
    {
        def defaultAttrs = [
            'warname': abuild.resolveAsString('java.warName'),
            'webxml': abuild.resolveAsString('java.webxml'),
            'wartype': abuild.resolveAsString('java.warType'),
            'distdir': getPathVariable('dist'),
            'classesdir': getPathVariable('classes'),
            'webdirs': [getPathVariable('webContent'),
                        getPathVariable('generatedWebContent')],
            'extrawebdirs' : getPathListVariable('extraWebContent'),
            'metainfdirs' : [getPathVariable('metainf'),
                             getPathVariable('generatedMetainf')],
            'extrametainfdirs' : getPathListVariable('extraMetainf'),
            'webinfdirs' : [getPathVariable('webinf'),
                            getPathVariable('generatedWebinf')],
            'extrawebinfdirs' : getPathListVariable('extraWebinf'),
            'archives' : defaultPackageClassPath,
        ]

        abuild.runActions('java.packageWar', this.&packageWar, defaultAttrs)
    }

    def packageEar(Map attributes)
    {
        // Remove keys that we will handle expicitly
        def earname = attributes.remove('earname')
        def appxml = attributes.remove('appxml')
        if (! (earname && appxml))
        {
            return
        }
        if (! new File(appxml).isAbsolute())
        {
            appxml = new File(abuild.sourceDirectory, appxml).absolutePath
        }

        def distdir = attributes.remove('distdir')
        def resourcesdirs = attributes.remove('resourcesdirs')
        resourcesdirs.addAll(attributes.remove('extraresourcesdirs'))
        def metainfdirs = attributes.remove('metainfdirs')
        metainfdirs.addAll(attributes.remove('extrametainfdirs'))
        def mainclass = attributes.remove('mainclass')
        def manifestClassPath = attributes.remove('manifestclasspath')
        def archives = attributes.remove('archives')

        // Filter out non-existent directories
        resourcesdirs = resourcesdirs.grep { new File(it).isDirectory() }
        metainfdirs = metainfdirs.grep { new File(it).isDirectory() }

        ant.mkdir('dir' : distdir)
        def earAttrs = attributes
        earAttrs['destfile'] = "${distdir}/${earname}"
        earAttrs['appxml'] = appxml
        ant.ear(earAttrs) {
            metainfdirs.each { metainf('dir': it) }
            resourcesdirs.each { fileset('dir': it) }
            archives.each {
                File f = new File(it)
                if (f.absolutePath !=
                    new File("${distdir}/${earname}").absolutePath)
                {
                    fileset('dir': f.parent, 'includes': f.name)
                }
            }
        }
    }

    def packageEarTarget()
    {
        def defaultAttrs = [
            'earname': abuild.resolveAsString('java.earName'),
            'appxml': abuild.resolveAsString('java.appxml'),
            'distdir': getPathVariable('dist'),
            'resourcesdirs': [getPathVariable('resources'),
                              getPathVariable('generatedResources')],
            'extraresourcesdirs' : getPathListVariable('extraResources'),
            'metainfdirs' : [getPathVariable('metainf'),
                             getPathVariable('generatedMetainf')],
            'extrametainfdirs' : getPathListVariable('extraMetainf'),
            'archives' : defaultPackageClassPath,
        ]

        abuild.runActions('java.packageEar', this.&packageEar, defaultAttrs)
    }

    def javadocTarget()
    {
        def title = abuild.resolveAsString('java.javadocTitle')
        def defaultAttrs = [
            'Doctitle': title,
            'Windowtitle': title,
            'srcdirs': ['src', 'generatedSrc'].collect {getPathVariable(it) },
            'classpath': this.defaultCompileClassPath,
            'extrasrcdirs': getPathListVariable('extraSrc'),
            'access': abuild.resolveAsString('java.doc.accessLevel',
                                             'protected'),
            'destdir': getPathVariable('generatedDoc')
        ]

        abuild.runActions('java.javadoc', this.&javadoc, defaultAttrs)
    }

    def javadoc(Map attributes)
    {
        def srcdirs = attributes.remove('srcdirs')
        srcdirs.addAll(attributes.remove('extrasrcdirs'))
        srcdirs = srcdirs.grep { dir -> new File(dir).isDirectory() }
        if (! srcdirs)
        {
            return
        }

        def javadocAttrs = attributes
        javadocAttrs['sourcepath'] = srcdirs.join(pathSep)
        javadocAttrs['classpath'] = attributes['classpath'].join(pathSep)
        def linkPaths = constructLinkPaths(attributes['destdir'])
        ant.javadoc(javadocAttrs) {
            linkPaths.each() {
                linkPath ->
                link('href' : linkPath)
            }
        }
    }

    private List<String> constructLinkPaths(String destinationDir)
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

    def wrapper(Map attributes)
    {
        def wrapperName = attributes['name']
        def mainClass = attributes['mainclass']
        def jarName = attributes['jarname']
        if (! (wrapperName && mainClass && jarName))
        {
            return
        }
        def wrapperDir = attributes['dir']
        def wrapperPath = "$wrapperDir/$wrapperName"
        def distDir = attributes['distdir']
        def wrapperClassPath = attributes['classpath']
        wrapperClassPath << "$distDir/$jarName"
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

    def wrapperTarget()
    {
        def buildDir = abuild.buildDirectory.absolutePath
        def defaultAttrs = [
            'name': abuild.resolveAsString('java.wrapperName'),
            'mainclass': abuild.resolveAsString('java.mainClass'),
            'jarname': abuild.resolveAsString('java.jarName'),
            'dir': abuild.buildDirectory.absolutePath,
            'distdir': getPathVariable('dist'),
            'classpath': defaultWrapperClassPath
        ]

        abuild.runActions('java.wrapper', this.&wrapper, defaultAttrs)
    }
}

def javaRules = new JavaRules(abuild, ant)

abuild.addTargetClosure('init', javaRules.&initTarget)
abuild.addTargetDependencies('all', ['package', 'wrapper'])
abuild.addTargetDependencies('package',
                             ['init',
                              'package-jar',
                              'package-war',
                              'package-ear'
                             ])
abuild.addTargetDependencies('generate', ['init'])
abuild.addTargetDependencies('doc', ['javadoc'])
abuild.configureTarget('compile', 'deps' : ['generate'],
                       javaRules.&compileTarget)
abuild.configureTarget('package-jar', 'deps' : ['compile'],
                       javaRules.&packageJarTarget)
abuild.configureTarget('package-war', 'deps' : ['compile'],
                       javaRules.&packageWarTarget)
abuild.configureTarget('package-ear', 'deps' : ['compile'],
                       javaRules.&packageEarTarget)
abuild.configureTarget('javadoc', 'deps' : ['compile'],
                       javaRules.&javadocTarget)
abuild.configureTarget('wrapper', 'deps' : ['package-jar'],
                       javaRules.&wrapperTarget)
