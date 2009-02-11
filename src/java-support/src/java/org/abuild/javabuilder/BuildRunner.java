package org.abuild.javabuilder;

import java.util.Vector;
import java.util.Map;
import org.apache.tools.ant.Project;

interface BuildRunner
{
    public boolean invokeBackend(
	String buildFile, String dir, BuildArgs buildArgs, Project antProject,
	Vector<String> targets, Map<String, String> defines);
}
