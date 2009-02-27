package org.abuild.javabuilder;

import java.util.List;
import java.util.Map;
import org.apache.tools.ant.Project;

interface BuildRunner
{
    public boolean invokeBackend(
	String buildFile, String dir, BuildArgs buildArgs, Project antProject,
	List<String> targets, Map<String, String> defines);
}
