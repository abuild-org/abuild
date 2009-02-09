#!/usr/bin/env groovy

File srcDir = new File("java-support/src/java")
if (! srcDir.isDirectory())
{
    System.err.println "You must run this script from the abuild src directory"
    System.exit(2)
}
File buildDir = new File("java-support/abuild-java")
if (! buildDir.isDirectory())
{
    buildDir.mkdirs()
}
// XXX Maybe we should use our embedded version of ant, if any, or
// maybe it's okay to require this for bootstrapping.
def antHome = System.getenv('ANT_HOME')
if (antHome == null)
{
    System.err.println "ANT_HOME must be set"
    System.exit(2)
}
def antJar = antHome + "/lib/ant.jar"
if (! new File(antJar).isFile())
{
    System.err.println "Can't find ant.jar in ${antJar}"
    System.exit(2)
}

def ant = new AntBuilder()
ant.project.setBasedir(buildDir.absolutePath)
// XXX hard-coded groovy?
ant.taskdef('name': 'groovyc',
            'classname': 'org.codehaus.groovy.ant.Groovyc',
            'classpath': 'lib/groovy-all-1.5.7.jar')

def distDir = 'dist'
def classesDir = 'classes'
ant.mkdir('dir': distDir);
ant.mkdir('dir': classesDir);

// Not sure why includeantruntime doesn't seem to work here...
ant.javac('deprecation': 'yes',
          'destdir': classesDir,
          'classpath': antJar,
          'srcdir': srcDir.absolutePath)
ant.groovyc('destdir': classesDir,
            'srcdir': srcDir.absolutePath)
ant.jar('destfile': distDir + '/abuild-java-support.jar',
        'basedir': classesDir,
        'includes': '**/*.class')
