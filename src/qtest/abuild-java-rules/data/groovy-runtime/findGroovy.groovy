abuild.addTargetClosure('all') {
    File lib = new File(abuild.abuildTop + "/lib")
    def groovyJar = lib.list().grep { it =~ /^groovy.*\.jar$/ }
    if (! ((groovyJar instanceof List) && (groovyJar.size() == 1)))
    {
        fail("unable to find groovyJar under ${lib.absolutePath}")
    }
    groovyJar = lib.absolutePath + '/' + groovyJar[0]
    File out = new File(abuild.buildDirectory.absolutePath + "/after.interface")
    out.write("abuild.classpath.external = ${groovyJar}\n")
}
