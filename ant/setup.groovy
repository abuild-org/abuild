def basedir = properties['basedir']
def files = []
// Plugins: load plugin-ant.xml in each path
properties['abuild.plugins']?.split(/,\s*/).each {
    files << new File("${it}/plugin-ant.xml")
}

// Hook build items: load ant-hooks.xml in each build item
def hookBuildItems = properties['abuild.hook-build-items']
hookBuildItems?.trim()?.split(/,\s*/).each {
    files << new File(properties["abuild.dir.${it}"] + "/ant-hooks.xml")
}

// Local hooks: load local-buildfile
if (properties.containsKey('abuild.local-buildfile'))
{
    files << new File(properties['abuild.private.dir.item'] + "/" +
		      properties['abuild.local-buildfile'])
}

// Filter out files that don't exist.
files = files.grep { it.isFile() }

// Save the list of hook files and also the ant builder for later
project.addReference('abuild.ant.hook-files', files)
project.addReference('abuild.ant.AntBuilder', ant)
