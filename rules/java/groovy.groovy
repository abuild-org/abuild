import org.abuild.groovy.Util

class GroovyRules
{
    def abuild
    def ant
    def pathSep

    List<String> defaultCompileClassPath = []

    GroovyRules(abuild, ant)
    {
        this.abuild = abuild
        this.ant = ant
        this.pathSep = ant.project.properties['path.separator']
    }

    def getPathVariable(String var, String prefix)
    {
        def result = abuild.resolveAsString("${prefix}.dir.${var}")
        if (! new File(result).isAbsolute())
        {
            result = "${itemDir}/${result}"
        }
        new File(result).absolutePath
    }

    def initTarget()
    {
        // We have three classpath interface variables that we combine
        // in various ways to initialize our various classpath
        // variables here.  See java_help.txt for details.  For
        // groovy, we are concerned only with the compile classpath.
        // We rely on the java rules for everythnig else.

        defaultCompileClassPath.addAll(
            abuild.resolve('abuild.classpath') ?: [])
        defaultCompileClassPath.addAll(
            abuild.resolve('abuild.classpath.external') ?: [])

        // Filter out jars built by this build item from the compile
        // classpath.
        def dist = getPathVariable('dist', 'java')
        defaultCompileClassPath = defaultCompileClassPath.grep {
            dir -> new File(dir).parent != dist
        }
    }

    def compile(Map attributes)
    {
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
        def srcdirs = attributes.remove('srcdirs')

        def groovycArgs = attributes
        groovycArgs['classpath'] = compileClassPath.join(pathSep)
        ant.mkdir('dir' : attributes['destdir'])
        ant.groovyc(groovycArgs) {
            srcdirs.each { dir -> src('path' : dir) }
            includes?.each { include('name' : it) }
            excludes?.each { exclude('name' : it) }
        }
    }

    def compileTarget()
    {
        def defaultAttrs = [
            'srcdirs': ['src', 'generatedSrc'].collect {
                getPathVariable(it, 'groovy')
            },
            'destdir': getPathVariable('classes', 'java'),
            'classpath': this.defaultCompileClassPath,
        ]

        abuild.runActions('groovy.compile', this.&compile, defaultAttrs)
    }
}

ant.taskdef('name': 'groovyc',
            'classname': 'org.codehaus.groovy.ant.Groovyc')

def groovyRules = new GroovyRules(abuild, ant)

if (! abuild.resolve('abuild.rules').grep { it == 'java' })
{
    abuild.fail('use of groovy rules requires use of java rules')
}

abuild.addTargetClosure('init', groovyRules.&initTarget)
abuild.addTargetClosure('compile', groovyRules.&compileTarget)
