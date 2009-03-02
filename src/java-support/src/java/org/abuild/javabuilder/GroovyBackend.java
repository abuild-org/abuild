package org.abuild.javabuilder;

import java.io.File;
import java.util.List;
import java.util.Map;
import org.apache.tools.ant.Project;

public interface GroovyBackend
{
    boolean run(File buildDirectory, BuildArgs buildArgs, Project antProject,
		List<String> targets, Map<String, String> defines);
}
