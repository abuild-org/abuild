package org.abuild.ant;

import java.util.Stack;

import org.apache.tools.ant.Project;
import org.apache.tools.ant.DefaultLogger;
import org.apache.tools.ant.BuildEvent;
import org.apache.tools.ant.util.StringUtils;

/**
 * Extends DefaultLogger to change the target printing to behave
 * better in the case of nested targets.  We print the target name
 * before any line of output that was generated from a diffent target
 * than the last one we generated output for.  Does not print the
 * names of targets with no output.  Not tested for parallel targets.
 *
 */
public class AbuildLogger extends DefaultLogger {
    // The last target whose name we logged
    private String last_target;

    // Stack of targets we're currently running
    private Stack<String> target_stack = new Stack<String>();

    /**
     * Logs a message with the name of the target if this logger
     * allows information-level messages.
     *
     * @param event An event with any relevant extra information.
     *              Must not be <code>null</code>.
     */
    private void logTargetName(String target_name, BuildEvent event) {
        if (Project.MSG_INFO <= msgOutputLevel) {
            String msg = StringUtils.LINE_SEP
                + target_name + ":";
            printMessage(msg, out, event.getPriority());
            log(msg);
        }
    }

    /**
     * Push the name of the new target onto the target stack.
     *
     * @param event An event with any relevant extra information.
     *              Must not be <code>null</code>.
     */
    public void targetStarted(BuildEvent event) {
	target_stack.push(event.getTarget().getName());
    }

    /**
     * Remove the target at the top of the stack.  We might want to
     * check to make sure this is the target we expect it to be by
     * comparing the target name with the top of the stack.
     *
     * @param event Ignored.
     */
    public void targetFinished(BuildEvent event) {
	target_stack.pop();
    }

    /**
     * Logs a message, if the priority is suitable.  In non-emacs
     * mode, task level messages are prefixed by the task name which
     * is right-justified.  This is copied from DefaultLogger except
     * that we call logTargetName if needed.
     *
     * @param event A BuildEvent containing message information.
     *              Must not be <code>null</code>.
     */
    public void messageLogged(BuildEvent event) {
        int priority = event.getPriority();
        // Filter out messages based on priority
        if (priority <= msgOutputLevel) {

	    String this_target = null;
	    if (target_stack.size() > 0)
	    {
		this_target = target_stack.peek();
		if (this_target != last_target)
		{
		    logTargetName(this_target, event);
		}
		last_target = this_target;
	    }

	    super.messageLogged(event);
	}
    }
}
