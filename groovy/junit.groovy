class JUnitSupport
{
    def abuild
    def ant
    def pathSep

    JUnitSupport(abuild, ant)
    {
        this.abuild = abuild
        this.ant = ant
        this.pathSep = ant.project.properties['path.separator']
    }

    def getPathVariable(String var)
    {
        // XXX This basic code is duplicated in several places.

        def result = abuild.resolveAsString("java.dir.${var}")
        if (! new File(result).isAbsolute())
        {
            result = new File(abuild.sourceDirectory, result)
        }
        new File(result).absolutePath
    }

    def runJunit(Map attributes)
    {
        def testsuite = attributes.remove('testsuite')
        if (! testsuite)
        {
            return
        }
        def distdir = attributes.remove('distdir')
        def reportdir = attributes.remove('reportdir')
        def testClassPath = attributes.remove('classpath')

        ant.mkdir('dir': distdir)
        def junitAttrs = attributes
        ant.junit(junitAttrs) {
            classpath {
                testClassPath.each {
                    pathelement('location': it)
                }
                fileset('dir': distdir, 'includes': '*.jar')
            }
            test('name': testsuite,
                 'todir': distdir) {
                formatter('type': 'xml')
            }
        }
        ant.junitreport('todir': distdir) {
            fileset('dir': distdir, 'includes':  'TEST-*.xml')
            report('format': 'frames', 'todir': reportdir)
        }
    }

    def target()
    {
        def defaultClassPath = []
        defaultClassPath.addAll(
            abuild.resolve('abuild.classpath') ?: [])
        defaultClassPath.addAll(
            abuild.resolve('abuild.classpath.external') ?: [])

        def buildDir = abuild.buildDirectory.absolutePath
        def defaultAttrs = [
            'testsuite': abuild.resolveAsString('junit.testsuite'),
            'classpath': defaultClassPath,
            'distdir': getPathVariable('dist'),
            'reportdir': new File(abuild.buildDirectory,
                                  'junit/html').absolutePath,
            'printsummary': 'yes',
            'haltonfailure': 'yes',
            'fork': 'true'
        ]

        abuild.runActions('junit.config', this.&runJunit, defaultAttrs)
    }
}

def junitSupport = new JUnitSupport(abuild, ant)
abuild.addTargetClosure('test-only', junitSupport.&target)
