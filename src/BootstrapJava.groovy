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

def ant = new AntBuilder()
def antJar = ant.project.getProperty('ant.core.lib')

ant.project.setBasedir(buildDir.absolutePath)
ant.taskdef('name': 'groovyc',
            'classname': 'org.codehaus.groovy.ant.Groovyc')

def distDir = 'dist'
def classesDir = 'classes'
ant.mkdir('dir': distDir);
ant.mkdir('dir': classesDir);

ant.javac('deprecation': 'yes',
          'destdir': classesDir,
          'classpath': antJar,
          'srcdir': srcDir.absolutePath)
ant.groovyc('destdir': classesDir,
            'srcdir': srcDir.absolutePath)
ant.jar('destfile': distDir + '/abuild-java-support.jar',
        'basedir': classesDir,
        'includes': '**/*.class')
