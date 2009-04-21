package org.abuild.ant

import org.apache.tools.ant.Project
import org.apache.tools.ant.Task
import org.abuild.ant.CheckForTarget

class HookRunner extends Task
{
    String name
    public void execute ()
    {
	AntBuilder ant = project.getReference("abuild.ant.AntBuilder")
	def hookFiles = project.getReference("abuild.ant.hook-files")
	def target = "-${name}"
	hookFiles.each {
	    file ->
	    if (CheckForTarget.hasTarget(project, file, target))
	    {
		ant.ant('antfile': file.path, 'target': target)
	    }
	}
    }
}
