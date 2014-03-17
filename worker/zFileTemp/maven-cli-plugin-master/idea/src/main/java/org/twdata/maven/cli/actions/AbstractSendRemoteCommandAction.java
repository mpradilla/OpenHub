package org.twdata.maven.cli.actions;

import com.intellij.openapi.actionSystem.AnAction;
import com.intellij.openapi.actionSystem.AnActionEvent;
import com.intellij.openapi.actionSystem.DataConstants;
import com.intellij.openapi.project.Project;
import org.twdata.maven.cli.CliClientProjectComponent;

/**
 * Sends a remote command
 */
public abstract class AbstractSendRemoteCommandAction extends AnAction {
    private final int index;

    public AbstractSendRemoteCommandAction(int index) {
        this.index = index;
    }

    public void actionPerformed(AnActionEvent e) {
        Project project = (Project) e.getDataContext().getData(DataConstants.PROJECT);
        CliClientProjectComponent comp = project.getComponent(CliClientProjectComponent.class);
        comp.sendCommand(index);
    }
}
