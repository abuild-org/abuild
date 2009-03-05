import org.abuild.groovy.Util

abuild.addTargetClosure('test-only') {
    def src = abuild.sourceDirectory.path
    def qtest = new File("${src}/qtest")
    if (qtest.isDirectory())
    {
        def build = abuild.buildDirectory.path
        def tty = abuild.interfaceVars['ABUILD_STDOUT_IS_TTY']
        def command = 'qtest-driver';
        def args = ['-datadir', '../qtest',
            '-bindirs', '..:.', '-covdir', '..',
            "-stdout-tty=${tty}"]
        if (Util.inWindows)
        {
            command = 'perl'
            args = ['-S', 'qtest-driver', *args]
        }

        ant.exec('failonerror':'true', 'executable': command,
                 'dir': build)
        {
            if (! abuild.buildArgs.emacsMode)
            {
                env('key':'QTEST_EXTRA_MARGIN',
                    'value':12)
            }
            def toExport = abuild.resolveAsList('qtest.export', [])
            if (abuild.resolve('TESTS'))
            {
                toExport << 'TESTS'
            }
            if (toExport)
            {
                // Windows wants environment variables to be all
                // upper-case.
                toExport.each {
                    env('key': it.toUpperCase(),
                        'value': abuild.resolveAsList(it).join(' '))
                }
            }
            args.each {
                arg('value':it)
            }
        }
    }
}
