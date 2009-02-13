
abuild.setTarget('test-only') {
    def src = abuild.sourceDirectory.path
    def qtest = new File("${src}/qtest")
    if (qtest.isDirectory())
    {
        def build = abuild.buildDirectory.path
        def tty = abuild.ifc['ABUILD_STDOUT_IS_TTY']
        def command = 'qtest-driver';
        def args = ['-datadir', '../qtest',
            '-bindirs', '..:.', '-covdir', '..',
            "-stdout-tty=${tty}"]
        if (System.getProperty('os.name') =~ /(?i:windows).*/)
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
            def toExport = abuild.getVariable('qtest.export', [])
            if (abuild.getVariable('TESTS'))
            {
                toExport << 'TESTS'
            }
            if (toExport)
            {
                // Windows wants environment variables to be all
                // upper-case.
                toExport.each {
                    env('key': it.toUpperCase(),
                        'value': abuild.getVariableAsString(it))
                }
            }
            args.each {
                arg('value':it)
            }
        }
    }
}
