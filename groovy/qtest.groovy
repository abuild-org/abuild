import org.abuild.groovy.Util
import org.abuild.ant.QTest

ant.taskdef('name': 'qtest', 'classname': 'org.abuild.ant.QTest')

abuild.addTargetDependencies('test-only', ['test-qtest'])
abuild.addTargetClosure('test-qtest') {
    def src = abuild.sourceDirectory.path
    def qtest = new File("${src}/qtest")
    if (qtest.isDirectory())
    {
        def tty = abuild.interfaceVars['ABUILD_STDOUT_IS_TTY']
        def toExport = abuild.resolveAsList('qtest.export', [])
        ['TESTS', 'TC_SRCS'].each {
            if (abuild.resolve(it) != null)
            {
                toExport << it
            }
        }

        def qtestAttrs = [
            'stdoutistty': (tty == "1"),
            'emacsmode': abuild.buildArgs.emacsMode
        ]
        if (toExport)
        {
            qtestAttrs['environment'] = toExport.collect {
                // Windows wants environment variables to be all
                // upper-case.
                it.toUpperCase() + '=' + abuild.resolveAsList(it).join(' ')
            }.join('\000')
        }

        ant.qtest(qtestAttrs)
    }
}
